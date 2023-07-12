#! /usr/bin/env python3

import os
os.system("perf record -e cycles:u,branch-misses -j any,u -a")
# Processing (with patched perf)
# $> perf script -F pid,tid,dso,ip,sym,brstack,time --no-demangle
