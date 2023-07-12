#ifndef SRC_D_UTIL_D_ALGO_H_
#define SRC_D_UTIL_D_ALGO_H_

////////////////////////////////////////////////////////////////////////////////
// Just wrapped around std <algorithm> that disabled wstrict-overflow warning.

#include "src/util/compiler.h"

// Appears to be be bug in GCC12 with std::sort + wstrict-overflow.
TLO_DISABLE_WSTRICT_OVERFLOW
#include <algorithm>
TLO_REENABLE_WSTRICT_OVERFLOW

#endif
