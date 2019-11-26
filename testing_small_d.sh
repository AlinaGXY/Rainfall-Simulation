#!/bin/bash

threads="1 2 4 8"

d=4
echo "dimention = "$d
echo "seq:"
./rainfall_seq 1 10 0.25 4 sample_4x4.in > result.txt
python ./check.py $d sample_4x4.out result.txt
head -n3 result.txt
echo "parallel"
for t in $threads; do
    ./rainfall_pt $t 10 0.25 4 sample_4x4.in > result.txt
    head -n3 result.txt
done

d=16
echo "dimention = "$d
echo "seq:"
./rainfall_seq 1 20 0.5 16 sample_16x16.in > result.txt
python ./check.py $d sample_16x16.out result.txt
head -n3 result.txt
echo "parallel"
for t in $threads; do
    ./rainfall_pt $t 20 0.5 16 sample_16x16.in > result.txt
    python ./check.py $d sample_16x16.out result.txt
    head -n3 result.txt
done

d=32
echo "dimention = "$d
echo "seq:"
./rainfall_seq 1 20 0.5 32 sample_32x32.in > result.txt
python ./check.py $d sample_32x32.out result.txt
head -n3 result.txt
echo "parallel"
for t in $threads; do
    ./rainfall_pt $t 20 0.5 32 sample_32x32.in > result.txt
    python ./check.py $d sample_32x32.out result.txt
    head -n3 result.txt
done

d=128
echo "dimention = "$d
echo "seq:"
./rainfall_seq 1 30 0.25 128 sample_128x128.in > result.txt
python ./check.py $d sample_128x128.out result.txt
head -n3 result.txt
echo "parallel"
for t in $threads; do
    ./rainfall_pt $t 30 0.25 128 sample_128x128.in > result.txt
    python ./check.py $d sample_128x128.out result.txt
    head -n3 result.txt
done

d=512
echo "dimention = "$d
echo "seq:"
./rainfall_seq 1 30 0.75 512 sample_512x512.in > result.txt
python ./check.py $d sample_512x512.out result.txt
head -n3 result.txt
echo "parallel"
for t in $threads; do
    ./rainfall_pt $t 30 0.75 512 sample_512x512.in > result.txt
    python ./check.py $d sample_512x512.out result.txt
    head -n3 result.txt
done

d=2048
echo "dimention = "$d
echo "seq:"
./rainfall_seq 1 35 0.5 2048 sample_2048x2048.in > result.txt
python ./check.py $d sample_2048x2048.out result.txt
head -n3 result.txt
echo "parallel"
for t in $threads; do
    ./rainfall_pt $t 35 0.5 2048 sample_2048x2048.in > result.txt
    python ./check.py $d sample_2048x2048.out result.txt
    head -n3 result.txt
done