#ifndef SRC_D_SYM_D_DSO_H_
#define SRC_D_SYM_D_DSO_H_

////////////////////////////////////////////////////////////////////////////////
// DSO symbols. Each DSO is has an array of function clumps that we lookup perf
// event locations in. Note each function clump is a set of multiple functions
// that a clumped together. DSOs clump together functions that have overlapping
// addresses. Functions are further clumped if they have overlapping names
// (later before CFG generation).

#include "src/sym/func.h"
#include "src/util/file-ops.h"
#include "src/util/memory.h"
#include "src/util/path.h"
#include "src/util/strbuf.h"
#include "src/util/type-info.h"


// #define TLO_DEBUG
#include "src/util/debug.h"


#include <charconv>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

namespace tlo {
namespace sym {


struct func_clump_t {
    static constexpr uint8_t k_contiguous = 1;
    static constexpr uint8_t k_temporary  = 2;
    static constexpr uint8_t k_reloaded   = 4;
    static constexpr uint8_t k_cg_ready   = 8;


    std::span<func_t> funcs_;
    addr_range_t      clumped_range_;
    uint64_t          size_;
    uint64_t          hash_;
    uint8_t           contiguous_or_temporary_;
    constexpr func_clump_t() = default;
    func_clump_t(std::span<func_t> funcs, addr_range_t clumped_range)
        : funcs_(funcs),
          clumped_range_(clumped_range),
          size_(clumped_range.size()),
          contiguous_or_temporary_(k_contiguous) {
        sort_and_rehash();
        assert(!is_temporary());
    }

    func_clump_t(std::span<func_t> funcs,
                 uint64_t          size,
                 addr_range_t      clumped_range,
                 bool              reloaded)
        : funcs_(funcs),
          clumped_range_(clumped_range),
          size_(size),
          contiguous_or_temporary_(
              k_contiguous | static_cast<uint8_t>(reloaded ? k_reloaded : 0u)) {
        sort_and_rehash();
        assert(!is_temporary());
    }

    func_clump_t(func_t * func)
        : funcs_({ func, 1 }),
          clumped_range_({}),
          size_(0),
          contiguous_or_temporary_(k_temporary) {
        sort_and_rehash();
        assert(is_temporary());
    }

    void
    cleanup() const {
        assert(!is_temporary());
        if (!is_contiguous()) {
            arr_free<func_t>(funcs_.data(), num_funcs());
        }
    }

    func_t *
    first() const {
        return funcs_.data();
    }


    constexpr bool
    is_cg_ready() const {
        return (contiguous_or_temporary_ & k_cg_ready) != 0;
    }

    void
    set_cg_ready() {
        contiguous_or_temporary_ |= k_cg_ready;
    }

    void
    cg_prepare() {
        assert(!is_cg_ready());

        size_t new_nfuncs = 1;
        funcs_[0].cg_prepare();
        for (size_t i = 1; i < num_funcs(); ++i) {
            funcs_[i].cg_prepare();
            if (funcs_[new_nfuncs - 1].name_eq(funcs_[i])) {
                continue;
            }

            else if (i != new_nfuncs) {
                funcs_[new_nfuncs] = funcs_[i];
            }
            new_nfuncs += 1;
        }

        funcs_ = { funcs_.data(), new_nfuncs };

        set_cg_ready();
        assert(is_cg_ready());
    }


    void
    sort_and_rehash() {
        assert(!is_cg_ready());
        std::sort(std::begin(funcs_), std::end(funcs_), func_t::name_cmp_t{});
        uint64_t hv = 0;
        for (const func_t & func : funcs_) {
            TLO_DISABLE_WARITH_CONVERSION
            hv ^= rol(func.hash(), ((hv & 7UL) + 1UL));
            TLO_REENABLE_WARITH_CONVERSION
        }
        hash_ = hv;
    }

    template<bool k_tab_type_unused>
    void
    finalize(char * funcp, strtab_t<k_tab_type_unused> * func_and_ident_tab) {
        assert(!is_cg_ready());
        assert(is_temporary() && is_single());
        func_t * func = new (funcp) func_t{ *first() };

        funcs_                   = { func, 1 };
        clumped_range_           = func->get_addr_range();
        size_                    = clumped_range_.size();
        contiguous_or_temporary_ = k_contiguous;
        assert(clumped_range_.contains(func->get_addr_range()) ||
               clumped_range_.eq(func->get_addr_range()));
        func->finalize(nullptr, func_and_ident_tab);
        sort_and_rehash();
        assert(!is_temporary());
        assert(is_contiguous());
        assert(is_single());
    }

    size_t
    extra_size() const {
        assert(!is_cg_ready());
        assert(is_temporary());
        // TODO: Handle alignment
        static_assert(alignof(func_t) <= alignof(func_clump_t));
        return first()->extra_size() + sizeof(func_t);
    }


    void merge(func_clump_t * other, bool max_size = false);

    constexpr bool
    contains(addr_range_t loc) const {
        assert(!is_cg_ready());
        assert(!is_temporary());
        if (is_contiguous()) {
            return get_addr_range().contains(loc);
        }
        for (const func_t & func : funcs_) {
            if (func.get_addr_range().contains(loc)) {
                return true;
            }
        }
        return false;
    }

    static bool
    overlap(const func_clump_t * fc, addr_range_t loc) {
        assert(!fc->is_cg_ready());
        assert(!fc->is_temporary());
        if (fc->is_unknown()) {
            return false;
        }

        if (fc->is_contiguous()) {
            return addr_range_t::overlap(fc->get_addr_range(), loc);
        }
        for (const func_t & func : fc->funcs_) {
            if (addr_range_t::overlap(func.get_addr_range(), loc)) {
                return true;
            }
        }
        return false;
    }

    static bool
    overlap(const func_clump_t * lhs, const func_clump_t * rhs) {
        assert(!lhs->is_cg_ready());
        assert(!rhs->is_cg_ready());
        assert(!lhs->is_temporary());
        assert(!rhs->is_temporary());

        if (lhs->is_unknown() || rhs->is_unknown()) {
            return lhs == rhs;
        }

        if (lhs->dso() != rhs->dso()) {
            return false;
        }
        if (lhs->is_contiguous()) {
            return overlap(rhs, lhs->get_addr_range());
        }
        if (rhs->is_contiguous()) {
            return overlap(lhs, rhs->get_addr_range());
        }
        for (const func_t & lfunc : lhs->funcs_) {
            for (const func_t & rfunc : rhs->funcs_) {
                if (addr_range_t::overlap(lfunc.get_addr_range(),
                                          rfunc.get_addr_range())) {
                    return true;
                }
            }
        }
        return false;
    }

    void dump(int vlvl = 1, FILE * fp = stdout, const char * prefix = "") const;
    void
    add_sample_addr(uint64_t addr) {
        assert(!is_cg_ready());
        assert(is_contiguous());
        assert(!is_temporary());
        if (!first()->has_elfinfo()) {
            assert(num_funcs() == 1);
            first()->add_sample_addr(addr);
            clumped_range_.merge(first()->get_addr_range());
            size_ = clumped_range_.size();
        }
    }


    std::string_view
    label(vec_t<char> * buf, std::string_view postfix = {}) const {
        assert(!is_temporary());
        size_t i, e;
        for (i = 0, e = num_funcs(); i < e;) {
            funcs_[i].label(buf, postfix);
            ++i;
            if (i == e) {
                break;
            }
            while (!buf->empty() && buf->back() == '\0') {
                buf->pop_back();
            }
            buf->emplace_back('\n');
        }
        return { buf->data(), buf->size() - 1 };
    }
    constexpr bool
    is_temporary() const {
        return (contiguous_or_temporary_ & k_temporary) != 0;
    }
    constexpr bool
    is_contiguous() const {
        return (contiguous_or_temporary_ & k_contiguous) != 0;
    }
    constexpr void
    unset_contiguous() {
        contiguous_or_temporary_ &= static_cast<uint8_t>(~k_contiguous);
    }
    constexpr bool
    is_reloaded() const {
        return (contiguous_or_temporary_ & k_reloaded) != 0;
    }
    constexpr void
    set_size(size_t sz) {
        assert(!is_cg_ready());
        size_ = sz;
    }
    constexpr bool
    is_single() const {
        return num_funcs() == 1;
    }
    constexpr uint64_t
    num_funcs() const {
        return funcs_.size();
    }
    constexpr std::span<func_t>
    funcs() const {
        return funcs_;
    }

    constexpr uint64_t
    size() const {
        assert(!is_temporary());
        if (is_contiguous() && !is_cg_ready()) {
            assert(size_ == get_addr_range().size());
        }
        return size_;
    }

    constexpr addr_range_t
    get_addr_range() const {
        assert(!is_cg_ready());
        assert(!is_temporary());
        assert(is_contiguous());
        return clumped_range_;
    }

    constexpr bool
    eq(const func_clump_t * other) const {
        assert(!is_temporary() && !other->is_temporary());
        return this == other;
    }

    bool name_lt(const func_clump_t * other) const;

    bool is_unknown() const;
    bool is_findable() const;

    bool
    in_dso(const dso_t * dso) const {
        assert(!is_temporary());
        return first()->in_dso(dso);
    }

    constexpr bool
    has_elfinfo() const {
        assert(!is_temporary());
        for (const func_t & func : funcs_) {
            if (!func.has_elfinfo()) {
                return false;
            }
        }
        return true;
    }

    dso_t const *
    dso() const {
        return first()->dso();
    }


    bool
    valid() const {
        assert(!is_temporary());
        size_t        agr_size = 0;
        const dso_t * dso      = first()->dso();
        for (const func_t & func : funcs_) {
            if (func.is_unknown()) {
                if (num_funcs() != 1 && (!is_reloaded() || is_cg_ready())) {
                    TLO_TRACE("Invalid unknown func 1\n");
                    return false;
                }
                if (has_elfinfo()) {
                    TLO_TRACE("Invalid unknown func 2\n");
                    return false;
                }
                if (!is_contiguous() && !is_reloaded()) {
                    TLO_TRACE("Invalid unknown func 3\n");
                    return false;
                }
            }
            if (!func.valid(is_cg_ready(), is_reloaded())) {
                TLO_TRACE("Invalid inner func\n");
                return false;
            }
            if (dso != func.dso()) {
                TLO_TRACE("Mismatch DSO\n");
                return false;
            }
            if (!is_cg_ready() && !is_reloaded()) {
                if (is_contiguous()) {
                    if (!get_addr_range().contains(func.get_addr_range()) &&
                        !get_addr_range().eq(func.get_addr_range())) {
                        TLO_TRACE(
                            "Uncontained(%zu, %d): [%lx, %lx) in [%lx, %lx)\n",
                            num_funcs(), has_elfinfo(),
                            func.get_addr_range().lo_addr_inclusive_,
                            func.get_addr_range().hi_addr_exclusive_,
                            get_addr_range().lo_addr_inclusive_,
                            get_addr_range().hi_addr_exclusive_);
                        return false;
                    }
                }
                agr_size += func.size();
            }
        }
        if (!is_cg_ready() && !is_reloaded() && agr_size < size()) {
            TLO_TRACE("Invalid size: %zu vs %zu\n", agr_size, size());
            return false;
        }
        if (size() == 0) {
            TLO_TRACE("Invalid zero size\n");
            return false;
        }
        if (!std::is_sorted(std::begin(funcs_), std::end(funcs_),
                            func_t::name_cmp_t{})) {
            TLO_TRACE("Functions not sorted\n");
            return false;
        }

        return true;
    }

    constexpr bool
    name_eq(const func_clump_t & other) const {
        if (hash() != other.hash()) {
            return false;
        }
        if (dso() != other.dso()) {
            return false;
        }
        if (other.funcs_.size() != funcs_.size()) {
            return false;
        }
        for (size_t i = 0; i < funcs_.size(); ++i) {
            if (!other.funcs_[i].name_eq(funcs_[i])) {
                return false;
            }
        }
        return true;
    }

    constexpr uint64_t
    hash() const {
        return hash_;
    }
};

struct dso_t {
    // Name of this DSO + meta bit for if we can access its file (its
    // findable).
    using comms_set_t    = basic_uset<strbuf_t<>>;
    using buildids_set_t = basic_uset<strbuf_t<>>;

    static constexpr size_t k_dso_pathlen = PATH_MAX;
    static constexpr size_t k_dso_extlen  = 16;
    strbuf_t<1>             name_;
    std::span<func_clump_t> func_clumps_;
    std::span<strbuf_t<>>   deps_;
    std::span<func_t>       all_funcs_;
    comms_set_t *           comms_;
    buildids_set_t *        buildids_;
    bool                    has_dbg_;
    bool                    from_reload_;
    bool                    finalized_;
    int                     fd_;
    static std::string_view G_dso_root_path;


    constexpr dso_t() = default;
    constexpr dso_t(strbuf_t<> name, bool from_rematerialize)
        : name_(name),
          func_clumps_({}),
          deps_({}),
          all_funcs_({}),
          comms_(nullptr),
          buildids_(nullptr),
          has_dbg_(false),
          from_reload_(from_rematerialize),
          finalized_(false),
          fd_(0) {}

    void
    cleanup() const {
        if (fd_ > 0) {
            close(fd_);
        }
        if (!func_clumps_.empty()) {
            for (size_t i = 0; i < func_clumps_.size(); ++i) {
                func_clumps_[i].cleanup();
            }
            arr_free(func_clumps_.data(), func_clumps_.size());
        }
        if (!all_funcs_.empty()) {
            arr_free(all_funcs_.data(), all_funcs_.size());
        }
        if (!deps_.empty()) {
            arr_free(deps_.data(), deps_.size());
        }

        comms_->~comms_set_t();
        buildids_->~buildids_set_t();
    }

    constexpr size_t
    num_func_clumps() const {
        return func_clumps_.size();
    }

    constexpr size_t
    num_func_refs() const {
        return all_funcs_.size();
    }

    constexpr size_t
    num_deps() const {
        return deps_.size();
    }

    constexpr std::span<func_clump_t>
    func_clumps() const {
        return func_clumps_;
    }

    constexpr std::span<strbuf_t<>>
    deps() const {
        return deps_;
    }


    void
    add_comm_use(strbuf_t<> comm) {
        if (comms_ != nullptr) {
            comms_->emplace(comm);
        }
    }
    const comms_set_t &
    comm_uses() const {
        assert(has_comm_uses());
        return *(comms_);
    }

    size_t
    num_comm_uses() const {
        assert(has_comm_uses());
        return comms_->size();
    }

    bool
    has_comm_uses() const {
        return comms_ != nullptr;
    }

    constexpr char const *
    str() const {
        return name_.str();
    }

    constexpr size_t
    slen() const {
        return name_.len();
    }

    bool
    has_buildids() const {
        return buildids_ != nullptr;
    }

    size_t
    num_buildids() const {
        assert(has_buildids());
        return buildids_->size();
    }
    const buildids_set_t &
    buildids() const {
        assert(has_buildids());
        return *(buildids_);
    }

    void
    add_buildid(strbuf_t<> buildid) {
        if (buildids_ != nullptr) {
            buildids_->emplace(buildid);
        }
    }

    func_clump_t *
    lookup_func_clump(addr_range_t loc) const {
        auto res = addr_range_t::find_closest(std::begin(func_clumps_),
                                              std::end(func_clumps_), loc);

        if (res == std::end(func_clumps_)) {
            return nullptr;
        }
        return &(*res);
    }

    static constexpr size_t
    comms_set_align() {
        return alignof(comms_set_t);
    }

    static constexpr size_t
    buildids_set_align() {
        return alignof(buildids_set_t);
    }


    constexpr size_t
    extra_size() const {
        return sizeof(comms_set_t) + comms_set_align() +
               sizeof(buildids_set_t) + buildids_set_align() + name_.len() + 1;
    }


    std::string_view
    filename() const {
        return path_get_filename(name_.sview());
    }

    std::string_view
    complete_filename() const {
        return name_.sview();
    }

    static std::span<func_clump_t>
    create_func_clumps(std::span<func_t> funcs) {
        if (funcs.empty()) {
            return {};
        }

        std::sort(std::begin(funcs), std::end(funcs), addr_range_t::cmp_t{});
        size_t i, e;

        // Align addresses to 16-bytes. This isn't necessary (and shouldn't
        // be done at all), but here is enough inprecision in
        // elfs/dwarf/etc... that it helps with attribution of location.
        funcs[0].loc_.lo_addr_inclusive_ &= -16UL;
        for (i = 0, e = funcs.size(); (i + 1) < e; ++i) {
            func_t * func      = &funcs[i];
            func_t * func_next = &funcs[i + 1];
            assert(!func->get_addr_range().single());
            assert(!func_next->get_addr_range().single());

            if (addr_range_t::overlap(func->get_addr_range(),
                                      func_next->get_addr_range())) {
                continue;
            }

            uint64_t func_end = func->loc_.hi_addr_exclusive_;
            uint64_t func_next_begin =
                func_next->get_addr_range().lo_addr_inclusive_;

            func_end += 15U;
            func_end &= -16UL;
            if (func_next_begin >= func_end) {
                func->loc_.hi_addr_exclusive_ = func_end;
            }
            assert(!addr_range_t::overlap(func->get_addr_range(),
                                          func_next->get_addr_range()));

            func_next_begin &= -16UL;
            if (func_next_begin >= func_end) {
                func_next->loc_.lo_addr_inclusive_ = func_next_begin;
            }
            assert(!addr_range_t::overlap(func->get_addr_range(),
                                          func_next->get_addr_range()));
        }

        func_clump_t * clumps  = arr_alloc<func_clump_t>(funcs.size());
        size_t         nclumps = 0;

        size_t       last          = 0;
        addr_range_t clumped_range = funcs[0].get_addr_range();
        for (i = 1, e = funcs.size(); i < e; ++i) {
            TLO_DEBUG_ONLY(assert(funcs[i].get_addr_range().complete() ||
                                  funcs[i].get_addr_range().active()));
            TLO_DEBUG_ONLY(assert(funcs[i].has_elfinfo()));
            addr_range_t next_range = funcs[i].get_addr_range();
            if (addr_range_t::overlap(clumped_range, next_range)) {
                clumped_range.merge(next_range);
            }
            else {
                clumps[nclumps] =
                    func_clump_t{ { funcs.data() + last, i - last },
                                  clumped_range };
                clumped_range = next_range;
                last          = i;
                ++nclumps;
            }
        }

        clumps[nclumps] =
            func_clump_t{ { funcs.data() + last, i - last }, clumped_range };
        ++nclumps;
        if (nclumps != funcs.size()) {
            clumps = arr_realloc<func_clump_t>(clumps, funcs.size(), nclumps);
        }

        return { clumps, nclumps };
    }

    std::span<const char>
    fmt_path(std::array<char, k_dso_pathlen> * path,
             std::string_view                  postfix = "") const {
        std::string_view root_path = get_dso_root_path();
        size_t           off       = 0;
        if ((root_path.length() + slen() + k_dso_extlen + postfix.length()) >=
            path->size()) {
            return std::span<const char>{ "", 0 };
        }
        std::memcpy(path->data() + off, root_path.data(), root_path.length());
        off += root_path.length();
        std::memcpy(path->data() + off, str(), slen());
        off += slen();
        if (!postfix.empty()) {
            std::memcpy(path->data() + off, postfix.data(), postfix.length());
            off += postfix.length();
        }
        std::memset(path->data() + off, '\0', 1);
        off += 1;

        return { path->data(), off };
    }

    template<bool k_tab_type_unused>
    void
    finalize(char * str_ptr, strtab_t<k_tab_type_unused> * name_tab) {
        if (comms_ == nullptr) {
            if (comms_set_align() != 0) {
                uintptr_t misalignment =
                    (-reinterpret_cast<uintptr_t>(str_ptr)) %
                    alignof(comms_set_t);
                str_ptr += misalignment;
            }
            comms_ = new (str_ptr) comms_set_t{};
            str_ptr += sizeof(comms_set_t);
        }
        if (buildids_ == nullptr) {
            if (buildids_set_align() != 0) {
                uintptr_t misalignment =
                    (-reinterpret_cast<uintptr_t>(str_ptr)) %
                    alignof(buildids_set_t);
                str_ptr += misalignment;
            }
            buildids_ = new (str_ptr) buildids_set_t{};
            str_ptr += sizeof(buildids_set_t);
        }

        name_.save_to(str_ptr);
        assert(is_findable());
        if (!from_reload_) {
            TLO_DISABLE_WUNDEFINED_FUNC_TEMPLATE
            finalize_from_perf<k_tab_type_unused>(str_ptr, name_tab);
            TLO_REENABLE_WUNDEFINED_FUNC_TEMPLATE
        }
    }

    void
    finalize_from_reload(bool findable) {
        if (findable) {
            set_is_findable();
        }
        else {
            set_non_findable();
        }

        finalized_ = true;
    }
    bool find_debug_file_for_dso(std::array<char, k_dso_pathlen> * path) const;
    template<bool k_tab_type_unused>
    void finalize_from_perf(char *                        str_ptr,
                            strtab_t<k_tab_type_unused> * name_tab);

    constexpr void
    set_non_findable() {
        name_.set_extra(1);
    }

    constexpr void
    set_is_findable() {
        name_.set_extra(0);
    }
    constexpr bool
    is_findable() const {
        return name_.extra() == 0;
    }

    bool
    open_dso() {
        if (fd_ <= 0) {
            if (!is_findable()) {
                return false;
            }
            std::array<char, k_dso_pathlen> path{};
            fmt_path(&path);
            fd_ = open(path.data(), O_RDONLY);
            if (fd_ < 0) {
                return false;
            }
        }
        return true;
    }
    void
    close_dso() {
        if (fd_ > 0) {
            close(fd_);
            fd_ = 0;
        }
    }

    bool
    read_insn(uint64_t addr, std::span<uint8_t> insn_bytes) const {
        assert(!from_reload_);
        std::memset(insn_bytes.data(), 0, insn_bytes.size());
        if (fd_ <= 0) {
            return false;
        }
        // TODO: share the mapping with the elffile and do this w.o syscall.
        if (file_ops::ensure_read(fd_, insn_bytes.data(), insn_bytes.size(),
                                  static_cast<ssize_t>(addr)) ==
            file_ops::k_err) {
            return false;
        }
        return true;
    }

    constexpr bool
    from_reload() const {
        return from_reload_;
    }

    constexpr bool
    is_unknown() const {
        return name_.eq("[unknown]");
    }

    constexpr bool
    is_vdso() const {
        return name_.eq("[vdso]");
    }

    constexpr bool
    name_eq(const dso_t & other) const {
        return name_.eq(other.name_);
    }

    constexpr bool
    eq(const dso_t * other) const {
        return this == other;
    }

    constexpr uint64_t
    hash() const {
        return name_.hash();
    }

    constexpr bool
    valid() const {
        return true;
    }

    static void
    set_dso_root_path(std::string_view sv) {
        assert(G_dso_root_path.empty());
        G_dso_root_path = sv;
    }

    static void
    set_dso_root_path(const char * s) {
        set_dso_root_path(std::string_view{ s });
    }

    static std::string_view
    get_dso_root_path() {
        return G_dso_root_path;
    }

    static bool
    has_dso_root_path() {
        return !get_dso_root_path().empty();
    }

    void
    dump(int vlvl, FILE * fp = stdout) const {
        if (!has_verbosity(vlvl)) {
            return;
        }
        TLO_fprint_ifv(vlvl, fp, "%s%s\n", str(), has_dbg_ ? " (+Debug)" : "");
        for (const auto & dep : deps_) {
            TLO_fprint_ifv(vlvl, fp, "\tDep -> %s\n", dep.str());
        }
        if (has_comm_uses()) {
            for (const auto & comm : comm_uses()) {
                TLO_fprint_ifv(vlvl, fp, "\tUse -> %s\n", comm.str());
            }
        }
        std::array<char, 128> prefix_buf{};
        size_t                i = 0;
        prefix_buf[0]           = '\t';
        for (const auto & fc : func_clumps_) {
            auto res =
                std::to_chars(prefix_buf.begin() + 1, prefix_buf.end() - 1, i);
            if (res.ec != std::errc()) {
                prefix_buf[1] = '\0';
            }
            else {
                *(res.ptr) = '\0';
            }
            fc.dump(vlvl, fp, prefix_buf.data());
            i += 1;
        }
    }
};


// static_assert(has_okay_type_traits<dso_t>::value);

}  // namespace sym
}  // namespace tlo
#endif
