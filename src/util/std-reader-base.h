#ifndef SRC_D_UTIL_D_STD_READER_BASE_H_
#define SRC_D_UTIL_D_STD_READER_BASE_H_


#include "src/util/file-ops.h"
#include "src/util/memory.h"
#include "src/util/reader.h"

#include <span>
namespace tlo {

////////////////////////////////////////////////////////////////////////////////
// Some common helpers used by file-readers


struct std_reader_base_t : reader_base_t {
    static constexpr size_t k_default_mem_size = 4096;


    void
    release() {
        if (mem_ != nullptr) {
            sys::freemem(mem_, cap_);
            mem_ = nullptr;
        }
    }

    bool
    init_buffers() {
        if (mem_ == nullptr) {
            cap_ = k_default_mem_size;
            mem_ = reinterpret_cast<uint8_t *>(sys::getmem(k_default_mem_size));
        }
        assert(cap_ == k_default_mem_size);
        remaining_ = 0;
        cur_       = mem_ + cap_;
        return true;
    }

    std::span<uint8_t>
    peek() const {
        return { cur_, remaining_ };
    }

    template<bool k_pread>
    size_t
    refill_impl() {
        assert(remaining_ == 0);
        assert(cur_ >= mem_);
        assert(cur_ <= (mem_ + cap_));

        if (cur_ != (mem_ + cap_)) {
            return k_done;
        }

        size_t res;
        if constexpr (k_pread) {
            res = file_ops::ensure_read(fd_, mem_, cap_, fd_off_);
        }
        else {
            res = file_ops::ensure_read(fd_, mem_, cap_);
        }

        if (res == file_ops::k_err) {
            return k_err;
        }
        assert(res <= cap_);
        remaining_ = res;
        cur_       = mem_;
        fd_off_ += static_cast<ssize_t>(res);
        return res ? k_cont : k_done;
    }
};


}  // namespace tlo
#endif
