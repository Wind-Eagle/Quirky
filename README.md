# Quirky

## Overview
Quirky is a simple chess engine, supporting UCI protocol. Approximate ELO rating is somewhere near 3000.

## Building from source
The project is using [CMake](https://cmake.org) as a build system. You will also need a C++20-compatible compiler.

To build the engine, do the following:

~~~~~
$ mkdir build
$ cd build
$ cmake ..
$ make -j8
~~~~~

To build the engine without BMI2 and AVX2 instruction sets, use the following command instead of simple cmake:
~~~~~
$ cmake -DNO_BMI2=ON -DNO_AVX2=ON ..
~~~~~

## Thanks
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page), for many useful articles
  and descriptions of various useful heuristics.
- [alex65536](https://github.com/alex65536) for providing [useful chess engine tools](https://github.com/alex65536/sofcheck-engine-tester)
- [Dataset used for model tuning](https://bitbucket.org/zurichess/tuner/downloads)

Some code logic and architecture are derived from:
- [Albatross](https://github.com/Wind-Eagle/Albatross)
- [SoFCheck](https://github.com/alex65536/sofcheck)

Other engines used for inspiration:
- [Stockfish](https://github.com/official-stockfish/Stockfish/tree/master/src)
- [Berserk](https://github.com/jhonnold/berserk/tree/main)
- [Belette](https://github.com/vincentbab/Belette/tree/main)
- [Simbelmyne](https://github.com/sroelants/simbelmyne/tree/main)
