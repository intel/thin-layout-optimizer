#ifndef SRC_D_UTIL_D_SMAP_H_
#define SRC_D_UTIL_D_SMAP_H_


////////////////////////////////////////////////////////////////////////////////
// Wrapper for std::map

#include <map>
#include <set>

namespace tlo {

namespace detail {
template<typename T_t>
struct basic_compares_t {
    constexpr bool
    operator()(const T_t & lhs, const T_t & rhs) const {
        if constexpr (std::is_pointer<T_t>::value) {
            return lhs->lt(rhs);
        }
        else {
            return lhs.lt(rhs);
        }
    }
};


}  // namespace detail

template<typename Tkey_t,
         typename Tval_t,
         typename Tcompare   = detail::basic_compares_t<Tkey_t>,
         typename Tallocator = std::allocator<std::pair<const Tkey_t, Tval_t>>>
using smap = std::map<Tkey_t, Tval_t, Tcompare, Tallocator>;


template<typename Tkey_t,
         typename Tcompare   = detail::basic_compares_t<Tkey_t>,
         typename Tallocator = std::allocator<Tkey_t>>
using sset = std::set<Tkey_t, Tcompare, Tallocator>;


}  // namespace tlo


#endif
