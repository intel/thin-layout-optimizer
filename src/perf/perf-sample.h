#ifndef SRC_D_PERF_D_PERF_SAMPLE_H_
#define SRC_D_PERF_D_PERF_SAMPLE_H_

#include "src/sym/addr-range.h"

#include "src/system/br-insn.h"
#include "src/util/packed-ptr.h"
#include "src/util/type-info.h"

#include <stdint.h>
#include <variant>
////////////////////////////////////////////////////////////////////////////////
// Various sample type we expect.

namespace tlo {
namespace perf {

// Location of the sample. We have this for ALL samples.
struct sample_loc_t {
    uint64_t mapped_addr_;
    uint64_t unmapped_addr_;

    // NOTE: dso_ is not actually allocated! It is just a pointer to
    // where in the currently parsed line the same/dso are and an
    // assosiated length. Before moving onto the next line all dso_
    // fields in all relevant sample locations must handled.
    small_str_t<char const *> dso_;

    constexpr bool
    valid() const {
        return sym::addr_range_t{ mapped_addr_ }.valid() && dso_.active();
    }
};

// A given branch sample.
struct lbr_br_sample_t {
    static constexpr uint32_t k_mispred = 0;
    static constexpr uint32_t k_pred    = 1;
    static constexpr uint32_t k_unknown = 2;

    sample_loc_t from_;
    sample_loc_t to_;
    // We don't really use cycles/predicated/in_tx/aborted. But we parse them.
    uint32_t cycles_;
    uint8_t  predicted_ : 2;
    uint8_t  in_tx_     : 1;
    uint8_t  aborted_   : 1;
    // Used to see if we want to track this (i.e we don't want to add `ret`
    // (0xc3) edges to the CFG.
    system::br_insn_t br_insn_;
    constexpr bool
    is_trackable_call() const {
        return br_insn_.good() && br_insn_.is_trackable_call();
    }

    constexpr bool
    valid() const {
        return from_.valid() && to_.valid() && br_insn_.valid();
    }
};

// Header of any sample line.
struct sample_hdr_t {
    uint32_t pid_;
    uint32_t tid_;
    // Currently unused.
    uint64_t timestamp_;
    // comm_ is not actually allocated. Its just a reference to the parsed line,
    // so it must be consumed BEFORE parsing the next line.
    small_str_t<char const *> comm_;

    

    constexpr uint64_t
    tpid() const {
        return (static_cast<uint64_t>(pid_) << 32U) | tid_;
    }

    constexpr bool
    valid() const {
        return true;
    }
};


// Sample of mmap events. Gives us mapping info.
struct sample_mmap_t {
    uint64_t map_base_;
    uint64_t map_size_;
    uint64_t map_off_;

    uint32_t pid_;
    uint32_t tid_;

    uint8_t                   read_   : 1;
    uint8_t                   write_  : 1;
    uint8_t                   exec_   : 1;
    uint8_t                   shared_ : 1;
    uint8_t                   priv_   : 1;
    small_str_t<char const *> dso_;

    bool
    is_executable() const {
        return read_ && exec_;
    }

    constexpr uint64_t
    tpid() const {
        return (static_cast<uint64_t>(pid_) << 32U) | tid_;
    }

    constexpr bool
    valid() const {
        return true;
    }
};

// Sample of comm event (changing cmdline). At the moment unused.
struct sample_comm_t {
    uint32_t pid_;
    uint32_t tid_;

    bool                      exec_;
    small_str_t<char const *> comm_;

    constexpr uint64_t
    tpid() const {
        return (static_cast<uint64_t>(pid_) << 32U) | tid_;
    }


    constexpr bool
    valid() const {
        return true;
    }
};

// Sample of fork event. We copy parents current mapping to child.
struct sample_fork_t {
    uint32_t ppid_;
    uint32_t ptid_;
    uint32_t cpid_;
    uint32_t ctid_;

    constexpr uint64_t
    parent_tpid() const {
        return (static_cast<uint64_t>(ppid_) << 32U) | ptid_;
    }

    constexpr uint64_t
    child_tpid() const {
        return (static_cast<uint64_t>(cpid_) << 32U) | ctid_;
    }

    constexpr bool
    valid() const {
        return true;
    }
};


struct sample_unused_t {
    constexpr bool
    valid() const {
        return false;
    }
};

// "union" of mmap/fork/comm sample.
struct info_sample_t {
    sample_hdr_t hdr_;

    std::variant<sample_unused_t, sample_mmap_t, sample_fork_t, sample_comm_t>
        info_;

    void
    reset() {
        info_ = sample_unused_t{};
    }

    sample_mmap_t *
    get_mmap() {
        return std::get_if<1>(&info_);
    }

    sample_fork_t *
    get_fork() {
        return std::get_if<2>(&info_);
    }

    sample_comm_t *
    get_comm() {
        return std::get_if<3>(&info_);
    }

    void
    use_mmap() {
        info_ = sample_mmap_t{};
    }

    void
    use_fork() {
        info_ = sample_fork_t{};
    }

    void
    use_comm() {
        info_ = sample_comm_t{};
    }

    const sample_mmap_t *
    get_mmap() const {
        return std::get_if<1>(&info_);
    }

    const sample_fork_t *
    get_fork() const {
        return std::get_if<2>(&info_);
    }

    const sample_comm_t *
    get_comm() const {
        return std::get_if<3>(&info_);
    }

    constexpr bool
    is_mmap() const {
        return std::holds_alternative<sample_mmap_t>(info_);
    }

    constexpr bool
    is_fork() const {
        return std::holds_alternative<sample_fork_t>(info_);
    }

    constexpr bool
    is_comm() const {
        return std::holds_alternative<sample_comm_t>(info_);
    }


    constexpr bool
    active() const {
        return !std::holds_alternative<sample_unused_t>(info_);
    }

    constexpr bool
    valid() const {
        return hdr_.valid() &&
               std::visit([](auto & r) noexcept { return r.valid(); }, info_);
    }
};

// Not much to a simple sample, just a location, timestamp, and tid/pid.
struct simple_sample_t {
    sample_hdr_t hdr_;
    sample_loc_t loc_;

    constexpr uint64_t
    tpid() const {
        return hdr_.tpid();
    }

    constexpr bool
    valid() const {
        return loc_.valid() && hdr_.valid();
    }
};

// 32 branch samples per lbr sample.
// TODO: Make 32 variables.
struct lbr_sample_t : simple_sample_t {
    static constexpr uint32_t k_max_lbr_samples = 32;
    uint32_t                  num_samples_;
    lbr_br_sample_t           samples_[k_max_lbr_samples];

    constexpr uint32_t
    num_lbr_samples() const {
        return num_samples_;
    }

    constexpr bool
    valid() const {
        if (num_samples_ == 0 || num_samples_ > k_max_lbr_samples) {
            return false;
        }
        for (uint32_t i = 0; i < num_lbr_samples(); ++i) {
            if (!samples_[i].valid()) {
                return false;
            }
        }
        return true;
    }
};

static_assert(has_okay_type_traits<sample_hdr_t>::value);
static_assert(has_okay_type_traits<simple_sample_t>::value);
static_assert(has_okay_type_traits<lbr_sample_t>::value);
static_assert(has_okay_type_traits<lbr_br_sample_t>::value);

}  // namespace perf
}  // namespace tlo
#endif
