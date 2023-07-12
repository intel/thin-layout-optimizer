#ifndef SRC_D_UTIL_D_STRBUF_H_
#define SRC_D_UTIL_D_STRBUF_H_


#include "src/util/memory.h"
#include "src/util/packed-ptr.h"
#include "src/util/type-info.h"
#include "src/util/umap.h"
#include "src/util/xxhash.h"

#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Common buffer to hold a string. Maintains length / hash value.

namespace tlo {

// TODO: Templatize so we can accept some meta data bits.
template<size_t Tk_extra_bits = 0>
struct strbuf_t {
    using sstr_t = small_str_t<char const *, Tk_extra_bits>;
    sstr_t   buf_;
    uint64_t hash_;

    void
    save_to(char * dst) {
        memcpy(dst, str(), len());
        buf_.update_str(dst);
    }

    void
    save() {
        save_to(tlo::str_malloc(len()));
    }

    void
    unsave() const {
        tlo::str_free(str(), len());
    }

    template<typename T_t>
    static constexpr bool
    len_will_fit(T_t l) {
        return sstr_t::len_will_fit(l);
    }

    static constexpr bool
    extra_will_fit(uint16_t e) {
        return sstr_t::extra_will_fit(e);
    }

    static constexpr bool
    fits(size_t sz) {
        return sstr_t::fits(sz);
    }

    static constexpr bool
    fits(std::string_view sv) {
        return sstr_t::fits(sv.length());
    }

    template<size_t Tk_other_extra_bits>
    static constexpr bool
    fits(const strbuf_t<Tk_other_extra_bits> & other) {
        return sstr_t::fits(other.buf_);
    }

    template<size_t Tk_other_extra_bits>
    static constexpr bool
    fits(small_str_t<char const *, Tk_other_extra_bits> ss) {
        return sstr_t::fits(ss);
    }

    constexpr strbuf_t() = default;

    constexpr strbuf_t(char const * s, size_t sz, uint16_t meta)
        : buf_(s, static_cast<uint16_t>(sz), meta), hash_(xxhash::run(s, sz)) {}

    constexpr strbuf_t(char const * s, size_t sz)
        : buf_(s, static_cast<uint16_t>(sz)), hash_(xxhash::run(s, sz)) {}


    template<size_t Tk_other_extra_bits>
    constexpr strbuf_t(small_str_t<char const *, Tk_other_extra_bits> ss,
                       uint64_t                                       hash)
        : buf_(ss), hash_(hash) {
        if constexpr (Tk_other_extra_bits != Tk_extra_bits) {
            assert(fits(ss));
        }
    }

    template<size_t Tk_other_extra_bits>
    constexpr strbuf_t(small_str_t<char const *, Tk_other_extra_bits> ss)
        : strbuf_t(ss, xxhash::run(ss.str(), ss.len())) {}

    template<size_t Tk_other_extra_bits>
    constexpr strbuf_t(small_str_t<char const *, Tk_other_extra_bits> ss,
                       uint16_t                                       meta)
        : strbuf_t(ss.str(), ss.len(), meta) {}

    template<size_t Tk_other_extra_bits>
        requires(Tk_other_extra_bits != Tk_extra_bits)
    constexpr strbuf_t(const strbuf_t<Tk_other_extra_bits> & other)
        : buf_(other.buf_), hash_(other.hash_) {
        assert(fits(other));
    }

    constexpr strbuf_t(std::string_view sv)
        : strbuf_t(sv.data(), sv.length()) {}
    constexpr strbuf_t(std::string_view sv, uint16_t meta)
        : strbuf_t(sv.data(), sv.length(), meta) {}


    template<size_t Tk_other_extra_bits>
    constexpr bool
    eq(strbuf_t<Tk_other_extra_bits> const & other) const {
        return other.hash_ == hash_ && other.len() == len() &&
               (len() == 0 || memcmp(str(), other.str(), len()) == 0);
    }

    constexpr bool
    eq(std::string_view sv) const {
        return sv.length() == len() &&
               (len() == 0 || memcmp(str(), sv.data(), sv.length()) == 0);
    }

    static constexpr int
    cmp_strs(const char * s0, size_t len0, const char * s1, size_t len1) {
        if (len0 == len1) {
            return memcmp(s0, s1, len0);
        }
        if (len0 > len1) {
            return memcmp(s0, s1, len1) | 1;
        }
        int r = memcmp(s0, s1, len0);
        return r <= 0 ? -1 : r;
    }

    template<size_t Tk_other_extra_bits>
    constexpr int
    cmp(strbuf_t<Tk_other_extra_bits> const & other) const {
        return cmp_strs(str(), len(), other.str(), other.len());
    }

    constexpr int
    cmp(std::string_view sv) const {
        return cmp_strs(str(), len(), sv.data(), sv.length());
    }


    template<size_t Tk_other_extra_bits>
    constexpr bool
    lt(strbuf_t<Tk_other_extra_bits> const & other) const {
        return cmp(other) < 0;
    }

    constexpr bool
    lt(std::string_view sv) const {
        return cmp(sv) < 0;
    }

    constexpr bool
    startswith(std::string_view sv) const {
        return sv.length() <= len() &&
               memcmp(str(), sv.data(), sv.length()) == 0;
    }

    constexpr strbuf_t<>
    without_extra() const {
        auto copy = *this;
        copy.clear_extra();
        return copy;
    }

    constexpr void
    clear_extra() {
        buf_.clear_extra();
    }

    constexpr void
    set_extra(uint16_t v) {
        buf_.set_extra(v);
    }

    constexpr uint16_t
    extra() const {
        return buf_.extra();
    }

    constexpr bool
    empty() const {
        return buf_.empty();
    }

    constexpr size_t
    len() const {
        return buf_.len();
    }

    constexpr uint64_t
    hash() const {
        return hash_;
    }

    constexpr char const *
    str() const {
        return buf_.str();
    }

    constexpr bool
    null() const {
        return buf_.null();
    }

    constexpr bool
    active() const {
        return buf_.active();
    }

    constexpr std::string_view
    sview() const {
        return std::string_view(str(), len());
    }
};


static_assert(has_okay_type_traits<strbuf_t<>>::value);
static_assert(has_okay_type_traits<strbuf_t<1>>::value);

}  // namespace tlo
#endif
