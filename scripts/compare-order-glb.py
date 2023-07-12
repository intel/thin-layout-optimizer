import subprocess
import sys
import re
import os

G_INPUT0 = None
G_INPUT1 = None
G_MAP = None


def proc_cmdline(args):
    if isinstance(args, list):
        args = " ".join(args)
    return args


def proc_check_args(args):
    if args is None:
        return False
    if isinstance(args, list):
        if len(args) == 0:
            return False
        try:
            args = [str(arg) for arg in args]
        except Exception:
            return False

        return True

    if isinstance(args, str):
        if len(args) == 0:
            return False

        return True

    return False


def proc_run(args, timeout=None, decode=True, stdin_data=None):
    success = False
    ret = None
    stdout_data = None
    stderr_data = None

    if not proc_check_args(args):
        print("Invalid process arguments")
        return success, ret, stdout_data, stderr_data

    in_shell = True
    if isinstance(args, list):
        in_shell = False
        args = [str(arg) for arg in args]

    if (" " not in args):
        in_shell = False
    print("Running: {}".format(proc_cmdline(args)))
    try:
        sproc = None
        stdout_data = None
        stderr_data = None
        if stdin_data is not None:
            sproc = subprocess.Popen(args,
                                     shell=in_shell,
                                     stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
            stdout_data, stderr_data = sproc.communicate(input=bytes(
                stdin_data, "ascii"),
                                                         timeout=timeout)
        else:
            sproc = subprocess.Popen(args,
                                     shell=in_shell,
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
            stdout_data, stderr_data = sproc.communicate(timeout=timeout)
        ret = sproc.returncode
        if decode:
            try:
                if stdout_data is not None:
                    stdout_data = stdout_data.decode("utf-8")
                if stderr_data is not None:
                    stderr_data = stderr_data.decode("utf-8")
            except Exception:
                print("Unable to decode output for: {}".format(
                    proc_cmdline(args)))
                stdout_data = None
                stderr_data = None

        success = True

    except Exception as e:
        print("Error Running: {}\n{}".format(proc_cmdline(args), str(e)))
    return success, ret, stdout_data, stderr_data


def usage():
    print("{} <File to Check> <Opt:file to compare> -m=<Opt: Map File>".format(
        sys.argv[0]))
    sys.exit(0)


def is_elf(f):
    success, ret, stdout_data, stderr_data = proc_run(["file", f])
    if not success or ret != 0:
        print("Unable to determine if {} is an elf".format(f))
        return False

    if re.match(r'{}:\sELF\s\d+-bit.*x86-64.*BuildID'.format(f), stdout_data):
        return True
    return False


def elf_func_order(f):
    success, ret, stdout, stderr = proc_run("readelf -Ws {}".format(f))
    if not success or ret != 0:
        print("Error dumping binary: {}".format(f))
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

        syms_w_addr.append((addr, func))
    syms = []
    syms_w_addr = sorted(syms_w_addr, key=lambda x: x[0])
    for i in range(0, len(syms_w_addr)):
        syms.append(syms_w_addr[i][1])
    return syms


def script_func_order(f):
    content = ""
    try:
        fhandle = open(f)
        content = fhandle.read()
        fhandle.close()
    except Exception as e:
        print("Unable to get function order for {}\n\t{}".format(f, str(e)))
        return []

    content = content.split("\n")
    syms = []
    has = set()
    for line in content:
        line = line.lstrip().rstrip()
        add = None
        if line.startswith("*(") and line.endswith(")"):
            line = line[2:-1]
        if line.startswith(".text.cold."):
            line = line[11:]
            add = line
        elif line.startswith(".text.unlikely."):
            line = line[15:]
            add = line
        elif line.startswith(".text.hot."):
            line = line[10:]
            add = line
        elif line.startswith(".text.*."):
            line = line[8:]
            add = line
        elif line.startswith(".text."):
            line = line[6:]
            add = line
        if add is None:
            continue
        add = line.split()[0]
        if add in has:
            continue
        syms.append(add)
        has.add(add)
    return syms


def get_order(f):
    if is_elf(f):
        return elf_func_order(f)
    else:
        return script_func_order(f)


def compare(f0, f1):
    ord0 = get_order(f0)
    ord1 = get_order(f1)

    if len(ord0) == 0:
        print("No order found for: {}".format(f0))
        return

    if len(ord1) == 0:
        print("No order found for: {}".format(f1))
        return

    common = set(ord0) & set(ord1)
    assert len(common) <= len(ord0)
    assert len(common) <= len(ord1)
    common_pos0 = {}
    for i in range(0, len(ord0)):
        sym = ord0[i]
        if sym not in common:
            continue
        common_pos0[sym] = len(common_pos0)

    inv_mean = 0.0
    cnt = 0
    for i in range(0, len(ord1)):
        sym = ord1[i]
        if sym not in common:
            continue
        pos0 = common_pos0[sym]
        pos1 = cnt

        cnt += 1
        dist = abs(float(pos0 - pos1)) + 1.0
        inv_mean += 1.0 / (dist * dist)
    hmean = float(cnt) / inv_mean

    print("Difference between\ni0={}\ni1={}".format(f0, f1))
    print("Distance: {}".format(hmean))
    print("Common    : {}".format(len(common)))
    print("i0 Uniques: {}".format(len(ord0) - len(common)))
    print("i1 Uniques: {}".format(len(ord1) - len(common)))


def find_script_file(f):
    if G_MAP is None or not os.access(G_MAP, os.R_OK):
        return None

    if not is_elf(f):
        return None

    realf = os.path.basename(os.path.realpath(f))
    if ".so." in realf:
        realf = realf.split(".so")[0] + ".so"

    try:
        for line in open(G_MAP):
            line = line.lstrip().rstrip()

            pieces = line.split()
            if len(pieces) != 2:
                continue

            name = pieces[0]
            if ".so" in name:
                name = name.split(".so")[0] + ".so"

            if name == realf:
                return pieces[1]

    except Exception as e:
        print("Unable to read map {}\n\t{}".format(G_MAP, str(e)))
    return None


for G_ARG in sys.argv[1:]:
    if G_ARG.startswith("-m="):
        if G_MAP is not None:
            print("-m set twice")
            sys.exit(1)
        G_MAP = G_ARG[3:]

    elif G_INPUT0 is None:
        G_INPUT0 = G_ARG
    else:
        G_INPUT1 = G_ARG

if G_MAP is None:
    if "LD_ORDERING_SCRIPT_MAP" in os.environ:
        G_MAP = os.environ["LD_ORDERING_SCRIPT_MAP"]
    elif "GOLD_ORDERING_SCRIPT_MAP" in os.environ:
        G_MAP = os.environ["GOLD_ORDERING_SCRIPT_MAP"]

if G_INPUT0 is None:
    print("Missing input file!")
    sys.exit(2)
if not os.path.isfile(G_INPUT0):
    print("{} is not a file!")
    sys.exit(2)

if G_INPUT1 is None and G_MAP is None:
    print("Need either second file or map")
    sys.exit(2)

if G_INPUT1 is None:
    G_INPUT1 = find_script_file(G_INPUT0)
    if G_INPUT1 is None:
        print("Unable to find script for {}".format(G_INPUT0))
        sys.exit(2)

compare(G_INPUT0, G_INPUT1)
