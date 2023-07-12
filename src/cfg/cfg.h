#ifndef SRC_D_CFG_D_CFG_H_
#define SRC_D_CFG_D_CFG_H_

#include "src/perf/perf-stats.h"

#include "src/util/iterator.h"
#include "src/util/strbuf.h"
#include "src/util/type-info.h"
#include "src/util/vec.h"
#include "src/util/xxhash.h"

////////////////////////////////////////////////////////////////////////////////
// CFG built from all the calls/samples we have.

namespace tlo {
using psample_val_t = perf::psample_val_t;

struct cfg_func_order_info_t {
    const sym::func_clump_t * fc_;
    size_t                    order_;
    double                    weight_in_;
    double                    weight_out_;

    cfg_func_order_info_t() = default;
    cfg_func_order_info_t(const sym::func_clump_t * fc,
                          size_t                    order,
                          double                    weight_in,
                          double                    weight_out) noexcept
        : fc_(fc),
          order_(order),
          weight_in_(weight_in),
          weight_out_(weight_out) {}

    bool
    valid() const {
        return fc_->valid() && weight_in_ >= 0.0 && weight_out_ >= 0.0 &&
               weight_in_ <= 1.0 && weight_out_ <= 1.0;
    }

    void
    label(tlo::vec_t<char> * buf) const {
        static constexpr size_t        k_tmpbuf_len = 512;
        std::array<char, k_tmpbuf_len> tmpbuf;
        int len = snprintf(tmpbuf.data(), tmpbuf.size(), " -- %lf:%lf",
                           weight_in_, weight_out_);
        assert(len >= 0);
        size_t postfix_len = static_cast<size_t>(len);
        assert(postfix_len < k_tmpbuf_len);
        fc_->label(buf, { tmpbuf.data(), postfix_len });
    }
};
struct cfg_t {
    // Node id as size_t allows for easy indexing (useful for ordering
    // functions).
    struct node_t;
    using node_id_t                        = size_t;
    static constexpr node_id_t k_null_node = std::numeric_limits<size_t>::max();


    // Edge, has target (id_) and number of times it was sampled (weight_)
    struct edge_t {
        node_id_t     id_;
        psample_val_t weight_;

        constexpr edge_t() = default;
        constexpr edge_t(node_id_t id, psample_val_t weight) noexcept
            : id_(id), weight_(weight) {}
    };
    static_assert(has_okay_type_traits<edge_t>::value);

    // Actual CFG node
    struct node_t {
        // Function this node is for.
        const sym::func_clump_t * func_clump_;
        // All independent samples (non LBR samples)
        psample_val_t sample_weight_;
        // Combined weight out (total number of times we call any successor).
        psample_val_t weight_succs_;
        // Combined weight in (total number of times this function was called)
        psample_val_t weight_preds_;
        // Edges.
        small_vec_t<edge_t> succs_;
        small_vec_t<edge_t> preds_;

        node_t() = default;
        node_t(const sym::func_clump_t * func_clump) noexcept
            : func_clump_(func_clump),
              sample_weight_{},
              weight_succs_{},
              weight_preds_{},
              succs_{},
              preds_{} {}

        const sym::dso_t *
        dso() const {
            return func_clump_->dso();
        }

        uint64_t
        size() const {
            return func_clump_->is_unknown() ? 1 : func_clump_->size();
        }
#if 0
        constexpr char const *
        str() const {
            return func_clump_->str();
        }
#endif
        char const *
        label(vec_t<char> * buf) const {
            buf->clear();
            return func_clump_->label(buf).data();
        }

        const sym::func_clump_t *
        func_clump() const {
            return func_clump_;
        }

        constexpr psample_val_t
        samples() const {
            // TODO: sample_weight_ should probably serve as a multiplier for
            // pred/succ weight (or something else).
            return sample_weight_;
        }

        constexpr bool
        is_size_estimated() const {
            return !func_clump_->has_elfinfo();
        }

        template<typename T_t>
        static constexpr bool
        prob_geq_ratio(psample_val_t all, psample_val_t event, T_t ratio) {
            return all <= static_cast<psample_val_t>(T_t(event) * ratio);
        }

        static constexpr bool
        prob_geq(psample_val_t all, psample_val_t event, double prob) {
            return prob_geq_ratio(all, event, double(1) / prob);
        }

        // Helpers for checking probability of a given edge (used by clustering
        // algorithms).
        static constexpr bool
        edge_more_likely_than(psample_val_t weight_total,
                              psample_val_t weight_edge,
                              double        prob) {
            return prob_geq(weight_total, weight_edge, prob);
        }

        template<typename T_t>
        static constexpr bool
        edge_more_likely_than_ratio(psample_val_t weight_total,
                                    psample_val_t weight_edge,
                                    T_t           ratio) {
            return prob_geq_ratio(weight_total, weight_edge, ratio);
        }

        constexpr bool
        pred_more_likely_than(const edge_t & edge, double prob) const {
            return edge_more_likely_than(weight_preds_, edge.weight_, prob);
        }

        template<typename T_t>
        constexpr bool
        pred_more_likely_than_ratio(const edge_t & edge, T_t ratio) const {
            return edge_more_likely_than_ratio(weight_preds_, edge.weight_,
                                               ratio);
        }

        constexpr bool
        succ_more_likely_than(const edge_t & edge, double prob) const {
            return edge_more_likely_than(weight_succs_, edge.weight_, prob);
        }

        template<typename T_t>
        constexpr bool
        succ_more_likely_than_ratio(const edge_t & edge, T_t ratio) const {
            return edge_more_likely_than_ratio(weight_succs_, edge.weight_,
                                               ratio);
        }
        void
        add_succ(node_id_t id, psample_val_t edge_weight) {
            succs_.emplace_back(id, edge_weight);
            weight_succs_ += edge_weight;
        }

        void
        add_pred(node_id_t id, psample_val_t edge_weight) {
            preds_.emplace_back(id, edge_weight);
            weight_preds_ += edge_weight;
        }

        bool
        pred_empty() const {
            return preds_.empty();
        }

        bool
        succ_empty() const {
            return succs_.empty();
        }

        const edge_t *
        hottest_pred() const {
            return pred_empty() ? nullptr : &(preds_.front());
        }

        const edge_t *
        hottest_succ() const {
            return succ_empty() ? nullptr : &(succs_.front());
        }

        // Conventient to have edges sorted (helps with clustering algos).
        void
        finalize() {
            struct sort_by_edge_weight_t {
                bool
                operator()(const edge_t & lhs, const edge_t & rhs) {
                    return lhs.weight_ > rhs.weight_;
                }
            };
            std::sort(succs_.begin(), succs_.end(), sort_by_edge_weight_t{});
            std::sort(preds_.begin(), preds_.end(), sort_by_edge_weight_t{});
        }

        // Sanitzy checker.
        bool
        valid(node_id_t this_id) const {
            auto f_sorted_and_sum = [this_id](psample_val_t expec_total_weight,
                                              const vec_t<edge_t> & edges) {
                psample_val_t total_weight = 0;
                for (size_t i = 0; i < edges.size(); ++i) {
                    total_weight += edges[i].weight_;
                    if (this_id == edges[i].id_) {
                        return false;
                    }
                    if (i) {
                        if (edges[i - 1].weight_ < edges[i].weight_) {
                            return false;
                        }
                    }
                }
                return total_weight == expec_total_weight;
            };
            if (func_clump_->is_unknown() || func_clump_->dso()->is_unknown()) {
                //   return false;
            }
            if (!f_sorted_and_sum(weight_succs_, succs_) ||
                !f_sorted_and_sum(weight_preds_, preds_)) {
                return false;
            }
            return samples() != 0 || !(pred_empty() && succ_empty());
        }

        void
        dump(const cfg_t * cg, FILE * fp = stdout) const {
            vec_t<char> buf{};
            (void)fprintf(fp, "%s\n", label(&buf));
            (void)fprintf(fp, "\tSuccs(%zu):\n", succs_.size());
            for (const edge_t & succ : succs_) {
                (void)fprintf(fp, "\t\t%" TLO_PSAMPLE_VAL_FMT " -> %s\n",
                              succ.weight_,
                              cg->get_node(succ.id_)->label(&buf));
            }
            (void)fprintf(fp, "\tPreds(%zu):\n", preds_.size());
            for (const edge_t & pred : preds_) {
                (void)fprintf(fp, "\t\t%" TLO_PSAMPLE_VAL_FMT " -> %s\n",
                              pred.weight_,
                              cg->get_node(pred.id_)->label(&buf));
            }
        }
    };
    // TODO
    // static_assert(has_okay_type_traits<node_t>::value);

    node_id_t
    new_node(const sym::func_clump_t * func_clump) noexcept {
        node_id_t id = nodes_.size();
        nodes_.emplace_back(func_clump);
        return id;
    }

    node_t *
    get_node(node_id_t id) {
        assert(id < nodes_.size());
        return &(nodes_[id]);
    }

    node_t const *
    get_node(node_id_t id) const {
        assert(id < nodes_.size());
        return &(nodes_[id]);
    }

    const sym::func_clump_t *
    get_func_clump(node_id_t id) const {
        assert(id < nodes_.size());
        return nodes_[id].func_clump_;
    }

    cfg_t() = delete;
    cfg_t(const vec_t<perf::perf_func_t> & pfuncs,
          const vec_t<perf::perf_edge_t> & pedges) noexcept;


    size_t
    num_nodes() const {
        return nodes_.size();
    }

#if 0
    decltype(auto)
    node_ids() const {
        return std::ranges::iota_view{ decltype(num_nodes()){ 0 },
                                       num_nodes() };
    }
#else
    decltype(auto)
    node_ids() const {
        return make_cnt_iterator<size_t>(0, num_nodes());
    }
#endif

    // Sanity tests.
    bool
    valid() const {
        vec_t<bool> ids(num_nodes(), false);
        for (node_id_t id : node_ids()) {
            if (!get_node(id)->valid(id)) {
                return false;
            }
            if (ids[id]) {
                return false;
            }
            ids[id] = true;
        }
        return true;
    }

    // Algorithms we support
    enum order_algorithm { k_hfsort_c3 = 0, k_hfsort_hotsort = 1 };

    // Perform some ordering algorithm (the entire reason we construct the CFG).
    // Implemented in cfg-order.cc
    void order_nodes(order_algorithm                algo,
                     vec_t<cfg_func_order_info_t> * order_out) const;


    void dump_dot(FILE * fp, const sym::dso_t * dso = nullptr) const;
    void dump_dso_dot(FILE * fp, const sym::dso_t * dso) const;


    static void
    cfg_prepare(vec_t<perf::perf_func_t> * pfuncs,
                vec_t<perf::perf_edge_t> * pedges) {
        for (const auto & pfunc : *pfuncs) {
            pfunc.cg_prepare();
        }
        for (const auto & pedge : *pedges) {
            pedge.cg_prepare();
        }


        std::sort(pfuncs->begin(), pfuncs->end(), perf::perf_func_t::cmp_t{});
        std::sort(pedges->begin(), pedges->end(), perf::perf_edge_t::cmp_t{});
    }


    psample_val_t  total_weight_in_;
    psample_val_t  total_weight_out_;
    vec_t<node_t>  nodes_;
    bump_alloc_t<> alloc_;
};
}  // namespace tlo

#endif
