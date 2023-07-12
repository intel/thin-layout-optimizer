#ifndef SRC_D_PERF_D_PERF_MAPPINGS_H_
#define SRC_D_PERF_D_PERF_MAPPINGS_H_

#include "src/perf/perf-sample.h"
#include "src/sym/syms.h"
#include "src/system/br-insn.h"
#include "src/util/global-stats.h"
#include "src/util/strbuf.h"
#include "src/util/type-info.h"
#include "src/util/umap.h"
#include "src/util/vec.h"
#include "src/util/xxhash.h"

////////////////////////////////////////////////////////////////////////////////
// Handle mapping pids -> address space. Used to translate IP (from events) to
// locations in the elf (for function/branch info).

namespace tlo {
namespace perf {


struct perf_map_info_t {
    // Keep timestamp incase multiple mappings (this is rarely if ever happens).
    uint64_t ts_;
    uint64_t base_;
    uint64_t size_;
    uint64_t off_;


    constexpr uint64_t
    contains_addr(uint64_t addr) const {
        return (addr - base_) < size_;
    }

    constexpr uint64_t
    unmap_addr(uint64_t addr) const {
        // Get address loc in elf file.
        return addr - base_ + off_;
    }

    struct cmp_t {
        template<typename T0_t, typename T1_t>
        constexpr bool
        operator()(T0_t const & lhs, T1_t const & rhs) {
            uint64_t lhs_ts, rhs_ts;
            if constexpr (std::is_integral_v<T0_t>) {
                lhs_ts = lhs;
            }
            else {
                lhs_ts = lhs.ts_;
            }

            if constexpr (std::is_integral_v<T1_t>) {
                rhs_ts = rhs;
            }
            else {
                rhs_ts = rhs.ts_;
            }

            return lhs_ts < rhs_ts;
        }
    };

    void
    dump(int vlvl, FILE * fp, const char * prefix = "") const {
        TLO_fprint_ifv(vlvl, fp, "%s%lx: [%lx + %lx -> %lx]\n", prefix, ts_,
                       base_, off_, size_);
    }
};
struct perf_dso_mappings_t {
    using mapping_t = small_vec_t<perf_map_info_t>;
    mapping_t mappings_;

    perf_dso_mappings_t() = default;

    bool
    merge(const perf_dso_mappings_t & other) {
        std::copy(other.mappings_.begin(), other.mappings_.end(),
                  std::back_inserter(mappings_));
        return true;
    }
    bool
    add_sample(const info_sample_t & sample) {
        assert(sample.is_mmap());
        const sample_mmap_t * mmap = sample.get_mmap();
        mappings_.emplace_back(
            perf_map_info_t{ sample.hdr_.timestamp_, mmap->map_base_,
                             mmap->map_size_, mmap->map_off_ });
        return true;
    }

    bool
    finalize() {
        std::sort(mappings_.begin(), mappings_.end(), perf_map_info_t::cmp_t{});
        return !mappings_.empty();
    }

    bool
    fillin_sample_loc(const sym::dso_t *   dso,
                      const sample_hdr_t & hdr,
                      sample_loc_t *       loc,
                      system::br_insn_t *  br_insn_out) const {
        for (auto it = mappings_.end(); it != mappings_.begin();) {
            --it;
            // Start from most recent and try to find map info that contains our
            // IP.
            if (it->ts_ <= hdr.timestamp_) {
                if (it->contains_addr(loc->mapped_addr_)) {
                    // Set samples unmapped addr field.
                    loc->unmapped_addr_ = it->unmap_addr(loc->mapped_addr_);
                    if (br_insn_out == nullptr) {
                        return true;
                    }
                    // Read raw bytes from file. They should be a branch.
                    std::array<uint8_t, system::k_max_insn_sz> insn_bytes;
                    uint64_t insn_loc = loc->unmapped_addr_;

                    dso->read_insn(insn_loc,
                                   { insn_bytes.data(), insn_bytes.size() });
                    system::br_insn_t br_insn =
                        system::br_insn_t::find(insn_bytes);
                    TLO_INCR_STAT(total_insn_searched_);
                    if (!br_insn.good()) {
                        if (tlo::has_verbosity(2)) {
                            TLO_perrvv("%s + %lx\n", dso->str(), insn_loc);
                            TLO_perrvv("\t.byte ");
                            for (uint32_t i = 0; i < insn_bytes.size(); ++i) {
                                TLO_perrvv("0x%02x", insn_bytes[i]);
                                if ((i + 1) < insn_bytes.size()) {
                                    TLO_perrvv(",");
                                }
                                else {
                                    TLO_perrvv("\n");
                                }
                            }
                        }
                    }
                    else {
                        TLO_INCR_STAT(total_insn_decoded_);
                    }


                    *br_insn_out = br_insn;

                    return true;
                }
            }
        }
        return false;
    }

    void
    dump(int vlvl, FILE * fp, const char * prefix = "") const {
        for (const auto & mapinfo : mappings_) {
            mapinfo.dump(vlvl, fp, prefix);
        }
    }
};

struct perf_pid_mappings_t {
    using mapping_t = basic_umap<strbuf_t<>, perf_dso_mappings_t>;
    mapping_t mappings_;
    template<bool k_unused>
    bool
    add_sample(strtab_t<k_unused> * stab, const info_sample_t & sample) {
        assert(sample.is_mmap());
        const sample_mmap_t * mmap_sample = sample.get_mmap();
        assert(mmap_sample != nullptr);
        auto res = mappings_.emplace(stab->get_sbuf(mmap_sample->dso_),
                                     perf_dso_mappings_t{});
        return res.first->second.add_sample(sample);
    }

    bool
    merge(const perf_pid_mappings_t & other) {
        bool ret = false;
        for (const auto & kvp : other.mappings_) {
            auto res = mappings_.emplace(kvp.first, perf_dso_mappings_t{});
            ret |= res.first->second.merge(kvp.second);
        }
        return ret;
    }

    bool
    finalize() {
        bool ret = false;
        for (auto & kvp : mappings_) {
            ret |= kvp.second.finalize();
        }
        return ret;
    }

    bool
    fillin_sample_loc(const sym::dso_t *   dso,
                      const sample_hdr_t & hdr,
                      sample_loc_t *       loc,
                      system::br_insn_t *  br_insn_out) const {
        auto res = mappings_.find(dso->name_.without_extra());
        if (res == mappings_.end()) {
            return false;
        }
        return res->second.fillin_sample_loc(dso, hdr, loc, br_insn_out);
    }

    void
    dump(int vlvl = 1, FILE * fp = stdout) const {
        if (!has_verbosity(vlvl)) {
            return;
        }
        for (const auto & dso_and_mappings : mappings_) {
            TLO_fprint_ifv(vlvl, fp, "%s\n", dso_and_mappings.first.str());
            dso_and_mappings.second.dump(vlvl, fp, "\t");
        }
    }
};

struct perf_mappings_t {
    using mapping_t = umap<uint64_t, perf_pid_mappings_t>;
    mapping_t mappings_;

    // Add an mmap sample.
    template<bool k_unused>
    bool
    add_sample(strtab_t<k_unused> * stab, const info_sample_t & sample) {
        assert(sample.is_mmap());
        const sample_mmap_t * mmap = sample.get_mmap();
        if (!mmap->is_executable()) {
            return false;
        }
        auto res = mappings_.emplace(mmap->pid_, perf_pid_mappings_t{});
        return res.first->second.add_sample(stab, sample);
    }

    // We forked so copy mappings from parent->child.
    bool
    add_fork_sample(const info_sample_t & sample) {
        assert(sample.is_fork());
        bool                  ret  = false;
        const sample_fork_t * fork = sample.get_fork();
        TLO_INCR_STAT(total_mappings_);
        if (fork->cpid_ != fork->ppid_) {
            auto pres = mappings_.find(fork->ppid_);
            if (pres != mappings_.end()) {
                auto res = mappings_.emplace(fork->cpid_, pres->second);
                ret      = res.second;
                if (!ret) {
                    TLO_INCR_STAT(total_bad_mappings_);
                    TLO_perr(
                        "Warning: fork (%u -> %u) duplicates pid (%u) at: %lu.%lu\n",
                        fork->ppid_, fork->cpid_, fork->cpid_,
                        sample.hdr_.timestamp_ >> 32U,
                        sample.hdr_.timestamp_ & 0xffffffff);
                    TLO_fprintv(stderr, "-- Existing --\n");
                    res.first->second.dump(1, stderr);
                    TLO_fprintv(stderr, "-- Combine With --\n");
                    pres->second.dump(1, stderr);
                    res.first->second.merge(pres->second);
                }
            }
        }
        return ret;
    }

    // Must be called before we start processing perf events. Called after done
    // processing mmap/fork events.
    bool
    finalize() {
        bool ret = false;
        for (auto & kvp : mappings_) {
            ret |= kvp.second.finalize();
        }
        return ret;
    }

    bool
    fillin_sample_loc_impl(const sym::dso_t *   dso,
                           const sample_hdr_t & hdr,
                           sample_loc_t *       loc,
                           system::br_insn_t *  br_insn_out,
                           uint64_t             pid_or_tpid) const {
        auto res = mappings_.find(pid_or_tpid);
        if (res == mappings_.end()) {
            return false;
        }
        return res->second.fillin_sample_loc(dso, hdr, loc, br_insn_out);
    }
    // Get the unmapped addr and optionally assosiated br_insn for the samples
    // IP.
    bool
    fillin_sample_loc(const sym::dso_t *   dso,
                      const sample_hdr_t & hdr,
                      sample_loc_t *       loc,
                      system::br_insn_t *  br_insn_out = nullptr) const {
        bool r = fillin_sample_loc_impl(dso, hdr, loc, br_insn_out, hdr.pid_);
        return r;
    }
};

}  // namespace perf
}  // namespace tlo


#endif
