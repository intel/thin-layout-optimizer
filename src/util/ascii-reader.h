#ifndef SRC_D_UTIL_D_ASCII_READER_H_
#define SRC_D_UTIL_D_ASCII_READER_H_

////////////////////////////////////////////////////////////////////////////////
// Reader interface for normal ascii file.
// Partially implemented in reader_base_t.

#include "src/util/file-ops.h"
#include "src/util/memory.h"
#include "src/util/reader.h"
#include "src/util/std-reader-base.h"

#include <span>
namespace tlo {


struct areader_t : std_reader_base_t {
    static constexpr size_t k_default_mem_size = 4096;

    void
    shutdown() {
        fshutdown();
    }

    void
    cleanup() {
        shutdown();
        release();
    }

    bool
    init(const char * path) {
        if (!finit(path)) {
            return false;
        }
        return init_buffers();
    }


    size_t
    refill() {
        return refill_impl<true>();
    }


    size_t
    init_output() {
        if (remaining_ == 0) {
            return refill();
        }
        return k_cont;
    }

    size_t
    readn(uint8_t * umem, size_t usz) {
        return readn_impl(this, umem, usz);
    }

    size_t
    readline(uint8_t ** mem_ptr, size_t * sz_ptr) {
        return readline_impl(this, mem_ptr, sz_ptr);
    }
};


}  // namespace tlo
#endif
