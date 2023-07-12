#ifndef SRC_D_UTIL_D_PROCESS_READER_H_
#define SRC_D_UTIL_D_PROCESS_READER_H_

////////////////////////////////////////////////////////////////////////////////
// Reader interface for reading stdout of an external process.
// Used when used provides raw 'perf.data' file.
// Partially implemented in reader_base_t.


#include <span>
#include "src/util/file-ops.h"
#include "src/util/memory.h"
#include "src/util/reader.h"
#include "src/util/std-reader-base.h"
namespace tlo {

struct preader_t : std_reader_base_t {
    using cmdline_t = std::array<char, 512>;

    FILE * process_fp_;

    constexpr preader_t() = default;

    void
    shutdown() {
        if (process_fp_ != nullptr) {
            pclose(process_fp_);
        }
        process_fp_ = nullptr;
    }

    void
    cleanup() {
        shutdown();
        release();
    }

    bool
    init(const char * cmdline) {
        shutdown();
        process_fp_ = popen(cmdline, "r");
        if (process_fp_ == nullptr) {
            return false;
        }
        fd_ = fileno(process_fp_);
        if (fd_ <= 0) {
            shutdown();
            return false;
        }
        fd_off_ = 0;
        return init_buffers();
    }

    size_t
    nbytes_total() const {
        return file_ops::k_err;
    }

    size_t
    refill() {
        return refill_impl<false>();
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
