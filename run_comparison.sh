#!/usr/bin/env bash

data=data/large_50000_64.csv
labels=data/large_50000_64_labels.csv
ours=build/benchmarking/measurements_runner_bitset_nocalc_avx.txt
CPU_FREQ=2.8


python python_HDBSCAN_benchmark/python_HDBSCAN_benchmark.py -d ${data} -l ${labels} -m euclidean > out.txt
# optional for report: include Java reference
python plotting/plot_comparison.py --python_reference out.txt --ours ${ours} --cpu_freq ${CPU_FREQ}
rm out.txt
