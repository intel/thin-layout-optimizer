#ifndef SRC_D_UTIL_D_BITFIELD_H_
#define SRC_D_UTIL_D_BITFIELD_H_

////////////////////////////////////////////////////////////////////////////////
// A bitfield helper that creates essentially arbitrary bitfields.

#include "src/util/type-info.h"

#include <utility>

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

template<size_t... Ts_k_field_sizes>
class bitfield_t {
    static constexpr size_t k_num_fields = sizeof...(Ts_k_field_sizes);
    static_assert(k_num_fields > 0, "Requires at least one bitfield member!");

    template<size_t Tk_remaining,
             size_t Tk_offset,
             size_t Tk_size,
             size_t... Ts_k_next_sizes>
    struct field_info_generator {
        static_assert(Tk_remaining < k_num_fields, "Invalid state reached");
        using next_field_info_generator =
            field_info_generator<Tk_remaining - 1,
                                 Tk_offset + Tk_size,
                                 Ts_k_next_sizes...>;

        static constexpr size_t k_offset = next_field_info_generator::k_offset;
        static constexpr size_t k_bitwidth =
            next_field_info_generator::k_bitwidth;
        static constexpr size_t k_sum = next_field_info_generator::k_sum;
    };

    template<size_t Tk_offset, size_t Tk_size, size_t... Ts_k_next_sizes>
    struct field_info_generator<0, Tk_offset, Tk_size, Ts_k_next_sizes...> {
        static constexpr size_t k_offset   = Tk_offset;
        static constexpr size_t k_bitwidth = Tk_size;
        static constexpr size_t k_sum      = k_offset + k_bitwidth;
    };


    static constexpr size_t k_raw_bitwidth =
        field_info_generator<k_num_fields - 1, 0, Ts_k_field_sizes...>::k_sum;
    static_assert(
        k_raw_bitwidth == 8 || k_raw_bitwidth == 16 || k_raw_bitwidth == 32 ||
            k_raw_bitwidth == 64,
        "Total bitfield width is invalid (doesn't match any known integer type)");

   public:
    using base_t = typename std::conditional<
        k_raw_bitwidth <= 16,
        typename std::conditional<k_raw_bitwidth == 8, uint8_t, uint16_t>::type,
        typename std::conditional<k_raw_bitwidth == 32, uint32_t, uint64_t>::
            type>::type;

   private:
    static constexpr size_t k_base_bitwidth = sizeof(base_t) * CHAR_BIT;
    static_assert(k_base_bitwidth == k_raw_bitwidth, "Invalid state reached");


    template<size_t Tk_size>
    struct bitmask {
        static constexpr base_t k_mask =
            Tk_size == k_base_bitwidth
                ? std::numeric_limits<base_t>::max()
                : ((static_cast<base_t>(1) << Tk_size) - 1);
    };

    template<size_t Tk_idx>
    struct field_query {
        static_assert(Tk_idx < k_num_fields, "Out of bounds index");

        using info = field_info_generator<Tk_idx, 0, Ts_k_field_sizes...>;

        static constexpr size_t k_offset       = info::k_offset;
        static constexpr size_t k_bitwidth     = info::k_bitwidth;
        static constexpr size_t k_sum          = info::k_sum;
        static constexpr base_t k_mask         = bitmask<k_bitwidth>::k_mask;
        static constexpr base_t k_mask_inplace = k_mask << k_offset;
    };


    template<size_t Tk_idx, size_t Tk_size, size_t... Ts_k_next_sizes>
    struct serializer {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        template<typename... Ts_t>
        static constexpr base_t
        get(base_t b, Ts_t... ts) {
            static_assert(sizeof...(ts) < k_num_fields,
                          "To many initializers!");
            return get(b) |
                   static_cast<base_t>(
                       serializer<Tk_idx + 1, Ts_k_next_sizes...>::get(ts...)
                       << Tk_size);
        }


        static constexpr base_t
        get(base_t b) {
            return b & bitmask<Tk_size>::k_mask;
        }
    };

    template<size_t Tk_size, size_t... Ts_k_next_sizes>
    struct serializer<k_num_fields - 1, Tk_size, Ts_k_next_sizes...> {
        template<typename... Ts_t>
        static constexpr base_t
        get(base_t b) {
            return b & bitmask<Tk_size>::k_mask;
        }
    };


   public:
    template<size_t Tk_idx>
    static constexpr size_t
    field_size() {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        return field_query<Tk_idx>::k_bitwidth;
    }

    template<size_t Tk_idx>
    static constexpr size_t
    field_offset() {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        return field_query<Tk_idx>::k_offset;
    }

    template<size_t Tk_idx>
    static constexpr base_t
    field_mask() {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        return field_query<Tk_idx>::k_mask;
    }

    template<size_t Tk_idx, typename T_t>
    static constexpr bool
    fits(T_t v) {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        static_assert(std::is_integral<T_t>::value,
                      "Only integral types are usable in a bitfield");
        using query = field_query<Tk_idx>;
        return static_cast<base_t>(v) <= query::k_mask;
    }


    template<size_t Tk_idx>
    constexpr base_t
    get() const {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        using query = field_query<Tk_idx>;
        return (base_ >> query::k_offset) & query::k_mask;
    }


    template<size_t Tk_idx, typename T_t>
#if __cplusplus >= 201402L
    constexpr
#endif
        void
        init(T_t v) {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        static_assert(std::is_integral<T_t>::value,
                      "Only integral types are usable in a bitfield");
        using query = field_query<Tk_idx>;
        base_t bv =
            static_cast<base_t>(static_cast<base_t>(v) << query::k_offset);
        base_ |= (bv & query::k_mask_inplace);
    }

    template<size_t Tk_idx>
#if __cplusplus >= 201402L
    constexpr
#endif
        void
        clear() {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        using query = field_query<Tk_idx>;
        base_ &= (~query::k_mask_inplace);
    }

    template<size_t Tk_idx, typename T_t>
#if __cplusplus >= 201402L
    constexpr
#endif
        void
        set(T_t v) {
        static_assert(Tk_idx < k_num_fields, "Index out of range");
        clear<Tk_idx>();
        init<Tk_idx>(v);
    }

    constexpr base_t
    serialized() const {
        return base_;
    }

    template<typename... Ts_t>
    constexpr bitfield_t(Ts_t... ts)
        : base_(serializer<0, Ts_k_field_sizes...>::get(
              static_cast<base_t>(std::forward<Ts_t>(ts))...)) {
        static_assert(sizeof...(ts) <= k_num_fields, "To many initializers!");
    }

    template<typename T_t>
    constexpr bitfield_t(T_t v) : base_(static_cast<base_t>(v)) {
        static_assert(std::is_integral<T_t>::value);
    }

    constexpr bitfield_t() = default;

   private:
    base_t base_;
};


#endif
