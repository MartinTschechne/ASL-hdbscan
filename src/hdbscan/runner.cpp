#include <hdbscan/HDBSCAN_star_runner.h>
#include <hdbscan/HDBSCAN_star.h>
#include <gflags/gflags.h>
#include <stdexcept>
#include <cassert>

#include <distances/distances.h>

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
DEFINE_string(optimization_level, "no_optimization", "Which optimization level to use. One of [no_optimization, symmetry, unroll, vectorise]");

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
        FLAGS_optimization_level
    };
}

DistanceCalculator GetDistanceCalculator(const std::string& func_name) {
    if(func_name == "euclidean") return EuclidianDistance;
    if(func_name == "cosine") return CosineSimilarity;
    if(func_name == "manhattan") return ManhattanDistance;
    if(func_name == "pearson") return PearsonCorrelation;
    if(func_name == "supremum") return SupremumDistance;

    throw std::invalid_argument("Distance function not supported");
}

void HDBSCANRunner(RunnerConfig config) {
    DistanceCalculator dist_fun = GetDistanceCalculator(config.dist_function);
    assert(config.hierarchy_file != "");
    assert(config.tree_file != "");
    assert(config.partition_file != "");
    assert(config.visualization_file != "");
    assert(config.outlier_score_file != "");
    assert(config.num_points > 0 && config.num_dimensions > 0);
    
    size_t num_points = config.num_points;
    size_t num_dimensions = config.num_dimensions;
    const double * const * data_set = ReadInDataSet(config.points_file, ',', num_points, num_dimensions);
    

    std::vector<Constraint> constraints;
    if(config.constraints != "") {
        constraints = ReadInConstraints(config.constraints);
    }

    CalculateCoreDistances_t calculate_core_distances_f = GetCalculateCoreDistancesFunction(config.optimization_level);
    const double* const core_distances = calculate_core_distances_f(data_set, config.num_neighbors, dist_fun, num_points, num_dimensions);

    UndirectedGraph mst = ConstructMST(data_set, core_distances, true, dist_fun, num_points, num_dimensions);
    mst.QuicksortByEdgeWeight();

    FreeDataset(data_set, num_points);

    double* point_noise_levels = new double[num_points];
    size_t* point_last_clusters = new size_t[num_points];

    Vector* clusters = new Vector;
    vector_init(clusters);

    ComputeHierarchyAndClusterTree(mst, config.min_cl_size, config.compact, 
        constraints, config.hierarchy_file, config.tree_file, ',', point_noise_levels, 
        point_last_clusters, config.visualization_file, clusters);

    bool inf_stability = PropagateTree(clusters);

    std::vector<size_t> prominent_clusters;
    FindProminentClusters(clusters, config.hierarchy_file, config.partition_file, 
        ',', num_points, inf_stability, prominent_clusters);

    std::vector<OutlierScore> outlier_scores;
    CalculateOutlierScores(clusters, point_noise_levels, num_points, point_last_clusters, 
        core_distances, config.outlier_score_file, ',', inf_stability, outlier_scores);

    delete[] point_last_clusters;
    delete[] point_noise_levels;
    delete[] core_distances;
}
