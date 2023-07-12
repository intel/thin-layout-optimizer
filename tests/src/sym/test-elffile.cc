#include "gtest/gtest.h"

#include "src/sym/elffile.h"
#include "src/util/strtab.h"
#include "src/util/verbosity.h"

#include <charconv>
#include <string_view>
#include <utility>

#include <stdio.h>
#include <string.h>

#define TEST_INPUTS_FMT                                                        \
 TLO_PROJECT_DIR "/tests/src/sym/test-inputs/test-elffile-%s-inputs"


TEST(sym, elffile_validation) {

    char * buf     = nullptr;
    size_t buf_len = 0;
    TLO_DISABLE_WMISSING_BRACES
    const std::array<std::pair<char const *, bool>, 3> k_test_confs = {
        std::pair<char const *, bool>{ reinterpret_cast<const char *>("good"),
                                       true },
        std::pair<char const *, bool>{ reinterpret_cast<const char *>("bad"),
                                       false },
        std::pair<char const *, bool>{ reinterpret_cast<const char *>("buggy"),
                                       false }
    };
    TLO_REENABLE_WMISSING_BRACES

    for (auto conf : k_test_confs) {
        // NOLINTNEXTLINE(*magic*)
        std::array<char, 256> path;
        const int             sp_res =
            snprintf(path.data(), path.size(), TEST_INPUTS_FMT, conf.first);
        ASSERT_GT(sp_res, 0);
        ASSERT_LT(static_cast<size_t>(sp_res), path.size());
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        FILE * fp = fopen(path.data(), "r");
        ASSERT_NE(fp, nullptr);
        bool did_test = false;

        tlo::strtab_t<> func_tab{};

        for (;;) {
            const ssize_t res = getline(&buf, &buf_len, fp);
            if (res <= 0) {
                ASSERT_TRUE(feof(fp));
                break;
            }
            size_t ures = static_cast<size_t>(res);
            buf[--ures] = '\0';
            std::string_view input{ buf, ures };
            char *           elffile_path     = buf;
            size_t           expec_nfuncs     = 0;
            size_t           expec_nlinks     = 0;
            bool             has_extra_expecs = true;
            size_t           tok_idx          = 0;
            for (tok_idx = 0; tok_idx < 3; ++tok_idx) {
                const size_t pos = input.find('%');
                if (pos == std::string_view::npos) {
                    break;
                }

                if (tok_idx == 0) {
                    elffile_path[pos] = '\0';
                }
                else if (tok_idx == 1) {
                    if (std::from_chars(input.begin(), input.begin() + pos,
                                        expec_nfuncs)
                            .ec != std::errc()) {
                        has_extra_expecs = false;
                    }
                }
                else if (tok_idx == 2) {
                    if (std::from_chars(input.begin(), input.begin() + pos,
                                        expec_nlinks)
                            .ec != std::errc()) {
                        has_extra_expecs = false;
                    }
                }
                input = input.substr(pos + 1, input.size() - (pos + 1));
            }
            has_extra_expecs &= (tok_idx == 3);

            if (access(elffile_path, R_OK) != 0) {
                continue;
            }


            tlo::elf_file_t ef{};
            // NOTE: For the "buggy" elf files, these are files that properly
            // violate the elf standard. That moment we refuse to load them.
            // These also might be somewhat system/version specific.
            ASSERT_EQ(ef.init(elffile_path), conf.second) << elffile_path;
            if (conf.second) {
                std::span<tlo::sym::func_t>      funcs{};
                std::span<tlo::strbuf_t<>>       links{};
                tlo::basic_uset<tlo::strbuf_t<>> buildids{};
                ASSERT_EQ(ef.extract_functions(nullptr, &funcs, &links,
                                               &buildids, &func_tab),
                          conf.second)
                    << elffile_path;
                if (has_extra_expecs) {
                    ASSERT_EQ(funcs.size(), expec_nfuncs);
                    ASSERT_EQ(links.size(), expec_nlinks);
                }
                if (funcs.data() != nullptr) {
                    tlo::arr_free(funcs.data(), funcs.size());
                }
                if (links.data() != nullptr) {
                    tlo::arr_free(links.data(), links.size());
                }
            }
            ef.cleanup();
            did_test = true;
        }
        ASSERT_TRUE(did_test);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        (void)fclose(fp);
    }

    if (buf != nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        free(buf);
    }
}
