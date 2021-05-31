#!/usr/bin/env bash

mkdir -p build/output

flag=0

echo O0
./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv \
    -hierarchy_file=build/output/hierarchy.csv \
    -tree_file=build/output/tree.csv \
    -visualization_file=build/output/results.vis \
    -partition_file=build/output/partition.csv \
    -outlier_score_file=build/output/outlier_scores.csv \
    -num_dimension=32 \
    -num_points=10000

mv measurements_runner.txt build/benchmarking/measurements_runner_O${flag}_dim_32.txt
python plotting/plot_measurements.py build/benchmarking/measurements_runner_O${flag}_dim_32.txt ${flag} 32
mv measurement.png build/benchmarking/measurements_runner_O${flag}_dim_32.png

flag=3

echo O3
./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv \
    -hierarchy_file=build/output/hierarchy.csv \
    -tree_file=build/output/tree.csv \
    -visualization_file=build/output/results.vis \
    -partition_file=build/output/partition.csv \
    -outlier_score_file=build/output/outlier_scores.csv \
    -num_dimension=32 \
    -num_points=10000 \
    -optimization_level=no_optimization

mv measurements_runner.txt build/benchmarking/measurements_runner_O${flag}_dim_32.txt
python plotting/plot_measurements.py build/benchmarking/measurements_runner_O${flag}_dim_32.txt ${flag} 32
mv measurement.png build/benchmarking/measurements_runner_O${flag}_dim_32.png
python plotting/plot_speedup.py build/benchmarking/measurements_runner_O${flag}_dim_32.txt build/benchmarking/measurements_runner_O0_dim_32.txt ${flag} 32
mv speedup.png build/benchmarking/speedup_O${flag}_dim_32.png

echo symmetry
./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv \
    -hierarchy_file=build/output/hierarchy.csv \
    -tree_file=build/output/tree.csv \
    -visualization_file=build/output/results.vis \
    -partition_file=build/output/partition.csv \
    -outlier_score_file=build/output/outlier_scores.csv \
    -num_dimension=32 \
    -num_points=10000 \
    -optimization_level=symmetry

mv measurements_runner.txt build/benchmarking/measurements_runner_symmetry_dim_32.txt
python plotting/plot_measurements.py build/benchmarking/measurements_runner_symmetry_dim_32.txt symmetry 32
mv measurement.png build/benchmarking/measurements_runner_symmetry_dim_32.png
python plotting/plot_speedup.py build/benchmarking/measurements_runner_symmetry_dim_32.txt build/benchmarking/measurements_runner_O0_dim_32.txt symmetry 32
mv speedup.png build/benchmarking/speedup_symmetry_dim_32.png

echo unroll2
./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv \
    -hierarchy_file=build/output/hierarchy.csv \
    -tree_file=build/output/tree.csv \
    -visualization_file=build/output/results.vis \
    -partition_file=build/output/partition.csv \
    -outlier_score_file=build/output/outlier_scores.csv \
    -num_dimension=32 \
    -num_points=10000 \
    -optimization_level=unroll2

mv measurements_runner.txt build/benchmarking/measurements_runner_unroll2_dim_32.txt
python plotting/plot_measurements.py build/benchmarking/measurements_runner_unroll2_dim_32.txt unroll2 32
mv measurement.png build/benchmarking/measurements_runner_unroll2_dim_32.png
python plotting/plot_speedup.py build/benchmarking/measurements_runner_unroll2_dim_32.txt build/benchmarking/measurements_runner_O0_dim_32.txt unroll2 32
mv speedup.png build/benchmarking/speedup_unroll2_dim_32.png


echo unroll4
./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv \
    -hierarchy_file=build/output/hierarchy.csv \
    -tree_file=build/output/tree.csv \
    -visualization_file=build/output/results.vis \
    -partition_file=build/output/partition.csv \
    -outlier_score_file=build/output/outlier_scores.csv \
    -num_dimension=32 \
    -num_points=10000 \
    -optimization_level=unroll4

mv measurements_runner.txt build/benchmarking/measurements_runner_unroll4_dim_32.txt
python plotting/plot_measurements.py build/benchmarking/measurements_runner_unroll4_dim_32.txt unroll4 32
mv measurement.png build/benchmarking/measurements_runner_unroll4_dim_32.png
python plotting/plot_speedup.py build/benchmarking/measurements_runner_unroll4_dim_32.txt build/benchmarking/measurements_runner_O0_dim_32.txt unroll4 32
mv speedup.png build/benchmarking/speedup_unroll4_dim_32.png


echo vectorise
./build/benchmarking/hdbscan-O${flag} -points_file=data/unit_tests/example_high_dim_10000_32.csv \
    -hierarchy_file=build/output/hierarchy.csv \
    -tree_file=build/output/tree.csv \
    -visualization_file=build/output/results.vis \
    -partition_file=build/output/partition.csv \
    -outlier_score_file=build/output/outlier_scores.csv \
    -num_dimension=32 \
    -num_points=10000 \
    -optimization_level=vectorise

mv measurements_runner.txt build/benchmarking/measurements_runner_vectorise_dim_32.txt
python plotting/plot_measurements.py build/benchmarking/measurements_runner_vectorise_dim_32.txt vectorize 32
mv measurement.png build/benchmarking/measurements_runner_vectorise_dim_32.png
python plotting/plot_speedup.py build/benchmarking/measurements_runner_vectorise_dim_32.txt build/benchmarking/measurements_runner_O0_dim_32.txt vectorise 32
mv speedup.png build/benchmarking/speedup_vectorise_dim_32.png
