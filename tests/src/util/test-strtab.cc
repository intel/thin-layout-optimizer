#include "gtest/gtest.h"

#include "src/util/strtab.h"
#include "src/util/umap.h"

template<bool k_maybe_empty>
static void
basic_stab_test() {
    tlo::strtab_t<k_maybe_empty> stab{};

    tlo::strtab_ret_t str = stab.get(tlo::strbuf_t<>("Hello World!"));
    ASSERT_EQ(str.sview(), "Hello World!");
    ASSERT_TRUE(str.added());
    tlo::strtab_ret_t str2 = stab.get(tlo::strbuf_t<>("Hello World!"));
    ASSERT_EQ(str2.sview(), "Hello World!");
    ASSERT_FALSE(str2.added());

    ASSERT_EQ(str2.str(), str.str());
    ASSERT_EQ(str2.hash(), str.hash());
    ASSERT_EQ(str2.len(), str.len());

    tlo::fixed_str_t fstr  = str.fstr();
    tlo::fixed_str_t fstr2 = str.fstr();
    ASSERT_FALSE(fstr.status());
    ASSERT_FALSE(fstr2.status());
    ASSERT_TRUE(!memcmp(&fstr, &fstr2, sizeof(str)));

    ASSERT_EQ(stab.size(), 1U);

    str2 = stab.get(tlo::strbuf_t<>(""));
    ASSERT_EQ(str2.sview(), "");

    ASSERT_NE(str2.str(), str.str());
    ASSERT_NE(str2.len(), str.len());
    ASSERT_TRUE(str2.added());
    fstr2 = str2.fstr();
    ASSERT_FALSE(fstr2.status());

    str = stab.get(tlo::strbuf_t<>(""));
    ASSERT_EQ(str.sview(), "");

    ASSERT_EQ(str2.str(), str.str());
    ASSERT_EQ(str2.hash(), str.hash());
    ASSERT_EQ(str2.len(), str.len());
    ASSERT_FALSE(str.added());
    fstr = str.fstr();
    ASSERT_FALSE(fstr.status());
    ASSERT_TRUE(!memcmp(&fstr, &fstr2, sizeof(str)));

    ASSERT_EQ(stab.size(), 2U);
}

TEST(util, basic) {
    basic_stab_test<true>();
    basic_stab_test<false>();
}

TEST(util, strbuf_meta) {
    tlo::strtab_t<> stab{};
    // NOLINTNEXTLINE(*magic*)
    tlo::strbuf_t<4> sb =
        stab.template get_sbuf<4>(tlo::strbuf_t<>("Hello World!"), 3);
    ASSERT_EQ(sb.sview(), "Hello World!");
    // NOLINTNEXTLINE(*magic*)
    ASSERT_EQ(sb.extra(), 3U);

    // NOLINTNEXTLINE(*magic*)
    sb = stab.template get_sbuf<4>(tlo::strbuf_t<>("Hello World!"), 5);
    ASSERT_EQ(sb.sview(), "Hello World!");
    // NOLINTNEXTLINE(*magic*)
    ASSERT_EQ(sb.extra(), 5U);

    ASSERT_EQ(stab.size(), 1U);

    // NOLINTNEXTLINE(*magic*)
    tlo::strbuf_t<4> sb2 = stab.template get_sbuf<4>(sb);
    ASSERT_EQ(sb2.sview(), "Hello World!");
    // NOLINTNEXTLINE(*magic*)
    ASSERT_EQ(sb2.extra(), 5U);

    ASSERT_TRUE(!memcmp(&sb, &sb2, sizeof(sb)));

    ASSERT_EQ(stab.size(), 1U);

    ASSERT_EQ(stab.template get_sbuf(sb).sview(), "Hello World!");
    ASSERT_EQ(stab.size(), 1U);

    // NOLINTNEXTLINE(*magic*)
    const tlo::strbuf_t<5> sb3 = stab.template get_sbuf<5>(sb);
    ASSERT_EQ(sb3.sview(), "Hello World!");
    // NOLINTNEXTLINE(*magic*)
    ASSERT_EQ(sb3.extra(), 5U);

    // NOLINTNEXTLINE(*magic*)
    const tlo::strbuf_t<5> sb4 = stab.template get_sbuf<5>(sb, 9);
    ASSERT_EQ(sb4.sview(), "Hello World!");
    // NOLINTNEXTLINE(*magic*)
    ASSERT_EQ(sb4.extra(), 9U);

    ASSERT_EQ(stab.size(), 1U);

    tlo::strbuf_t<> sb5 = stab.get_sbuf("Hello World!");
    ASSERT_EQ(sb5.sview(), "Hello World!");
    ASSERT_EQ(stab.size(), 1U);

    tlo::strbuf_t<> sb6 = stab.get_sbuf(sb4);
    ASSERT_EQ(sb6.sview(), "Hello World!");
    ASSERT_EQ(stab.size(), 1U);

    ASSERT_TRUE(!memcmp(&sb5, &sb6, sizeof(sb5)));

    ASSERT_EQ(stab.get_sbuf("Hello World!!").sview(), "Hello World!!");
    ASSERT_EQ(stab.size(), 2U);
}
TEST(util, strbuf_many) {
    const std::string_view base_str = {
        "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-=_+[]{};:,<.>?~|"
    };
    tlo::uset<std::string_view> corr_set{};
    tlo::strtab_t<false>        stab0{};
    tlo::strtab_t<true>         stab1{};

    static constexpr size_t k_max_tests = static_cast<size_t>(2 * 1000 * 1000);
    const size_t            slen        = base_str.length();
    for (size_t i = 0; i < slen; ++i) {
        for (size_t j = i; j < slen; ++j) {
            const std::string_view slice = base_str.substr(i, (j - i));
            const bool             expec = corr_set.emplace(slice).second;
            ASSERT_EQ(stab0.get(slice).added(), expec);
            ASSERT_EQ(stab1.get(slice).added(), expec);
        }
        if (corr_set.size() > k_max_tests) {
            break;
        }
    }
    ASSERT_EQ(corr_set.size(), stab0.size());
    ASSERT_EQ(corr_set.size(), stab1.size());

    size_t iter_cnt = 0;
    for (const std::string_view slice : stab0.sviews()) {
        ASSERT_TRUE(corr_set.contains(slice));
        ++iter_cnt;
    }
    ASSERT_EQ(stab0.size(), iter_cnt);

    iter_cnt = 0;
    for (const std::string_view slice : stab1.sviews()) {
        ASSERT_TRUE(corr_set.contains(slice));
        ++iter_cnt;
    }
    ASSERT_EQ(stab1.size(), iter_cnt);

    for (const std::string_view slice : corr_set) {
        ASSERT_FALSE(stab0.get(slice).added());
        ASSERT_FALSE(stab1.get(slice).added());
    }

    ASSERT_EQ(corr_set.size(), stab0.size());
    ASSERT_EQ(corr_set.size(), stab1.size());
}
