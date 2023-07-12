#ifndef SRC_D_UTIL_D_XXHASH_H_
#define SRC_D_UTIL_D_XXHASH_H_

////////////////////////////////////////////////////////////////////////////////
// Wrapper for reasonable hash function (xxhash).

#define XXH_INLINE_ALL
#include "xxhash-raw.h"


#include <type_traits>
namespace tlo {
namespace xxhash {

#if TLO_USING_CLANG
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

template<typename T_t,
         typename = typename std::enable_if<std::is_pointer<T_t>::value>::type>
static uint64_t
run(const T_t v, size_t sz) noexcept {
    return XXH3_64bits(v, sz);
}

static uint64_t
empty() {
    return XXH3_64bits(nullptr, 0);
}

template<typename T_t,
         typename = typename std::enable_if<!std::is_pointer<T_t>::value>::type>
static constexpr uint64_t
run(T_t const & v) noexcept {
    // gcc 13+ has some bugs with O3 when optimizing xxhash.
#if TLO_USING_GCC >= TLO_VERNUM(13, 0, 0)
    __asm__ volatile("" : : : "memory");
#endif
    return XXH3_64bits(&v, sizeof(v));
}


#if TLO_USING_CLANG
# pragma clang diagnostic pop
#endif

}  // namespace xxhash
template<typename T_t>
struct xxhasher_t {
    using is_avalanching = void;

    static_assert(!std::is_pointer<T_t>::value);
    constexpr uint64_t
    operator()(T_t const & v) const noexcept {
        return xxhash::run(v);
    }
};

}  // namespace tlo


#endif
