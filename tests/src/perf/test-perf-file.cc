#include "src/util/algo.h"

#include "gtest/gtest.h"

#include "src/perf/perf-file.h"
#include "src/perf/perf-saver.h"
#include "src/perf/perf-stats.h"

#include "src/util/file-ops.h"
#include "src/util/file-reader.h"
#include "src/util/verbosity.h"

#include <string>

#include "perf-file-and-save-state-helper.h"


#define INPUT_PATH                                                             \
 TLO_PROJECT_DIR "/tests/src/perf/test-inputs/test-perf-file-input"


#define SAVE_PATH_FMT                                                          \
 TLO_PROJECT_DIR                                                               \
 "/tests/src/perf/test-inputs/saved-state"

#define ORDER_PATH_FMT                                                         \
 TLO_PROJECT_DIR                                                               \
 "/tests/src/perf/test-inputs/expec-order-out-c3"

static void
collect_and_check_perf_file(tlo::perf::perf_stats_t * stats) {
    G_okay = false;
    tlo::file_reader_t        fr_events, fr_mmap;
    tlo::preader_t::cmdline_t cmdline;
    ASSERT_TRUE(tlo::perf::create_perf_events_cmdline(INPUT_PATH, &cmdline));
    fr_events.init(cmdline.data());
    ASSERT_TRUE(fr_events.active());
    ASSERT_TRUE(fr_events.is_process());

    ASSERT_TRUE(tlo::perf::create_perf_info_cmdline(INPUT_PATH, &cmdline));
    fr_mmap.init(cmdline.data());
    ASSERT_TRUE(fr_mmap.active());
    ASSERT_TRUE(fr_mmap.is_process());
    ASSERT_TRUE(tlo::perf::collect_perf_file_info(&fr_mmap, stats));
    ASSERT_TRUE(tlo::perf::collect_perf_file_events(&fr_events, stats));
    fr_mmap.cleanup();
    fr_events.cleanup();

    const bool valid = stats->valid();
    if (!valid) {
        stats->dump();
    }
    ASSERT_TRUE(valid);

    EXPECT_EQ(stats->agr_func_stats_.num_samples_, 65415UL);
    EXPECT_EQ(stats->agr_func_stats_.num_tracked_br_samples_in_, 0UL);
    EXPECT_EQ(stats->agr_func_stats_.num_tracked_br_samples_out_, 0UL);
    EXPECT_EQ(stats->agr_func_stats_.num_br_samples_in_, 2326682UL);
    EXPECT_EQ(stats->agr_func_stats_.num_br_samples_out_, 2326682UL);
    EXPECT_EQ(stats->agr_edge_stats_.num_edges_, 2326682UL);

    G_okay = true;
}

static void
collect_and_check_funcs_and_edges(
    const tlo::perf::perf_stats_t &      stats,
    tlo::vec_t<tlo::perf::perf_func_t> * funcs_out,
    tlo::vec_t<tlo::perf::perf_edge_t> * edges_out,
    bool                                 unknowns) {

    G_okay = false;
    tlo::perf::perf_func_stats_t                         check_func_stats{};
    tlo::perf::perf_edge_stats_t                         check_edge_stats{};
    const tlo::perf::perf_stats_function_order_clumper_t clumper{};
    uint64_t                                             total_size = 0;
    if (unknowns) {
        stats.filter_funcs(tlo::perf::perf_stats_func_filter_t{}, funcs_out);
        EXPECT_EQ(funcs_out->size(), 4219UL);

        for (const auto & pfunc : *funcs_out) {
            check_func_stats.add(pfunc.stats());
        }
        ASSERT_EQ(stats.agr_func_stats_.num_samples_,
                  check_func_stats.num_samples_);

        stats.filter_edges(tlo::perf::perf_stats_edge_filter_t{}, edges_out);
        EXPECT_EQ(edges_out->size(), 22105UL);


        for (const auto & pedge : *edges_out) {
            check_edge_stats.add(pedge.stats());
        }

        ASSERT_EQ(stats.agr_edge_stats_.num_edges_,
                  check_edge_stats.num_edges_);

        clumper.clump(funcs_out, edges_out);
        EXPECT_EQ(funcs_out->size(), 4217UL);
        EXPECT_EQ(edges_out->size(), 7921UL);
        for (const auto & func : *funcs_out) {
            ASSERT_TRUE(func.valid());
        }

        check_func_stats = {};
        check_edge_stats = {};
        for (const auto & pfunc : *funcs_out) {
            ASSERT_TRUE(pfunc.valid());
            check_func_stats.add(pfunc.stats());
            total_size += pfunc.func_clump_->size();
        }

        for (const auto & pedge : *edges_out) {
            ASSERT_TRUE(pedge.valid());
            check_edge_stats.add(pedge.stats());
            total_size += pedge.from_->size();
            total_size += pedge.to_->size();
        }


        ASSERT_GE(stats.agr_edge_stats_.num_edges_,
                  check_edge_stats.num_edges_);
        ASSERT_GE(stats.agr_func_stats_.num_samples_,
                  check_func_stats.num_samples_);
        ASSERT_LE(stats.agr_func_stats_.num_tracked_br_samples_in_,
                  check_func_stats.num_tracked_br_samples_in_);
        ASSERT_LE(stats.agr_func_stats_.num_tracked_br_samples_out_,
                  check_func_stats.num_tracked_br_samples_out_);
        ASSERT_GE(stats.agr_func_stats_.num_br_samples_in_,
                  check_func_stats.num_br_samples_in_);
        ASSERT_GE(stats.agr_func_stats_.num_br_samples_out_,
                  check_func_stats.num_br_samples_out_);
        EXPECT_EQ(total_size, 49071372013UL);
        EXPECT_EQ(check_func_stats.num_samples_, 65415UL);
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_, 221055UL);
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_, 221055UL);
        EXPECT_EQ(check_func_stats.num_br_samples_in_, 2326682UL);
        EXPECT_EQ(check_func_stats.num_br_samples_out_, 2326682UL);
        EXPECT_EQ(check_edge_stats.num_edges_, 221055UL);
    }
    else {
        stats.filter_and_clump(tlo::perf::perf_stats_func_findable_filter_t{},
                               tlo::perf::perf_stats_edge_findable_filter_t{},
                               clumper, funcs_out, edges_out);

        EXPECT_EQ(funcs_out->size(), 4093UL);
        EXPECT_EQ(edges_out->size(), 2528UL);

        check_func_stats = {};
        check_edge_stats = {};
        for (const auto & pfunc : *funcs_out) {
            check_func_stats.add(pfunc.stats());
            total_size += pfunc.func_clump_->size();
        }
        for (const auto & pedge : *edges_out) {
            check_edge_stats.add(pedge.stats());
            total_size += pedge.from_->size();
            total_size += pedge.to_->size();
        }
        ASSERT_GE(stats.agr_edge_stats_.num_edges_,
                  check_edge_stats.num_edges_);
        ASSERT_GE(stats.agr_func_stats_.num_samples_,
                  check_func_stats.num_samples_);
        ASSERT_LE(stats.agr_func_stats_.num_tracked_br_samples_in_,
                  check_func_stats.num_tracked_br_samples_in_);
        ASSERT_LE(stats.agr_func_stats_.num_tracked_br_samples_out_,
                  check_func_stats.num_tracked_br_samples_out_);
        ASSERT_GE(stats.agr_func_stats_.num_br_samples_in_,
                  check_func_stats.num_br_samples_in_);
        ASSERT_GE(stats.agr_func_stats_.num_br_samples_out_,
                  check_func_stats.num_br_samples_out_);

        EXPECT_EQ(total_size, 6402534UL);
        EXPECT_EQ(check_func_stats.num_samples_, 15166UL);
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_, 53428UL);
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_, 53428UL);
        EXPECT_EQ(check_func_stats.num_br_samples_in_, 585577UL);
        EXPECT_EQ(check_func_stats.num_br_samples_out_, 573949UL);
        EXPECT_EQ(check_edge_stats.num_edges_, 53428UL);
    }

    G_okay = true;
}
static void
init_scaling(double                            v,
             uint32_t                          mask,
             tlo::perf::perf_state_scaling_t * scaling_todo_inout) {
    if (v == 0.0) {
        scaling_todo_inout->set_force_no_scale(mask);
    }
    else if (v != 1.0) {
        scaling_todo_inout->set_add_scale(v, mask);
        scaling_todo_inout->set_use_local_scale(mask);
    }
}

static void
check_saved_state(const char *                         saved_path,
                  const char *                         expec_order_path,
                  const tlo::sym::sym_state_t *        ss_in,
                  tlo::vec_t<tlo::perf::perf_func_t> * funcs,
                  tlo::vec_t<tlo::perf::perf_edge_t> * edges,
                  bool                                 unknowns,
                  double                               scaling_func,
                  double                               scaling_edge) {
    G_okay = false;
    tlo::vec_t<tlo::perf::perf_func_t> funcs_tmp{};
    tlo::vec_t<tlo::perf::perf_edge_t> edges_tmp{};

    // NOLINTNEXTLINE(*magic*)
    std::array<char, 256>           tmpfile;
    const int                       fd = tlo::file_ops::new_tmpfile(&tmpfile);
    tlo::perf::perf_state_scaling_t scaling_todo{};
    init_scaling(scaling_func, tlo::perf::perf_state_scaling_t::k_func_only,
                 &scaling_todo);
    init_scaling(scaling_edge, tlo::perf::perf_state_scaling_t::k_edge_only,
                 &scaling_todo);
    ASSERT_EQ(scaling_todo.has_add_scale(
                  tlo::perf::perf_state_scaling_t::k_func_only),
              scaling_func != 1.0 && scaling_func != 0.0);
    ASSERT_EQ(scaling_todo.has_add_scale(
                  tlo::perf::perf_state_scaling_t::k_edge_only),
              scaling_edge != 1.0 && scaling_edge != 0.0);
    ASSERT_EQ(scaling_todo.has_add_scale(
                  tlo::perf::perf_state_scaling_t::k_edge_only),
              scaling_edge != 1.0 && scaling_edge != 0.0);
    ASSERT_EQ(scaling_todo.has_add_scale(
                  tlo::perf::perf_state_scaling_t::k_func_only),
              scaling_todo.use_local_scale(
                  tlo::perf::perf_state_scaling_t::k_func_only));
    ASSERT_EQ(scaling_todo.has_add_scale(
                  tlo::perf::perf_state_scaling_t::k_edge_only),
              scaling_todo.use_local_scale(
                  tlo::perf::perf_state_scaling_t::k_edge_only));

    ASSERT_EQ(scaling_todo.has_add_scale(),
              scaling_func != 1.0 && scaling_func != 0.0 &&
                  scaling_edge != 1.0 && scaling_edge != 0.0);
    ASSERT_EQ(scaling_todo.has_add_scale(), scaling_todo.use_local_scale());

    ASSERT_TRUE(!scaling_todo.has_add_scale(
                    tlo::perf::perf_state_scaling_t::k_func_only) ||
                scaling_todo.added_func_scale() == scaling_func);
    ASSERT_TRUE(!scaling_todo.has_add_scale(
                    tlo::perf::perf_state_scaling_t::k_edge_only) ||
                scaling_todo.added_edge_scale() == scaling_edge);

    ASSERT_GT(fd, 0);
    const tlo::perf::perf_state_saver_t saver{ ss_in };
    ASSERT_TRUE(saver.save_state(tmpfile.data(), funcs, edges, &scaling_todo));

    tlo::sym::sym_state_t                  ss{};
    const tlo::perf::perf_state_reloader_t reloader{ &ss };

    ASSERT_TRUE(reloader.reload_state(tmpfile.data(), &funcs_tmp, &edges_tmp,
                                      &scaling_todo));


    ASSERT_NE(
        scaling_todo.force_no_scale(
            tlo::perf::perf_state_scaling_t::k_func_only),
        scaling_todo.did_scale(tlo::perf::perf_state_scaling_t::k_func_only));
    ASSERT_NE(
        scaling_todo.force_no_scale(
            tlo::perf::perf_state_scaling_t::k_edge_only),
        scaling_todo.did_scale(tlo::perf::perf_state_scaling_t::k_edge_only));

    ASSERT_EQ(funcs_tmp.size(), funcs->size());
    ASSERT_EQ(edges_tmp.size(), edges->size());

    tlo::perf::perf_func_stats_t check_func_stats{};
    tlo::perf::perf_edge_stats_t check_edge_stats{};
    uint64_t                     total_size = 0;
    for (const auto & pfunc : funcs_tmp) {
        ASSERT_TRUE(pfunc.valid());
        check_func_stats.add(pfunc.stats());
        total_size += pfunc.func_clump_->size();
    }

    for (const auto & pedge : edges_tmp) {
        ASSERT_TRUE(pedge.valid());
        check_edge_stats.add(pedge.stats());
        total_size += pedge.from_->size();
        total_size += pedge.to_->size();
    }

    if (unknowns) {
        EXPECT_EQ(total_size, 49071372013UL);
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_func_only)) {
            const double sv = scaling_todo.has_add_scale(
                                  tlo::perf::perf_state_scaling_t::k_func_only)
                                  ? scaling_todo.added_func_scale()
                                  : 1.0;
            EXPECT_NEAR(
                check_func_stats.num_samples_,
                static_cast<tlo::perf::psample_val_t>(
                    static_cast<double>(tlo::perf::k_func_scale_point) * sv),
                .99);
        }
        else {
            EXPECT_EQ(check_func_stats.num_samples_,
                      static_cast<tlo::perf::psample_val_t>(65415UL));
        }
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(221055UL));
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(221055UL));
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(2326682UL));
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(2326682UL));
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_edge_only)) {
            const double sv = scaling_todo.has_add_scale(
                                  tlo::perf::perf_state_scaling_t::k_edge_only)
                                  ? scaling_todo.added_edge_scale()
                                  : 1.0;
            EXPECT_NEAR(
                check_edge_stats.num_edges_,
                static_cast<tlo::perf::psample_val_t>(
                    static_cast<double>(tlo::perf::k_edge_scale_point) * sv),
                .99);
        }
        else {
            EXPECT_EQ(check_edge_stats.num_edges_,
                      static_cast<tlo::perf::psample_val_t>(221055UL));
        }
    }
    else {
        EXPECT_EQ(total_size, 6402534UL);
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_func_only)) {
            const double sv = scaling_todo.has_add_scale(
                                  tlo::perf::perf_state_scaling_t::k_func_only)
                                  ? scaling_todo.added_func_scale()
                                  : 1.0;

            EXPECT_NEAR(
                check_func_stats.num_samples_,
                static_cast<tlo::perf::psample_val_t>(
                    static_cast<double>(tlo::perf::k_func_scale_point) * sv),
                .99);
        }
        else {
            EXPECT_EQ(check_func_stats.num_samples_,
                      static_cast<tlo::perf::psample_val_t>(15166UL));
        }
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(53428UL));
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(53428UL));
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(585577UL));
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(573949UL));
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_edge_only)) {
            const double sv = scaling_todo.has_add_scale(
                                  tlo::perf::perf_state_scaling_t::k_edge_only)
                                  ? scaling_todo.added_edge_scale()
                                  : 1.0;
            EXPECT_NEAR(
                check_edge_stats.num_edges_,
                static_cast<tlo::perf::psample_val_t>(
                    static_cast<double>(tlo::perf::k_edge_scale_point) * sv),
                .99);
        }
        else {
            EXPECT_EQ(check_edge_stats.num_edges_,
                      static_cast<tlo::perf::psample_val_t>(53428UL));
        }
    }
    // NOLINTBEGIN(*magic*)
    create_and_check_cg_order(
        &funcs_tmp, &edges_tmp, expec_order_path,
        scaling_func == 0.0 && scaling_edge == 0.0 ? 0.0 : 0.9);
    // NOLINTEND(*magic*)
    ASSERT_TRUE(G_okay);

    tlo::sym::sym_state_t                  ss2{};
    const tlo::perf::perf_state_reloader_t reloader2{ &ss2 };

    ASSERT_TRUE(
        reloader2.reload_state(saved_path, funcs, edges, &scaling_todo));
    ASSERT_EQ(funcs_tmp.size(), funcs->size());
    ASSERT_EQ(edges_tmp.size(), edges->size());

    check_func_stats = {};
    check_edge_stats = {};
    total_size       = 0;
    for (const auto & pfunc : *funcs) {
        ASSERT_TRUE(pfunc.valid());
        check_func_stats.add(pfunc.stats());
        total_size += pfunc.func_clump_->size();
    }

    for (const auto & pedge : *edges) {
        ASSERT_TRUE(pedge.valid());
        check_edge_stats.add(pedge.stats());
        total_size += pedge.from_->size();
        total_size += pedge.to_->size();
    }
    if (unknowns) {
        EXPECT_EQ(total_size, 49071372013UL);
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_func_only)) {
            EXPECT_NEAR(check_func_stats.num_samples_,
                        static_cast<tlo::perf::psample_val_t>(
                            static_cast<double>(tlo::perf::k_func_scale_point)),
                        .99)
                << tmpfile.data();
        }
        else {
            EXPECT_EQ(check_func_stats.num_samples_,
                      static_cast<tlo::perf::psample_val_t>(65415UL));
        }
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(221055UL));
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(221055UL));
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(2326682UL));
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(2326682UL));
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_edge_only)) {
            EXPECT_NEAR(check_edge_stats.num_edges_,
                        static_cast<tlo::perf::psample_val_t>(
                            static_cast<double>(tlo::perf::k_edge_scale_point)),
                        .99);
        }
        else {
            EXPECT_EQ(check_edge_stats.num_edges_,
                      static_cast<tlo::perf::psample_val_t>(221055UL));
        }
    }
    else {
        EXPECT_EQ(total_size, 6402534UL);
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_func_only)) {
            EXPECT_NEAR(check_func_stats.num_samples_,
                        static_cast<tlo::perf::psample_val_t>(
                            static_cast<double>(tlo::perf::k_func_scale_point)),
                        .99);
        }
        else {
            EXPECT_EQ(check_func_stats.num_samples_,
                      static_cast<tlo::perf::psample_val_t>(15166UL));
        }
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(53428UL));
        EXPECT_EQ(check_func_stats.num_tracked_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(53428UL));
        EXPECT_EQ(check_func_stats.num_br_samples_in_,
                  static_cast<tlo::perf::psample_val_t>(585577UL));
        EXPECT_EQ(check_func_stats.num_br_samples_out_,
                  static_cast<tlo::perf::psample_val_t>(573949UL));
        if (scaling_todo.did_scale(
                tlo::perf::perf_state_scaling_t::k_edge_only)) {
            EXPECT_NEAR(check_edge_stats.num_edges_,
                        static_cast<tlo::perf::psample_val_t>(
                            static_cast<double>(tlo::perf::k_edge_scale_point)),
                        .99);
        }
        else {
            EXPECT_EQ(check_edge_stats.num_edges_,
                      static_cast<tlo::perf::psample_val_t>(53428UL));
        }
    }

    // NOLINTBEGIN(*magic*)
    create_and_check_cg_order(
        funcs, edges, expec_order_path,
        scaling_func == 0.0 && scaling_edge == 0.0 ? 0.0 : 0.9);
    // NOLINTEND(*magic*)
    close(fd);
    ASSERT_TRUE(G_okay);
}

static constexpr std::array<std::pair<double, double>, 10> k_confs = {
    { std::pair<double, double>{ 0.0, 0.0 },
      std::pair<double, double>{ 1.0, 0.0 },
      std::pair<double, double>{ 1.0, 1.0 },
      std::pair<double, double>{ 0.0, 1.0 },
      std::pair<double, double>{ 2.0, 0.0 },
      std::pair<double, double>{ 2.5, 1.0 },
      std::pair<double, double>{ 1.0, 2.0 },
      std::pair<double, double>{ 0.0, 2.5 },
      std::pair<double, double>{ 2.5, 2.0 },
      std::pair<double, double>{ 0.5, 0.5 } }
};

TEST(perf, collect_perf_file_unknowns) {
    init_env();
    tlo::sym::sym_state_t   ss{};
    tlo::perf::perf_stats_t stats{ &ss };
    collect_and_check_perf_file(&stats);
    ASSERT_TRUE(G_okay);

    tlo::vec_t<tlo::perf::perf_func_t> funcs;
    tlo::vec_t<tlo::perf::perf_edge_t> edges;
    collect_and_check_funcs_and_edges(stats, &funcs, &edges, true);
    ASSERT_TRUE(G_okay);

    create_and_check_cg_order(&funcs, &edges, ORDER_PATH_FMT "-unknowns");
    ASSERT_TRUE(G_okay);

    const tlo::vec_t<tlo::perf::perf_func_t> funcs_saved = funcs;
    const tlo::vec_t<tlo::perf::perf_edge_t> edges_saved = edges;

    for (const auto & conf : k_confs) {
        funcs = funcs_saved;
        edges = edges_saved;
        check_saved_state(SAVE_PATH_FMT "-unknowns", ORDER_PATH_FMT "-unknowns",
                          &ss, &funcs, &edges, true, conf.first, conf.second);
        ASSERT_TRUE(G_okay);
    }
}


TEST(perf, collect_perf_file_known) {
    init_env();
    tlo::sym::sym_state_t   ss{};
    tlo::perf::perf_stats_t stats{ &ss };
    collect_and_check_perf_file(&stats);
    ASSERT_TRUE(G_okay);

    tlo::vec_t<tlo::perf::perf_func_t> funcs;
    tlo::vec_t<tlo::perf::perf_edge_t> edges;
    collect_and_check_funcs_and_edges(stats, &funcs, &edges, false);
    ASSERT_TRUE(G_okay);


    create_and_check_cg_order(&funcs, &edges, ORDER_PATH_FMT "-findable");
    ASSERT_TRUE(G_okay);


    const tlo::vec_t<tlo::perf::perf_func_t> funcs_saved = funcs;
    const tlo::vec_t<tlo::perf::perf_edge_t> edges_saved = edges;

    for (const auto & conf : k_confs) {
        funcs = funcs_saved;
        edges = edges_saved;
        check_saved_state(SAVE_PATH_FMT "-findable", ORDER_PATH_FMT "-findable",
                          &ss, &funcs, &edges, false, conf.first, conf.second);
        ASSERT_TRUE(G_okay);
    }
}
