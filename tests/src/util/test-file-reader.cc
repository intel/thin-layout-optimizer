#include "gtest/gtest.h"

#include "tests/test-helpers/test-help.h"

#include "src/util/file-ops.h"
#include "src/util/file-reader.h"

#include "compressed-files-test-helper.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include <stdint.h>

TEST(util, NO_ZSTD_DISABLED(file_reader_init)) {
    tlo::file_reader_t fr;
    ASSERT_TRUE(fr.is_empty());
    fr.init("does-not-exist");
    ASSERT_TRUE(fr.is_ascii());
    ASSERT_EQ(fr.active(), tlo::file_ops::exists("does-not-exist"));
    fr.cleanup();
    ASSERT_TRUE(fr.is_empty());
    fr.init("does-not-exist.zst");
    ASSERT_TRUE(fr.is_zst());
    ASSERT_EQ(fr.active(), tlo::file_ops::exists("does-not-exist.zst"));
    fr.init("does-not-exist");
    ASSERT_TRUE(fr.is_ascii());
    ASSERT_EQ(fr.active(), tlo::file_ops::exists("does-not-exist"));
    fr.cleanup();
    ASSERT_TRUE(fr.is_empty());
}

TEST(util, NO_ZSTD_DISABLED(file_reader_readlines)) {
    std::vector<test_pair_t> tests;
    collect_file_pairs(&tests);
    ASSERT_GT(tests.size(), 0U);

    std::array<tlo::file_reader_t, 2> fr;
    for (const test_pair_t & tp : tests) {
        for (const std::string & zst_path : tp.zst_paths_) {
            for (uint32_t i = 0; i < 2; ++i) {
                const uint32_t ascii = i;
                const uint32_t zst   = 1 - i;
                fr[ascii].init(tp.ascii_path_);
                fr[zst].init(zst_path);

                ASSERT_TRUE(fr[ascii].is_ascii());
                ASSERT_TRUE(fr[zst].is_zst());

                ASSERT_TRUE(fr[ascii].active());
                ASSERT_TRUE(fr[zst].active());

                for (;;) {
                    std::string_view ares, zres;

                    ares = fr[ascii].nextline();
                    zres = fr[zst].nextline();

                    ASSERT_EQ(ares, zres);

                    if (ares.empty()) {
                        break;
                    }
                }
            }
        }
    }
    fr[0].cleanup();
    fr[1].cleanup();
}
