#ifndef SRC_D_PERF_D_PERF_STATS_TRACKER_H_
#define SRC_D_PERF_D_PERF_STATS_TRACKER_H_

////////////////////////////////////////////////////////////////////////////////
// Clump the perf functions with the intention so they are in a format fit for
// the CFG.

#include "src/perf/perf-stats-types.h"
#include "src/sym/syms.h"
#include "src/util/global-stats.h"
#include "src/util/umap.h"
#include "src/util/verbosity.h"
namespace tlo {
namespace perf {
struct perf_stats_clumper_t {
    void
    clump(vec_t<perf_func_t> * pfuncs_inout,
          vec_t<perf_edge_t> * pedges_inout) const {
        (void)pfuncs_inout;
        (void)pedges_inout;
    }
};


struct perf_stats_function_order_clumper_t : perf_stats_clumper_t {

    void
    clump(vec_t<perf_func_t> * pfuncs_inout,
          vec_t<perf_edge_t> * pedges_inout) const {
        std::sort(std::begin(*pfuncs_inout), std::end(*pfuncs_inout),
                  sym::addr_range_t::cmp_t{});
        if (pedges_inout->empty() || pfuncs_inout->empty()) {
            return;
        }
        TLO_ADD_STAT(total_funcs_, pfuncs_inout->size());
        TLO_ADD_STAT(total_edges_, pedges_inout->size());

        // Clump any functions whose addressed overlap.
        vec_t<perf_func_t *> remapping;
        size_t               i, e;
        perf_func_t *        last = &(*pfuncs_inout)[0];
        remapping.reserve(pfuncs_inout->size());
        remapping.emplace_back(last);
        for (i = 1, e = pfuncs_inout->size(); i < e; ++i) {
            perf_func_t * cur = &(*pfuncs_inout)[i];
            assert(last != cur);
            assert(last->func_clump_ != cur->func_clump_);

            if (perf_func_t::overlap(last, cur)) {
                last->func_clump_->dump(2, stdout, "Addr Adding To:   ");
                cur->func_clump_->dump(2, stdout, "Addr Adding From: ");
                last->merge(cur);
                last->func_clump_->dump(2, stdout, "Addr Merged:   ");
            }
            else {
                last = cur;
            }
            remapping.emplace_back(last);
        }


        for (i = 0, e = remapping.size(); i < e; ++i) {
            if (&(*pfuncs_inout)[i] != remapping[i]) {
                for (size_t j = 0; j < e; ++j) {
                    assert(remapping[j] != &(*pfuncs_inout)[i]);
                }
            }
        }

        for (const perf_func_t * pf : remapping) {
            assert(pf->valid());
        }

#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        // test:
        assert(remapping.size() == pfuncs_inout->size());
        for (i = 0, e = pfuncs_inout->size(); i < e; ++i) {
            assert(perf_func_t::overlap(remapping[i], &(*pfuncs_inout)[i]));
            if (remapping[i] == &(*pfuncs_inout)[i]) {
                assert(remapping[i]->func_clump_->num_funcs() >=
                       (*pfuncs_inout)[i].func_clump_->num_funcs());
                assert(remapping[i]->func_clump_->size() >=
                       (*pfuncs_inout)[i].func_clump_->size());
            }
            else {
                assert(!remapping[i]->func_clump_->is_contiguous());
                assert(!remapping[i]->func_clump_->is_single());
                assert(remapping[i]->func_clump_->num_funcs() >
                       (*pfuncs_inout)[i].func_clump_->num_funcs());
                assert(remapping[i]->func_clump_->size() >
                       (*pfuncs_inout)[i].func_clump_->size());
            }
        }

        for (i = 0, e = remapping.size(); i < e; ++i) {
            if (&(*pfuncs_inout)[i] != remapping[i]) {
                for (size_t j = 0; j < e; ++j) {
                    assert(remapping[j] != &(*pfuncs_inout)[i]);
                }
            }
        }
        for (const perf_func_t * pf : remapping) {
            assert(pf->valid());
        }
#endif
        // Clump any functions with the same name.
        bool changed = false;
        umap<const sym::func_t *, perf_func_t *, sym::func_t::name_hasher_t,
             sym::func_t::name_equals_t>
            name_to_clump;
        TLO_DISABLE_WALLOC_ZERO
        name_to_clump.reserve(remapping.size() + 256UL);
        TLO_REENABLE_WALLOC_ZERO
        for (i = 0, e = remapping.size(); i < e; ++i) {
            if (remapping[i] != &(*pfuncs_inout)[i]) {
                //                continue;
            }
            perf_func_t * cur = remapping[i];
            for (const sym::func_t & func : cur->func_clump_->funcs_) {
                name_to_clump.emplace(&func, cur);
            }
        }
        do {
            changed = false;
            for (i = 0, e = remapping.size(); i < e; ++i) {
                if (remapping[i] != &(*pfuncs_inout)[i]) {
                    for (;;) {
                        perf_func_t * remap = remapping[i];
                        size_t        idx =
                            static_cast<size_t>(remap - pfuncs_inout->data());
                        assert(idx < e);
                        if (remapping[idx] == &(*pfuncs_inout)[idx]) {
                            break;
                        }
                        changed      = true;
                        remapping[i] = remapping[idx];
                    }

                    continue;
                }
                perf_func_t * cur = remapping[i];
                perf_func_t * tgt = nullptr;
                for (const sym::func_t & func : cur->func_clump_->funcs_) {
                    auto res = name_to_clump.find(&func);
                    assert(res != name_to_clump.end());
                    if (res->second != cur) {
                        tgt = res->second;
                        break;
                    }
                }
                if (tgt != nullptr) {
                    changed = true;
                    for (const sym::func_t & func : cur->func_clump_->funcs_) {
                        name_to_clump.erase(&func);
                    }
                    for (const sym::func_t & func : tgt->func_clump_->funcs_) {
                        name_to_clump.erase(&func);
                    }
                    tgt->func_clump_->dump(2, stdout, "Name Adding To:   ");
                    cur->func_clump_->dump(2, stdout, "Name Adding From: ");
                    tgt->merge(cur);
                    remapping[i] = tgt;
                    tgt->func_clump_->dump(2, stdout, "Name Merged:   ");
                    for (const sym::func_t & func : tgt->func_clump_->funcs_) {
                        name_to_clump[&func] = tgt;
                    }
                }
            }
        } while (changed);
#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        // test:
        name_to_clump.clear();
        for (i = 0, e = remapping.size(); i < e; ++i) {
            perf_func_t * cur = remapping[i];
            for (const sym::func_t & func : cur->func_clump_->funcs_) {
                auto res = name_to_clump.emplace(&func, cur);
                assert(!res.second || res.first->second == cur);
            }
        }

        for (i = 0, e = remapping.size(); i < e; ++i) {
            if (&(*pfuncs_inout)[i] != remapping[i]) {
                for (size_t j = 0; j < e; ++j) {
                    assert(remapping[j] != &(*pfuncs_inout)[i]);
                }
            }
        }
#endif

        // Finally cleanup edges based on new clumps. Discard any non-calls at
        // the same time.
        basic_umap<const sym::func_clump_t *, perf_func_t *> fc_remapping;
        TLO_DISABLE_WALLOC_ZERO
        fc_remapping.reserve(remapping.size());
        TLO_REENABLE_WALLOC_ZERO
        for (i = 0, e = remapping.size(); i < e; ++i) {
            assert(fc_remapping
                       .emplace((*pfuncs_inout)[i].func_clump_, remapping[i])
                       .second);
        }


        basic_uset<perf_edge_t *> new_edges;
        TLO_DISABLE_WALLOC_ZERO
        new_edges.reserve(pedges_inout->size());
        TLO_REENABLE_WALLOC_ZERO
        size_t pedges_out_size = 0;
        for (i = 0, e = pedges_inout->size(); i < e; ++i) {
            perf_edge_t pe = (*pedges_inout)[i];
            if (pe.from_ == pe.to_) {
                continue;
            }
            if (!pe.br_insn_.good() || !pe.br_insn_.is_trackable_call()) {
                continue;
            }


            auto mapped_from = fc_remapping.find(pe.from_);
            auto mapped_to   = fc_remapping.find(pe.to_);

            assert(mapped_from != fc_remapping.end() &&
                   mapped_to != fc_remapping.end());

            if (mapped_from->second->func_clump_ ==
                mapped_to->second->func_clump_) {
                continue;
            }


            (*pedges_inout)[pedges_out_size] =
                perf_edge_t{ mapped_from->second->func_clump_,
                             mapped_to->second->func_clump_,
                             system::br_insn_t::make_bad(),
                             {} };
            auto res = new_edges.emplace(&(*pedges_inout)[pedges_out_size]);
            (*res.first)->stats_.add(pe.stats());
            if (res.second) {
                ++pedges_out_size;
            }
        }

        pedges_inout->resize(pedges_out_size);
        for (const perf_edge_t & pe : *pedges_inout) {
            auto mapped_from = fc_remapping.find(pe.from_);
            auto mapped_to   = fc_remapping.find(pe.to_);
            mapped_from->second->add_edge_stats(pe.stats(), false);
            mapped_to->second->add_edge_stats(pe.stats(), true);
        }


        for (i = 0, e = remapping.size(); i < e; ++i) {
            perf_func_t *lo, *hi;
            lo = &(*pfuncs_inout)[i];
            if (lo == remapping[i]) {
                continue;
            }
            for (; i < (--e);) {
                hi = &(*pfuncs_inout)[e];
                if (hi == remapping[e]) {
                    std::swap((*pfuncs_inout)[i], (*pfuncs_inout)[e]);
                    remapping[e] = remapping[i];
                    remapping[i] = &(*pfuncs_inout)[i];
                    assert(&(*pfuncs_inout)[i] == remapping[i]);
                    assert(&(*pfuncs_inout)[e] != remapping[e]);
                    break;
                }
            }
        }
        assert(i == e);
        size_t pfuncs_out_size = i;

        // test:
        name_to_clump.clear();
        bool expec_same = true;
        for (i = 0, e = remapping.size(); i < e; ++i) {
            assert((*pfuncs_inout)[i].valid());
            if (&(*pfuncs_inout)[i] == remapping[i]) {
                assert(i < pfuncs_out_size);
                assert(expec_same);
            }
            else {
                assert(i >= pfuncs_out_size);
                expec_same = false;

                bool okay = false;
                for (size_t j = 0; j < i; ++j) {
                    if (&(*pfuncs_inout)[j] == remapping[i]) {
                        assert(!okay);
                        okay = true;
                    }
                }
                assert(okay);
            }
        }


        pfuncs_inout->resize(pfuncs_out_size);


#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        // final validation
        for (const perf_func_t & pf : *pfuncs_inout) {
            assert(pf.valid());
        }
        for (const perf_edge_t & pe : *pedges_inout) {
            assert(pe.valid());
        }
#endif
        TLO_ADD_STAT(total_clumped_funcs_, pfuncs_inout->size());
        TLO_ADD_STAT(total_tracked_edges_, pedges_inout->size());
    }
};

}  // namespace perf
}  // namespace tlo

#endif
