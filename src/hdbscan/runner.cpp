#include <hdbscan/HDBSCAN_star_runner.h>
#include <hdbscan/HDBSCAN_star.h>
#include <gflags/gflags.h>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <fstream>

#include <distances/distances.h>
#include <benchmark/tsc_x86.h>

DEFINE_int32(num_points, 0, "Number of points in the dataset");
DEFINE_int32(num_dimension, 0, "Number of dimensions in the dataset");
DEFINE_string(points_file, "", "Input points, csv format");
DEFINE_string(constraints, "", "Constraints file, optional, csv format");
DEFINE_string(hierarchy_file, "", "Where to write hierarchy to");
DEFINE_string(tree_file, "", "Where to write the tree to");
DEFINE_string(visualization_file, "", "Where to write the visualization to");
DEFINE_string(partition_file, "", "Where to write the partition to");
DEFINE_string(outlier_score_file, "", "Where to write the outlier scores to");
DEFINE_int32(num_neighbors, 8, "Nearest neighbors");
DEFINE_int32(min_cl_size, 8, "Minimum cluster size");
DEFINE_bool(compact, true, "Whether or not to compact the output");
DEFINE_string(dist_function, "euclidean", "Which metric to use. One of [euclidean, cosine, manhattan, pearson, supremum]");
DEFINE_string(mst_optimization_level, "no_optimization", "Which optimization to use for ConstructMST. CAUTION: xxx_nocalc only work when the CoreDistance function also returns the distance matrix! One of [no_optimization, bitset_unroll, bitset_nocalc, bitset_unroll_nocalc, bitset_nocalc_avx, bitset_nocalc_avx_unroll_2, bitset_nocalc_avx_unroll_4, nobitset_unroll, nobitset_unroll_nocalc, nobitset_nocalc_avx256, nobitset_nocalc_avx512]");
DEFINE_string(optimization_level, "no_optimization", "Which optimization level to use. One of [no_optimization, symmetry, unroll2, unroll4, vectorise]");
DEFINE_string(compiler_flags, "O0", "Which compiler flags are used. One of [O0, O3]");


RunnerConfig RunnerConfigFromFlags() {
    return {
        static_cast<size_t>(FLAGS_num_points),
        static_cast<size_t>(FLAGS_num_dimension),
        FLAGS_points_file,
        FLAGS_constraints,
        FLAGS_hierarchy_file,
        FLAGS_tree_file,
        FLAGS_visualization_file,
        FLAGS_partition_file,
        FLAGS_outlier_score_file,
        static_cast<size_t>(FLAGS_num_neighbors),
        static_cast<size_t>(FLAGS_min_cl_size),
        FLAGS_compact,
        FLAGS_dist_function,
        FLAGS_optimization_level,
        FLAGS_mst_optimization_level,
        FLAGS_compiler_flags
    };
}

RunnerConfig CreateRunnerConfig(
    size_t num_points,
    size_t num_dimensions,
    const std::string& points_file,
    const std::string& constraints, 
    const std::string& hierarchy_file, 
    const std::string& tree_file, 
    const std::string& vis_file,
    const std::string& part_file, 
    const std::string& outlier_score_file,
    size_t num_neighbors,
    size_t min_cluster_size,
    bool compact,
    const std::string& dist_function,
    const std::string& optimization_level,
    const std::string& mst_optimization_level,
    const std::string& compiler_flags
) {
    return {
        num_points,
        num_dimensions,
        points_file,
        constraints,
        hierarchy_file,
        tree_file,
        vis_file,
        part_file,
        outlier_score_file,
        num_neighbors,
        min_cluster_size,
        compact,
        dist_function,
        optimization_level,
        mst_optimization_level,
        compiler_flags
    };
}

DistanceCalculator GetDistanceCalculator(const std::string& func_name, const std::string& opt_level) {
    if (opt_level == "unroll2") {
        if(func_name == "euclidean") return EuclideanDistanceUnrolled;
        if(func_name == "cosine") return CosineSimilarityUnrolled;
        if(func_name == "manhattan") return ManhattanDistanceUnrolled;
        if(func_name == "pearson") return PearsonCorrelationUnrolled;
        if(func_name == "supremum") return SupremumDistanceUnrolled;
    } else if (opt_level == "unroll4") {
        if(func_name == "euclidean") return EuclideanDistance_4Unrolled;
        if(func_name == "cosine") return CosineSimilarity_4Unrolled;
        if(func_name == "manhattan") return ManhattanDistance_4Unrolled;
        if(func_name == "pearson") return PearsonCorrelation_4Unrolled;
        if(func_name == "supremum") return SupremumDistance_4Unrolled;
#ifdef __AVX2__
    } else if (opt_level == "vectorise") {
        if(func_name == "euclidean") return EuclideanDistance_Vectorized;
        if(func_name == "cosine") return CosineSimilarity_Vectorized;
        if(func_name == "manhattan") return ManhattanDistance_Vectorized;
        if(func_name == "pearson") return PearsonCorrelation_Vectorized;
        if(func_name == "supremum") return SupremumDistance_Vectorized;
#endif //__AVX2__
    } else {
        if(func_name == "euclidean") return EuclideanDistance;
        if(func_name == "cosine") return CosineSimilarity;
        if(func_name == "manhattan") return ManhattanDistance;
        if(func_name == "pearson") return PearsonCorrelation;
        if(func_name == "supremum") return SupremumDistance;
    }

    throw std::invalid_argument("Distance function not supported");
}

void HDBSCANRunner(RunnerConfig config) {
    DistanceCalculator dist_fun = GetDistanceCalculator(config.dist_function, config.optimization_level);
    assert(config.hierarchy_file != "");
    assert(config.tree_file != "");
    assert(config.partition_file != "");
    assert(config.visualization_file != "");
    assert(config.outlier_score_file != "");
    assert(config.num_points > 0 && config.num_dimensions > 0);

    size_t num_points = config.num_points;
    size_t num_dimensions = config.num_dimensions;
    double** data_set = ReadInDataSet(config.points_file, ',', num_points, num_dimensions);

    Vector* constraints = nullptr;
    if(config.constraints != "") {
        constraints = ReadInConstraints(config.constraints);
    }

    std::ofstream benchmark_total;
    benchmark_total.open("measurements_totals.txt", std::ios_base::app);
    // benchmark_total << "Data Points,Data Dimension,Distance Function,Optimization Flags,Compiler Flags,Total Cycles\n";
    benchmark_total << num_points << "," << num_dimensions << ","
                    << config.dist_function << "," << config.optimization_level
                    << "," << config.compiler_flags << ",";
    long int total_start = start_tsc();


    std::ofstream benchmark_runner;
    benchmark_runner.open("measurements_runner.txt");
    benchmark_runner << "# Benchmarks from file runner\n";
    benchmark_runner << "# Optimization flags: " << FLAGS_optimization_level << "\n";
    benchmark_runner << "# Compiler flags: " << "Unknown" << "\n";
    benchmark_runner << "# Dimensions of data set: " << num_dimensions << "\n";
    benchmark_runner << "Region,Cycles" << '\n';

    long int start = start_tsc();
    double** distance_matrix = nullptr;
    CalculateCoreDistances_t calculate_core_distances_f = GetCalculateCoreDistancesFunction(config.optimization_level);
    double* core_distances = calculate_core_distances_f(data_set, config.num_neighbors, dist_fun, num_points, num_dimensions, distance_matrix);
    long int cycles = stop_tsc(start);
    benchmark_runner << "calculate_distances," << cycles << "\n";

    start = start_tsc();
    ConstructMST_t construct_mst_f = GetConstructMSTFunction(config.mst_optimization_level);
    UndirectedGraph_C* mst = construct_mst_f(data_set, core_distances, true, dist_fun, num_points, num_dimensions, distance_matrix);
    cycles = stop_tsc(start);
    benchmark_runner << "construct_mst," << cycles << "\n";
    start = start_tsc();
    UDG_QuicksortByEdgeWeight(mst);
    cycles = stop_tsc(start);
    benchmark_runner << "mst_quicksort," << cycles << "\n";

    FreeDataset(data_set, num_points);

    double* point_noise_levels = (double*)malloc(sizeof(double)*num_points);
    size_t* point_last_clusters = (size_t*)malloc(sizeof(size_t)*num_points);

    Vector* clusters = vector_create();

    start = start_tsc();
    ComputeHierarchyAndClusterTree(mst, config.min_cl_size, config.compact,
        constraints, config.hierarchy_file, config.tree_file, ',', point_noise_levels,
        point_last_clusters, config.visualization_file, clusters);
    cycles = stop_tsc(start);
    benchmark_runner << "compute_hierarchy," << cycles << "\n";

    UDG_Free(mst);

    start = start_tsc();
    bool inf_stability = PropagateTree(clusters);
    cycles = stop_tsc(start);
    benchmark_runner << "propagate_tree," << cycles << "\n";

    vector* prominent_clusters = vector_create();

    start = start_tsc();
    FindProminentClusters(clusters, config.hierarchy_file, config.partition_file,
        ',', num_points, inf_stability, prominent_clusters);
    cycles = stop_tsc(start);
    benchmark_runner << "find_clusters," << cycles << "\n";

    vector* outlier_scores = vector_create();

    start = start_tsc();
    CalculateOutlierScores(clusters, point_noise_levels, num_points, point_last_clusters,
        core_distances, config.outlier_score_file, ',', inf_stability, outlier_scores);
    cycles = stop_tsc(start);
    benchmark_runner << "calculate_outliers," << cycles << "\n";

    benchmark_runner.close();

    long int total_cycles = stop_tsc(total_start);
    benchmark_total << cycles << "\n";
    benchmark_total.close();

    free(point_last_clusters);
    free(point_noise_levels);
    free(core_distances);
}
