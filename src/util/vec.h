#ifndef SRC_D_UTIL_D_VEC_H_
#define SRC_D_UTIL_D_VEC_H_

////////////////////////////////////////////////////////////////////////////////
// Wrapper for std::vec

#include <vector>

#include <stddef.h>

namespace tlo {
template<typename T_t, typename T_allocator = std::allocator<T_t> >
using vec_t = std::vector<T_t, T_allocator>;

// TODO: Implement me.
template<typename T_t,
         size_t Tk_n          = 0,
         typename T_allocator = std::allocator<T_t> >
using small_vec_t = std::vector<T_t, T_allocator>;

}  // namespace tlo

#endif
