#ifndef SRC_D_UTIL_D_JSON_H_
#define SRC_D_UTIL_D_JSON_H_


////////////////////////////////////////////////////////////////////////////////
// Wrapper around: https://github.com/nlohmann/json

#include "src/util/compiler.h"

TLO_DISABLE_WCOVERED_SWITCH_DEFAULT
TLO_DISABLE_WWEAK_VTABLES
TLO_DISABLE_WSTRICT_OVERFLOW
TLO_DISABLE_WSUGGEST_FINAL_METHODS
TLO_DISABLE_WSUGGEST_FINAL_TYPES
#define JSON_NOEXCEPTION
#include "src/util/json-raw.h"
TLO_REENABLE_WSUGGEST_FINAL_TYPES
TLO_REENABLE_WSUGGEST_FINAL_METHODS
TLO_REENABLE_WSTRICT_OVERFLOW
TLO_REENABLE_WWEAK_VTABLES
TLO_REENABLE_WCOVERED_SWITCH_DEFAULT

namespace tlo {
using json_t = nlohmann::json;
}

#endif
