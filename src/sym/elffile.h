#ifndef SRC_D_SYM_D_ELFFILE_H_
#define SRC_D_SYM_D_ELFFILE_H_

////////////////////////////////////////////////////////////////////////////////
// Elf implementation. The important APIs are init() and extract_functions().
// The former is clear, the latter is used by the DSO the get all the known
// functions for lookup when we parse perf events. The implementation is
// aggressively conservative (and slow). We check everything to make sure we
// fully understand it. If we don't understand anything (the elf is offspec),
// even in a trivial/unimportant way, we fail and defer to perf. Note that long
// term we have plans to move away from using perf to determine function
// locations/symbols so we will probably want to make this more willing to look
// past known trivial errors.


#include "src/sym/addr-range.h"
#include "src/sym/func.h"

#include "src/util/bits.h"
#include "src/util/file-ops.h"
#include "src/util/iterator.h"
#include "src/util/memory.h"
#include "src/util/smap.h"
#include "src/util/str-ops.h"
#include "src/util/strbuf.h"
#include "src/util/strtab.h"
#include "src/util/type-info.h"
#include "src/util/umap.h"
#include "src/util/vec.h"

// #define TLO_DEBUG
#include "src/util/debug.h"
#include "src/util/verbosity.h"

#include <elf.h>
#ifndef SHT_RELR
# define SHT_RELR 19
#endif
#ifndef DT_RELRSZ
# define DT_RELRSZ 35
#endif
#ifndef DT_RELR
# define DT_RELR 36
#endif
#ifndef DT_RELRENT
# define DT_RELRENT 37
#endif

#include <optional>
#include <span>

namespace tlo {
namespace sym {
struct dso_t;
}
struct elf_file_t {
    using eword_t = Elf64_Word;
    using ehalf_t = Elf64_Half;

    static constexpr size_t k_invalid_index =
        std::numeric_limits<size_t>::max();
    static constexpr size_t k_invalid_offset = k_invalid_index;
    static constexpr size_t k_invalid_addr   = k_invalid_index;
    static constexpr size_t k_invalid_size   = k_invalid_index;
    static constexpr size_t k_invalid_info   = k_invalid_index;


    struct strtab_t {
        using str_t = std::string_view;

        static constexpr str_t k_invalid_str = {};

        std::span<const char> tab_;

        constexpr strtab_t() = default;
        constexpr strtab_t(const char * s, size_t len) : tab_(s, len) {}

        static constexpr bool
        is_invalid_str(str_t s) {
            return s.data() == nullptr;
        }

        bool
        load_okay() const {
            return tab_.data() != nullptr;
        }

        str_t
        load_name(size_t index) const {
            assert(load_okay());
            if (index == k_invalid_index || index >= tab_.size()) {
                return k_invalid_str;
            }
            const char * val = &(tab_[index]);
            size_t       len = strlen(val);
            return { val, len };
        }
    };

    enum vstatus_t : uint8_t {
        k_okay     = 0,
        k_skip     = 1,
        k_invalid  = 2,
        k_unusable = 3,
    };


    elf_file_t()  = default;
    ~elf_file_t() = default;


    bool
    init(const char * path) {

        TLO_printvv("Initializing: %s\n", path);
        cleanup();
        mapping_  = file_ops::map_file(path, file_ops::k_map_read, false);
        is_debug_ = std::string_view{ path, strlen(path) }.ends_with(".debug");

        if (!mapping_.active()) {
            TLO_printvv("Bad File\n");
            return false;
        }
        if (validate() != k_okay) {
            TLO_printvv("\tBad Elf\n");
            return false;
        }

        TLO_printvv("\tGood\n");
        return true;
    }

    template<bool k_tab_type_unused>
    bool
    extract_functions(const sym::dso_t *                 dso,
                      std::span<sym::func_t> *           funcs_out,
                      std::span<strbuf_t<>> *            links_out,
                      basic_uset<strbuf_t<>> *           buildids,
                      tlo::strtab_t<k_tab_type_unused> * name_tab) {
        if (funcs_out == nullptr || name_tab == nullptr) {
            TLO_TRACE("Null arguments\n");
            return false;
        }
        sect_dynamic_info_t   dyn_info{};
        sect_versym_indexes_t vsi{};
        sect_versym_defs_t    vsd{};
        sect_versym_need_t    vsn{};
        sect_notes_t          build_id_note{};
        sect_symtab_t         dsymtab{};
        sect_symtab_t         ssymtab{};
        for (sect_hdr_t sect_hdr : sect_hdr_it()) {
            if (!sect_hdr.section_is_loadable(this)) {
                continue;
            }
            if (sect_hdr.is_dynamic_info()) {
                if (dyn_info.active()) {
                    TLO_TRACE("Bad dyninfo\n");
                    return false;
                }
                dyn_info.init(sect_hdr, this);
                continue;
            }
            if (sect_hdr.is_dsymtab()) {
                if (dsymtab.active()) {
                    TLO_TRACE("Bad dsymtab\n");
                    return false;
                }
                dsymtab.init(sect_hdr, this);
                continue;
            }
            if (sect_hdr.is_ssymtab()) {
                if (ssymtab.active()) {
                    TLO_TRACE("Bad ssymtab\n");
                    return false;
                }
                ssymtab.init(sect_hdr, this);
                continue;
            }
            if (sect_hdr.is_versym_indexes()) {
                if (vsi.active()) {
                    TLO_TRACE("Bad vsym_indexes\n");
                    return false;
                }
                vsi.init(sect_hdr, this);
                continue;
            }
            if (sect_hdr.is_versym_defs()) {
                if (vsd.active()) {
                    TLO_TRACE("Bad vsym_defs\n");
                    return false;
                }
                vsd.init(sect_hdr, this, vsi);
                continue;
            }
            if (sect_hdr.is_versym_need()) {
                if (vsn.active()) {
                    TLO_TRACE("Bad vsym_need\n");
                    return false;
                }
                vsn.init(sect_hdr, this, vsi);
                continue;
            }
            if (sect_hdr.is_note()) {
                sect_notes_t note{ sect_hdr, this };
                if (note.is_build_id()) {
                    if (build_id_note.active()) {
                        TLO_TRACE("Duplicate build id!");
                        return false;
                    }
                    build_id_note = note;
                    assert(build_id_note.active());
                }
            }
        }

        if (buildids != nullptr && build_id_note.active()) {
            std::span<const uint8_t> build_id = build_id_note.load_build_id();
            if (!build_id.empty()) {
                std::array<char, 256> build_id_str;
                std::string_view sv = to_hex_string(build_id, &build_id_str);
                if (!sv.empty()) {
                    buildids->emplace(name_tab->get_sbuf(sv));
                }
            }
        }


        if (links_out != nullptr && dyn_info.active()) {
            size_t nelem = dyn_info.dso_deps_.size();
            if (nelem != 0) {
                assert(links_out->empty());
                strbuf_t<> * links_mem = arr_alloc<strbuf_t<>>(nelem);
                *links_out             = { links_mem, nelem };
                for (strtab_t::str_t dso_dep : dyn_info.dso_deps_) {
                    *links_mem = { name_tab->get_sbuf(dso_dep) };
                    ++links_mem;
                }
            }
        }

        size_t max_funcs_out = 0;
        if (ssymtab.active()) {
            max_funcs_out += ssymtab.estimate_num_functions();
        }
        if (dsymtab.active()) {
            max_funcs_out += dsymtab.estimate_num_functions();
        }

        sym::func_t * funcs_mem     = nullptr;
        size_t        funcs_mem_idx = 0;
        if (funcs_out->empty()) {
            funcs_mem = arr_alloc<sym::func_t>(max_funcs_out);
        }
        else {
            funcs_mem_idx = funcs_out->size();
            max_funcs_out += funcs_mem_idx;
            funcs_mem = arr_realloc<sym::func_t>(funcs_out->data(),
                                                 funcs_mem_idx, max_funcs_out);
        }


        auto add_func = [&funcs_mem, &funcs_mem_idx, &max_funcs_out,
                         name_tab](auto... ts) {
            // This really should NEVER happen.
            if (funcs_mem_idx >= max_funcs_out) {
                // Grow slowly. If this happens its liable never more than once.
                max_funcs_out = (funcs_mem_idx * 5u) / 4u + 1;
                funcs_mem =
                    arr_realloc(funcs_mem, funcs_mem_idx, max_funcs_out);
            }
            new (funcs_mem + funcs_mem_idx)
                sym::func_t{ std::forward<sym::func_t>(ts)... };
            funcs_mem[funcs_mem_idx].finalize_in_tab(name_tab);
            ++funcs_mem_idx;
        };

        if (ssymtab.active()) {
            for (sect_symtab_t::symtab_entry_t symtab_ent : ssymtab) {
                if (!symtab_ent.has_func_info(this)) {
                    continue;
                }
                // No version info assosiated with non-dynamic functions.
                add_func(symtab_ent.extract_func(ssymtab, dso));
            }
        }

        if (dsymtab.active()) {
            bool use_vsn = false;
            bool use_vsd = false;

            if (vsi.active()) {
                use_vsn = dyn_info.use_for_vinfo(&vsn);
                use_vsd = dyn_info.use_for_vinfo(&vsd);
            }
            if (!use_vsn && !use_vsd) {
                for (sect_symtab_t::symtab_entry_t symtab_ent : dsymtab) {
                    if (!symtab_ent.has_func_info(this)) {
                        continue;
                    }

                    add_func(symtab_ent.extract_func(dsymtab, dso));
                }
            }
            else {
                size_t nentries = dsymtab.get_section_num_entries();
                auto   dsym_it  = dsymtab.begin();
                auto   vsym_it  = vsi.begin();
                for (; nentries; --nentries, ++dsym_it, ++vsym_it) {
                    if (!(*dsym_it).has_func_info(this)) {
                        continue;
                    }

                    std::pair<strtab_t::str_t, bool> vinfo = {
                        strtab_t::k_invalid_str, false
                    };

                    if (use_vsn) {
                        vinfo = vsn.find_ver_ident((*vsym_it).get_index(),
                                                   (*dsym_it));
                    }
                    if (use_vsd && strtab_t::is_invalid_str(vinfo.first)) {
                        vinfo = vsd.find_ver_ident((*vsym_it).get_index(),
                                                   (*dsym_it));
                    }

                    add_func((*dsym_it).extract_func(dsymtab, dso, vinfo.first,
                                                     vinfo.second));
                }
            }
        }
        funcs_mem  = arr_realloc(funcs_mem, max_funcs_out, funcs_mem_idx);
        *funcs_out = { funcs_mem, funcs_mem_idx };
        return true;
    }

    void
    dump() const {
        return;
        for (sect_hdr_t sect_hdr : sect_hdr_it()) {
            fprintf(stderr, "Section: %s [%zu, %zu, %zu]\n",
                    sect_hdr.section_name(this).data(),
                    sect_hdr.get_section_load_begin(),
                    sect_hdr.get_section_load_end(),
                    sect_hdr.get_section_entry_load_size());
            if (sect_hdr.section_is_loadable(this) && sect_hdr.is_symtab()) {
                size_t        cnt = 0;
                sect_symtab_t symtab(sect_hdr, this);
                for (sect_symtab_t::symtab_entry_t symtab_ent : symtab) {
                    if (symtab_ent.is_func() &&
                        symtab_ent.has_func_info(this)) {
#if 0
                        fprintf(stderr, "\tSym: %-16s [%zx, %zu]\n",
                                symtab_ent.entry_name(this, sect_hdr).data(),
                                symtab_ent.has_sym_vaddr(this)
                                    ? symtab_ent.sym_vaddr()
                                    : 0,
                                symtab_ent.has_sym_size()
                                    ? symtab_ent.sym_size()
                                    : 0);
#endif
                        ++cnt;
                    }
                }
                fprintf(stderr, "\tNum Funcs: %zu\n", cnt);
            }
        }
    }

    void
    cleanup() {
        if (mapping_.active()) {
            file_ops::unmap_file(mapping_);
            mapping_.deactivate();
        }
    }

    bool
    active() const {
        return mapping_.active();
    }

    bool
    loadable(size_t file_offset, size_t size) const {
        return active() && mapping_.inbounds(file_offset, size);
    }


    template<typename T_base_t>
    struct hdr_base_t {

        const T_base_t * base_;
        constexpr hdr_base_t(const T_base_t * p) : base_(p) {}
        TLO_DISABLE_WCAST_ALIGN
        hdr_base_t(const uint8_t * p)
            : hdr_base_t(reinterpret_cast<const T_base_t *>(p)) {}
        constexpr hdr_base_t() = default;
        TLO_REENABLE_WCAST_ALIGN
        bool
        load_okay() const {
            return base_ != nullptr;
        }

        // File interaction API
        static constexpr size_t
        load_reqsize() {
            return sizeof(T_base_t);
        }
    };

    struct elf_hdr_t : hdr_base_t<Elf64_Ehdr> {

        template<typename... Ts_t>
        elf_hdr_t(Ts_t... ts)
            : hdr_base_t<Elf64_Ehdr>(std::forward<Ts_t>(ts)...) {}
        constexpr elf_hdr_t() = default;
        // Field API.
        size_t
        get_elf_hdr_load_size() const {
            assert(load_okay());
            return base_->e_ehsize;
        }

        size_t
        get_prog_hdrs_load_begin() const {
            assert(load_okay());
            return base_->e_phoff;
        }

        size_t
        get_prog_hdr_load_size() const {
            assert(load_okay());
            return base_->e_phentsize;
        }

        size_t
        get_sect_hdrs_load_begin() const {
            assert(load_okay());
            return base_->e_shoff;
        }

        size_t
        get_sect_hdr_load_size() const {
            assert(load_okay());
            return base_->e_shentsize;
        }

        bool
        is_sysv_abi() const {
            assert(load_okay());
            return base_->e_ident[EI_OSABI] == ELFOSABI_SYSV;
        }

        bool
        is_gnu_abi() const {
            assert(load_okay());
            return base_->e_ident[EI_OSABI] == ELFOSABI_GNU;
        }

        bool
        is_x86_64() const {
            assert(load_okay());
            return base_->e_machine == EM_X86_64;
        }

        bool
        is_executable() const {
            assert(load_okay());
            return base_->e_type == ET_EXEC;
        }

        bool
        is_shared_obj_or_pie() const {
            assert(load_okay());
            return base_->e_type == ET_DYN;
        }

        std::optional<size_t>
        get_1st_sect_strtab_hdr_index() const {
            assert(load_okay());
            // If == SHN_UNDEF -> no sect_strtab
            // elif == SHN_XINDEX -> first_sect->sh_link
            // else -> this
            size_t idx = base_->e_shstrndx;
            if (idx == SHN_UNDEF) {
                return k_invalid_index;
            }
            else if (idx == SHN_XINDEX) {
                return std::nullopt;
            }
            return idx;
        }

        std::optional<size_t>
        get_1st_num_prog_hdrs() const {
            assert(load_okay());
            // If == PN_XNUM -> first_sect->sh_info
            // else -> this
            size_t off = base_->e_phnum;
            if (off == PN_XNUM) {
                return std::nullopt;
            }
            return off;
        }

        std::optional<size_t>
        get_1st_num_sect_hdrs() const {
            assert(load_okay());
            // If == 0 -> first_sect->sh_size
            // else -> this
            size_t off = base_->e_shnum;
            if (off == 0) {
                return std::nullopt;
            }
            return off;
        }

        vstatus_t
        sanity_check() const {
            if (!load_okay()) {
                return k_invalid;
            }
            // Elf header thinks its size isn't the same as what we have for
            // it. Probably means 32-bit header. Supported right now.
            if (base_->e_ehsize != load_reqsize()) {
                TLO_TRACE("Bad size: %hu vs %zu", base_->e_ehsize,
                          load_reqsize());
                return k_invalid;
            }

            // Sanity Check `e_indent` field.
            {
                // Check magic numbers.
                if (base_->e_ident[EI_MAG0] != ELFMAG0 ||
                    base_->e_ident[EI_MAG1] != ELFMAG1 ||
                    base_->e_ident[EI_MAG2] != ELFMAG2 ||
                    base_->e_ident[EI_MAG3] != ELFMAG3) {
                    TLO_TRACE("Bad magic");
                    return k_invalid;
                }

                switch (base_->e_ident[EI_CLASS]) {
                        // Only 64-bit for now.
                    case ELFCLASS64:  // 64-bit objects
                        break;
                    case ELFCLASS32:  // 32-bit objects
                        TLO_TRACE("Bad class0");
                        return k_unusable;
                    case ELFCLASSNONE:  // Invalid class
                    default:
                        TLO_TRACE("Bad class1");
                        return k_invalid;
                }


                switch (base_->e_ident[EI_DATA]) {
                        // Only x86 encoding for now.
                    case ELFDATA2LSB:  // 2's complement, little endian
                        break;
                    case ELFDATA2MSB:  // 2's complement, big endian
                        TLO_TRACE("Bad byteorder0");
                        return k_unusable;
                    case ELFDATANONE:  // Invalid data encoding
                    default:
                        TLO_TRACE("Bad byteorder1");
                        return k_invalid;
                }

                switch (base_->e_ident[EI_VERSION]) {
                    case EV_CURRENT:
                        break;
                    case EV_NONE:  // Invalid
                    default:
                        TLO_TRACE("Bad verbose");
                        return k_invalid;
                }

                switch (base_->e_ident[EI_OSABI]) {
                    // Only handle SYSV/GNU abi.
                    case ELFOSABI_SYSV:  // UNIX System V ABI (Also: NONE)
                    case ELFOSABI_GNU:   // Object uses GNU ELF extensions
                                         // (Also: LINUX)

                        break;
                    case ELFOSABI_HPUX:        // HP-UX
                    case ELFOSABI_NETBSD:      // NetBSD.
                    case ELFOSABI_SOLARIS:     // Sun Solaris.
                    case ELFOSABI_AIX:         // IBM AIX.
                    case ELFOSABI_IRIX:        // SGI Irix.
                    case ELFOSABI_FREEBSD:     // FreeBSD.
                    case ELFOSABI_TRU64:       // Compaq TRU64 UNIX.
                    case ELFOSABI_MODESTO:     // Novell Modesto.
                    case ELFOSABI_OPENBSD:     // OpenBSD.
                    case ELFOSABI_ARM_AEABI:   // ARM EABI
                    case ELFOSABI_ARM:         // ARM
                    case ELFOSABI_STANDALONE:  // Standalone (embedded)
                                               // application
                        TLO_TRACE("Bad osabi0");
                        return k_unusable;
                    default:
                        TLO_TRACE("Bad osabi1");
                        return k_invalid;
                }

                // Supposed to be zero-padded.
                for (size_t i = EI_PAD; i < EI_NIDENT; ++i) {
                    if (base_->e_ident[i] != 0) {
                        TLO_TRACE("Bad pad");
                        return k_invalid;
                    }
                }
            }
            // Sanity Check `e_type` field.
            {
                switch (base_->e_type) {
                    // We only really care about libraries/executables.
                    case ET_EXEC:  // Executable file
                    case ET_DYN:   // Shared object file
                        break;
                        // We shouldn't need to support static libaries, we
                        // should just treat all their symbols as belonging
                        // to the executable.
                    case ET_REL:   // Relocatable file
                    case ET_CORE:  // Core file
                        // We can probably fail on any of these fields...
                        TLO_TRACE("Bad type0");
                        return k_unusable;
                    case ET_NONE:  // No file type
                        TLO_TRACE("Bad type1");
                        return k_invalid;
                    default:
                        // In processor specific range.
                        TLO_DISABLE_WTYPE_LIMITS;
                        if (base_->e_type >= ET_LOPROC &&
                            base_->e_type <= ET_HIPROC) {
                            break;
                        }
                        TLO_REENABLE_WTYPE_LIMITS
                        // In OS specific range.
                        if (base_->e_type >= ET_LOOS &&
                            base_->e_type <= ET_HIOS) {
                            break;
                        }
                        TLO_TRACE("Bad type2");
                        return k_invalid;
                }
            }

            // Sanity Check `e_machine` field.

            {
                switch (base_->e_machine) {
                    // Only handle x86-64 for now.
                    case EM_X86_64:  // AMD x86-64 architecture
                        break;
                    case EM_386:           // Intel 80386
                    case EM_56800EX:       // Freescale 56800EX DSC
                    case EM_68HC05:        // Motorola MC68HC05 microcontroller
                    case EM_68HC08:        // Motorola MC68HC08 microcontroller
                    case EM_68HC11:        // Motorola MC68HC11 microcontroller
                    case EM_68HC12:        // Motorola M68HC12
                    case EM_68HC16:        // Motorola MC68HC16 microcontroller
                    case EM_68K:           // Motorola m68k family
                    case EM_78KOR:         // Renesas 78KOR
                    case EM_8051:          // Intel 8051 and variants
                    case EM_860:           // Intel 80860
                    case EM_88K:           // Motorola m88k family
                    case EM_960:           // Intel 80960
                    case EM_AARCH64:       // ARM AARCH64
                    case EM_ALTERA_NIOS2:  // Altera Nios II
                    case EM_AMDGPU:        // AMD GPU
                    case EM_ARC:           // Argonaut RISC Core
                    case EM_ARCA:          // Arca RISC
                    case EM_ARCV2:         // Synopsys ARCv2 ISA.
                    case EM_ARC_COMPACT:   // ARC International ARCompact
                    case EM_ARM:           // ARM
                    case EM_AVR32:         // Amtel 32-bit microprocessor
                    case EM_AVR:           // Atmel AVR 8-bit microcontroller
                    case EM_BA1:           // Beyond BA1
                    case EM_BA2:           // Beyond BA2
                    case EM_BLACKFIN:      // Analog Devices Blackfin DSP
                    case EM_BPF:   // Linux BPF -- in-kernel virtual machine
                    case EM_C166:  // Infineon C16x/XC16x
                    case EM_CDP:   // Paneve CDP
                    case EM_CE:    // Freescale Communication Engine RISC
                    case EM_CLOUDSHIELD:  // CloudShield
                    case EM_COGE:         // Cognitive Smart Memory Processor
                    case EM_COLDFIRE:     // Motorola Coldfire
                    case EM_COOL:         // Bluechip CoolEngine
                    case EM_COREA_1ST:    // KIPO-KAIST Core-A 1st gen.
                    case EM_COREA_2ND:    // KIPO-KAIST Core-A 2nd gen.
                    case EM_CR16:         // National Semi. CompactRISC CR16
                    case EM_CR:           // National Semi. CompactRISC
                    case EM_CRAYNV2:      // Cray NV2 vector architecture
                    case EM_CRIS:         // Axis Communications 32-bit emb.proc
                    case EM_CRX:          // National Semi. CompactRISC CRX
                    case EM_CSKY:         // C-SKY
                    case EM_CSR_KALIMBA:  // CSR Kalimba
                    case EM_CUDA:         // NVIDIA CUDA
                    case EM_CYPRESS_M8C:  // Cypress M8C
                    case EM_D10V:         // Mitsubishi D10V
                    case EM_D30V:         // Mitsubishi D30V
                    case EM_DSP24:        // New Japan Radio (NJR) 24-bit DSP
                    case EM_DSPIC30F:     // Microchip Technology dsPIC30F
                    case EM_DXP:         // Icera Semi. Deep Execution Processor
                    case EM_ECOG16:      // Cyan Technology eCOG16
                    case EM_ECOG1X:      // Cyan Technology eCOG1X
                    case EM_ECOG2:       // Cyan Technology eCOG2
                    case EM_EMX16:       // KM211 KMX16
                    case EM_EMX8:        // KM211 KMX8
                    case EM_ETPU:        // Freescale Extended Time Processing
                                         // Unit
                    case EM_EXCESS:      // eXcess configurable cpu
                    case EM_F2MC16:      // Fujitsu F2MC16
                    case EM_FAKE_ALPHA:  // Digital Alpha
                    case EM_FIREPATH:    // Element 14 64-bit DSP Processor
                    case EM_FR20:        // Fujitsu FR20
                    case EM_FR30:        // Fujitsu FR30
                    case EM_FT32:        // FTDI Chip FT32
                    case EM_FX66:        // Siemens FX66 microcontroller
                    case EM_H8S:         // Hitachi H8S
                    case EM_H8_300:      // Hitachi H8/300
                    case EM_H8_300H:     // Hitachi H8/300H
                    case EM_H8_500:      // Hitachi H8/500
                    case EM_HUANY:       // Harvard University
                                         // machine-independent object files
                    case EM_IAMCU:       // Intel MCU
                    case EM_IA_64:       // Intel Merced
                    case EM_INTELGT:     // Intel Graphics Technology
                    case EM_IP2K:        // Ubicom IP2xxx
                    case EM_JAVELIN:     // Infineon Technologies 32-bit
                                         // emb.proc
                    case EM_K10M:        // Intel K10M
                    case EM_KM32:        // KM211 KM32
                    case EM_KMX32:       // KM211 KMX32
                    case EM_KVARC:       // KM211 KVARC
                    case EM_L10M:        // Intel L10M
                    case EM_LATTICEMICO32:  // RISC for Lattice FPGA
                    case EM_M16C:           // Renesas M16C
                    case EM_M32:            // AT&T WE 32100
                    case EM_M32C:           // Renesas M32C
                    case EM_M32R:           // Mitsubishi M32R
                    case EM_MANIK:          // M2000 Reconfigurable RISC
                    case EM_MAX:            // MAX processor
                    case EM_MAXQ30:         // Dallas Semi. MAXQ30 mc
                    case EM_MCHP_PIC:       // Microchip 8-bit PIC(r)
                    case EM_MCST_ELBRUS:    // MCST Elbrus
                    case EM_ME16:           // Toyota ME16 processor
                    case EM_METAG:          // Imagination Tech. META
                    case EM_MICROBLAZE:     // Xilinx MicroBlaze
                    case EM_MIPS:           // MIPS R3000 big-endian
                    case EM_MIPS_RS3_LE:    // MIPS R3000 little-endian
                    case EM_MIPS_X:         // Stanford MIPS-X
                    case EM_MMA:         // Fujitsu MMA Multimedia Accelerator
                    case EM_MMDSP_PLUS:  // STMicroelectronics 64bit VLIW
                                         // DSP
                    case EM_MMIX:      // Donald Knuth's educational 64-bit proc
                    case EM_MN10200:   // Matsushita MN10200
                    case EM_MN10300:   // Matsushita MN10300
                    case EM_MOXIE:     // Moxie processor
                    case EM_MSP430:    // Texas Instruments msp430
                    case EM_NCPU:      // Sony nCPU embeeded RISC
                    case EM_NDR1:      // Denso NDR1 microprocessor
                    case EM_NDS32:     // Andes Tech. compact code emb. RISC
                    case EM_NONE:      // No machine
                    case EM_NORC:      // Nanoradio Optimized RISC
                    case EM_NS32K:     // National Semi. 32000
                    case EM_OPEN8:     // Open8 RISC
                    case EM_OPENRISC:  // OpenRISC 32-bit embedded processor
                    case EM_PARISC:    // HPPA
                    case EM_PCP:       // Siemens PCP
                    case EM_PDP10:     // Digital PDP-10
                    case EM_PDP11:     // Digital PDP-11
                    case EM_PDSP:      // Sony DSP Processor
                    case EM_PJ:        // picoJava
                    case EM_PPC64:     // PowerPC 64-bit
                    case EM_PPC:       // PowerPC
                    case EM_PRISM:     // SiTera Prism
                    case EM_QDSP6:     // QUALCOMM DSP6
                    case EM_R32C:      // Renesas R32C
                    case EM_RCE:       // Motorola RCE
                    case EM_RH32:      // TRW RH-32
                    case EM_RISCV:     // RISC-V
                    case EM_RL78:      // Renesas RL78
                    case EM_RS08:      // Freescale RS08
                    case EM_RX:        // Renesas RX
                    case EM_S370:      // IBM System/370
                    case EM_S390:      // IBM S390
                    case EM_SCORE7:    // Sunplus S+core7 RISC
                    case EM_SEP:       // Sharp embedded microprocessor
                    case EM_SE_C17:    // Seiko Epson C17
                    case EM_SE_C33:    // Seiko Epson S1C33 family
                    case EM_SH:        // Hitachi SH
                    case EM_SHARC:     // Analog Devices SHARC family
                    case EM_SLE9X:     // Infineon Tech. SLE9X
                    case EM_SNP1K:     // Trebia SNP 1000
                    case EM_SPARC32PLUS:  // Sun's "v8plus"
                    case EM_SPARC:        // SUN SPARC
                    case EM_SPARCV9:      // SPARC v9 64-bit
                    case EM_SPU:          // IBM SPU/SPC
                    case EM_ST100:        // STMicroelectronic ST100 processor
                    case EM_ST19:         // STMicroelectronics ST19 8 bit mc
                    case EM_ST200:        // STMicroelectronics ST200
                    case EM_ST7:          // STmicroelectronics ST7 8 bit mc
                    case EM_ST9PLUS:      // STMicroelectronics ST9+ 8/16 mc
                    case EM_STARCORE:     // Motorola Start*Core processor
                    case EM_STM8:         // STMicroelectronics STM8
                    case EM_STXP7X:       // STMicroelectronics STxP7x
                    case EM_SVX:          // Silicon Graphics SVx
                    case EM_TILE64:       // Tilera TILE64
                    case EM_TILEGX:       // Tilera TILE-Gx
                    case EM_TILEPRO:      // Tilera TILEPro
                    case EM_TINYJ:        // Advanced Logic Corp. Tinyj emb.fam
                    case EM_TI_ARP32:     // Texas Instruments App. Specific
                                          // RISC
                    case EM_TI_C2000:     // Texas Instruments TMS320C2000 DSP
                    case EM_TI_C5500:     // Texas Instruments TMS320C55x DSP
                    case EM_TI_C6000:     // Texas Instruments TMS320C6000 DSP
                    case EM_TI_PRU:       // Texas Instruments Prog. Realtime
                                          // Unit
                    case EM_TMM_GPP:      // Thompson Multimedia General Purpose
                                          // Proc
                    case EM_TPC:          // Tenor Network TPC
                    case EM_TRICORE:      // Siemens Tricore
                    case EM_TRIMEDIA:     // NXP Semi. TriMedia
                    case EM_TSK3000:      // Altium TSK3000
                    case EM_UNICORE:      // PKU-Unity & MPRC Peking Uni. mc
                                          // series
                    case EM_V800:         // NEC V800 series
                    case EM_V850:         // NEC v850
                    case EM_VAX:          // Digital VAX
                    case EM_VIDEOCORE3:   // Broadcom VideoCore III
                    case EM_VIDEOCORE5:   // Broadcom VideoCore V
                    case EM_VIDEOCORE:    // Alphamosaic VideoCore
                    case EM_VISIUM:       // Controls and Data Services
                                          // VISIUMcore
                    case EM_VPP500:       // Fujitsu VPP500
                    case EM_XCORE:        // XMOS xCORE
                    case EM_XGATE:        // Motorola XGATE
                    case EM_XIMO16:       // New Japan Radio (NJR) 16-bit DSP
                    case EM_XTENSA:       // Tensilica Xtensa Architecture
                    case EM_Z80:          // Zilog Z80
                    case EM_ZSP:          // LSI Logic 16-bit DSP Processor
                        TLO_TRACE("Bad proc0");
                        return k_unusable;
                    default:
                        TLO_TRACE("Bad proc1");
                        return k_invalid;
                }
            }

            // Sanity Check `e_version` field
            {
                // Non-matching version.
                if (base_->e_version != base_->e_ident[EI_VERSION]) {
                    TLO_TRACE("Bad version0");
                    return k_invalid;
                }
                switch (base_->e_version) {
                    case EV_CURRENT:  //   Current version
                        break;
                    case EV_NONE:
                    default:
                        TLO_TRACE("Bad version1");
                        return k_invalid;
                }
            }

            // TODO: Sanity check 'e_entry'
            {

            }

            // Sanity Check `ph_*` (prog header) fields
            {
                // This is technically possible, but we not via normal modes
                // of compilation.
                if (base_->e_phoff < base_->e_ehsize) {
                    TLO_TRACE("Bad phsize");
                    return k_unusable;
                }

                // program header entries are zero-sized. This doesn't make
                // sense (should be either elf64 or elf32 size).
                if (base_->e_phentsize == 0) {
                    TLO_TRACE("Bad phesize");
                    return k_invalid;
                }

                if (base_->e_phnum == 0) {
                    // No program headers...
                    TLO_TRACE("Bad phnum");
                    return k_unusable;
                }
            }

            // Sanity Check `sh_*` (sect header) fields
            {
                // This is technically possible, but we not via normal modes
                // of compilation.
                if (base_->e_shoff <= base_->e_ehsize) {
                    TLO_TRACE("Bad shsize: %zu vs %hu", base_->e_shoff,
                              base_->e_ehsize);
                    return k_unusable;
                }

                // section header entries are zero-sized. This doesn't make
                // sense (should be either elf64 or elf32 size).
                if (base_->e_shentsize == 0) {
                    TLO_TRACE("Bad shesize");
                    return k_invalid;
                }

                // If number of sections >= SHN_LORESERVE, then e_shnum is
                // zero and we use `sh_size` of first section header.
                if (base_->e_shnum >= SHN_LORESERVE) {
                    TLO_TRACE("Bad shnum");
                    return k_invalid;
                }
            }

            // Sanity check `e_shstrndx` (sect_strtab) field.
            {
                switch (base_->e_shstrndx) {
                    case SHN_UNDEF:   // No sect_strtab.
                    case SHN_XINDEX:  // sect_strtab is in `sh_link` of
                                      // first section header.
                        break;
                    default:
                        // If `e_shstrndx` >= SHN_LORESERVE, field should be
                        // `SHN_XINDEX`.
                        if (base_->e_shstrndx >= SHN_LORESERVE) {
                            TLO_TRACE("Bad strnndx");
                            return k_invalid;
                        }
                        break;
                }
            }

            return k_okay;
        }

        vstatus_t
        validate(const elf_file_t * ef) const {
            if (!load_okay()) {
                return k_invalid;
            }
            vstatus_t res = sanity_check();
            if (res != k_okay) {
                TLO_TRACE("Bad Sanity!");
                return res;
            }

            // Now we do some actual consistency checks.

            // Check section headers first, then prog/sect_strtab. We may
            // need to use sect info to actually determine state of prog
            // size.
            if (ef->get_sect_hdr_load_size() != sect_hdr_t::load_reqsize()) {
                TLO_TRACE("BAD LOAD");
                return k_unusable;
            }
            // Section headers overlap with elf header.
            if (ef->get_sect_hdrs_load_begin() < ef->get_elf_hdrs_load_end()) {
                return k_unusable;
            }

            // `e_shnum` (number of section headers in elfheader) should
            // only be zero if we are deferming to `sh_size` of first
            // section header.
            if (base_->e_shnum == 0) {
                if (ef->get_num_sect_hdrs() < SHN_LORESERVE) {
                    return k_invalid;
                }
            }

            // Check basics of prog headers.
            if (ef->get_prog_hdr_load_size() != prog_hdr_t::load_reqsize()) {
                return k_unusable;
            }

            // Section headers overlap with elf header.
            if (ef->get_prog_hdrs_load_begin() < ef->get_elf_hdrs_load_end()) {
                return k_unusable;
            }

            // If `e_phnum` has then it means `sh_info` of first section
            // header has real count and count >= PN_XNUM.
            if (base_->e_phnum == PN_XNUM) {
                if (ef->get_num_prog_hdrs() < PN_XNUM) {
                    return k_invalid;
                }
            }

            // If `e_shstrndx` is `SHN_XINDEX` it means `sh_link` of first
            // section header has real found. Only true of index >=
            // SHN_LORESERVE.
            if (base_->e_shstrndx == SHN_XINDEX) {
                if (ef->get_sect_strtab_hdr_index() < SHN_LORESERVE) {
                    return k_invalid;
                }
            }

            // If we have a sect_strtab, make sure its in range.
            if (base_->e_shstrndx != SHN_UNDEF) {
                if (ef->get_sect_strtab_hdr_index() >=
                    ef->get_num_sect_hdrs()) {
                    return k_invalid;
                }
            }

            // Check for any overlap between prog headers and sect headers
            // regions.
            size_t prog_hdrs_begin = ef->get_prog_hdrs_load_begin();
            size_t prog_hdrs_end   = ef->get_prog_hdrs_load_end();

            size_t sect_hdrs_begin = ef->get_sect_hdrs_load_begin();
            size_t sect_hdrs_end   = ef->get_sect_hdrs_load_end();

            // This is technically possible if we have zero-sized sections,
            // so just return unusable.
            if (prog_hdrs_begin == sect_hdrs_begin ||
                prog_hdrs_end == sect_hdrs_end) {
                return k_unusable;
            }
            if (prog_hdrs_begin >= sect_hdrs_begin &&
                prog_hdrs_begin < sect_hdrs_end) {
                return k_invalid;
            }
            if (sect_hdrs_begin >= prog_hdrs_begin &&
                sect_hdrs_begin < prog_hdrs_end) {
                return k_invalid;
            }
            return k_okay;
        }
    };

    struct prog_hdr_t : hdr_base_t<Elf64_Phdr> {
        template<typename... Ts_t>
        prog_hdr_t(Ts_t... ts)
            : hdr_base_t<Elf64_Phdr>(std::forward<Ts_t>(ts)...) {}
        constexpr prog_hdr_t() = default;
        vstatus_t
        validate(const elf_file_t * ef,
                 bool               first,
                 size_t *           last_ld_vaddr) const {
            if (!load_okay()) {
                return k_invalid;
            }

            // Validate check the `p_type` field
            switch (base_->p_type) {
                    // TODO: Integrate p_vaddr/p_paddr into these checks.
                case PT_LOAD:  // Loadable program segment
                    // PT_LOAD segments are supposed to be sorted by vaddr.
                    if (base_->p_vaddr < *last_ld_vaddr) {
                        TLO_TRACE("Bad addr order: %zu vs %zu", base_->p_vaddr,
                                  *last_ld_vaddr);
                        return k_invalid;
                    }
                    *last_ld_vaddr = base_->p_vaddr;
                    break;
                case PT_PHDR:  // Entry for header table itself
                    // Should only occur once and only at the first entry.
                    if (!first) {
                        TLO_TRACE("Bad first");
                        return k_invalid;
                    }
                    break;
                case PT_NULL:          // Program header table entry unused
                case PT_DYNAMIC:       // Dynamic linking information
                case PT_INTERP:        // Program interpreter
                case PT_NOTE:          // Auxiliary information
                case PT_TLS:           // Thread-local storage segment
                case PT_GNU_EH_FRAME:  // GCC .eh_frame_hdr segment
                case PT_GNU_STACK:     // Indicates stack executability
                case PT_GNU_RELRO:     // RELR relative relocations
                case PT_GNU_PROPERTY:  // GNU property
                case PT_SUNWBSS:       // Sun Specific segment.
                case PT_SUNWSTACK:     // Stack segment
                    break;
                case PT_SHLIB:  // Reserved. The ELF file is non-conforming
                                // to any known ABI. Skip it...
                    TLO_TRACE("Bad type0");
                    return k_invalid;
                default:
                    // In processor specific range.
                    if (base_->p_type >= PT_LOPROC &&
                        base_->p_type <= PT_HIPROC) {
                        break;
                    }
                    // In OS specific range.
                    if (base_->p_type >= PT_LOOS && base_->p_type <= PT_HIOS) {
                        break;
                    }
                    // Sun specific range.
                    if (base_->p_type >= PT_LOSUNW &&
                        base_->p_type <= PT_HISUNW) {
                    }
                    TLO_TRACE("Bad type1");
                    return k_invalid;
            }

            if (!ef->is_debug()) {
                // Segment is not part of the file...
                if (!ef->loadable(base_->p_offset, base_->p_filesz)) {
                    TLO_TRACE("Bad size0");
                    return k_invalid;
                }
                // memsz is always larger than filesz.
                if (base_->p_memsz < base_->p_filesz) {
                    TLO_TRACE("Bad size1");
                    return k_invalid;
                }

                // TODO: Ensure bytes from p_offset +[p_filesz, p_memsz) are all
                // zerod (or unmapped).

                // TODO: Reevaluate if these checks are useful...
                // We aren't touching any elfs that have write+execute bits.
                if ((base_->p_flags & PF_X) && (base_->p_flags & PF_W)) {
                    // return k_unusable;
                }
                // Likewise no segment should have absolutely no permissions.
                if (!(base_->p_flags & (PF_X | PF_R | PF_W))) {
                    // return k_unusable;
                }

                if (base_->p_align) {
                    // alignment must be a power of 2.
                    if (!is_pow2(base_->p_align)) {
                        TLO_TRACE("Bad align0");
                        return k_invalid;
                    }
                    // The page offsets in file and of mapping should match.
                    if ((base_->p_vaddr %
                         std::min(base_->p_align, k_page_size)) !=
                        (base_->p_offset %
                         std::min(base_->p_align, k_page_size))) {
                        TLO_TRACE("Bad align1");
                        return k_invalid;
                    }
                }
            }
            return k_okay;
        }
    };


    struct sect_hdr_t : hdr_base_t<Elf64_Shdr> {
        template<typename... Ts_t>
        constexpr sect_hdr_t(Ts_t... ts)
            : hdr_base_t<Elf64_Shdr>(std::forward<Ts_t>(ts)...) {}
        constexpr sect_hdr_t() = default;
        strtab_t::str_t
        section_name(const elf_file_t * ef) const {
            return ef->load_sect_name(base_->sh_name);
        }


        bool
        section_is_undef() const {
            assert(load_okay());
            return base_->sh_type == SHT_NULL;
        }

        size_t
        get_section_image_addr() const {
            assert(load_okay());
            if (section_is_undef() || base_->sh_addr == 0) {
                return k_invalid_offset;
            }
            return base_->sh_addr;
        }

        size_t
        get_section_load_begin() const {
            assert(load_okay());
            if (section_is_undef() || base_->sh_offset == 0) {
                return k_invalid_offset;
            }
            return base_->sh_offset;
        }

        size_t
        get_section_entry_load_size() const {
            assert(load_okay());
            if (section_is_undef() || base_->sh_entsize == 0) {
                return k_invalid_size;
            }
            return base_->sh_entsize;
        }

        size_t
        get_section_num_entries() const {
            if (section_is_undef()) {
                return k_invalid_size;
            }

            size_t sect_size     = get_section_load_size();
            size_t sect_ent_size = get_section_entry_load_size();
            if (sect_size == k_invalid_size ||
                sect_ent_size == k_invalid_size) {
                return k_invalid_size;
            }

            if ((sect_size % sect_ent_size) != 0) {
                return k_invalid_size;
            }

            return sect_size / sect_ent_size;
        }

        size_t
        get_section_load_end() const {
            size_t section_begin = get_section_load_begin();
            size_t section_size  = get_section_load_size();

            if (section_begin == k_invalid_offset ||
                section_size == k_invalid_size) {
                return k_invalid_offset;
            }
            return section_begin + section_size;
        }

        bool
        section_is_loadable(const elf_file_t * ef,
                            size_t at_offset = k_invalid_offset) const {
            size_t section_begin = get_section_load_begin();
            size_t section_size  = get_section_load_size();

            if (section_begin == k_invalid_offset ||
                section_size == k_invalid_size) {
                return false;
            }
            if (section_size == 0) {
                return false;
            }
            if (at_offset != k_invalid_offset) {
                if (at_offset < section_begin ||
                    at_offset >= (section_begin + section_size)) {
                    return false;
                }
            }

            return ef->loadable(section_begin, section_size);
        }

        bool
        section_is_index_loadable(const elf_file_t * ef,
                                  size_t at_index = k_invalid_index) const {
            size_t ent_size = get_section_entry_load_size();
            if (ent_size == k_invalid_size) {
                return false;
            }
            if (at_index != k_invalid_index) {
                at_index = get_section_load_begin() + at_index * ent_size;
            }
            if (!section_is_loadable(ef, at_index)) {
                return false;
            }
            return true;
        }


        size_t
        get_section_load_size() const {
            if (section_is_undef() || base_->sh_size == 0) {
                return k_invalid_size;
            }
            return base_->sh_size;
        }

        // Section types:
        // SHT_NOBITS:
        //      bss: alloc, write
        // SHT_PROGBITS:
        //      fini: alloc, exec
        //      init: alloc, exec
        //      note.GNU-stack: exec
        //      text: alloc, exec
        //      plt: (opt:all)
        //      comment: none
        //      debug: none
        //      got: none
        //      line: none
        //      ctors: alloc, write
        //      data: alloc, write
        //      data1: alloc, write
        //      dtor: alloc, write
        //      dynamic: alloc (opt:write)
        //      rodata: alloc
        //      rodata1: alloc
        //      interp: (opt:alloc)
        // SHT_STRTAB:
        //      dynstr: alloc
        //      strtab: alloc
        //      shstrtab: none
        // SHT_GNU_versym:
        //      gnu.version: alloc
        //      gnu.version_r: alloc
        // SHT_GNU_verdef:
        //      gnu.version_d: alloc
        // SHT_HASH:
        //      hash: alloc
        // SHT_NOTE:
        //      note.ABI-tag: alloc
        //      note.gnu.build-id: alloc
        // SHT_REL:
        //      relNAME: alloc
        // SHT_RELA:
        //      relaNAME: alloc
        // SHT_SYMTAB:
        //      symtab: alloc
        //      dyntab: alloc

        bool
        is_text() const {
            return base_->sh_type == SHT_PROGBITS &&
                   base_->sh_flags == (SHF_ALLOC | SHF_EXECINSTR);
        }

        bool
        is_nobits_text() const {
            return base_->sh_type == SHT_NOBITS &&
                   base_->sh_flags == (SHF_ALLOC | SHF_EXECINSTR);
        }

        bool
        is_dynamic_info() const {
            return base_->sh_type == SHT_DYNAMIC &&
                   ((base_->sh_flags & SHF_ALLOC) != 0);
        }

        bool
        is_symtab() const {
            return is_ssymtab() || is_dsymtab();
        }

        bool
        is_ssymtab() const {
            return (base_->sh_type == SHT_SYMTAB) &&
                   (base_->sh_flags == 0 || base_->sh_flags == SHF_ALLOC);
        }

        bool
        is_nobits() const {
            return base_->sh_type == SHT_NOBITS;
        }

        bool
        is_note() const {
            return base_->sh_type == SHT_NOTE && base_->sh_flags == SHF_ALLOC;
        }

        bool
        is_dsymtab() const {
            assert(load_okay());
            return base_->sh_type == SHT_DYNSYM &&
                   (base_->sh_flags == 0 || base_->sh_flags == SHF_ALLOC);
        }

        bool
        is_strtab() const {
            // Intentionally excludes section name strtab (shstrtab).
            return base_->sh_type == SHT_STRTAB &&
                   (base_->sh_flags == 0 || base_->sh_flags == SHF_ALLOC);
        }

        bool
        is_versym_indexes() const {
            return base_->sh_type == SHT_GNU_versym &&
                   base_->sh_flags == SHF_ALLOC;
        }

        bool
        is_versym_need() const {
            return base_->sh_type == SHT_GNU_verneed &&
                   base_->sh_flags == SHF_ALLOC;
        }

        bool
        is_versym_defs() const {
            return base_->sh_type == SHT_GNU_verdef &&
                   base_->sh_flags == SHF_ALLOC;
        }


        size_t
        get_assosiated_strtab_index() const {
            assert(load_okay());
            return base_->sh_link;
        }

        strtab_t
        load_assosiated_strtab(const elf_file_t * ef) const {
            size_t   strtab_index = get_assosiated_strtab_index();
            strtab_t strtab       = ef->load_strtab(strtab_index);
            if (!strtab.load_okay()) {
                return {};
            }
            return strtab;
        }

        strtab_t::str_t
        load_name_from_assosiated_strtab(const elf_file_t * ef,
                                         size_t             name_offset) const {
            strtab_t strtab = load_assosiated_strtab(ef);
            if (!strtab.load_okay()) {
                return {};
            }
            return strtab.load_name(name_offset);
        }


        size_t
        get_2nd_sect_strtab_hdr_index() const {
            assert(load_okay());
            return base_->sh_link;
        }

        size_t
        get_2nd_num_prog_hdrs() const {
            assert(load_okay());
            return base_->sh_info;
        }

        size_t
        get_2nd_num_sect_hdrs() const {
            assert(load_okay());
            return base_->sh_size;
        }

        vstatus_t
        validate(const elf_file_t * ef, bool first) {
            if (first) {
                // All fields at zero except: sh_size, sh_info, sh_link
                if (base_->sh_name == 0 && base_->sh_type == 0 &&
                    base_->sh_flags == 0 && base_->sh_addr == 0 &&
                    base_->sh_offset == 0 && base_->sh_addralign == 0 &&
                    base_->sh_entsize == 0) {
                    return k_okay;
                }
                TLO_TRACE("Bad first");
                return k_invalid;
            }

            switch (base_->sh_type) {
                case SHT_NULL:  // Section header table entry unused
                    // Rest of members are undefined.
                    return k_okay;
                case SHT_PROGBITS:        // Program data
                case SHT_SYMTAB:          // Symbol table
                case SHT_STRTAB:          // String table
                case SHT_RELA:            // Relocation entries with addends
                case SHT_HASH:            // Symbol hash table
                case SHT_DYNAMIC:         // Dynamic linking information
                case SHT_NOTE:            // Notes
                case SHT_NOBITS:          // Program space with no data (bss)
                case SHT_REL:             // Relocation entries, no addends
                case SHT_SHLIB:           // Reserved
                case SHT_DYNSYM:          // Dynamic linker symbol table
                case SHT_INIT_ARRAY:      // Array of constructors
                case SHT_FINI_ARRAY:      // Array of destructors
                case SHT_PREINIT_ARRAY:   // Array of pre-constructors
                case SHT_GROUP:           // Section group
                case SHT_SYMTAB_SHNDX:    // Extended section indices
                case SHT_GNU_ATTRIBUTES:  // Object attributes.
                case SHT_GNU_HASH:        // GNU-style hash table.
                case SHT_GNU_LIBLIST:     // Prelink library list
                case SHT_CHECKSUM:        // Checksum for DSO content.
                case SHT_SUNW_move:       // Sun-specific low bound. (Also:
                                          // SHT_LOSUNW)
                case SHT_SUNW_COMDAT:     //
                case SHT_SUNW_syminfo:    //
                case SHT_GNU_verdef:      // Version definition section.
                case SHT_GNU_verneed:     // Version needs section.
                case SHT_RELR:            // Relative relocation entries.
                    break;
                default:
                    // In processor specific range.
                    if (base_->sh_type >= SHT_LOPROC &&
                        base_->sh_type <= SHT_HIPROC) {
                        break;
                    }
                    // In OS specific range.
                    if (base_->sh_type >= SHT_LOOS &&
                        base_->sh_type <= SHT_HIOS) {
                        break;
                    }
                    // In application specific range.
                    if (base_->sh_type >= SHT_LOUSER &&
                        base_->sh_type <= SHT_HIUSER) {
                        break;
                    }
                    // Sun specific.
                    if (base_->sh_type >= SHT_LOSUNW &&
                        base_->sh_type <= SHT_HISUNW) {
                        break;
                    }
                    TLO_perr("Warning: invalid section type: %u\n",
                             base_->sh_type);
                    break;
            }


            // Possibly self modifying code.
            // TODO: Maybe support this!
            if ((base_->sh_flags & SHF_WRITE) &&
                (base_->sh_flags & SHF_EXECINSTR)) {
                TLO_TRACE("Bad flags");
                return k_unusable;
            }
            // This is in memory
            if (base_->sh_flags & SHF_ALLOC) {
                // No address for us to be at!
                // NOTE: TLS is special. Note sure why but some applications
                // have no-addr for TLS allocation.
                if (!(base_->sh_flags & SHF_TLS) && base_->sh_addr == 0) {
                    TLO_TRACE("Bad addr0: %s", section_name(ef).data());
                    return k_unusable;
                }
            }
            else {
                // We have an address in nowhere!
                // NOTE: Progbits seem to have some exceptions here.
                if (base_->sh_type != SHT_PROGBITS && base_->sh_addr != 0) {
                    TLO_TRACE("Bad addr1: %s: %lx (%lx)",
                              section_name(ef).data(), base_->sh_flags,
                              base_->sh_addr);
                    return k_unusable;
                }
            }
            if (base_->sh_type != SHT_NOBITS) {
                // We cannot find this section.
                if (base_->sh_offset == 0) {
                    TLO_TRACE("Bad NB offset");
                    return k_unusable;
                }
                if (base_->sh_size != 0) {
                    // Section is in inaccessible memory.
                    if (!ef->loadable(base_->sh_offset, base_->sh_size)) {
                        TLO_TRACE("Bad region");
                        return k_invalid;
                    }
                }
                // Maybe exceptions. Not really sure the 100% rule (0 size
                // sounds hypothetically fine tbh).
                else if (base_->sh_type != SHT_PROGBITS) {
                    assert(0 && "This zero-size check is probably unneeded");
                    return k_unusable;
                }
            }
            if (base_->sh_addralign != 0) {
                // Must be pow2 alignment.
                if (!is_pow2(base_->sh_addralign)) {
                    TLO_TRACE("Bad align0");
                    return k_invalid;
                }
                // Actually address is not properly aligned.
                if ((base_->sh_addr % base_->sh_addralign) != 0) {
                    TLO_TRACE("Bad align1");
                    return k_invalid;
                }
            }

            // Entry sizes in this sections table should never exceed the
            // sections actual size.
            if (base_->sh_entsize > base_->sh_size) {
                TLO_TRACE("Bad entsize");
                return k_invalid;
            }

            return k_okay;
        }
    };

    template<typename sect_t, bool k_use_ref = false>
    struct sect_entry_it_t {

        size_t index_;
        using base_t =
            typename std::conditional_t<k_use_ref, const sect_t &, sect_t>;
        const base_t sect_;

        typename sect_t::entry_t
        operator*() const {
            return sect_.load_entry_index(index_);
        }

        sect_entry_it_t<sect_t, k_use_ref> &
        operator++() {
            ++index_;
            return *this;
        }

        bool
        operator!=(const sect_entry_it_t<sect_t, k_use_ref> & other) {
            return index_ != other.index_;
        }
    };


    struct sect_symtab_t : sect_hdr_t {
        const elf_file_t * ef_;

        constexpr sect_symtab_t() = default;
        constexpr sect_symtab_t(sect_hdr_t sh, const elf_file_t * ef)
            : sect_hdr_t(sh), ef_(ef) {}

        constexpr bool
        active() const {
            return ef_ != nullptr;
        }

        void
        init(sect_hdr_t sh, const elf_file_t * ef) {
            assert(!active());
            base_ = sh.base_;
            ef_   = ef;
        }

        size_t
        estimate_num_functions() const {
            assert(load_okay() && active());
            size_t total_entries = get_section_num_entries();
            if (total_entries == k_invalid_size) {
                return 0;
            }
            return total_entries;
        }


        struct symtab_entry_t : hdr_base_t<Elf64_Sym> {
            template<typename... Ts_t>
            constexpr symtab_entry_t(Ts_t... ts)
                : hdr_base_t<Elf64_Sym>(std::forward<Ts_t>(ts)...) {}
            constexpr symtab_entry_t() = default;

            size_t
            strtab_sect_index(const sect_hdr_t sh) const {
                assert(load_okay());
                // Using `sh_link` of header based on:
                // https://stackoverflow.com/questions/32088140/multiple-string-tables-in-elf-object
                return sh.base_->sh_link;
            }


            size_t
            get_assosiated_section(const elf_file_t * ef) const {
                assert(load_okay());
                size_t idx = base_->st_shndx;
                if (idx >= ef->get_num_sect_hdrs()) {
                    return k_invalid_index;
                }
                if (idx == SHN_ABS || idx == SHN_UNDEF) {
                    return k_invalid_index;
                }
                return idx;
            }

            size_t
            strtab_str_index() const {
                assert(load_okay());
                return base_->st_name;
            }

            strtab_t::str_t
            entry_name(const elf_file_t * ef, const sect_hdr_t sh) const {
                assert(load_okay() && sh.load_okay());
                size_t   strtab_index = strtab_sect_index(sh);
                strtab_t strtab       = ef->load_strtab(strtab_index);
                return strtab.load_name(strtab_str_index());
            }

            bool
            has_sym_size() const {
                assert(load_okay());
                return base_->st_size != 0;
            }

            bool
            has_sym_vaddr(const elf_file_t * ef) const {
                assert(load_okay());
                return (ef->is_sysv_abi() || ef->is_gnu_abi()) &&
                       ef->is_x86_64() &&
                       (ef->is_executable() || ef->is_shared_obj_or_pie()) &&
                       base_->st_value != 0;
            }

            bool
            has_findable_sym(const elf_file_t * ef) const {
                assert(load_okay());

                if (!has_sym_size() || !has_sym_vaddr(ef)) {
                    return false;
                }
                size_t assos_sh_idx = get_assosiated_section(ef);
                if (assos_sh_idx == k_invalid_index) {
                    return false;
                }
                const sect_hdr_t assos_sh =
                    ef->load_sect_hdr_index(assos_sh_idx);

                size_t sect_begin = assos_sh.get_section_load_begin();
                size_t sect_end   = assos_sh.get_section_load_end();

                if (sect_begin == k_invalid_offset ||
                    sect_end == k_invalid_offset) {
                    return false;
                }

                // This is kind of bullshit, we some symbols placed in custom
                // .text.* sections have end up with incorrect info here making
                // them appear out of bounds.
                if (ef->is_debug() && assos_sh.is_nobits_text()) {
                    return true;
                }

                if (sym_vaddr_begin() <= sect_begin) {
                    return false;
                }

                if (sym_vaddr_end() > sect_end) {
                    return false;
                }
                return true;
            }

            size_t
            sym_size() const {
                assert(load_okay());
                return base_->st_size;
            }

            size_t
            sym_vaddr_begin() const {
                assert(load_okay());
                return base_->st_value;
            }

            size_t
            sym_vaddr_end() const {
                assert(load_okay());
                return sym_vaddr_begin() + sym_size();
            }


            bool
            is_func() const {
                assert(load_okay());
                return get_st_type() == STT_FUNC;
            }

            bool
            has_func_info(const elf_file_t * ef) const {
                assert(load_okay());
                return is_func() && has_findable_sym(ef);
            }

            sym::func_t
            extract_func(const sect_symtab_t st,
                         const sym::dso_t *  dso,
                         strtab_t::str_t     name,
                         sym::ident_t        ident) const {
                assert(load_okay());
                assert(has_func_info(st.ef_));
                size_t vaddr_begin = sym_vaddr_begin();
                size_t vaddr_end   = sym_vaddr_end();
                return sym::func_t{ dso, true, name, ident,
                                    sym::addr_range_t{ vaddr_begin,
                                                       vaddr_end } };
            }

            sym::func_t
            extract_func(const sect_symtab_t st,
                         const sym::dso_t *  dso,
                         sym::ident_t        ident) const {
                assert(load_okay());
                assert(has_func_info(st.ef_));
                return extract_func(st, dso, entry_name(st.ef_, st), ident);
            }


            sym::func_t
            extract_func(const sect_symtab_t st, const sym::dso_t * dso) const {
                assert(load_okay());
                assert(has_func_info(st.ef_));
                strtab_t::str_t name = entry_name(st.ef_, st);
                auto            loc  = name.find('@');
                loc                  = std::min(loc, name.length());
                return extract_func(st, dso, name.substr(0, loc),
                                    sym::ident_t{ name.substr(loc) });
            }

            sym::func_t
            extract_func(const sect_symtab_t st,
                         const sym::dso_t *  dso,
                         std::string_view    ident_name,
                         bool                hidden) const {
                if (ident_name.data() == nullptr) {
                    ident_name = "";
                }
                return extract_func(
                    st, dso,
                    sym::ident_t{
                        ident_name,
                        ident_name.empty()
                            ? sym::ident_t::k_is_none
                            : (hidden ? sym::ident_t::k_is_hidden_ver
                                      : sym::ident_t::k_is_global_ver) });
            }

            size_t
            get_st_type() const {
                assert(load_okay());
                return ELF64_ST_TYPE(base_->st_info);
            }

            size_t
            get_st_bind() const {
                assert(load_okay());
                return ELF64_ST_BIND(base_->st_info);
            }

            size_t
            get_st_visibility() const {
                assert(load_okay());
                return ELF64_ST_VISIBILITY(base_->st_other);
            }


            vstatus_t
            validate(const sect_symtab_t sect, bool first) const {
                assert(load_okay());
                if (sect.get_section_entry_load_size() != load_reqsize()) {
                    return k_invalid;
                }

                if (first) {
                    if (base_->st_name != 0 || base_->st_value != 0 ||
                        base_->st_size != 0 || base_->st_info != 0 ||
                        base_->st_other != 0 || base_->st_shndx != SHN_UNDEF) {
                        return k_invalid;
                    }
                    return k_okay;
                }
                if (base_->st_name != 0) {
                    strtab_t::str_t symname =
                        sect.load_name_from_assosiated_strtab(sect.ef_,
                                                              base_->st_name);
                    if (strtab_t::is_invalid_str(symname)) {
                        return k_invalid;
                    }
                }
                // SYSV ABI:
                // https://www.sco.com/developers/devspecs/gabi41.pdf#page=70
                // If this is an executable or so, then st_value is address
                // of the symbol.
                // TODO: Do this check for GNU as well (some symbols seem to
                // not be loadable) in things like libc.

#if 0
                if ((ef->is_sysv_abi() || ef->is_gnu_abi()) &&
                    ef->is_x86_64() &&
                    (ef->is_executable() || ef->is_shared_obj_or_pie())) {
                    if (base_->st_value != 0 && base_->st_size != 0) {
                        if (!ef->loadable(base_->st_value, base_->st_size)) {
                            return k_invalid;
                        }

                        size_t assos_sect_index = get_assosiated_section(ef);
                        if (assos_sect_index == k_invalid_index) {
                            return k_invalid;
                        }

                        if (assos_sect_index >= ef->get_num_sect_hdrs()) {
                            return k_invalid;
                        }

                        const sect_hdr_t assos_sh =
                            ef->load_sect_hdr_index(assos_sect_index);

                        size_t sect_begin = assos_sh.get_section_load_begin();
                        size_t sect_end   = assos_sh.get_section_load_end();
                        if (sect_begin == k_invalid_offset ||
                            sect_end == k_invalid_offset) {
                            return k_invalid;
                        }
                        // less-equals because first sym is reserved (handled
                        // above) so we should always be past begining.
                        if (base_->st_value <= sect_begin) {
                            return k_invalid;
                        }
                        if ((base_->st_value + base_->st_size) > sect_end) {
                            return k_invalid;
                        }
                    }
                }
#endif

                switch (get_st_type()) {
                    // Symbol Types:
                    case STT_NOTYPE:     // Symbol type is unspecified
                    case STT_OBJECT:     // Symbol is a data object
                    case STT_FUNC:       // Symbol is a code object
                    case STT_SECTION:    // Symbol associated with a section
                    case STT_FILE:       // Symbol's name is file name
                    case STT_COMMON:     // Symbol is a common data object
                    case STT_TLS:        // Symbol is thread-local data object
                    case STT_GNU_IFUNC:  // Symbol is indirect code object
                        break;
                    default:
                        // In processor specific range.
                        if (base_->st_info >= STT_LOPROC &&
                            base_->st_info <= STT_HIPROC) {
                            break;
                        }
                        // In OS specific range.
                        if (base_->st_info >= STT_LOOS &&
                            base_->st_info <= STT_HIOS) {
                            break;
                        }
                        return k_invalid;
                }
                switch (get_st_bind()) {
                    // Symbol bindings:
                    case STB_LOCAL:       // Local symbol
                    case STB_GLOBAL:      // Global symbol
                    case STB_WEAK:        // Weak symbol
                    case STB_GNU_UNIQUE:  // Unique symbol.
                        break;
                    default:
                        // In processor specific range.
                        if (base_->st_info >= STB_LOPROC &&
                            base_->st_info <= STB_HIPROC) {
                            break;
                        }
                        // In OS specific range.
                        if (base_->st_info >= STB_LOOS &&
                            base_->st_info <= STB_HIOS) {
                            break;
                        }
                        return k_invalid;
                }
                switch (get_st_visibility()) {
                    case STV_DEFAULT:    // Default symbol visibility rules
                    case STV_INTERNAL:   // Processor specific hidden class
                    case STV_HIDDEN:     // Sym unavailable in other modules
                    case STV_PROTECTED:  // Not preemptible, not exported
                        break;
                    default:
                        return k_invalid;
                }
                if (base_->st_shndx >= sect.ef_->get_num_sect_hdrs() &&
                    base_->st_shndx != SHN_ABS &&
                    base_->st_shndx != SHN_UNDEF) {
                    return k_invalid;
                }
                return k_okay;
            }
        };

        symtab_entry_t
        load_entry(size_t offset) const {
            assert(load_okay());
            assert(is_symtab());
            if (!section_is_loadable(ef_, offset)) {
                return {};
            }
            return ef_->loadt<symtab_entry_t>(offset);
        }

        symtab_entry_t
        load_entry_index(size_t index) const {
            assert(load_okay());
            assert(is_symtab());
            if (!section_is_index_loadable(ef_, index)) {
                return {};
            }
            size_t ent_size   = get_section_entry_load_size();
            size_t ents_begin = get_section_load_begin();
            return load_entry(ents_begin + index * ent_size);
        }

        using entry_t = symtab_entry_t;
        constexpr sect_entry_it_t<sect_symtab_t>
        begin() const {
            return { 0, *this };
        }

        sect_entry_it_t<sect_symtab_t>
        end() const {
            return { get_section_num_entries(), *this };
        }

        vstatus_t
        validate_entries() const {
            bool first_symtab_ent = true;
            for (symtab_entry_t symtab_ent : *this) {
                vstatus_t res = symtab_ent.validate(*this, first_symtab_ent);
                if (res != k_okay) {
                    return res;
                }
                first_symtab_ent = false;
            }
            return k_okay;
        }
    };


    struct sect_versym_indexes_t : sect_hdr_t {
        const elf_file_t * ef_;


        constexpr sect_versym_indexes_t() = default;

        constexpr bool
        active() const {
            return ef_ != nullptr;
        }

        static constexpr bool
        is_index_usable(size_t index) {
            return index > 1;
        }

        static constexpr bool
        index_is_hidden(size_t index) {
            return index & 0x8000;
        }

        static constexpr uint16_t
        index_as_id(size_t index) {
            return index & 0x7fff;
        }

        void
        init(sect_hdr_t sh, const elf_file_t * ef) {
            assert(!active());
            base_ = sh.base_;
            ef_   = ef;
        }

        struct versym_index_t : hdr_base_t<Elf64_Half> {
            template<typename... Ts_t>
            versym_index_t(Ts_t... ts)
                : hdr_base_t<Elf64_Half>(std::forward<Ts_t>(ts)...) {}
            constexpr versym_index_t() = default;

            size_t
            get_index() const {
                assert(load_okay());
                return static_cast<size_t>(*base_);
            }

            vstatus_t
            validate(const sect_versym_indexes_t) const {
                return load_okay() ? k_okay : k_invalid;
            }
        };


        versym_index_t
        load_entry(size_t offset) const {
            assert(load_okay());
            assert(is_versym_indexes());
            if (!section_is_loadable(ef_, offset)) {
                return {};
            }
            return ef_->loadt<versym_index_t>(offset);
        }

        versym_index_t
        load_entry_index(size_t index) const {
            assert(load_okay());
            assert(is_versym_indexes());
            if (!section_is_index_loadable(ef_, index)) {
                return {};
            }
            size_t ent_size   = get_section_entry_load_size();
            size_t ents_begin = get_section_load_begin();
            return load_entry(ents_begin + index * ent_size);
        }

        using entry_t = versym_index_t;

        constexpr sect_entry_it_t<sect_versym_indexes_t>
        begin() const {
            return { 0, *this };
        }

        sect_entry_it_t<sect_versym_indexes_t>
        end() const {
            return { get_section_num_entries(), *this };
        }

        vstatus_t
        validate_entries() const {
            if (!load_okay()) {
                return k_invalid;
            }
            size_t assosiated_strtab_index = get_assosiated_strtab_index();
            if (assosiated_strtab_index != 0) {
                sect_hdr_t assosiated_strtab_sect =
                    ef_->load_sect_hdr_index(assosiated_strtab_index);
                if (!assosiated_strtab_sect.load_okay() ||
                    !assosiated_strtab_sect.is_dsymtab()) {
                    TLO_TRACE("Bad Assosiated Symtab: %zu (%d/%d)\n",
                              assosiated_strtab_index,
                              assosiated_strtab_sect.is_ssymtab(),
                              assosiated_strtab_sect.is_dsymtab());
                    return k_invalid;
                }
            }
            for (versym_index_t vi : *this) {
                vstatus_t res = vi.validate(*this);
                if (res != k_okay) {
                    return res;
                }
            }
            return k_okay;
        }
    };


    template<typename sect_t, typename ent_t>
    struct sect_entry_off_it_t {
        size_t         nremaining_;
        size_t         offset_;
        const sect_t * sect_;

        ent_t
        operator*() const {
            return sect_->template load_entry<ent_t>(offset_);
        }

        sect_entry_off_it_t<sect_t, ent_t> &
        operator++() {
            offset_ +=
                sect_->template load_entry<ent_t>(offset_).get_offset_to_next();
            --nremaining_;
            return *this;
        }

        bool
        operator!=(const sect_entry_off_it_t<sect_t, ent_t> & other) {
            return nremaining_ != other.nremaining_;
        }
    };


    struct sect_notes_t : sect_hdr_t {
        const elf_file_t * ef_;
        constexpr sect_notes_t() = default;
        constexpr sect_notes_t(sect_hdr_t sh, const elf_file_t * ef)
            : sect_hdr_t(sh), ef_(ef) {}

        constexpr bool
        active() const {
            return ef_ != nullptr;
        }

        void
        init(sect_hdr_t sh, const elf_file_t * ef) {
            assert(!active());
            base_ = sh.base_;
            ef_   = ef;
        }


        struct note_hdr_t : hdr_base_t<Elf64_Nhdr> {
            struct note_desc_t {
                std::span<const uint8_t> buf_;

                note_desc_t() = default;
                note_desc_t(const uint8_t * p, size_t sz) : buf_(p, sz) {}
            };

            struct note_name_t {
                std::string_view str_;
                note_name_t() = default;
                note_name_t(const uint8_t * p, size_t sz) {
                    if (sz == 0) {
                        str_ = { nullptr, 0 };
                    }
                    else {
                        str_ = { reinterpret_cast<const char *>(p), sz - 1 };
                    }
                }
            };

            template<typename... Ts_t>
            note_hdr_t(Ts_t... ts)
                : hdr_base_t<Elf64_Nhdr>(std::forward<Ts_t>(ts)...) {}
            constexpr note_hdr_t() = default;


            size_t
            name_size() const {
                assert(load_okay());
                return base_->n_namesz;
            }

            size_t
            desc_size() const {
                assert(load_okay());
                return base_->n_descsz;
            }

            size_t
            desc_offset() const {
                assert(load_okay());
                return name_offset() + roundup(name_size(), 4);
            }

            size_t
            name_offset() const {
                assert(load_okay());
                return load_reqsize();
            }

            size_t
            get_offset_to_next() const {
                assert(load_okay());
                return desc_offset() + roundup(desc_size(), 4);
            }

            note_name_t
            load_name(const sect_notes_t * sn, size_t base_off) const {
                return sn->load_note_name(base_off + name_offset(),
                                          name_size());
            }

            note_desc_t
            load_desc(const sect_notes_t * sn, size_t base_off) const {
                return sn->load_note_desc(base_off + desc_offset(),
                                          desc_size());
            }

            bool
            is_build_id(const sect_notes_t * sn, size_t base_off) const {
                if (!load_okay()) {
                    TLO_TRACE("Bad Load\n");
                    return false;
                }
                note_name_t name = load_name(sn, base_off);
                return name.str_ == "GNU" && base_->n_type == NT_GNU_BUILD_ID;
            }

            vstatus_t
            validate(const sect_notes_t * sn, size_t base_off) {
                if (!load_okay()) {
                    return k_invalid;
                }

                note_name_t name = load_name(sn, base_off);
                if (name.str_ == "GNU") {
                    switch (base_->n_type) {
                        case NT_GNU_ABI_TAG:
                            break;
                        case NT_GNU_HWCAP:
                            break;
                        case NT_GNU_BUILD_ID:
                            if (desc_size() < 8) {
                                TLO_TRACE("Tiny build id: %zu\n", desc_size());
                                return k_unusable;
                            }
                            break;
                        case NT_GNU_GOLD_VERSION:
                            break;
                        case NT_FDO_PACKAGING_METADATA:
                            break;
                        case NT_GNU_PROPERTY_TYPE_0:
                            break;
                        default:
                            TLO_TRACE("Unknown GNU type: %x\n", base_->n_type);
                            return k_unusable;
                    }
                }

                return k_okay;
            }
        };

        template<typename T_t>
        T_t
        load_note_field(size_t offset, size_t sz) const {
            assert(load_okay());
            if (!section_is_loadable(ef_, offset)) {
                return {};
            }
            return ef_->loadv<T_t>(offset, sz);
        }

        note_hdr_t::note_name_t
        load_note_name(size_t offset, size_t sz) const {
            return load_note_field<note_hdr_t::note_name_t>(offset, sz);
        }

        note_hdr_t::note_desc_t
        load_note_desc(size_t offset, size_t sz) const {
            return load_note_field<note_hdr_t::note_desc_t>(offset, sz);
        }

        note_hdr_t
        load_entry(size_t offset) const {
            assert(load_okay());
            if (!section_is_loadable(ef_, offset)) {
                return note_hdr_t{};
            }
            return ef_->loadt<note_hdr_t>(offset);
        }

        bool
        is_build_id() const {
            assert(load_okay());
            return load_entry(get_section_load_begin())
                .is_build_id(this, get_section_load_begin());
        }

        std::span<const uint8_t>
        load_build_id() const {
            assert(is_build_id());
            note_hdr_t note = load_entry(get_section_load_begin());
            return note.load_desc(this, get_section_load_begin()).buf_;
        }


        vstatus_t
        validate() const {
            assert(load_okay());
            note_hdr_t note = load_entry(get_section_load_begin());
            if (!note.load_okay()) {
                return k_unusable;
            }

            return note.validate(this, get_section_load_begin());
        }
    };


    struct sect_versym_defs_t : sect_hdr_t {
        static constexpr size_t k_vinfo_index = DT_VERDEF;

        sect_versym_defs_t() = default;

        constexpr bool
        active() const {
            return ef_ != nullptr;
        }

        void
        init(sect_hdr_t                  sh,
             const elf_file_t *          ef,
             const sect_versym_indexes_t vsi) {
            assert(!active());
            base_ = sh.base_;
            ef_   = ef;
            okay_ = validate_and_collect_entries(vsi);
        }

        struct versym_def_t : hdr_base_t<Elf64_Verdef> {
            template<typename... Ts_t>
            versym_def_t(Ts_t... ts)
                : hdr_base_t<Elf64_Verdef>(std::forward<Ts_t>(ts)...) {}
            constexpr versym_def_t() = default;

            struct versym_daux_t : hdr_base_t<Elf64_Verdaux> {
                template<typename... Ts_t>
                versym_daux_t(Ts_t... ts)
                    : hdr_base_t<Elf64_Verdaux>(std::forward<Ts_t>(ts)...) {}
                constexpr versym_daux_t() = default;
                size_t
                get_offset_to_next() const {
                    assert(load_okay());
                    return base_->vda_next;
                }

                size_t
                get_name_offset() const {
                    assert(load_okay());
                    return base_->vda_name;
                }

                strtab_t::str_t
                load_name(const sect_versym_defs_t * vsd) const {
                    assert(load_okay());
                    return vsd->load_name_from_assosiated_strtab(
                        vsd->ef_, get_name_offset());
                }

                vstatus_t
                validate(const sect_versym_defs_t * vsd) const {
                    if (!load_okay()) {
                        return k_invalid;
                    }

                    if (base_->vda_next == 0) {
                        return k_skip;
                    }
                    if (base_->vda_next != load_reqsize()) {
                        return k_invalid;
                    }

                    strtab_t::str_t aux_name = load_name(vsd).data();
                    if (strtab_t::is_invalid_str(aux_name)) {
                        return k_invalid;
                    }
                    return k_okay;
                }

                void
                dump(const sect_versym_defs_t * vsd, int vlvl) {
                    TLO_print_ifv(vlvl, "VDef: %s\n", load_name(vsd).data());
                }
            };

            vstatus_t
            validate(const sect_versym_defs_t *  vsd,
                     const sect_versym_indexes_t vsi,
                     size_t                      cur_off) const {
                if (!load_okay()) {
                    TLO_TRACE("Bad load");
                    return k_invalid;
                }

                // https://www.akkadia.org/drepper/symbol-versioning
                switch (base_->vd_version) {
                    case VER_DEF_CURRENT:
                        break;
                    case VER_DEF_NONE:
                    default:
                        TLO_TRACE("Bad version");
                        return k_invalid;
                }

                switch (base_->vd_flags) {
                    case 0:
                    case VER_FLG_BASE:
                    case VER_FLG_WEAK:
                        break;
                    default:
                        TLO_TRACE("Bad flags");
                        return k_invalid;
                }

                if (vsi.active()) {
                    sect_versym_indexes_t::versym_index_t vi =
                        vsi.load_entry_index(base_->vd_ndx);
                    if (!vi.load_okay()) {
                        TLO_TRACE("Bad index load");
                        return k_invalid;
                    }
                }

                if (base_->vd_cnt == 0) {
                    TLO_TRACE("Bad cnt");
                    return k_invalid;
                }

                if (base_->vd_aux < load_reqsize()) {
                    TLO_TRACE("Bad aux load");
                    return k_invalid;
                }
                if (base_->vd_next != 0 && base_->vd_next < load_reqsize()) {
                    TLO_TRACE("Bad next");
                    return k_invalid;
                }

                bool last = false;
                for (auto aux_it = begin(vsd, cur_off); aux_it != end();
                     ++aux_it) {
                    (*aux_it).dump(vsd, 3);
                    vstatus_t res = (*aux_it).validate(vsd);
                    if (res == k_skip) {
                        last = true;
                        continue;
                    }
                    if (res != k_okay) {
                        return res;
                    }
                    last = false;
                }
                if (!last) {
                    TLO_TRACE("Bad aux order");
                    return k_invalid;
                }


                return base_->vd_next == 0 ? k_skip : k_okay;
            }

            size_t
            get_vsym_index_match_field() const {
                assert(load_okay());
                return base_->vd_ndx;
            }

            size_t
            get_num_aux_entries() const {
                assert(load_okay());
                return base_->vd_cnt;
            }
            size_t
            get_aux_load_begin(size_t cur_off) const {
                assert(load_okay());
                return cur_off + base_->vd_aux;
            }

            size_t
            get_offset_to_next() const {
                assert(load_okay());
                return base_->vd_next;
            }

            sect_entry_off_it_t<sect_versym_defs_t, versym_daux_t>
            begin(const sect_versym_defs_t * vsd, size_t cur_off) const {
                return { get_num_aux_entries(), get_aux_load_begin(cur_off),
                         vsd };
            }

            constexpr sect_entry_off_it_t<sect_versym_defs_t, versym_daux_t>
            end() const {
                return { 0, 0, nullptr };
            }
        };

        template<typename ent_t>
        ent_t
        load_entry(size_t offset) const {
            assert(load_okay());
            assert(is_versym_defs());
            if (!section_is_loadable(ef_, offset)) {
                return {};
            }
            return ef_->loadt<ent_t>(offset);
        }

        using entry_t   = versym_def_t;
        using vdef_it_t = sect_entry_off_it_t<sect_versym_defs_t, versym_def_t>;

        size_t
        get_num_def_entries() const {
            return base_->sh_info;
        }


        std::pair<strtab_t::str_t, bool>
        find_ver_ident(size_t                        vsym_index,
                       sect_symtab_t::symtab_entry_t symtab_ent) const {
            assert(load_okay() && active());
            if (!sect_versym_indexes_t::is_index_usable(vsym_index)) {
                return { strtab_t::k_invalid_str, false };
            }
            auto res =
                ver_tbl_.find(sect_versym_indexes_t::index_as_id(vsym_index));
            if (res == ver_tbl_.end()) {
                return { strtab_t::k_invalid_str, false };
            }

            auto vd_it = res->second;
            for (auto aux_it = (*vd_it).begin(this, vd_it.offset_);
                 aux_it != (*vd_it).end(); ++aux_it) {
                if ((*aux_it).get_name_offset() !=
                    symtab_ent.strtab_str_index()) {
                    return { (*aux_it).load_name(this),
                             sect_versym_indexes_t::index_is_hidden(
                                 vsym_index) };
                }
            }

            return { strtab_t::k_invalid_str, false };
        }

        vstatus_t
        validate_and_collect_ver_tbl() {
            if (!load_okay()) {
                return k_invalid;
            }
            for (auto vd_it = begin(); vd_it != end(); ++vd_it) {
                auto res = ver_tbl_.emplace(
                    (*vd_it).get_vsym_index_match_field(), vd_it);
                if (!res.second) {
                    return k_invalid;
                }
            }
            return k_okay;
        }

        vstatus_t
        validate_and_collect_entries(const sect_versym_indexes_t vsi) {
            if (!load_okay()) {
                TLO_TRACE("Bad load");
                return k_invalid;
            }

            bool last = get_num_def_entries() == 0;
            for (auto vd_it = begin(); vd_it != end(); ++vd_it) {
                vstatus_t res = (*vd_it).validate(this, vsi, vd_it.offset_);
                if (res == k_skip) {
                    last = true;
                    continue;
                }
                if (res != k_okay) {
                    return res;
                }
                last = false;
            }
            if (!last) {
                TLO_TRACE("Bad order");
                return k_invalid;
            }
            return validate_and_collect_ver_tbl();
        }

        vstatus_t
        validate_entries() {
            assert(active());
            return okay_;
        }

        vdef_it_t
        begin() const {
            return { get_num_def_entries(), get_section_load_begin(), this };
        }

        constexpr vdef_it_t
        end() const {
            return { 0, 0, {} };
        }

        const elf_file_t *        ef_;
        umap<uint16_t, vdef_it_t> ver_tbl_;
        vstatus_t                 okay_;
    };

    struct sect_versym_need_t : sect_hdr_t {
        static constexpr size_t k_vinfo_index = DT_VERNEED;

        sect_versym_need_t() = default;

        constexpr bool
        active() const {
            return ef_ != nullptr;
        }

        void
        init(sect_hdr_t                  sh,
             const elf_file_t *          ef,
             const sect_versym_indexes_t vsi) {
            assert(!active());
            base_ = sh.base_;
            ef_   = ef;
            okay_ = validate_and_collect_entries(vsi);
        }

        struct versym_need_t : hdr_base_t<Elf64_Verneed> {
            template<typename... Ts_t>
            versym_need_t(Ts_t... ts)
                : hdr_base_t<Elf64_Verneed>(std::forward<Ts_t>(ts)...) {}
            constexpr versym_need_t() = default;

            struct versym_naux_t : hdr_base_t<Elf64_Vernaux> {
                template<typename... Ts_t>
                versym_naux_t(Ts_t... ts)
                    : hdr_base_t<Elf64_Vernaux>(std::forward<Ts_t>(ts)...) {}
                constexpr versym_naux_t() = default;

                size_t
                get_offset_to_next() const {
                    assert(load_okay());
                    return base_->vna_next;
                }

                size_t
                get_vsym_index_match_field() const {
                    assert(load_okay());
                    return base_->vna_other;
                }


                strtab_t::str_t
                load_name(const sect_versym_need_t * vsn) const {
                    assert(load_okay());
                    return vsn->load_name_from_assosiated_strtab(
                        vsn->ef_, base_->vna_name);
                }

                vstatus_t
                validate(const sect_versym_need_t * vsn) const {
                    if (!load_okay()) {
                        return k_invalid;
                    }

                    switch (base_->vna_flags) {
                        case 0:
                        case VER_FLG_BASE:
                        case VER_FLG_WEAK:
                            break;
                        default:
                            return k_invalid;
                    }
                    if (!sect_versym_indexes_t::is_index_usable(
                            base_->vna_other)) {
                        return k_invalid;
                    }

                    if (base_->vna_next == 0) {
                        return k_skip;
                    }
                    if (base_->vna_next != load_reqsize()) {
                        return k_invalid;
                    }
                    strtab_t::str_t aux_name = load_name(vsn).data();
                    if (strtab_t::is_invalid_str(aux_name)) {
                        return k_invalid;
                    }
                    return k_okay;
                }

                void
                dump(const sect_versym_need_t * vsn, int vlvl) {
                    TLO_print_ifv(vlvl, "VNeed: %s\n", load_name(vsn).data());
                }
            };

            size_t
            get_num_aux_entries() const {
                assert(load_okay());
                return base_->vn_cnt;
            }

            size_t
            get_aux_load_begin(size_t cur_off) const {
                assert(load_okay());
                return cur_off + base_->vn_aux;
            }

            size_t
            get_offset_to_next() const {
                assert(load_okay());
                return base_->vn_next;
            }


            strtab_t::str_t
            load_needed_dso_name(const sect_versym_need_t * vsn) const {
                assert(load_okay());
                return vsn->load_name_from_assosiated_strtab(vsn->ef_,
                                                             base_->vn_file);
            }

            sect_entry_off_it_t<sect_versym_need_t, versym_naux_t>
            begin(const sect_versym_need_t * vsn, size_t cur_off) const {
                return { get_num_aux_entries(), get_aux_load_begin(cur_off),
                         vsn };
            }

            constexpr sect_entry_off_it_t<sect_versym_need_t, versym_naux_t>
            end() const {
                return { 0, 0, {} };
            }

            vstatus_t
            validate(const sect_versym_need_t *  vsn,
                     const sect_versym_indexes_t vsi,
                     size_t                      cur_off) const {
                if (!load_okay()) {
                    return k_invalid;
                }
                switch (base_->vn_version) {
                    case VER_NEED_CURRENT:
                        break;
                    case VER_NEED_NONE:
                    default:
                        return k_invalid;
                }
                if (base_->vn_cnt == 0) {
                    return k_invalid;
                }
                if (strtab_t::is_invalid_str(load_needed_dso_name(vsn))) {
                    return k_invalid;
                }
                if (base_->vn_aux < load_reqsize()) {
                    return k_invalid;
                }

                bool last = false;
                for (auto aux_it = begin(vsn, cur_off); aux_it != end();
                     ++aux_it) {
                    (*aux_it).dump(vsn, 3);
                    vstatus_t res = (*aux_it).validate(vsn);
                    if (res == k_skip) {
                        last = true;
                        continue;
                    }
                    if (res != k_okay) {
                        return res;
                    }
                    last = false;
                }
                if (!last) {
                    return k_invalid;
                }
                (void)vsi;
                return base_->vn_next == 0 ? k_skip : k_okay;
            }
        };

        size_t
        get_num_need_entries() const {
            assert(load_okay());
            return base_->sh_info;
        }

        template<typename ent_t>
        ent_t
        load_entry(size_t offset) const {
            assert(load_okay());
            assert(is_versym_need());
            if (!section_is_loadable(ef_, offset)) {
                return {};
            }
            return ef_->loadt<ent_t>(offset);
        }

        using entry_t = versym_need_t;

        std::pair<strtab_t::str_t, bool>
        find_ver_ident(size_t                        vsym_index,
                       sect_symtab_t::symtab_entry_t symtab_ent) const {
            assert(load_okay() && active());
            if (!sect_versym_indexes_t::is_index_usable(vsym_index)) {
                return { strtab_t::k_invalid_str, false };
            }
            auto res =
                ver_tbl_.find(sect_versym_indexes_t::index_as_id(vsym_index));
            if (res == ver_tbl_.end()) {
                return { strtab_t::k_invalid_str, false };
            }

            (void)symtab_ent;
            return { res->second,
                     sect_versym_indexes_t::index_is_hidden(vsym_index) };
        }

        vstatus_t
        validate_and_collect_ver_tbl() {
            if (!load_okay()) {
                return k_invalid;
            }
            for (auto vn_it = begin(); vn_it != end(); ++vn_it) {
                for (auto aux_it = (*vn_it).begin(this, vn_it.offset_);
                     aux_it != (*vn_it).end(); ++aux_it) {

                    auto res =
                        ver_tbl_.emplace((*aux_it).get_vsym_index_match_field(),
                                         (*aux_it).load_name(this));
                    if (!res.second) {
                        return k_invalid;
                    }
                }
            }
            return k_okay;
        }

        vstatus_t
        validate_and_collect_entries(const sect_versym_indexes_t vsi) {
            if (!load_okay()) {
                return k_invalid;
            }
            bool last = get_num_need_entries() == 0;
            for (auto vn_it = begin(); vn_it != end(); ++vn_it) {
                vstatus_t res = (*vn_it).validate(this, vsi, vn_it.offset_);
                if (res == k_skip) {
                    last = true;
                    continue;
                }
                if (res != k_okay) {
                    return res;
                }
                last = false;
            }
            if (!last) {
                return k_invalid;
            }
            return validate_and_collect_ver_tbl();
        }

        vstatus_t
        validate_entries() const {
            assert(active());
            return okay_;
        }


        sect_entry_off_it_t<sect_versym_need_t, versym_need_t>
        begin() const {
            return { get_num_need_entries(), get_section_load_begin(), this };
        }

        constexpr sect_entry_off_it_t<sect_versym_need_t, versym_need_t>
        end() const {
            return { 0, 0, nullptr };
        }

        const elf_file_t *              ef_;
        umap<uint16_t, strtab_t::str_t> ver_tbl_;
        vstatus_t                       okay_;
    };


    struct sect_dynamic_info_t : sect_hdr_t {
        // Based on readelf.c:
        // https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=binutils/readelf.c;h=bd112ff53a66969091d8531ced46c56f54145e52;hb=HEAD#l293
        static constexpr size_t k_num_vinfo_slots = 16;

        const elf_file_t *                    ef_;
        small_vec_t<strtab_t::str_t>          dso_deps_;
        size_t                                dsymtab_addr_;
        size_t                                versym_indexes_addr_;
        size_t                                verneed_addr_;
        size_t                                verdef_addr_;
        size_t                                verneed_num_entries_;
        size_t                                verdef_num_entries_;
        size_t                                end_index_;
        std::array<size_t, k_num_vinfo_slots> ver_infos_;
        bool                                  is_pie_;
        vstatus_t                             okay_;
        sect_dynamic_info_t()                            = default;
        sect_dynamic_info_t(const sect_dynamic_info_t &) = delete;
        sect_dynamic_info_t(sect_dynamic_info_t &&)      = delete;


        template<typename T_sect_versym_info_t>
        constexpr bool
        use_for_vinfo(const T_sect_versym_info_t * vs) {
            if (vs == nullptr || !vs->load_okay() || !vs->active()) {
                return false;
            }
            size_t vs_vinfo_index =
                compute_vinfo_index(T_sect_versym_info_t::k_vinfo_index);
            if (is_vinfo_index_bad(vs_vinfo_index)) {
                return false;
            }
            return true;
        }

        constexpr bool
        active() const {
            return ef_ != nullptr;
        }

        void
        init(sect_hdr_t sh, const elf_file_t * ef) {
            assert(!active());
            base_ = sh.base_;
            ef_   = ef;

            dsymtab_addr_        = k_invalid_addr;
            versym_indexes_addr_ = k_invalid_addr;
            verneed_addr_        = k_invalid_addr;
            verdef_addr_         = k_invalid_addr;
            end_index_           = k_invalid_index;
            verneed_num_entries_ = k_invalid_size;
            verdef_num_entries_  = k_invalid_size;
            is_pie_              = false;

            okay_ = validate_and_collect_entries();
            if (end_index_ == k_invalid_index ||
                end_index_ > get_section_num_entries()) {
                okay_ = k_invalid;
            }
        }

        static size_t
        compute_vinfo_index(size_t tag) {
            // This is based on codes from readelf.c:
            // https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=binutils/readelf.c;h=bd112ff53a66969091d8531ced46c56f54145e52;hb=HEAD#l12397
            if (tag < DT_VERSYM || tag > DT_VERNEEDNUM) {
                return k_invalid_index;
            }
            size_t index = DT_VERSIONTAGIDX(tag);
            if (index >= k_num_vinfo_slots) {
                return k_num_vinfo_slots;
            }
            return index;
        }

        static bool
        is_vinfo_index_bad(size_t index) {
            return index == k_invalid_index || index == k_num_vinfo_slots;
        }

        size_t
        get_vinfo_at_index(size_t index) const {
            assert(!is_vinfo_index_bad(index));
            return ver_infos_[index];
        }


        struct dynamic_entry_t : hdr_base_t<Elf64_Dyn> {

            template<typename... Ts_t>
            dynamic_entry_t(Ts_t... ts)
                : hdr_base_t<Elf64_Dyn>(std::forward<Ts_t>(ts)...) {}
            constexpr dynamic_entry_t() = default;

            vstatus_t
            validate_and_collect(sect_dynamic_info_t * sect) const {
                if (!load_okay()) {
                    TLO_TRACE("Bad load\n");
                    return k_invalid;
                }
                if (sect->get_section_entry_load_size() != load_reqsize()) {
                    TLO_TRACE("Bad load size\n");
                    return k_invalid;
                }


                switch (base_->d_tag) {
                    case DT_NULL:  // Marks end of dynamic section
                        return k_skip;
                    case DT_NEEDED:  // Name of needed library
                    {
                        strtab_t::str_t symname =
                            sect->load_name_from_assosiated_strtab(
                                sect->ef_, base_->d_un.d_val);
                        if (strtab_t::is_invalid_str(symname)) {
                            return k_invalid;
                        }
                        sect->dso_deps_.emplace_back(symname);
                    } break;
                    case DT_SYMTAB:  // Address of symbol table
                        if (sect->dsymtab_addr_ != k_invalid_addr) {
                            TLO_TRACE("Bad symtab addr\n");
                            return k_invalid;
                        }
                        sect->dsymtab_addr_ = base_->d_un.d_ptr;
                        break;
                    case DT_VERSYM:
                        if (sect->versym_indexes_addr_ != k_invalid_addr) {
                            TLO_TRACE("Bad versym addr\n");
                            return k_invalid;
                        }
                        sect->versym_indexes_addr_ = base_->d_un.d_ptr;
                        break;
                    case DT_VERDEF:
                        if (sect->verdef_addr_ != k_invalid_addr) {
                            TLO_TRACE("Bad verdef addr\n");
                            return k_invalid;
                        }
                        sect->verdef_addr_ = base_->d_un.d_ptr;
                        break;
                    case DT_VERNEED:
                        if (sect->verneed_addr_ != k_invalid_addr) {
                            TLO_TRACE("Bad verneed addr\n");
                            return k_invalid;
                        }
                        sect->verneed_addr_ = base_->d_un.d_ptr;
                        break;
                    case DT_VERNEEDNUM:
                        if (sect->verneed_num_entries_ != k_invalid_size) {
                            TLO_TRACE("Bad verneed entries\n");
                            return k_invalid;
                        }
                        sect->verneed_num_entries_ = base_->d_un.d_val;
                        break;
                    case DT_VERDEFNUM:
                        if (sect->verdef_num_entries_ != k_invalid_size) {
                            TLO_TRACE("Bad verdef entries\n");
                            return k_invalid;
                        }
                        sect->verdef_num_entries_ = base_->d_un.d_val;
                        break;
                    case DT_FLAGS_1:
                        sect->is_pie_ = ((base_->d_un.d_val & DF_1_PIE) != 0);
                        break;
                    case DT_RELACOUNT:
                    case DT_RELCOUNT:
                    case DT_PLTRELSZ:      // Size in bytes of PLT relocs
                    case DT_PLTGOT:        // Processor defined value
                    case DT_HASH:          // Address of symbol hash table
                    case DT_STRTAB:        // Address of string table
                    case DT_RELA:          // Address of Rela relocs
                    case DT_RELASZ:        // Total size of Rela relocs
                    case DT_RELAENT:       // Size of one Rela reloc
                    case DT_STRSZ:         // Size of string table
                    case DT_SYMENT:        // Size of one symbol table entry
                    case DT_INIT:          // Address of init function
                    case DT_FINI:          // Address of termination function
                    case DT_SONAME:        // Name of shared object
                    case DT_RPATH:         // Library search path (deprecated)
                    case DT_SYMBOLIC:      // Start symbol search here
                    case DT_REL:           // Address of Rel relocs
                    case DT_RELSZ:         // Total size of Rel relocs
                    case DT_RELENT:        // Size of one Rel reloc
                    case DT_PLTREL:        // Type of reloc in PLT
                    case DT_DEBUG:         // For debugging; unspecified
                    case DT_TEXTREL:       // Reloc might modify .text
                    case DT_JMPREL:        // Address of PLT relocs
                    case DT_BIND_NOW:      // Process relocations of object
                    case DT_INIT_ARRAY:    // Array with addresses of init fct
                    case DT_FINI_ARRAY:    // Array with addresses of fini fct
                    case DT_INIT_ARRAYSZ:  // Size in bytes of DT_INIT_ARRAY
                    case DT_FINI_ARRAYSZ:  // Size in bytes of DT_FINI_ARRAY
                    case DT_RUNPATH:       // Library search path
                    case DT_FLAGS:         // Flags for the object being loaded
                    case DT_ENCODING:      // Start of encoded range
                    case DT_SYMTAB_SHNDX:  // Address of SYMTAB_SHNDX section
                    case DT_RELRSZ:   // Total size of RELR relative relocations
                    case DT_RELR:     // Address of RELR relative relocations
                    case DT_RELRENT:  // Size of one RELR relative relocaction
                    case DT_ADDRRNGLO:
                    case DT_GNU_HASH:
                    case DT_TLSDESC_PLT:
                    case DT_TLSDESC_GOT:
                    case DT_GNU_CONFLICT:
                    case DT_GNU_LIBLIST:
                    case DT_CONFIG:
                    case DT_DEPAUDIT:
                    case DT_AUDIT:
                    case DT_PLTPAD:
                    case DT_MOVETAB:
                    case DT_SYMINFO:
                        break;
                    default:
                        // In processor specific range.
                        if (base_->d_tag >= DT_LOPROC &&
                            base_->d_tag <= DT_HIPROC) {
                            break;
                        }
                        // In OS specific range.
                        if (base_->d_tag >= DT_LOOS &&
                            base_->d_tag <= DT_HIOS) {
                            break;
                        }
                        TLO_perr("Warning: dynamic section tag: %ld\n",
                                 base_->d_tag);
                        break;
                }


                size_t vinfo_index =
                    compute_vinfo_index(static_cast<size_t>(base_->d_tag));
                if (vinfo_index != k_invalid_index) {
                    if (vinfo_index >= k_num_vinfo_slots) {
                        TLO_TRACE("bad slot\n");
                        return k_invalid;
                    }
                    sect->ver_infos_[vinfo_index] = base_->d_un.d_val;
                }


                return k_okay;
            }

            bool
            is_end() const {
                assert(load_okay());
                return base_->d_tag == DT_NULL;
            }
        };

        dynamic_entry_t
        load_entry(size_t offset) const {
            assert(load_okay());
            assert(is_dynamic_info());
            if (!section_is_loadable(ef_, offset)) {
                return {};
            }
            return ef_->loadt<dynamic_entry_t>(offset);
        }

        dynamic_entry_t
        load_entry_index(size_t index) const {
            assert(load_okay());
            assert(is_dynamic_info());
            if (!section_is_index_loadable(ef_, index)) {
                return {};
            }
            size_t ent_size   = get_section_entry_load_size();
            size_t ents_begin = get_section_load_begin();
            return load_entry(ents_begin + index * ent_size);
        }

        using entry_t = dynamic_entry_t;

        constexpr sect_entry_it_t<sect_dynamic_info_t, true>
        begin() const {
            return { 0, *this };
        }

        constexpr sect_entry_it_t<sect_dynamic_info_t, true>
        end() const {
            return { end_index_, *this };
        }

        vstatus_t
        validate_and_collect_entries() {
            size_t cnt = 0;
            for (dynamic_entry_t de : *this) {
                vstatus_t res = de.validate_and_collect(this);
                if (res == k_skip) {
                    break;
                }
                if (res != k_okay) {
                    return res;
                }
                ++cnt;
            }
            end_index_ = cnt;
            return k_okay;
        }

        vstatus_t
        validate_entries() const {
            assert(active());
            return okay_;
        }
    };


    template<typename T_hdr_t>
    struct hdr_it_t {
        const elf_file_t * ef_;
        struct hdr_iterator_impl_t {
            size_t             index_;
            const elf_file_t * ef_;


            T_hdr_t
            operator*() const {
                return ef_->load_hdr_index<T_hdr_t>(index_);
            }

            hdr_iterator_impl_t &
            operator++() {
                ++index_;
                return *this;
            }


            bool
            operator!=(hdr_iterator_impl_t const & other) {
                return index_ != other.index_;
            }
        };

        hdr_iterator_impl_t
        begin() const {
            return hdr_iterator_impl_t{ 0, ef_ };
        }

        hdr_iterator_impl_t
        end() const {
            return hdr_iterator_impl_t{ ef_->get_num_hdrs<T_hdr_t>(), {} };
        }
    };

    hdr_it_t<prog_hdr_t>
    prog_hdr_it() const {
        return hdr_it_t<prog_hdr_t>{ this };
    }

    hdr_it_t<sect_hdr_t>
    sect_hdr_it() const {
        return hdr_it_t<sect_hdr_t>{ this };
    }

    bool
    is_sysv_abi() const {
        return load_elf_hdr().is_sysv_abi();
    }

    bool
    is_gnu_abi() const {
        return load_elf_hdr().is_gnu_abi();
    }

    bool
    is_x86_64() const {
        return load_elf_hdr().is_x86_64();
    }

    bool
    is_executable() const {
        return load_elf_hdr().is_executable();
    }

    bool
    is_shared_obj_or_pie() const {
        return load_elf_hdr().is_shared_obj_or_pie();
    }

    size_t
    get_sect_strtab_hdr_index() const {
        auto first = load_elf_hdr().get_1st_sect_strtab_hdr_index();
        if (first) {
            return *first;
        }
        return load_first_sect_hdr().get_2nd_sect_strtab_hdr_index();
    }


    strtab_t
    load_strtab(size_t index) const {
        sect_hdr_t strtab_sect_hdr = load_sect_hdr_index(index);
        if (!strtab_sect_hdr.load_okay()) {
            return {};
        }
        if (!strtab_sect_hdr.is_strtab()) {
            return {};
        }

        size_t sect_strtab_begin = strtab_sect_hdr.get_section_load_begin();
        size_t sect_strtab_size  = strtab_sect_hdr.get_section_load_size();
        if (sect_strtab_begin == k_invalid_offset ||
            sect_strtab_size == k_invalid_size) {
            return {};
        }
        if (sect_strtab_begin == 0 || sect_strtab_size == 0) {
            return {};
        }
        if (!(loadable(sect_strtab_begin, sect_strtab_size))) {
            return {};
        }
        char const * names = reinterpret_cast<char const *>(
            mapping_.region(sect_strtab_begin, sect_strtab_size));
        return { names, sect_strtab_size };
    }

    strtab_t
    load_sect_strtab() const {
        return load_strtab(get_sect_strtab_hdr_index());
    }

    strtab_t::str_t
    load_sect_name(size_t name_offset) const {
        return load_sect_strtab().load_name(name_offset);
    }

    // Helpers for accessing the loadable headers.

    template<typename T_t>
    T_t
    loadt(size_t file_offset) const {
        assert(active());
        if (!loadable(file_offset, T_t::load_reqsize())) {
            return T_t{};
        }
        return T_t(mapping_.region(file_offset, T_t::load_reqsize()));
    }

    template<typename T_t>
    T_t
    loadv(size_t file_offset, size_t size) const {
        assert(active());
        if (!loadable(file_offset, size)) {
            return T_t{};
        }
        return T_t(mapping_.region(file_offset, size), size);
    }

    template<typename T_hdr_t>
    T_hdr_t
    load_hdr(size_t file_offset) const {
        return loadt<T_hdr_t>(file_offset);
    }

    elf_hdr_t
    load_elf_hdr() const {
        elf_hdr_t hdr = load_hdr<elf_hdr_t>(0);
        return hdr;
    }

    prog_hdr_t
    load_prog_hdr(size_t file_offset) const {
        prog_hdr_t hdr = load_hdr<prog_hdr_t>(file_offset);
        return hdr;
    }

    sect_hdr_t
    load_sect_hdr(size_t file_offset) const {
        sect_hdr_t hdr = load_hdr<sect_hdr_t>(file_offset);
        return hdr;
    }


    template<typename T_hdr_t>
    T_hdr_t
    load_hdr_index(size_t index) const {
        if (index == k_invalid_index || index >= get_num_hdrs<T_hdr_t>()) {
            return T_hdr_t{};
        }
        return load_hdr<T_hdr_t>(get_hdrs_load_begin<T_hdr_t>() +
                                 get_hdr_load_size<T_hdr_t>() * index);
    }
    prog_hdr_t
    load_prog_hdr_index(size_t index) const {
        return load_hdr_index<prog_hdr_t>(index);
    }

    sect_hdr_t
    load_sect_hdr_index(size_t index) const {
        if (index == k_invalid_index || index >= get_num_sect_hdrs()) {
            return {};
        }
        return load_hdr_index<sect_hdr_t>(index);
    }


    sect_hdr_t
    load_first_sect_hdr() const {
        return load_sect_hdr(get_sect_hdrs_load_begin());
    }


    template<typename T_hdr_t>
    size_t
    get_hdrs_load_begin() const {
        if constexpr (std::is_same<T_hdr_t, elf_hdr_t>::value) {
            return 0;
        }
        else if constexpr (std::is_same<T_hdr_t, prog_hdr_t>::value) {
            return load_elf_hdr().get_prog_hdrs_load_begin();
        }
        else if constexpr (std::is_same<T_hdr_t, sect_hdr_t>::value) {
            return load_elf_hdr().get_sect_hdrs_load_begin();
        }
        else {
            static_assert(!std::is_same<T_hdr_t, T_hdr_t>::value,
                          "Unknown header type");
        }
    }

    size_t
    get_elf_hdrs_load_begin() const {
        return get_hdrs_load_begin<elf_hdr_t>();
    }
    size_t
    get_prog_hdrs_load_begin() const {
        return get_hdrs_load_begin<prog_hdr_t>();
    }
    size_t
    get_sect_hdrs_load_begin() const {
        return get_hdrs_load_begin<sect_hdr_t>();
    }

    template<typename T_hdr_t>
    size_t
    get_num_hdrs() const {
        if constexpr (std::is_same<T_hdr_t, elf_hdr_t>::value) {
            return 1;
        }
        else if constexpr (std::is_same<T_hdr_t, prog_hdr_t>::value) {
            auto first = load_elf_hdr().get_1st_num_prog_hdrs();
            if (first) {
                return *first;
            }
            return load_first_sect_hdr().get_2nd_num_prog_hdrs();
        }
        else if constexpr (std::is_same<T_hdr_t, sect_hdr_t>::value) {
            auto first = load_elf_hdr().get_1st_num_sect_hdrs();
            if (first) {
                return *first;
            }
            return load_first_sect_hdr().get_2nd_num_sect_hdrs();
        }
        else {
            static_assert(!std::is_same<T_hdr_t, T_hdr_t>::value,
                          "Unknown header type");
        }
    }


    size_t
    get_num_elf_hdrs() const {
        return get_num_hdrs<elf_hdr_t>();
    }
    size_t
    get_num_prog_hdrs() const {
        return get_num_hdrs<prog_hdr_t>();
    }
    size_t
    get_num_sect_hdrs() const {
        return get_num_hdrs<sect_hdr_t>();
    }


    template<typename T_hdr_t>
    size_t
    get_hdr_load_size() const {
        if constexpr (std::is_same<T_hdr_t, elf_hdr_t>::value) {
            return load_elf_hdr().get_elf_hdr_load_size();
        }
        else if constexpr (std::is_same<T_hdr_t, prog_hdr_t>::value) {
            return load_elf_hdr().get_prog_hdr_load_size();
        }
        else if constexpr (std::is_same<T_hdr_t, sect_hdr_t>::value) {
            return load_elf_hdr().get_sect_hdr_load_size();
        }
        else {
            static_assert(!std::is_same<T_hdr_t, T_hdr_t>::value,
                          "Unknown header type");
        }
    }

    size_t
    get_elf_hdr_load_size() const {
        return get_hdr_load_size<elf_hdr_t>();
    }

    size_t
    get_prog_hdr_load_size() const {
        return get_hdr_load_size<prog_hdr_t>();
    }

    size_t
    get_sect_hdr_load_size() const {
        return get_hdr_load_size<sect_hdr_t>();
    }

    size_t
    get_elf_hdrs_load_end() const {
        return get_elf_hdrs_load_begin() +
               get_elf_hdr_load_size() * get_num_elf_hdrs();
    }
    size_t
    get_prog_hdrs_load_end() const {
        return get_prog_hdrs_load_begin() +
               get_prog_hdr_load_size() * get_num_prog_hdrs();
    }
    size_t
    get_sect_hdrs_load_end() const {
        return get_sect_hdrs_load_begin() +
               get_sect_hdr_load_size() * get_num_sect_hdrs();
    }

    // Validations.
    vstatus_t
    validate_elf_hdr() const {
        return load_elf_hdr().validate(this);
    }
    vstatus_t
    validate_prog_hdrs() const {
        size_t last_ld_vaddr = 0;
        bool   first         = true;
        for (prog_hdr_t prog_hdr : prog_hdr_it()) {
            vstatus_t res = prog_hdr.validate(this, first, &last_ld_vaddr);
            if (res != k_okay) {
                return res;
            }
            first = false;
        }
        return k_okay;
    }

    vstatus_t
    validate_sect_hdrs() const {
        bool   first      = true;
        size_t dsym_size  = k_invalid_size;
        size_t vsym_size  = k_invalid_size;
        size_t vneed_off  = k_invalid_offset;
        size_t vdef_off   = k_invalid_offset;
        size_t dsym_off   = k_invalid_offset;
        size_t vsym_off   = k_invalid_offset;
        size_t vneed_addr = k_invalid_addr;
        size_t vdef_addr  = k_invalid_addr;
        size_t dsym_addr  = k_invalid_addr;
        size_t vsym_addr  = k_invalid_addr;

        sect_dynamic_info_t   dyn_info{};
        sect_versym_indexes_t vsi{};
        sect_versym_defs_t    vsd{};
        sect_versym_need_t    vsn{};
        sect_notes_t          build_id_note{};
        sect_symtab_t         dsymtab{};
        sect_symtab_t         ssymtab{};
        for (sect_hdr_t sect_hdr : sect_hdr_it()) {
            vstatus_t res = sect_hdr.validate(this, first);
            if (res != k_okay) {
                TLO_TRACE("Bad internals");
                return res;
            }
            first                     = false;
            strtab_t::str_t sect_name = sect_hdr.section_name(this);
            if (strtab_t::is_invalid_str(sect_name)) {
                TLO_TRACE("Bad name");
                return k_invalid;
            }
            if ((sect_name == ".note.gnu.build-id") && !sect_hdr.is_note()) {
                TLO_TRACE("Bad build_id: %s (%x/%lx)\n", sect_name.data(),
                          sect_hdr.base_->sh_type, sect_hdr.base_->sh_flags);

                return k_invalid;
            }
            if (sect_hdr.is_note()) {
                sect_notes_t notes{ sect_hdr, this };
                if (notes.validate() != k_okay) {
                    TLO_TRACE("Invalid notes");
                    return k_invalid;
                }
                if ((sect_name == ".note.gnu.build-id") !=
                    notes.is_build_id()) {
                    TLO_TRACE("Invalid build id");
                    return k_invalid;
                }
                if (notes.is_build_id()) {
                    if (build_id_note.active()) {
                        TLO_TRACE("Duplicate build id");
                        return k_invalid;
                    }
                    build_id_note = notes;
                }
            }

            // fprintf(stderr, "At: %s\n", sect_name.data());
            if ((sect_name == ".dynamic") != sect_hdr.is_dynamic_info()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad dynamic section: %s (%x/%lx)\n",
                              sect_name.data(), sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".symtab" || sect_name == ".dynsym") !=
                sect_hdr.is_symtab()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad symtab section: %s( (%d/%x/%lx)\n",
                              sect_name.data(), sect_hdr.is_symtab(),
                              sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".dynsym") != sect_hdr.is_dsymtab()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad dsymtab section: %s (%x/%lx)\n",
                              sect_name.data(), sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".symtab") != sect_hdr.is_ssymtab()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad dsymtab section: %s (%x/%lx)\n",
                              sect_name.data(), sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".strtab" || sect_name == ".dynstr" ||
                 sect_name == ".shstrtab") != sect_hdr.is_strtab()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad strtab section: %s (%x/%lx)\n",
                              sect_name.data(), sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".gnu.version") != sect_hdr.is_versym_indexes()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad vsyms: %s (%x/%lx)\n", sect_name.data(),
                              sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".gnu.version_r") != sect_hdr.is_versym_need()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad vsyms_r: %s (%x/%lx)\n", sect_name.data(),
                              sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }
            if ((sect_name == ".gnu.version_d") != sect_hdr.is_versym_defs()) {
                if (is_debug() && !sect_hdr.is_nobits()) {
                    TLO_TRACE("Bad vsyms_d: %s (%x/%lx)\n", sect_name.data(),
                              sect_hdr.base_->sh_type,
                              sect_hdr.base_->sh_flags);
                    return k_invalid;
                }
            }

            if (sect_hdr.is_dynamic_info()) {
                if (dyn_info.active()) {
                    TLO_TRACE("Bad DynOrder");
                    return k_invalid;
                }
                dyn_info.init(sect_hdr, this);
                if (!dyn_info.active()) {
                    TLO_TRACE("Bad DynSetup");
                    return k_invalid;
                }
                res = dyn_info.validate_entries();
                if (res != k_okay) {
                    TLO_TRACE("Bad DynValid");
                    return res;
                }
            }

            if (sect_hdr.is_dsymtab()) {
                if (dsym_size != k_invalid_size) {
                    TLO_TRACE("Bad DsymOrder1");
                    return k_invalid;
                }
                if (dsym_off != k_invalid_offset) {
                    TLO_TRACE("Bad DsymOrder2");
                    return k_invalid;
                }
                if (dsym_addr != k_invalid_addr) {
                    TLO_TRACE("Bad DsymOrder3");
                    return k_invalid;
                }
                if (dsymtab.active()) {
                    TLO_TRACE("Bad DsymOrder4");
                    return k_invalid;
                }
                dsym_off  = sect_hdr.get_section_load_begin();
                dsym_addr = sect_hdr.get_section_image_addr();
                dsym_size = sect_hdr.get_section_num_entries();
                dsymtab.init(sect_hdr, this);
                res = dsymtab.validate_entries();
                if (res != k_okay) {
                    TLO_TRACE("Bad DSymValid");
                    return res;
                }
            }
            else if (sect_hdr.is_ssymtab()) {
                ssymtab.init(sect_hdr, this);
                res = ssymtab.validate_entries();
                if (res != k_okay) {
                    TLO_TRACE("Bad SSymValid");
                    return res;
                }
            }


            if (sect_hdr.is_versym_need()) {
                if (vneed_off != k_invalid_offset) {
                    TLO_TRACE("Bad VNOrder1");
                    return k_invalid;
                }
                if (vneed_addr != k_invalid_addr) {
                    TLO_TRACE("Bad VNOrder2");
                    return k_invalid;
                }
                if (vsn.active()) {
                    TLO_TRACE("Bad VNOrder3");
                    return k_invalid;
                }
                vneed_off  = sect_hdr.get_section_load_begin();
                vneed_addr = sect_hdr.get_section_image_addr();
                vsn.init(sect_hdr, this, vsi);

                if (vsi.active()) {
                    res = vsn.validate_entries();
                    if (res != k_okay) {
                        TLO_TRACE("Bad VNValid");
                        return res;
                    }
                }
            }

            if (sect_hdr.is_versym_indexes()) {
                if (vsym_size != k_invalid_size) {
                    TLO_TRACE("Bad VIOrder1");
                    return k_invalid;
                }
                if (vsi.active()) {
                    TLO_TRACE("Bad VIOrder2");
                    return k_invalid;
                }
                vsym_off  = sect_hdr.get_section_load_begin();
                vsym_addr = sect_hdr.get_section_image_addr();
                vsym_size = sect_hdr.get_section_num_entries();

                vsi.init(sect_hdr, this);
                res = vsi.validate_entries();
                if (res != k_okay) {
                    TLO_TRACE("Bad VIValid");
                    return res;
                }

                if (vsd.active()) {
                    res = vsd.validate_entries();
                    if (res != k_okay) {
                        TLO_TRACE("Bad VDValid2");
                        return res;
                    }
                }
                if (vsn.active()) {
                    res = vsn.validate_entries();
                    if (res != k_okay) {
                        TLO_TRACE("Bad VNValid2");
                        return res;
                    }
                }
            }

            if (sect_hdr.is_versym_defs()) {
                if (vdef_off != k_invalid_offset) {
                    TLO_TRACE("Bad VDOrder0");
                    return k_invalid;
                }
                vdef_off  = sect_hdr.get_section_load_begin();
                vdef_addr = sect_hdr.get_section_image_addr();

                if (vsd.active()) {
                    TLO_TRACE("Bad VDOrder1");
                    return k_invalid;
                }
                vsd.init(sect_hdr, this, vsi);

                if (vsi.active()) {
                    res = vsd.validate_entries();
                    if (res != k_okay) {
                        TLO_TRACE("Bad VDValid");
                        return res;
                    }
                }
            }
        }

        if (!dyn_info.active()) {
            // Its a static library
            if (vsym_size != k_invalid_size || dsym_size != k_invalid_size ||
                vsym_off != k_invalid_offset || dsym_off != k_invalid_offset ||
                vdef_off != k_invalid_offset || vneed_off != k_invalid_offset ||
                vsym_addr != k_invalid_addr || dsym_addr != k_invalid_addr ||
                vdef_addr != k_invalid_addr || vneed_addr != k_invalid_addr) {
                TLO_TRACE("Bad Conf0");
                return k_invalid;
            }
            if (vsi.active() || dsymtab.active() || vsd.active() ||
                vsn.active()) {
                TLO_TRACE("Bad Conf1");
                return k_invalid;
            }
            return k_okay;
        }


        if (vsym_size != dsym_size && vsym_size != k_invalid_size) {
            TLO_TRACE("Bad Conf2");
            return k_invalid;
        }
        if (dsym_addr != dyn_info.dsymtab_addr_) {
            TLO_TRACE("Bad Conf3: %lx vs %lx", dsym_addr,
                      dyn_info.dsymtab_addr_);
            return k_invalid;
        }
        if (vsym_addr != dyn_info.versym_indexes_addr_) {
            TLO_TRACE("Bad Conf4");
            return k_invalid;
        }
        if (vdef_addr != dyn_info.verdef_addr_) {
            TLO_TRACE("Bad Conf5");
            return k_invalid;
        }
        if (vneed_addr != dyn_info.verneed_addr_) {
            TLO_TRACE("Bad Conf6");
            return k_invalid;
        }

        if (vsn.active()) {
            if (vsn.get_num_need_entries() != dyn_info.verneed_num_entries_) {
                TLO_TRACE("Bad Conf7");
                return k_invalid;
            }
        }
        if (vsd.active()) {
            if (vsd.get_num_def_entries() != dyn_info.verdef_num_entries_) {
                TLO_TRACE("Bad Conf8");
                return k_invalid;
            }
        }

#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        vec_t<std::tuple<sym::func_t, bool, uint8_t>> all_funcs{};

        static constexpr size_t k_local_funcs  = 0;
        static constexpr size_t k_global_funcs = 1;
        static constexpr size_t k_weak_funcs   = 2;
        static constexpr size_t k_other_funcs  = 3;

        static_assert(k_local_funcs == STB_LOCAL);
        static_assert(k_global_funcs == STB_GLOBAL);
        static_assert(k_weak_funcs == STB_WEAK);


        struct func_eq_t {
            constexpr bool
            operator()(const sym::func_t & lhs, const sym::func_t & rhs) const {
                return lhs.name_eq(rhs);
            }
        };

        struct func_hash_t {
            constexpr uint64_t
            operator()(const sym::func_t & func) const {
                return func.hash();
            }
        };

        if (ssymtab.active()) {
            std::array<basic_uset<sym::func_t, func_hash_t, func_eq_t>, 4>
                st_func_names;
            for (sect_symtab_t::symtab_entry_t symtab_ent : ssymtab) {
                if (!symtab_ent.has_func_info(this)) {
                    continue;
                }
                sym::func_t func = symtab_ent.extract_func(ssymtab, nullptr);

                size_t vis = std::min(symtab_ent.get_st_bind(), k_other_funcs);
                auto   res_st_vname = st_func_names[vis].emplace(func);
                if (!func.loc_.complete() && !func.loc_.empty()) {
                    TLO_TRACE("Bad addr info!\n");
                    return k_invalid;
                }
                if (!res_st_vname.second && vis != k_local_funcs) {
                    TLO_TRACE("Dup st name");
                    return k_invalid;
                }
                all_funcs.emplace_back(func, true, vis);
            }
        }


        if (dsymtab.active() && !vsi.active()) {
            std::array<basic_uset<sym::func_t, func_hash_t, func_eq_t>, 4>
                st_func_names;
            for (sect_symtab_t::symtab_entry_t symtab_ent : dsymtab) {
                if (!symtab_ent.has_func_info(this)) {
                    continue;
                }
                sym::func_t func = symtab_ent.extract_func(dsymtab, nullptr);
                size_t vis = std::min(symtab_ent.get_st_bind(), k_other_funcs);
                auto   res_st_vname = st_func_names[vis].emplace(func);
                if (!func.loc_.complete() && !func.loc_.empty()) {
                    TLO_TRACE("Bad addr info!\n");
                    return k_invalid;
                }
                if (!res_st_vname.second && vis != k_local_funcs) {
                    TLO_TRACE("Dup st name");
                    return k_invalid;
                }
                all_funcs.emplace_back(func, true, vis);
            }
        }

#endif

        if (dsymtab.active() && vsi.active()) {
#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
            std::array<basic_uset<sym::func_t, func_hash_t, func_eq_t>, 4>
                dyn_func_names;
#endif

            if (!dyn_info.active()) {
                TLO_TRACE("No dyn info\n");
                return k_invalid;
            }
            if (dsymtab.get_section_num_entries() !=
                vsi.get_section_num_entries()) {
                TLO_TRACE("Invalid num entries\n");
                return k_invalid;
            }

            bool use_vsn = dyn_info.use_for_vinfo(&vsn);
            bool use_vsd = dyn_info.use_for_vinfo(&vsd);
            if (!use_vsn && !use_vsd) {
                TLO_TRACE("No vinfo\n");
                return k_invalid;
            }

            size_t nentries = dsymtab.get_section_num_entries();
            auto   dsym_it  = dsymtab.begin();
            auto   vsym_it  = vsi.begin();

            for (; nentries; --nentries, ++dsym_it, ++vsym_it) {
                if (!(*dsym_it).has_func_info(this)) {
                    continue;
                }

                std::pair<strtab_t::str_t, bool> vsn_info = {
                    strtab_t::k_invalid_str, false
                };
                std::pair<strtab_t::str_t, bool> vsd_info = {
                    strtab_t::k_invalid_str, false
                };
                if (use_vsn) {
                    vsn_info =
                        vsn.find_ver_ident((*vsym_it).get_index(), (*dsym_it));
                }
                if (use_vsd) {
                    vsd_info =
                        vsd.find_ver_ident((*vsym_it).get_index(), (*dsym_it));
                }

                if (!vsn_info.first.empty() && !vsd_info.first.empty()) {
                    TLO_TRACE("Invalid version info0");
                    return k_invalid;
                }


#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
                auto vinfo       = vsd_info.first.empty() ? vsn_info : vsd_info;
                sym::func_t func = (*dsym_it).extract_func(
                    dsymtab, nullptr, vinfo.first, vinfo.second);
                size_t vis = std::min((*dsym_it).get_st_bind(), k_other_funcs);
                auto   res_dyn_vname = dyn_func_names[vis].emplace(func);
                if (!func.loc_.complete() && !func.loc_.empty()) {
                    TLO_TRACE("Bad addr info!\n");
                    return k_invalid;
                }
                if (!res_dyn_vname.second) {
                    TLO_TRACE("Dup dyn name!");
                    return k_invalid;
                }
                all_funcs.emplace_back(func, false, vis);
#endif
            }
        }
#if (defined TLO_DEBUG_ENABLED) || (defined TLO_DEBUG_ENABLED_GLBL)
        std::sort(all_funcs.begin(), all_funcs.end(),
                  [](const auto & lhs, const auto & rhs) {
                      return std::get<0>(lhs).lt(std::get<0>(rhs));
                  });
        size_t i, e;
        for (i = 0, e = all_funcs.size(); (i + 1) < e; ++i) {
            const auto & item0 = all_funcs[i];
            const auto & item1 = all_funcs[i + 1];

            const auto & f0 = std::get<0>(item0);
            const auto & f1 = std::get<0>(item1);

            if (f0.loc_.overlaps_with(f1.loc_) && !f0.loc_.eq(f1.loc_)) {
                TLO_perr("Warning: Overlapping funcs! %d/%d, %d/%d\n",
                         std::get<1>(item0), std::get<1>(item1),
                         std::get<2>(item0), std::get<2>(item1));
                TLO_DEBUG_ONLY(f0.dump(0, stderr));
                TLO_DEBUG_ONLY(f1.dump(0, stderr));
            }
        }
#endif


        return k_okay;
    }

    vstatus_t
    validate() const {

        vstatus_t res = validate_elf_hdr();
        if (res != k_okay) {

            TLO_TRACE("\tBad Sect");
            return res;
        }
        res = validate_prog_hdrs();
        if (res != k_okay) {

            TLO_TRACE("\tBad PHdrs");
            return res;
        }
        res = validate_sect_hdrs();
        if (res != k_okay) {
            TLO_TRACE("\tBad SHdrs");

            return res;
        }

        return k_okay;
    }

    bool
    is_debug() const {
        return is_debug_;
    }

    // TODO: We might be better of use pread. If the ELF file is really big
    // and we touch a lot of pages the page-fault overhead will be large.
    file_ops::mapped_file_t mapping_;
    bool                    is_debug_;
};
}  // namespace tlo


#endif
