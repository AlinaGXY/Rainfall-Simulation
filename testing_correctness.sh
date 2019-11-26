#!/bin/bash

for t in $(seq 1 100); do
    ./rainfall_pt 8 50 0.5 4096 measurement_4096x4096.in > result.txt
    python ./check.py 4096 measurement_4096x4096.out result.txt
done
