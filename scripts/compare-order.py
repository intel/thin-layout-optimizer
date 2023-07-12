import sys
import tlo_common

if len(sys.argv) < 3:
    print("Usage: {} <order-file> <1st DSO> ... <Nth DSO>".format(sys.argv[0]))
    sys.exit(0)

ORDER_FILE = sys.argv[1]

ORDER = {}
try:
    for line in open(ORDER_FILE):
        line = line.replace(" ", "")
        line = line.replace("\t", "")
        line = line.split(",")

        if line[0] != "order":
            continue
        assert line[2] not in ORDER, "{} dup".format(line[2])
        ORDER[line[2]] = len(ORDER)

except Exception as e:
    print("IO Error: {}".format(e))
    sys.exit(1)


def get_bin_func_order(binname):
    success, ret, stdout, stderr = tlo_common.proc_run(
        "readelf -Ws {}".format(binname))

    if not success or ret != 0:
        print("Error dumping binary: {}".format(binname))
        return []
    stdout = stdout.split("\n")
    syms_w_addr = []
    for line in stdout:
        line = line.split()
        if len(line) < 8:
            continue

        addr = line[1]
        size = line[2]
        stype = line[3]
        func = line[7]

        try:
            addr = int(addr, 16)
        except Exception:
            continue

        try:
            size = int(size)
        except Exception:
            continue

        if stype != "FUNC":
            continue

        if addr == 0:
            continue
        if size == 0:
            continue

        if func not in ORDER:
            continue

        syms_w_addr.append((addr, func))
    syms = []
    syms_w_addr = sorted(syms_w_addr, key=lambda x: x[0])
    for i in range(0, len(syms_w_addr)):
        syms.append((i, syms_w_addr[i][1]))
    return syms


def get_expec_placement(sym_w_idx):
    global ORDER
    if sym_w_idx[1] not in ORDER:
        return len(ORDER) + sym_w_idx[0]
    return ORDER[sym_w_idx[1]]


def measure_distance(syms_w_order, valid_only):
    global ORDER
    sorted_syms = sorted(syms_w_order, key=lambda x: get_expec_placement(x))

    inv_mean = 0.0
    cnt = 0

    for i in range(0, len(sorted_syms)):
        if valid_only and sorted_syms[i][1] not in ORDER:
            continue
        cnt += 1
        dist = abs(float(i - sorted_syms[i][0])) + 1.0
        inv_mean += 1.0 / (dist * dist)
    hmean = float(cnt) / inv_mean

    return round(hmean, 3)


for arg in sys.argv[2:]:
    binorder = get_bin_func_order(arg)

    if len(binorder) == 0:
        continue
    distv = measure_distance(binorder, True)
    distnv = measure_distance(binorder, False)

    print("{} -> {}/{}".format(arg, distv, distnv))
