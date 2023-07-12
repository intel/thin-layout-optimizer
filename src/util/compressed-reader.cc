#include "src/util/compressed-reader-impl.h"
#include "src/util/file-ops.h"

#include "src/util/memory.h"
#include "src/util/verbosity.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef TLO_ZSTD
# error "Compressed reader requires ZSTD"
#endif

////////////////////////////////////////////////////////////////////////////////
// Reader interface for reading from a file compressed with zstd

namespace tlo {
bool
creader_t::init(const char * path) {
#if (defined TLO_MSAN) && !(defined TLO_ZSTD_MSAN)
    assert(0 && "Compressed reader is incompatible with msan");
#endif

    // Initialize (open) file.
    if (!finit(path)) {
        return false;
    }

    // Initialize out decompression stream.
    if (!zinit()) {
        TLO_perr("Unable to init zstream for: %s\n", path);
        shutdown();
        return false;
    }


    // Get size needed (desired really) for our buffers.
    const size_t cap_compressed = zstream_buf_compressed_size();
    const size_t cap_out        = zstream_buf_size();


    uint8_t * alloc;
    if (alloc_start() == nullptr ||
        (cap_compressed + cap_out) != alloc_size()) {
        // If we don't already have buffer of the right size, allocate new.
        release();
        alloc =
            reinterpret_cast<uint8_t *>(sys::getmem(cap_out + cap_compressed));
        if (alloc == nullptr) {
            TLO_perr("Unable to allocate buffer of size: %zu\n",
                     cap_compressed + cap_out);
            shutdown();
            return false;
        }
    }
    else {
        // We buffer already!
        alloc = alloc_start();
    }

    // Initialize our two buffers.

    // First buffer, stores decompressed data to be written to user.
    mem_       = alloc;
    cur_       = mem_ + cap_out;
    remaining_ = 0;
    cap_       = cap_out;

    // Second buffer, stores compressed data to be written to first buffer.
    mem_compressed_ = alloc + cap_out;
    pos_compressed_ = cap_compressed;
    sz_compressed_  = cap_compressed;
    cap_compressed_ = cap_compressed;

    return true;
}

// Fill compressed buffer with new data from the file.
size_t
creader_t::file_fill_buf() {
    // Make sure valid state.
    assert(pos_compressed_ == sz_compressed_);
    assert(sz_compressed_ == cap_compressed_);


    uint8_t *     omem       = mem_compressed_;
    const size_t  oremaining = cap_compressed_;
    const int     fd         = fd_;
    const ssize_t fd_off     = fd_off_;

    const size_t nread = file_ops::ensure_read(fd, omem, oremaining, fd_off);
    if (nread == file_ops::k_err) {
        return k_err;
    }
    pos_compressed_ = 0;
    sz_compressed_  = nread;
    fd_off_         = fd_off + static_cast<ssize_t>(nread);
    return nread == oremaining ? k_cont : k_done;
}

size_t
creader_t::refill() {
    assert(pos_compressed_ <= sz_compressed_);
    assert(sz_compressed_ <= cap_compressed_);

    size_t res;
    // Need to refill decompression buffer.
    if (pos_compressed_ == sz_compressed_) {
        // We have filled it all.
        if (sz_compressed_ != cap_compressed_) {
            return k_done;
        }

        // Get new data from file.
        res = file_fill_buf();
        if (res == k_err) {
            return k_err;
        }

        assert(pos_compressed_ == 0);
        assert(sz_compressed_ <= cap_compressed_);
    }

    // Decompress more bytes.
    ZSTD_inBuffer buf_in = { mem_compressed_, sz_compressed_, pos_compressed_ };
    ZSTD_outBuffer buf_out = { mem_, cap_, 0 };
    res                    = ZSTD_decompressStream(zstream_, &buf_out, &buf_in);
    cur_                   = mem_;
    remaining_             = buf_out.pos;
    pos_compressed_        = buf_in.pos;


    if (zokay(res)) {
        return k_cont;
    }
    return k_err;
}
}  // namespace tlo
