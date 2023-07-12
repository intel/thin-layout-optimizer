#ifndef SRC_D_PERF_D_PERF_STATS_FILTER_H_
#define SRC_D_PERF_D_PERF_STATS_FILTER_H_

#include "src/sym/syms.h"

////////////////////////////////////////////////////////////////////////////////
// Used to inspect samples stores in `perf_stats_t` to decide which to export.
// Decision can be made based on tid/pid or function (function has access to
// its DSO).

namespace tlo {
namespace perf {
struct perf_stats_func_filter_t {
    constexpr bool
    match_tpid(uint64_t tpid) const {
        return true;
        (void)tpid;
    }

    constexpr bool
    match_func(const sym::func_clump_t * func) const {
        return true;
        (void)func;
    }
};

struct perf_stats_edge_filter_t {
    constexpr bool
    match_tpid(uint64_t tpid) const {
        return true;
        (void)tpid;
    }

    constexpr bool
    match_edge(const sym::func_clump_t * from,
               const sym::func_clump_t * to,
               system::br_insn_t         br_insn) const {
        return true;
        (void)from;
        (void)to;
        (void)br_insn;
    }
};

struct perf_stats_func_findable_filter_t : perf_stats_func_filter_t {
    constexpr bool
    match_tpid(uint64_t tpid) const {
        return true;
        (void)tpid;
    }

    bool
    match_func(const sym::func_clump_t * func) const {
        return func->is_findable() && func->dso()->is_findable();
    }
};

struct perf_stats_edge_findable_filter_t : perf_stats_edge_filter_t {
    constexpr bool
    match_tpid(uint64_t tpid) const {
        return true;
        (void)tpid;
    }

    bool
    match_edge(const sym::func_clump_t * from,
               const sym::func_clump_t * to,
               system::br_insn_t         br_insn) const {
        return from->is_findable() && to->is_findable() &&
               from->dso()->is_findable() && to->dso()->is_findable();
        (void)br_insn;
    }
};


}  // namespace perf
}  // namespace tlo
#endif
