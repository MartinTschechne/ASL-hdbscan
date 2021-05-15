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
    std::string points_file;
    std::string constraints;
    std::string hierarchy_file;
    std::string tree_file;
    std::string visualization_file;
    std::string partition_file;
    std::string outlier_score_file;
    size_t min_pts; 
    size_t min_cl_size;
    bool compact;
    std::string dist_function;
    std::string optimization_level;
};

/**
 * @brief Create a RunnerConfig from gflags
 * 
 * @return RunnerConfig 
 */
RunnerConfig RunnerConfigFromFlags();

/**
 * @brief Get the Distance Calculator object, throws error if the function is not implemented
 * 
 * @param func_name 
 * @return DistanceCalculator 
 */
DistanceCalculator GetDistanceCalculator(const std::string& func_name);

/**
 * @brief Runs the clustering algorithm
 * 
 * @param config 
 */
void HDBSCANRunner(RunnerConfig config);


#endif
