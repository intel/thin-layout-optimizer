#ifndef SRC_D_UTIL_D_RANDOM_H_
#define SRC_D_UTIL_D_RANDOM_H_

#include <stdint.h>
#include <sys/random.h>

////////////////////////////////////////////////////////////////////////////////
// Wrappers around `getrandom`

namespace tlo {


static bool
randomize(uint8_t * buf, size_t bufsz) {
    return static_cast<size_t>(getrandom(buf, bufsz, 0)) == bufsz;
}

static bool
randomize_str(char * str, size_t slen) {
    if (!randomize(reinterpret_cast<uint8_t *>(str), slen)) {
        return false;
    }
    for (size_t i = 0; i < slen; ++i) {
        char base = static_cast<char>(static_cast<unsigned char>(str[i]) >> 2);
        char todo = str[i] & 3;
        switch (todo) {
            case 0:
                base = static_cast<char>('0' + (base % 10));
                break;
            case 1:
                base = static_cast<char>('a' + (base % 26));
                break;
            default:
                base = static_cast<char>('A' + (base % 26));
                break;
        }
        str[i] = base;
    }
    return true;
}


}  // namespace tlo

#endif
