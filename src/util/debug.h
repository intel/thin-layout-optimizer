////////////////////////////////////////////////////////////////////////////////
// Just some debugging support. Can be arbitrarily included.
// define `TLO_DEBUG` before including it to enable all macros.

// Need to manually undef all macros as we can't use header guard (otherwise
// `TLO_DEBUG` won't work per-include).
#undef TLO_TRACE
#undef TLO_TRACE_IMPL_
#undef TLO_DEBUG_ONLY
#undef TLO_STDOUT
#undef TLO_DEBUG_ENABLED
#undef TLO_DEBUG_SILENT
#include "src/util/path.h"

#include <stdio.h>

#include <string_view>

#if (defined TLO_DEBUG) || (defined TLO_DEBUG_SILENT)
# ifndef TLO_STDOUT
#  define TLO_STDOUT stderr
# endif

# define TLO_DEBUG_ENABLED
# define TLO_TRACE_IMPL_(fmt, ...)                                             \
  (void)fprintf(stderr,                                                        \
                "%-17s:%4u: ", tlo::path_get_filename(__FILE__).data(),        \
                static_cast<unsigned>(__LINE__));                              \
  (void)fprintf(stderr, fmt, ##__VA_ARGS__);                                   \
  if (!std::string_view(fmt).ends_with("\n")) {                                \
   (void)fprintf(stderr, "\n");                                                \
  }

# ifdef TLO_DEBUG
#  define TLO_TRACE(...) TLO_TRACE_IMPL_("" __VA_ARGS__)
# else
#  ifdef IN_ELF
#   error "QQ2"
#  endif
#  define TLO_TRACE(...)
# endif

# define TLO_DEBUG_ONLY(...) __VA_ARGS__

#else
# ifndef TLO_STDOUT
#  define TLO_STDOUT stdout
# endif
# define TLO_TRACE(...)
# define TLO_DEBUG_ONLY(...)
#endif

#undef TLO_DEBUG
