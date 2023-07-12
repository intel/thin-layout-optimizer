#include "gtest/gtest.h"

#include "tests/test-helpers/test-help.h"

#include "src/util/ascii-reader.h"
#include "src/util/bits.h"
#include "src/util/compressed-reader.h"
#include "src/util/file-ops.h"
#include "src/util/memory.h"
#include "src/util/reader.h"

#include "compressed-files-test-helper.h"

static size_t
ensure_fread(FILE * fp, uint8_t * buf, size_t nbytes) {
    size_t nr = 0;
    for (;;) {
        const size_t r = fread(reinterpret_cast<void *>(buf), 1, nbytes, fp);
        nr += r;
        if (r == 0) {
            assert(feof(fp) && "Internal IO Error!");
            return nr;
        }
        nbytes -= r;
        if (nbytes == 0) {
            return nr;
        }
        buf += r;
    }
}

template<typename T_reader_t>
void
run_reader_tests(const std::vector<test_pair_t> & tps) {
    for (const test_pair_t & tp : tps) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        FILE * fp = fopen(tp.ascii_path_.c_str(), "r");
        ASSERT_TRUE(fp);
        ASSERT_EQ(fseek(fp, 0L, SEEK_END), 0);
        // NOLINTNEXTLINE(google-runtime-int)
        const long fsz_l = ftell(fp);
        ASSERT_GT(fsz_l, 0);
        const size_t fsz = static_cast<size_t>(fsz_l);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        uint8_t * file_expec_bytes = reinterpret_cast<uint8_t *>(malloc(fsz));
        ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
        ASSERT_EQ(ensure_fread(fp, file_expec_bytes, fsz), fsz);


        T_reader_t   reader{};
        const size_t alloc_sz = tlo::roundup(fsz, tlo::k_page_size);
        uint8_t *    reader_bytes_base =
            reinterpret_cast<uint8_t *>(tlo::sys::getmem(alloc_sz));

        uint8_t * reader_page_base =
            reinterpret_cast<uint8_t *>(tlo::sys::getmem(tlo::k_page_size));


        for (const std::string & zst_path : tp.zst_paths_) {
            for (uint32_t j = 0; j < 2; ++j) {
                uint8_t * reader_bytes =
                    j ? reader_bytes_base
                      : (reader_bytes_base + (alloc_sz - fsz));
                ASSERT_TRUE(reader.init(zst_path.c_str()));
                ASSERT_TRUE(reader.active());
                ASSERT_EQ(reader.readn(reader_bytes, fsz), fsz);
                ASSERT_EQ(reader.readn(reader_bytes, fsz), 0U);
                ASSERT_EQ(reader.readn(reader_bytes, fsz), 0U);

                ASSERT_FALSE(memcmp(reader_bytes, file_expec_bytes, fsz));
            }


            for (uint32_t j = 0; j < 2; ++j) {
                ASSERT_TRUE(reader.init(zst_path.c_str()));
                ASSERT_TRUE(reader.active());
                size_t off = 0;
                for (;;) {
                    static constexpr size_t k_rd_chunk_sz = 256;
                    // NOLINTBEGIN(cert-msc30-c,cert-msc50-cpp,concurrency-mt-unsafe)
                    const size_t sz =
                        static_cast<size_t>(rand()) % k_rd_chunk_sz;
                    // NOLINTEND(cert-msc30-c,cert-msc50-cpp,concurrency-mt-unsafe)
                    uint8_t * reader_bytes =
                        j ? reader_page_base
                          : (reader_page_base + (k_rd_chunk_sz - sz));

                    const size_t r = reader.readn(reader_bytes, sz);
                    if (off == fsz) {
                        ASSERT_EQ(r, 0U);
                        break;
                    }
                    if ((off + sz) > fsz) {
                        ASSERT_EQ(r, fsz - off);
                    }
                    else {
                        ASSERT_EQ(r, sz);
                    }
                    ASSERT_FALSE(
                        memcmp(reader_bytes, file_expec_bytes + off, r));
                    off += r;
                }
                // NOLINTBEGIN(*magic*)
                uint8_t c = 0x12;
                ASSERT_TRUE(reader.active());
                ASSERT_EQ(reader.readn(&c, 1), 0U);
                ASSERT_EQ(c, 0x12);
                ASSERT_EQ(reader.readn(&c, 1), 0U);
                ASSERT_EQ(c, 0x12);
                // NOLINTEND(*magic*)
            }

            ASSERT_EQ(fseek(fp, 0, SEEK_SET), 0);
            uint8_t * reader_ln;
            char *    fp_ln;
            size_t    reader_sz, fp_sz;

            reader_ln = nullptr;
            fp_ln     = nullptr;
            reader_sz = 0;
            fp_sz     = 0;

            ASSERT_TRUE(reader.init(zst_path.c_str()));
            ASSERT_TRUE(reader.active());
            for (;;) {


                const size_t fp_res =
                    static_cast<size_t>(getline(&fp_ln, &fp_sz, fp));

                const size_t reader_res =
                    reader.readline(&reader_ln, &reader_sz);

                if (reader_res == 0) {
                    ASSERT_TRUE(feof(fp));
                    ASSERT_EQ(fp_res, static_cast<size_t>(-1));
                    break;
                }
                ASSERT_NE(reader_res, static_cast<size_t>(-1));
                ASSERT_EQ(fp_res, reader_res);

                ASSERT_FALSE(memcmp(reader_ln, fp_ln, reader_res));
            }
            uint8_t *    expec_reader_ln = reader_ln;
            const size_t expec_reader_sz = reader_sz;
            ASSERT_EQ(reader.readline(&reader_ln, &reader_sz), 0U);
            ASSERT_EQ(expec_reader_ln, reader_ln);
            ASSERT_EQ(expec_reader_sz, reader_sz);

            reader.free_line(reader_ln, reader_sz);
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            free(fp_ln);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        free(file_expec_bytes);
        tlo::sys::freemem(reader_bytes_base, alloc_sz);
        tlo::sys::freemem(reader_page_base, tlo::k_page_size);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        (void)fclose(fp);
        reader.cleanup();
    }
}


TEST(util, NO_ZSTD_DISABLED(creader_init)) {
    tlo::creader_t cr{};

    ASSERT_TRUE(access("does-not-exist", F_OK));
    ASSERT_FALSE(cr.active());
    cr.init("does-not-exist");
    ASSERT_FALSE(cr.active());
    cr.cleanup();
    ASSERT_FALSE(cr.active());
}


TEST(util, areader_init) {
    tlo::areader_t ar{};

    ASSERT_TRUE(access("does-not-exist", F_OK));
    ASSERT_FALSE(ar.active());
    ar.init("does-not-exist");
    ASSERT_FALSE(ar.active());
    ar.cleanup();
    ASSERT_FALSE(ar.active());
}


TEST(util, NO_ZSTD_DISABLED(creader_read)) {
    std::vector<test_pair_t> tests;
    collect_file_pairs(&tests);
    ASSERT_GT(tests.size(), 0U);
    run_reader_tests<tlo::creader_t>(tests);
}


TEST(util, areader_read) {
    std::vector<test_pair_t> tests;
    char const *             input_path = CINPUT_PATH;
    uint32_t                 cnt;
    for (cnt = 0; cnt < k_max_ctypes; ++cnt) {
        static constexpr int             k_file_path_sz = 512;
        std::array<char, k_file_path_sz> file_path;
        const int                        res =
            snprintf(file_path.data(), k_file_path_sz, "%s%u", input_path, cnt);
        ASSERT_LT(res, k_file_path_sz - 1);
        ASSERT_GT(res, 0);
        if (!tlo::file_ops::exists(file_path.data())) {
            continue;
        }
        tests.emplace_back(file_path.data());
        tests.back().add_path(file_path.data());
    }
    ASSERT_GT(tests.size(), 0U);
    run_reader_tests<tlo::areader_t>(tests);
}
