#include "src/perf/perf-saver.h"
#include "src/perf/perf-stats.h"

#include "src/util/file-ops.h"
#include "src/util/json.h"

#include "src/util/debug.h"

#include <iostream>

#include <time.h>


////////////////////////////////////////////////////////////////////////////////
// Implementation for producing save states
//
// Puts all accumulated information from parsing/processing perf events into
// a json / reloads said information.

namespace tlo {
namespace perf {

// When combining multiple save states, we have option to scale the data by
// either custom factor, or scale all save states s.t there weights will be
// equal (i.e state A has 10 events and state B has 30 events, we would scale
// all event in A by factor of 3).
static constexpr double k_extremely_large_scale_factor = 1000.0 * 1000.0;
struct perf_stats_scaler_t {
    static constexpr double k_func_scale_point_dbl =
        static_cast<double>(k_func_scale_point);
    static constexpr double k_edge_scale_point_dbl =
        static_cast<double>(k_edge_scale_point);

    double scale_;

    bool
    modifies() const {
        return usable() && scale_ != 1.0;
    }

    bool
    usable() const {
        return scale_ != 0.0;
    }

    void
    mul(double scale) {
        scale_ *= scale;
    }


    bool
    init(double scale) {
        scale_ = scale;
        return true;
    }

    bool
    init(const perf_edge_stats_t & pe_stats) {
        if (pe_stats.empty() || pe_stats.num_edges_ == 0) {
            scale_ = 1.0;
            return false;
        }
        scale_ =
            k_edge_scale_point_dbl / static_cast<double>(pe_stats.num_edges_);

        return true;
    }

    bool
    init(const perf_func_stats_t & pf_stats) {
        if (pf_stats.empty() || pf_stats.num_samples_ == 0) {
            scale_ = 1.0;
            return false;
        }
        scale_ =
            k_func_scale_point_dbl / static_cast<double>(pf_stats.num_samples_);

        return true;
    }

    psample_val_t
    scale_val(double v) const {
        assert(usable());
        return static_cast<psample_val_t>(v * scale_);
    }

    void
    scale(perf_edge_stats_t * pe_stats_inout) const {
        assert(usable());
        pe_stats_inout->num_edges_ =
            scale_val(static_cast<double>(pe_stats_inout->num_edges_));
    }
    void
    scale(perf_func_stats_t * pf_stats_inout) const {
        assert(usable());
        pf_stats_inout->num_samples_ =
            scale_val(static_cast<double>(pf_stats_inout->num_samples_));
    }
};

static bool
is_perf_edge_stats_normalized(const perf_edge_stats_t & edge_stats) {
    return edge_stats.num_edges_ == k_edge_scale_point;
}

static bool
is_perf_func_stats_normalized(const perf_func_stats_t & func_stats) {
    return func_stats.num_samples_ == k_func_scale_point;
}

static bool
write_edge_stats(json_t * js_out, const perf_edge_stats_t & edge_stats) {
    // Only "cnt" (weight) for edges
    (*js_out)["edge_stats"]        = json_t{};
    (*js_out)["edge_stats"]["cnt"] = edge_stats.num_edges_;
    return true;
}

static bool
read_edge_stats(const json_t &              js_in,
                perf_edge_stats_t *         edge_stats_out,
                const perf_stats_scaler_t * pe_stats_scaler) {
    if (!js_in.contains("edge_stats")) {
        TLO_TRACE("Missing edge_stats");
        return false;
    }
    const json_t & js_pe_stats = js_in["edge_stats"];
    if (!js_pe_stats.contains("cnt")) {
        TLO_TRACE("Missing cnt");
        return false;
    }

    const double        cnt = js_pe_stats["cnt"];
    perf_stats_scaler_t default_scaler{};
    if (pe_stats_scaler == nullptr) {
        default_scaler.init(1.0);
        pe_stats_scaler = &default_scaler;
        assert(!pe_stats_scaler->modifies());
    }


    *edge_stats_out = { pe_stats_scaler->scale_val(cnt) };
    return true;
}


static bool
write_func_stats(json_t * js_out, const perf_func_stats_t & func_stats) {
    (*js_out)["func_stats"]        = json_t{};
    (*js_out)["func_stats"]["cnt"] = func_stats.num_samples_;
    (*js_out)["func_stats"]["track_br_in"] =
        func_stats.num_tracked_br_samples_in_;
    (*js_out)["func_stats"]["track_br_out"] =
        func_stats.num_tracked_br_samples_out_;
    (*js_out)["func_stats"]["total_br_in"]  = func_stats.num_br_samples_in_;
    (*js_out)["func_stats"]["total_br_out"] = func_stats.num_br_samples_out_;
    return true;
}

static bool
read_func_stats(const json_t &              js_in,
                perf_func_stats_t *         func_stats_out,
                const perf_stats_scaler_t * pf_stats_scaler) {
    if (!js_in.contains("func_stats")) {
        TLO_TRACE("Missing func_stats");
        return false;
    }
    const json_t & js_pf_stats = js_in["func_stats"];
    if (!js_pf_stats.contains("cnt")) {
        TLO_TRACE("Missing cnt");
        return false;
    }
    if (!js_pf_stats.contains("track_br_in")) {
        TLO_TRACE("Missing track_br_in");
        return false;
    }
    if (!js_pf_stats.contains("track_br_out")) {
        TLO_TRACE("Missing track_br_out");
        return false;
    }
    if (!js_pf_stats.contains("total_br_in")) {
        TLO_TRACE("Missing total_br_in");
        return false;
    }
    if (!js_pf_stats.contains("total_br_out")) {
        TLO_TRACE("Missing total_br_out");
        return false;
    }

    const double cnt          = js_pf_stats["cnt"];
    const double track_br_in  = js_pf_stats["track_br_in"];
    const double track_br_out = js_pf_stats["track_br_out"];
    const double total_br_in  = js_pf_stats["total_br_in"];
    const double total_br_out = js_pf_stats["total_br_out"];

    perf_stats_scaler_t default_scaler{};
    if (pf_stats_scaler == nullptr) {
        default_scaler.init(1.0);
        pf_stats_scaler = &default_scaler;
        assert(!pf_stats_scaler->modifies());
    }


    *func_stats_out = { pf_stats_scaler->scale_val(cnt),
                        static_cast<psample_val_t>(track_br_in),
                        static_cast<psample_val_t>(track_br_out),
                        static_cast<psample_val_t>(total_br_in),
                        static_cast<psample_val_t>(total_br_out) };
    return true;
}

static bool
write_perf_func(json_t *                                js_out,
                const perf_func_t &                     pf,
                basic_uset<const sym::func_clump_t *> * all_fcs) {

    (*js_out)["func_uid"] = reinterpret_cast<uintptr_t>(pf.func_clump_);
    all_fcs->emplace(pf.func_clump_);
    return write_func_stats(js_out, pf.stats());
}

static bool
write_perf_edge(json_t *                                js_out,
                const perf_edge_t &                     pe,
                basic_uset<const sym::func_clump_t *> * all_fcs) {
    (*js_out)["func_from_uid"] = reinterpret_cast<uintptr_t>(pe.from_);
    (*js_out)["func_to_uid"]   = reinterpret_cast<uintptr_t>(pe.to_);
    all_fcs->emplace(pe.from_);
    all_fcs->emplace(pe.to_);
    return write_edge_stats(js_out, pe.stats());
}


static bool
write_func(json_t * js_out, const sym::func_t * func) {
    (*js_out)["name"]       = func->name_.sview();
    (*js_out)["ident"]      = func->ident_.sview();
    (*js_out)["ident_meta"] = func->ident_.extra();
    (*js_out)["exact_info"] = func->has_elfinfo();

    return true;
}

static bool
write_func_clump(json_t * js_out, const sym::func_clump_t * fc) {
    (*js_out)["uid"]       = reinterpret_cast<uintptr_t>(fc);
    (*js_out)["dso_uid"]   = reinterpret_cast<uintptr_t>(fc->dso());
    (*js_out)["size"]      = fc->size();
    (*js_out)["num_funcs"] = fc->num_funcs();


    (*js_out)["funcs"] = json_t::array();
    for (const sym::func_t & func : fc->funcs()) {
        json_t js_func{};
        if (!write_func(&js_func, &func)) {
            TLO_TRACE("Failure");
            return false;
        }

        (*js_out)["funcs"].emplace_back(js_func);
    }


    return true;
}


static bool
write_dso(json_t * js_out, const sym::dso_t * dso) {
    (*js_out)["name"]     = dso->name_.sview();
    (*js_out)["uid"]      = reinterpret_cast<uintptr_t>(dso);
    (*js_out)["findable"] = dso->is_findable();
    if (dso->has_buildids()) {
        (*js_out)["buildids"] = json_t::array();
        for (const strbuf_t<> & buildid : dso->buildids()) {
            (*js_out)["buildids"].emplace_back(buildid.sview());
        }
    }
    if (dso->has_comm_uses()) {
        (*js_out)["comm_uses"] = json_t::array();
        for (const strbuf_t<> & comm : dso->comm_uses()) {
            (*js_out)["comm_uses"].emplace_back(comm.sview());
        }
    }
    (*js_out)["deps"] = json_t::array();
    for (const strbuf_t<> & dep : dso->deps()) {
        (*js_out)["deps"].emplace_back(dep.sview());
    }
    return true;
}

static bool
save_state_impl(const char *                 file_path,
                const vec_t<perf_func_t> *   funcs,
                const vec_t<perf_edge_t> *   edges,
                const sym::sym_state_t *     state,
                const perf_state_scaling_t * scaling_todo) {
    json_t js_out{};
    js_out["ver"] = k_perf_state_saver_ver;
    json_t js_scaling{};
    js_scaling["func_normalized"] =
        scaling_todo->did_scale(perf_state_scaling_t::k_func_only);
    js_scaling["edge_normalized"] =
        scaling_todo->did_scale(perf_state_scaling_t::k_edge_only);
    if (scaling_todo->has_add_scale(perf_state_scaling_t::k_func_only)) {
        js_scaling["func_scale"] = scaling_todo->added_func_scale();
    }
    if (scaling_todo->has_add_scale(perf_state_scaling_t::k_edge_only)) {
        js_scaling["edge_scale"] = scaling_todo->added_edge_scale();
    }

    js_out["scaling"] = js_scaling;

    vec_t<stat_counter_t> gbl_stats{};
    global_stats_collect(&gbl_stats);
    json_t js_stats{};
    for (const auto & pair : gbl_stats) {
        js_stats[pair.first] = pair.second;
    }
    js_out["global_stats"] = js_stats;

    {
        TLO_DISABLE_WREDUNDANT_TAGS
        struct tm result;
        TLO_REENABLE_WREDUNDANT_TAGS
        // NOLINTNEXTLINE(*magic*)
        std::array<char, 32> tmpbuf{ "" };
        const time_t         cur_time = time(nullptr);
        if (localtime_r(&cur_time, &result) != nullptr) {
            // NOLINTNEXTLINE(bugprone-unsafe-functions,cert-msc24-c,cert-msc33-c)
            if (asctime_r(&result, tmpbuf.data()) == nullptr) {
                tmpbuf[0] = '\0';
            }
            char * nl = reinterpret_cast<char *>(
                std::memchr(tmpbuf.data(), '\n', tmpbuf.size()));
            if (nl != nullptr) {
                *nl = '\0';
            }
        }
        js_out["timestamp"] = std::string_view{ tmpbuf.data() };
    }
    js_out["dsos"] = json_t::array();
    for (const sym::dso_t * dso : state->dsos()) {
        json_t js_dso_out{};
        if (!write_dso(&js_dso_out, dso)) {
            TLO_TRACE("Failure");
            return false;
        }
        js_out["dsos"].emplace_back(js_dso_out);
    }

    basic_uset<const sym::func_clump_t *> all_fcs{};
    js_out["perf_funcs"] = json_t::array();
    perf_func_stats_t agr_pf_stats{};
    for (const perf_func_t & pf : (*funcs)) {
        agr_pf_stats.add(pf.stats());
        json_t js_pf_out{};
        if (!write_perf_func(&js_pf_out, pf, &all_fcs)) {
            TLO_TRACE("Failure");
            return false;
        }
        js_out["perf_funcs"].emplace_back(js_pf_out);
    }

    js_out["perf_edges"] = json_t::array();
    perf_edge_stats_t agr_pe_stats{};
    for (const perf_edge_t & pe : (*edges)) {
        json_t js_pe_out{};
        agr_pe_stats.add(pe.stats());
        if (!write_perf_edge(&js_pe_out, pe, &all_fcs)) {
            TLO_TRACE("Failure");
            return false;
        }
        js_out["perf_edges"].emplace_back(js_pe_out);
    }
    json_t js_agr_stats{};
    write_func_stats(&js_agr_stats, agr_pf_stats);
    write_edge_stats(&js_agr_stats, agr_pe_stats);
    js_out["perf_aggregate_stats"] = js_agr_stats;


    if (all_fcs.empty()) {
        TLO_TRACE("Failure");
        return false;
    }

    json_t js_fcs_out{};
    js_fcs_out["func_clumps"]     = json_t::array();
    js_fcs_out["num_func_clumps"] = all_fcs.size();
    for (const sym::func_clump_t * fc : all_fcs) {
        json_t js_fc_out{};
        if (!write_func_clump(&js_fc_out, fc)) {
            TLO_TRACE("Failure");
            return false;
        }
        js_fcs_out["func_clumps"].emplace_back(js_fc_out);
    }
    js_out["all_funcs"] = js_fcs_out;


    std::string js_content = js_out.dump(1);
    return file_ops::writefile(
        file_path,
        file_ops::filebuf_t{ reinterpret_cast<uint8_t *>(js_content.data()),
                             js_content.length() },
        O_TRUNC);
}

bool
perf_state_saver_t::save_state(
    const char *                 file_path,
    const vec_t<perf_func_t> *   funcs,
    const vec_t<perf_edge_t> *   edges,
    const perf_state_scaling_t * scaling_todo) const {
#if (defined TLO_MSAN)
    assert(0 && "State saving unsupported w/ MSAN");
#endif
    return save_state_impl(file_path, funcs, edges, state_, scaling_todo);
}

struct uid_t {
    static constexpr uint64_t k_addr_free_bits = 16;
    uint64_t                  v_;
    uid_t(uint64_t addr_uid, uint64_t file_uid)
        : v_((addr_uid << k_addr_free_bits) | file_uid) {}

    uid_t() = default;

    uint64_t
    hash() const {
        return xxhash::run(v_);
    }

    bool
    eq(uid_t other) const {
        return v_ == other.v_;
    }
};


using func_to_fc_map_t = umap<sym::func_t,
                              std::pair<uid_t, sym::func_clump_t *>,
                              sym::func_t::name_hasher_t,
                              sym::func_t::name_equals_t>;

using uid_to_fc_map_t  = basic_umap<uid_t, sym::func_clump_t *>;
using uid_to_dso_map_t = basic_umap<uid_t, sym::dso_t *>;


using pe_set_t = basic_uset<perf_edge_t>;
using pf_set_t = basic_uset<perf_func_t>;

static bool
reload_func_state_from_json(const json_t &      js_func,
                            sym::func_t *       func_out,
                            const sym ::dso_t * dso,
                            sym::sym_state_t *  state) {
    if (!js_func.contains("name")) {
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_func.contains("ident")) {
        TLO_TRACE("Missing: \"ident\"");
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_func.contains("ident_meta")) {
        TLO_TRACE("Missing: \"ident_meta\"");
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_func.contains("exact_info")) {
        TLO_TRACE("Missing: \"exact_info\"");
        TLO_TRACE("Failure");
        return false;
    }

    const std::string name_str   = js_func["name"];
    const std::string ident_str  = js_func["ident"];
    const uint16_t    ident_meta = js_func["ident_meta"];
    const bool        exact      = js_func["exact_info"];

    const sym::ident_t ident = {
        state->get_strtab()->get(std::string_view{ ident_str }).sview(),
        ident_meta
    };
    const strbuf_t<> name_sb =
        state->get_strtab()->get_sbuf(std::string_view{ name_str });

    *func_out =
        sym::func_t{ dso, exact, name_sb, ident, sym::addr_range_t{ 0, 0 } };
    assert(func_out->name_.eq(name_sb));
    assert(func_out->ident_.eq(ident));
    assert(func_out->has_elfinfo() == exact);
    assert(func_out->ident_.extra() == ident_meta);
    return true;
}

TLO_DISABLE_WSTACK_PROTECTOR
static sym::func_clump_t *
reload_fc_state_from_json(const json_t &           js_fc,
                          uint64_t                 file_uid,
                          sym::func_clump_t *      fc_out,
                          uid_to_fc_map_t *        uids_to_fc_map,
                          const uid_to_dso_map_t * uids_to_dso_map,
                          func_to_fc_map_t *       funcs_map,
                          sym::sym_state_t *       state) {

    if (fc_out == nullptr) {
        fc_out = state->get_allocator()->getzT<sym::func_clump_t>();
    }

    if (!js_fc.contains("uid")) {
        TLO_TRACE("Failure");
        return fc_out;
    }
    if (!js_fc.contains("dso_uid")) {
        TLO_TRACE("Failure");
        return fc_out;
    }
    if (!js_fc.contains("size")) {
        TLO_TRACE("Failure");
        return fc_out;
    }
    if (!js_fc.contains("num_funcs")) {
        TLO_TRACE("Failure");
        return fc_out;
    }
    if (!js_fc.contains("funcs")) {
        TLO_TRACE("Failure");
        return fc_out;
    }

    const uint64_t dso_uid          = js_fc["dso_uid"];
    const uid_t    dso_and_file_uid = uid_t{ dso_uid, file_uid };
    auto           dso_res          = uids_to_dso_map->find(dso_and_file_uid);
    assert(dso_res != uids_to_dso_map->end());
    sym::dso_t * dso = dso_res->second;
    assert(dso != nullptr);


    const uint64_t size      = js_fc["size"];
    const uint64_t num_funcs = js_fc["num_funcs"];

    sym::func_t * funcs_mem =
        state->get_allocator()->getz_arr<sym::func_t>(num_funcs);

    size_t i = 0;

    for (const json_t & js_func : js_fc["funcs"]) {
        assert(i < num_funcs);
        if (!reload_func_state_from_json(js_func, funcs_mem + i, dso, state)) {
            TLO_TRACE("Failure");
            return fc_out;
        }
        ++i;
    }
    assert(i == num_funcs);

    *fc_out = sym::func_clump_t{ { funcs_mem, num_funcs },
                                 size,
                                 sym::addr_range_t{ 0, size },
                                 true };

    assert(fc_out->in_dso(dso));
    assert(fc_out->num_funcs() == num_funcs);
    assert(fc_out->size() == size);
    assert(fc_out->is_reloaded());
    assert(fc_out->is_contiguous());

    const uint64_t      addr_uid = js_fc["uid"];
    uid_t               uid      = uid_t{ addr_uid, file_uid };
    sym::func_clump_t * cur_fc   = fc_out;
    for (i = 0; i < num_funcs; ++i) {
        auto res = funcs_map->emplace(
            funcs_mem[i], std::pair<uid_t, sym::func_clump_t *>{ uid, cur_fc });
        if (res.second) {
            continue;
        }

        const uid_t         other_uid = res.first->second.first;
        sym::func_clump_t * other_fc  = res.first->second.second;
        if (other_fc == cur_fc) {
            continue;
        }

        for (size_t j = 0; j < i; ++j) {
            const std::pair<uid_t, sym::func_clump_t *> uid_and_fc{ other_uid,
                                                                    other_fc };
            const sym::func_t                           k = funcs_mem[j];
            (*funcs_map)[k]                               = uid_and_fc;
        }
        other_fc->merge(cur_fc, true);

        auto remap_res = uids_to_fc_map->emplace(uid, other_fc);
        if (!remap_res.second) {
            remap_res.first->second = other_fc;
        }

        cur_fc->cleanup();

        cur_fc = other_fc;
        uid    = other_uid;
    }

    auto res = uids_to_fc_map->emplace(uid, cur_fc);
    if (!res.second) {
        assert(res.first->second == cur_fc);
    }

    if (cur_fc == fc_out) {
        return nullptr;
    }

    const size_t nfuncs = fc_out->num_funcs();
    state->get_allocator()->try_ungetT<sym::func_t>(nfuncs);

    return fc_out;
}
TLO_REENABLE_WSTACK_PROTECTOR

TLO_DISABLE_WSTACK_PROTECTOR
static bool
reload_dso_state_from_json(const json_t &     js_dso,
                           uint64_t           file_uid,
                           uid_to_dso_map_t * uids_to_dso_map,
                           sym::sym_state_t * state) {

    if (!js_dso.contains("name")) {
        TLO_TRACE("Missing: \"name\"");
        TLO_TRACE("Failure");
        return false;
    }

    const std::string name = js_dso["name"];
    sym::dso_t *      dso  = state->get_reloaded_dso(std::string_view{ name });

    if (!js_dso.contains("uid")) {
        TLO_TRACE("Failure");
        return false;
    }

    const uint64_t addr_uid = js_dso["uid"];
    const uid_t    uid{ addr_uid, file_uid };
    assert(uids_to_dso_map->emplace(uid, dso).second);
    assert(dso->from_reload());

    if (!js_dso.contains("findable")) {
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_dso.contains("deps")) {
        TLO_TRACE("Failure");
        return false;
    }

    basic_uset<strbuf_t<>> deps{};
    for (const std::string str : js_dso["deps"]) {
        deps.emplace(state->get_strtab()->get_sbuf(std::string_view{ str }));
    }
    bool findable = js_dso["findable"];

    if (dso->finalized_) {
        findable |= dso->is_findable();
        for (const strbuf_t<> & dep : dso->deps_) {
            deps.emplace(dep);
        }
        if (findable) {
            dso->set_is_findable();
        }

        arr_free<strbuf_t<>>(dso->deps_.data(), dso->deps_.size());
    }

    if (dso->has_comm_uses() && js_dso.contains("comm_uses")) {
        for (const std::string str : js_dso["comm_uses"]) {
            dso->add_comm_use(
                state->get_strtab()->get_sbuf(std::string_view{ str }));
        }
    }
    if (dso->has_buildids() && js_dso.contains("buildids")) {
        for (const std::string str : js_dso["buildids"]) {
            dso->add_buildid(
                state->get_strtab()->get_sbuf(std::string_view{ str }));
        }
    }

    if (!deps.empty()) {
        strbuf_t<> * dep_mem = arr_zalloc<strbuf_t<>>(deps.size());
        size_t       i       = 0;
        for (const strbuf_t<> & dep : deps) {
            dep_mem[i] = dep;
        }
        ++i;
        dso->deps_ = { dep_mem, deps.size() };
    }
    else {
        dso->deps_ = {};
    }

    if (!dso->finalized_) {
        dso->finalize_from_reload(findable);
    }

    assert(dso->finalized_);
    assert(dso->from_reload());
    return true;
}
TLO_REENABLE_WSTACK_PROTECTOR

static bool
reload_pf_stats_from_json(const json_t &              js_pf,
                          uint64_t                    file_uid,
                          const uid_to_fc_map_t *     uids_to_fc_map,
                          pf_set_t *                  funcs_out,
                          const perf_stats_scaler_t & pf_stats_scaler) {
    if (!js_pf.contains("func_uid")) {
        TLO_TRACE("Missing: \"func_uid\"");
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_pf.contains("func_stats")) {
        TLO_TRACE("Missing: \"func_stats\"");
        TLO_TRACE("Failure");
        return false;
    }

    const uint64_t addr_uid = js_pf["func_uid"];
    const uid_t    uid{ addr_uid, file_uid };
    auto           res = uids_to_fc_map->find(uid);
    if (res == uids_to_fc_map->end()) {
        return false;
    }

    sym::func_clump_t * fc = res->second;
    assert(fc != nullptr);

    const perf_func_t pf{ fc, perf_func_stats_t{} };

    auto pf_res = funcs_out->emplace(pf);


    perf_func_stats_t pf_stats{};
    if (!read_func_stats(js_pf, &pf_stats, &pf_stats_scaler)) {
        return false;
    }

    pf_res.first->stats_.add(pf_stats);
    return true;
}

static bool
reload_pe_stats_from_json(const json_t &              js_pe,
                          uint64_t                    file_uid,
                          const uid_to_fc_map_t *     uids_to_fc_map,
                          pe_set_t *                  edges_out,
                          const perf_stats_scaler_t & pe_stats_scaler) {
    if (!js_pe.contains("func_from_uid")) {
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_pe.contains("func_to_uid")) {
        TLO_TRACE("Failure");
        return false;
    }
    if (!js_pe.contains("edge_stats")) {
        TLO_TRACE("Failure");
        return false;
    }

    const uint64_t from_addr_uid = js_pe["func_from_uid"];
    const uint64_t to_addr_uid   = js_pe["func_to_uid"];

    const uid_t from_uid{ from_addr_uid, file_uid };
    const uid_t to_uid{ to_addr_uid, file_uid };

    assert(!from_uid.eq(to_uid));

    auto from_res = uids_to_fc_map->find(from_uid);
    if (from_res == uids_to_fc_map->end()) {
        TLO_TRACE("Failure");
        return false;
    }
    auto to_res = uids_to_fc_map->find(to_uid);
    if (to_res == uids_to_fc_map->end()) {
        TLO_TRACE("Failure");
        return false;
    }


    sym::func_clump_t * from_fc = from_res->second;
    assert(from_fc != nullptr);
    sym::func_clump_t * to_fc = to_res->second;
    assert(to_fc != nullptr);

    // Because of remaping this now a dead-edge
    if (from_fc == to_fc) {
        TLO_print("Warning: Dead edge found at regen!");
        return false;
    }

    assert(from_fc != to_fc);


    const perf_edge_t pe{ from_fc, to_fc, system::br_insn_t::make_bad(),
                          perf_edge_stats_t{} };
    assert(pe.from_ != pe.to_);

    auto pe_res = edges_out->emplace(pe);


    perf_edge_stats_t pe_stats{};
    if (!read_edge_stats(js_pe, &pe_stats, &pe_stats_scaler)) {
        return false;
    }
    pe_res.first->stats_.add(pe_stats);
    return true;
}

static bool
reload_state_from_func_clumps(const json_t &           js_in,
                              uint64_t                 file_uid,
                              uid_to_fc_map_t *        uids_to_fc_map,
                              const uid_to_dso_map_t * uids_to_dso_map,
                              func_to_fc_map_t *       funcs_map,
                              sym::sym_state_t *       state) {
    if (!js_in.contains("func_clumps")) {
        TLO_TRACE("Failure");
        return false;
    }

    bool                ret        = false;
    sym::func_clump_t * fc_realloc = nullptr;
    for (const json_t & js_fc : js_in["func_clumps"]) {
        fc_realloc = reload_fc_state_from_json(js_fc, file_uid, fc_realloc,
                                               uids_to_fc_map, uids_to_dso_map,
                                               funcs_map, state);
        if (fc_realloc == nullptr) {
            ret = true;
        }
    }
    return ret;
}


bool
perf_state_reloader_t::reload_state(const vec_t<std::string_view> * file_paths,
                                    vec_t<perf_func_t> *            funcs_out,
                                    vec_t<perf_edge_t> *            edges_out,
                                    perf_state_scaling_t * scaling_todo) const {

#if (defined TLO_MSAN)
    assert(0 && "State saving unsupported w/ MSAN");
#endif
    perf_state_scaling_t default_scaling_todo{};
    if (scaling_todo == nullptr) {
        scaling_todo = &default_scaling_todo;
    }
    TLO_TRACE("At reload\n");
    bool          ret = false;
    vec_t<json_t> js_inputs{};
    js_inputs.reserve(file_paths->size());
    for (const std::string_view & sview : *file_paths) {
        TLO_printv("Reload From: %s\n", sview.data());
        TLO_TRACE("Parseing\n");
        file_ops::filebuf_t content = file_ops::readfile(sview.data());
        if (!content.active()) {
            TLO_perr("Warning: Unable to load save-state from: %s\n",
                     sview.data());
            continue;
        }
        js_inputs.emplace_back(json_t::parse(std::string_view{
            reinterpret_cast<const char *>(content.data()), content.size() }));

        content.cleanup();
    }
    for (const json_t & js_in : js_inputs) {
        if (js_in.contains("global_stats")) {
            const json_t & js_gstats = js_in["global_stats"];
            for (auto it = js_gstats.begin(); it != js_gstats.end(); ++it) {
                const uint64_t val = js_gstats[it.key()];
                global_stats_reload(stat_counter_t{ it.key().c_str(), val });
            }
        }
    }

    uid_to_fc_map_t  uids_to_fc_map{};
    uid_to_dso_map_t uids_to_dso_map{};
    func_to_fc_map_t funcs_map{};
    pf_set_t         all_pf{};
    pe_set_t         all_pe{};

    bool did_func_scale = false;
    bool did_edge_scale = false;

    uint64_t file_uid = 0;
    TLO_TRACE("Doing DSOS");
    for (const json_t & js_in : js_inputs) {
        if (js_in.contains("dsos")) {
            for (const json_t & js_dso : js_in["dsos"]) {
                ret |= reload_dso_state_from_json(js_dso, file_uid,
                                                  &uids_to_dso_map, state_);
            }
        }
        ++file_uid;
    }

    TLO_TRACE("Doing Funcs");
    file_uid = 0;
    // reload functions first so we can create edges as we reload them.
    for (const json_t & js_in : js_inputs) {
        if (js_in.contains("all_funcs")) {
            ret |= reload_state_from_func_clumps(
                js_in["all_funcs"], file_uid, &uids_to_fc_map, &uids_to_dso_map,
                &funcs_map, state_);
        }
        ++file_uid;
    }

    file_uid = 0;
    TLO_TRACE("Doing Perf Info");
    for (const json_t & js_in : js_inputs) {
        bool   is_func_scaled     = false;
        bool   is_edge_scaled     = false;
        double local_func_scaling = 1.0;
        double local_edge_scaling = 1.0;
        if (js_in.contains("scaling")) {
            if (js_in["scaling"].contains("func_normalized")) {
                is_func_scaled = js_in["scaling"]["func_normalized"];
            }
            if (js_in["scaling"].contains("edge_normalized")) {
                is_edge_scaled = js_in["scaling"]["edge_normalized"];
            }
            if (js_in["scaling"].contains("scale")) {
                local_func_scaling = js_in["scaling"]["scale"];
                local_edge_scaling = js_in["scaling"]["scale"];
            }
            else {
                if (js_in["scaling"].contains("func_scale")) {
                    local_func_scaling = js_in["scaling"]["func_scale"];
                }

                if (js_in["scaling"].contains("edge_scale")) {
                    local_edge_scaling = js_in["scaling"]["edge_scale"];
                }
            }
        }
        if (local_func_scaling <= 0.0) {
            TLO_perr(
                "Func scaling is invalid:\n\t%s -> %lf\nDefaulting to 1.0\n",
                (*file_paths)[file_uid].data(), local_func_scaling);
            local_func_scaling = 1.0;
        }
        if (local_edge_scaling <= 0.0) {
            TLO_perr(
                "Edge scaling is invalid:\n\t%s -> %lf\nDefaulting to 1.0\n",
                (*file_paths)[file_uid].data(), local_edge_scaling);
            local_edge_scaling = 1.0;
        }

        TLO_TRACE("Doing Perf Funcs");

        perf_func_stats_t agr_pf_stats{};
        perf_edge_stats_t agr_pe_stats{};
        if (scaling_todo->should_scale(perf_state_scaling_t::k_func_only)) {
            if (!js_in.contains("perf_aggregate_stats") ||
                !read_func_stats(js_in["perf_aggregate_stats"], &agr_pf_stats,
                                 nullptr)) {

                for (const json_t & js_pf : js_in["perf_funcs"]) {
                    perf_func_stats_t pf_stats{};
                    if (!read_func_stats(js_pf, &pf_stats, nullptr)) {
                        TLO_perr(
                            "Unable to aggregate perf function stats for normalization\n\t%s\n",
                            (*file_paths)[file_uid].data());
                        agr_pf_stats = {};
                        break;
                    }
                    agr_pf_stats.add(pf_stats);
                }
            }
        }
        perf_stats_scaler_t pf_stats_scaler{};
        if (!pf_stats_scaler.init(agr_pf_stats)) {
            if (scaling_todo->should_scale(perf_state_scaling_t::k_func_only)) {
                TLO_perr("Unable to normalize perf function stats\n\t%s\n",
                         (*file_paths)[file_uid].data());
            }
        }

        if (scaling_todo->should_scale(perf_state_scaling_t::k_edge_only)) {
            if (!js_in.contains("perf_aggregate_stats") ||
                !read_edge_stats(js_in["perf_aggregate_stats"], &agr_pe_stats,
                                 nullptr)) {

                for (const json_t & js_pe : js_in["perf_edges"]) {
                    perf_edge_stats_t pe_stats{};
                    if (!read_edge_stats(js_pe, &pe_stats, nullptr)) {
                        TLO_perr(
                            "Unable to aggregate perf edge stats for normalization\n\t%s\n",
                            (*file_paths)[file_uid].data());
                        agr_pe_stats = {};
                        break;
                    }
                    agr_pe_stats.add(pe_stats);
                }
            }
        }
        perf_stats_scaler_t pe_stats_scaler{};
        if (!pe_stats_scaler.init(agr_pe_stats)) {
            if (scaling_todo->should_scale(perf_state_scaling_t::k_edge_only)) {
                TLO_perr("Unable to normalize perf edge stats\n\t%s\n",
                         (*file_paths)[file_uid].data());
            }
        }

        if (!pf_stats_scaler.modifies() && is_func_scaled &&
            !is_perf_func_stats_normalized(agr_pf_stats)) {
            TLO_perr(
                "Normalization is disabled but save state has already been normalized\n\t%s\n",
                (*file_paths)[file_uid].data());
            if (!scaling_todo->force_no_scale(
                    perf_state_scaling_t::k_func_only)) {
                return false;
            }
        }

        if (!pe_stats_scaler.modifies() && is_edge_scaled &&
            !is_perf_edge_stats_normalized(agr_pe_stats)) {
            TLO_perr(
                "Normalization is disabled but save state has already been normalized\n\t%s\n",
                (*file_paths)[file_uid].data());
            if (!scaling_todo->force_no_scale(
                    perf_state_scaling_t::k_edge_only)) {
                return false;
            }
        }

        did_func_scale |= (is_func_scaled || pf_stats_scaler.modifies());
        did_edge_scale |= (is_edge_scaled || pe_stats_scaler.modifies());


        if (scaling_todo->use_local_scale(perf_state_scaling_t::k_func_only)) {
            if (local_func_scaling > k_extremely_large_scale_factor) {
                TLO_perr("Custom function scaling is very high: %lf\n",
                         local_func_scaling);
            }
            pf_stats_scaler.mul(local_func_scaling);
        }
        if (scaling_todo->use_local_scale(perf_state_scaling_t::k_edge_only)) {
            if (local_func_scaling > k_extremely_large_scale_factor) {
                TLO_perr("Custom edge scaling is very high: %lf\n",
                         local_edge_scaling);
            }
            pe_stats_scaler.mul(local_edge_scaling);
        }


        assert(pe_stats_scaler.usable() && pe_stats_scaler.usable());
        assert(
            scaling_todo->should_scale(perf_state_scaling_t::k_func_only) ||
            (scaling_todo->use_local_scale(perf_state_scaling_t::k_func_only) ||
             local_func_scaling != 1.0) ||
            !pf_stats_scaler.modifies());
        assert(
            scaling_todo->should_scale(perf_state_scaling_t::k_edge_only) ||
            (scaling_todo->use_local_scale(perf_state_scaling_t::k_edge_only) &&
             local_edge_scaling != 1.0) ||
            !pe_stats_scaler.modifies());


        if (js_in.contains("perf_funcs")) {
            for (const json_t & js_pf : js_in["perf_funcs"]) {
                ret |= reload_pf_stats_from_json(
                    js_pf, file_uid, &uids_to_fc_map, &all_pf, pf_stats_scaler);
            }
        }
        else {
            TLO_perr("No perf function stats in save state: %s\n",
                     (*file_paths)[file_uid].data());
        }

        TLO_TRACE("Doing Perf Edges");

        if (js_in.contains("perf_edges")) {
            for (const json_t & js_pe : js_in["perf_edges"]) {
                ret |= reload_pe_stats_from_json(
                    js_pe, file_uid, &uids_to_fc_map, &all_pe, pe_stats_scaler);
            }
        }
        else {
            TLO_perr("No perf edges stats in save state: %s\n",
                     (*file_paths)[file_uid].data());
        }
        ++file_uid;
    }


    funcs_out->clear();
    funcs_out->reserve(all_pf.size());
    edges_out->clear();
    edges_out->reserve(all_pe.size());


    basic_uset<const sym::func_clump_t *> all_fc{};
    for (const perf_func_t & pf : all_pf) {
        funcs_out->emplace_back(pf);
        if (all_fc.emplace(pf.func_clump_).second) {
            state_->add_fc_to_free(pf.func_clump_);
        }
    }

    for (const perf_edge_t & pe : all_pe) {
        assert(pe.from_ != pe.to_);
        edges_out->emplace_back(pe);
        if (all_fc.emplace(pe.from_).second) {
            state_->add_fc_to_free(pe.from_);
        }
        if (all_fc.emplace(pe.to_).second) {
            state_->add_fc_to_free(pe.to_);
        }
    }

    scaling_todo->set_did_scale(did_func_scale,
                                perf_state_scaling_t::k_func_only);
    scaling_todo->set_did_scale(did_edge_scale,
                                perf_state_scaling_t::k_edge_only);


    return ret;
}

}  // namespace perf
}  // namespace tlo
