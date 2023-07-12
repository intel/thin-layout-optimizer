#ifndef SRC_D_SYSTEM_D_BR_INSN_H_
#define SRC_D_SYSTEM_D_BR_INSN_H_

#include "src/system/insn.h"

#include "src/util/compiler.h"
#include "src/util/packed-ptr.h"
#include "src/util/type-info.h"
#include "src/util/xxhash.h"
#include "src/util/algo.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>


#include <limits>
#include <span>

////////////////////////////////////////////////////////////////////////////////
// Class for storing meta data on the different branch types + decoding bytes ->
// branch type.


namespace tlo {
namespace system {


namespace detail {
using br_enc_t = uint32_t;


// Traits that each branch can have. Not really compact. This entire struct
// should be rewritten.
enum brtype_t : uint32_t {
    CALL        = 1,
    JMP         = 2,
    IND         = 4,
    REL         = 8,
    COND        = 16,
    UNCOND      = 32,
    INT         = 64,
    RET         = 128,
    LOOP        = 256,
    RIPREL      = 512,
    DBL_IND     = 1024,
    BAD         = 2048,
    END         = 2048,
    BRTYPE_BITS = 12,
};
static_assert((1 << brtype_t::BRTYPE_BITS) == 2 * brtype_t::END);
static_assert(brtype_t::BRTYPE_BITS <= 12);
// Descriptions of branch (the actually br_insn_t is a 1-byte offset used to
// lookup a br_insn_desc_t).
struct br_insn_desc_t {

    static constexpr size_t k_max_enc_length = 4;
    static_assert(k_max_enc_length <= sizeof(br_enc_t));

    static constexpr size_t k_brtype_bits = brtype_t::BRTYPE_BITS;
    static constexpr size_t k_len_bits    = 16 - k_brtype_bits;

    static constexpr size_t k_brtype_idx = 0;
    static constexpr size_t k_len_idx    = 1;

    char const * name_;
    brtype_t     brtype_;
    br_enc_t     enc_;


    // Some bullshit C++ to construct from our table.
    template<typename T_t, typename... Ts_t>
    static constexpr br_enc_t
    enc_bytes_to_enc_impl(T_t first_byte, Ts_t &&... enc_bytes) {
        return (static_cast<br_enc_t>(first_byte)
                << (sizeof...(Ts_t) * CHAR_BIT)) |
               enc_bytes_to_enc_impl(std::forward<Ts_t>(enc_bytes)...);
    }

    template<typename T_t>
    static constexpr br_enc_t
    enc_bytes_to_enc_impl(T_t first_byte) {
        return static_cast<br_enc_t>(first_byte);
    }

    template<typename... Ts_t>
    static constexpr br_enc_t
    enc_bytes_to_enc(Ts_t &&... enc_bytes) {
        static_assert(sizeof...(Ts_t) <= sizeof(br_enc_t));
        static_assert(sizeof...(Ts_t));
        return enc_bytes_to_enc_impl(std::forward<Ts_t>(enc_bytes)...);
    }


    constexpr br_insn_desc_t() = default;
    template<typename... Ts_t>
    constexpr br_insn_desc_t(char const * name,
                             uint32_t     brtype,
                             Ts_t &&... enc_bytes)
        : name_(name),
          brtype_(static_cast<brtype_t>(brtype)),
          enc_(enc_bytes_to_enc(std::forward<Ts_t>(enc_bytes)...)) {
        static_assert(sizeof...(Ts_t) <= sizeof(br_enc_t));
        static_assert(sizeof...(Ts_t) <= k_max_enc_length);
    }


    constexpr char const *
    name() const {
        return name_;
    }

    constexpr uint32_t
    brtype() const {
        return static_cast<uint32_t>(brtype_);
    }

    constexpr bool
    is_call() const {
        return !!(brtype() & brtype_t::CALL);
    }

    constexpr bool
    is_ret() const {
        return !!(brtype() & brtype_t::RET);
    }

    constexpr bool
    is_jmp() const {
        return !!(brtype() & brtype_t::JMP);
    }

    constexpr bool
    lt(br_enc_t test_enc) const {
        return enc_ < test_enc;
    }

    constexpr bool
    lt(br_insn_desc_t const & other) const {
        return lt(other.enc_);
    }

    constexpr bool
    gt(br_enc_t test_enc) const {
        return enc_ > test_enc;
    }

    constexpr bool
    gt(br_insn_desc_t const & other) const {
        return gt(other.enc_);
    }

    constexpr bool
    eq(br_enc_t test_enc) const {
        return enc_ == test_enc;
    }

    constexpr bool
    eq(br_insn_desc_t const & other) const {
        return eq(other.enc_);
    }

    constexpr uint64_t
    hash() const {
        return xxhash::run(enc_);
    }
};

// A few tests we properly translate encoding bytes -> u32.
static_assert(br_insn_desc_t::enc_bytes_to_enc(0x87) == 0x87);
static_assert(br_insn_desc_t::enc_bytes_to_enc(0x87, 0x65) == 0x8765);
static_assert(br_insn_desc_t::enc_bytes_to_enc(0x87, 0x65, 0x43) == 0x876543);
static_assert(br_insn_desc_t::enc_bytes_to_enc(0x87, 0x65, 0x43, 0x21) ==
              0x87654321);


static_assert(has_okay_type_traits<br_insn_desc_t>::value);
static_assert(sizeof(br_insn_desc_t) == 16);
extern const std::span<const br_insn_desc_t> k_br_insn_descs;

}  // namespace detail


// Actual handle we use in perf_sample.
struct br_insn_t {
    uint16_t desc_idx_;


    constexpr br_insn_t() = default;
    template<typename T_t>
        requires(std::is_integral<T_t>::value)
    constexpr br_insn_t(T_t i) : desc_idx_(static_cast<uint16_t>(i)) {}


    static detail::br_insn_desc_t const *
    end_desc() {
        return &(detail::k_br_insn_descs[end()]);
    }

    static detail::br_insn_desc_t const *
    start_desc() {
        return &(detail::k_br_insn_descs[0]);
    }

    static constexpr size_t
    end() {
        return detail::k_br_insn_descs.size();
    }

    constexpr bool
    valid() const {
        return desc_idx_ <= end();
    }

    constexpr bool
    bad() const {
        return desc_idx_ == end();
    }

    constexpr bool
    good() const {
        return desc_idx_ < end();
    }

    constexpr detail::br_insn_desc_t const *
    desc() const {
        return &(detail::k_br_insn_descs[desc_idx_]);
    }

    constexpr bool
    lt(br_insn_t other) const {
        return desc()->lt(*(other.desc()));
    }

    constexpr bool
    gt(br_insn_t other) const {
        return desc()->gt(*(other.desc()));
    }

    constexpr bool
    eq(br_insn_t other) const {
        return desc()->eq(*(other.desc()));
    }

    constexpr uint64_t
    hash() const {
        return desc()->hash();
    }

    constexpr char const *
    name() const {
        return desc()->name();
    }


    constexpr bool
    is_trackable_call() const {
        return desc()->is_call() || desc()->is_jmp();
    }

    constexpr bool
    is_ret() const {
        return desc()->is_ret();
    }

    static constexpr br_insn_t
    make_bad() {
        return br_insn_t{ static_cast<uint16_t>(end()) };
    }
    static constexpr bool
    is_prefix_byte(uint8_t byte) {
        switch (byte) {
            default:
                return false;
            case 0xf2:  // bnd (repnz)
            case 0xf3:  // repz
            case 0x3e:  // notrack
            case 0x66:  // 16-bit (lcp)
            case 0x48:  // rex (64-bit)
                return true;
        }
    }

    // Called on set of insn_bytes in perf-parse. We decode by basically looking
    // up in sorted br_insn_desc table.
    static br_insn_t TLO_PURE
    find(std::array<uint8_t, system::k_max_insn_sz> insn_bytes) {
        // notrack, bnd, repz prefix
        uint32_t idx = 0;
        for (; idx < insn_bytes.size() && is_prefix_byte(insn_bytes[idx]);
             ++idx) {
        }
        assert(idx != insn_bytes.size());
        detail::br_enc_t enc     = 0;
        uint32_t         end_idx = idx + static_cast<uint32_t>(
                                     detail::br_insn_desc_t::k_max_enc_length);
        for (; idx < end_idx; ++idx) {
            enc <<= 8;
            enc |= insn_bytes[idx];
            const detail::br_insn_desc_t * res =
                std::lower_bound(start_desc(), end_desc(), enc,
                                 [](detail::br_insn_desc_t const & desc,
                                    detail::br_enc_t search_enc) -> bool {
                                     return desc.lt(search_enc);
                                 });
            if (res < end_desc() && res->eq(enc)) {
                return br_insn_t{ res - start_desc() };
            }
        }
        return make_bad();
    }
};
static_assert(has_okay_type_traits<br_insn_t>::value);
}  // namespace system
}  // namespace tlo


#endif
