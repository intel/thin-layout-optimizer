#ifndef SRC_D_PERF_D_PERF_FILE_AND_SAVE_STATE_HELPER_H_
#define SRC_D_PERF_D_PERF_FILE_AND_SAVE_STATE_HELPER_H_


#include "src/cfg/cfg.h"
#include "src/sym/syms.h"

#include "src/util/umap.h"

static bool G_okay = false;  // NOLINT(*avoid-non-const-global-variables)

#define ROOT_PATH TLO_PROJECT_DIR "/tests/src/perf/test-inputs/test-root"


static void
init_env() {
    if (tlo::sym::dso_t::get_dso_root_path().empty()) {
        if (tlo::file_ops::is_dir(ROOT_PATH)) {
            tlo::sym::dso_t::set_dso_root_path(ROOT_PATH);
        }
        else {
            (void)fprintf(
                stderr,
                "Warning: Didn't find stored elf files for the test profile.\n"
                "Due to system differences, test may (likely will) fail.\n"
                "To generate the test directory run: `$> make-test-inputs.sh`\n");
        }
    }
}


static bool
lines_to_strings(const char *                          lines_begin,
                 const char *                          lines_end,
                 tlo::vec_t<char> *                    strs_base,
                 tlo::umap<std::string_view, size_t> * str_to_idx,
                 tlo::vec_t<std::string_view> *        strs) {
    if (strs_base == nullptr) {
        return false;
    }
    if ((str_to_idx == nullptr) == (strs == nullptr)) {
        return false;
    }

    const char * lines_last = nullptr;
    size_t       cnt        = 0;

    strs_base->clear();
    strs_base->reserve(static_cast<size_t>(lines_end - lines_begin));


    for (; lines_begin < lines_end; ++lines_begin) {
        if (*lines_begin == '\n') {
            if (lines_last == nullptr) {
                continue;
            }
            strs_base->push_back('\0');
            std::string_view sv{ lines_last, static_cast<size_t>(lines_begin -
                                                                 lines_last) };
            if (str_to_idx != nullptr) {
                str_to_idx->emplace(sv, cnt);
            }
            else {
                strs->emplace_back(sv);
            }
            ++cnt;
            lines_last = nullptr;
            continue;
        }

        if (lines_last == nullptr) {
            lines_last = lines_begin;
        }

        strs_base->push_back(*lines_begin);
    }
    return true;
}

static void
create_and_check_cg_order_impl(const tlo::vec_t<tlo::perf::perf_func_t> & funcs,
                               const tlo::vec_t<tlo::perf::perf_edge_t> & edges,
                               const char * expec_order_path,
                               double       hmean_bound) {
    G_okay = false;
    const tlo::cfg_t cg(funcs, edges);
    ASSERT_TRUE(cg.valid());

    tlo::vec_t<tlo::cfg_func_order_info_t> ordered_funcs;
    cg.order_nodes(tlo::cfg_t::order_algorithm::k_hfsort_c3, &ordered_funcs);

    ASSERT_EQ(ordered_funcs.size(), cg.num_nodes());


    tlo::vec_t<char> order_dump       = {};
    double           total_weight_in  = 0.0;
    double           total_weight_out = 0.0;
    ssize_t          prev_order       = -1L;
    for (const tlo::cfg_func_order_info_t foi : ordered_funcs) {
        ASSERT_TRUE(foi.valid());
        foi.fc_->label(&order_dump);
        order_dump.emplace_back('\n');
        total_weight_in += foi.weight_in_;
        total_weight_out += foi.weight_out_;
        ASSERT_GT(static_cast<ssize_t>(foi.order_), prev_order);
        prev_order = static_cast<ssize_t>(foi.order_);
    }

    ASSERT_NEAR(total_weight_in, 1.0, .01);
    ASSERT_NEAR(total_weight_out, 1.0, .01);

    tlo::file_ops::filebuf_t expec_buf =
        tlo::file_ops::readfile(expec_order_path);
    ASSERT_TRUE(expec_buf.active()) << expec_order_path;

    tlo::vec_t<char>                    order_cpy, expec_cpy;
    tlo::vec_t<std::string_view>        order_strs{};
    tlo::umap<std::string_view, size_t> expec_strs_to_pos{};
    double                              hmean = 0.0;
    size_t                              i, e;
    if (order_dump.size() != expec_buf.size()) {
        goto write_tmp_and_fail;
    }

    if (memcmp(order_dump.data(), expec_buf.data(), expec_buf.size()) == 0) {
        expec_buf.cleanup();
        G_okay = true;
        return;
    }


    if (!lines_to_strings(
            reinterpret_cast<const char *>(expec_buf.data()),
            reinterpret_cast<const char *>(expec_buf.data() + expec_buf.size()),
            &expec_cpy, &expec_strs_to_pos, nullptr)) {
        TLO_TRACE("Unable to extract expec strings");
        goto write_tmp_and_fail;
    }

    if (!lines_to_strings(order_dump.data(),
                          order_dump.data() + order_dump.size(), &order_cpy,
                          nullptr, &order_strs)) {
        fprintf(stderr, "Unable to extract order strings");
        goto write_tmp_and_fail;
    }
    if (order_strs.size() != expec_strs_to_pos.size()) {
        fprintf(stderr, "Mismatched number of strings");
        goto write_tmp_and_fail;
    }

    EXPECT_TRUE(!order_strs.empty());
    for (i = 0, e = order_strs.size(); i < e; ++i) {
        auto res = expec_strs_to_pos.find(order_strs[i]);
        if (res == expec_strs_to_pos.end()) {
            fprintf(stderr, "Bad line: %s", order_strs[i].data());
            goto write_tmp_and_fail;
        }
        size_t pos = res->second;

        double dist = static_cast<double>(pos - i);
        if (dist == 0) {
            continue;
        }
        dist *= dist;

        hmean += 1.0 / dist;
    }
    if (hmean != 0.0) {
        hmean = static_cast<double>(1.0) / hmean;
    }

    EXPECT_GT(hmean, static_cast<double>(0.0))
        << "Zero harmonic-means implies no difference...";

    if (hmean > hmean_bound) {
        fprintf(stderr, "difference too large: %lf > %lf\n", hmean,
                hmean_bound);
        goto write_tmp_and_fail;
    }


    expec_buf.cleanup();
    G_okay = true;
    return;

write_tmp_and_fail:
    std::array<char, 256> tmpfile;
    int                   fd = tlo::file_ops::new_tmpfile(&tmpfile);
    fprintf(stderr, "Order does not match: %s\n", expec_order_path);
    if (fd < 0 || tlo::file_ops::ensure_write(
                      fd, reinterpret_cast<uint8_t *>(order_dump.data()),
                      order_dump.size()) != order_dump.size()) {
        fprintf(stderr, "\tUnable to write true output\n");
    }
    else {
        fprintf(stderr, "\tSee true output at: %s\n", tmpfile.data());
    }
    if (fd > 0) {
        close(fd);
    }
    // Fail intentionally.
    ASSERT_FALSE(true);
}


static void
create_and_check_cg_order(tlo::vec_t<tlo::perf::perf_func_t> * funcs,
                          tlo::vec_t<tlo::perf::perf_edge_t> * edges,
                          const char *                         expec_order_path,
                          double hmean_bound = 0.0) {
    tlo::cfg_t::cfg_prepare(funcs, edges);

    const tlo::perf::perf_func_t * prev_func = nullptr;

    for (const auto & pfunc : *funcs) {
        if (prev_func != nullptr) {
            ASSERT_TRUE(prev_func->eq(pfunc) || prev_func->cmp(pfunc));
        }
        prev_func = &pfunc;
        ASSERT_TRUE(pfunc.valid());
        ASSERT_TRUE(pfunc.func_clump_->is_cg_ready());
    }

    const tlo::perf::perf_edge_t * prev_edge = nullptr;
    for (const auto & pedge : *edges) {
        if (prev_edge != nullptr) {
            ASSERT_TRUE(prev_edge->eq(pedge) || prev_edge->cmp(pedge));
        }
        prev_edge = &pedge;
        ASSERT_TRUE(pedge.valid());
        ASSERT_TRUE(pedge.from_->is_cg_ready());
        ASSERT_TRUE(pedge.to_->is_cg_ready());
    }

    create_and_check_cg_order_impl(*funcs, *edges, expec_order_path,
                                   hmean_bound);
}
#endif
