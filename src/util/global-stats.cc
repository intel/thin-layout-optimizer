#include "src/util/global-stats.h"
#include "src/util/verbosity.h"

#include <cstring>

namespace tlo {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
total_stats_t G_total_stats{};
void
total_stats_t::collect(vec_t<stat_counter_t> * stats_out) const {
    stats_out->clear();

    stats_out->emplace_back(total_true_calls_);
    stats_out->emplace_back(total_page_cross_calls_);
    stats_out->emplace_back(average_call_dist_);
    stats_out->emplace_back(total_insn_searched_);
    stats_out->emplace_back(total_insn_decoded_);
    stats_out->emplace_back(total_mappings_);
    stats_out->emplace_back(total_bad_mappings_);
    stats_out->emplace_back(total_tracked_samples_);
    stats_out->emplace_back(total_samples_);
    stats_out->emplace_back(total_tracked_branches_);
    stats_out->emplace_back(total_branches_);
    stats_out->emplace_back(total_unknown_funcs_);
    stats_out->emplace_back(total_known_funcs_);
    stats_out->emplace_back(total_dsos_);
    stats_out->emplace_back(total_processed_dsos_);
    stats_out->emplace_back(total_processed_dso_debugs_);
    stats_out->emplace_back(total_funcs_);
    stats_out->emplace_back(total_edges_);
    stats_out->emplace_back(total_clumped_funcs_);
    stats_out->emplace_back(total_tracked_edges_);
}

void
total_stats_t::reload(stat_counter_t stat_in) {
    if (std::strcmp(total_true_calls_.first, stat_in.first) == 0) {
        total_true_calls_.second += stat_in.second;
    }
    else if (std::strcmp(total_page_cross_calls_.first, stat_in.first) == 0) {
        total_page_cross_calls_.second += stat_in.second;
    }
    else if (std::strcmp(average_call_dist_.first, stat_in.first) == 0) {
        average_call_dist_.second += stat_in.second;
    }
    else if (std::strcmp(total_insn_searched_.first, stat_in.first) == 0) {
        total_insn_searched_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_insn_decoded_.first) == 0) {
        total_insn_decoded_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_mappings_.first) == 0) {
        total_mappings_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_bad_mappings_.first) == 0) {
        total_bad_mappings_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_tracked_samples_.first) == 0) {
        total_tracked_samples_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_samples_.first) == 0) {
        total_samples_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_tracked_branches_.first) == 0) {
        total_tracked_branches_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_branches_.first) == 0) {
        total_branches_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_unknown_funcs_.first) == 0) {
        total_unknown_funcs_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_known_funcs_.first) == 0) {
        total_known_funcs_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_dsos_.first) == 0) {
        total_dsos_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_processed_dsos_.first) == 0) {
        total_processed_dsos_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_processed_dso_debugs_.first) ==
             0) {
        total_processed_dso_debugs_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_funcs_.first) == 0) {
        total_funcs_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_edges_.first) == 0) {
        total_edges_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_clumped_funcs_.first) == 0) {
        total_clumped_funcs_.second += stat_in.second;
    }
    else if (std::strcmp(stat_in.first, total_tracked_edges_.first) == 0) {
        total_tracked_edges_.second += stat_in.second;
    }
}


void
total_stats_t::dump(int vlvl, FILE * fp) const {
    if (!tlo::has_verbosity(vlvl)) {
        return;
    }
    (void)fprintf(fp, "%-32s: %lf\n", total_true_calls_.first,
                  total_true_calls_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_page_cross_calls_.first,
                  total_page_cross_calls_.second);
    (void)fprintf(fp, "%-32s: %lf\n", average_call_dist_.first,
                  average_call_dist_.second / total_true_calls_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_insn_searched_.first,
                  total_insn_searched_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_insn_decoded_.first,
                  total_insn_decoded_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_mappings_.first,
                  total_mappings_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_bad_mappings_.first,
                  total_bad_mappings_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_tracked_samples_.first,
                  total_tracked_samples_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_samples_.first,
                  total_samples_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_tracked_branches_.first,
                  total_tracked_branches_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_branches_.first,
                  total_branches_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_unknown_funcs_.first,
                  total_unknown_funcs_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_known_funcs_.first,
                  total_known_funcs_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_dsos_.first, total_dsos_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_processed_dsos_.first,
                  total_processed_dsos_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_processed_dso_debugs_.first,
                  total_processed_dso_debugs_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_funcs_.first, total_funcs_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_edges_.first, total_edges_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_clumped_funcs_.first,
                  total_clumped_funcs_.second);
    (void)fprintf(fp, "%-32s: %lf\n", total_tracked_edges_.first,
                  total_tracked_edges_.second);
}


}  // namespace tlo
