#ifndef TEAM33_HDBSCAN_STAR_RUNNER_H
#define TEAM33_HDBSCAN_STAR_RUNNER_H

#include <cstddef>
#include <string>
#include <vector>

#include <distances/distance_calculator.h>

/**
 * @brief Entry point for the HDBSCAN* algorithm.
 */
class HDBSCANStarRunner {
private:
    inline static const std::string file_flag{"file="};
    inline static const std::string constraints_flag{"constraints="};
    inline static const std::string min_pts_flag{"min_pts="};
    inline static const std::string min_cl_size_flag{"min_cl_size"};
    inline static const std::string compact_flag{"compact="};
    inline static const std::string distance_function_flag{"dist_function="};

    inline static const std::string euclidean_distance{"euclidean"};
    inline static const std::string cosine_similarity{"cosine"};
    inline static const std::string pearson_correlation{"pearson"};
    inline static const std::string manhattan_distance{"manhattan"};
    inline static const std::string supremum_distance{"supremum"};

public:
    /**
	 * @brief Runs the HDBSCAN* algorithm given an input data set file and a 
     * value for minPoints and minClusterSize.  Note that the input file must 
     * be a comma-separated value (CSV) file, and that all of the output files 
     * will be CSV files as well.  The flags "file=", "minPts=", "minClSize=", 
     * "constraints=", and "distance_function=" should be used to specify
     * the input data set file, value for minPoints, value for minClusterSize, 
     * input constraints file, and the distance function to use, respectively.
     * 
	 * @param args The input arguments for the algorithm
	 */
	void Run(std::vector<std::string> args);

private:
    /**
	 * @brief Parses out the input parameters from the program arguments.
     * Prints out a help message and exits the program if the parameters are 
     * incorrect.
     * 
     * @param args The input arguments for the program
	 * @return Input parameters for HDBSCAN*
	 */
	static HDBSCANStarParameters checkInputParameters(std::vector<std::string>
        args);

	/**
	 * @brief Prints a help message that explains the usage of 
     * HDBSCANStarRunner, and then exits the program.
	 */
	static void PrintHelpMessageAndExit();
};

/**
 * @brief Simple class for storing input parameters.
 */
class HDBSCANStarParameters {
    public:
        std::string input_file;
        std::string constraints_file;
        size_t min_points;
        size_t min_cluster_size;
        bool compact_hierarchy;
        DistanceCalculator distance_function;

        std::string hierarchy_file;
        std::string cluster_tree_file;
        std::string partition_file;
        std::string outlier_score_file;
        std::string visualization_file;
};

#endif
