#include "src/util/algo.h"
#include "src/util/compiler.h"

#include "src/cfg/cfg.h"

namespace tlo {
cfg_t::cfg_t(const vec_t<perf::perf_func_t> & pfuncs,
             const vec_t<perf::perf_edge_t> & pedges) noexcept {
    basic_umap<const sym::func_clump_t *, node_id_t> node_map;
    // Iterate through all the edge samples we found.
    for (const auto & pedge : pedges) {
        assert(pedge.from_ != pedge.to_);
        assert(pedge.from_->is_cg_ready());
        assert(pedge.to_->is_cg_ready());
        // Only track if we can actually find the necessary symbols. (this
        // filters out unknown dso / vdso / kernel functions / etc...)
        if (!pedge.from_->is_findable() || !pedge.to_->is_findable()) {
            // continue;
        }
        if (!pedge.from_->dso()->is_findable() ||
            !pedge.to_->dso()->is_findable()) {
            // continue;
        }

        // If we already have nodes for from/to, then update them.
        auto res_from = node_map.emplace(pedge.from_, k_null_node);
        if (res_from.second) {
            res_from.first->second = new_node(pedge.from_);
        }
        const node_id_t nfrom  = res_from.first->second;
        auto            res_to = node_map.emplace(pedge.to_, k_null_node);
        if (res_to.second) {
            res_to.first->second = new_node(pedge.to_);
        }
        const node_id_t nto = res_to.first->second;

        // Update CFG with edge.
        get_node(nfrom)->add_succ(nto, pedge.stats().num_edges_);
        get_node(nto)->add_pred(nfrom, pedge.stats().num_edges_);
    }

    // Go through all function samples and just update weights on functions that
    // already exist in the CFG.
    for (const auto & pfunc : pfuncs) {
        assert(pfunc.func_clump_->is_cg_ready());
        if (pfunc.samples() == 0 || !pfunc.func_clump_->is_findable()) {
            continue;
        }

        auto res = node_map.emplace(pfunc.func_clump_, k_null_node);
        if (res.second) {
            res.first->second = new_node(pfunc.func_clump_);
        }
        assert(get_node(res.first->second)->sample_weight_ == 0);
        get_node(res.first->second)->sample_weight_ =
            pfunc.stats().num_samples_;
    }


    psample_val_t total_weight_in  = 0;
    psample_val_t total_weight_out = 0;
    for (const node_id_t id : node_ids()) {
        node_t * node = get_node(id);
        node->finalize();
        total_weight_in += node->weight_preds_;
        total_weight_out += node->weight_succs_;
    }

    total_weight_in_  = total_weight_in;
    total_weight_out_ = total_weight_out;
}

// Helpers for dumping CFG as dot. Incomplete!
void
cfg_t::dump_dot(FILE * fp, const sym::dso_t * dso) const {
    if (dso != nullptr) {
        dump_dso_dot(fp, dso);
        return;
    }
    vec_t<char> buf{};
    (void)fprintf(fp, "digraph g {\n");
    for (const node_id_t id : node_ids()) {
        const node_t * node = get_node(id);
        (void)fprintf(fp,
                      "f%zu [label=\"%s\\nsamples=%" TLO_PSAMPLE_VAL_FMT
                      "\\nsize=%zu\"];\n",
                      id, node->label(&buf), node->samples(), node->size());
    }
    for (const node_id_t id : node_ids()) {
        const node_t * node = get_node(id);
        for (const edge_t & edge : node->succs_) {
            (void)fprintf(fp,
                          "f%zu -> f%zu [label=\"weight=%" TLO_PSAMPLE_VAL_FMT
                          "\"];\n",
                          id, edge.id_, edge.weight_);
        }
    }
    (void)fprintf(fp, "}\n");
}

void
cfg_t::dump_dso_dot(FILE * fp, const sym::dso_t * dso) const {
    std::vector<bool> printed(nodes_.size(), false);
    vec_t<char>       buf{};

    (void)fprintf(fp, "digraph g {\n");
    for (const node_id_t id : node_ids()) {
        if (printed[id]) {
            continue;
        }
        const node_t * node = get_node(id);
        if (!node->func_clump_->in_dso(dso)) {
            continue;
        }

        printed[id] = true;
        (void)fprintf(fp,
                      "f%zu [label=\"%s\\nin=1\\nsamples=%" TLO_PSAMPLE_VAL_FMT
                      "\\nsize=%zu\"];\n",
                      id, node->label(&buf), node->samples(), node->size());
        for (int i = 0; i < 2; ++i) {
            for (const edge_t & edge : i == 0 ? node->preds_ : node->succs_) {
                if (printed[edge.id_]) {
                    continue;
                }
                const node_t * pnode = get_node(edge.id_);
                if (!pnode->func_clump_->in_dso(dso)) {
                    printed[edge.id_] = true;
                    (void)fprintf(
                        fp,
                        "f%zu [label=\"%s\\nin=0\\nsamples=%" TLO_PSAMPLE_VAL_FMT
                        "\\nsize=%zu\"];\n",
                        edge.id_, pnode->label(&buf), pnode->samples(),
                        pnode->size());
                }
            }
        }
    }
    for (const node_id_t id : node_ids()) {
        if (!printed[id]) {
            continue;
        }
        const node_t * node = get_node(id);
        for (const edge_t & edge : node->succs_) {
            if (printed[edge.id_]) {
                (void)fprintf(
                    fp,
                    "f%zu -> f%zu [label=\"weight=%" TLO_PSAMPLE_VAL_FMT
                    "\"];\n",
                    id, edge.id_, edge.weight_);
            }
        }

        for (const edge_t & edge : node->preds_) {
            if (printed[edge.id_]) {
                (void)fprintf(
                    fp,
                    "f%zu -> f%zu [label=\"weight=%" TLO_PSAMPLE_VAL_FMT
                    "\"];\n",
                    edge.id_, id, edge.weight_);
            }
        }
    }
    (void)fprintf(fp, "}\n");
}

}  // namespace tlo
