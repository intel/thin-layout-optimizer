#ifndef SRC_D_UTIL_D_BITS_H_
#define SRC_D_UTIL_D_BITS_H_

#include "src/util/type-info.h"

#include <bit>

#include <limits.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Some helpers for doing bit manipulation. Not very interesting.

namespace tlo {
namespace detail {
template<typename T_t, typename T1_t>
constexpr uint32_t
shift_mask(T1_t amt) {
    return static_cast<uint32_t>(amt) & (CHAR_BIT * sizeof(T_t) - 1);
}
}  // namespace detail

template<typename T_t>
    requires(std::is_integral<T_t>::value)
constexpr T_t rol(T_t v, uint32_t amt) {
    return (v << detail::shift_mask<T_t>(amt)) |
           (v >> detail::shift_mask<T_t>(-amt));
}

template<typename T_t>
    requires(std::is_integral<T_t>::value)
constexpr T_t ror(T_t v, uint32_t amt) {
    return (v >> detail::shift_mask<T_t>(amt)) |
           (v << detail::shift_mask<T_t>(-amt));
}


template<typename T0_t, typename T1_t>
    requires(std::is_integral<T0_t>::value && std::is_integral<T1_t>::value)
static constexpr T0_t roundup(T0_t val, T1_t roundby) {
    T0_t rb = static_cast<T0_t>(roundby);
    return rb * ((val + rb - T0_t(1)) / rb);
}

template<typename T0_t, typename T1_t>
    requires(std::is_integral<T0_t>::value && std::is_integral<T1_t>::value)
static constexpr T0_t rounddown(T0_t val, T1_t roundby) {
    T0_t rb = static_cast<T0_t>(roundby);
    return rb * (val / rb);
}


template<typename T_t>
    requires(std::is_integral<T_t>::value)
static constexpr T_t clz(T_t bits) {
    using u_t = typename std::make_unsigned_t<T_t>;
    return static_cast<T_t>(std::countl_zero(static_cast<u_t>(bits)));
}

template<typename T_t>
    requires(std::is_integral<T_t>::value)
static constexpr T_t log2_roundup(T_t bits) {
    return static_cast<T_t>(sizeof(T_t) * CHAR_BIT) - clz<T_t>(bits);
}


template<typename T_t>
    requires(std::is_integral<T_t>::value)
static constexpr T_t next_p2(T_t bits) {
    return bits <= T_t(2) ? bits
                          : (T_t(1) << detail::shift_mask<T_t>(
                                 log2_roundup<T_t>(bits - 1)));
}


template<typename T_t>
    requires(std::is_integral<T_t>::value)
static constexpr bool is_pow2(T_t bits) {
    return (bits & (bits - 1)) == 0;
}


template<typename T_t, uintptr_t k_alignment>
    requires(std::is_pointer<T_t>::value && is_pow2(k_alignment) &&
             (k_alignment >= 1))
static constexpr T_t alignup(T_t p) {
    uintptr_t pi = reinterpret_cast<uintptr_t>(p);
    pi += (k_alignment - 1);
    pi &= (-k_alignment);
    return reinterpret_cast<T_t>(pi);
}


}  // namespace tlo
#endif
