#!/usr/bin/env bash

mkdir -p build/output

### deafault args
default_args="-points_file=data/large_50000_64.csv
-hierarchy_file=build/output/hierarchy.csv \
-tree_file=build/output/tree.csv \
-visualization_file=build/output/results.vis \
-partition_file=build/output/partition.csv \
-outlier_score_file=build/output/outlier_scores.csv \
-dist_function=euclidean \
-num_dimension=64 \
-num_points=50000"

num_dim=64

echo Baseline
flag=0
tic=$(date +%s.%N)
likwid-perfctr -g FLOPS_DP -C S0:0 -o likwid_baseline.txt -m ./build/benchmarking/hdbscan-O${flag} \
    ${default_args} \
    -compiler_flags=O${flag} \
    -optimization_level=no_optimization \
    -mst_optimization_level=no_optimization
toc=$(date +%s.%N)
diff=$(echo "$toc - $tic" | bc)
echo Execution took $diff s
out_file=build/benchmarking/measurements_runner_O0
mv measurements_runner.txt ${out_file}.txt
python plotting/plot_measurements.py ${out_file}.txt "-O${flag}"
mv measurement.png ${out_file}.png

base_file=${out_file}.txt
flag=3

echo Benchmark Core Distances
for opt_level in no_optimization symmetry unroll2 unroll4 vectorise vectorise_fma full
do
    echo ${opt_level}
    tic=$(date +%s.%N)
    likwid-perfctr -g FLOPS_DP -C S0:0 -o likwid_dist_${opt_level}.txt -m ./build/benchmarking/hdbscan-O${flag} \
        ${default_args} \
        -compiler_flags=O${flag} \
        -optimization_level=${opt_level} \
        -mst_optimization_level=no_optimization
    toc=$(date +%s.%N)
    diff=$(echo "$toc - $tic" | bc)
    echo Execution took $diff s
    out_file=build/benchmarking/measurements_runner_${opt_level}
    png_file=build/benchmarking/speedup_${opt_level}.png
    mv measurements_runner.txt ${out_file}.txt
    python plotting/plot_measurements.py ${out_file}.txt "-O${flag} + ${opt_level}"
    mv measurement.png ${out_file}.png
    python plotting/plot_speedup.py ${out_file}.txt ${base_file} "-O${flag} + ${opt_level}"
    mv speedup.png ${png_file}
done


echo Benchmark Construct MST
for opt_level in bitset_unroll bitset_nocalc bitset_unroll_nocalc bitset_nocalc_avx bitset_nocalc_avx_unroll_2 bitset_nocalc_avx_unroll_4
do
    echo ${opt_level}
    tic=$(date +%s.%N)
    likwid-perfctr -g FLOPS_DP -C S0:0 -o likwid_mst_${opt_level}.txt -m ./build/benchmarking/hdbscan-O${flag} \
        ${default_args} \
        -compiler_flags=O${flag} \
        -optimization_level=full \
        -mst_optimization_level=${opt_level}
    toc=$(date +%s.%N)
    diff=$(echo "$toc - $tic" | bc)
    echo Execution took $diff s
    out_file=build/benchmarking/measurements_runner_${opt_level}
    png_file=build/benchmarking/speedup_${opt_level}.png
    mv measurements_runner.txt ${out_file}.txt
    python plotting/plot_measurements.py ${out_file}.txt "-O${flag} + ${opt_level}"
    mv measurement.png ${out_file}.png
    python plotting/plot_speedup.py ${out_file}.txt ${base_file} "-O${flag} + ${opt_level}"
    mv speedup.png ${png_file}
done
