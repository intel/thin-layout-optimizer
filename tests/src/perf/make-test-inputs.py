import os
import sys

if os.path.isfile(os.path.join(os.getcwd(), sys.argv[0]) + ".done"):
    sys.exit(0)

print("Generating perf tests. This may take a while")
    
assert len(sys.argv) == 2
assert os.path.isdir(sys.argv[1])
if not os.path.exists(os.path.join(sys.argv[1], "test-inputs")):
    pieces = os.path.join(sys.argv[1],
                          ".test-input-pieces/test-input.tar.gz.piece.")
    pieces += "*"
    os.system("cat {} | tar -C {} -xzf -".format(pieces, sys.argv[1]))
os.system(
    "echo 1 > {}".format(os.path.join(os.getcwd(), sys.argv[0]) + ".done"))
