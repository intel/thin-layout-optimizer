#ifndef SRC_D_SYSTEM_D_INSN_H_
#define SRC_D_SYSTEM_D_INSN_H_
////////////////////////////////////////////////////////////////////////////////
// Just a hook for better estimating function size.

#include <stddef.h>

namespace tlo {
namespace system {
// Completely fucking artibtrary. Used to adjust estimated function size (based
// on samples) past the last known sample point.
static constexpr size_t k_average_insn_size = 4;
static constexpr size_t k_max_insn_sz       = 16;
}  // namespace system
}  // namespace tlo

#endif
