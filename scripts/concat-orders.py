import sys
import os
import pathlib
import argparse
import fnmatch
from order_file import Order_File

parser = argparse.ArgumentParser(
    description="Concat multiple ordering files into one")
parser.add_argument("-i",
                    "--idir",
                    action="store",
                    default=None,
                    help="Input directory")
parser.add_argument("-o", action="store", default=None, help="Output file")
parser.add_argument(
    "-c",
    "--comms",
    action="store",
    default=None,
    help=
    "Command lines to create order for as csv. (i.e for llvm maybe: clang,lld,opt,llc)"
)
parser.add_argument("-d",
                    "--dsos",
                    action="store",
                    default=None,
                    help="Dsos to create order for.")
parser.add_argument("--cd",
                    action="store",
                    default=None,
                    help="Either comm or DSO.")
parser.add_argument(
    "-r",
    "--recurse",
    action="store",
    default="1",
    help=
    "Level of recursing on dso dependencies.\n\t0 -> No recursing\n\t1 -> Recurse on all packages except some system ones (like libc/libm/vdso/ld-linux)\n\t2 -> Recurse on all"
)


def collect_files_impl(idir):
    if os.path.isdir(idir):
        return list(set(pathlib.Path(idir).rglob("*.txt")))
    else:
        return []


def collect_files(idir):
    files = collect_files_impl(idir)
    out = []
    for f in files:
        out.append(Order_File(os.path.join(idir, f.name)))
    return sorted(out, key=lambda x: len(x.order), reverse=True)


def concat_files(files, comms, dsos, comm_or_dso, recurse):
    output = []
    kset = set()
    changed = True
    files = sorted(files, key=lambda x: x.filesize(), reverse=True)

    while changed:
        changed = False
        for f in files:
            if f.key() in kset:
                continue
            add = 0
            for comm in comms:
                if (recurse == "0" or recurse == "1") and f.should_skip_dso():
                    continue
                if add:
                    break
                if f.matches_comm(comm):
                    add = 1
                elif f.matches_comm(comm_or_dso):
                    add = 1
            for dso in dsos:
                if (recurse == "0"
                        or recurse == "1") and f.should_skip_dso(dso):
                    continue
                if add:
                    break
                if f.matches_dso(dso):
                    add = 2
                elif f.matches_dso(comm_or_dso):
                    add = 2
            if add != 0:
                changed = True
                output.append(f)
                kset.add(f.key())
                print("Getting order for: {}".format(f.key()))
                if add == 1:
                    dsos |= f.extract_dso_deps(recurse)
    if len(output) == 0:
        return None

    for i in range(1, len(output)):
        output[0] = output[0].merge(output[i])
    return output[0]


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


def process_dir(idir, outfile, comms, dsos, comm_or_dso, recurse):
    files = collect_files(idir)
    if len(files) == 0:
        print("Unable to find orders in: {}".format(idir))
        return False
    if comms is not None:
        comms = set(comms.split(","))
    else:
        comms = set()
    if dsos is not None:
        dsos = set(dsos.split(","))
    else:
        dsos = set()

    output = concat_files(files, comms, dsos, comm_or_dso, recurse)
    if output is not None:
        output = output.output_ordering_file()

    if output is None:
        print("No output...")
        return False

    if not write_file(outfile, output):
        print("Error writing result")
        return False
    return True


ARGS = parser.parse_args()
INPUT_DIR = ARGS.idir
OUTPUT_FILE = ARGS.o
COMMS = ARGS.comms
DSOS = ARGS.dsos
COMM_OR_DSO = ARGS.cd
RECURSE = ARGS.recurse
if INPUT_DIR is None:
    print("Input directory required")
    sys.exit(-1)
if COMMS is None and DSOS is None:
    print("No comms/dsos... nothing to do")
    sys.exit(-1)

if OUTPUT_FILE is None:
    print("Warning: No output file, defaulting to stdout")

if process_dir(INPUT_DIR, OUTPUT_FILE, COMMS, DSOS, COMM_OR_DSO, RECURSE):
    sys.exit(0)
sys.exit(1)
