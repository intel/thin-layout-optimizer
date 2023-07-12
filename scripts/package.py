import sys
import os
import tlo_common
import pathlib

if len(sys.argv) < 3:
    print(
        "Usage: python3 package.py <perf.data file> <output.tar> <opt:compress>"
    )
    sys.exit(0)


def is_elf64(filename):
    success, ret, stdout, stderr = tlo_common.proc_run(
        "file {}".format(filename))
    if not success or ret != 0:
        return False
    return "Elf 64-bit" in stdout


class Debug_Files():

    def __init__(self):
        sys_debug_files = tlo_common.glob_files("/usr", "*.debug")

        self.all_files = {}
        for path in sys_debug_files:
            if not is_elf64(path):
                continue
            self.all_files[os.path.basename(path)].setdefault([]).append(path)
        for path in self.all_files:
            self.all_files[os.path.basename(path)].sort(
                key=lambda x: os.stat(x).st_ctime, reversed=True)

    def get_buildid(self, dso_filename):
        success, ret, stdout, stderr = tlo_common.proc_run(
            "file {}".format(dso_filename))
        if not success or ret != 0:
            return None

        key = "BuildID[sha1]="
        pos = stdout.find(key)
        if pos < 0:
            return None

        buildid = stdout[pos + len(key):]
        buildid = buildid.split(",")[0]
        buildid = buildid.lstrip().rstrip()
        return buildid

    def get_debugfile(self, dso_filename):
        buildid = self.get_buildid(dso_filename)
        if buildid is not None:
            buildlow = buildid[0:2]
            buildhi = buildid[2:]
            check = "/usr/lib/debug/.build-id/{}/{}.debug".format(
                buildlow, buildhi)
            if os.path.isfile(check):
                return check

            if buildid in self.all_files:
                return self.all_files[buildid][0]

        basename = os.path.basename(dso_filename)
        basename += ".debug"
        best = None
        if basename in self.all_files:
            for path in self.all_files[basename]:
                if best is None:
                    best = path
                if len(best) < len(os.path.commonpath([dso_filename, path])):
                    best = path
        return best


def get_dsos(perf_data_file):
    CMD = "perf script -i " + perf_data_file + " -F dso --show-mmap-events"
    success, ret, stdout_data, stderr_data = tlo_common.proc_run(CMD)

    if (not success) or (not isinstance(ret, int) or ret != 0) or (stdout_data
                                                                   is None):
        print("Unable to find dso list!")
        return []
    raw_dsos = stdout_data.split("\n")
    dsos = set()
    for dso in raw_dsos:
        dso = dso.split()
        if len(dso) <= 2:
            continue

        name = dso[-1]
        if name == "(deleted)":
            name = dso[-2]
        dsos.add(name)


#    dsos = list(set(stdout_data.split("\n")))
    return dsos


def copy_file(src, dst, dbginfo=None, full_path=False):
    if not os.path.isfile(src) or not os.access(src, os.R_OK):
        return False
    if full_path:
        full_path = "--parents"
    else:
        full_path = ""
    CMD = "cp {} {} {}".format(src, full_path, dst)
    if tlo_common.os_do(CMD) != 0:
        print("Unable to copy: {} -> {}".format(src, dst))
        return False

    if dbginfo is None:
        return True
    dbgfile = dbginfo.get_debugfile(src)
    if dbgfile is None:
        return True

    true_dst = os.path.join(dst, src)

    while dst.endswith("/"):
        dst = dst[:-1]
    while src.startswith("/"):
        src = src[1:]

    true_dst = "{}/{}".format(dst, src)
    if not os.path.isfile(true_dst):
        return False

    CMD = "cp {} {}".format(dbgfile, true_dst + ".debug")
    tlo_common.os_do(CMD)

    return True


def copy_perf_file(src, dst, do_compress):
    dst_perf_file = os.path.join(dst, "perf.data")
    if not copy_file(src, dst_perf_file):
        print("Unable to copy perf.data")
        return False
    if do_compress:
        return tlo_common.perf_script_and_compress(
            dst_perf_file, os.path.join(dst, "profile.zst"),
            os.path.join(dst, "info-events.zst"))
    return True


def copy_dsos(dsos, dst, dbginfo):
    for dso in dsos:
        if not copy_file(dso, dst, dbginfo, True):
            print("Unable to copy: {}".format(dso))


def package(perf_data_file, dst, do_compress):
    dbginfo = Debug_Files()

    tmpdir = tlo_common.new_tmp_path()
    os.mkdir(tmpdir)
    dsos = get_dsos(perf_data_file)
    copy_dsos(dsos, tmpdir, dbginfo)

    if not copy_perf_file(perf_data_file, tmpdir, do_compress):
        print("Error making package")
        return False

    if dst.endswith(".tar.gz"):
        pass
    elif dst.endswith(".tar"):
        dst += ".gz"
    else:
        dst += ".tar.gz"
    CMD = "tar -C {} -cvzf {} .".format(tmpdir, dst)
    return tlo_common.os_do(CMD) == 0


DO_COMPRESS = False
if len(sys.argv) > 3 and sys.argv[3].lower() == "compress":
    DO_COMPRESS = True
if not package(sys.argv[1], sys.argv[2], DO_COMPRESS):
    print("Error packaging")
    sys.exit(-1)
sys.exit(0)
