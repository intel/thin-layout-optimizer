# Perf Profile -> Optimized Linking Function Layout

`thin-layout-optimizer` is a project that takes a performance profile from a
tool like `perf` and uses the data (particularly `lbr` data) to the
layout of the profiled DSO(s) functions.

Currently it uses a basic `hfsort` algorithm (or other re-ordering
algorithms) to do this. The output is a text file with some
information from the profiles (including the order), which is meant to
be consumed by the script
`finalize-order.py` to create a function order list for either `ld` or
`gold`.

## Build

- `mkdir -p build && (cd build && cmake -GNinja .. && ninja)`

    Note: There are soft dependencies on `zstd` and `gtest`. Without
    `zstd` there is no support for reading compressed
    profiles. Without `gtest` the tests don't work.

    `cmake` will try to find the respective system packages for the
    two dependencies above. Alternatively you can manually set this
    them `ZSTD_PATH` and `GTEST_PATH` respectively.

    For example:
    ```
    cmake .. -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DZSTD_PATH=/home/noah/programs/libraries/zstd/build-std-flto
    ```



### Tests
- `ninja check-all`

### Sanitized Build
- `ninja san`
    - This will produce builds for `asan`, `usan`, `lsan`, and (if compiling with `clang`) `msan`.
    Note: Sanitized tests can be run with `ninja check-all-san` (or
    `check-all-all` to also run non-sanitized tests).

    Note: To support compression with `msan`, you will need to provide a path to `zstd` built with `msan`. For example:

    ```
    cmake .. -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DZSTD_PATH=/home/noah/programs/libraries/zstd/build-std-flto -DZSTD_MSAN_PATH=/home/noah/programs/libraries/zstd/build-msan16
    ```


## Usage

### ld Setup



- Since `ld` does not naturally have support for passing an
  independent function-ordering file, **we require patches to
  `ld`**. The patches add a new option to `ld`,
  `--text-section-ordering-file` which accepts an ordering file. This
  can be difficult to generically add to existing link steps as they
  might rely on previously existing linker scripts. To simplify this
  process we have a patch for `ld.2.41` (most recent release as of
  writing this) to read a linker script from an env variable;
  `LD_ORDERING_SCRIPT`. The patch makes is so that if no linker script
  argument was provided, it will check if `LD_ORDERING_SCRIPT`
  contains a valid script. Furthermore, as it can be difficult to
  juggle the right env variable for multiple DSOs, there is also an
  env variable `LD_ORDERING_SCRIPT_MAP`. This variable acts a map from
  DSO -> ordering script. When linking, if there is no commandline or
  explicit env linker script provided, it will lookup the output DSO
  in the file the env variable `LD_ORDERING_SCRIPT_MAP` it set to. If
  it find the output DSO, it will use its corresponding argument as
  the file for the linker script.

1. Patches:

    - `git am patches/ld/*.patch`

2. Usage(`LD_ORDERING_SCRIPT`):

    - `export LD_ORDERING_SCRIPT=ordering.lds; ld ...`

    **NOTE** If the `.lds` linker script file exists, it must be a
    syntactically valid linker script, otherwise `ld` will fail. If
    the fail doesn't exist it will be ignored.

3. Usage(`LD_ORDERING_SCRIPT_MAP`):
    Map syntax:
    ```
    target:<target_name0>,<target_name1>,...<target_nameN>
    DSO0 <ordering-script0>
    DSO1 <ordering-script1>
    ...
    DSON <ordering-scriptN>
    ```
    - `export LD_ORDERING_SCRIPT_MAP=ordering_map; ld ...`


    Note the target name is optional. If it is present, we will only
    using the ordering script map if the target string matches the
    link target for `ld`/`gold`. If it is not present, we will always
    use the map. The target name is the name used for the emulation
    (`-m`) option of `ld` (even if using `gold`, specify the `ld`
    emulation name). For example to make a map file for only `x86_64`
    specify `target:elf_x86_64` before any DSOs.

    Note: Prior to reaching a `target:` line, we will watch DSOs to
    any target. This also means that if `target:` is not present, we
    will match all the DSOs regardless of target.

4. Unpatched `ld`:
    It is possible to use the `finalize-order.py` script to generate
    an `ld` linker script using the `ld.orig` target. The generated
    linker script will have the ordering embedded in the best guess of
    the default linker script. While you may have some success with
    this, it is not advised as some `ld` options may essentially end
    up overriden by overriding the default linker script.



#### gold
- Equivilent patches exist for `gold` which allows the section
  ordering script to be passed through the `GOLD_ORDERING_SCRIPT` or
  `GOLD_ORDERING_SCRIPT_MAP` env variables. That being said, `gold`
  natively supports a section ordering file with the
  `--section-ordering-file` argument.

##### Without Patches
1. Usage Without Patches:

    - `gold --section-ordering-file ordering.txt ...`

    **NOTE** The `gold` linker tends to do a slightly better job than
    `ld`.

##### With Patches

1. Setup patches:

    -`patches/ld/*.patch`

2. Usage(`GOLD_ORDERING_SCRIPT`) With Patches:

    - `export GOLD_ORDERING_SCRIPT=ordering.gold; gold ...`

3. Usage(`GOLD_ORDERING_SCRIPT_MAP`) With Patches:

    - `export GOLD_ORDERING_SCRIPT_MAP=ordering_map; gold ...`


### thin-layout-optimizer

1. **Collect a profile of the system using**.

    - `perf record -e cycles:u,branch-misses -j any,u -a`

2. **Package the result**. This will copy all the referenced DSOs + some
   debug file to a new `tar` file. `thin-layout-optimizer` will use the copied
   DSOs as its references (as opposed to the system ones which, if
   updated, will changed the addresses of functions and make the old
   data unusable).

   - `python3 scripts/package.py <input:perf.data file> <dst:packaged-profile>`

   `<dst>` will always be a `.tar.gz` file, even if you didn't specify `.tar.gz`

3. **Unpackage the results** when you are ready to create the function ordering(s).

    - `python3 scripts/unpackage.py <src:packaged-profile.tar.gz> <dst:unpackaged-profile dir>`

    This is really just a wrapped for untarring the `tar` created in Step 2.


   Note: Both the `package.py` and `unpackage.py` scripts accepts a
   3rd optional argumet `compress`. If this is provided it will run
   `perf` on the `perf.data` profile and compress the output (with
   `zstd`). Once you have the compressed `zst` files, it is fine to
   delete the `perf.data` file.

4. **Run `thin-layout-optimizer`**.

    - `thin-layout-optimizer -r <src:unpackaged-profile dir> -o <dst:dir-for-ordering-file> --save <dst:saved-state-file>`

    Alternatively, if you just want to save a profile to combine with other future profiles, or just to create ordering files later you can use:

    Note: There are more options (see `thin-layout-optimizer -h`). For the most
    part just using the above command should be all you need to do.

5. **(Optional) Save/Reload From Saved States**.
    - When running `thin-layout-optimizer` with a new `perf.data` profile, you can use the option `--save` to store the state just before call-graph creation. After creating a save-state, you can re-run `thin-layout-optimizer` using the `--reload` option to avoid the time-consuming task of processing the `perf.data` files. You can also combine multiple save-states with the option. For example:

    1. Saving a profile:
        - `thin-layout-optimizer -r <src:unpackaged-profile dir> --save <dst:saved-state-file>`

    2. Reloading save state
        - `thin-layout-optimizer --reload <src:saved-state-file> -o <dst:dir-for-ordering-file>`

    3. Reloading and combining multiple saved state
        - `thin-layout-optimizer --reload <src:saved-state-file0>,<src:saved-state-file1>,<src:saved-state-fileN> --save <dst:combined-save-state-file>`


6. **Finalize ordering for a target**.

   Note: This section assumes usage of the `ld`/`gold` patches with
   the `*_ORDERING_SCRIPT_MAP` env variable.

    - Once a set of ordering files have been created the final step is
      to create the linker scripts to be used for re-linking. Note the
      ordering files can either be from a `perf.data` file, save
      states, or a combination of the two.

    - The method for creating the `*_ORDERING_SCRIPT_MAP` and linker
      scripts from the ordering files is to use the
      `scripts/finalize-order.py` script.


    - `python3 scripts/finalize-order.py -i <src:dir-with-ordering-files> -t <'ld', 'ld.orig', or 'gold'> -o <dst:directory for linker scripts> -m <dst:map files for env variable> -p <perfix for paths in map file> --align-hot-n <float:0-100> --align-till <float:0-100> --alignment=<int:0-12> --align-per-dso --aliases <file:alias-file>`


        - The `-i` argument is the directory where you saved the ordering files (the `-o` argument from `thin-layout-optimizer`)
        - The `-m` argument will create the file you should set to `*_ORDERING_SCRIPT_MAP`.
        - The `-o` argument will be where the actual linker scripts
          are stored.
        - The `-p` argument will set prefix for where the directory
          with the scripts should be searched for by the
          `*_ORDERING_SCRIPT_MAP`. I.e if you use `-p ~/foobar`, the
          `*_ORDERING_SCRIPT_MAP` will contain entries like `<dso
          name> ~/foobar/<linker script for dso name>`. If the `-p`
          option isn't present it will default to what was used for
          `-o`.
        - The `--alignment` option specifies the log2 of the alignment
          we want. I.e `--alignment=5` (default) will align functions
          to 32 bytes. `--alignment=10` will likewise be 1024 byte
          alignment.
        - The `--align-hot-n` option species we want to add
          `--alignment` to the top N percentage of **called**
          functions. I.e if `--align-hot-n=1` and `--alignment=5`, we
          will align the functions that are in the 99% percentile of
          number of incoming calls to 32 bytes.
        - The `--align-till` option species we want to add
          `--alignment` to functions untill we account of N percentage
          of total calls.  I.e if `--align-till=33` and
          `--alignment=12`, we will keep aligning functions to 4096
          bytes until 33% of the total incoming edges have been
          accounted for.
        - The `--align-per-dso` option takes the above two constraints
          (`--align-hot-n` and `--align-till`), and instead of
          applying them globally, will apply them for each DSO. For
          example if we have two DSOs `A` and `B` with `A` containing
          99% of the total calls. If you **don't** specify
          `--align-per-dso`, then we will align functions in both
          `A`/`B` based on their global hotness (so likely no function
          from `B`). If you **do** specify `--align-per-dso`, we will
          apply both alignment constraints to both `A` and `B`
          regardless of their relative weight.
        - The `--aliases` option takes a file that maps one target to
          another. This is useful if the commandline profiled is
          different than the build target, or if you want to use the
          same ordering file for multiple targets. Note if the
          `--alias` argument is missing, it will try to find one at
          the path specified by the env variable
          `ORDERING_SCRIPT_ALIASES`.



    Note: An example from running this script when targeting llvm:
    - For `gold` (with an explicit prefix):

        - `python3 scripts/finalize-order.py -i orders/ -t gold -o llvm-orders/ -m llvm-orders-map.gold -p ~/.llvm-orders`

    - For `ld` (without an explicit prefix):

        - `python3 scripts/finalize-order.py -i orders/ -t ld -o ~/.llvm-orders/ -m llvm-orders-map.ld`

    By default the script will name the actual script files with a
    `.ld` or `.gold` extension based on the target, so scripts for
    `ld`/`gold` can safely be stored to the same directory.


    **NOTE**: By default finalize order will emit lines to find each function at:
    1. `.text.<func>`
    2. `.text.hot.<func>`
    3. `.text.cold.<func>`
    4. `.text.unlikely.<func>`

    To skip the `cold` and `unlikely` locations, pass `--no-cold` to `finalize-order.py`.

7. **Rebuild the project**.
    - For `ld` either pass the output `order-script.lds` to `ld` directly with the `-T` option, or if using the patched version set env `export LD_ORDERING_SCRIPT=/path/to/order-script.lds`
    - For `gold` you can use the builtin argument `--section-ordering-file`.

8. **Verify new order**.
    - To verify a new order, you can use
      `scripts/compare-order.py`. This will compare the ordering in a
      built binary, compare it with an ordering file, and produce a
      "score" for how close the binary and ordering file are. The
      lower the score, the closer the orders are.. **Note** the
      ordering file referenced here is not the output script used in
      the link stage, but the assosiated file from Step 4 in directory
      `<dir-for-ordering-file>`.

      `python3 scripts/compare-order.py <src:ordering file> <src:bin0> ... <src:binN>`

      I.e when using for LLVM:
      ```
      python3 scripts/compare-order.py orders-new-new/ordering--home-noah-programs-opensource-llvm-dev-src-project3-build-ld-bin-clang-18.txt /home/noah/programs/opensource/llvm-dev/src/project3/build-gold-order/bin/clang-18 /home/noah/programs/opensource/llvm-dev/src/project3/build-ld-order/bin/clang-18 /home/noah/programs/opensource/llvm-dev/src/project3/build-ld/bin/clang-18
        /home/noah/programs/opensource/llvm-dev/src/project3/build-gold-order/bin/clang-18 -> 12.72/12.72
        /home/noah/programs/opensource/llvm-dev/src/project3/build-ld-order/bin/clang-18 -> 68.423/68.423
        /home/noah/programs/opensource/llvm-dev/src/project3/build-no-order/bin/clang-18 -> 16963.985/16963.985
      ```

      Note: Generally there will be no difference between the two
      numbers, if there is its an indication that the profile was too
      minimal.

#### Comparing Benchmark Results
    To get an idea if new linker scripts are needed, based on new
    benchmark results, you can use the `compare-dir.py` script. This
    script will see how different the function orders in two
    directories are. Then, based on the result, you can decide if the
    results warrant creating new linker scripts.

1. **Get Orders for Benchmarks**:
    - `thin-layout-optimizer -r <src:unpackaged-profile dir benchmark0> -o <dst:dir-for-ordering-files0> --save <dst:saved-state-file0>`
    - `thin-layout-optimizer -r <src:unpackaged-profile dir benchmark1> -o <dst:dir-for-ordering-files1> --save <dst:saved-state-file1>`

2. **Compare Output Directories**:
    - `python3 scripts/compare-dir.py <dst:dir-for-ordering-files0> <dst:dir-for-ordering-files1>`

    This will the difference of all common and distinct DSOs in both
    order directores. As well as a summary at the end.

3. **Advanced DSO Matching**:
    By default `compare-dir.py` will match `.so` files across
    different versions (i.e match `foo.so.1.1` with `foo.so.1.2`). To
    disable this pass `--exact-version`.

4. **Common DSOs Only**:
   To only compare the common DSOs (skip the summary of DSOs that are
   not in both directories), use the `--ignore-distinct` option.


#### Advanced Benchmark Recombination
    By default, you reload multiple save-states the save states are
    all normalized. This ensure that longer running/shorter running
    (but equally important) benchmarks are both weighted the same when
    recombined.

1. **Normalization**:
    Normalization is on by default. To disable to you can use either
    `--no-normalize` or `--force-no-normalize`. The only difference
    between these two options, is that `--no-normalize` will fail if
    the reloaded save states have a mixed of already normalized /
    not-normalized files (its pretty easy to see why this is likely
    not desirable). The `--force-no-normalize` option will stil warn
    in this case, but won't kill the process. An example use might be:
    `thin-layout-optimizer --no-normalize --reload <src:saved-state-file0>,<src:saved-state-file1>,<src:saved-state-fileN> --save <dst:combined-save-state-file>`



2. **Scaling**:
    It may be desirable to scale certain benchmarks. I.e there may be
    a set of very important benchmarks, but also some less important
    ones that still provide meaningful coverage. To scale certain
    benchmarks (increase / decrease there importance) you can either
    use the `--add-scale` option when creating a save state, or
    manually edit a save state.

    - **Adding a Scale By Hand**:
        Underneath the hood, the save states a just a json file. To manually add a save state you can insert entries under the `"scaling"` field:
       ```
       "scaling": {
        "scale": <Some Double above 0.0>
       }
       ```
        Its possible the `"scaling"` field won't already exist in the
        save state. If thats the case, just add it.

        You might also see other options in the `"scaling"` field. For example:
       ```
       "scaling": {
        "edge_normalized": true,
        "func_normalized": true,
        "edge_scale" : 2.5,
        "func_scale" : 2.0
        },
        ```

        **The `"edge_normalized"` and `"func_normalized"` fields
        should never be modified by hand**. These are for tracking
        which files have been normalized.

        The `"edge_scale"` and `"func_scale"` options are equivilent
        to `"scale"` but with a bit more precision as to what they
        apply to. To scale only weights among edges, but not function
        sample rate, or vice versa, you can modify/insert these fields
        independently. **If the `"scale"` field exists, its value will
        override both `"func_scale"` and `"edge_scale"`**. For example
        with:

       ```
       "scaling": {
        "edge_normalized": true,
        "func_normalized": true,
        "edge_scale" : 2.5,
        "func_scale" : 2.0
        "scale" : 3.0
        },
        ```

        Both function samples and edge samples will be scaled by 3.0.


    - **Adding a Scale With The Commandline**:
        To add a scale to a single file, instead of modify by hand you can also do:
        `thin-layout-optimizer --no-normalize --add-scale <Some Double Above 0.0> --reload <src:saved-state-file0> --save <dst:saved-state-file0-with-assosiated-scale>`

        The `--no-normalize` is not strictly necessary, but will preserve the original values.

        Likewise, if you wish to combine multiple save states and assosiate a scale with the combined save state you can also do:
        `thin-layout-optimizer --add-scale <Some Double Above 0.0> --reload <src:saved-state-file0>,<src:saved-state-file1>,<src:saved-state-fileN>  --save <dst:saved-state-file0-N-with-assosiated-scale>`

        Whether you normalize is optional again, just keep in mind
        once a save state has been normalized, its original precise
        values will have been lost.

    **Using a Scale**:
        To actually scale a file, use the `--use-custom-scale` commandline option.
        `thin-layout-optimizer --no-normalize --reload --save <dst:saved-state-file0-with-assosiated-scale> --save <dst:saved-state-file0-scaled>`

        The `--use-custom-scale` option will check whats in the
        `"scaling"` field and use any `"scale"`, `"func_scale"`,
        and/or `"edge_scale"` values to multiply the relevant metrics.

        **NOTE: You <u>cannot</u> use both `--add-scale` and
        `--use-custom-scale` at the same time. `--add-scale` only
        assosiates a scale with the save state. It does not actually
        multiply the fields.**.

        Finally, you can invoke `--use-custom-scale` when recombining multiple save states:
        `thin-layout-optimizer --use-custom-scale --reload <src:saved-state-file0>,<src:saved-state-file1>,<src:saved-state-fileN>  --save <dst:saved-state-file0-N-scaled>`

        When combining multiple save states with `--use-custom-scale`,
        the scale will be ontop of normalization. As well, each
        save-state will use its own `"scaling"` field.
