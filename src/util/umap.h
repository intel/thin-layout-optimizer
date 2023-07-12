#ifndef SRC_D_UTIL_D_UMAP_H_
#define SRC_D_UTIL_D_UMAP_H_

////////////////////////////////////////////////////////////////////////////////
// Wrapper for a reasonably good C++ hashtable.

#include "umap-raw.h"


namespace tlo {
namespace detail {
template<typename T_t>
struct basic_hasher_t {
    using is_avalanching = void;

    constexpr uint64_t
    operator()(const T_t & item) const noexcept {
        if constexpr (std::is_pointer<T_t>::value) {
            return item->hash();
        }
        else {
            return item.hash();
        }
    }
};

template<typename T_t>
struct basic_equals_t {
    constexpr bool
    operator()(const T_t & lhs, const T_t & rhs) const noexcept {
        if constexpr (std::is_pointer<T_t>::value) {
            return lhs->eq(rhs);
        }
        else {
            return lhs.eq(rhs);
        }
    }
};

}  // namespace detail
template<typename Tkey_t,
         typename Tval_t,
         typename Thasher = detail::basic_hasher_t<Tkey_t>,
         typename Tequals = detail::basic_equals_t<Tkey_t>>
using basic_umap =
    typename ankerl::unordered_dense::map<Tkey_t, Tval_t, Thasher, Tequals>;

template<typename T_t,
         typename Thasher = detail::basic_hasher_t<T_t>,
         typename Tequals = detail::basic_equals_t<T_t>>
using basic_uset = typename ankerl::unordered_dense::set<T_t, Thasher, Tequals>;

template<typename Tkey_t,
         typename Tval_t,
         typename Thasher = std::hash<Tkey_t>,
         typename Tequals = std::equal_to<Tkey_t>>
using umap =
    typename ankerl::unordered_dense::map<Tkey_t, Tval_t, Thasher, Tequals>;

template<typename T_t,
         typename Thasher = std::hash<T_t>,
         typename Tequals = std::equal_to<T_t>>
using uset = typename ankerl::unordered_dense::set<T_t, Thasher, Tequals>;


}  // namespace tlo


#endif
