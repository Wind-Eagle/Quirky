# Quirky

## Overview
Quirky is a chess project, aiming at creating tools for analyzing chess games. Currently there is only an engine, but much more is planned.

### Engine
Quirky engine has estimated strength of ~2200 Elo, but there is still much space for improvement. It can be loaded into GUI, but supports only UCI protocol for now. Current version is beta-1.

## Building from source
The project is using [CMake](https://cmake.org) as a build system. You will also need a C++20-compatible compiler.

The following dependencies are optional:
- [_Google Test_](https://github.com/google/googletest/): to run the unit tests
- [_Google Benchmark_](https://github.com/google/benchmark): to run benchmarks

To build the engine, do the following:

~~~~~
$ mkdir build
$ cd build
$ cmake ..
$ make -j8
~~~~~

## Thanks
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page), for many useful articles
  and descriptions of various useful heuristics.
- [alex65536](https://github.com/alex65536) for providing [useful chess engine tools](https://github.com/alex65536/sofcheck-engine-tester)
