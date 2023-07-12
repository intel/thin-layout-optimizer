#ifndef SRC_D_UTIL_D_TYPE_INFO_H_
#define SRC_D_UTIL_D_TYPE_INFO_H_

////////////////////////////////////////////////////////////////////////////////
// Just some wrappers for type_traits / include of type_traits.

#include <type_traits>
#include <limits>
namespace tlo {
template<typename T_t>
struct has_okay_type_traits {
    static constexpr bool k_triv_construct =
        std::is_trivially_constructible<T_t>::value;
    static constexpr bool k_triv_copy_construct =
        std::is_trivially_copy_constructible<T_t>::value;
    static constexpr bool k_triv_move_construct =
        std::is_trivially_move_constructible<T_t>::value;
    static constexpr bool k_triv_copyable =
        std::is_trivially_copyable<T_t>::value;
    static constexpr bool k_triv_destruct =
        std::is_trivially_destructible<T_t>::value;
    static_assert(k_triv_construct);
    static_assert(k_triv_copy_construct);
    static_assert(k_triv_move_construct);
    static_assert(k_triv_copyable);
    static_assert(k_triv_destruct);
    static constexpr bool value = k_triv_construct && k_triv_copy_construct &&
                                  k_triv_move_construct && k_triv_copyable &&
                                  k_triv_destruct;
};


}  // namespace tlo
#endif
