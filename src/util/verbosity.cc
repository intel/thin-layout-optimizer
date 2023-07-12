#include "verbosity.h"

namespace tlo {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int g_verbosity = 0;
void
set_verbosity(int lvl) {
    g_verbosity = lvl;
}

}  // namespace tlo
