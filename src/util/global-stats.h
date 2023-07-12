#ifndef SRC_D_UTIL_D_GLOBAL_STATS_H_
#define SRC_D_UTIL_D_GLOBAL_STATS_H_

////////////////////////////////////////////////////////////////////////////////
// Class for tracking various stats about the profile and our processing.


#include "src/util/vec.h"

#include <stdio.h>
#include <cstdint>
#include <utility>
namespace tlo {
using stat_counter_t = std::pair<const char *, double>;


struct total_stats_t {
    stat_counter_t total_true_calls_       = { "total_true_calls", 0 };
    stat_counter_t total_page_cross_calls_ = { "total_page_cross_calls", 0 };
    stat_counter_t average_call_dist_      = { "average_call_dist", 0 };
    stat_counter_t total_insn_searched_    = { "total_insn_searched", 0 };
    stat_counter_t total_insn_decoded_     = { "total_insn_decoded", 0 };
    stat_counter_t total_mappings_         = { "total_mappings", 0 };
    stat_counter_t total_bad_mappings_     = { "total_bad_mappings", 0 };
    stat_counter_t total_tracked_samples_  = { "total_tracked_samples", 0 };
    stat_counter_t total_samples_          = { "total_samples", 0 };
    stat_counter_t total_tracked_branches_ = { "total_tracked_branches", 0 };
    stat_counter_t total_branches_         = { "total_branches", 0 };
    stat_counter_t total_unknown_funcs_    = { "total_unknown_funcs", 0 };
    stat_counter_t total_known_funcs_      = { "total_known_funcs", 0 };
    stat_counter_t total_dsos_             = { "total_dsos", 0 };
    stat_counter_t total_processed_dsos_   = { "total_processed_dsos", 0 };
    stat_counter_t total_processed_dso_debugs_ = { "total_processed_dso_debugs",
                                                   0 };
    stat_counter_t total_funcs_                = { "total_funcs", 0 };
    stat_counter_t total_edges_                = { "total_edges", 0 };
    stat_counter_t total_clumped_funcs_        = { "total_clumped_funcs", 0 };
    stat_counter_t total_tracked_edges_        = { "total_tracked_edges", 0 };


    void dump(int vlvl = 0, FILE * fp = stdout) const;
    void collect(vec_t<stat_counter_t> * stats_out) const;
    void reload(stat_counter_t stat_in);
};

extern total_stats_t G_total_stats;

#define TLO_INCR_STAT(field)     global_stats_incr(&(G_total_stats.field))
#define TLO_ADD_STAT(field, val) global_stats_add(&(G_total_stats.field), val)

constexpr static void
global_stats_incr(stat_counter_t * counter) {
    counter->second += static_cast<double>(1);
}
constexpr static void
global_stats_add(stat_counter_t * counter, uint64_t val) {
    counter->second += static_cast<double>(val);
}

static void
global_stats_dump(int vlvl) {
    G_total_stats.dump(vlvl);
}

static void
global_stats_collect(tlo::vec_t<stat_counter_t> * stats_out) {
    G_total_stats.collect(stats_out);
}

static void
global_stats_reload(stat_counter_t stat_in) {
    G_total_stats.reload(stat_in);
}
}  // namespace tlo

#endif
