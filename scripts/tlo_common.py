import subprocess
import random
import os
import string
import datetime
import multiprocessing
import pathlib


def cmd_exists(cmd):
    return subprocess.call("type " + cmd,
                           shell=True,
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE) == 0


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


def proc_cmdline(args):
    if isinstance(args, list):
        args = " ".join(args)
    return args


def glob_files(directory, pattern):
    return [
        os.path.join(x.parent, x.name)
        for x in pathlib.Path(directory).rglob(pattern)
    ]


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


def new_tmp_path():
    date_uid = str(datetime.datetime.now()).replace(" ", "-").replace(
        ":", "-").replace(".", "-")
    rand_uid = "".join([
        random.choice(string.ascii_letters + string.digits)
        for n in range(0, 16)
    ])
    tmp_dir = ".tmp-{}-{}".format(rand_uid, date_uid)

    complete_path = os.path.join("/tmp/", tmp_dir)
    assert os.path.exists(complete_path) is False
    return complete_path


def os_do(cmd):
    print(cmd)
    return os.system(cmd)


def perf_script_and_compress_impl(CMD_fmt, perf_file, outfile):
    nthreads = multiprocessing.cpu_count()
    if nthreads < 4:
        nthreads /= 2
    else:
        nthreads -= 2
    nthreads = int(nthreads)
    size_hint = os.path.getsize(perf_file)
    if False and size_hint is not None and int(size_hint) > 1000 * 1000:
        size_hint = "--stream-size={}".format(int(size_hint) * 1024)
    else:
        size_hint = ""
        CMD = CMD_fmt.format(perf_file, nthreads, size_hint, outfile)
    return os_do(CMD) == 0


def perf_script_and_compress(perf_file, prof_file, info_file):
    res = False
    if not cmd_exists("zstd"):
        return False

    if prof_file is not None:
        CMD = "perf script -i {} -F comm,pid,tid,time,ip,dso,brstack | zstd -9  -T{} {} > {}"
        res |= perf_script_and_compress_impl(CMD, perf_file, prof_file)
    if info_file is not None:
        CMD = "perf script -i {} -F comm,pid,tid,time  --show-mmap-events --show-task-events | zstd -20 --ultra  -T{} {} > {}"
        res |= perf_script_and_compress_impl(CMD, perf_file, info_file)
    return res


def get_all_func_sizes(dso_path):
    if not os.access(dso_path, os.R_OK):
        print("Unable to find: '{}'".format(dso_path))
        return (None, {})

    success, ret, stdout, stderr = proc_run("readelf -Ws {}".format(dso_path))

    if not success or ret != 0:
        print("Unable to dump dso: '{}'".format(dso_path))
        return (None, {})

    out = {}
    stdout = stdout.split("\n")
    for line in stdout:
        line = line.split()
        if len(line) < 8:
            continue

        size = line[2]
        stype = line[3]
        func = line[7]
        try:
            size = int(size)
        except Exception:
            continue
        if stype != "FUNC":
            continue
        if size == 0:
            continue

        if func in out:
            print("Duplicate function: '{}'".format(func))
            out[func] = max(size, out[func])
        else:
            out[func] = size

    dso = os.path.basename(dso_path)
    return (dso, out)
