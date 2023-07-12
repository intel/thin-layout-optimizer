#ifndef SRC_D_SYM_D_FUNC_H_
#define SRC_D_SYM_D_FUNC_H_

////////////////////////////////////////////////////////////////////////////////
// Function symbol. Used as a building block for func_clump_t. Has a name and an
// ident. The ident stores qualifier information (like plt/version).

#include "src/util/strbuf.h"
#include "src/util/strtab.h"
#include "src/util/type-info.h"

#include "src/sym/addr-range.h"

// #define TLO_DEBUG
#include "src/util/debug.h"

#include <stdint.h>
#include <unistd.h>


namespace tlo {
namespace sym {

struct dso_t;


// Version of plt identified
struct ident_t {
    static constexpr uint16_t k_is_global_ver = 3;
    static constexpr uint16_t k_is_hidden_ver = 2;
    static constexpr uint16_t k_is_ver        = 2;
    static constexpr uint16_t k_is_plt        = 1;
    static constexpr uint16_t k_is_none       = 0;

    static constexpr size_t k_num_meta_bits = 2;

    strbuf_t<k_num_meta_bits> str_;

    static constexpr strbuf_t<k_num_meta_bits>
    create_strbuf(std::string_view sview) {
        if (sview.empty()) {
            return { std::string_view{ "" }, k_is_none };
        }
        if (sview.starts_with("@@")) {
            sview.remove_prefix(2);
            return { sview, k_is_global_ver };
        }
        if (sview == "@plt") {
            return { std::string_view{ "" }, k_is_plt };
        }
        if (sview.starts_with("@")) {
            sview.remove_prefix(1);
            return { sview, k_is_hidden_ver };
        }
        return { sview, k_is_none };
    }

    constexpr ident_t() = default;
    constexpr ident_t(std::string_view sview) : str_(create_strbuf(sview)) {}
    constexpr ident_t(small_str_t<const char *> ss)
        : str_(create_strbuf(ss.sview())) {}

    constexpr ident_t(std::string_view sview, uint16_t meta)
        : str_(sview, meta) {}
    constexpr ident_t(small_str_t<const char *> ss, uint16_t meta)
        : str_(ss, meta) {}


    template<bool k_tab_type_unused>
    void
    finalize_in_tab(strtab_t<k_tab_type_unused> * ident_tab) {
        str_ = ident_tab->template get_sbuf<k_num_meta_bits>(str_);
    }

    void
    save_to(char * p) {
        str_.save_to(p);
    }


    constexpr std::string_view
    prefix() const {
        switch (extra()) {
            case k_is_none:
                return "";
            case k_is_plt:
                return "@plt";
            case k_is_hidden_ver:
                return "@";
            case k_is_global_ver:
                return "@@";
            default:
                assert(0 && "Invalid extra information!");
        }
    }

    constexpr std::string_view
    sview() const {
        return str_.sview();
    }

    constexpr char const *
    str() const {
        return str_.str();
    }

    constexpr size_t
    len() const {
        return str_.len();
    }

    constexpr uint16_t
    extra() const {
        return str_.extra();
    }

    constexpr bool
    is_plt() const {
        return extra() == k_is_plt;
    }

    constexpr bool
    is_hidden_ver() const {
        return extra() == k_is_hidden_ver;
    }

    constexpr bool
    is_global_ver() const {
        return extra() == k_is_global_ver;
    }

    constexpr bool
    is_ver() const {
        return extra() & k_is_ver;
    }

    constexpr bool
    is_none() const {
        return extra() == k_is_none;
    }

    constexpr bool
    active() const {
        return str_.active();
    }

    constexpr bool
    eq(const ident_t other) const {
        if (other.extra() != extra()) {
            return false;
        }
        return is_ver() ? str_.eq(other.str_) : true;
    }

    constexpr bool
    lt(const ident_t other) const {
        if (other.extra() != extra()) {
            return extra() < other.extra();
        }
        return is_ver() ? str_.lt(other.str_) : false;
    }

    constexpr uint64_t
    hash() const {
        return str_.hash() ^ static_cast<uint64_t>(extra());
    }

    constexpr bool
    valid() const {
        return extra() != k_is_none || str_.empty();
    }
};
static_assert(has_okay_type_traits<ident_t>::value);

struct func_t {
    // 32MB max function size (sanity check basically)
    static constexpr size_t k_max_func_size = 32 * 1024 * 1024;

    static constexpr size_t k_has_elfinfo_midx = 0;
    static constexpr size_t k_unused_midx      = 1;

    using dso_and_meta_t = pptr_t<const dso_t *, 1, 15>;

    dso_and_meta_t dso_and_meta_;


    // Functions name (mangled)
    strbuf_t<> name_;
    ident_t    ident_;

    // At the moment, these are update by each profile point so we get a
    // reasonable estimate for the hot functions at least.
    addr_range_t loc_;


    constexpr func_t() = default;

    void
    cg_prepare() {
        loc_ = { 0, 0 };
    }


    template<typename T0_t, typename T1_t>
    func_t(const dso_t * dso,
           bool          has_exact,
           T0_t &&       name,
           T1_t &&       ident,
           addr_range_t  loc)
        : dso_and_meta_(dso, static_cast<uint16_t>(has_exact), 0),
          name_(std::forward<T0_t>(name)),
          ident_(std::forward<T1_t>(ident)),
          loc_(std::forward<addr_range_t>(loc)) {}
    template<typename T0_t, typename T1_t>
    func_t(const dso_t * dso, T0_t && name, T1_t && ident, addr_range_t loc)
        : func_t(dso,
                 false,
                 std::forward<T0_t>(name),
                 std::forward<T1_t>(ident),
                 std::forward<addr_range_t>(loc)) {}

    constexpr char const *
    str() const {
        return name_.str();
    }

    constexpr size_t
    extra_size() const {
        return 0;
    }

    constexpr char const *
    ident_str() const {
        return ident_.str();
    }
    constexpr char const *
    ident_prefix() const {
        return ident_.prefix().data();
    }


    template<bool k_tab_type_unused>
    void
    finalize_in_tab(strtab_t<k_tab_type_unused> * func_and_ident_tab) {
        name_ = func_and_ident_tab->get_sbuf(name_);
        ident_.finalize_in_tab(func_and_ident_tab);
    }
    template<bool k_tab_type_unused>
    void
    finalize(char *, strtab_t<k_tab_type_unused> * func_and_ident_tab) {
        finalize_in_tab(func_and_ident_tab);
        dump(3, stdout, "Creating: ");
    }

    constexpr const dso_t *
    dso() const {
        return dso_and_meta_.ptr();
    }

    constexpr bool
    in_dso(const dso_t * other_dso) const {
        return other_dso == dso();
    }

    constexpr bool
    is_specialized() const {
        return !ident_.is_none();
    }

    constexpr bool
    is_plt() const {
        return ident_.is_plt();
    }

    constexpr bool
    is_versioned() const {
        return ident_.is_ver();
    }

    constexpr std::string_view
    section() const {
        // TODO: Implement me!
        return { "", 0 };
    }

    void
    write_order_label(FILE * fp, size_t order) const {
        (void)fprintf(fp, "%d,%s,%zu\n", 0, str(), order);
    }

    std::string_view label(vec_t<char> *    buf,
                           std::string_view postfix = {}) const;


    constexpr void
    add_sample_addr(uint64_t sample_addr) {
        if (has_elfinfo()) {
            assert(!is_unknown());
            return;
        }
        loc_.add_addr(sample_addr);
    }

    constexpr uint64_t
    start_addr() const {
        return loc_.lo_addr_inclusive_;
    }

    constexpr uint64_t
    end_addr() const {
        return loc_.hi_addr_exclusive_;
    }

    constexpr addr_range_t
    get_addr_range() const {
        return loc_;
    }

    constexpr bool
    has_elfinfo() const {
        return dso_and_meta_.template meta<k_has_elfinfo_midx>();
    }

    constexpr uint64_t
    size() const {
        return loc_.size();
    }

    constexpr bool
    is_unknown() const {
        return name_.eq("[unknown]");
    }

    constexpr bool
    is_findable() const {
        // We can't re-order PLT
        return !is_unknown() && !is_plt();
    }

    constexpr bool
    name_eq(const func_t & other) const {
        return hash() == other.hash() && dso() == other.dso() &&
               name_.eq(other.name_) && ident_.eq(other.ident_);
    }

    constexpr bool
    name_lt(const func_t & other) const {
        // Keep sort stable for the sake of testing.
#if 0
        if (dso() != other.dso()) {
            return dso() < other.dso();
        }
        if (hash() != other.hash()) {
            return hash() < other.hash();
        }
#endif
        int res = name_.cmp(other.name_);
        if (res == 0) {
            return ident_.lt(other.ident_);
        }
        return res < 0;
    }

    constexpr bool
    lt(const func_t & other) const {
        return loc_.lt(other.loc_);
    }

    constexpr bool
    eq(const func_t * other) const {
        return this == other;
    }
#if TLO_USING_CLANG
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Winvalid-constexpr"
#endif
    constexpr uint64_t
    hash() const {
        return xxhash::run(reinterpret_cast<uintptr_t>(dso())) ^
               rol(name_.hash(), 13) ^ rol(ident_.hash(), 17);
    }
#if TLO_USING_CLANG
# pragma clang diagnostic pop
#endif

    void dump(int vlvl = 1, FILE * fp = stdout, const char * prefix = "") const;

    constexpr bool
    valid(bool cg_ready = false, bool reloaded = false) const {
        if (!loc_.valid()) {
            TLO_TRACE("Invalid loc\n");
            return false;
        }
        if (has_elfinfo()) {
            if (!cg_ready && !reloaded && loc_.single()) {
                TLO_TRACE("Invalid elfinfo\n");
                return false;
            }
        }
        if ((cg_ready || reloaded) && (loc_.active() || size() != 0)) {
            TLO_TRACE("Invalid CG Prepared\n");
            return false;
        }
        // ifuncs seems to violate this.
        if (!cg_ready && !reloaded && is_plt() && size() > 16) {
            TLO_TRACE("Invalid plt size (%d:%zu)\n", has_elfinfo(), size());
            return false;
        }
        if (!ident_.valid()) {
            TLO_TRACE("Invalid identifier\n");
            return false;
        }
        if (!cg_ready && !reloaded) {
            if (size() > k_max_func_size) {
                if (!is_unknown()) {
                    TLO_TRACE("Invalid func size\n");
                    return false;
                }
            }
        }
        return true;
    }
    struct name_cmp_t {
        constexpr bool
        operator()(func_t const & lhs, func_t const & rhs) {
            return lhs.name_lt(rhs);
        }
    };

    struct name_equals_t {
        template<typename T_t>
        constexpr bool
        operator()(const T_t lhs, const T_t rhs) const {
            if constexpr (std::is_pointer_v<T_t>) {
                return lhs->name_eq(*rhs);
            }
            else {
                return lhs.name_eq(rhs);
            }
        }
    };

    struct name_hasher_t {
        using is_avalanching = void;

        template<typename T_t>
        constexpr uint64_t
        operator()(const T_t item) const {
            if constexpr (std::is_pointer_v<T_t>) {
                return item->hash();
            }
            else {
                return item.hash();
            }
        }
    };
};
static_assert(has_okay_type_traits<func_t>::value);

}  // namespace sym
}  // namespace tlo
#endif
