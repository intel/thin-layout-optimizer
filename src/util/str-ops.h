#ifndef SRC_D_UTIL_D_STR_OPS_H_
#define SRC_D_UTIL_D_STR_OPS_H_

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <span>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// String utils
//
// Only implements writing a byte-array as hex string at the moment (used for
// things like dumping build id).

namespace tlo {
template<size_t k_N>
static std::string_view
to_hex_string(std::span<const uint8_t> bytes_in,
              std::array<char, k_N> *  str_out) {
    if (str_out->size() < (bytes_in.size() * 2U + 1U)) {
        return { "", 0 };
    }
    static char hex_to_char[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    size_t      i, sz;
    sz = 0;
    for (i = 0; i < bytes_in.size(); ++i) {
        uint8_t byte   = bytes_in[i];
        (*str_out)[sz] = hex_to_char[(byte >> 4) & 0xf];
        ++sz;
        (*str_out)[sz] = hex_to_char[byte & 0xf];
        ++sz;
    }

    (*str_out)[sz] = '\0';
    return { str_out->data(), sz };
}

}  // namespace tlo

#endif
