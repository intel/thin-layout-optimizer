#ifndef SRC_D_PERF_D_PERF_SAVER_H_
#define SRC_D_PERF_D_PERF_SAVER_H_

#include "src/perf/perf-stats-types.h"

#include "src/sym/syms.h"
#include "src/util/global-stats.h"
#include "src/util/vec.h"

#include <cmath>

namespace tlo {
namespace perf {

static constexpr uint32_t      k_perf_state_saver_ver = 0;
static constexpr psample_val_t k_func_scale_point =
    static_cast<psample_val_t>(1UL << 20U);
static constexpr psample_val_t k_edge_scale_point =
    static_cast<psample_val_t>(1UL << 30U);


// Scaling information for how we want to reload / save a given state.
struct perf_state_scaling_t {

    static constexpr uint32_t k_no_scale_bit        = 0x1;
    static constexpr uint32_t k_force_no_scale_bit  = 0x2;
    static constexpr uint32_t k_add_scale_bit       = 0x4;
    static constexpr uint32_t k_use_local_scale_bit = 0x8;
    static constexpr uint32_t k_did_scale_bit       = 0x10;

    static constexpr uint32_t k_num_scale_options = 5;

    static constexpr uint32_t k_no_scale =
        (k_no_scale_bit | (k_no_scale_bit << k_num_scale_options));
    static constexpr uint32_t k_force_no_scale =
        (k_force_no_scale_bit | (k_force_no_scale_bit << k_num_scale_options));
    static constexpr uint32_t k_add_scale =
        (k_add_scale_bit | (k_add_scale_bit << k_num_scale_options));
    static constexpr uint32_t k_use_local_scale =
        (k_use_local_scale_bit |
         (k_use_local_scale_bit << k_num_scale_options));
    static constexpr uint32_t k_did_scale =
        (k_did_scale_bit | (k_did_scale_bit << k_num_scale_options));

    static constexpr uint32_t k_scale_options_mask =
        (1u << k_num_scale_options) - 1;

    static constexpr uint32_t k_func_only_scale_options =
        k_num_scale_options * 0;
    static constexpr uint32_t k_edge_only_scale_options =
        k_num_scale_options * 1;

    static constexpr uint32_t k_func_only = k_scale_options_mask
                                            << k_func_only_scale_options;
    static constexpr uint32_t k_edge_only = k_scale_options_mask
                                            << k_edge_only_scale_options;

    static constexpr uint32_t k_all = k_func_only | k_edge_only;
    uint32_t                  scaling_;
    double                    scale_func_;
    double                    scale_edge_;


    constexpr void
    set_no_scale(uint32_t mask = k_all) {
        scaling_ |= (k_no_scale & mask);
    }

    constexpr void
    set_force_no_scale(uint32_t mask = k_all) {
        scaling_ |= (k_force_no_scale & mask);
    }

    constexpr bool
    set_add_scale(double v, uint32_t mask = k_all) {
        if (has_add_scale(mask) || v <= 0 || std::isnan(v) || std::isinf(v) ||
            v == HUGE_VAL) {
            return false;
        }
        scaling_ |= (k_add_scale & mask);
        if ((mask & k_func_only) != 0) {
            scale_func_ = v;
        }
        if ((mask & k_edge_only) != 0) {
            scale_edge_ = v;
        }
        return true;
    }

    constexpr void
    set_use_local_scale(uint32_t mask = k_all) {
        scaling_ |= (k_use_local_scale & mask);
    }

    constexpr bool
    should_scale(uint32_t mask = k_all) const {
        return !no_scale(mask);
    }

    constexpr bool
    soft_no_scale(uint32_t mask = k_all) const {
        return (scaling_ & k_no_scale & mask) == (k_no_scale & mask);
    }

    constexpr bool
    force_no_scale(uint32_t mask = k_all) const {
        return (scaling_ & k_force_no_scale & mask) ==
               (k_force_no_scale & mask);
    }

    constexpr bool
    no_scale(uint32_t mask = k_all) const {
        return force_no_scale(mask) || soft_no_scale(mask);
    }

    constexpr bool
    has_add_scale(uint32_t mask = k_all) const {
        return (scaling_ & k_add_scale & mask) == (k_add_scale & mask);
    }

    constexpr bool
    use_local_scale(uint32_t mask = k_all) const {
        return (scaling_ & k_use_local_scale & mask) ==
               (k_use_local_scale & mask);
    }

    constexpr bool
    did_scale(uint32_t mask = k_all) const {
        return (scaling_ & k_did_scale & mask) == (k_did_scale & mask);
    }

    constexpr bool
    did_scale_any(uint32_t mask = k_all) const {
        return (scaling_ & k_did_scale & mask) != 0;
    }

    constexpr void
    set_did_scale(bool v, uint32_t mask = k_all) {
        if (v) {
            scaling_ |= (k_did_scale & mask);
        }
        else {
            scaling_ &= (~(k_did_scale & mask));
        }
        assert(did_scale(mask) == v);
    }

    constexpr double
    added_func_scale() const {
        return scale_func_;
    }
    constexpr double
    added_edge_scale() const {
        return scale_edge_;
    }
};

// Create save state at file path
struct perf_state_saver_t {
    const sym::sym_state_t * const state_;

    bool save_state(const char *                 file_path,
                    const vec_t<perf_func_t> *   funcs,
                    const vec_t<perf_edge_t> *   edges,
                    const perf_state_scaling_t * scaling_todo) const;
};

// Reload all save states from the vec for file_paths.
struct perf_state_reloader_t {
    sym::sym_state_t * const state_;
    bool reload_state(const vec_t<std::string_view> * file_paths,
                      vec_t<perf_func_t> *            funcs,
                      vec_t<perf_edge_t> *            edges,
                      perf_state_scaling_t *          scaling_todo) const;

    bool
    reload_state(std::string_view       file_path,
                 vec_t<perf_func_t> *   funcs,
                 vec_t<perf_edge_t> *   edges,
                 perf_state_scaling_t * scaling_todo) const {
        vec_t<std::string_view> paths = { file_path };
        return reload_state(&paths, funcs, edges, scaling_todo);
    }
};

}  // namespace perf
}  // namespace tlo
#endif
