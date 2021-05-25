#!/usr/bin/env bash

mkdir -p build/output

for flag in 0 3
do
    ./build/benchmarking/hdbscan-O${flag} -points_file=data/data_set_large.csv -hierarchy_file=build/output/hierarchy.csv -tree_file=build/output/tree.csv -visualization_file=build/output/results.vis -partition_file=build/output/partition.csv -outlier_score_file=build/output/outlier_scores.csv -num_dimension=2 -num_points=5000
    mv measurements_runner.txt build/benchmarking/measurements_runner_O${flag}_dim_2.txt
    python plotting/plot_measurements.py build/benchmarking/measurements_runner_O${flag}_dim_2.txt ${flag} 2
    mv measurement.png build/benchmarking/measurements_runner_O${flag}_dim_2.png

    ./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv -hierarchy_file=build/output/hierarchy.csv -tree_file=build/output/tree.csv -visualization_file=build/output/results.vis -partition_file=build/output/partition.csv -outlier_score_file=build/output/outlier_scores.csv -num_dimension=32 -num_points=10000
    mv measurements_runner.txt build/benchmarking/measurements_runner_O${flag}_dim_32.txt
    python plotting/plot_measurements.py build/benchmarking/measurements_runner_O${flag}_dim_32.txt ${flag} 32
    mv measurement.png build/benchmarking/measurements_runner_O${flag}_dim_32.png
done
