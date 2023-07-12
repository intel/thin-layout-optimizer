#ifndef SRC_D_UTIL_D_STRBUF_TAB_H_
#define SRC_D_UTIL_D_STRBUF_TAB_H_

////////////////////////////////////////////////////////////////////////////////
// String table. Used by the elffile / dso to allocate all function names,
// version numbers, and dsos.

#include <cstring>
#include "src/util/compiler.h"
#include "src/util/memory.h"
#include "src/util/packed-ptr.h"
#include "src/util/strbuf.h"
#include "src/util/umap.h"
#include "src/util/xxhash.h"

namespace tlo {

struct fixed_str_t {
    struct internal_str_t {
        const uint64_t hash_;
        TLO_DISABLE_WC99_EXTENSIONS
        char chars_[1];
        TLO_REENABLE_WC99_EXTENSIONS

        constexpr explicit internal_str_t(uint64_t     hash,
                                          const char * s,
                                          size_t       slen)
            : hash_(hash) {
            std::memcpy(chars_, s, slen);
        }

        internal_str_t() : hash_(xxhash::empty()) {}
    };


    pptr_t<const internal_str_t *, 15, 1> pptr_;


    constexpr bool
    status() const {
        return pptr_.template meta<1>();
    }


    constexpr void
    set_status() {
        pptr_.template set_meta<1>(1);
    }

    constexpr void
    set_status_v(bool v) {
        pptr_.template set_meta<1>(static_cast<uint16_t>(v));
    }

    constexpr void
    clr_status() {
        pptr_.template set_meta<1>(0);
    }


    constexpr uint16_t
    len() const {
        return pptr_.template meta<0>();
    }

    constexpr const char *
    str() const {
        return pptr_.ptr()->chars_;
    }

    template<size_t k_meta_bits>
    constexpr small_str_t<const char *, k_meta_bits>
    sstr(uint16_t meta) const {
        return { str(), len(), meta };
    }

    constexpr small_str_t<const char *>
    sstr() const {
        return { str(), len() };
    }

    constexpr uint64_t
    hash() const {
        return pptr_.ptr()->hash_;
    }

    template<size_t k_meta_bits>
    constexpr strbuf_t<k_meta_bits>
    sbuf(uint16_t meta) const {
        return strbuf_t<k_meta_bits>{ sstr<k_meta_bits>(meta), hash() };
    }

    constexpr strbuf_t<>
    sbuf() const {
        return strbuf_t<>{ sstr(), hash() };
    }

    constexpr std::string_view
    sview() const {
        return { str(), len() };
    }

    template<size_t k_meta_bits>
    explicit fixed_str_t(const strbuf_t<k_meta_bits> sb,
                         bump_alloc_t<> *            allocator)
        : fixed_str_t(new(allocator->getz(sizeof(internal_str_t) + sb.len() + 1,
                                          alignof(internal_str_t)))
                          internal_str_t{ sb.hash(), sb.str(), sb.len() },
                      static_cast<uint16_t>(sb.len()),
                      static_cast<uint16_t>(0)) {}

    constexpr explicit fixed_str_t(const internal_str_t * istr,
                                   uint16_t               istr_len)
        : fixed_str_t(istr, istr_len, false) {}

    constexpr fixed_str_t(const internal_str_t * istr,
                          uint16_t               istr_len,
                          bool                   status)
        : pptr_(istr, istr_len, static_cast<uint16_t>(status)) {}

    constexpr fixed_str_t
    with_status_v(bool status) const {
        fixed_str_t other = *this;
        other.set_status_v(status);
        return other;
    }
};

struct strtab_ret_t {
    fixed_str_t fstr_;


    constexpr strtab_ret_t(fixed_str_t fstr, bool added)
        : fstr_(fstr.with_status_v(added)) {}

    constexpr strtab_ret_t(fixed_str_t fstr) : fstr_(fstr) {}

    constexpr bool
    added() const {
        return fstr_.status();
    }

    constexpr fixed_str_t
    fstr() const {
        return fstr_.with_status_v(false);
    }

    template<size_t k_meta_bits>
    constexpr strbuf_t<k_meta_bits>
    sbuf(uint16_t meta) const {
        return fstr_.template sbuf<k_meta_bits>(meta);
    }

    constexpr strbuf_t<>
    sbuf() const {
        return fstr_.sbuf();
    }

    constexpr std::string_view
    sview() const {
        return fstr_.sview();
    }

    constexpr uint16_t
    len() const {
        return fstr_.len();
    }

    constexpr uint64_t
    hash() const {
        return fstr_.hash();
    }

    constexpr const char *
    str() const {
        return fstr_.str();
    }
};


template<bool k_maybe_empty = false>
struct strtab_t {
    static strtab_ret_t
    get_empty(bool status) {
        static const typename fixed_str_t::internal_str_t k_empty{};
        return { { &k_empty, static_cast<uint16_t>(0), status } };
    }


    template<size_t k_meta_bits_in>
    strtab_ret_t
    get_impl(strbuf_t<k_meta_bits_in> sb) {
        // Any string type convertible to strbuf works

        if constexpr (k_maybe_empty) {
            if (sb.empty()) {
                if (!set_.hit_empty_) {
                    set_.hit_empty_  = true;
                    strtab_ret_t ret = get_empty(true);
                    set_.set_.emplace(ret.sbuf(), &allocator_);
                    return ret;
                }
                return get_empty(false);
            }
        }
        auto res = set_.set_.emplace(sb, &allocator_);

        return strtab_ret_t{ *(res.first), res.second };
    }

    template<size_t k_meta_bits_in>
    strtab_ret_t
    get(strbuf_t<k_meta_bits_in> sb) {
        strtab_ret_t ret = get_impl(sb);
        // This is mostly to help sanitizers out.
        TLO_DEBUG_ONLY(assert(std::strlen(ret.str()) == ret.len()));
        return ret;
    }

    strtab_ret_t
    get(std::string_view str) {
        return get(strbuf_t<>{ str });
    }

    template<size_t k_meta_bits = 0, size_t k_meta_bits_in>
    strbuf_t<k_meta_bits>
    get_sbuf(strbuf_t<k_meta_bits_in> sb) {
        if constexpr (k_meta_bits_in == 0 || k_meta_bits == 0) {
            return get(sb).sbuf();
        }
        else {
            if constexpr (k_meta_bits_in > k_meta_bits) {
                assert(strbuf_t<k_meta_bits>::extra_will_fit(sb.extra()));
            }
            return get(sb).template sbuf<k_meta_bits>(sb.extra());
        }
    }

    template<size_t k_meta_bits, typename T_str_t>
    strbuf_t<k_meta_bits>
    get_sbuf(T_str_t str, uint16_t meta) {
        return get(str).template sbuf<k_meta_bits>(meta);
    }

    strbuf_t<>
    get_sbuf(std::string_view str) {
        return get(strbuf_t<>{ str }).sbuf();
    }

    strbuf_t<>
    get_sbuf(small_str_t<char const *> str) {
        return get(strbuf_t<>{ str }).sbuf();
    }

    size_t
    size() const {
        size_t sz = set_.set_.size();
        return sz;
    }

    struct strtab_hasher_t {
        using is_avalanching = void;
        using is_transparent = void;

        template<typename T_t>
        constexpr uint64_t
        operator()(const T_t & item) const {
            return item.hash();
        }
    };

    struct strtab_equals_t {
        using is_transparent = void;

        template<typename T0_t, typename T1_t>
        constexpr bool
        operator()(const T0_t & lhs, const T1_t & rhs) const {
            return lhs.len() == rhs.len() && lhs.hash() == rhs.hash() &&
                   !std::memcmp(lhs.str(), rhs.str(), rhs.len());
        }
    };


    using base_str_set_t =
        basic_uset<fixed_str_t, strtab_hasher_t, strtab_equals_t>;

    struct str_set_with_empty_t {
        base_str_set_t set_;
        bool           hit_empty_;
    };
    struct str_set_alone_t {
        base_str_set_t set_;
    };

    using str_set_t = typename std::
        conditional<k_maybe_empty, str_set_with_empty_t, str_set_alone_t>::type;
    str_set_t      set_;
    bump_alloc_t<> allocator_;


    template<typename T_t>
    struct str_iterator_t {
        using str_it_base_t = decltype(set_.set_);
        const str_it_base_t & base_;

        struct str_iterator_impl_t {

            using str_it_type_t     = decltype(base_.cbegin());
            using iterator_category = typename str_it_type_t::iterator_category;
            str_it_type_t it_;

            const T_t
            operator*() const {
                if constexpr (std::is_same<T_t, std::string_view>::value) {
                    return (*it_).sview();
                }
                else if constexpr (std::is_same<T_t, strbuf_t<>>::value) {
                    return (*it_).sbuf();
                }
                else {
                    return *it_;
                }
            }

            str_iterator_impl_t &
            operator++() {
                it_++;
                return *this;
            }

            bool
            operator!=(str_iterator_impl_t const & other) {
                return it_ != other.it_;
            }
        };

        str_iterator_impl_t
        begin() const {
            return str_iterator_impl_t{ base_.cbegin() };
        }

        str_iterator_impl_t
        end() const {
            return str_iterator_impl_t{ base_.cend() };
        }
    };

    str_iterator_t<strbuf_t<>>
    sbufs() const {
        return str_iterator_t<strbuf_t<>>{ set_.set_ };
    }

    str_iterator_t<fixed_str_t>
    fixed_strs() const {
        return str_iterator_t<strbuf_t<>>{ set_.set_ };
    }

    str_iterator_t<std::string_view>
    sviews() const {
        return str_iterator_t<std::string_view>{ set_.set_ };
    }
};


}  // namespace tlo
#endif
