import json
import argparse
import tlo_common
import copy

PARSER = argparse.ArgumentParser(
    description="Create new function info from input dsos")

PARSER.add_argument("-o",
                    "--out",
                    action="store",
                    required=True,
                    default=None,
                    help="Output file")

ARGS, UNKNOWNARGS = PARSER.parse_known_args()

OUT = ARGS.out
assert OUT is not None

OUT_DICT = {}
for DSO in UNKNOWNARGS:
    (DSO_NAME, DSO_DICT) = tlo_common.get_all_func_sizes(DSO)

    if DSO_NAME in OUT_DICT:
        print("Duplicate DSO: '{}'".format(DSO_NAME))
        continue
    OUT_DICT[DSO_NAME] = copy.deepcopy(DSO_DICT)

try:
    OUT_F = open(OUT, "w+")
    json.dump(OUT_DICT, OUT_F, indent=2)
    OUT_F.close()
except Exception as e:
    print("Unable to write result to: '{}'\n\t{}".format(OUT, str(e)))
