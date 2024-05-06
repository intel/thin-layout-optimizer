import json
import argparse
import sys
import os

PARSER = argparse.ArgumentParser(
    description="Update function information in a save-state")

PARSER.add_argument("-i",
                    "--input",
                    action="store",
                    required=True,
                    default=None,
                    help="Input save-state")
PARSER.add_argument("-n",
                    "--new",
                    action="store",
                    required=True,
                    default=None,
                    help="New func-info-map (see script get-dso-infos.py).")
PARSER.add_argument("-o",
                    "--out",
                    action="store",
                    required=True,
                    default=None,
                    help="Output file")


def load_json(path):
    js = None
    try:
        if not os.access(INPUT, os.R_OK):
            print("File '{}' does not exist with read permission".format(path))
            return None
        f = open(path)
        js = json.load(f)
        f.close()
    except Exception as e:
        print("Unable to load json: '{}'\n\t{}".format(f.name, str(e)))
    return js


ARGS = PARSER.parse_args()

INPUT = ARGS.input
NEW = ARGS.new
OUT = ARGS.out

assert (INPUT is not None) and (NEW is not None) and (OUT is not None)

INPUT_JS = load_json(INPUT)
if INPUT_JS is None:
    sys.exit(-1)

NEW_JS = load_json(NEW)
if NEW_JS is None:
    sys.exit(-1)

if "dsos" not in INPUT_JS:
    print("Input missing 'dsos' field")
    sys.exit(-1)

INPUT_JS_DSOS = INPUT_JS["dsos"]
DSO_UID_MAP = {}
ALL_NAMES = set()
for JS_DSO in INPUT_JS_DSOS:
    if "uid" not in JS_DSO:
        print("Invalid dso field in input: Missing 'uid'")
        sys.exit(-1)
    if "name" not in JS_DSO:
        print("Invalid dso field in input: Missing 'name'")
        sys.exit(-1)

    NAME = JS_DSO["name"]
    UID = JS_DSO["uid"]

    if NAME in ALL_NAMES:
        print("Duplicate DSO: '{}'".format(NAME))
        sys.exit(-1)
    if UID in DSO_UID_MAP:
        print("Duplicate UID: '{}'".format(UID))
        sys.exit(-1)

    DSO_UID_MAP[UID] = NAME
    ALL_NAMES.add(UID)

if "all_funcs" not in INPUT_JS:
    print("Input missing 'all_funcs' field")
    sys.exit(-1)

INPUT_JS_FUNCS = INPUT_JS["all_funcs"]
if "func_clumps" not in INPUT_JS_FUNCS:
    print("Input missing 'all_funcs'['func_clumps']")
    sys.exit(-1)

INPUT_JS_FUNCS = INPUT_JS_FUNCS["func_clumps"]
for FC_JS in INPUT_JS_FUNCS:
    if "dso_uid" not in FC_JS:
        print("Func clump missing 'dso_uid'")
        sys.exit(-1)
    if "funcs" not in FC_JS:
        print("Func clump missing 'funcs'")
        sys.exit(-1)
    if "size" not in FC_JS:
        print("Func clump missing 'size'")
        sys.exit(-1)

    UID = FC_JS["dso_uid"]
    FUNCS_ARR = FC_JS["funcs"]
    SIZE = FC_JS["size"]
    try:
        SIZE = int(SIZE)
    except Exception:
        print("Size is non-integral")
        sys.exit(-1)

    if UID not in DSO_UID_MAP:
        print("Unknown dso_uid: '{}'".format(UID))
        sys.exit(-1)

    DNAME = DSO_UID_MAP[UID]
    DNAME_BASE = os.path.basename(DNAME)

    NEW_SIZE = 0
    FOUND = False
    for FUNC_JS in FUNCS_ARR:
        if "exact_info" not in FUNC_JS:
            print("Function missing 'exact_info'")
            sys.exit(-1)
        if "name" not in FUNC_JS:
            print("Function missing 'name'")
            sys.exit(-1)

        FUNC_JS["exact_info"] = False
        FNAME = FUNC_JS["name"]

        if DNAME_BASE not in NEW_JS:
            continue

        if FNAME not in NEW_JS[DNAME_BASE]:
            continue

        FOUND = True
        NSIZE = NEW_JS[DNAME_BASE][FNAME]
        try:
            NSIZE = int(NSIZE)
        except Exception:
            print("New size is non-integral")
            sys.exit(-1)

        NEW_SIZE += NSIZE

    if FOUND:
        FC_JS["size"] = NEW_SIZE

try:
    OUT_F = open(OUT, "w+")
    json.dump(INPUT_JS, OUT_F, indent=2)
    OUT_F.close()
except Exception as e:
    print("Unable to write result to: '{}'\n\t{}".format(OUT, str(e)))
