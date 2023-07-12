#include "gtest/gtest.h"

#include "src/sym/addr-range.h"

#include "src/util/algo.h"
#include "src/util/umap.h"
#include "src/util/vec.h"

#include <random>
#include <utility>

#include <stdio.h>
#include <string.h>


TEST(sym, addr_range_basic) {
    {
        const tlo::sym::addr_range_t ar{};

        ASSERT_TRUE(ar.valid());
        ASSERT_TRUE(ar.single());
        ASSERT_FALSE(ar.active());
    }

    {
        const tlo::sym::addr_range_t ar{ 10 };

        ASSERT_TRUE(ar.valid());
        ASSERT_TRUE(ar.single());
        ASSERT_TRUE(ar.active());
        ASSERT_EQ(ar.size(), 0U);
    }

    {
        const tlo::sym::addr_range_t ar{ 10, 12 };

        ASSERT_TRUE(ar.valid());
        ASSERT_FALSE(ar.single());
        ASSERT_TRUE(ar.active());
        ASSERT_EQ(ar.size(), 2U);
    }

    {
        const tlo::sym::addr_range_t ar{ 10, 8 };

        ASSERT_FALSE(ar.valid());
    }

    {
        const tlo::sym::addr_range_t ar{ 0, 8 };

        ASSERT_TRUE(ar.valid());
        ASSERT_FALSE(ar.single());
        ASSERT_FALSE(ar.active());
    }

    {
        const tlo::sym::addr_range_t ar{ 8, 8 };

        ASSERT_FALSE(ar.valid());
    }
}

TEST(sym, addr_range_contains_single) {
    {
        static constexpr uint64_t k_lb = 0x80;
        static constexpr uint64_t k_ub = 0x800;


        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains(k_lb));
        ASSERT_FALSE(ar.contains(k_ub));
        ASSERT_FALSE(ar.contains(k_lb - 1));
        ASSERT_FALSE(ar.contains(k_ub + 1));
        ASSERT_TRUE(ar.contains(k_ub - 1));
        ASSERT_FALSE(ar.contains(std::numeric_limits<uint64_t>::min()));
        ASSERT_FALSE(ar.contains(std::numeric_limits<uint64_t>::max()));
    }

    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains(k_lb));
        ASSERT_FALSE(ar.contains(k_ub));
        ASSERT_TRUE(ar.contains(k_ub - 1));
    }

    {
        static constexpr uint64_t k_lb =
            std::numeric_limits<uint64_t>::min() + 1;
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains(k_lb));
        ASSERT_FALSE(ar.contains(k_ub));
        ASSERT_FALSE(ar.contains(k_lb - 1));
        ASSERT_TRUE(ar.contains(k_ub - 1));
    }

    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub =
            std::numeric_limits<uint64_t>::max() - 1;

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains(k_lb));
        ASSERT_FALSE(ar.contains(k_ub));
        ASSERT_TRUE(ar.contains(k_ub - 1));
    }
}

TEST(sym, addr_range_contains_pair) {
    {
        static constexpr uint64_t k_lb = 0x80;
        static constexpr uint64_t k_ub = 0x800;


        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains({ k_lb, k_ub }));
        ASSERT_FALSE(ar.contains({ k_lb, k_ub + 1 }));
        ASSERT_TRUE(ar.contains({ k_lb, k_ub - 1 }));

        ASSERT_FALSE(ar.contains({ k_lb - 1, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub }));

        ASSERT_FALSE(ar.contains({ k_lb - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.contains({ k_lb + 1, k_ub + 1 }));

        ASSERT_FALSE(ar.contains({ k_lb - 1, k_ub - 1 }));
        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub - 1 }));

        ASSERT_FALSE(
            ar.contains({ k_lb, std::numeric_limits<uint64_t>::max() }));
        ASSERT_TRUE(ar.contains({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.contains({ k_ub - 1, k_ub }));
        ASSERT_FALSE(ar.contains({ 1U, k_ub }));
        ASSERT_FALSE(
            ar.contains({ std::numeric_limits<uint64_t>::min(), k_ub }));
        ASSERT_FALSE(
            ar.contains({ std::numeric_limits<uint64_t>::min(), k_lb }));

        ASSERT_TRUE(ar.contains({ k_lb, k_lb + 1 }));
        ASSERT_FALSE(ar.contains({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.contains({ k_lb - 1, k_lb }));
        ASSERT_TRUE(ar.contains({ k_ub - 1, k_ub }));
        ASSERT_FALSE(ar.contains({ k_ub - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.contains({ k_ub, k_ub + 1 }));
    }
    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains({ k_lb, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb, k_ub - 1 }));

        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub - 1 }));

        ASSERT_TRUE(ar.contains({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.contains({ k_ub - 1, k_ub }));
    }

    {
        static constexpr uint64_t k_lb =
            std::numeric_limits<uint64_t>::min() + 1;
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.contains({ k_lb, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb, k_ub - 1 }));

        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub - 1 }));

        ASSERT_FALSE(ar.contains({ k_lb - 1, k_ub }));
        ASSERT_FALSE(ar.contains({ k_lb - 1, k_ub - 1 }));

        ASSERT_TRUE(ar.contains({ k_lb, k_lb + 1 }));
        ASSERT_FALSE(ar.contains({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.contains({ k_lb - 1, k_lb }));
        ASSERT_TRUE(ar.contains({ k_ub - 1, k_ub }));
    }
    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub =
            std::numeric_limits<uint64_t>::max() - 1;

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_FALSE(ar.contains({ k_lb, k_ub + 1 }));
        ASSERT_TRUE(ar.contains({ k_lb, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb, k_ub - 1 }));

        ASSERT_TRUE(ar.contains({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.contains({ k_ub - 1, k_ub }));
        ASSERT_FALSE(ar.contains({ k_ub - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.contains({ k_ub, k_ub + 1 }));


        ASSERT_FALSE(ar.contains({ k_lb + 1, k_ub + 1 }));
        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.contains({ k_lb + 1, k_ub - 1 }));
    }
}


TEST(sym, addr_range_overlaps_with) {
    {
        static constexpr uint64_t k_lb = 0x80;
        static constexpr uint64_t k_ub = 0x800;


        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub - 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub + 1 }));
        ASSERT_TRUE(
            tlo::sym::addr_range_t::overlap(ar, { k_lb - 1, k_ub + 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub - 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_ub - 1 }));

        ASSERT_FALSE(ar.overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_FALSE(ar.overlaps_with({ k_ub, k_ub + 1 }));

        ASSERT_TRUE(ar.overlaps_with(
            { k_lb + 1, std::numeric_limits<uint64_t>::max() }));
        ASSERT_TRUE(ar.overlaps_with(
            { k_lb + 0, std::numeric_limits<uint64_t>::max() }));
        ASSERT_TRUE(tlo::sym::addr_range_t::overlap(
            ar, { k_lb - 1, std::numeric_limits<uint64_t>::max() }));
        ASSERT_FALSE(ar.overlaps_with(
            { k_ub + 1, std::numeric_limits<uint64_t>::max() }));
        ASSERT_FALSE(ar.overlaps_with(
            { k_ub + 0, std::numeric_limits<uint64_t>::max() }));
        ASSERT_TRUE(ar.overlaps_with(
            { k_ub - 1, std::numeric_limits<uint64_t>::max() }));

        ASSERT_TRUE(ar.overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_lb + 1 }));
        ASSERT_FALSE(ar.overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_lb + 0 }));
        ASSERT_FALSE(ar.overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_lb - 1 }));

        ASSERT_TRUE(tlo::sym::addr_range_t::overlap(
            ar, { std::numeric_limits<uint64_t>::min(), k_ub + 1 }));
        ASSERT_TRUE(ar.overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_ub + 0 }));
        ASSERT_TRUE(ar.overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_ub - 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.overlaps_with({ k_ub, k_ub + 1 }));
    }
    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub - 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub - 1 }));


        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));
    }

    {
        static constexpr uint64_t k_lb =
            std::numeric_limits<uint64_t>::min() + 1;
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub - 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub - 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_ub - 1 }));


        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));
    }

    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub =
            std::numeric_limits<uint64_t>::max() - 1;

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub - 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_ub + 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub - 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_lb + 1, k_ub + 1 }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_FALSE(ar.overlaps_with({ k_ub, k_ub + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));

        ASSERT_TRUE(ar.overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub }));
        ASSERT_TRUE(ar.overlaps_with({ k_ub - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.overlaps_with({ k_ub, k_ub + 1 }));
    }
}


TEST(sym, addr_range_partially_overlaps_with) {
    {
        static constexpr uint64_t k_lb = 0x80;
        static constexpr uint64_t k_ub = 0x800;


        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub }));

        ASSERT_FALSE(
            tlo::sym::addr_range_t::partially_overlap(ar, { k_lb, k_ub + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_FALSE(
            tlo::sym::addr_range_t::partially_overlap(ar, { k_lb - 1, k_ub }));

        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb + 1, k_ub + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb - 1, k_ub + 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub - 1 }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb - 1, k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub, k_ub + 1 }));

        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_ub - 1, k_ub + 1 }));


        ASSERT_TRUE(ar.partially_overlaps_with(
            { k_lb + 1, std::numeric_limits<uint64_t>::max() }));
        ASSERT_FALSE(tlo::sym::addr_range_t::partially_overlap(
            ar, { k_lb + 0, std::numeric_limits<uint64_t>::max() }));
        ASSERT_FALSE(ar.partially_overlaps_with(
            { k_lb - 1, std::numeric_limits<uint64_t>::max() }));
        ASSERT_FALSE(ar.partially_overlaps_with(
            { k_ub + 1, std::numeric_limits<uint64_t>::max() }));
        ASSERT_FALSE(ar.partially_overlaps_with(
            { k_ub + 0, std::numeric_limits<uint64_t>::max() }));
        ASSERT_TRUE(ar.partially_overlaps_with(
            { k_ub - 1, std::numeric_limits<uint64_t>::max() }));

        ASSERT_TRUE(ar.partially_overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_lb + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_lb + 0 }));
        ASSERT_FALSE(ar.partially_overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_lb - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_ub + 1 }));
        ASSERT_FALSE(tlo::sym::addr_range_t::partially_overlap(
            ar, { std::numeric_limits<uint64_t>::min(), k_ub + 0 }));
        ASSERT_TRUE(ar.partially_overlaps_with(
            { std::numeric_limits<uint64_t>::min(), k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub - 1, k_ub }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_ub - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub, k_ub + 1 }));
    }
    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub - 1, k_ub }));
    }

    {
        static constexpr uint64_t k_lb =
            std::numeric_limits<uint64_t>::min() + 1;
        static constexpr uint64_t k_ub = std::numeric_limits<uint64_t>::max();

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub - 1 }));

        ASSERT_FALSE(
            tlo::sym::addr_range_t::partially_overlap(ar, { k_lb - 1, k_ub }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb - 1, k_ub - 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_lb + 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb - 1, k_lb + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb - 1, k_lb }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub - 1, k_ub }));
    }

    {
        static constexpr uint64_t k_lb = std::numeric_limits<uint64_t>::min();
        static constexpr uint64_t k_ub =
            std::numeric_limits<uint64_t>::max() - 1;

        const tlo::sym::addr_range_t ar{ k_lb, k_ub };
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_ub - 1 }));
        ASSERT_FALSE(
            tlo::sym::addr_range_t::partially_overlap(ar, { k_lb, k_ub + 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb + 1, k_ub - 1 }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_lb + 1, k_ub + 1 }));

        ASSERT_FALSE(ar.partially_overlaps_with({ k_lb, k_lb + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub - 1, k_ub }));
        ASSERT_TRUE(ar.partially_overlaps_with({ k_ub - 1, k_ub + 1 }));
        ASSERT_FALSE(ar.partially_overlaps_with({ k_ub, k_ub + 1 }));
    }
}


TEST(sym, addr_range_sort_and_find_no_overlap) {
    static constexpr size_t k_nranges    = 10000;
    static constexpr size_t k_addr_start = 100;
    static constexpr size_t k_max_size   = 16;
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp)
    auto rng = std::default_random_engine{};
    for (int att = 0; att < 2; ++att) {
        tlo::vec_t<tlo::sym::addr_range_t> ranges{};
        tlo::vec_t<tlo::sym::addr_range_t> ranges_invalid{};

        size_t cur = k_addr_start;
        for (size_t i = 0; i < k_nranges; ++i) {
            const size_t lo = cur;
            const size_t hi =
                cur + (static_cast<size_t>(rng()) % k_max_size) + 1;
            cur = hi;
            if (att != 0 && i != (k_nranges - 1)) {
                const size_t skip_sz = static_cast<size_t>(rng()) % k_max_size;
                if (skip_sz != 0) {
                    ranges_invalid.emplace_back(cur, cur + skip_sz);
                    cur += skip_sz;
                }
            }
            ranges.emplace_back(lo, hi);
        }
        const size_t                       nranges = ranges.size();
        tlo::vec_t<tlo::sym::addr_range_t> copy    = ranges;

        std::shuffle(std::begin(copy), std::end(copy), rng);

        std::sort(std::begin(copy), std::end(copy),
                  tlo::sym::addr_range_t::cmp_t{});

        for (size_t i = 0; i < nranges; ++i) {
            ASSERT_TRUE(ranges[i].valid());
            ASSERT_TRUE(ranges[i].active());
            ASSERT_FALSE(ranges[i].single());
            ASSERT_TRUE(ranges[i].eq(copy[i]));
        }

        for (auto const & range : copy) {
            auto res = std::lower_bound(std::begin(ranges), std::end(ranges),
                                        range, tlo::sym::addr_range_t::cmp_t{});

            ASSERT_NE(res, std::end(ranges));
            ASSERT_TRUE(res->eq(range));

            res = tlo::sym::addr_range_t::find_closest(std::begin(ranges),
                                                       std::end(ranges), range);
            ASSERT_NE(res, std::end(ranges));
            ASSERT_TRUE(res->eq(range));
        }

        for (auto const & range : ranges_invalid) {
            auto res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges), std::end(ranges), range);

            ASSERT_EQ(res, std::end(ranges));
            res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges), std::end(ranges), range.lo_addr_inclusive_);

            ASSERT_EQ(res, std::end(ranges));
            res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges), std::end(ranges),
                range.hi_addr_inclusive());

            ASSERT_EQ(res, std::end(ranges));

            res = tlo::sym::addr_range_t::find_overlapping(
                std::begin(ranges), std::end(ranges), range);

            ASSERT_EQ(res, std::end(ranges));
        }

        for (auto const & range : ranges) {
            auto res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges_invalid), std::end(ranges_invalid), range);

            ASSERT_EQ(res, std::end(ranges_invalid));
            res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges_invalid), std::end(ranges_invalid),
                range.lo_addr_inclusive_);

            ASSERT_EQ(res, std::end(ranges_invalid));
            res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges_invalid), std::end(ranges_invalid),
                range.hi_addr_inclusive());

            ASSERT_EQ(res, std::end(ranges_invalid));

            res = tlo::sym::addr_range_t::find_overlapping(
                std::begin(ranges_invalid), std::end(ranges_invalid), range);

            ASSERT_EQ(res, std::end(ranges_invalid));
        }

        for (auto range_it = ranges_invalid.begin();
             range_it != ranges_invalid.end(); ++range_it) {
            if (range_it != ranges_invalid.begin()) {
                tlo::sym::addr_range_t range_m1 = *range_it;
                range_m1.lo_addr_inclusive_ -= 1;
                auto res = tlo::sym::addr_range_t::find_overlapping(
                    std::begin(ranges), std::end(ranges), range_m1);
                ASSERT_NE(res, std::end(ranges));
            }
            if (range_it != ranges_invalid.end()) {
                tlo::sym::addr_range_t range_m1 = *range_it;
                range_m1.hi_addr_exclusive_ += 1;
                auto res = tlo::sym::addr_range_t::find_overlapping(
                    std::begin(ranges), std::end(ranges), range_m1);
                ASSERT_NE(res, std::end(ranges));
            }
        }

        size_t cur_range_idx = 0;
        for (size_t i = 0; i < cur + k_addr_start; ++i) {
            auto res = std::begin(ranges);
            if ((i % 2) == 0) {
                res = tlo::sym::addr_range_t::find_closest(std::begin(ranges),
                                                           std::end(ranges), i);
            }
            else {
                res = tlo::sym::addr_range_t::find_closest(
                    std::begin(ranges), std::end(ranges),
                    tlo::sym::addr_range_t{ i });
            }

            const bool expec_not_found = i < k_addr_start || i >= cur;
            if (expec_not_found) {
                ASSERT_TRUE(res == std::end(ranges) || !res->contains(i));
            }
            else {
                if (ranges[cur_range_idx].lt(i)) {
                    ++cur_range_idx;
                }
                if (cur_range_idx > nranges) {
                    cur_range_idx = nranges;
                }

                if (ranges[cur_range_idx].contains(i)) {
                    ASSERT_NE(res, std::end(ranges));
                    ASSERT_TRUE(res->contains(i));
                }
                else {
                    ASSERT_EQ(att, 1);
                    ASSERT_EQ(res, std::end(ranges));
                    res = tlo::sym::addr_range_t::find_closest(
                        std::begin(ranges_invalid), std::end(ranges_invalid),
                        i);
                    ASSERT_NE(res, std::end(ranges));
                    ASSERT_TRUE(res->contains(i));
                }
            }
        }
    }
}
// NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if)
#if 0
// Fatally flawed
TEST(sym, addr_range_sort_and_find_some_overlap) {
    static constexpr size_t k_nranges    = 10000;
    static constexpr size_t k_addr_start = 100;
    for (int att = 0; att < 2; ++att) {
        tlo::vec_t<tlo::sym::addr_range_t> ranges{};
        tlo::vec_t<tlo::sym::addr_range_t> ranges_invalid{};

        size_t cur        = k_addr_start;
        size_t ranges_end = 0;
        auto   rng        = std::default_random_engine{};
        size_t min_sz     = 0;

        for (size_t i = 0; i < k_nranges; ++i) {
            static constexpr size_t k_size_mod = 16;
            static constexpr size_t k_max_size = 20;
            size_t                  sz;
            do {
                sz = static_cast<size_t>(rng() % k_size_mod) + k_max_size -
                     k_size_mod;
            } while (sz <= min_sz);

            size_t lo  = cur;
            size_t hi  = lo + sz;
            ranges_end = std::max(hi, ranges_end);
            switch (rng() & 3) {
                case 0:
                    cur = hi;
                    if (att && i != (k_nranges - 1)) {
                        size_t skip_sz = static_cast<size_t>(rng() & 0xf);
                        if (skip_sz != 0) {
                            ranges_invalid.emplace_back(cur, cur + skip_sz);
                            cur += skip_sz;
                        }
                    }
                    break;
                case 1:
                    cur = lo + sz / 4;
                    break;
                case 2:
                    cur = hi - sz / 4;
                    break;
                default:
                    if (sz < (k_max_size - 2)) {
                        cur    = lo;
                        min_sz = sz;
                    }
                    else {
                        cur = lo + 1;
                    }
                    break;
            }

            ranges.emplace_back(lo, hi);
            if (ranges.size() > 1) {
                assert((ranges.end() - 2)->lt(*(ranges.end() - 1)) ||
                       (ranges.end() - 2)->eq(*(ranges.end() - 1)));
            }
        }
        size_t                             nranges = ranges.size();
        tlo::vec_t<tlo::sym::addr_range_t> copy    = ranges;

        std::shuffle(std::begin(copy), std::end(copy),
                     std::default_random_engine{});

        std::sort(std::begin(copy), std::end(copy),
                  tlo::sym::addr_range_t::cmp_t{});

        for (size_t i = 0; i < nranges; ++i) {
            ASSERT_TRUE(ranges[i].valid());
            ASSERT_TRUE(ranges[i].active());
            ASSERT_FALSE(ranges[i].single());
            ASSERT_TRUE(ranges[i].eq(copy[i]));
        }

        for (auto const & range : copy) {
            auto res = std::lower_bound(std::begin(ranges), std::end(ranges),
                                        range, tlo::sym::addr_range_t::cmp_t{});

            ASSERT_NE(res, std::end(ranges));
            ASSERT_TRUE(res->eq(range));

            res = tlo::sym::addr_range_t::find_closest(std::begin(ranges),
                                                       std::end(ranges), range);

            ASSERT_NE(res, std::end(ranges));
            ASSERT_TRUE(res->eq(range));
        }

        for (auto const & range : ranges_invalid) {
            auto res = tlo::sym::addr_range_t::find_closest(
                std::begin(ranges), std::end(ranges), range);

            ASSERT_EQ(res, std::end(ranges));
        }

        for (auto range_it = ranges_invalid.begin();
             range_it != ranges_invalid.end(); ++range_it) {
            {
                auto res = tlo::sym::addr_range_t::find_overlapping(
                    std::begin(ranges), std::end(ranges), *range_it);
                ASSERT_EQ(res, std::end(ranges));
            }

            if (range_it != ranges_invalid.begin()) {
                tlo::sym::addr_range_t range_m1 = *range_it;
                range_m1.lo_addr_inclusive_ -= 1;
                auto res = tlo::sym::addr_range_t::find_overlapping(
                    std::begin(ranges), std::end(ranges), range_m1);
                ASSERT_NE(res, std::end(ranges));
            }
            if (range_it != ranges_invalid.end()) {
                tlo::sym::addr_range_t range_m1 = *range_it;
                range_m1.hi_addr_exclusive_ += 1;
                auto res = tlo::sym::addr_range_t::find_overlapping(
                    std::begin(ranges), std::end(ranges), range_m1);
                ASSERT_NE(res, std::end(ranges));
            }
        }


        for (size_t i = 0; i < ranges_end + k_addr_start; ++i) {
            auto res = std::begin(ranges);
            if (i % 2) {
                res = tlo::sym::addr_range_t::find_closest(std::begin(ranges),
                                                           std::end(ranges), i);
            }
            else {
                res = tlo::sym::addr_range_t::find_closest(
                    std::begin(ranges), std::end(ranges),
                    tlo::sym::addr_range_t{ i });
            }

            bool expec_not_found = i < k_addr_start || i >= ranges_end;
            if (expec_not_found) {
                ASSERT_TRUE(res == std::end(ranges) || !res->contains(i));
            }
            else {
                auto res2 = tlo::sym::addr_range_t::find_closest(
                    std::begin(ranges_invalid), std::end(ranges_invalid), i);
                if (res2 == std::end(ranges_invalid)) {
                    ASSERT_NE(res, std::end(ranges)) << i;
                    ASSERT_TRUE(res->contains(i)) << att;
                    tlo::sym::addr_range_t found          = *res;
                    auto                   possible_begin = res;
                    auto                   possible_end   = res;

                    while (possible_begin != std::begin(ranges)) {
                        auto next = possible_begin;
                        --next;
                        if (!next->contains(i)) {
                            break;
                        }
                        possible_begin = next;
                    }

                    while (possible_end != std::end(ranges) &&
                           possible_end->contains(i)) {
                        ++possible_end;
                    }
                    for (res = possible_begin; res != possible_end; ++res) {
                        ASSERT_TRUE(res->eq(found) ||
                                    res->lo_addr_inclusive_ <
                                        found.lo_addr_inclusive_ ||
                                    (res->lo_addr_inclusive_ ==
                                         found.lo_addr_inclusive_ &&
                                     res->hi_addr_exclusive_ >
                                         found.hi_addr_exclusive_));
                    }
                }
                else {
                    ASSERT_EQ(att, 1);
                    ASSERT_EQ(res, std::end(ranges));
                }
            }
        }
    }
}
#endif

TEST(sym, addr_range_clump_overlapping) {
    static constexpr size_t            k_nranges    = 10000;
    static constexpr size_t            k_addr_start = 100;
    static constexpr size_t            k_size_mod   = 16;
    static constexpr size_t            k_max_size   = 20;
    tlo::vec_t<tlo::sym::addr_range_t> ranges{};
    tlo::vec_t<tlo::sym::addr_range_t> ranges_invalid{};
    size_t                             cur        = k_addr_start;
    size_t                             ranges_end = 0;
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp)
    auto   rng    = std::default_random_engine{};
    size_t min_sz = 0;
    for (size_t i = 0; i < k_nranges; ++i) {
        size_t sz;
        do {
            sz = static_cast<size_t>(rng() % k_size_mod) + k_max_size -
                 k_size_mod;
        } while (sz <= min_sz);

        const size_t lo = cur;
        const size_t hi = lo + sz;
        ranges_end      = std::max(hi, ranges_end);
        min_sz          = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        switch (rng() % 5U) {
            case 0:
                cur = hi;
                break;
            case 1:
                cur = lo + sz / 4;
                break;
            case 2:
                cur = hi - sz / 4;
                break;
            case 3:
                if (sz < (k_max_size - 2)) {
                    min_sz = sz;
                    cur    = lo;
                }
                else {
                    cur = lo + 1;
                }
                break;
            default:
                ranges_invalid.emplace_back(ranges_end, ranges_end + sz);
                cur        = ranges_end + sz;
                ranges_end = cur;
        }
        ranges.emplace_back(lo, hi);
    }

    std::sort(std::begin(ranges), std::end(ranges),
              tlo::sym::addr_range_t::cmp_t{});
    tlo::vec_t<tlo::sym::addr_range_t> clumped = { ranges.front() };
    for (auto range : ranges) {
        if (tlo::sym::addr_range_t::overlap(clumped.back(), range)) {
            clumped.back().merge(range);
        }
        else {
            clumped.emplace_back(range);
        }
    }

    for (auto range : ranges) {
        auto res = tlo::sym::addr_range_t::find_overlapping(
            std::begin(clumped), std::end(clumped), range);
        ASSERT_NE(res, std::end(clumped));
        ASSERT_TRUE(res->contains(range));
    }

    for (auto range : ranges_invalid) {
        auto res = tlo::sym::addr_range_t::find_overlapping(
            std::begin(clumped), std::end(clumped), range);
        ASSERT_EQ(res, std::end(clumped));
    }

    for (auto range : clumped) {
        auto res = tlo::sym::addr_range_t::find_overlapping(
            std::begin(ranges), std::end(ranges), range);
        ASSERT_NE(res, std::end(ranges));
        ASSERT_TRUE(tlo::sym::addr_range_t::overlap(*res, range));
    }
}
