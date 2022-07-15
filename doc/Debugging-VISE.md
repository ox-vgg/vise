# Debugging VISE

## Using GDB

VISE can be compiled in debugging model which removes
optimisations and allows users to inspect the gdb back trace with more
detailed information. Note that when VISE is compiled in debugging
mode, the SIFT based feature extraction code fails. Therefore, at
present, the debugging mode can only be used for debugging an already
indexed projects.

First, we need to update settings such that the `gdb` debugger saves
the core (i.e. context of execution just befor it crashed) of a
program after crash.

```
ulimit -c
ulimit -c unlimited
cat /proc/sys/kernel/core_pattern  # show the location of core files
```

Next, we compile VISE in debug mode as follows.

```
cd $VISE_CODE/vise/cmake_build

$VISE_DEP/bin/cmake \
  -DCMAKE_C_FLAGS_DEBUG="-g -O0" \
  -DCMAKE_CXX_FLAGS_DEBUG="-g -O0" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH=$VISE_DEP \
  ../src

make -j
```

Next, we run VISE using `gdb` debugger.

```
echo "run" > gdb_run_cmd.txt

gdb --command gdb_run_cmd.txt
  --args $VISE_CODE/vise/cmake_build/vise/vise-cli \
  --cmd=serve-project \
  --http-www-dir=$VISE_CODE/vise/src/www/ \
  --http-address=0.0.0.0 \
  --http-port=9669 \
  --http-worker=8 \
  --http-namespace=/ \
  oxford-buildings:/data/vggdemos/vise/www/vise/oxford-buildings/data/conf.txt
```

Now, run the VISE software as usual. When the VISE server crashes, we
can capture the back trace showing the issue as follows.

```
...
terminate called after ...
...
(gdb) bt
#0  0x00007ffff54cea9f in raise () from /lib64/libc.so.6
#1  0x00007ffff54a1e05 in abort () from /lib64/libc.so.6
#2  0x00007ffff5e6f09b in __gnu_cxx::__verbose_terminate_handler() [clone .cold.1] () from /lib64/libstdc++.so.6
#3  0x00007ffff5e7553c in __cxxabiv1::__terminate(void (*)()) () from /lib64/libstdc++.so.6
#4  0x00007ffff5e75597 in std::terminate() () from /lib64/libstdc++.so.6
#5  0x00007ffff5e757f8 in __cxa_throw () from /lib64/libstdc++.so.6
...
```

The backtrack is very useful in understanding the source of the
crash. One can get additional details about each frame
(i.e. #frame-id) can be shows as follows.

```
frame 9      # show details about a frame shown in backtrace
info locals  # show local variables for the current frame
```

If you encounter a software crash and that the crash is reproducible,
then please share the detailed backtrace of the crash. If the core
dump file is available, please [share](mailto:adutta@robots.ox.ac.uk)
it as well.

## References
  * [GDB - Core Dumps](https://www.cse.unsw.edu.au/~learn/debugging/modules/gdb_coredumps/)




***

Contact [Abhishek Dutta](mailto:adutta@robots.ox.ac.uk) for queries and feedback related to this page.