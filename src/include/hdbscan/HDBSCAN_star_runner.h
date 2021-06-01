#ifndef TEAM33_HDBSCAN_STAR_RUNNER_H
#define TEAM33_HDBSCAN_STAR_RUNNER_H

#include <cstddef>
#include <string>
#include <vector>

#include <distances/distance_calculator.h>


/**
 * @brief Simple class for storing input parameters.
 */
struct RunnerConfig {
    size_t num_points;
    size_t num_dimensions;
    std::string points_file;
    std::string constraints;
    std::string hierarchy_file;
    std::string tree_file;
    std::string visualization_file;
    std::string partition_file;
    std::string outlier_score_file;
    size_t num_neighbors;
    size_t min_cl_size;
    bool compact;
    std::string dist_function;
    std::string optimization_level;
    std::string mst_optimization_level;
    std::string compiler_flags;
};

/**
 * @brief Create a RunnerConfig from gflags
 *
 * @return RunnerConfig
 */
RunnerConfig RunnerConfigFromFlags();
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
    const std::string& mst_optimization_level
);

/**
 * @brief Get the Distance Calculator object, throws error if the function is not implemented
 *
 * @param func_name
 * @return DistanceCalculator
 */
DistanceCalculator GetDistanceCalculator(const std::string& func_name, const std::string& opt_level);

/**
 * @brief Runs the clustering algorithm
 *
 * @param config
 */
void HDBSCANRunner(RunnerConfig config);


#endif
