#include "gtest/gtest.h"

#include "src/perf/perf-parse.h"

#include <string>


TEST(perf, parse_simple_line) {
    tlo::perf::simple_sample_t sample;
    std::string                test_s =
        "abc/123 t 3412366/3412367 1114875.029688:             7f996cd1aaff (/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))";
    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "abc/123 t");
    ASSERT_EQ(sample.hdr_.pid_, 3412366U);
    ASSERT_EQ(sample.hdr_.tid_, 3412367U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("/usr/lib/x86_64-linux-gnu/libc.so.6"),
              sample.loc_.dso_.sview());

    test_s += '\n';
    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "abc/123 t");
    ASSERT_EQ(sample.hdr_.pid_, 3412366U);
    ASSERT_EQ(sample.hdr_.tid_, 3412367U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("/usr/lib/x86_64-linux-gnu/libc.so.6"),
              sample.loc_.dso_.sview());


    test_s += "\nABCDEF";
    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "abc/123 t");
    ASSERT_EQ(sample.hdr_.pid_, 3412366U);
    ASSERT_EQ(sample.hdr_.tid_, 3412367U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("/usr/lib/x86_64-linux-gnu/libc.so.6"),
              sample.loc_.dso_.sview());

    test_s =
        "abcdef 3412/3412 1114875.029688:             7f996cd1aaff (/usr/lib/x86_64-linux-gnu/libc.so.6)";
    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "abcdef");
    ASSERT_EQ(sample.hdr_.pid_, 3412U);
    ASSERT_EQ(sample.hdr_.tid_, 3412U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("/usr/lib/x86_64-linux-gnu/libc.so.6"),
              sample.loc_.dso_.sview());

    test_s =
        "123 3412/3412 1114875.029688:             7f996cd1aaff ([unknown])";
    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "123");
    ASSERT_EQ(sample.hdr_.pid_, 3412U);
    ASSERT_EQ(sample.hdr_.tid_, 3412U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("[unknown]"), sample.loc_.dso_.sview());

    test_s =
        "123 3412/3412 1114875.029688:             7f996cd1aaff ([unknown])          ";
    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "123");
    ASSERT_EQ(sample.hdr_.pid_, 3412U);
    ASSERT_EQ(sample.hdr_.tid_, 3412U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("[unknown]"), sample.loc_.dso_.sview());

    test_s += "\nQQQ";

    ASSERT_EQ(
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample),
        tlo::perf::k_parse_done);

    ASSERT_EQ(sample.hdr_.comm_.sview(), "123");
    ASSERT_EQ(sample.hdr_.pid_, 3412U);
    ASSERT_EQ(sample.hdr_.tid_, 3412U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(1114875) << 32U) + uint64_t(29688));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f996cd1aaffUL);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
    ASSERT_EQ(std::string_view("[unknown]"), sample.loc_.dso_.sview());
}

TEST(perf, parse_lbr_line_new) {
    size_t                  res{};
    std::string             test_s{};
    tlo::perf::lbr_sample_t sample{};
    std::array<std::string, tlo::perf::lbr_sample_t::k_max_lbr_samples> k_test_lines = {
        { "0x7f6db26516df(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db265159f(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6db26543c8(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db26516dd(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/5/",
          "0x7f6db2654341(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db26543b3(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db2654556(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2654320(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6db26516d8(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2654550(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6dc81aeffa (/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))/0x7f6db26516d4 (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/2//-",
          "0x7f6dc81aefc3 (/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))/0x7f6dc81aefe6(/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))/P/-/-/1/",
          "0x7f6dc81aef8b(/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))/0x7f6dc81aefc0(/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))/P/-/-/1/",
          "0x7f6db2092674(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6dc81aef80(/usr/lib/x86_64-linux-gnu/libc.so.6 (deleted))/P/-/-/1/",
          "0x7f6db2654919(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2092670(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/5/",
          "0x7f6db2654656(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db26548f2(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/2/",
          "0x7f6db265460a(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2654618(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/16/",
          "0x7f6db26548ed(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2654580(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db26516cf(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db26548c0 (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3//-",
          "0x7f6db265157f(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2651690(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db23c2b38(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2651540(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db239129a(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2ae7(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/2/",
          "0x7f6db23911fe(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2391290(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/10/",
          "0x7f6db23c2ae2(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23911d0 (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/2//-",
          "0x7f6db23c2a6f (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2ad0(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6db2185e86(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2a64(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db21853b8(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2185e5b(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db2185e56(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2185390(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db23c2a5f(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2185e20(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6db2185c84(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2a59(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db2185308(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2185c59(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3//-",
          "0x7f6db2185c54(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db21852e0(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/3/",
          "0x7f6db23c2a54(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db2185c20(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6db23c2b92(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2a4e(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/1/",
          "0x7f6db23c2a48(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2b88(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/4/",
          "0x7f6db23c2e33 (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2a00(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/2//-",
          "0x7f6db21684b1(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/0x7f6db23c2e20(/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)/P/-/-/2/" }
    };

    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    static const uint64_t k_expec_mapped_addrs_from[] = {
        0x7f6db21684b1, 0x7f6db23c2e33, 0x7f6db23c2a48, 0x7f6db23c2b92,
        0x7f6db23c2a54, 0x7f6db2185c54, 0x7f6db2185308, 0x7f6db2185c84,
        0x7f6db23c2a5f, 0x7f6db2185e56, 0x7f6db21853b8, 0x7f6db2185e86,
        0x7f6db23c2a6f, 0x7f6db23c2ae2, 0x7f6db23911fe, 0x7f6db239129a,
        0x7f6db23c2b38, 0x7f6db265157f, 0x7f6db26516cf, 0x7f6db26548ed,
        0x7f6db265460a, 0x7f6db2654656, 0x7f6db2654919, 0x7f6db2092674,
        0x7f6dc81aef8b, 0x7f6dc81aefc3, 0x7f6dc81aeffa, 0x7f6db26516d8,
        0x7f6db2654556, 0x7f6db2654341, 0x7f6db26543c8, 0x7f6db26516df,
    };

    static const uint64_t k_expec_mapped_addrs_to[] = {
        0x7f6db23c2e20, 0x7f6db23c2a00, 0x7f6db23c2b88, 0x7f6db23c2a4e,
        0x7f6db2185c20, 0x7f6db21852e0, 0x7f6db2185c59, 0x7f6db23c2a59,
        0x7f6db2185e20, 0x7f6db2185390, 0x7f6db2185e5b, 0x7f6db23c2a64,
        0x7f6db23c2ad0, 0x7f6db23911d0, 0x7f6db2391290, 0x7f6db23c2ae7,
        0x7f6db2651540, 0x7f6db2651690, 0x7f6db26548c0, 0x7f6db2654580,
        0x7f6db2654618, 0x7f6db26548f2, 0x7f6db2092670, 0x7f6dc81aef80,
        0x7f6dc81aefc0, 0x7f6dc81aefe6, 0x7f6db26516d4, 0x7f6db2654550,
        0x7f6db2654320, 0x7f6db26543b3, 0x7f6db26516dd, 0x7f6db265159f,
    };


    static const std::string_view k_expec_dsos_from[] = {
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
    };
    static const std::string_view k_expec_dsos_to[] = {
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/libc.so.6",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",
        "/usr/lib/x86_64-linux-gnu/dri/iris_dri.so",

    };
    static const uint32_t k_expec_cycles[] = {
        2, 2, 4, 1, 1,  3, 3, 3, 1, 3, 3, 3, 1, 2, 10, 2,
        3, 3, 3, 3, 16, 2, 5, 1, 1, 1, 2, 1, 1, 3, 5,  1,
    };
    static const uint32_t k_expec_predicts[] = {
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,
        tlo::perf::lbr_br_sample_t::k_pred, tlo::perf::lbr_br_sample_t::k_pred,

    };

    static const bool k_expec_in_txs[] = {
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
    };
    static const bool k_expec_aborts[] = {
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
    };

    // NOLINTEND(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)

    static_assert(sizeof(k_expec_mapped_addrs_from) /
                      sizeof(k_expec_mapped_addrs_from[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);
    static_assert(sizeof(k_expec_dsos_from) / sizeof(k_expec_dsos_from[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);

    static_assert(sizeof(k_expec_mapped_addrs_to) /
                      sizeof(k_expec_mapped_addrs_to[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);
    static_assert(sizeof(k_expec_dsos_to) / sizeof(k_expec_dsos_to[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);

    static_assert(sizeof(k_expec_cycles) / sizeof(k_expec_cycles[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);
    static_assert(sizeof(k_expec_predicts) / sizeof(k_expec_predicts[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);
    static_assert(sizeof(k_expec_in_txs) / sizeof(k_expec_in_txs[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);
    static_assert(sizeof(k_expec_aborts) / sizeof(k_expec_aborts[0]) ==
                  tlo::perf::lbr_sample_t::k_max_lbr_samples);

    for (uint32_t nsamples = 0;
         nsamples < tlo::perf::lbr_sample_t::k_max_lbr_samples; ++nsamples) {
        for (uint32_t att = 0; att < 2; ++att) {
            test_s =
                "Xwayland  3346/3346  472467.383778:      7f6db265163d (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so) ";
            for (uint32_t i = 0; i < nsamples; ++i) {
                test_s +=
                    k_test_lines[tlo::perf::lbr_sample_t::k_max_lbr_samples -
                                 nsamples + i];
                if ((i + 1) < nsamples) {
                    test_s += " ";
                }
            }
            if (att != 0) {
                test_s += " ";
            }
            test_s += "\n";


            // Xwayland  3346/3346  472467.383778:      7f6db265163d
            // (/usr/lib/x86_64-linux-gnu/dri/iris_dri.so)
            res = tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(),
                                               &sample);
            ASSERT_NE(res, tlo::perf::k_parse_error);
            ASSERT_NE(res, tlo::perf::k_parse_incomplete);


            ASSERT_EQ(sample.hdr_.comm_.sview(), "Xwayland");
            ASSERT_EQ(sample.hdr_.pid_, 3346U);
            ASSERT_EQ(sample.hdr_.tid_, 3346U);
            ASSERT_EQ(sample.hdr_.timestamp_,
                      (uint64_t(472467) << 32U) + uint64_t(383778));
            ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f6db265163dUL);
            ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
            ASSERT_EQ(
                std::string_view("/usr/lib/x86_64-linux-gnu/dri/iris_dri.so"),
                sample.loc_.dso_.sview());

            if (nsamples == 0) {
                ASSERT_EQ(res, tlo::perf::k_parse_done);
                continue;
            }
            ASSERT_NE(res, tlo::perf::k_parse_done);
            ASSERT_EQ(tlo::perf::parse_lbr_line(test_s.c_str(), test_s.length(),
                                                res, &sample),
                      tlo::perf::k_parse_done);

            ASSERT_EQ(sample.num_lbr_samples(), nsamples);

            ASSERT_TRUE(sample.valid());

            for (uint32_t i = 0; i < sample.num_lbr_samples(); ++i) {
                ASSERT_EQ(sample.samples_[i].from_.mapped_addr_,
                          k_expec_mapped_addrs_from[i]);
                ASSERT_EQ(sample.samples_[i].from_.unmapped_addr_, 0);
                ASSERT_EQ(sample.samples_[i].from_.dso_.sview(),
                          k_expec_dsos_from[i]);


                ASSERT_EQ(sample.samples_[i].to_.mapped_addr_,
                          k_expec_mapped_addrs_to[i]);
                ASSERT_EQ(sample.samples_[i].from_.unmapped_addr_, 0);
                ASSERT_EQ(sample.samples_[i].to_.dso_.sview(),
                          k_expec_dsos_to[i]);

                ASSERT_EQ(sample.samples_[i].cycles_, k_expec_cycles[i]);
                ASSERT_EQ(sample.samples_[i].predicted_, k_expec_predicts[i]);
                ASSERT_EQ(sample.samples_[i].in_tx_, k_expec_in_txs[i]);
                ASSERT_EQ(sample.samples_[i].aborted_, k_expec_aborts[i]);
            }


            res = tlo::perf::parse_sample_line(test_s.c_str(),
                                               test_s.length() - 1, &sample);
            ASSERT_NE(res, tlo::perf::k_parse_done);
            ASSERT_NE(res, tlo::perf::k_parse_error);
            ASSERT_NE(res, tlo::perf::k_parse_incomplete);

            ASSERT_EQ(sample.hdr_.comm_.sview(), "Xwayland");
            ASSERT_EQ(sample.hdr_.pid_, 3346U);
            ASSERT_EQ(sample.hdr_.tid_, 3346U);
            ASSERT_EQ(sample.hdr_.timestamp_,
                      (uint64_t(472467) << 32U) + uint64_t(383778));
            ASSERT_EQ(sample.loc_.mapped_addr_, 0x7f6db265163dUL);
            ASSERT_EQ(sample.loc_.unmapped_addr_, 0);
            ASSERT_EQ(
                std::string_view("/usr/lib/x86_64-linux-gnu/dri/iris_dri.so"),
                sample.loc_.dso_.sview());


            ASSERT_EQ(tlo::perf::parse_lbr_line(test_s.c_str(), test_s.length(),
                                                res, &sample),
                      tlo::perf::k_parse_done);
            ASSERT_EQ(sample.num_lbr_samples(), nsamples);
            ASSERT_TRUE(sample.valid());
            for (uint32_t i = 0; i < sample.num_lbr_samples(); ++i) {
                ASSERT_EQ(sample.samples_[i].from_.mapped_addr_,
                          k_expec_mapped_addrs_from[i]);
                ASSERT_EQ(sample.samples_[i].from_.unmapped_addr_, 0);
                ASSERT_EQ(sample.samples_[i].from_.dso_.sview(),
                          k_expec_dsos_from[i]);


                ASSERT_EQ(sample.samples_[i].to_.mapped_addr_,
                          k_expec_mapped_addrs_to[i]);
                ASSERT_EQ(sample.samples_[i].from_.unmapped_addr_, 0);
                ASSERT_EQ(sample.samples_[i].to_.dso_.sview(),
                          k_expec_dsos_to[i]);

                ASSERT_EQ(sample.samples_[i].cycles_, k_expec_cycles[i]);
                ASSERT_EQ(sample.samples_[i].predicted_, k_expec_predicts[i]);
                ASSERT_EQ(sample.samples_[i].in_tx_, k_expec_in_txs[i]);
                ASSERT_EQ(sample.samples_[i].aborted_, k_expec_aborts[i]);
            }
        }
    }
    test_s =
        "            perf 2920544/2920544 472467.383545:      55877acea540 (/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf) 0x55877ae9b1ce(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877acea540(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/31/  0x55877ae9b1ab(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b1b4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/47/  0x55877ae9a0d6(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b198(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/2/  0x55877ae9a083(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a0d5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/102/  0x55877ae9a0d0(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a03c(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/28/  0x55877ae9a0b2(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a0bb(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/16/  0x55877ae9b193(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a084(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/245/  0x55877ae9b20f(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b158(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/27/  0x55877ae9b23c(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b1f8(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/1/  0x55877ae9a0e8(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b239(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/282/  0x55877ae9b234(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a0d7(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/15/  0x55877ae9b1f6(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b226(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/89/  0x55877ae9b3a7(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b1d5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/112/  0x55877ad8a78b(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b37e(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/12/  0x55877ad82885(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad8a760(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/326/  0x55877ad81a34(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad82848(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/32/  0x55877ad819d4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad81a20(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/1/  0x55877ae995cd(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad819ce(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/1/  0x55877ae995a2(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995cc(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/48/  0x55877ae995c5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99570(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/32/  0x55877ae995ac(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995b6(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/28/  0x55877ae9959d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995a4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/48/  0x55877ae995c5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99570(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/14/  0x55877ae995b4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995bf(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/27/  0x55877ae9959d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995a4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/109/  0x55877ae995c5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99570(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/8/  0x55877ae9956e(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995bf(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/179/  0x55877ae99554(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9955d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/9/  0x55877ad819c9(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99540(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/302/  0x55877ad81a1b(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad819c2(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/190/  0x55877adfe446(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad81a14(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/3/  0x55877adfe485(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877adfe42d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/660/";

    res =
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample);
    ASSERT_NE(res, tlo::perf::k_parse_done);
    ASSERT_NE(res, tlo::perf::k_parse_error);
    ASSERT_NE(res, tlo::perf::k_parse_incomplete);
    ASSERT_EQ(sample.hdr_.comm_.sview(), "perf");

    ASSERT_EQ(sample.hdr_.pid_, 2920544U);
    ASSERT_EQ(sample.hdr_.tid_, 2920544U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(472467) << 32U) + uint64_t(383545));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x55877acea540);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0U);
    ASSERT_EQ(std::string_view("/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf"),
              sample.loc_.dso_.sview());

    ASSERT_EQ(tlo::perf::parse_lbr_line(test_s.c_str(), test_s.length(), res,
                                        &sample),
              tlo::perf::k_parse_done);
    ASSERT_TRUE(sample.valid());

    test_s =
        "            perf 2920544/2920544 472467.383545:      55877acea540 (/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf) 0x55877ae9b1ce(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877acea540(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/31/  0x55877ae9b1ab(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b1b4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/47/  0x55877ae9a0d6(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b198(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/2/  0x55877ae9a083(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a0d5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/102/  0x55877ae9a0d0(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a03c(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/28/  0x55877ae9a0b2(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a0bb(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/16/  0x55877ae9b193(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a084(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/245/  0x55877ae9b20f(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b158(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/27/  0x55877ae9b23c(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b1f8(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/1/  0x55877ae9a0e8(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b239(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/282/  0x55877ae9b234(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9a0d7(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/15/  0x55877ae9b1f6(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b226(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/89/  0x55877ae9b3a7(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b1d5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/112/  0x55877ad8a78b(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9b37e(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/12/  0x55877ad82885(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad8a760(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/326/  0x55877ad81a34(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad82848(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/32/  0x55877ad819d4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad81a20(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/1/  0x55877ae995cd(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad819ce(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/1/  0x55877ae995a2(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995cc(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/48/  0x55877ae995c5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99570(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/32/  0x55877ae995ac(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995b6(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/28/  0x55877ae9959d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995a4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/48/  0x55877ae995c5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99570(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/14/  0x55877ae995b4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995bf(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/27/  0x55877ae9959d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995a4(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/109/  0x55877ae995c5(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99570(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/8/  0x55877ae9956e(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae995bf(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/179/  0x55877ae99554(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae9955d(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/9/  0x55877ad819c9(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ae99540(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/302/  0x55877ad81a1b(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad819c2(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/P/-/-/190/  0x55877adfe446(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/0x55877ad81a14(/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf)/M/-/-/3/";

    res =
        tlo::perf::parse_sample_line(test_s.c_str(), test_s.length(), &sample);
    ASSERT_NE(res, tlo::perf::k_parse_done);
    ASSERT_NE(res, tlo::perf::k_parse_error);
    ASSERT_NE(res, tlo::perf::k_parse_incomplete);
    ASSERT_EQ(sample.hdr_.comm_.sview(), "perf");

    ASSERT_EQ(sample.hdr_.pid_, 2920544U);
    ASSERT_EQ(sample.hdr_.tid_, 2920544U);
    ASSERT_EQ(sample.hdr_.timestamp_,
              (uint64_t(472467) << 32U) + uint64_t(383545));
    ASSERT_EQ(sample.loc_.mapped_addr_, 0x55877acea540);
    ASSERT_EQ(sample.loc_.unmapped_addr_, 0U);
    ASSERT_EQ(std::string_view("/usr/lib/linux-hwe-6.2-tools-6.2.0-33/perf"),
              sample.loc_.dso_.sview());

    ASSERT_EQ(tlo::perf::parse_lbr_line(test_s.c_str(), test_s.length(), res,
                                        &sample),
              tlo::perf::k_parse_done);
    ASSERT_EQ(sample.num_lbr_samples(), 31U);
    ASSERT_TRUE(sample.valid());
}
