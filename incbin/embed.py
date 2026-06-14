#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--input')
parser.add_argument('-o', '--output')
args = parser.parse_args()

values = []
with open(args.input, 'r') as f:
    values = list(map(float, f.read().split()))

with open(args.output, 'w') as f:
    print('#include <array>', file=f)
    print('namespace q_eval {', file=f)
    print('inline static const std::array<float,', len(values), '> MODEL_WEIGHTS = {', sep='', end='', file=f)
    for i in range(len(values)):
        if i > 0:
            print(',', end='', file=f)
        print(values[i], end='', file=f)
    print('};', file=f)
    print('}', file=f)
