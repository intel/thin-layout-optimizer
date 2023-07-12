#ifndef SRC_D_SYM_D_ADDR_RANGE_H_
#define SRC_D_SYM_D_ADDR_RANGE_H_

////////////////////////////////////////////////////////////////////////////////
// Address range. Each symbol is assosiated with one of these. Its used for
// matching event address -> symbol and a variety of other things. An addr_range
// can be in two states. A single value which represents a point (i.e the
// location of an event) or a proper range with an inclusive low bound and
// exclusive upper bound.

#include "src/util/algo.h"
#include "src/util/debug.h"
#include "src/util/type-info.h"

#include <stdint.h>

namespace tlo {
namespace sym {
struct addr_range_t {
    uint64_t lo_addr_inclusive_;
    uint64_t hi_addr_exclusive_;

    constexpr explicit addr_range_t() = default;
    constexpr explicit addr_range_t(uint64_t addr) noexcept
        : lo_addr_inclusive_(addr), hi_addr_exclusive_(0) {}

    constexpr addr_range_t(uint64_t lo_addr_inclusive,
                           uint64_t hi_addr_exclusive) noexcept
        : lo_addr_inclusive_(lo_addr_inclusive),
          hi_addr_exclusive_(hi_addr_exclusive) {}

    template<typename T0_t, typename T1_t>
        requires(std::is_integral<T0_t>::value &&
                 std::is_integral<T1_t>::value &&
                 std::is_unsigned<T0_t>::value && std::is_unsigned<T1_t>::value)
    constexpr addr_range_t(std::pair<T0_t, T1_t> p)
        : lo_addr_inclusive_(p.first), hi_addr_exclusive_(p.second) {}


    constexpr uint64_t
    hi_addr_inclusive() const {
        TLO_DEBUG_ONLY(assert(valid()));
        return single() ? lo_addr_inclusive_ : (hi_addr_exclusive_ - 1);
    }

    constexpr void
    add_addr(uint64_t addr) {

        if (!active()) {
            lo_addr_inclusive_ = addr;
            hi_addr_exclusive_ = 0;
        }
        else if (addr) {
            lo_addr_inclusive_ = std::min(addr, lo_addr_inclusive_);
            hi_addr_exclusive_ = std::max(hi_addr_exclusive_, addr + 1);
        }
    }

    constexpr bool
    contains(uint64_t addr) const {
        TLO_DEBUG_ONLY(assert(valid()));
        return addr >= lo_addr_inclusive_ && addr < hi_addr_exclusive_;
    }

    constexpr bool
    contains(const addr_range_t other) const {
        TLO_DEBUG_ONLY(assert(valid() && other.valid()));
        return contains(other.lo_addr_inclusive_) &&
               (other.single() || contains(other.hi_addr_inclusive()));
    }

    static constexpr addr_range_t
    merged(const addr_range_t lhs, const addr_range_t rhs) {
        uint64_t new_lo =
            std::min(lhs.lo_addr_inclusive_, rhs.lo_addr_inclusive_);
        uint64_t new_hi =
            std::max(lhs.hi_addr_exclusive_, rhs.hi_addr_exclusive_);

        if (new_lo == 0) {
            new_lo = lhs.lo_addr_inclusive_ + rhs.lo_addr_inclusive_;
        }
        return { new_lo, new_hi };
    }
    constexpr void
    merge(const addr_range_t other) {
        *this = merged(*this, other);
    }


    constexpr bool
    overlaps_with(const addr_range_t other) const {
        TLO_DEBUG_ONLY(assert(valid() && other.valid()));
        return contains(other.lo_addr_inclusive_) ||
               contains(other.hi_addr_inclusive());
    }

    constexpr bool
    fully_overlaps_with(const addr_range_t other) const {
        TLO_DEBUG_ONLY(assert(valid() && other.valid()));
        return contains(other);
    }

    constexpr bool
    partially_overlaps_with(const addr_range_t other) const {
        TLO_DEBUG_ONLY(assert(valid() && other.valid()));
        return (contains(other.lo_addr_inclusive_) !=
                contains(other.hi_addr_inclusive()));
    }

    static constexpr bool
    overlap(const addr_range_t lhs, const addr_range_t rhs) {
        TLO_DEBUG_ONLY(assert(lhs.valid() && rhs.valid()));
        return lhs.overlaps_with(rhs) || rhs.overlaps_with(lhs);
    }

    static constexpr bool
    fully_overlap(const addr_range_t lhs, const addr_range_t rhs) {
        TLO_DEBUG_ONLY(assert(lhs.valid() && rhs.valid()));
        return lhs.fully_overlaps_with(rhs) || rhs.fully_overlaps_with(lhs);
    }

    static constexpr bool
    partially_overlap(const addr_range_t lhs, const addr_range_t rhs) {
        TLO_DEBUG_ONLY(assert(lhs.valid() && rhs.valid()));
        return lhs.partially_overlaps_with(rhs) &&
               rhs.partially_overlaps_with(lhs);
    }


    constexpr uint64_t
    size() const {
        TLO_DEBUG_ONLY(assert(valid()));
        return single() ? 0u : (hi_addr_exclusive_ - lo_addr_inclusive_);
    }

    constexpr bool
    lt(const addr_range_t other) const {
        TLO_DEBUG_ONLY(assert(valid() && other.valid()));
        if (other.single()) {
            return lt(other.lo_addr_inclusive_);
        }
        if (lt(other.lo_addr_inclusive_)) {
            return true;
        }
        if (overlap(*this, other)) {
            if (lo_addr_inclusive_ < other.lo_addr_inclusive_) {
                return true;
            }
            if (lo_addr_inclusive_ == other.lo_addr_inclusive_) {
                return hi_addr_exclusive_ < other.hi_addr_exclusive_;
            }
        }
        return false;
    }

    constexpr bool
    lt(uint64_t addr) const {
        if (single()) {
            return lo_addr_inclusive_ < addr;
        }
        return hi_addr_inclusive() < addr;
    }

    constexpr bool
    eq(const addr_range_t other) const {
        TLO_DEBUG_ONLY(assert(valid() && other.valid()));
        return lo_addr_inclusive_ == other.lo_addr_inclusive_ &&
               hi_addr_exclusive_ == other.hi_addr_exclusive_;
    }

    constexpr bool
    single() const {
        TLO_DEBUG_ONLY(assert(valid()));
        return hi_addr_exclusive_ == 0;
    }
    constexpr bool
    empty() const {
        return lo_addr_inclusive_ == 0 && hi_addr_exclusive_ == 0;
    }
    constexpr bool
    active() const {
        TLO_DEBUG_ONLY(assert(valid()));
        return lo_addr_inclusive_ != 0;
    }
    constexpr bool
    size_only() const {
        TLO_DEBUG_ONLY(assert(valid()));
        return lo_addr_inclusive_ == 0 && hi_addr_exclusive_ != 0;
    }

    constexpr bool
    complete() const {
        return !empty() && !single() && valid();
    }
    constexpr bool
    valid() const {
        return lo_addr_inclusive_ == 0 || hi_addr_exclusive_ == 0 ||
               hi_addr_exclusive_ > lo_addr_inclusive_;
    }

    struct cmp_t {
        template<typename T0_t, typename T1_t>
        constexpr bool
        operator()(T0_t const & lhs, T1_t const & rhs) {
            return lhs.get_addr_range().lt(rhs.get_addr_range());
        }
    };

    constexpr addr_range_t
    get_addr_range() const {
        return *this;
    }

    template<typename T_it, typename T_t>
    static constexpr T_it
    find_closest(T_it begin, T_it end, T_t range) {
        auto res = std::lower_bound(begin, end, addr_range_t{ range }, cmp_t{});
        if (res == end) {
            return end;
        }
        if (!(*res).get_addr_range().contains(range)) {
            return end;
        }
        return res;
    }

    template<typename T_it, typename T_t>
    static constexpr T_it
    find_overlapping(T_it begin, T_it end, T_t range) {
        addr_range_t arange = addr_range_t{ range };
        auto         res    = std::lower_bound(
            begin, end, addr_range_t{ arange.lo_addr_inclusive_ }, cmp_t{});
        if (res != end &&
            addr_range_t::overlap((*res).get_addr_range(), range)) {
            return res;
        }
        if (arange.single()) {
            return end;
        }
        res = std::lower_bound(
            begin, end, addr_range_t{ arange.hi_addr_inclusive() }, cmp_t{});
        if (res != end &&
            addr_range_t::overlap((*res).get_addr_range(), range)) {
            return res;
        }
        return end;
    }
};
static_assert(has_okay_type_traits<addr_range_t>::value);
}  // namespace sym
}  // namespace tlo
#endif
