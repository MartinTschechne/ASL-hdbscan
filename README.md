# ASL21 
Project work for the course [Advanced Systems Lab](http://www.vvz.ethz.ch/Vorlesungsverzeichnis/lerneinheit.view?lang=en&lerneinheitId=151004&semkez=2021S&ansicht=KATALOGDATEN&), FS2021 @ETHZ

# Team members (alphabetical order)
* Mark Frey
* Beat Hubmann
* Martin Tschechne
* Robin Worreby

# Report
[Team 33 Final Report](/report/33_report.pdf)

# Code

## Building and running 
Create the build directory and run cmake and then make. The optimization level can be set from the command line using 
```
cmake -D CMAKE_CXX_FLAGS="-O3"
```
This will generate two files, `unit_tests` and `hdbscan`. For testing just execute `unit_tests`.  
`hdbscan` takes 6 required and 6 optional command line arguments. The minimal command to run is 
```
./hdbscan -points_file=../data/data_set_medium.csv -hierarchy_file=output/hierarchy.csv -tree_file=output/tree.csv -visualization_file=output/results.vis -partition_file=output/partition.csv -outlier_score_file=output/outlier_scores.csv -num_points 1000 -num_dimension 2
```
The optional arguments are  
* `constraints` path to the constraints csv 
* `min_pts` minimum number of points (default: 8)
* `min_cl_size` minimal cluster size (default: 8)
* `compact` Whether or not to compact the output (default: true) 
* `dist_function` Which distance function to use (default: euclidean)
* `optimization_level` Which optimization level to use (default: no_optimization)

This information can also be accessed by executing 
```
hdbscan -help
```

# Benchmarking
Different build variants (compiler optimizations) can be added in CMakeLists. To create a new build variant add 
```
build_hdbscan_benchmarking("-O3")
```
to CMakeLists and change `-O3` to whatever flag you want. All builds will be written to `build/benchmarking` with name `hdbscan<OPTIMIZATION FLAG>` (without <>). These executables can then e.g. be used by a python script to create the plots.

## Benchmark MST 
From the plotting folder execute
```
python3 plot_mst_benchmark.py --points_file=../data/unit_tests/example_high_dim_10000_32.csv --num_points=10000 --num_dimension=32 --recompute --output_name=results_10k --title_heatmap="Relative speed-up 10k points, 32 dimensions" --title_bar="Cycles by optimization, 10k points, 32 dimensions" --disable_nobitset --num_runs=2
```
Adapt the num_runs paramter to taste

# Benchmarking results

![final comparison](/results/comparison.png "Compared to sklearn-contrib high-perf HDBSCAN")
[Distances](/results/distances/)
[MST](/results/mst/)


# Literature
[Hierarchical Density Estimates for Data Clustering, Visualization, and Outlier Detection](https://dl.acm.org/doi/10.1145/2733381)  
[Accelerated Hierarchical Density Clustering](https://arxiv.org/abs/1705.07321)
