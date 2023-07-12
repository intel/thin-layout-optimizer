#ifndef SRC_D_UTIL_D_FILE_READER_H_
#define SRC_D_UTIL_D_FILE_READER_H_

////////////////////////////////////////////////////////////////////////////////
// Implementes `file_reader_t`. Helper for reading from user file.
// Can read from zst compressed file, ascii file, or from  process.
// Since we parse on line-by-line basis, most important method is `nextline`.
// Really just a manager for the actually file readers (creader_r, areader_t, or
// preader_t).
#include "src/util/ascii-reader.h"
#include "src/util/compiler.h"
#include "src/util/compressed-reader.h"
#include "src/util/empty-reader.h"
#include "src/util/memory.h"
#include "src/util/path.h"
#include "src/util/process-reader.h"
#include "src/util/type-info.h"


#include <string_view>
#include <variant>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace tlo {


struct file_reader_t {
    // Different reader type based on file being read (chosen based on file
    // extension).
    std::variant<detail::empty_reader_t, creader_t, areader_t, preader_t>
        reader_;

    uint8_t * line_;
    size_t    line_size_;

    constexpr file_reader_t() : line_(nullptr), line_size_(0) {}
    file_reader_t(const file_reader_t &) = delete;
    file_reader_t(file_reader_t &)       = delete;
    file_reader_t(file_reader_t &&)      = delete;
    ~file_reader_t() {
        cleanup_if_exists();
    }

    bool
    is_mappable() const {
        return is_ascii();
    }

    bool
    is_zst() const {
        return std::holds_alternative<creader_t>(reader_);
    }

    bool
    is_ascii() const {
        return std::holds_alternative<areader_t>(reader_);
    }

    bool
    is_process() const {
        return std::holds_alternative<preader_t>(reader_);
    }

    bool
    is_empty() const {
        return std::holds_alternative<detail::empty_reader_t>(reader_);
    }

    creader_t *
    creader() {
        return std::get_if<1>(&reader_);
    }

    areader_t *
    areader() {
        return std::get_if<2>(&reader_);
    }


    // Add new path for reading.
    bool
    init(std::string_view path) {
        if (memchr(path.data(), ' ', path.length()) != nullptr) {
            if (!is_process()) {
                cleanup_if_exists();
                reader_ = preader_t{};
            }
        }
        else {
            bool is_zstd_ext =
                path.ends_with(".zst") || path.ends_with(".zstd");
            if (!is_zstd_ext) {

                std::string_view ext = path_get_file_start_ext(path);
                is_zstd_ext = ext.starts_with("zst") || ext.starts_with("zstd");
            }
            if (is_zstd_ext) {
                if (!is_zst()) {
                    cleanup_if_exists();
                    reader_ = creader_t{};
                }
#ifndef TLO_ZSTD
                TLO_fprint_ifv(
                    0, stderr,
                    "Warning: perf-hfsort was no built with ZSTD support\n");
                return false;
#endif
            }
            else {
                if (!is_ascii()) {
                    cleanup_if_exists();
                    reader_ = areader_t{};
                }
            }
        }
        return std::visit(
            [path](auto & r) noexcept { return r.init(path.data()); }, reader_);
    }

    void
    cleanup() {
        std::visit(
            [this](auto & r) noexcept {
                if (line_ != nullptr) {
                    r.free_line(line_, line_size_);
                }
                r.cleanup();
            },
            reader_);
        line_   = nullptr;
        reader_ = detail::empty_reader_t{};
    }

    void
    cleanup_if_exists() {
        if (!is_empty()) {
            cleanup();
        }
    }

    void
    shutdown() {
        std::visit([](auto & r) noexcept { r.shutdown(); }, reader_);
    }

    bool
    active() const {
        return std::visit([](const auto & r) noexcept { return r.active(); },
                          reader_);
    }

    size_t
    nbytes_read() const {
        return std::visit(
            [](const auto & r) noexcept { return r.nbytes_read(); }, reader_);
    }

    size_t
    nbytes_total() const {
        return std::visit(
            [](const auto & r) noexcept { return r.nbytes_total(); }, reader_);
    }

    // Get next line of file. Will be null-terminated.
    std::string_view
    nextline() {
        size_t res = std::visit(
            [this](auto & r) noexcept {
                return r.readline(&line_, &line_size_);
            },
            reader_);

        if (res == reader_base_t::k_err || res == 0) {
            if (res == reader_base_t::k_err) {
                (void)fprintf(stderr, "Error reading input lines\n");
            }
            return std::string_view{ "", 0 };
        }

        // Null term.
        std::tie(line_, line_size_) =
            reader_base_t::maybe_realloc_mem(line_, res, line_size_, 1);
        line_[res] = 0;
        return std::string_view{ reinterpret_cast<char *>(line_), res + 1 };
    }
};

}  // namespace tlo


#endif
