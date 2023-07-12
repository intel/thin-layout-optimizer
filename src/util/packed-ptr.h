#ifndef SRC_D_UTIL_D_PACKED_PTR_H_
#define SRC_D_UTIL_D_PACKED_PTR_H_

#include "src/util/bitfield.h"
#include "src/util/type-info.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string_view>

////////////////////////////////////////////////////////////////////////////////
// Helper for storing pointer + meta data in the same class.


namespace tlo {

template<typename T_ptr_t, size_t... Tk_meta_sizes>
struct pptr_t {
    using bits_t = bitfield_t<Tk_meta_sizes..., 48>;
    static_assert(sizeof(bits_t) == sizeof(uintptr_t));

    static_assert(std::is_pointer<T_ptr_t>::value);
    static_assert(std::is_trivially_constructible<bits_t>::value);
    static_assert(std::is_trivially_copy_constructible<bits_t>::value);
    static_assert(std::is_trivially_move_constructible<bits_t>::value);
    static_assert(std::is_trivially_copyable<bits_t>::value);
    static_assert(std::is_trivially_destructible<bits_t>::value);


    static constexpr size_t k_num_meta = sizeof...(Tk_meta_sizes);

    bits_t bits_;

    template<typename... Ts_t>
    constexpr pptr_t(T_ptr_t p, Ts_t... ts) {
        set_ptr_meta(p, ts...);
    }

    constexpr pptr_t(T_ptr_t p) : pptr_t(p, 0) {}
    template<typename... Ts_t>
    constexpr pptr_t(Ts_t... ts) : pptr_t(nullptr, ts...) {}
    constexpr pptr_t() = default;

    constexpr bool
    null() const {
        return bits_.serialized() == 0;
    }

    constexpr T_ptr_t
    ptr() const {
        return reinterpret_cast<T_ptr_t>(bits_.template get<k_num_meta>());
    }


    template<typename... Ts_t>
    constexpr void
    set_ptr_meta(T_ptr_t p, Ts_t... ts) {
        bits_ = { ts... };
        bits_.template init<k_num_meta>(reinterpret_cast<uintptr_t>(p));
    }

    template<size_t Tk_idx>
    constexpr uint16_t
    meta() const {
        static_assert(Tk_idx < k_num_meta);
        return static_cast<uint16_t>(bits_.template get<Tk_idx>());
    }

    template<size_t Tk_idx>
    static constexpr size_t
    meta_max() {
        static_assert(Tk_idx < k_num_meta);
        return bits_t::template field_mask<Tk_idx>();
    }

    template<size_t Tk_idx, typename T_t>
    constexpr void
    set_meta(T_t v) {
        static_assert(Tk_idx < k_num_meta);
        bits_.template set<Tk_idx>(v);
    }

    template<size_t Tk_idx>
    constexpr void
    clear_meta() {
        static_assert(Tk_idx < k_num_meta);
        bits_.template clear<Tk_idx>();
    }

    constexpr void
    set_ptr(T_ptr_t p) {
        bits_.template set<k_num_meta>(reinterpret_cast<uintptr_t>(p));
    }
};

// Small string (just specialized version of packed pointer).
template<typename T_ptr_t, size_t Tk_extra_bits = 0>
struct small_str_t
    : public pptr_t<T_ptr_t, (16 - Tk_extra_bits), Tk_extra_bits> {

    using pptr_base_t = pptr_t<T_ptr_t, (16 - Tk_extra_bits), Tk_extra_bits>;

    static_assert(std::is_same<T_ptr_t, char const *>::value ||
                  std::is_same<T_ptr_t, char *>::value);


    static constexpr size_t k_len_bits   = (16 - Tk_extra_bits);
    static constexpr size_t k_extra_bits = Tk_extra_bits;

    static constexpr size_t k_max_len   = pptr_base_t::template meta_max<0>();
    static constexpr size_t k_max_extra = pptr_base_t::template meta_max<1>();

    template<size_t Tk_other_extra_bits>
    constexpr small_str_t(small_str_t<T_ptr_t, Tk_other_extra_bits> other)
        : small_str_t(other.str(),
                      static_cast<uint16_t>(other.len()),
                      other.extra()) {
        if constexpr (Tk_other_extra_bits != k_extra_bits) {
            if (!fits(other)) {
                fprintf(stderr, "Bad packing: %zu/%zu -> %zu (%s)\n",
                        other.len(), Tk_other_extra_bits, k_extra_bits,
                        other.str());
            }
            assert(fits(other));
        }
    }

    constexpr small_str_t(T_ptr_t p, uint16_t len) : pptr_base_t(p, len) {}
    constexpr small_str_t(T_ptr_t p, uint16_t len, uint16_t e)
        : pptr_base_t(p, len, e) {}
    constexpr small_str_t() = default;

    template<typename T_t>
    static constexpr bool
    len_will_fit(T_t l) {
        return l <= k_max_len;
    }

    static bool
    extra_will_fit(uint16_t e) {
        return e <= k_max_extra;
    }

    template<typename T_t>
    static constexpr bool
    fits(T_t l) {
        return len_will_fit(l);
    }


    template<typename T_t>
    static constexpr bool
    fits(T_t l, uint16_t m) {
        return len_will_fit(l) && extra_will_fit(m);
    }

    template<size_t Tk_other_extra_bits>
    static constexpr bool
    fits(small_str_t<T_ptr_t, Tk_other_extra_bits> other) {
        return Tk_other_extra_bits == k_extra_bits ||
               fits(other.len(), other.extra());
    }

    constexpr size_t
    len() const {
        return this->template meta<0>();
    }

    constexpr uint16_t
    extra() const {
        return this->template meta<1>();
    }

    constexpr void
    set_extra(uint16_t ebits) {
        this->template set_meta<1>(ebits);
    }

    constexpr void
    clear_extra() {
        this->template clear_meta<1>();
    }

    constexpr T_ptr_t
    str() const {
        return this->ptr();
    }

    constexpr bool
    empty() const {
        return len() == 0;
    }

    template<size_t Tk_other_extra_bits>
    constexpr bool
    eq(small_str_t<const char *, Tk_other_extra_bits> other) const {
        return other.len() == len() && !memcmp(other.str(), str(), len());
    }

    template<size_t Tk_other_extra_bits>
    constexpr bool
    eq(small_str_t<char *, Tk_other_extra_bits> other) const {
        return other.len() == len() && !memcmp(other.str(), str(), len());
    }

    constexpr bool
    eq(std::string_view sv) const {
        return sv.length() == len() && !memcmp(sv.data(), str(), sv.length());
    }

    constexpr std::string_view
    sview() const {
        return std::string_view{ str(), len() };
    }

    constexpr void
    update_str(T_ptr_t p) {
        this->set_ptr(p);
    }

    constexpr bool
    active() const {
        return this->bits_.serialized() != 0;
    }
};

static_assert(has_okay_type_traits<small_str_t<char *>>::value);
static_assert(has_okay_type_traits<small_str_t<char const *>>::value);
}  // namespace tlo
#endif
