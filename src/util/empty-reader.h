#ifndef SRC_D_UTIL_D_EMPTY_READER_H_
#define SRC_D_UTIL_D_EMPTY_READER_H_

#include <assert.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Just empty reader to hold a "null" slot in the file-reader's variant.

namespace tlo {

namespace detail {
// Dummy reader (just so we can essentially have null slot in our variant).
template<int k_unused>
struct empty_reader_impl_t {
    bool TLO_NORETURN
    init(char const *) {
        assert(0 && "Should be unreachable");
    }
    bool TLO_NORETURN
    shutdown() {
        assert(0 && "Should be unreachable");
    }
    bool
    active() const {
        return false;
    }
    void
    cleanup() {
        return;
    }
    size_t TLO_NORETURN
    readline(uint8_t **, size_t *) {
        assert(0 && "Should be unreachable");
    }
    size_t TLO_NORETURN
    readn(uint8_t *, size_t) {
        assert(0 && "Should be unreachable");
    }
    size_t TLO_NORETURN
    nbytes_total() const {
        assert(0 && "Should be unreachable");
    }
    size_t TLO_NORETURN
    nbytes_read() const {
        assert(0 && "Should be unreachable");
    }
    void TLO_NORETURN
    free_line(uint8_t *, size_t) const {
        assert(0 && "Should be unreachable");
    }
};
using empty_reader_t = empty_reader_impl_t<0>;
}  // namespace detail

}  // namespace tlo
#endif
