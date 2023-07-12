#ifndef SRC_D_UTIL_D_READER_H_
#define SRC_D_UTIL_D_READER_H_

#include "src/util/bits.h"
#include "src/util/file-ops.h"

#include <span>
#include <tuple>

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "src/util/debug.h"


////////////////////////////////////////////////////////////////////////////////
// General reader "interface"
// Should never be used directly, always use either creader_t or areader_t

namespace tlo {
// TODO: Wrapper in detail:: and pull out return enum.
struct reader_base_t {
    // Different return statuses common to all readers.

    // k_cont must be zero.
    static constexpr size_t k_cont = 0;
    static constexpr size_t k_done = std::numeric_limits<size_t>::max() - 1;
    // k_err must be greater than systems object size.
    static constexpr size_t k_err = std::numeric_limits<size_t>::max();

    // How much we allocate at a time for reading lines (not for each line, but
    // by this amount each time we need to realloc).
    static constexpr size_t k_default_ln_sz = 512;

    struct copy_ret_t {
        bool   cont_;
        size_t copied_;
    };


    uint8_t * mem_;
    uint8_t * cur_;
    size_t    remaining_;
    size_t    cap_;
    ssize_t   fd_off_;
    int       fd_;

    constexpr reader_base_t() = default;

    // Does the reader have an active fd.
    bool
    active() const {
        return fd_ > 0;
    }

    // Close the file.
    void
    fshutdown() {
        if (fd_ > 0) {
            close(fd_);
        }
        fd_ = 0;
    }

    size_t
    nbytes_read() const {
        return static_cast<size_t>(fd_off_);
    }

    size_t
    nbytes_total() const {
        return file_ops::filesize(fd_);
    }

    // Initialize the file.
    bool
    finit(const char * path) {
        fshutdown();
        fd_ = open(path, O_RDONLY);
        if (fd_ <= 0) {
            fshutdown();
            return false;
        }
        fd_off_ = 0;
        return true;
    }

    // Copy `oremaining` bytes for `mem_` to `omem`.
    copy_ret_t
    copy_buf(uint8_t * omem, size_t oremaining) {
        assert(remaining_);
        assert(cur_ >= mem_);
        assert(cur_ < mem_ + cap_);
        assert((cur_ + remaining_) <= (mem_ + cap_));


        const uint8_t * rmem       = cur_;
        size_t          rremaining = remaining_;

        if (oremaining <= rremaining) {
            cur_ += oremaining;
            remaining_ = rremaining - oremaining;
            memcpy(omem, rmem, oremaining);
            // We completed the copy (don't need to continue).
            return { false, oremaining };
        }
        cur_ += rremaining;
        memcpy(omem, rmem, rremaining);
        remaining_ = 0;
        // Need to continue copying (next we will refill `mem_` buffer).
        return { true, rremaining };
    }

    std::span<uint8_t>
    peek() const {
        return { cur_, remaining_ };
    }


    // Helper used by the areader_t / creader_t. Reads `usz` bytes into `umem`.
    template<typename T_reader_impl_t>
        requires(std::is_base_of_v<reader_base_t, T_reader_impl_t>)
    static size_t
        readn_impl(T_reader_impl_t * reader, uint8_t * umem, size_t usz) {
        size_t total_read = 0;
        size_t res        = reader->init_output();
        if (res) {
            return res == k_err ? k_err : 0;
        }
        for (;;) {
            // Copy from current buffer.
            copy_ret_t r = reader->copy_buf(umem, usz);
            total_read += r.copied_;
            if (!r.cont_) {
                return total_read;
            }

            // Check incomplete copy state is correct
            assert(reader->remaining_ == 0);
            assert(reader->cur_ <= (reader->mem_ + reader->cap_));

            // Validate return
            assert(usz > r.copied_);

            umem += r.copied_;
            usz -= r.copied_;

            // Need to fetch more (i.e from file/decompression stream).
            res = reader->refill();
            if (res) {
                return res == k_err ? k_err : total_read;
            }
        }
    }

    // Helper for reallocating user buffer in readline.
    static std::tuple<uint8_t *, size_t>
    maybe_realloc_mem(uint8_t * mem, size_t off, size_t sz, size_t desired) {
        assert(sz >= off);
        size_t avail = sz - off;
        if (desired > avail) {
            size_t req     = desired - avail;
            size_t next_sz = roundup(sz + req, k_default_ln_sz);

            mem = reinterpret_cast<uint8_t *>(buf_realloc(mem, sz, next_sz));
            sz  = next_sz;
        }
        return { mem, sz };
    }

    static void
    free_alloced_line(uint8_t * mem, size_t sz) {
        buf_free(mem, sz);
    }

    // Can be called by user of readline to free the allocated pointer.
    void
    free_line(uint8_t * mem, size_t sz) const {
        free_alloced_line(mem, sz);
    }

    // readline, basically behaves the exact same as getline. Return `k_err` on
    // error or number of bytes read.
    // Will allocate enough bytes to read the line and update *mem_ptr / *sz_ptr
    // accordingly. *mem_ptr should be deallocated using the above free_line
    // method.
    template<typename T_reader_impl_t>
        requires(std::is_base_of_v<reader_base_t, T_reader_impl_t>)
    static size_t readline_impl(T_reader_impl_t * reader,
                                uint8_t **        mem_ptr,
                                size_t *          sz_ptr) {
        // Invalid arguments.
        if (mem_ptr == nullptr || sz_ptr == nullptr) {
            return k_err;
        }

        // Fetch initial data.
        size_t res = reader->init_output();
        if (res) {
            return res == k_err ? k_err : 0;
        }
        uint8_t * mem = *mem_ptr;
        size_t    off = 0;
        size_t    sz  = *sz_ptr;
        if (mem == nullptr) {
            mem = reinterpret_cast<uint8_t *>(buf_alloc(k_default_ln_sz));
            sz  = k_default_ln_sz;
        }
        for (;;) {
            // Loop till we are done.
            uint8_t * cur       = reader->cur_;
            size_t    remaining = reader->remaining_;
            assert(remaining);
            // See if current buffer has a new line.
            uint8_t * newline_pos = reinterpret_cast<uint8_t *>(
                memchr(reinterpret_cast<void *>(cur), '\n', remaining));
            if (newline_pos == nullptr) {
                // Nope, so copy rest of mem_ buffer then refill.
                size_t to_read = remaining;
                assert(to_read);
                std::tie(mem, sz) =
                    maybe_realloc_mem(mem, off, sz, to_read + 1);
                copy_ret_t r = reader->copy_buf(mem + off, to_read);
                assert(reader->remaining_ == 0);
                assert(reader->cur_ <= (reader->mem_ + reader->cap_));
                assert(!r.cont_);
                assert(r.copied_ == to_read);

                off += r.copied_;
                // Fetch more bytes to mem_ buffer.
                res = reader->refill();
                if (res) {
                    break;
                }
            }
            else {
                // We have a newline in our current buffer, so copy up to it
                // (including the newline char) then return.
                assert(newline_pos >= cur);
                size_t to_read = static_cast<size_t>(newline_pos - cur) + 1;
                assert(remaining >= to_read);
                std::tie(mem, sz) = maybe_realloc_mem(mem, off, sz, to_read);
                copy_ret_t r      = reader->copy_buf(mem + off, to_read);
                assert(!r.cont_);
                assert(r.copied_ == to_read);
                assert(reader->remaining_ == (remaining - to_read));
                off += r.copied_;
                break;
            }
        }
        // Store possibly allocated pointer + new size.
        *mem_ptr = mem;
        *sz_ptr  = sz;
        return res == k_err ? k_err : off;
    }
};

}  // namespace tlo


#endif
