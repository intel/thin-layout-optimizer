import sys
import os
import pathlib
import tlo_common
# /home/noah/programs/opensource/linux-dev/src/custom-perf/bin/perf
# PYTHON=python3 make -C tools/perf/ prefix=/home/noah/programs/opensource/linux-dev/src/custom-perf install
if len(sys.argv) < 3:
    print("Usage: python3 package.py <tar file> <output dir> <opt:compress>")
    sys.exit(0)


def process_perf_file(do_compress, unpack_dir):
    if not os.path.isdir(unpack_dir):
        print("Unable to access unpack dir: {}".format(unpack_dir))
        return False
    perf_files = list(pathlib.Path(unpack_dir).rglob("perf.data"))
    if len(perf_files) != 1:
        print("Unable to find perf file: {}/{}".format(unpack_dir, perf_files))
        return False
    perf_file = perf_files[0].absolute()

    if not os.path.isfile(perf_file) or not os.access(perf_file, os.R_OK):
        print("Unable to find perf.data (tried: {})".format(perf_file))
        return False

    if not do_compress:
        return True

    prof_file = os.path.join(unpack_dir, "profile.zst")
    info_file = os.path.join(unpack_dir, "info-events.zst")

    if os.path.isfile(prof_file) and os.path.isfile(info_file):
        return True

    if os.path.isfile(prof_file):
        prof_file = None
    if os.path.isfile(info_file):
        info_file = None

    return tlo_common.perf_script_and_compress(perf_file, prof_file, info_file)


def unpack_tar(tar_file, dst):
    if os.path.isdir(dst):
        print("Error: Unpack dst already exists ({})".format(dst))
        return False
    pathlib.Path(dst).mkdir(parents=True, exist_ok=True)
    CMD = "tar -C {} -xvzf {}".format(dst, tar_file)
    return tlo_common.os_do(CMD) == 0


def unpackage(tar_file, dst, do_compress):
    if not unpack_tar(tar_file, dst):
        print("Error unpacking")
        return False
    if not process_perf_file(do_compress, dst):
        print("Error processing")
        return False

    return True


DO_COMPRESS = False
if len(sys.argv) > 3 and sys.argv[3].lower() == "compress":
    DO_COMPRESS = True
if not unpackage(sys.argv[1], sys.argv[2], DO_COMPRESS):
    print("Error unpackaging")
    sys.exit(-1)
sys.exit(0)
