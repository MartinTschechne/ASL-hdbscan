#!/usr/bin/env bash

fname=benchmark_core_distance.csv
touch ${fname}
echo NAME,NUM_POINTS,DIM,CYC >> ${fname}
for N in 1024 2048 4096 8192 16384 32768 65536;
do
  for D in 2 4 8 16 32 64 128 256 512;
  do
    ./build/cd_benchmark "--num_points" ${N} "--num_dims" ${D} >> ${fname}
  done
done
mv ${fname} build/benchmarking/
python ./plotting/plot_distance_benchmark.py --csv_path build/benchmarking/${fname} --save_path build/benchmarking
