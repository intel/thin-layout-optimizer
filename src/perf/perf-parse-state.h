#include "src/util/compiler.h"
#include "src/util/macro.h"
#include "src/util/memory.h"

#include "src/util/verbosity.h"

#include <charconv>
#include <string_view>
#include <tuple>

#include <assert.h>
namespace tlo {
namespace perf {
// Helper. Maintains the buffer and has APIs for matching expected tokens.
struct parse_state_t {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    const std::string_view buf_;
    size_t                 off_;

    template<typename T_t>
    std::tuple<T_t, bool>
    get_int(int base) {
        bool neg = false;
        // NOLINTNEXTLINE(*magic*)
        if (base != 10 && base != 16) {
            return { 0, true };
        }

        // NOLINTNEXTLINE(*magic*)
        if (base == 16 && (off_ + 2) < buf_.size() && buf_[off_] == '0' &&
            (buf_[off_ + 1] == 'x' || buf_[off_ + 1] == 'X')) {
            off_ += 2;
        }
        // NOLINTNEXTLINE(*magic*)
        else if (base == 10 && (off_ + 1) < buf_.size() && buf_[off_] == '-') {
            neg = true;
            off_ += 1;
        }

        T_t  out = 0;
        auto res = std::from_chars(buf_.begin() + off_, buf_.end(), out, base);
        if (neg) {
            out = static_cast<T_t>(-out);
        }

        if (TLO_UNLIKELY((res.ec != std::errc()) ||
                         (res.ptr <= (buf_.begin() + off_)))) {
            return { out, true };
        }
        off_ = static_cast<size_t>(res.ptr - buf_.begin());
        return { out, false };
    }

    template<typename T_t>
    std::tuple<T_t, bool>
    get_hexint() {
        return get_int<T_t>(16);  // NOLINT(*magic*)
    }

    template<typename T_t>
    std::tuple<T_t, bool>
    get_decint() {
        return get_int<T_t>(10);  // NOLINT(*magic*)
    }

    size_t
    bytes_parsed() const {
        return off_;
    }

    size_t
    remaining() const {
        return buf_.size() - off_;
    }

    char const *
    cur() const {
        return buf_.data() + off_;
    }

    static bool
    is_end(char c) {
        return c == '\0' || c == '\n';
    }

    static bool
    is_ws(char c) {
        return c == ' ';
    }

    static bool
    is_digit(char c) {
        return (c >= '0' && c <= '9') || c == '-';
    }

    bool
    skip_to(char c) {
        size_t i, e;
        for (i = off_, e = buf_.size(); i < e && buf_[i] != c; ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    skip_to(char c0, char c1) {
        size_t i, e;
        for (i = off_, e = buf_.size(); i < e && buf_[i] != c0 && buf_[i] != c1;
             ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    skip_to_or_ws(char c) {
        size_t i, e;
        for (i = off_, e = buf_.size();
             i < e && buf_[i] != c && !is_ws(buf_[i]); ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    skip_to_ws() {
        size_t i, e;
        for (i = off_, e = buf_.size(); i < e && !is_ws(buf_[i]); ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    skip_back_before_ws() {
        size_t i;
        for (i = off_; i >= 1 && is_ws(buf_[i - 1]); --i) {
        }
        off_ = i;
        return i > 1;
    }

    bool
    skip_back_to_c(char c) {
        size_t i;
        for (i = off_; i != 0 && buf_[i] != c; --i) {
        }
        off_ = i;
        return i != 0;
    }

    bool
    skip_back_before_digit() {
        size_t i;
        for (i = off_; i >= 1 && is_digit(buf_[i - 1]); --i) {
        }
        off_ = i;
        return i > 1;
    }


    bool
    skip_to_digit() {
        size_t i, e;
        for (i = off_, e = buf_.size(); i < e && !is_digit(buf_[i]); ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    skip_at(char c) {
        size_t i, e;
        for (i = off_, e = buf_.size(); i < e && buf_[i] == c; ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    skip_ws() {
        size_t i, e;
        for (i = off_, e = buf_.size(); i < e && is_ws(buf_[i]); ++i) {
        }
        off_ = i;
        return i != e;
    }

    bool
    at_end() const {
        return off_ >= buf_.size() || is_end(buf_[off_]);
    }


    bool
    at_ws() const {
        return off_ < buf_.size() && is_ws(buf_[off_]);
    }

    bool
    at_c(char c) const {
        return off_ < buf_.size() && buf_[off_] == c;
    }

    bool
    at_str(const std::string_view sv) const {
        return (off_ + sv.length()) < buf_.size() &&
               (memcmp(buf_.data() + off_, sv.data(), sv.length()) == 0);
    }


    bool
    skip_fwd(size_t n) {
        if (n + off_ > buf_.size()) {
            return false;
        }
        off_ += n;
        return true;
    }

    bool
    skip_bkwd(size_t n) {
        if (off_ < n) {
            return false;
        }
        off_ -= n;
        return true;
    }

    bool
    skip_str(std::string_view sv) {
        return at_str(sv) && skip_fwd(sv.length());
    }

    void
    dump(FILE * fp = stderr, int vlvl = 1, size_t maxlen = 0) const {
        // TODO: This unsafe! string_view doesn't always have null term. We are
        // pretty careful about maintaining one for usability, but this should
        // be replaced with something safer.
        if (maxlen != 0) {
            char * print_buf = reinterpret_cast<char *>(buf_zalloc(maxlen + 1));

            memcpy(print_buf, cur(), std::min(buf_.size(), maxlen));
            TLO_fprint_ifv(vlvl, fp, "\"%s\"%s\n", print_buf,
                           maxlen < buf_.size() ? "..." : "");
            buf_free(print_buf, maxlen + 1);
        }
        else {
            TLO_fprint_ifv(vlvl, fp, "\"%s\"\n", cur());
        }
    }
};

}  // namespace perf
}  // namespace tlo
