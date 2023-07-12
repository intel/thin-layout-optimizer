#ifndef SRC_D_UTIL_D_COMPRESSED_FILES_TEST_HELPER_H_
#define SRC_D_UTIL_D_COMPRESSED_FILES_TEST_HELPER_H_

#include "src/util/file-ops.h"

#include <string>
#include <vector>


#define CINPUT_PATH                                                            \
 TLO_PROJECT_DIR                                                               \
 "/tests/src/util/test-inputs/generated-test-compressed-reader-input-c"

static constexpr size_t k_max_clevels = 30;
static constexpr size_t k_max_ctypes  = 20;

struct test_pair_t {
    std::string              ascii_path_;
    std::vector<std::string> zst_paths_;

    template<typename... Ts_t>
    test_pair_t(Ts_t &&... ts)
        : ascii_path_(std::forward<Ts_t>(ts)...), zst_paths_({}) {}

    template<typename... Ts_t>
    void
    add_path(Ts_t &&... ts) {
        zst_paths_.emplace_back(std::forward<Ts_t>(ts)...);
    }
};

static void
collect_file_pairs(std::vector<test_pair_t> * tests) {
    char const * input_path = CINPUT_PATH;
    uint32_t     cnt, inp;
    for (cnt = 0; cnt < k_max_ctypes; ++cnt) {
        static constexpr int             k_file_path_sz = 512;
        std::array<char, k_file_path_sz> file_path;
        int                              res =
            snprintf(file_path.data(), k_file_path_sz, "%s%u", input_path, cnt);
        ASSERT_LT(res, k_file_path_sz - 1);
        ASSERT_GT(res, 0);
        if (!tlo::file_ops::exists(file_path.data())) {
            continue;
        }
        tests->emplace_back(file_path.data());
        for (inp = 1; inp < k_max_clevels; ++inp) {
            res = snprintf(file_path.data(), k_file_path_sz, "%s%u.zst.%u",
                           input_path, cnt, inp);
            ASSERT_LT(res, k_file_path_sz - 1);
            ASSERT_GT(res, 0);
            if (!tlo::file_ops::exists(file_path.data())) {
                continue;
            }
            tests->back().add_path(file_path.data());
        }
        ASSERT_GT(tests->back().zst_paths_.size(), 0u);
    }
    if (tests->size() == 0u) {
        fprintf(stderr,
                "!! No test files found. !!\n"
                "!! Maybe run: scripts/gen-compression-tests.py\n");
    }
}

#endif
