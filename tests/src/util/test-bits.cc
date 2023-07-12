#include "gtest/gtest.h"

#include "src/util/bits.h"

#include <stdint.h>

TEST(util, rol) {
    ASSERT_EQ(tlo::rol(static_cast<uint64_t>(0x4), 3), 0x20U);
    ASSERT_EQ(tlo::rol(0x81000001, 3), 0x800000cU);
}

TEST(util, ror) {
    ASSERT_EQ(tlo::ror(static_cast<uint64_t>(0x8), 3), 0x1U);
    ASSERT_EQ(tlo::ror(0x81000001, 3), 0x30200000U);
    ASSERT_EQ(tlo::ror(static_cast<uint64_t>(0x8), 67), 0x1U);
}
TEST(util, roundup) {
    ASSERT_EQ(tlo::roundup(10, 5), 10);
    ASSERT_EQ(tlo::roundup(9, 5), 10);
    ASSERT_EQ(tlo::roundup(9, 4), 12);
    ASSERT_EQ(tlo::roundup(8, 4), 8);
}

TEST(util, rounddown) {
    ASSERT_EQ(tlo::rounddown(10, 5), 10);
    ASSERT_EQ(tlo::rounddown(9, 5), 5);
    ASSERT_EQ(tlo::rounddown(9, 4), 8);
    ASSERT_EQ(tlo::rounddown(8, 4), 8);
}

TEST(util, clz) {
    ASSERT_EQ(tlo::clz(1), 31);
    ASSERT_EQ(tlo::clz(2), 30);
    ASSERT_EQ(tlo::clz(static_cast<uint64_t>(2)), 62U);
    ASSERT_EQ(tlo::clz(uint8_t(2)), 6);
    ASSERT_EQ(tlo::clz(static_cast<uint64_t>(1) << 60U), 3U);
    ASSERT_EQ(tlo::clz(4096U | (static_cast<uint64_t>(1) << 60U)), 3U);
}


TEST(util, log2_roundup) {
    ASSERT_EQ(tlo::log2_roundup(1), 1);
    ASSERT_EQ(tlo::log2_roundup(2), 2);
    ASSERT_EQ(tlo::log2_roundup(3), 2);
    ASSERT_EQ(tlo::log2_roundup(4), 3);
    ASSERT_EQ(tlo::log2_roundup(5), 3);
    ASSERT_EQ(tlo::log2_roundup(0x80000000), 32U);
    ASSERT_EQ(tlo::log2_roundup(0x80000001), 32U);
    ASSERT_EQ(tlo::log2_roundup(0x8fffffff), 32U);
    ASSERT_EQ(tlo::log2_roundup(0xffffffff), 32U);
    ASSERT_EQ(tlo::log2_roundup(0x7fffffff), 31);
}

TEST(util, next_p2) {
    ASSERT_EQ(tlo::next_p2(1), 1);
    ASSERT_EQ(tlo::next_p2(2), 2);
    ASSERT_EQ(tlo::next_p2(3), 4);
    ASSERT_EQ(tlo::next_p2(4), 4);
    ASSERT_EQ(tlo::next_p2(5), 8);
    ASSERT_EQ(tlo::next_p2(0x80000000), 0x80000000);
    ASSERT_EQ(static_cast<uint32_t>(tlo::next_p2(0x7fffffff)), 0x80000000);
}
