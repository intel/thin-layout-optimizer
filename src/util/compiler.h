#ifndef SRC_D_UTIL_D_COMPILER_H_
#define SRC_D_UTIL_D_COMPILER_H_

////////////////////////////////////////////////////////////////////////////////
// Macros that at some pointer should be made target independent (maybe just
// grab hedley).

#include "src/util/macro.h"

#define TLO_ASSUME(cond)                                                       \
 if (!(cond)) {                                                                \
  __builtin_unreachable();                                                     \
 }

#define TLO_PURE         __attribute__((pure))
#define TLO_NORETURN     __attribute__((noreturn))
#define TLO_MAY_ALIAS    __attribute__((may_alias))
#define TLO_FORMATF(...) __attribute__((format(__VA_ARGS__)))

#define TLO_LIKELY(...)   __builtin_expect(__VA_ARGS__, 1)
#define TLO_UNLIKELY(...) __builtin_expect(__VA_ARGS__, 0)


#define TLO_VERNUM(major, minor, patch)                                        \
 ((major)*100 * 100 + (minor)*100 + (patch))

#ifdef __clang__
# define TLO_USING_CLANG                                                       \
  TLO_VERNUM(__clang_major__, __clang_minor__, __clang_patchlevel__)
# define TLO_USING_GCC 0

# define TLO_DISABLE_WARNING(warning)                                          \
  _Pragma("clang diagnostic push")                                             \
      _Pragma(TLO_STRINGIFY(clang diagnostic ignored warning))

# define TLO_REENABLE_WARNING _Pragma("clang diagnostic pop")


#elif defined __GNUC__
# define TLO_USING_GCC   TLO_VERNUM(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
# define TLO_USING_CLANG 0

# define TLO_DISABLE_WARNING(warning)                                          \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma(TLO_STRINGIFY(GCC diagnostic ignored warning))
# define TLO_REENABLE_WARNING _Pragma("GCC diagnostic pop")


#else
# error "Unknown compiler"
#endif

#if TLO_USING_GCC >= TLO_VERNUM(4, 4, 7)

# define TLO_DISABLE_WSTACK_PROTECTOR  TLO_DISABLE_WARNING("-Wstack-protector")
# define TLO_REENABLE_WSTACK_PROTECTOR TLO_REENABLE_WARNING

# define TLO_DISABLE_WTYPE_LIMITS      TLO_DISABLE_WARNING("-Wtype-limits")
# define TLO_REENABLE_WTYPE_LIMITS     TLO_REENABLE_WARNING
# define TLO_DISABLE_WSTRICT_OVERFLOW  TLO_DISABLE_WARNING("-Wstrict-overflow")
# define TLO_REENABLE_WSTRICT_OVERFLOW TLO_REENABLE_WARNING
# define TLO_DISABLE_WMISSING_BRACES   TLO_DISABLE_WARNING("-Wmissing-braces")
# define TLO_REENABLE_WMISSING_BRACES  TLO_REENABLE_WARNING
#else
# define TLO_DISABLE_WSTACK_PROTECTOR
# define TLO_REENABLE_WSTACK_PROTECTOR
# define TLO_DISABLE_WTYPE_LIMITS
# define TLO_REENABLE_WTYPE_LIMITS
# define TLO_DISABLE_WSTRICT_OVERFLOW
# define TLO_REENABLE_WSTRICT_OVERFLOW
# define TLO_DISABLE_WMISSING_BRACES
# define TLO_REENABLE_WMISSING_BRACES
#endif

#if TLO_USING_GCC >= TLO_VERNUM(5, 1, 0)
# define TLO_DISABLE_WSUGGEST_FINAL_METHODS                                    \
  TLO_DISABLE_WARNING("-Wsuggest-final-methods")
# define TLO_REENABLE_WSUGGEST_FINAL_METHODS TLO_REENABLE_WARNING
# define TLO_DISABLE_WSUGGEST_FINAL_TYPES                                      \
  TLO_DISABLE_WARNING("-Wsuggest-final-types")
# define TLO_REENABLE_WSUGGEST_FINAL_TYPES TLO_REENABLE_WARNING

#else
# define TLO_DISABLE_WSUGGEST_FINAL_METHODS
# define TLO_REENABLE_WSUGGEST_FINAL_METHODS
# define TLO_DISABLE_WSUGGEST_FINAL_TYPES
# define TLO_REENABLE_WSUGGEST_FINAL_TYPES
#endif

#if TLO_USING_GCC >= TLO_VERNUM(6, 1, 0)
# define TLO_DISABLE_WNULL_DEREFERENCE  TLO_DISABLE_WARNING("-Wnull-dereference")
# define TLO_REENABLE_WNULL_DEREFERENCE TLO_REENABLE_WARNING
#else
# define TLO_DISABLE_WNULL_DEREFERENCE
# define TLO_REENABLE_WNULL_DEREFERENCE
#endif

#if TLO_USING_GCC >= TLO_VERNUM(7, 1, 0)
# define TLO_DISABLE_WALLOC_ZERO  TLO_DISABLE_WARNING("-Walloc-zero")
# define TLO_REENABLE_WALLOC_ZERO TLO_REENABLE_WARNING
#else
# define TLO_DISABLE_WALLOC_ZERO
# define TLO_REENABLE_WALLOC_ZERO
#endif

#if TLO_USING_GCC >= TLO_VERNUM(8, 1, 0)
# define TLO_DISABLE_WCAST_ALIGN  TLO_DISABLE_WARNING("-Wcast-align")
# define TLO_REENABLE_WCAST_ALIGN TLO_REENABLE_WARNING
#else
# define TLO_DISABLE_WCAST_ALIGN
# define TLO_REENABLE_WCAST_ALIGN
#endif


#if TLO_USING_GCC >= TLO_VERNUM(10, 1, 0)
# define TLO_DISABLE_WREDUNDANT_TAGS    TLO_DISABLE_WARNING("-Wredundant-tags")
# define TLO_REENABLE_WREDUNDANT_TAGS   TLO_REENABLE_WARNING
# define TLO_DISABLE_WARITH_CONVERSION  TLO_DISABLE_WARNING("-Warith-conversion")
# define TLO_REENABLE_WARITH_CONVERSION TLO_REENABLE_WARNING

#else
# define TLO_DISABLE_WREDUNDANT_TAGS
# define TLO_REENABLE_WREDUNDANT_TAGS
# define TLO_DISABLE_WARITH_CONVERSION
# define TLO_REENABLE_WARITH_CONVERSION

#endif

#if TLO_USING_CLANG >= TLO_VERNUM(4, 0, 0)
# define TLO_DISABLE_WC99_EXTENSIONS  TLO_DISABLE_WARNING("-Wc99-extensions")
# define TLO_REENABLE_WC99_EXTENSIONS TLO_REENABLE_WARNING

# define TLO_DISABLE_WCOVERED_SWITCH_DEFAULT                                   \
  TLO_DISABLE_WARNING("-Wcovered-switch-default")
# define TLO_REENABLE_WCOVERED_SWITCH_DEFAULT TLO_REENABLE_WARNING

# define TLO_DISABLE_WUNDEFINED_FUNC_TEMPLATE                                  \
  TLO_DISABLE_WARNING("-Wundefined-func-template")
# define TLO_REENABLE_WUNDEFINED_FUNC_TEMPLATE TLO_REENABLE_WARNING

# define TLO_DISABLE_WEXIT_TIME_DESTRUCTORS                                    \
  TLO_DISABLE_WARNING("-Wexit-time-destructors")
# define TLO_REENABLE_WEXIT_TIME_DESTRUCTORS TLO_REENABLE_WARNING

# define TLO_DISABLE_WWEAK_VTABLES  TLO_DISABLE_WARNING("-Wweak-vtables")
# define TLO_REENABLE_WWEAK_VTABLES TLO_REENABLE_WARNING
#else
# define TLO_DISABLE_WC99_EXTENSIONS
# define TLO_REENABLE_WC99_EXTENSIONS

# define TLO_DISABLE_WCOVERED_SWITCH_DEFAULT
# define TLO_REENABLE_WCOVERED_SWITCH_DEFAULT

# define TLO_DISABLE_WUNDEFINED_FUNC_TEMPLATE
# define TLO_REENABLE_WUNDEFINED_FUNC_TEMPLATE

# define TLO_DISABLE_WWEAK_VTABLES
# define TLO_REENABLE_WWEAK_VTABLES

# define TLO_DISABLE_WEXIT_TIME_DESTRUCTORS
# define TLO_REENABLE_WEXIT_TIME_DESTRUCTORS
#endif

#ifdef TLO_SSP_BUFFER_SIZE
# if TLO_SSP_BUFFER_SIZE >= 32
#  define TLO_DISABLE_WSTACK_PROTECTOR32  TLO_DISABLE_WSTACK_PROTECTOR
#  define TLO_REENABLE_WSTACK_PROTECTOR32 TLO_REENABLE_WSTACK_PROTECTOR
# endif
# if TLO_SSP_BUFFER_SIZE >= 16
#  define TLO_DISABLE_WSTACK_PROTECTOR16  TLO_DISABLE_WSTACK_PROTECTOR
#  define TLO_REENABLE_WSTACK_PROTECTOR16 TLO_REENABLE_WSTACK_PROTECTOR
# endif
#endif

#ifndef TLO_DISABLE_WSTACK_PROTECTOR32
# define TLO_DISABLE_WSTACK_PROTECTOR32
# define TLO_REENABLE_WSTACK_PROTECTOR32
# define TLO_DISABLE_WSTACK_PROTECTOR16
# define TLO_REENABLE_WSTACK_PROTECTOR16
#endif


#endif
