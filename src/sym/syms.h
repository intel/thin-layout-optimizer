#ifndef SRC_D_SYM_D_SYMS_H_
#define SRC_D_SYM_D_SYMS_H_


#include "src/sym/dso.h"
#include "src/sym/func.h"

#include "src/perf/perf-sample.h"

#include "src/system/insn.h"
#include "src/util/bits.h"
#define TLO_DEBUG
#include "src/util/debug.h"
#include "src/util/file-ops.h"
#include "src/util/memory.h"
#include "src/util/path.h"
#include "src/util/strbuf.h"
#include "src/util/type-info.h"
#include "src/util/global-stats.h"

#include <stdint.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// Symbols that we tracking.
// At the moment on DSOs and Functions. The symbols are managed by a runtime
// `sym_state_t` which ensures that each unique symbols will have exactly one
// pointer (so pointer checks work for symbols).

// For DSOs, uniqueness is purely from the name. For Functions its DSO +
// Function name.

// TODO: Generalize functions into arbitrary sections.

namespace tlo {
namespace sym {


struct sym_state_t {
    // Table to manage symbols (dso/func).
    // Tracks each symbol by unique name.
    template<typename T_t>
    struct alloc_tbl_t {
        struct wrapper_t {
            mutable T_t * ptr_;
            template<bool k_tab_type_unused>
            void
            update(T_t *                         ptr,
                   char *                        str_ptr,
                   strtab_t<k_tab_type_unused> * name_tab) const {
                ptr_ = ptr;
                ptr_->finalize(str_ptr, name_tab);
            }

            constexpr bool
            eq(const wrapper_t other) const {
                return ptr_->name_eq(*(other.ptr_));
            }

            constexpr uint64_t
            hash() const {
                return ptr_->hash();
            }
        };

        uint64_t
        size() const {
            return set_.size();
        }

        // If the symbol exists, return pointer to it. Otherwise create a new
        // one (and return its pointers).
        template<bool k_tab_type_unused, typename... Ts_t>
        T_t *
        get(bump_alloc_t<> *              allocator,
            strtab_t<k_tab_type_unused> * name_tab,
            Ts_t &&... ts) {
            T_t  item{ ts... };
            auto res = set_.emplace(wrapper_t{ &item });
            if (res.second) {
                void * p = allocator->getz(sizeof(T_t) + item.extra_size(),
                                           alignof(T_t));
                T_t *  item_ptr = new (p) T_t{ std::forward<Ts_t>(ts)... };
                char * str_ptr  = nullptr;
                if (item.extra_size() != 0) {
                    str_ptr = reinterpret_cast<char *>(p) + sizeof(T_t);
                }
                res.first->update(item_ptr, str_ptr, name_tab);
                return item_ptr;
            }
            return res.first->ptr_;
        }

        template<typename... Ts_t>
        T_t *
        find(Ts_t &&... ts) const {
            T_t  item{ std::forward<Ts_t>(ts)... };
            auto res = set_.find(wrapper_t{ &item });
            if (res == set_.end()) {
                return nullptr;
            }
            return res->ptr_;
        }

        using base_set_t = basic_uset<wrapper_t>;
        base_set_t set_;
    };
    func_clump_t *
    get_unknown_func(dso_t const * dso, addr_range_t loc) {
        strbuf_t<> name_sb = strbuf_t<>{ "[unknown]" };
        ident_t    ident   = ident_t{ "" };
        func_t     tmp_func{ dso, false, name_sb, ident, loc };
        return func_tab_.get(&alloc_, &name_tab_, &tmp_func);
    }

    func_clump_t *
    get_func(dso_t const * dso, addr_range_t loc) noexcept {
        func_clump_t * func_clump = dso->lookup_func_clump(loc);
        if (func_clump != nullptr) {
            TLO_INCR_STAT(total_known_funcs_);
            return func_clump;
        }
        TLO_INCR_STAT(total_unknown_funcs_);
        return get_unknown_func(dso, loc);
    }

    func_clump_t *
    get_func(dso_t const * dso, const perf::sample_loc_t * sample) {
        return get_func(dso, addr_range_t{ sample->unmapped_addr_ });
    }

    dso_t *
    get_dso(strbuf_t<> dso_str) {
        return dso_tab_.get(&alloc_, &name_tab_, dso_str, false);
    }

    dso_t *
    find_dso(strbuf_t<> dso_str) {
        return dso_tab_.find(dso_str, false);
    }

    dso_t *
    get_reloaded_dso(strbuf_t<> dso_str) {
        return dso_tab_.get(&alloc_, &name_tab_, dso_str, true);
    }

    dso_t *
    get_dso(const perf::sample_loc_t * sample) {
        return get_dso(sample->dso_);
    }

    bump_alloc_t<>              alloc_;
    alloc_tbl_t<dso_t>          dso_tab_;
    alloc_tbl_t<func_clump_t>   func_tab_;
    strtab_t<true>              name_tab_;
    vec_t<const func_clump_t *> fc_to_free_;
    // Helper for easy iteration through all tracked DSOs.
    // We should/could do the same for other symbols (so far no need).
    template<typename T_t>
    struct alloc_tab_iterator_t {
        using alloc_tab_it_base_t =
            typename std::conditional_t<std::is_same_v<T_t, dso_t>,
                                        decltype(dso_tab_.set_),
                                        decltype(func_tab_.set_)>;

        const alloc_tab_it_base_t & base_;

        struct alloc_tab_iterator_impl_t {

            using alloc_tab_it_type_t = decltype(base_.cbegin());
            using iterator_category =
                typename alloc_tab_it_type_t::iterator_category;
            alloc_tab_it_type_t it_;

            const T_t *
            operator*() const {
                return (*it_).ptr_;
            }

            alloc_tab_iterator_impl_t &
            operator++() {
                it_++;
                return *this;
            }

            bool
            operator!=(alloc_tab_iterator_impl_t const & other) {
                return it_ != other.it_;
            }
        };

        alloc_tab_iterator_impl_t
        begin() const {
            return alloc_tab_iterator_impl_t{ base_.cbegin() };
        }

        alloc_tab_iterator_impl_t
        end() const {
            return alloc_tab_iterator_impl_t{ base_.cend() };
        }
    };

    strtab_t<true> *
    get_strtab() {
        return &name_tab_;
    }

    bump_alloc_t<> *
    get_allocator() {
        return &alloc_;
    }

    using dso_iterator_t        = alloc_tab_iterator_t<dso_t>;
    using func_clump_iterator_t = alloc_tab_iterator_t<func_clump_t>;
    dso_iterator_t
    dsos() const {
        return dso_iterator_t{ dso_tab_.set_ };
    }


    func_clump_iterator_t
    func_clumps() const {
        return func_clump_iterator_t{ func_tab_.set_ };
    }

    void
    add_fc_to_free(const func_clump_t * fc) {
        fc_to_free_.emplace_back(fc);
    }

    ~sym_state_t() {
        for (const dso_t * dso : dsos()) {
            dso->cleanup();
        }
        for (const func_clump_t * fc : func_clumps()) {
            fc->cleanup();
        }
        for (const func_clump_t * fc : fc_to_free_) {
            fc->cleanup();
        }
    }
};

}  // namespace sym
}  // namespace tlo

#endif
