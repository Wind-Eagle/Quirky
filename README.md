# Quirky

## Overview
Quirky is a simple chess engine, supporting UCI protocol. Approximate CCRL 40/15 rating is around 3350.

The engine requires AVX2/BMI2 support to run, so make sure your processor supports these instruction sets.

## Building from source
The project uses [CMake](https://cmake.org) as a build system. You will also need a C++20-compatible compiler.

To build the engine, do the following:

~~~~~
$ mkdir build
$ cd build
$ cmake ..
$ make -j8
~~~~~

## Technical information
Quirky is based on alpha-beta pruning with various well-known heuristics and NNUE evaluation. It doesn't use autotuning of search constants, and aims to use as few complex and obscure heuristics as possible.

NNUE architecture is (768 -> 256) x2 -> 16 -> 32 -> 1. It was trained in the following way:
- Numerous self-play games of Quirky 1.0 were recorded. The time control was set to 20ms per move.
- From each game, we randomly selected 32 positions with White to move and 32 positions with Black to move (or fewer if the game was too short).
- We filtered out positions that were in check and positions where a simple quiescence search was able to find a winning capture.
- We then used Stockfish with WDL scores to analyze each position for 100 ms.
- Each entry for model learning consisted of a FEN string and a result (loss, draw, or win) sampled according to the WDL scores.

[Dataset used for Quirky 1.0 NNUE](https://bitbucket.org/zurichess/tuner/downloads)
[Dataset used for Quirky 2.0 NNUE](https://disk.yandex.ru/d/jcXiqAqF97RXeQ)

## Thanks
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page), for many useful articles
  and descriptions of various useful heuristics.
- [alex65536](https://github.com/alex65536) for providing useful chess engine tools.

Some code logic and architecture are derived from:
- [Albatross](https://github.com/Wind-Eagle/Albatross)
- [SoFCheck](https://github.com/alex65536/sofcheck)

Other engines used for inspiration:
- [Stockfish](https://github.com/official-stockfish/Stockfish/tree/master/src)
- [Berserk](https://github.com/jhonnold/berserk/tree/main)
- [Avalanche](https://github.com/SnowballSH/Avalanche)
- [Simbelmyne](https://github.com/sroelants/simbelmyne/tree/main)
- [Belette](https://github.com/vincentbab/Belette/tree/main)
