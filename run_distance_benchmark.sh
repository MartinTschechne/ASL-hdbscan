#!/usr/bin/env bash

for d_name in euclidean cosine manhattan pearson supremum;
do
  echo Run ${d_name}
  fname=benchmark_${d_name}.csv
  touch ${fname}
  echo NAME,NUM_POINTS,DIM,CYC >> ${fname}
  for N in {10..16}
  do
    for D in 2 4 8 16 32 64 128 256;
    do
      ./build/distance_benchmark $((2**${N})) ${D} ${d_name} >> ${fname}
    done
  done
  mv ${fname} build/benchmarking/
  python ./plotting/plot_distance_benchmark.py --csv_path build/benchmarking/${fname} --save_path build/benchmarking
done
