#ifndef SRC_D_PERF_D_PERF_STATS_H_
#define SRC_D_PERF_D_PERF_STATS_H_

#include "src/perf/perf-mappings.h"
#include "src/perf/perf-sample.h"
#include "src/perf/perf-stats-clumper.h"
#include "src/perf/perf-stats-filter.h"
#include "src/perf/perf-stats-types.h"


#include "src/sym/syms.h"

#define TLO_DEBUG
#include "src/util/bits.h"
#include "src/util/debug.h"
#include "src/util/global-stats.h"
#include "src/util/strbuf.h"
#include "src/util/type-info.h"
#include "src/util/umap.h"
#include "src/util/vec.h"
#include "src/util/verbosity.h"
#include "src/util/xxhash.h"

#include <utility>

////////////////////////////////////////////////////////////////////////////////
// Accumulate samples in hashtables.
// Two primary tables.
// Function Table:
//  tpid -> function -> func stats
// Edge Table
//  tpid -> edge -> edge stats.
//
// Note that function is unique to its dso (so a 'memcpy' in GLIBC and 'memcpy'
// and MUSL would not map to the same function).


namespace tlo {
namespace perf {

// Aggregated states for a tid/pid pair.
struct perf_tpid_stats_t {
    using func_set_t = basic_uset<perf_func_t>;
    using edge_set_t = basic_uset<perf_edge_t>;

    func_set_t funcs_;
    edge_set_t edges_;

    // For sanity checks really. Maybe useful at some point.
    perf_func_stats_t agr_func_stats_;
    perf_edge_stats_t agr_edge_stats_;

    perf_edge_stats_t
    add_edge_br_sample(const lbr_br_sample_t * sample,
                       sym::func_clump_t *     from,
                       sym::func_clump_t *     to) {

        auto pedge = edges_.emplace(
            perf_edge_t{ from, to, sample->br_insn_, perf_edge_stats_t{} });
        return pedge.first->add_br_sample(sample);
    }


    perf_func_stats_t
    add_func_br_sample(const lbr_br_sample_t * sample,
                       sym::func_clump_t *     from,
                       sym::func_clump_t *     to) {
        // branch sample. Its used both to update/add a new perf_edge and is
        // used to updated the perf_func assosiated with both from/to.
        auto pfunc = funcs_.emplace(perf_func_t{ from, perf_func_stats_t{} });
        perf_func_stats_t stats = pfunc.first->add_br_sample(sample, false);

        pfunc = funcs_.emplace(perf_func_t{ to, perf_func_stats_t{} });
        stats.add(pfunc.first->add_br_sample(sample, true));
        return stats;
    }


    std::tuple<perf_edge_stats_t, perf_func_stats_t>
    add_lbr_sample(sym::sym_state_t *      state,
                   const perf_mappings_t * mappings_,
                   lbr_sample_t *          sample) {
        perf_edge_stats_t agr_edge_stats{};
        perf_func_stats_t agr_func_stats{};
        strbuf_t<> comm = state->get_strtab()->get_sbuf(sample->hdr_.comm_);
        // Add each sample for each branch in lbr sample.
        for (uint32_t i = 0; i < sample->num_lbr_samples(); ++i) {
            lbr_br_sample_t * br_sample = &(sample->samples_[i]);

            sym::dso_t * from_dso = state->get_dso(&(br_sample->from_));
            sym::dso_t * to_dso   = state->get_dso(&(br_sample->to_));
            from_dso->add_comm_use(comm);
            to_dso->add_comm_use(comm);

            assert(from_dso != nullptr && to_dso != nullptr);
            TLO_INCR_STAT(total_branches_);
            if (!mappings_->fillin_sample_loc(from_dso, sample->hdr_,
                                              &(br_sample->from_),
                                              &(br_sample->br_insn_)) ||
                !mappings_->fillin_sample_loc(to_dso, sample->hdr_,
                                              &(br_sample->to_))) {

                TLO_print_if(tlo::has_verbosity(3),
                             "Unable to filling: %s -> %s (%lx -> %lx)\n",
                             from_dso->str(), to_dso->str(),
                             br_sample->from_.mapped_addr_,
                             br_sample->to_.mapped_addr_);
                continue;
            }


            sym::func_clump_t * from_func =
                state->get_func(from_dso, &(br_sample->from_));
            sym::func_clump_t * to_func =
                state->get_func(to_dso, &(br_sample->to_));
            TLO_INCR_STAT(total_tracked_branches_);

            from_func->add_sample_addr(br_sample->from_.unmapped_addr_);
            to_func->add_sample_addr(br_sample->to_.unmapped_addr_);

            if (from_func != to_func && br_sample->is_trackable_call()) {
                TLO_INCR_STAT(total_true_calls_);
                TLO_ADD_STAT(average_call_dist_,
                             std::max(br_sample->from_.unmapped_addr_,
                                      br_sample->to_.unmapped_addr_) -
                                 std::min(br_sample->from_.unmapped_addr_,
                                          br_sample->to_.unmapped_addr_));
                if ((br_sample->from_.unmapped_addr_ ^
                     br_sample->to_.unmapped_addr_) > 4096) {
                    TLO_INCR_STAT(total_page_cross_calls_);
                }
            }
            agr_edge_stats.add(
                add_edge_br_sample(br_sample, from_func, to_func));
            agr_func_stats.add(
                add_func_br_sample(br_sample, from_func, to_func));
        }
        agr_edge_stats_.add(agr_edge_stats);
        agr_func_stats_.add(agr_func_stats);
        return { agr_edge_stats, agr_func_stats };
    }

    perf_func_stats_t
    add_simple_sample(sym::sym_state_t *      state,
                      const perf_mappings_t * mappings_,
                      simple_sample_t *       sample) {
        // Add a simple sample to the function it was in.
        sym::dso_t * dso = state->get_dso(&(sample->loc_));
        dso->add_comm_use(state->get_strtab()->get_sbuf(sample->hdr_.comm_));
        assert(dso != nullptr);
        if (!mappings_->fillin_sample_loc(dso, sample->hdr_, &(sample->loc_))) {
            TLO_printvvv("Unable to filling: %s -> %lx\n", dso->str(),
                         sample->loc_.mapped_addr_);
            return {};
        }

        sym::func_clump_t * func = state->get_func(dso, &(sample->loc_));

        func->add_sample_addr(sample->loc_.unmapped_addr_);
        auto pfunc = funcs_.emplace(perf_func_t{ func, perf_func_stats_t{} });
        perf_func_stats_t stats = pfunc.first->add_simple_sample(sample);
        agr_func_stats_.add(stats);
        return stats;
    }

    const perf_func_stats_t &
    func_stats() const {
        return agr_func_stats_;
    }

    const perf_edge_stats_t &
    edge_stats() const {
        return agr_edge_stats_;
    }

    // Adds all samples from another set of tpid stats to this one (used for
    // collecting final vector of perf_edges/perf_funcs which is then used to
    // construct the CFG).
    void
    add(const perf_tpid_stats_t & other) {
        for (const auto & other_pfunc : other.funcs_) {
            auto pfunc = funcs_.emplace(
                perf_func_t{ other_pfunc.func_clump_, other_pfunc.stats_ });
            if (!pfunc.second) {
                pfunc.first->stats_.add(other_pfunc.stats_);
            }
        }

        for (const auto & other_pedge : other.edges_) {
            auto pedge = edges_.emplace(
                perf_edge_t{ other_pedge.from_, other_pedge.to_,
                             other_pedge.br_insn_, other_pedge.stats_ });
            if (!pedge.second) {
                pedge.first->stats_.add(other_pedge.stats_);
            }
        }

        agr_edge_stats_.add(other.edge_stats());
        agr_func_stats_.add(other.func_stats());
    }

    // Filter helper.
    template<typename T_filter_t>
    void
    filter_funcs(T_filter_t * filter, vec_t<perf_func_t> * pfuncs_out) {
        for (const auto & pfunc : funcs_) {
            if (filter->match_func(pfunc.func_clump_)) {
                pfuncs_out->emplace_back(pfunc);
            }
        }
    }

    template<typename T_filter_t>
    void
    filter_edges(T_filter_t * filter, vec_t<perf_edge_t> * pedges_out) {
        for (const auto & pedge : edges_) {
            if (filter->match_edge(pedge.from_, pedge.to_, pedge.br_insn_)) {
                pedges_out->emplace_back(pedge);
            }
        }
    }


    // Sanity tests
    bool
    valid() const {
        perf_func_stats_t func_stats{};
        for (const auto & pfunc : funcs_) {
            if (!pfunc.valid()) {
                TLO_TRACE("Invalid pfunc\n");
                return false;
            }
            func_stats.add(pfunc.stats());
        }
        if (!func_stats.eq(agr_func_stats_)) {
            TLO_TRACE("Unable to agr func stats(1)\n");
            return false;
        }

        perf_edge_stats_t edge_stats{};
        for (const auto & pedge : edges_) {
            if (!pedge.valid()) {
                TLO_TRACE("Invalid pedge\n");
                return false;
            }
            edge_stats.add(pedge.stats());
        }
        if (!func_stats.valid()) {
            return false;
        }
        if (!edge_stats.eq(agr_edge_stats_)) {
            TLO_TRACE("Unable to agr edge stats(1)\n");
            return false;
        }

        if (func_stats.num_tracked_br_samples_in_ !=
            func_stats.num_tracked_br_samples_out_) {
            TLO_TRACE("Unbalanced samples in/out(1)\n");
            return false;
        }
        if (func_stats.num_br_samples_in_ != edge_stats.num_edges_) {
            TLO_TRACE("Unbalanced samples in/out(2)\n");
            return false;
        }
        if (func_stats.num_br_samples_in_ != func_stats.num_br_samples_out_) {
            TLO_TRACE("Unbalanced samples in/out(3)\n");
            return false;
        }
        return true;
    }

    void
    dump(FILE * fp) const {
        if (!valid()) {
            fprintf(fp, "\tINVALID\n");
        }
        func_stats().dump(fp, "\t");
        for (const auto & pfunc : funcs_) {
            pfunc.dump(fp);
        }
        edge_stats().dump(fp, "\t");
        for (const auto & pedge : edges_) {
            pedge.dump(fp);
        }
    }
};

struct perf_stats_t {
    using tpid_map_t = umap<uint64_t, perf_tpid_stats_t, xxhasher_t<uint64_t>>;


    sym::sym_state_t * const state_;
    perf_mappings_t          mappings_;

    tpid_map_t tpids_;

    perf_func_stats_t agr_func_stats_;
    perf_edge_stats_t agr_edge_stats_;

    uint64_t nskipped_samples_;


    perf_stats_t() = delete;
    perf_stats_t(sym::sym_state_t * state)
        : state_(state),
          tpids_(64),
          agr_func_stats_({}),
          agr_edge_stats_({}),
          nskipped_samples_(0) {}


    perf_tpid_stats_t *
    emplace_sample(const simple_sample_t * sample) {
        return &(
            tpids_.emplace(sample->tpid(), perf_tpid_stats_t{}).first->second);
    }

    bool
    collect_mmap_sample(const info_sample_t & sample) {
        assert(sample.is_mmap());
        return mappings_.add_sample(state_->get_strtab(), sample);
    }

    bool
    collect_fork_sample(const info_sample_t & sample) {
        assert(sample.is_fork());
        return mappings_.add_fork_sample(sample);
    }

    bool
    finalize_mappings() {
        return mappings_.finalize();
    }


    bool
    collect_simple_sample_stats(simple_sample_t * sample) {
        TLO_INCR_STAT(total_samples_);
        if (!sample->valid()) {
            ++nskipped_samples_;
            return false;
        }
        perf_func_stats_t stats = emplace_sample(sample)->add_simple_sample(
            state_, &mappings_, sample);
        agr_func_stats_.add(stats);
        return !stats.empty();
    }

    bool
    collect_lbr_sample_stats(lbr_sample_t * sample) {
        if (!sample->valid()) {
            ++nskipped_samples_;
            return false;
        }
        bool ret = collect_simple_sample_stats(sample);
        auto [edge_stats, func_stats] =
            emplace_sample(sample)->add_lbr_sample(state_, &mappings_, sample);
        agr_edge_stats_.add(edge_stats);
        agr_func_stats_.add(func_stats);
        ret |= !(edge_stats.empty() && func_stats.empty());
        return ret;
    }


    template<typename T_filter_t>
    bool
    filter_agr_tpids(T_filter_t * filter, perf_tpid_stats_t * agr_stats) const {
        bool set = false;
        for (auto const & tpid_and_stats : tpids_) {
            if (filter->match_tpid(tpid_and_stats.first)) {
                if (set) {
                    agr_stats->add(tpid_and_stats.second);
                }
                else {
                    *agr_stats = tpid_and_stats.second;
                    set        = true;
                }
            }
        }
        return set;
    }

    template<typename T_filter_t>
    bool
    filter_agr_tpids(T_filter_t filter, perf_tpid_stats_t * agr_stats) const {
        return filter_agr_tpids(&filter, agr_stats);
    }


    // Populates pfunc_out vector with all perf_func_t that the filter matches
    // (see perf-stat-filter.h for example filter). Basiedgey this can be used
    // to aggregate/select which functions we care about for the CFG.
    template<typename T_filter_t>
        requires(std::is_base_of_v<perf_stats_func_filter_t, T_filter_t>)
    void
    filter_funcs(T_filter_t * filter, vec_t<perf_func_t> * pfunc_out) const {
        perf_tpid_stats_t agr_stats;
        if (filter_agr_tpids(filter, &agr_stats)) {
            agr_stats.filter_funcs(filter, pfunc_out);
        }
        else {
            assert(0 && "WTF?");
        }
    }

    template<typename T_filter_t>
        requires(std::is_base_of_v<perf_stats_func_filter_t, T_filter_t>)
    void
    filter_funcs(T_filter_t filter, vec_t<perf_func_t> * pfunc_out) const {
        filter_funcs(&filter, pfunc_out);
    }


    // Populates pedge_out vector with all perf_func_t that the filter matches
    // (see perf-stat-filter.h for example filter). Basiedgey this can be used
    // to aggregate/select which edges we care about for the CFG.
    template<typename T_filter_t>
        requires(std::is_base_of_v<perf_stats_edge_filter_t, T_filter_t>)
    void
    filter_edges(T_filter_t * filter, vec_t<perf_edge_t> * pedges_out) const {
        perf_tpid_stats_t agr_stats;
        if (filter_agr_tpids(filter, &agr_stats)) {
            agr_stats.filter_edges(filter, pedges_out);
        }
        else {
            assert(0 && "WTF?");
        }
    }
    template<typename T_filter_t>
        requires(std::is_base_of_v<perf_stats_edge_filter_t, T_filter_t>)
    void
    filter_edges(T_filter_t filter, vec_t<perf_edge_t> * pedges_out) const {
        filter_edges(&filter, pedges_out);
    }


    template<typename T_func_filter_t,
             typename T_edge_filter_t,
             typename T_clumper_t>
        requires(std::is_base_of_v<perf_stats_func_filter_t, T_func_filter_t> &&
                 std::is_base_of_v<perf_stats_edge_filter_t, T_edge_filter_t> &&
                 std::is_base_of_v<perf_stats_clumper_t, T_clumper_t>)
    void
    filter_and_clump(T_func_filter_t *    func_filter,
                     T_edge_filter_t *    edge_filter,
                     T_clumper_t *        clumper,
                     vec_t<perf_func_t> * pfuncs_out,
                     vec_t<perf_edge_t> * pedges_out) const {
        pfuncs_out->clear();
        pedges_out->clear();
        filter_funcs(func_filter, pfuncs_out);
        filter_edges(edge_filter, pedges_out);
        clumper->clump(pfuncs_out, pedges_out);
    }

    template<typename T_func_filter_t,
             typename T_edge_filter_t,
             typename T_clumper_t>
        requires(std::is_base_of_v<perf_stats_func_filter_t, T_func_filter_t> &&
                 std::is_base_of_v<perf_stats_edge_filter_t, T_edge_filter_t> &&
                 std::is_base_of_v<perf_stats_clumper_t, T_clumper_t>)
    void
    filter_and_clump(T_func_filter_t      func_filter,
                     T_edge_filter_t      edge_filter,
                     T_clumper_t          clumper,
                     vec_t<perf_func_t> * pfuncs_out,
                     vec_t<perf_edge_t> * pedges_out) const {
        filter_and_clump(&func_filter, &edge_filter, &clumper, pfuncs_out,
                         pedges_out);
    }


    // Sanity tests
    bool
    valid() const {
        perf_func_stats_t func_stats{};
        perf_edge_stats_t edge_stats{};
        for (auto const & tpid_and_stats : tpids_) {
            if (!tpid_and_stats.second.valid()) {
                TLO_TRACE("Invalid tpid\n");
                return false;
            }
            func_stats.add(tpid_and_stats.second.func_stats());
            edge_stats.add(tpid_and_stats.second.edge_stats());
        }
        if (!func_stats.valid()) {
            return false;
        }
        if (func_stats.num_tracked_br_samples_in_ !=
            func_stats.num_tracked_br_samples_out_) {
            TLO_TRACE("Unbalanced samples in/out(4)\n");
            return false;
        }
        if (func_stats.num_br_samples_in_ != edge_stats.num_edges_) {
            TLO_TRACE("Unbalanced samples in/out(5)\n");
            return false;
        }
        if (func_stats.num_br_samples_in_ != func_stats.num_br_samples_out_) {
            TLO_TRACE("Unbalanced samples in/out(6)\n");
            return false;
        }

        if (!func_stats.eq(agr_func_stats_)) {
            TLO_TRACE("Unable to agr func stats(2)\n");
            return false;
        }
        if (!edge_stats.eq(agr_edge_stats_)) {
            TLO_TRACE("Unable to agr edge stats(2)\n");
            return false;
        }

        if (nskipped_samples_ != 0) {
            TLO_TRACE("Had some invalid samples!\n");
            return false;
        }

        return true;
    }

    void
    dump(FILE * fp = TLO_STDOUT) const {
        return;
        agr_func_stats_.dump(fp, "All Funcs ");
        agr_edge_stats_.dump(fp, "All Edges ");
        if (!valid()) {
            fprintf(fp, "INVALID\n");
        }
        for (auto const & tpid_and_stats : tpids_) {
            fprintf(fp, "TPID: %lx\n", tpid_and_stats.first);
            tpid_and_stats.second.dump(fp);
        }
    }
};


}  // namespace perf
}  // namespace tlo

#endif
