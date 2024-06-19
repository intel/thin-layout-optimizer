import sys
import os
import pathlib
import argparse

from order_file import Order_File

parser = argparse.ArgumentParser(
    description="Finalize order for target linker+executable/dso")
parser.add_argument("-i",
                    "--input",
                    action="store",
                    default=None,
                    help="Input file or directory")
parser.add_argument("-a",
                    "--alignment",
                    action="store",
                    default="5",
                    help="Alignment for hot functions (log2)")
parser.add_argument("--align-hot-n",
                    action="store",
                    default=".25",
                    help="Hottest N% perfunctions to align (0-100)")
parser.add_argument(
    "--align-per-dso",
    action="store_true",
    default=False,
    help=
    "Normally align based on global hotness. Change to align based on hotness within DSO"
)
parser.add_argument(
    "--align-till",
    action="store",
    default="0.0",
    help=
    "Align functions till it we have aligned incoming edges accounting for N% of the total weight. Both --align-till and --align-hot-n can be specified at once"
)

parser.add_argument("--aliases",
                    action="store",
                    default=None,
                    help="File with commandline aliases")
parser.add_argument("-m",
                    "--map",
                    action="store",
                    default=None,
                    help="Create map file (if file name exists will append)")
parser.add_argument("-o", action="store", default=None, help="Output file")
parser.add_argument("-p",
                    "--prefix",
                    action="store",
                    default=None,
                    help="Prefix for all linker scripts in map file")
parser.add_argument("-t",
                    "--target",
                    action="store",
                    default="ld",
                    help="Target linker")
parser.add_argument("--machine",
                    action="store",
                    default="x86_64",
                    help="Machines to target (multiple specified as CSV)")
parser.add_argument("--no-cold",
                    action="store_true",
                    default=False,
                    help="Don't search in `.text.cold*` or `.text.unlikely*`")
parser.add_argument(
    "--wildcards",
    action="store_true",
    default=False,
    help=
    "Add wildcards to linker script (that can dramatically increase compile time)"
)

LD_BASE_SCRIPT = """
/* Script for -pie -z combreloc -z separate-code -z relro -z now */
/* Copyright (C) 2014-2022 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64",
              "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SEARCH_DIR("=/usr/local/lib/x86_64-linux-gnu"); SEARCH_DIR("=/lib/x86_64-linux-gnu"); SEARCH_DIR("=/usr/lib/x86_64-linux-gnu"); SEARCH_DIR("=/usr/lib/x86_64-linux-gnu64"); SEARCH_DIR("=/usr/local/lib64"); SEARCH_DIR("=/lib64"); SEARCH_DIR("=/usr/lib64"); SEARCH_DIR("=/usr/local/lib"); SEARCH_DIR("=/lib"); SEARCH_DIR("=/usr/lib"); SEARCH_DIR("=/usr/x86_64-linux-gnu/lib64"); SEARCH_DIR("=/usr/x86_64-linux-gnu/lib");
SECTIONS
{
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0)); . = SEGMENT_START("text-segment", 0) + SIZEOF_HEADERS;
  .interp         : { *(.interp) }
  .note.gnu.build-id  : { *(.note.gnu.build-id) }
  .hash           : { *(.hash) }
  .gnu.hash       : { *(.gnu.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rela.dyn       :
    {
      *(.rela.init)
      *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
      *(.rela.fini)
      *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
      *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
      *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
      *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
      *(.rela.ctors)
      *(.rela.dtors)
      *(.rela.got)
      *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
      *(.rela.ldata .rela.ldata.* .rela.gnu.linkonce.l.*)
      *(.rela.lbss .rela.lbss.* .rela.gnu.linkonce.lb.*)
      *(.rela.lrodata .rela.lrodata.* .rela.gnu.linkonce.lr.*)
      *(.rela.ifunc)
    }
  .rela.plt       :
    {
      *(.rela.plt)
      *(.rela.iplt)
    }
  .relr.dyn : { *(.relr.dyn) }
  . = ALIGN(CONSTANT (MAXPAGESIZE));
  .init           :
  {
    KEEP (*(SORT_NONE(.init)))
  }
  .plt            : { *(.plt) *(.iplt) }
.plt.got        : { *(.plt.got) }
.plt.sec        : { *(.plt.sec) }
  .text           :
  {
   ::CUSTOM_ORDER::
    *(.text.hot .text.hot.*)
    *(.text.startup .text.startup.*)
    *(SORT(.text.sorted.*))
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    /* .gnu.warning sections are handled specially by elf.em.  */
    *(.gnu.warning)
  }
  .fini           :
  {
    KEEP (*(SORT_NONE(.fini)))
  }
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  . = ALIGN(CONSTANT (MAXPAGESIZE));
  /* Adjust the address for the rodata segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = SEGMENT_START("rodata-segment", ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)));
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
  .rodata1        : { *(.rodata1) }
  .eh_frame_hdr   : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) }
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) *(.eh_frame.*) }
  .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }
  .gnu_extab   : ONLY_IF_RO { *(.gnu_extab*) }
  /* These sections are generated by the Sun/Oracle C++ compiler.  */
  .exception_ranges   : ONLY_IF_RO { *(.exception_ranges*) }
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  /* Exception handling  */
  .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) *(.eh_frame.*) }
  .gnu_extab      : ONLY_IF_RW { *(.gnu_extab) }
  .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }
  .exception_ranges   : ONLY_IF_RW { *(.exception_ranges*) }
  /* Thread Local Storage sections  */
  .tdata	  :
   {
     PROVIDE_HIDDEN (__tdata_start = .);
     *(.tdata .tdata.* .gnu.linkonce.td.*)
   }
  .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
  .preinit_array    :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array    :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array    :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }
  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }
  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  .jcr            : { KEEP (*(.jcr)) }
  .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
  .dynamic        : { *(.dynamic) }
  .got            : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) }
  . = DATA_SEGMENT_RELRO_END (0, .);
  .data           :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)
  }
  .data1          : { *(.data1) }
  _edata = .; PROVIDE (edata = .);
  . = .;
  __bss_start = .;
  .bss            :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.
      FIXME: Why do we need it? When there is no .bss section, we do not
      pad the .data section.  */
   . = ALIGN(. != 0 ? 64 / 8 : 1);
  }
  .lbss   :
  {
    *(.dynlbss)
    *(.lbss .lbss.* .gnu.linkonce.lb.*)
    *(LARGE_COMMON)
  }
  . = ALIGN(64 / 8);
  . = SEGMENT_START("ldata-segment", .);
  .lrodata   ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)) :
  {
    *(.lrodata .lrodata.* .gnu.linkonce.lr.*)
  }
  .ldata   ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)) :
  {
    *(.ldata .ldata.* .gnu.linkonce.l.*)
    . = ALIGN(. != 0 ? 64 / 8 : 1);
  }
  . = ALIGN(64 / 8);
  _end = .; PROVIDE (end = .);
  . = DATA_SEGMENT_END (.);
  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  .gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1.  */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions.  */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2.  */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2.  */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions.  */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3.  */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF 5.  */
  .debug_addr     0 : { *(.debug_addr) }
  .debug_line_str 0 : { *(.debug_line_str) }
  .debug_loclists 0 : { *(.debug_loclists) }
  .debug_macro    0 : { *(.debug_macro) }
  .debug_names    0 : { *(.debug_names) }
  .debug_rnglists 0 : { *(.debug_rnglists) }
  .debug_str_offsets 0 : { *(.debug_str_offsets) }
  .debug_sup      0 : { *(.debug_sup) }
  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }
}
"""


def collect_files_impl(input):
    if os.path.isdir(input):
        return [
            os.path.join(input, x.name)
            for x in list(set(pathlib.Path(input).rglob("*.txt")))
        ]
    elif os.path.isfile(input):
        return [input]
    else:
        return []


def collect_files(input):
    files = collect_files_impl(input)
    out = []
    for f in files:
        out.append(Order_File(f))
    return sorted(out, key=lambda x: len(x.order), reverse=True)


def write_file(outfile, order):
    try:
        f = sys.stdout
        if outfile is not None:
            f = open(outfile, "w+")
        f.write(order + "\n")
        if outfile is not None:
            f.close()
        return True
    except Exception as e:
        print("Error writing: {} -> {}".format(outfile, str(e)))
        return False


def prepare_for_target(target, lines):
    if lines is None:
        return None

    if target in ("ld.orig", "ld.patch"):
        for i in range(0, len(lines)):
            ln = lines[i].split()
            ln[0] = "*({})".format(ln[0])
            lines[i] = " ".join(ln)
        lines = "\n".join(lines)
        if target == "ld.patch":
            return lines
        lines = ".text : {\n" + lines + "\n}\n"
        return lines
    elif target == "gold":
        return "\n".join(lines)
    return None


def process_dir(target, input, output, mapfile, no_cold, wildcards, prefix,
                machines, align_hot_n, align_till, alignment, align_per_dso,
                aliases):
    files = collect_files(input)
    if len(files) == 0:
        print("Unable to find orders in: {}".format(input))
        return False

    ord_infos = {}
    num_aligned = 0
    total_fns = 0
    for f in files:
        total_fns += len(f.order)
    if align_per_dso:
        for f in files:
            num_aligned += f.add_alignment_to_hottest_per_dso(
                align_hot_n, alignment)
            num_aligned += f.add_alignment_till(align_till, alignment)
    else:
        all_win = []
        all_fns = []
        for i in range(0, len(files)):
            for fn in files[i].order:
                all_fns.append((i, fn))
            all_win += files[i].all_weights_in()

        all_fns.sort(reverse=True, key=lambda x: x[1].win_)

        all_win.sort(reverse=True)
        nths_percentile = 1.0
        if align_hot_n != 0.0:
            nths_percentile = int(float(len(all_win)) * align_hot_n)
            nths_percentile = all_win[nths_percentile]
        for f_and_fn in all_fns:
            if align_till <= 0.0:
                break
            files[f_and_fn[0]].order[f_and_fn[1].order_].add_alignment(
                alignment)
            num_aligned += 1

            align_till -= f_and_fn[1].win_

        for f in files:
            num_aligned += f.add_alignment_to_hottest_n_global(
                nths_percentile, alignment)
    print("Num Aligned: {} / {}".format(num_aligned, total_fns))
    for f in files:
        lines = prepare_for_target(target, f.add_order(no_cold, wildcards))
        if lines is None:
            print("Unable to get order for: {}".format(f.fname))
            continue
        outf = output
        if os.path.isdir(output):
            outf = os.path.join(output, f.script_name(target))
        if not write_file(outf, lines):
            print("Error writing result for: {}".format(f.fname))
            continue
        if prefix is None:
            outf = os.path.realpath(outf)
        else:
            outf = os.path.join(prefix, os.path.basename(outf))

        for dso in f.me:
            if dso in ord_infos:
                print("!!!!!Duplicate DSO Name: {}!!!!!".format(dso))
                continue
            ord_infos[dso] = outf

    if aliases is None:
        aliases = {}
    else:
        aliases_lines = aliases.split("\n")
        aliases = {}
        for alias in aliases_lines:
            if alias.lstrip().rstrip() == "":
                continue
            alias = alias.split()
            if len(alias) < 2:
                print("Warning: empty alias for: {}".format(alias))
                continue

            base = alias[0]
            if base not in aliases:
                aliases[base] = set()
            alias = alias[1:]
            for com in alias:
                if com == base:
                    print("Warning: {} aliases itself".format(alias[0]))
                    continue
                aliases[base].add(com)

    lines = []
    for k in ord_infos:
        base = os.path.basename(k)
        lines.append("{} {}".format(base, ord_infos[k]))
        if base in aliases:
            for alias in aliases[base]:
                lines.append("{} {}".format(alias, ord_infos[k]))
    if mapfile is not None:
        machines = machines.lstrip().rstrip()
        if len(machines) != 0:
            machines = machines.replace("-", "_").lower()
            machines = machines.split(",")
            mout = []
            for machine in machines:
                machine = machine.lstrip().rstrip()
                remap = {
                    "x86_64": "elf_x86_64",
                    "amd_64": "elf_x86_64",
                    "amd64": "elf_x86_64",
                    "x64": "elf_x86_64",
                    "i386": "elf_i386",
                    "x86": "elf_i386",
                    "x86_32": "elf_i386",
                    "x32": "elf_i386",
                }
                if machine in remap:
                    mout.append(remap[machine])
                elif len(machine) != 0:
                    mout.append(machine)
            machines = "target:" + ",".join(mout)
            lines = [machines] + lines
        if not write_file(mapfile, "\n".join(lines)):
            print("Unable to write mapfile: {}".format(mapfile))
            return False

    return len(ord_infos) != 0


ARGS = parser.parse_args()
ALIASES = ARGS.aliases
INPUT = ARGS.input
OUTPUT = ARGS.o
MAP = ARGS.map
TARGET = ARGS.target
NO_COLD = ARGS.no_cold
PREFIX = ARGS.prefix
WILDCARDS = ARGS.wildcards
MACHINES = ARGS.machine
ALIGN_HOT_N = ARGS.align_hot_n
ALIGN_TILL = ARGS.align_till
ALIGNMENT = ARGS.alignment
ALIGN_PER_DSO = ARGS.align_per_dso
if INPUT is None:
    print("Input required")
    sys.exit(-1)
if OUTPUT is None:
    print("Output required")
    sys.exit(-1)
if (not os.path.isfile(INPUT)) and (not os.path.isdir(INPUT)):
    print("Unable to find input!")
    sys.exit(-1)
if os.path.isdir(INPUT) and MAP is None:
    print("Input is a directory but map not specified")
    sys.exit(-1)
if os.path.isdir(INPUT) and os.path.isfile(OUTPUT):
    print("Input is a directory but output is a file")
    sys.exit(-1)
if os.path.isfile(INPUT) and os.path.isdir(OUTPUT):
    print("Input is a file but output is a dir")
    sys.exit(-1)

if os.path.isdir(INPUT) and not os.path.exists(OUTPUT):
    if os.system("mkdir -p {}".format(OUTPUT)) != 0:
        print("Unable to make output dir: {}".format(OUTPUT))
        sys.exit(-1)

if ALIASES is None:
    if "ORDERING_SCRIPT_ALIASES" in os.environ:
        ALIASES = os.environ["ORDERING_SCRIPT_ALIASES"]
if ALIASES is not None:
    if not os.path.isfile(ALIASES) or not os.access(ALIASES, os.R_OK):
        print("Warning: Unable to access aliases file at: {}".format(ALIASES))
        ALIASES = None
    else:
        try:
            F = open(ALIASES)
            ALIASES_DATA = F.read()
            F.close()
            ALIASES = ALIASES_DATA
        except Exception as e:
            print("Warning: Unable to read aliases file: {}\n\t{}".format(
                ALIASES, str(e)))
            ALIASES = None

if TARGET in ["gold", "ld.gold"]:
    TARGET = "gold"
elif TARGET in ["ld", "ld.orig"]:
    TARGET = "ld.orig"
elif TARGET in ["ld.patch"]:
    TARGET = "ld.patch"
else:
    print("Unknown/invalid target: {}".format(TARGET))
    sys.exit(-1)
try:
    ALIGN_HOT_N = float(ALIGN_HOT_N)
    assert ALIGN_HOT_N >= 0 and ALIGN_HOT_N <= 100.0
    ALIGN_HOT_N /= 100.0
except Exception:
    print("--align-hot-n must be a float between [0, 100]")
    sys.exit(-1)

try:
    ALIGN_TILL = float(ALIGN_TILL)
    assert ALIGN_TILL >= 0 and ALIGN_TILL <= 100.0
    ALIGN_TILL /= 100.0
except Exception:
    print("--align-till must be a float between [0, 100]")
    sys.exit(-1)

try:
    ALIGNMENT = int(ALIGNMENT)
    assert ALIGNMENT >= 0 and ALIGNMENT <= 12
except Exception:
    print("--alignment must be an integer [0, 12]")
    sys.exit(-1)

if process_dir(TARGET, INPUT, OUTPUT, MAP, NO_COLD, WILDCARDS, PREFIX,
               MACHINES, ALIGN_HOT_N, ALIGN_TILL, ALIGNMENT, ALIGN_PER_DSO,
               ALIASES):
    sys.exit(0)
sys.exit(1)
