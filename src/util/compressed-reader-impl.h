#ifndef SRC_D_UTIL_D_COMPRESSED_READER_IMPL_H_
#define SRC_D_UTIL_D_COMPRESSED_READER_IMPL_H_

#ifndef TLO_ZSTD
# error "Compressed reader requires ZSTD"
#endif

////////////////////////////////////////////////////////////////////////////////
// Class for streaming compressed file.
// Partially implemented in reader_base_t.

#include "src/util/bits.h"
#include "src/util/memory.h"
#include "src/util/reader.h"
#include "src/util/type-info.h"

#include <span>

#include "zstd.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

namespace tlo {

// Alot of the functionality is implemented in reader_base_t. This just has the
// hooks for reading in a compressed data stream.
struct creader_t : reader_base_t {
    using zstream_t = ZSTD_DStream;

    // Buffer for storing compressed data (reader_base_t has our user-facing
    // buffers / file handle).
    uint8_t * mem_compressed_;
    size_t    pos_compressed_;
    size_t    sz_compressed_;
    size_t    cap_compressed_;

    zstream_t * zstream_;


    constexpr creader_t() = default;


    static size_t
    zstream_buf_compressed_size() {
        return roundup(ZSTD_DStreamInSize(), 4096);
    }

    static size_t
    zstream_buf_size() {
        return roundup(ZSTD_DStreamOutSize(), 4096);
    }


    static bool
    zokay(size_t res) {
        return !ZSTD_isError(res);
    }

    bool
    zinit() {
        if (!zshutdown()) {
            return false;
        }
        zstream_ = ZSTD_createDStream();
        if (zstream_ == nullptr) {
            return false;
        }
        // Max window log is 31
        ZSTD_DCtx_setParameter(zstream_, ZSTD_d_windowLogMax, 31);
        return true;
    }

    bool
    zshutdown() {
        if (zstream_ != nullptr) {
            if (!zokay(ZSTD_freeDStream(zstream_))) {
                return false;
            }
            zstream_ = nullptr;
        }
        return true;
    }


    size_t
    alloc_size() const {
        return cap_compressed_ + cap_;
    }
    uint8_t *
    alloc_start() const {
        return mem_;
    }
    void
    alloc_null() {
        mem_ = nullptr;
    }


    void
    release() {
        if (alloc_start() != nullptr) {
            sys::freemem(alloc_start(), alloc_size());
            alloc_null();
        }
    }

    void
    shutdown() {
        fshutdown();
        zshutdown();
    }

    void
    cleanup() {
        shutdown();
        release();
    }


    bool init(const char * path);

    size_t refill();
    size_t file_fill_buf();

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
