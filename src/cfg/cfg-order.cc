#include "src/cfg/cfg.h"
#include "src/util/compiler.h"
#include "src/util/umap.h"
#include "src/util/vec.h"

////////////////////////////////////////////////////////////////////////////////
// Contains algorithms for ordering functions in the CFG.
//
// Any clustering algorithm must have an enum ID.
// The clustering algorithm will populate a vector of `cluster_t` types. The
// order of the clusters in that populated vector, as well as the order of the
// nodes within each cluster, will be the final function order for the linker
// script. For example the algo produces:
//  cluster0
//      node A
//      node B
//  cluster 1
//      node C
//
// The final order will be A, B, C.


namespace tlo {

using node_id_t = cfg_t::node_id_t;
using node_t    = cfg_t::node_t;
using edge_t    = cfg_t::edge_t;

enum merge_result_t : uint8_t {
    k_success       = 0,
    k_total_failure = 1,
    k_dso_failure   = 2,
};

// Concept of a cluster of functions (can contain functions for multiple DSOs).
struct cluster_t {
    // Make negative so we can sort out inactive densities;
    static constexpr double k_inactive_density = -1.0;

    // Minimal values (mostly kepts to ensure density > 0.0).
    // TODO: Get rid of these!
    static constexpr uint64_t k_min_func_size = 64;
    static constexpr double   k_min_samples   = 1;

    // Basic stats for a cluster, used to make decisions about merging /
    // placement.
    struct locality_stats_t {
        double   samples_;
        uint64_t code_size_;
        double   density_;

        static constexpr uint64_t
        get_code_size(uint64_t code_size) {
            return std::max(code_size, k_min_func_size);
        }

        static constexpr double
        get_samples(double samples) {
            return std::max(samples, k_min_samples);
        }

        static constexpr double
        compute_density(double samples, uint64_t code_size) {
            return samples / static_cast<double>(code_size);
        }

        bool
        active() const {
            return density_ > 0;
        }

        void
        deactivate() {
            density_ = k_inactive_density;
        }

        void
        merge_in(const locality_stats_t & other) {
            samples_ += other.samples_;
            code_size_ += other.code_size_;
            density_ = compute_density(samples_, code_size_);
        }

        // Sanity tests.
        bool
        valid() const {
            if (!active()) {
                return true;
            }
            if (density_ <= 0.0) {
                return false;
            }
            if (samples_ < k_min_samples) {
                return false;
            }
            if (code_size_ < k_min_func_size) {
                return false;
            }
            return true;
        }


        constexpr locality_stats_t(double samples, uint64_t code_size)
            : samples_(get_samples(samples)),
              code_size_(get_code_size(code_size)),
              density_(compute_density(get_samples(samples),
                                       get_code_size(code_size))) {}

        constexpr locality_stats_t() = default;
    };

    bool
    empty() const {
        return node_ids_.empty();
    }

    size_t
    num_nodes() const {
        return node_ids_.size();
    }


    uint64_t
    code_size() const {
        return agr_stats_.code_size_;
    }

    double
    density() const {
        return agr_stats_.density_;
    }

    node_id_t
    front() const {
        assert(!empty());
        return node_ids_[0];
    }

    void
    clear() {
        agr_stats_.deactivate();
        node_ids_.clear();
        per_dso_stats_.clear();
    }

    bool
    active() const {
        return agr_stats_.active();
    }

    uint64_t
    num_dsos() const {
        return per_dso_stats_.size();
    }

    static bool
    test_combined_localities(const locality_stats_t & combined,
                             const locality_stats_t & old,
                             uint64_t                 max_code_size,
                             double                   max_density_degragation) {
        if (max_code_size != 0 && combined.code_size_ > max_code_size) {
            return false;
        }
        if (max_density_degragation != 0.0 &&
            old.density_ > (combined.density_ * max_density_degragation)) {
            return false;
        }
        return true;
    }

    static locality_stats_t
    try_merge_localities(const locality_stats_t & lhs,
                         const locality_stats_t & rhs,
                         uint64_t                 max_code_size,
                         double                   max_density_degragation) {
        assert(lhs.active() && rhs.active());
        locality_stats_t agr = lhs;
        agr.merge_in(rhs);
        assert(agr.active());

        if (!test_combined_localities(agr, lhs, max_code_size,
                                      max_density_degragation)) {
            agr.deactivate();
        }

        return agr;
    }

    // Try to merge two clusters. Only do so if the new cluster is good enough
    // according to the max_* parameters.
    merge_result_t
    try_merge(cluster_t *          other,
              vec_t<cluster_t *> * id_to_cluster,
              uint64_t             max_total_code_size,
              uint64_t             max_per_dso_code_size,
              double               max_total_density_degragation,
              double               max_per_dso_density_degragation) {
        const locality_stats_t agr_combined = try_merge_localities(
            agr_stats_, other->agr_stats_, max_total_code_size,
            max_total_density_degragation);

        if (!agr_combined.active()) {
            return merge_result_t::k_total_failure;
        }

        const umap<const sym::dso_t *, locality_stats_t> & smaller_tbl =
            num_dsos() > other->num_dsos() ? other->per_dso_stats_
                                           : per_dso_stats_;

        umap<const sym::dso_t *, locality_stats_t> larger_tbl =
            num_dsos() > other->num_dsos() ? per_dso_stats_
                                           : other->per_dso_stats_;

        for (const auto & dso_and_stats : smaller_tbl) {
            auto emp_res =
                larger_tbl.emplace(dso_and_stats.first, dso_and_stats.second);
            if (emp_res.second) {
                continue;
            }
            const locality_stats_t dso_combined = try_merge_localities(
                emp_res.first->second, dso_and_stats.second,
                max_per_dso_code_size, max_per_dso_density_degragation);
            if (!dso_combined.active()) {
                return merge_result_t::k_dso_failure;
            }
        }

        agr_stats_     = agr_combined;
        per_dso_stats_ = larger_tbl;

        for (const node_id_t move_id : other->node_ids_) {
            (*id_to_cluster)[move_id] = this;
            node_ids_.emplace_back(move_id);
        }

        other->clear();
        return merge_result_t::k_success;
    }

    cluster_t() = default;
    cluster_t(const sym::dso_t * dso,
              double             samples,
              uint64_t           code_size,
              node_id_t          id)
        : agr_stats_(samples, code_size), node_ids_(1, id) {
        per_dso_stats_.emplace(dso, locality_stats_t(samples, code_size));
    }
    cluster_t(const node_t * node, node_id_t id)
        : cluster_t(node->dso(), node->samples(), node->size(), id) {}

    // Sanity tests.
    bool
    valid(const cfg_t * cg) const {
        if (node_ids_.empty() != per_dso_stats_.empty()) {
            return false;
        }
        if (empty() == agr_stats_.active()) {
            return false;
        }
        if (!agr_stats_.valid()) {
            return false;
        }

        uset<const sym::dso_t *> marked;
        for (const node_id_t id : node_ids_) {
            const node_t * node    = cg->get_node(id);
            auto           emp_res = marked.emplace(node->dso());
            if (emp_res.second) {
                auto find_res = per_dso_stats_.find(node->dso());
                if (find_res == per_dso_stats_.end()) {
                    return false;
                }
                if (!find_res->second.valid()) {
                    return false;
                }
                if (!find_res->second.active()) {
                    return false;
                }
            }
        }
        return marked.size() == per_dso_stats_.size();
    }


    locality_stats_t                           agr_stats_;
    umap<const sym::dso_t *, locality_stats_t> per_dso_stats_;
    vec_t<node_id_t>                           node_ids_;
};

// Implements the C3 clusterizing algorithm from the hfsort paper.
static void
hfsort_c3(const cfg_t * cg, vec_t<cluster_t> * clusters) {


    // Minimum probability for pred -> node for us to merge the pred's and
    // node's clusters.
    static constexpr double k_min_pred_merge_prob = .1;
    // Huge page size.
    static constexpr size_t k_max_cluster_code_size =
        static_cast<size_t>(2) * 1024 * 1024;
    // Density ratio we are willing to lose by doing a merge.
    static constexpr double k_max_density_degradation = 8;
    // 0 skips this check
    static constexpr size_t k_max_dso_cluster_code_size = 0;
    // 0 skips this check
    static constexpr double k_max_dso_density_degradation = 0;

    clusters->clear();
    clusters->reserve(cg->num_nodes());

    vec_t<node_id_t> sorted_ids;

    sorted_ids.reserve(cg->num_nodes());
    for (const node_id_t id : cg->node_ids()) {
        const node_t * node = cg->get_node(id);
        clusters->emplace_back(node, id);
        sorted_ids.emplace_back(id);
    }


    vec_t<cluster_t *> id_to_cluster;
    id_to_cluster.reserve(cg->num_nodes());

    for (cluster_t & cluster : (*clusters)) {
        id_to_cluster.emplace_back(&cluster);
    }
    assert(id_to_cluster.size() == cg->num_nodes());

    std::sort(sorted_ids.begin(), sorted_ids.end(),
              [cg](node_id_t lhs, node_id_t rhs) {
                  const node_t * lhs_node = cg->get_node(lhs);
                  const node_t * rhs_node = cg->get_node(rhs);
                  // In Bolt/HHVM this is:
                  //
                  // rhs_node->code_size() * lhs_node->samples() >
                  // lhs_node->code_size() * rhs_node->samples()
                  //
                  // This doesn't really make sense and seems to be wrong based
                  // on the hfsort paper:
                  // """
                  // It processes each function in the call graph, in decreasing
                  // order of profile weights.
                  // """
                  return lhs_node->samples() > rhs_node->samples();
              });

    size_t num_clusters = clusters->size();
    // Iterate through each node (according to the above sorted order).
    // At each node, try to merge its most likely predecessor into its cluster.
    for (const node_id_t id : sorted_ids) {
        cluster_t * cluster = id_to_cluster[id];
        assert(cluster != nullptr && cluster->active());

        // expensive correctness check
#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        assert(cluster->valid(cg));
#endif
        const node_t * node = cg->get_node(id);

        if (node->pred_empty()) {
            continue;
        }

        const edge_t * pred = node->hottest_pred();
        assert(pred != nullptr);

        // Nothing todo if they are already in the same cluster.
        cluster_t * pred_cluster = id_to_cluster[pred->id_];
        if (pred_cluster == cluster) {
            continue;
        }

        // Only merge if preds likelyhood is above threshold.
        if (!node->pred_more_likely_than(*pred, k_min_pred_merge_prob)) {
            continue;
        }

        // try to merge (try_merge will check other invariants, namely that the
        // new merged cluster doesn't exceed maximum code size or that its
        // density (samples / code_size) doesn't drop too low.
        const merge_result_t res = pred_cluster->try_merge(
            cluster, &id_to_cluster, k_max_cluster_code_size,
            k_max_dso_cluster_code_size, k_max_density_degradation,
            k_max_dso_density_degradation);
        if (res == merge_result_t::k_success) {
            --num_clusters;
            assert(cluster->empty());
            assert(!cluster->active());
        }
        else {
            assert(!cluster->empty());
            assert(cluster->active());
        }
        // expensive checks
#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        assert(pred_cluster->valid(cg));
        assert(cluster->valid(cg));
#endif
    }


    // Final order: sort clusters by density.
    std::sort(clusters->begin(), clusters->end(),
              [](const cluster_t & lhs, const cluster_t & rhs) {
                  assert(lhs.empty() != lhs.active());
                  assert(rhs.empty() != rhs.active());
                  assert(lhs.active() ? (lhs.density() > 0.0)
                                      : (lhs.density() < 0.0));
                  assert(rhs.active() ? (rhs.density() > 0.0)
                                      : (rhs.density() < 0.0));
                  return lhs.density() > rhs.density();
              });

#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
    // Sanity checks.
    auto dbg_validate = [&]() {
        assert(num_clusters != 0 && "We somehow have NO clusters!");
        assert(num_clusters < clusters->size() && "We didn't do any merging!");

        assert(clusters->at(num_clusters).empty());
        assert(!clusters->at(num_clusters - 1).empty());

        for (size_t i = 0; i < clusters->size(); ++i) {
            assert(clusters->at(i).valid(cg));
            if (i < num_clusters) {
                assert(!clusters->at(i).empty());
                if (i != 0) {
                    assert(clusters->at(i - 1).density() >=
                           clusters->at(i).density());
                }
            }
            else {
                assert(clusters->at(i).empty());
                assert(!clusters->at(i).active());
            }
        }
    };

    // expensive debug checks

    dbg_validate();
#endif
    clusters->resize(num_clusters);
}


// Naive algorithm, just sort by function hotness.
static void
hfsort_hotsort(const cfg_t * cg, vec_t<cluster_t> * clusters) {
    clusters->clear();
    clusters->reserve(cg->num_nodes());

    for (const node_id_t id : cg->node_ids()) {
        const node_t * node = cg->get_node(id);
        clusters->emplace_back(node, id);
    }

    std::sort(clusters->begin(), clusters->end(),
              [&](const cluster_t & lhs, const cluster_t & rhs) {
                  assert(lhs.num_nodes() == 1);
                  assert(rhs.num_nodes() == 1);
                  const node_t * lhs_node = cg->get_node(lhs.front());
                  const node_t * rhs_node = cg->get_node(rhs.front());
                  return lhs_node->samples() > rhs_node->samples();
              });

    return;
}


void
cfg_t::order_nodes(order_algorithm                algo,
                   vec_t<cfg_func_order_info_t> * order_out) const {
    vec_t<cluster_t> clusters;
    switch (algo) {

        // annoyingly, gcc complains if we remove the default case and clang
        // complains if we keep it.
#if TLO_USING_CLANG
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
        default:
            assert(0 && "Unrecognized ordering algorithm!");
#if TLO_USING_CLANG
# pragma clang diagnostic pop
#endif

        case k_hfsort_c3:
            hfsort_c3(this, &clusters);
            break;
        case k_hfsort_hotsort:
            hfsort_hotsort(this, &clusters);
            break;
    }


    // Collect all nodes from our clustering algorithm. Assumed that iteration
    // order of the clusters vec is final order we want nodes in.
    order_out->clear();
    order_out->reserve(num_nodes());
    uint64_t order = 0;
    for (const cluster_t & cluster : clusters) {
        for (const node_id_t id : cluster.node_ids_) {
            const node_t * node = get_node(id);
            const double   node_weight_in =
                static_cast<double>(node->weight_preds_) /
                static_cast<double>(total_weight_in_);
            const double node_weight_out =
                static_cast<double>(node->weight_succs_) /
                static_cast<double>(total_weight_out_);
            order_out->emplace_back(node->func_clump_, order, node_weight_in,
                                    node_weight_out);
            order += node->func_clump_->num_funcs();
        }
    }

    assert(order_out->size() == num_nodes());
}

}  // namespace tlo
