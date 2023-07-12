import sys
import os
import argparse
from order_file import Order_File

parser = argparse.ArgumentParser(
    description="See how different two order directories are")
parser.add_argument("--exact-version",
                    action="store_true",
                    default=False,
                    help="When matching .so files, also match exact version")
parser.add_argument("--ignore-distinct",
                    action="store_true",
                    default=False,
                    help="If a DSO exists in only 1 directory, ignore it")

args, unknownargs = parser.parse_known_args()

EXACT_VERSION = args.exact_version
IGNORE_DISTINCT = args.ignore_distinct

ODIRS = unknownargs


class Dir_Orders():

    def __init__(self, dname):
        global EXACT_VERSION

        self.err_ = None
        self.dso_name_map_ = {}
        self.ofile_map_ = {}

        if not os.path.isdir(dname):
            self.err_ = "Directory '{}' does not exist".format(dname)
            return

        for f in os.listdir(dname):
            f = os.path.realpath(os.path.join(dname, f))
            if not os.access(f, os.R_OK):
                continue
            ofile = Order_File(f)
            if not ofile.okay:
                print(
                    "Warning: Unable to read order_file for:\n\t{}".format(f))
                continue

            for dso in ofile.me:
                dsos = set()
                if not EXACT_VERSION:
                    dpieces = dso.split(".")
                    partial_dso = ""
                    for dpiece in dpieces:
                        partial_dso = partial_dso + "." + dpiece
                        if ".so" in partial_dso:
                            dsos.add(partial_dso)
                for pdso in dsos:
                    if pdso in self.dso_name_map_:
                        self.dso_name_map_[pdso] = None
                    else:
                        self.dso_name_map_[pdso] = dso
                if dso in self.ofile_map_:
                    print(
                        "!!!Warning: duplicate dso name!!!\n\t{}".format(dso))
                    continue
                self.ofile_map_[dso] = ofile

        del_set = []
        for dso in self.dso_name_map_:
            if self.dso_name_map_[dso] is None:
                del_set.append(dso)

        for dso in del_set:
            del self.dso_name_map_[dso]


def get_dso_groups(dorders0, dorders1):
    global IGNORE_DISTINCT
    groups = []

    for dso in dorders0.ofile_map_:
        if dso in dorders1.ofile_map_:
            groups.append((dso, dso, dorders0.ofile_map_[dso].raw_func_order(),
                           dorders1.ofile_map_[dso].raw_func_order()))
            continue
        if dso in dorders1.dso_name_map_:
            d1 = dorders1.dso_name_map_[dso]
            if d1 in dorders1.ofile_map_:
                groups.append(
                    (dso, d1, dorders0.ofile_map_[dso].raw_func_order(),
                     dorders1.ofile_map_[d1].raw_func_order()))
                continue
        if not IGNORE_DISTINCT:
            groups.append((dso, "Not Found",
                           dorders0.ofile_map_[dso].raw_func_order(), []))

    for dso in dorders1.ofile_map_:
        if dso in dorders0.ofile_map_:
            groups.append((dso, dso, dorders0.ofile_map_[dso].raw_func_order(),
                           dorders1.ofile_map_[dso].raw_func_order()))
            continue
        if dso in dorders0.dso_name_map_:
            d0 = dorders0.dso_name_map_[dso]
            if d0 in dorders0.ofile_map_:
                groups.append(
                    (d0, dso, dorders0.ofile_map_[d0].raw_func_order(),
                     dorders1.ofile_map_[dso].raw_func_order()))
                continue
        if not IGNORE_DISTINCT:
            groups.append(("Not Found", dso, [],
                           dorders1.ofile_map_[dso].raw_func_order()))
    return groups


def compare_difference(ord0, ord1):
    map0 = {}
    map1 = {}

    if len(ord0) == 0:
        return 1, len(ord1)
    elif len(ord1) == 0:
        return 0, len(ord0)

    allv = set()

    i = 0
    for v in ord0:
        assert v not in map0
        map0[v] = i
        i += 1
        allv.add(v)
    i = 0
    for v in ord1:
        assert v not in map1
        map1[v] = i
        i += 1
        allv.add(v)

    res = 0.0
    cnt = 0
    for v in allv:
        dif = None
        if v in map0 and v in map1:
            dif = map0[v] - map1[v]
        elif v in map0:
            dif = len(map0) - map0[v]
        elif v in map1:
            dif = len(map1) - map1[v]
        else:
            assert False
        dif = abs(dif)
        dif = float(dif) + 1.0
        res += 1.0 / (dif * dif)
        cnt += 1
    return 2, float(cnt) / res


if len(ODIRS) != 2:
    print("Need 2 directories to compare!")
    sys.exit(1)

ODIR0 = Dir_Orders(ODIRS[0])
ODIR1 = Dir_Orders(ODIRS[1])
if ODIR0.err_ is not None:
    print(ODIR0.err_)
    sys.exit(1)
if ODIR1.err_ is not None:
    print(ODIR1.err_)
    sys.exit(1)

GROUPS = get_dso_groups(ODIR0, ODIR1)

COMMON_RESULTS = []
RESULTS = [[], []]
for group in GROUPS:
    idx, res = compare_difference(group[2], group[3])
    if idx == 2:
        COMMON_RESULTS.append((res, group[0], group[1]))
    else:
        RESULTS[idx].append((res, group[idx]))

COMMON_RESULTS.sort(key=lambda x: x[0], reverse=True)
RESULTS[0].sort(key=lambda x: x[0], reverse=True)
RESULTS[1].sort(key=lambda x: x[0], reverse=True)

SUMS = [0.0, 0, 0]
print("Differences in Common DSOs:")
for res in COMMON_RESULTS:
    if res[1] == res[2]:
        print("\t{}\n\t\t{}".format(os.path.basename(res[1]), round(res[0],
                                                                    3)))
    else:
        print("\t{} vs {}\n\t\t{}".format(os.path.basename(res[1]),
                                          os.path.basename(res[2]),
                                          round(res[0], 3)))
    SUMS[0] += res[0]

for idx in range(0, 2):
    print("DSOs unique to {}:".format(ODIRS[idx]))
    for res in RESULTS[idx]:
        print("\t{}\n\t\t{}".format(os.path.basename(res[1]), res[0]))
        SUMS[idx + 1] += res[0]

print("Total Distance in {:<36}: {:<4} -> {}".format("Common DSOs",
                                                  len(COMMON_RESULTS),
                                                  round(SUMS[0], 3)))
print("Total New Functions Orders in {:<24}: {:<4} -> {}".format(
    ODIRS[0], len(RESULTS[0]), SUMS[1]))
print("Total New Functions Orders in {:<24}: {:<4} -> {}".format(
    ODIRS[1], len(RESULTS[1]), SUMS[2]))
