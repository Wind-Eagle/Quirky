# Quirky

## Overview
Quirky is a simple chess engine, supporting UCI protocol.

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

NNUE architecture is (768 -> 512) x2 -> 16 -> 32 -> 1. It was trained in the following way:
- Numerous self-play games were played. From this games we randomly sampled positions that were evaluated during search.
  - For Quirky 2.0 we used Quirky 1.0 as engine generating positions. The time control was set to 20ms per move.
  - For Quirky 2.1 we used six slightly different Quirky 1.0 variants as engines generating positions. The time control was set to 20ms per move.
  - For Quirky 3.0, we used a variant of Quirky 2.0 with slightly randomized evaluation as the engine for generating positions. Random values are added to each root node move score, which leads to different moves being played from the same position. The time control was set to 50ms per move.
- From each game, we randomly selected *n* positions evaluated with the first engine and *n* positions evaluated with the second engine.
  - For Quirky 2.1 and older, *n* = 32. For Quirky 3.0, *n* = 64.
- We filtered out positions that were in check and positions where a simple quiescence search was able to find a winning capture.
- We then used Stockfish with WDL scores to analyze each position for 100 ms.
- Each entry for model learning consisted of a FEN string and a result (loss, draw, or win) sampled according to the WDL scores.

Quirky 1.0 NNUE was trained using a third-party dataset.

[Dataset used for Quirky 1.0 NNUE](https://bitbucket.org/zurichess/tuner/downloads)

[Dataset used for Quirky 2.0 NNUE](https://disk.yandex.ru/d/jcXiqAqF97RXeQ)

[Dataset used for Quirky 2.1 NNUE](https://disk.yandex.ru/d/O6d1VJuWoj1Wgg)

[Dataset used for Quirky 3.0 NNUE](https://disk.yandex.ru/d/b8JZIX9TYR97qw)

## Thanks
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page), for many useful articles
  and descriptions of various useful heuristics.
- [alex65536](https://github.com/alex65536) for providing useful chess engine tools.

Some code logic and architecture are derived from:
- [Albatross](https://github.com/Wind-Eagle/Albatross)
- [SoFCheck](https://github.com/alex65536/sofcheck)

Other engines used for inspiration:
- [Stockfish](https://github.com/official-stockfish/Stockfish)
- [Obsidian](https://github.com/gab8192/Obsidian)
- [Berserk](https://github.com/jhonnold/berserk)
- [Avalanche](https://github.com/SnowballSH/Avalanche)
- [Simbelmyne](https://github.com/sroelants/simbelmyne)
- [Belette](https://github.com/vincentbab/Belette)
