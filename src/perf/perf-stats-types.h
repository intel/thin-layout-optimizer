#ifndef SRC_D_PERF_D_PERF_STATS_TYPES_H_
#define SRC_D_PERF_D_PERF_STATS_TYPES_H_

#include "src/perf/perf-sample.h"
#include "src/sym/syms.h"
#include "src/util/global-stats.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// edges/functions with assosiated stats from the profile.

namespace tlo {
namespace perf {

#define TLO_PSAMPLE_VAL_FMT "lf"
using psample_val_t = double;

// Assosiated with a given edge (also aggregated for sanity checks).
struct perf_edge_stats_t {
    // At the moment we only track the number edges.
    // Could add other stuff like pred/mispred or insn type or w.e.
    psample_val_t num_edges_;

    constexpr static perf_edge_stats_t
    create(const lbr_br_sample_t * sample) {
        return perf_edge_stats_t{ 1 };
        (void)sample;
    }

    constexpr perf_edge_stats_t
    add_br_sample(const lbr_br_sample_t * sample) {
        perf_edge_stats_t stat = create(sample);
        add(stat);
        return stat;
    }

    constexpr void
    add(perf_edge_stats_t const & other) {
        num_edges_ += other.num_edges_;
    }

    constexpr bool
    empty() const {
        return num_edges_ == 0;
    }

    constexpr const perf_edge_stats_t &
    stats() const {
        return *this;
    }

    void
    dump(FILE * fp, char const * prefix) const {
        fprintf(fp, "%snum_edges: %" TLO_PSAMPLE_VAL_FMT "\n", prefix,
                num_edges_);
    }

    constexpr bool
    eq(perf_edge_stats_t const & other) {
        return num_edges_ == other.num_edges_;
    }

    constexpr bool
    cmp(perf_edge_stats_t const & other) {
        return num_edges_ > other.num_edges_;
    }
};
static_assert(has_okay_type_traits<perf_edge_stats_t>::value);

// Assosiated with a given function (also aggregated, mostly for sanity checks
// elsewhere).
struct perf_func_stats_t {
    // Information stored on each function (obtained via simple samples and
    // branch samples).
    psample_val_t num_samples_;
    psample_val_t num_tracked_br_samples_in_;
    psample_val_t num_tracked_br_samples_out_;
    psample_val_t num_br_samples_in_;
    psample_val_t num_br_samples_out_;

    constexpr static perf_func_stats_t
    create(const simple_sample_t * sample) {
        return perf_func_stats_t{ 1, 0, 0, 0, 0 };
        (void)sample;
    }

    constexpr perf_func_stats_t
    add_simple_sample(const simple_sample_t * sample) {
        perf_func_stats_t stat = create(sample);
        add(stat);
        return stat;
    }

    constexpr static perf_func_stats_t
    create_br_sample(const lbr_br_sample_t * sample, bool in) {
        uint64_t in64 = static_cast<uint64_t>(in);
        return perf_func_stats_t{ 0, 0, 0, static_cast<psample_val_t>(in64),
                                  static_cast<psample_val_t>(1u - in64) };
        (void)sample;
    }


    constexpr perf_func_stats_t
    add_br_sample(const lbr_br_sample_t * sample, bool in) {
        perf_func_stats_t stat = create_br_sample(sample, in);
        add(stat);
        return stat;
    }

    constexpr static perf_func_stats_t
    create_edge_stats(const perf_edge_stats_t & estats, bool in) {
        return perf_func_stats_t{ 0, in ? estats.num_edges_ : 0,
                                  in ? 0 : estats.num_edges_, 0, 0 };
    }


    constexpr perf_func_stats_t
    add_edge_stats(const perf_edge_stats_t & estats, bool in) {
        perf_func_stats_t stat = create_edge_stats(estats, in);
        add(stat);
        return stat;
    }

    constexpr void
    add(perf_func_stats_t const & other) {
        num_samples_ += other.num_samples_;
        num_tracked_br_samples_in_ += other.num_tracked_br_samples_in_;
        num_tracked_br_samples_out_ += other.num_tracked_br_samples_out_;
        num_br_samples_in_ += other.num_br_samples_in_;
        num_br_samples_out_ += other.num_br_samples_out_;
    }


    constexpr bool
    empty() const {
        return num_samples_ == 0 && num_tracked_br_samples_in_ == 0 &&
               num_br_samples_in_ == 0 && num_tracked_br_samples_out_ == 0 &&
               num_br_samples_out_ == 0;
    }

    constexpr const perf_func_stats_t &
    stats() const {
        return *this;
    }


    constexpr bool
    valid() const {
        if (num_tracked_br_samples_in_ > num_br_samples_in_) {
            TLO_TRACE("More tracked than total in: %" TLO_PSAMPLE_VAL_FMT
                      " vs %" TLO_PSAMPLE_VAL_FMT "\n",
                      num_tracked_br_samples_in_, num_br_samples_in_);
            return false;
        }
        if (num_tracked_br_samples_out_ > num_br_samples_out_) {
            TLO_TRACE("More tracked than total out: %" TLO_PSAMPLE_VAL_FMT
                      " vs %" TLO_PSAMPLE_VAL_FMT " (%" TLO_PSAMPLE_VAL_FMT
                      " vs %" TLO_PSAMPLE_VAL_FMT ")\n",
                      num_tracked_br_samples_out_, num_br_samples_out_,
                      num_tracked_br_samples_in_, num_br_samples_in_);
            return false;
        }
        return true;
    }


    void
    dump(FILE * fp, char const * prefix) const {
        fprintf(fp,
                "%snum_samples                 : %" TLO_PSAMPLE_VAL_FMT "\n",
                prefix, num_samples_);
        fprintf(fp, "%snum_tracked_br_samples_in : %" TLO_PSAMPLE_VAL_FMT "\n",
                prefix, num_tracked_br_samples_in_);
        fprintf(fp, "%snum_tracked_br_samples_out: %" TLO_PSAMPLE_VAL_FMT "\n",
                prefix, num_tracked_br_samples_out_);
        fprintf(fp,
                "%snum_br_samples_in           : %" TLO_PSAMPLE_VAL_FMT "\n",
                prefix, num_br_samples_in_);
        fprintf(fp,
                "%snum_br_samples_out          : %" TLO_PSAMPLE_VAL_FMT "\n",
                prefix, num_br_samples_out_);
    }

    constexpr bool
    eq(perf_func_stats_t const & other) {
        return num_samples_ == other.num_samples_ &&
               num_tracked_br_samples_in_ == other.num_tracked_br_samples_in_ &&
               num_br_samples_in_ == other.num_br_samples_in_ &&
               num_tracked_br_samples_out_ ==
                   other.num_tracked_br_samples_out_ &&
               num_br_samples_out_ == other.num_br_samples_out_;
    }

    constexpr bool
    cmp(perf_func_stats_t const & other) {
        if (num_samples_ != other.num_samples_) {
            return num_samples_ > other.num_samples_;
        }
        if (num_tracked_br_samples_in_ != other.num_tracked_br_samples_in_) {
            return num_tracked_br_samples_in_ >
                   other.num_tracked_br_samples_in_;
        }
        if (num_tracked_br_samples_out_ != other.num_tracked_br_samples_out_) {
            return num_tracked_br_samples_out_ >
                   other.num_tracked_br_samples_out_;
        }
        if (num_br_samples_in_ != other.num_br_samples_in_) {
            return num_br_samples_in_ > other.num_br_samples_in_;
        }
        if (num_br_samples_out_ != other.num_br_samples_out_) {
            return num_br_samples_out_ > other.num_br_samples_out_;
        }
        return false;
    }
};
static_assert(has_okay_type_traits<perf_func_stats_t>::value);

struct perf_func_t {
    sym::func_clump_t *       func_clump_;
    mutable perf_func_stats_t stats_;

    constexpr perf_func_stats_t
    add_simple_sample(const simple_sample_t * sample) const {
        TLO_INCR_STAT(total_tracked_samples_);
        return stats_.add_simple_sample(sample);
    }

    constexpr perf_func_stats_t
    add_br_sample(const lbr_br_sample_t * sample, bool in) const {
        return stats_.add_br_sample(sample, in);
    }

    constexpr perf_func_stats_t
    add_edge_stats(const perf_edge_stats_t & estats, bool in) const {
        return stats_.add_edge_stats(estats, in);
    }

    constexpr const perf_func_stats_t &
    stats() const {
        return stats_;
    }

    constexpr psample_val_t
    samples() const {
        return stats_.num_samples_;
    }

    void
    cg_prepare() const {
        if (!func_clump_->is_cg_ready()) {
            func_clump_->cg_prepare();
        }
    }

    constexpr sym::addr_range_t
    get_addr_range() const {
        return func_clump_->get_addr_range();
    }

    void
    merge(const perf_func_t * other) {
        func_clump_->merge(other->func_clump_);
        stats_.add(other->stats_);
    }

    // Sanity tests
    bool
    valid() const {
        if (!stats().valid()) {
            TLO_TRACE("Invalid stats\n");
            return false;
        }
        if (!func_clump_->valid()) {
            TLO_TRACE("Invalid Func\n");
            return false;
        }
        // Unknown will get a lot of hits all over the place so don't enforce
        // size limit
        if (func_clump_->is_unknown()) {
            return true;
        }
        if (func_clump_->size() > sym::func_t::k_max_func_size) {
            TLO_TRACE("Impossible func size\n");
            return false;
        }
        return true;
    }

    void
    dump(FILE * fp = stdout) const {
        fprintf(fp, "\tDSO: %s\n", func_clump_->dso()->str());
        if (!valid()) {
            fprintf(fp, "\t\tINVALID\n");
        }
        func_clump_->dump(0, fp, "\t\t");
        stats().dump(fp, "\t\t\t");
    }

    static bool
    overlap(const perf_func_t * lhs, const perf_func_t * rhs) {
        return sym::func_clump_t::overlap(lhs->func_clump_, rhs->func_clump_);
    }


    constexpr bool
    eq(const perf_func_t & other) const {
        return func_clump_->eq(other.func_clump_);
    }

    constexpr bool
    cmp(const perf_func_t & other) const {
        if (!stats_.eq(other.stats_)) {
            return stats_.cmp(other.stats_);
        }
        return func_clump_->name_lt(other.func_clump_);
    }

    constexpr uint64_t
    hash() const {
        assert(!func_clump_->is_temporary());
        return func_clump_->hash();
    }
    struct cmp_t {
        constexpr bool
        operator()(perf_func_t const & lhs, perf_func_t const & rhs) {
            return lhs.cmp(rhs);
        }
    };
};
static_assert(has_okay_type_traits<perf_func_t>::value);

// A edge has a from -> to
struct perf_edge_t {
    sym::func_clump_t *       from_;
    sym::func_clump_t *       to_;
    system::br_insn_t         br_insn_;
    mutable perf_edge_stats_t stats_;


    constexpr perf_edge_stats_t
    add_br_sample(const lbr_br_sample_t * sample) const {
        return stats_.add_br_sample(sample);
    }

    constexpr void
    normalize_br_insn() {
        br_insn_ = system::br_insn_t::make_bad();
    }

    constexpr const perf_edge_stats_t &
    stats() const {
        return stats_;
    }

    // Sanity tests
    bool
    valid() const {
        if (!br_insn_.valid()) {
            TLO_TRACE("Invalid br\n");
            return false;
        }
        if (!from_->valid()) {
            TLO_TRACE("Invalid from\n");
            return false;
        }
        if (from_ != to_ && !to_->valid()) {
            TLO_TRACE("Invalid to\n");
            return false;
        }
        return true;
#if 0
        return from_ != to_ && from_->valid() && to_->valid() &&
               (!from_->ident_.eq(to_->ident_) ||
                !from_->name_.eq(to_->name_) ||
                !from_->dso()->name_.eq(to_->dso()->name_));
#endif
    }

    void
    dump(FILE * fp = stdout) const {
        if (!valid()) {
            (void)fprintf(fp, "\t\tINVALID\n");
        }
        (void)fprintf(fp, "\tDSO: %s -> %s\n", from_->dso()->str(),
                      to_->dso()->str());
        (void)fprintf(fp, "\t\tEdge(From)\n");
        from_->dump(0, fp, "\t\t");
        (void)fprintf(fp, "\t\tEdge(To)\n");
        to_->dump(0, fp, "\t\t");
        (void)fprintf(fp, "\t\tBr: %s\n", br_insn_.name());
        stats_.dump(fp, "\t\t\t");
    }

    constexpr bool
    eq(const perf_edge_t & other) const {
        return from_ == other.from_ && to_ == other.to_ &&
               br_insn_.eq(other.br_insn_);
    }

    constexpr bool
    eq(const perf_edge_t * other) const {
        return from_ == other->from_ && to_ == other->to_ &&
               br_insn_.eq(other->br_insn_);
    }

    void
    cg_prepare() const {
        if (!from_->is_cg_ready()) {
            from_->cg_prepare();
        }
        if (!to_->is_cg_ready()) {
            to_->cg_prepare();
        }
        assert(br_insn_.bad());
    }

    constexpr bool
    cmp(const perf_edge_t & other) const {
        if (!stats_.eq(other.stats_)) {
            return stats_.cmp(other.stats_);
        }
        if (from_ != other.from_) {
            return from_->name_lt(other.from_);
        }
        if (to_ != other.to_) {
            return to_->name_lt(other.to_);
        }
        return br_insn_.lt(other.br_insn_);
    }

    constexpr uint64_t
    hash() const {
        assert(!from_->is_temporary());
        assert(!to_->is_temporary());
        return from_->hash() ^ rol(to_->hash(), 1) ^ rol(br_insn_.hash(), 2);
    }

    struct cmp_t {
        constexpr bool
        operator()(perf_edge_t const & lhs, perf_edge_t const & rhs) {
            return lhs.cmp(rhs);
        }
    };
};
static_assert(has_okay_type_traits<perf_edge_t>::value);

}  // namespace perf
}  // namespace tlo

#endif
