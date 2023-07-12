#ifndef PERF_HFSORT_D_TESTS_D_TEST_HELPERS_D_TEST_HELP_H_
#define PERF_HFSORT_D_TESTS_D_TEST_HELPERS_D_TEST_HELP_H_

#include "src/util/macro.h"

#ifndef TLO_ZSTD
# define NO_ZSTD_DISABLED(...) TLO_CAT(DISABLED_, __VA_ARGS__)
#else
# define NO_ZSTD_DISABLED(...) __VA_ARGS__
#endif


#endif
