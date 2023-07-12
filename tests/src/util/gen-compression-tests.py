import os
import sys
import random
import datetime

if os.path.isfile(os.path.join(os.getcwd(), sys.argv[0]) + ".done"):
    sys.exit(0)

print("Generating compression tests. This may take a while")
    
SCRIPT_BASE_PATH = os.path.realpath(sys.argv[0])
SCRIPT_DIR = os.path.dirname(SCRIPT_BASE_PATH)
PROJECT_DIR = os.path.realpath(SCRIPT_DIR + "/../../../")

assert os.path.isfile(os.path.join(
    PROJECT_DIR, "CMakeLists.txt")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    PROJECT_DIR, "build")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    PROJECT_DIR, "src")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    PROJECT_DIR, "tests")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    PROJECT_DIR, "scripts")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    PROJECT_DIR, "resources")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    PROJECT_DIR, "cmake")), "Unable to verify directory structure"

TESTS_DIR = os.path.join(PROJECT_DIR, "tests")

assert os.path.isfile(os.path.join(
    TESTS_DIR, "CMakeLists.txt")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    TESTS_DIR, "src")), "Unable to verify directory structure"

TESTS_SRC_DIR = os.path.join(TESTS_DIR, "src")
assert os.path.isfile(os.path.join(
    TESTS_SRC_DIR, "CMakeLists.txt")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    TESTS_SRC_DIR, "util")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    TESTS_SRC_DIR, "system")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    TESTS_SRC_DIR, "sym")), "Unable to verify directory structure"
assert os.path.isdir(os.path.join(
    TESTS_SRC_DIR, "perf")), "Unable to verify directory structure"

TESTS_UTIL_DIR = os.path.join(TESTS_SRC_DIR, "util")
assert os.path.isfile(os.path.join(
    TESTS_UTIL_DIR, "CMakeLists.txt")), "Unable to verify directory structure"
assert os.path.isfile(os.path.join(
    TESTS_UTIL_DIR, "test-bits.cc")), "Unable to verify directory structure"
assert os.path.isfile(os.path.join(
    TESTS_UTIL_DIR,
    "test-file-reader.cc")), "Unable to verify directory structure"
assert os.path.isfile(os.path.join(
    TESTS_UTIL_DIR, "test-reader.cc")), "Unable to verify directory structure"
assert os.path.isfile(
    os.path.join(TESTS_UTIL_DIR, "compressed-files-test-helper.h")
), "Unable to verify directory structure"

DST_PATH = os.path.join(TESTS_UTIL_DIR, "test-inputs")
print(DST_PATH)

DATE_UID = str(datetime.datetime.now()).replace(" ", "-").replace(":",
                                                                  "-").replace(
                                                                      ".", "-")

EXE_PATH = "/tmp/gen-compression-test-files-{}".format(DATE_UID)
BUILD_CMD = "gcc -O2 -Wall " + SCRIPT_DIR + "/gen-compression-test-files.c -o " + EXE_PATH
TMP_DIR = "/tmp/generated-compression-tests-{}/".format(DATE_UID)


def os_do(cmd):
    print(cmd)
    assert os.system(cmd) == 0


os_do("mkdir -p {}".format(TMP_DIR))
os_do("mkdir -p {}".format(DST_PATH))

os_do(BUILD_CMD)

args = [(100, 100000, 1), (1000, 10000, 5), (10000, 1000, 10),
        (10, 1000000, 15), (500, 100000, 19)]

ZSTD_CMD = "zstd -{} --long={} {} {} -o " + DST_PATH + "/{}"

for existing_f in os.listdir(DST_PATH):
    if existing_f.startswith("generated-test-compressed-reader-input-c"):
        os_do("rm {}".format(os.path.join(DST_PATH, existing_f)))

for arg in args:
    os_do(EXE_PATH + " {} {} {} {}".format(TMP_DIR, arg[0], arg[1], arg[2]))
    in_file = "generated-test-compressed-reader-input-c{}".format(arg[2])
    in_path = os.path.join(TMP_DIR, in_file)
    assert os.path.isfile(in_path)

    nlevels = set()
    while len(nlevels) != 5:
        mwidth = random.randint(25, 31)
        clevel = random.randint(1, 22)
        if clevel in nlevels:
            continue

        nlevels.add(clevel)
        ultra = ""
        if clevel > 19:
            ultra = "--ultra"

        out_file = in_file + ".zst." + str(clevel)

        os_do(ZSTD_CMD.format(clevel, mwidth, ultra, in_path, out_file))
    os_do("cp {} {}".format(in_path, DST_PATH))
os_do("echo 1 > {}".format(
    os.path.join(os.getcwd(), sys.argv[0]) + ".done"))
