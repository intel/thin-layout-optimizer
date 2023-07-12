#include "src/sym/dso.h"
#include "src/sym/elffile.h"
#include "src/util/global-stats.h"

namespace tlo {
namespace sym {
bool
func_clump_t::is_unknown() const {
    assert(!is_temporary());
    for (const func_t & func : funcs_) {
        if (func.is_unknown()) {
            return true;
        }
    }
    return dso()->is_unknown();
}

bool
func_clump_t::name_lt(const func_clump_t * other) const {
    assert(!is_temporary() && !other->is_temporary());
    assert(is_cg_ready() && other->is_cg_ready());

    if (size() != other->size()) {
        return size() < other->size();
    }
    if (dso() != other->dso()) {
        return dso()->name_.lt(other->dso()->name_);
    }
    if (num_funcs() != other->num_funcs()) {
        return num_funcs() < other->num_funcs();
    }
    for (size_t i = 0; i < num_funcs(); ++i) {
        if (!funcs_[i].name_eq(other->funcs_[i])) {
            return funcs_[i].name_lt(other->funcs_[i]);
        }
    }
#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
    assert(this == other && "Almost certainly messed up de-dup");
#endif
    return false;
}

bool
func_clump_t::is_findable() const {
    assert(!is_temporary());
    if (!dso()->is_findable()) {
        return false;
    }
    // NOLINTNEXTLINE(readability-use-anyofallof,-warnings-as-errors)
    for (const func_t & func : funcs_) {
        if (!func.is_findable()) {
            return false;
        }
    }
    return true;
}

void
func_clump_t::dump(int vlvl, FILE * fp, const char * prefix) const {
    assert(!is_temporary());
    if (has_verbosity(vlvl)) {
        //        std::array<char, dso_t::k_dso_pathlen> path{};
        //        dso()->fmt_path(&path);
        //        fprintf(fp, "Path: %s\n", path.data());
        for (const func_t & func : funcs_) {
            func.dump(vlvl, fp, prefix);
        }
    }
}

void
func_clump_t::merge(func_clump_t * other, bool max_size) {
    assert(!is_temporary());
    assert(!other->is_temporary());
    assert(this != other);
    assert(dso() == other->dso());

    const size_t new_size = num_funcs() + other->num_funcs();
    func_t *     new_ptr  = nullptr;
    if (is_contiguous()) {
        assert(size_ == clumped_range_.size());
        new_ptr = arr_alloc<func_t>(new_size);
        memcpy(new_ptr, funcs_.data(), funcs_.size_bytes());
    }
    else {
        new_ptr = arr_realloc<func_t>(funcs_.data(), num_funcs(), new_size);
    }

    memcpy(new_ptr + num_funcs(), other->funcs_.data(),
           other->funcs_.size_bytes());
    funcs_ = { new_ptr, new_size };
    if (max_size) {
        assert(dso()->from_reload_);
        assert(other->dso()->from_reload_);
        assert(is_unknown() == other->is_unknown());
        set_size(std::max(size(), other->size()));
    }
    else if (overlap(this, other)) {
        assert(!dso()->from_reload_);
        assert(!other->dso()->from_reload_);
        vec_t<addr_range_t> ranges;
        for (const func_t & func : funcs_) {
            ranges.push_back(func.get_addr_range());
        }

        std::sort(std::begin(ranges), std::end(ranges), addr_range_t::cmp_t{});
        for (size_t i = 1; i < ranges.size(); ++i) {
            if (!ranges[i - 1].active() || !ranges[i].active()) {
                continue;
            }
            if (addr_range_t::overlap(ranges[i - 1], ranges[i])) {
                ranges[i].merge(ranges[i - 1]);
                ranges[i - 1] = addr_range_t{};
                assert(!ranges[i - 1].active());
            }
        }
        size_t size = 0;
        for (const addr_range_t & range : ranges) {
            if (!range.active()) {
                continue;
            }
            size += range.size();
        }
        set_size(size);
    }
    else {
        assert(!dso()->from_reload_);
        assert(!other->dso()->from_reload_);
        set_size(other->size() + size());
    }
    unset_contiguous();
    sort_and_rehash();

    assert(!is_contiguous());
}

bool
dso_t::find_debug_file_for_dso(
    std::array<char, k_dso_pathlen> * path_out) const {
    fmt_path(path_out, ".debug");
    if (file_ops::exists(path_out->data())) {
        return true;
    }
    if (has_dso_root_path()) {
        return false;
    }
    if (!has_buildids()) {
        return false;
    }


    const std::string_view sv      = name_.sview();
    const std::string_view fulldir = path_get_fulldir(sv);


    const std::string_view postfix = ".debug";
    tlo::vec_t<char>       path{};

    auto append_str = [](tlo::vec_t<char> * fpath,
                         std::string_view   fpostfix) -> void {
        while (!fpath->empty() && fpath->back() == '\0') {
            fpath->pop_back();
        }
        std::copy(fpostfix.begin(), fpostfix.end(), std::back_inserter(*fpath));
        fpath->push_back('\0');
    };

    bool okay = false;
    for (const auto & buildid : buildids()) {
        std::copy(buildid.str(), buildid.str() + buildid.len(),
                  path_out->begin());

        for (int att = 0; att < 2; ++att) {
            if (att == 1) {
                std::transform(path_out->begin(), path_out->end(),
                               path_out->begin(),
                               [](char c) { return std::tolower(c); });
            }
            const std::string_view sv_bid = { path_out->data(), buildid.len() };
            if (sv_bid.length() > 3) {
                path.clear();
                const std::string_view prefix = "/usr/lib/debug/.build-id/";
                const std::string_view dir    = sv_bid.substr(0, 2);
                const std::string_view file   = sv_bid.substr(2);
                path_join(&path, prefix);
                path_join(&path, dir);
                path_join(&path, file);
                append_str(&path, postfix);
                TLO_printvvv("Testing: %s -> %s\n", str(), path.data());
                if (file_ops::exists(path.data())) {
                    okay = true;
                }
            }
            if (!okay) {
                path.clear();
                const std::string_view prefix = "/usr/lib/debug/";
                path_join(&path, prefix);
                path_join(&path, fulldir);
                path_join(&path, sv_bid);
                append_str(&path, postfix);
                TLO_printvvv("Testing: %s -> %s\n", str(), path.data());
                if (file_ops::exists(path.data())) {
                    okay = true;
                }
            }
            if (!okay) {
                path.clear();
                path_join(&path, fulldir);
                path_join(&path, ".debug");
                path_join(&path, sv_bid);
                append_str(&path, postfix);
                TLO_printvvv("Testing: %s -> %s\n", str(), path.data());
                if (file_ops::exists(path.data())) {
                    okay = true;
                }
            }
            if (!okay) {
                path.clear();
                path_join(&path, fulldir);
                path_join(&path, sv_bid);
                append_str(&path, postfix);
                TLO_printvvv("Testing: %s -> %s\n", str(), path.data());
                if (file_ops::exists(path.data())) {
                    okay = true;
                }
            }

            if (okay) {
                std::copy(path.begin(), path.end(), path_out->begin());
                return true;
            }
        }
    }

    return false;
}


template<bool k_tab_type_unused>
void
dso_t::finalize_from_perf(
    char * str_ptr,  // NOLINT(readability-non-const-parameter)
    strtab_t<k_tab_type_unused> * name_tab) {
    (void)str_ptr;
    all_funcs_ = {};
    deps_      = {};
    std::array<char, k_dso_pathlen> path{};
    fmt_path(&path);
    TLO_INCR_STAT(total_dsos_);
    if (file_ops::exists(path.data())) {
        elf_file_t ef{};
        TLO_printv("Try to read DSO: %s\n", str());
        if (ef.init(path.data())) {
            ef.extract_functions(this, &all_funcs_, &deps_, buildids_,
                                 name_tab);
            TLO_INCR_STAT(total_processed_dsos_);
        }
        else {
        }
        ef.cleanup();
    }
    else {
        set_non_findable();
    }
    if (find_debug_file_for_dso(&path)) {
        TLO_printv("Found Debug For: %s\n\t-> %s\n", str(), path.data());

        elf_file_t ef{};
        if (ef.init(path.data())) {
            ef.extract_functions(this, &all_funcs_, nullptr, nullptr, name_tab);
            TLO_INCR_STAT(total_processed_dso_debugs_);
            has_dbg_ = true;
        }
        ef.cleanup();
    }
    func_clumps_ = create_func_clumps(all_funcs_);
    open_dso();
    finalized_ = true;
}
template void dso_t::finalize_from_perf<true>(char *, strtab_t<true> *);
template void dso_t::finalize_from_perf<false>(char *, strtab_t<false> *);


// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables,bugprone-string-constructor)
std::string_view dso_t::G_dso_root_path = { "", 0 };


}  // namespace sym
}  // namespace tlo
