#ifndef SRC_D_UTIL_D_FILE_OPS_H_
#define SRC_D_UTIL_D_FILE_OPS_H_

//////////////////////////////////////////////////////////////////////
// Helper class for doing basic file ops (just some syscall wrappers).

#include "src/util/debug.h"
#include "src/util/memory.h"
#include "src/util/path.h"
#include "src/util/random.h"
#include "src/util/type-info.h"
#include "src/util/verbosity.h"

#include <array>
#include <cstring>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <span>

namespace tlo {
struct file_ops {
    static constexpr size_t k_err = std::numeric_limits<size_t>::max();

    static size_t
    filesize(const char * path) {
        struct stat st;
        if (stat(path, &st) || st.st_size < 0) {
            return k_err;
        }
        return static_cast<size_t>(st.st_size);
    }

    static int
    new_tmpfile(std::array<char, 256> * outbuf,
                std::string_view        prefix = "/tmp/.tmp-") {
        if (prefix.length() >= 128U) {
            return -1;
        }

        std::memcpy(outbuf->data(), prefix.data(), prefix.length());

        size_t i = prefix.length();
        for (;;) {
            if ((i + 9U) >= outbuf->size()) {
                return -1;
            }

            if (!randomize_str(outbuf->data() + i, 8U)) {
                return -1;
            }
            i += 8U;
            (*outbuf)[i] = '\0';

            if (!exists(outbuf->data())) {
                break;
            }
        }

        return open(outbuf->data(), O_RDWR | O_CREAT, 0644);
    }


    static size_t
    filesize(int fd) {
        struct stat st;
        if (fstat(fd, &st) || st.st_size < 0) {
            return k_err;
        }
        return static_cast<size_t>(st.st_size);
    }

    static bool
    is_dir(const char * path) {
        struct stat sb;
        return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
    }

    static bool
    is_dir(std::string_view path) {
        return is_dir(path.data());
    }

    static bool
    is_empty_dir(std::string_view path) {
        DIR * dir = opendir(path.data());
        if (dir == nullptr) {
            return false;
        }
        bool ret = readdir(dir) == nullptr;
        (void)closedir(dir);
        return ret;
    }


    static bool
    remove_dir(std::string_view path,
               bool             recurse     = false,
               bool             delete_self = true) {
        DIR * dir = opendir(path.data());
        if (dir == nullptr) {
            return false;
        }
        vec_t<char> buf{};
        std::copy(path.begin(), path.end(), std::back_inserter(buf));
        size_t path_end = buf.size();
        TLO_DISABLE_WREDUNDANT_TAGS
        struct dirent * entry = nullptr;
        TLO_REENABLE_WREDUNDANT_TAGS
        bool ret = true;
        for (;;) {
            entry = readdir(dir);
            if (entry == nullptr) {
                break;
            }
            if (std::strcmp(entry->d_name, ".") == 0 ||
                std::strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            buf.resize(path_end);
            std::string_view next = path_join(&buf, entry->d_name);
            switch (entry->d_type) {
                case DT_DIR:
                    TLO_printvv("Recursing: %s\n", next.data());
                    if (!recurse || !remove_dir(next, true, true)) {
                        ret = false;
                    }
                    break;
                case DT_REG:
                    TLO_printvv("Removing: %s\n", next.data());
                    if (remove(next.data()) != 0) {
                        ret = false;
                    }
                    break;
                default:
                    ret = false;
            }
            if (!ret) {
                break;
            }
        }
        (void)closedir(dir);
        return ret && (!delete_self || rmdir(path.data()) == 0);
    }
    static bool
    clear_dir(std::string_view path, bool recurse = false) {
        return remove_dir(path, recurse, false);
    }

    static bool
    make_dir(const char * path) {
        if (is_dir(path)) {
            return true;
        }
        return mkdir(path, 0755) == 0 || errno == EEXIST;
    }
    static bool
    make_dir_p(std::string_view path) {
        tlo::vec_t<char> writable_path{};
        writable_path.reserve(path.length());
        std::copy(path.begin(), path.end(), std::back_inserter(writable_path));
        writable_path.emplace_back('\0');
        char * path_start = writable_path.data();
        char * path_end   = path_start + writable_path.size();
        char * path_cur   = path_start + 1;
        for (; path_end >= path_cur;) {
            path_cur = reinterpret_cast<char *>(std::memchr(
                path_cur, '/', static_cast<size_t>(path_end - path_cur)));
            if (path_cur == nullptr) {
                break;
            }
            assert(*path_cur == '/');
            *path_cur = '\0';

            if (!make_dir(path_start)) {
                return false;
            }
            *path_cur = '/';
            ++path_cur;
        }
        return make_dir(path_start);
    }

    static bool
    exists(const char * path) {
        return access(path, F_OK) == 0;
    }


    template<bool k_off, bool k_read, typename buftype_t>
    static size_t
    ensure_io_impl(int fd, buftype_t * buf, size_t bufsz, ssize_t file_offset) {
        size_t nread = 0;
        static_assert(sizeof(buftype_t) == sizeof(uint8_t));
        for (;;) {
            ssize_t sres;
            if constexpr (k_off) {
                if constexpr (k_read) {
                    sres = pread(fd, buf, bufsz, file_offset);
                }
                else {
                    sres = pwrite(fd, buf, bufsz, file_offset);
                }
            }
            else {
                if constexpr (k_read) {
                    sres = read(fd, buf, bufsz);
                }
                else {
                    sres = write(fd, buf, bufsz);
                }
            }

            if (sres < 0) {
                if (errno == EAGAIN) {
                    continue;
                }
                std::array<char, 512> error_buf;
                (void)fprintf(
                    stderr, "IO Error [%d] -> %s\n", errno,
                    strerror_r(errno, error_buf.data(), error_buf.size()));
                return k_err;
            }

            size_t res = static_cast<size_t>(sres);
            if (res == 0) {
                return nread;
            }

            bufsz -= res;
            nread += res;
            if constexpr (k_off) {
                file_offset += sres;
            }
            if (bufsz == 0) {
                return nread;
            }
            buf += res;
        }
    }

    static size_t
    ensure_read(int fd, uint8_t * buf, size_t bufsz, ssize_t file_offset) {
        return ensure_io_impl<true, true>(fd, buf, bufsz, file_offset);
    }
    static size_t
    ensure_read(int fd, uint8_t * buf, size_t bufsz) {
        return ensure_io_impl<false, true>(fd, buf, bufsz, 0);
    }

    static size_t
    ensure_write(int             fd,
                 const uint8_t * buf,
                 size_t          bufsz,
                 ssize_t         file_offset) {
        return ensure_io_impl<true, false>(fd, buf, bufsz, file_offset);
    }

    static size_t
    ensure_write(int fd, const uint8_t * buf, size_t bufsz) {
        return ensure_io_impl<false, false>(fd, buf, bufsz, 0);
    }

    struct filebuf_t {
        std::span<const uint8_t> buf_;

        constexpr filebuf_t() = default;
        filebuf_t(const uint8_t * buf, size_t bufsz) : buf_(buf, bufsz) {}

        void
        cleanup() {
            if (active()) {
                buf_free(buf_.data(), buf_.size_bytes());
                buf_ = {};
                assert(!active());
            }
        }

        constexpr bool
        active() const {
            return buf_.data() != nullptr && buf_.size() > 0;
        }

        constexpr const uint8_t *
        data() const {
            return buf_.data();
        }

        constexpr size_t
        size() const {
            return buf_.size_bytes();
        }
    };
    static filebuf_t
    readfile(const char * path) {
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            std::array<char, 512> error_buf;
            (void)fprintf(
                stderr, "Unable to open %s\n\t%d -> %s\n", path, errno,
                strerror_r(errno, error_buf.data(), error_buf.size()));
            return filebuf_t{};
        }
        size_t fsize = filesize(path);
        if (fsize == k_err) {
            return filebuf_t{};
        }
        uint8_t * buf = reinterpret_cast<uint8_t *>(buf_alloc(fsize));

        size_t ret = ensure_read(fd, buf, fsize);
        close(fd);
        if (ret != fsize) {
            buf_free(buf, fsize);
            return filebuf_t{};
        }

        return filebuf_t{ buf, fsize };
    }

    static bool
    writefile(const char * path, filebuf_t buf, int extra_flags = 0) {
        int fd = open(path, O_RDWR | O_CREAT | extra_flags, 0644);
        if (fd < 0) {
            return false;
        }

        size_t ret = ensure_write(fd, buf.data(), buf.size());
        close(fd);
        if (ret != buf.size()) {
            return false;
        }

        return true;
    }


    static constexpr int k_map_read  = PROT_READ;
    static constexpr int k_map_write = PROT_WRITE;

    // TODO: Support compressed files here.
    class mapped_file_t {
        uint8_t * addr_;
        size_t    size_;
        bool      copied_;


       public:
        constexpr mapped_file_t(uint8_t * addr, size_t size, bool copied)
            : addr_(addr), size_(size), copied_(copied) {}

        constexpr mapped_file_t(uint8_t * addr, size_t size)
            : mapped_file_t(addr, size, false) {}


        constexpr mapped_file_t() = default;

        constexpr bool
        inbounds(size_t off, size_t rsize = 0) const {
            return (off + rsize) <= size_;
        }
        constexpr uint8_t const *
        region(size_t off, size_t rsize) const {
            assert(inbounds(off, rsize));
            return addr_ + off;
        }

        constexpr uint8_t *
        mutable_region(size_t off, size_t rsize) const {
            assert(inbounds(off, rsize));
            return addr_ + off;
        }

        constexpr bool
        active() const {
            return addr_ != nullptr;
        }

        constexpr void
        deactivate() {
            addr_ = nullptr;
        }

        static constexpr mapped_file_t
        inactive() {
            return mapped_file_t{ nullptr, 0, 0 };
        }

        std::pair<uint8_t *, size_t>
        to_pair() const {
            return { addr_, size_ };
        }

        bool
        is_copied() const {
            return copied_;
        }
    };

    static void
    unmap_file(mapped_file_t mapping) {
        auto pair = mapping.to_pair();
        if (mapping.is_copied()) {
            buf_free(reinterpret_cast<void *>(pair.first), pair.second);
        }
        else {
            munmap(reinterpret_cast<void *>(pair.first), pair.second);
        }
    }

    static mapped_file_t
    map_file(int fd, int prot, bool copy) {
#ifdef TLO_SANITIZED
        copy = true;
#endif
        size_t sz = filesize(fd);
        if (sz == k_err) {
            return mapped_file_t::inactive();
        }
        void * p =
            copy ? buf_zalloc(sz) : mmap(nullptr, sz, prot, MAP_PRIVATE, fd, 0);

        // TODO: Since we use nullptr as invalid mapping we should
        // munmap here. Although truthfully if nullptr is an okay
        // mapping other parts of this program will have already broken
        // down.
        if (p == MAP_FAILED || p == nullptr) {
            // NOLINTNEXTLINE(clang-analyzer-unix.Malloc)
            return mapped_file_t::inactive();
        }
        if (copy &&
            ensure_read(fd, reinterpret_cast<uint8_t *>(p), sz, 0) == k_err) {
            buf_free(reinterpret_cast<void *>(p), sz);
            return mapped_file_t::inactive();
        }

        return mapped_file_t{ reinterpret_cast<uint8_t *>(p), sz, copy };
    }

    static mapped_file_t
    map_file(const char * path, int prot, bool copy) {
        int open_flags = 0;
        if ((prot & k_map_read) && (prot & k_map_write)) {
            open_flags = O_RDWR;
        }
        else if (prot & k_map_read) {
            open_flags = O_RDONLY;
        }
        else if (prot & k_map_write) {
            open_flags = O_WRONLY;
        }
        else {
            return mapped_file_t::inactive();
        }
        int fd = open(path, open_flags);
        if (fd < 0) {
            return mapped_file_t::inactive();
        }
        mapped_file_t mapping = map_file(fd, prot, copy);
        close(fd);
        return mapping;
    }
};

}  // namespace tlo

#endif
