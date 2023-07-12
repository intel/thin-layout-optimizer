#ifndef SRC_D_UTIL_D_MACRO_H_
#define SRC_D_UTIL_D_MACRO_H_

////////////////////////////////////////////////////////////////////////////////
// House an general use macros here. Not very interesting.

#define TLO_CAT_IMPL_(x, y) x##y
#define TLO_CAT(x, y)       TLO_CAT_IMPL_(x, y)

#define TLO_STRINGIFY_IMPL_(...) #__VA_ARGS__
#define TLO_STRINGIFY(...)       TLO_STRINGIFY_IMPL_(__VA_ARGS__)

#endif
