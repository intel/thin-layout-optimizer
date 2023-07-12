#include "gtest/gtest.h"

#include "src/util/str-ops.h"

#include <array>
#include <span>

TEST(str_ops, to_hex_str) {
    const std::array<uint8_t, 16> k_bytes{ { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
                                             0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98,
                                             0x76, 0x54, 0x32, 0x10 } };
    const std::array<char, 32>    k_expec{
           { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
             'B', 'C', 'D', 'E', 'F', 'F', 'E', 'D', 'C', 'B', 'A',
             '9', '8', '7', '6', '5', '4', '3', '2', '1', '0' }
    };
    // NOLINTBEGIN(*magic*)
    std::array<char, 33> outbuf33;
    std::array<char, 32> outbuf32;
    std::array<char, 16> outbuf16;
    // NOLINTEND(*magic*)
    for (size_t i = 0; i <= k_bytes.size(); ++i) {
        const std::span<const uint8_t> bytes{ k_bytes.data(), i };
        const std::string_view         expec{ k_expec.data(), i * 2U };

        ASSERT_EQ(tlo::to_hex_string(bytes, &outbuf33), expec);

        std::string_view res = tlo::to_hex_string(bytes, &outbuf32);
        // NOLINTNEXTLINE(*magic*)
        if (i >= 16) {
            ASSERT_TRUE(res.empty());
        }
        else {
            ASSERT_EQ(res, expec);
        }
        res = tlo::to_hex_string(bytes, &outbuf16);
        // NOLINTNEXTLINE(*magic*)
        if (i >= 8) {
            ASSERT_TRUE(res.empty());
        }
        else {
            ASSERT_EQ(res, expec);
        }
    }
}
