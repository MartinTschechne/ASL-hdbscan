#ifndef TEAM33_HDBSCAN_STAR_H
#define TEAM33_HDBSCAN_STAR_H

#include <cstddef>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <sstream>

#include <distances/distance_calculator.h>
#include <hdbscan/cluster.h>
#include <hdbscan/constraint.h>
#include <hdbscan/_C_undirected_graph.h>
#include <common/ordered_set.h>
#include <common/memory.h>

/**
 * Implementation of the HDBSCAN* algorithm, which is broken into several
 * methods.
 */

inline const std::string warning_message{ "----------------------------------------------- WARNING "
"-----------------------------------------------\n"
"With your current settings, the K-NN density estimate is discontinuous "
"as it is not well-defined\n"
"(infinite) for some data objects, either due to replicates in the data "
"(not a set) or due to numerical\n"
"roundings. This does not affect the construction of the density-based "
"clustering hierarchy, but\n"
"it affects the computation of cluster stability by means of relative "
"excess of mass. For this reason,\n"
"the post-processing routine to extract a flat partition containing the "
"most stable clusters may\n"
"produce unexpected results. It may be advisable to increase the value of "
"MinPts and/or M_clSize.\n"
"--------------------------------------------------------------------------""-----------------------------"};

struct OutlierScore {
    double score;
    double core_distance;
    size_t id;
};

int CompareTo(const OutlierScore& a, const OutlierScore& b);
int CompareOutlierScores(const void* a, const void* b);

typedef double*(*CalculateCoreDistances_t)(const double*, size_t,
    DistanceCalculator, const size_t, const size_t, double**& distance_matrix);

typedef UndirectedGraph_C*(*ConstructMST_t)(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

/**
 * @brief Reads in the input data set from the file given, assuming the
 * delimiter separates attributes for each data point, and each point is
 * given on a separate line.  Error messages are printed if any part of the
 * input is improperly formatted.
 *
 * @param file_name The path to the input file
 * @param delimiter A regular expression string that separates the
 * attributes of each point
 * @return A double[][] where index [i][j] indicates the jth attribute of
 * data point i
 */
double* ReadInDataSet(std::string const& file_name, const char delimiter, const size_t num_points, const size_t point_dimension, bool transpose=false);

void FreeDataset(double* dataset, size_t num_points);

/**
 * @brief Reads in constraints from the file given, assuming the delimiter
 * separates the points involved in the constraint and the type of the
 * constraint, and each constraint is given on a separate line.  Error
 * messages are printed if any part of the input is improperly formatted.
 *
 * @param file_name The path to the input file
 * @param delimiter A regular expression string that separates the points
 * and type of each constraint
 * @return A vector of Constraints
 */
Vector* ReadInConstraints(
    std::string const& file_name);

inline void FreeDistanceMatrix(double** mat, size_t num_points) {
    for(size_t i = 0; i < num_points; ++i) {
        free(mat[i]);
    }
    free(mat);
}

CalculateCoreDistances_t GetCalculateCoreDistancesFunction(const std::string& optimization_level);

inline void CoreDistanceFromDistanceMatrix(size_t num_points, size_t num_dimensions, size_t num_neighbors, double** distance_matrix, double* result){
    double* knn_distances = CreateAlignedDouble1D(num_neighbors);
    for(size_t point = 0; point < num_points; ++point) {
        distance_matrix[point][point] = __DBL_MAX__;
        for(size_t i = 0; i < num_neighbors; ++i) {
            knn_distances[i] = __DBL_MAX__;
        }
        for(size_t neighbor = 0; neighbor < num_points; ++neighbor) {
            if(point > neighbor) {
                distance_matrix[point][neighbor] = distance_matrix[neighbor][point];
            }

            double distance = distance_matrix[point][neighbor];
            //Check at which position in the nearest distances the current distance would fit:
            size_t neighbor_index = num_neighbors;
            while(neighbor_index >= 1 && distance < knn_distances[neighbor_index-1]) {
                neighbor_index--;
            }

            //Shift elements in the array to make room for the current distance:
            if(neighbor_index < num_neighbors) {
                for(size_t shift_index = num_neighbors - 1; shift_index > neighbor_index; shift_index--) {
                    knn_distances[shift_index] = knn_distances[shift_index - 1];
                }
                knn_distances[neighbor_index] = distance;
            }
        }
        result[point] = knn_distances[num_neighbors-1];
    }
    free(knn_distances);
}

/**
 * @brief  Calculates the core distances for each point in the data set,
 * given some value for k.
 *
 * @param dataSet A double[][] where index [i][j] indicates the jth
 * attribute of data point i
 * @param k Each point's core distance will be it's distance to the kth
 * nearest neighbor
 * @param distance_function A DistanceCalculator to compute distances
 * between points
 * @return A vector of core distances
 */
double* CalculateCoreDistancesNoOptimization(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix);

    /**
 * @brief  Symmetry optimized version of CalculateCoreDistances.
 *
 * @param dataSet A double[][] where index [i][j] indicates the jth
 * attribute of data point i
 * @param k Each point's core distance will be it's distance to the kth
 * nearest neighbor
 * @param distance_function A DistanceCalculator to compute distances
 * between points
 * @return A vector of core distances
 */
double* CalculateCoreDistancesSymmetry(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix);

double* CalculateCoreDistancesBlocked_Euclidean(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix);

double* CalculateCoreDistancesBlocked_Euclidean_With_Size(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix, size_t block_size);

double* CalculateCoreDistancesBlocked_Euclidean_Adaptive(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix);

double* CalculateCoreDistancesSymmetry_blocked(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix);

double* CalculateCoreDistancesBlocked_Euclidean_Transpose(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix);

ConstructMST_t GetConstructMSTFunction(const std::string& optimization_level);
/**
 * @brief Constructs the minimum spanning tree of mutual reachability
 * distances for the data set, given the core distances for each point.
 *
 * @param data_set A double[][] where index [i][j] indicates the jth
 * attribute of data point i
 * @param core_distances An array of core distances for each data point
 * @param self_edges If each point should have an edge to itself with
 * weight equal to core distance
 * @param distance_function A DistanceCalculator to compute distances between points
 * @return An UndirectedGraph containing the MST for the data set using the
 * mutual reachability distances
 */
UndirectedGraph_C* ConstructMST(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Unrolled_Bitset(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Bitset_NoCalc(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Unrolled_Bitset_NoCalc(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Bitset_NoCalc_AVX(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Bitset_NoCalc_AVX_Unrolled_2(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Bitset_NoCalc_AVX_Unrolled_4(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Unrolled_NoBitset_CalcDistances(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_Unrolled_NoBitset_NoCalcDistances(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_NoBitset_NoCalcDistances_AVX256(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

UndirectedGraph_C* ConstructMST_NoBitset_NoCalcDistances_AVX512(const double* data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension, double** distance_matrix);

/**
 * @brief Computes the hierarchy and cluster tree from the minimum spanning
 * tree, writing both to file, and returns the cluster tree.  Additionally,
 * the level at which each point becomes noise is computed.  Note that the
 * minimum spanning tree may also have self edges (meaning it is not a true
 * MST).
 *
 * @param mst A minimum spanning tree which has been sorted by edge weight
 * in descending order
 * @param min_cluster_size The minimum number of points which a cluster
 * needs to be a valid cluster
 * @param compact_hierarchy Indicates if hierarchy should include all
 * levels or only levels at which clusters first appear
 * @param constraints An optional vector of Constraints to calculate
 * cluster constraint satisfaction
 * @param hierarchy_output_file The path to the hierarchy output file
 * @param tree_output_file The path to the cluster tree output file
 * @param delimiter The delimiter to be used while writing both files
 * @param point_noise_levels A double[] to be filled with the levels at
 * which each point becomes noise
 * @param point_last_clusters An size_t[] to be filled with the last label
 * each point had before becoming noise
 * @return The cluster tree
 */
void ComputeHierarchyAndClusterTree(
    UndirectedGraph_C* mst, size_t min_cluster_size, bool compact_hierarchy,
    const Vector* const constraints, std::string hierarchy_output_file,
    std::string tree_output_file, const char delimiter,
    double* point_noise_levels, size_t* point_last_clusters,
    std::string visualization_output_file, Vector* result);

/**
 * @brief Propagates constraint satisfaction, stability, and lowest child
 * death level from each child cluster to each parent cluster in the tree.
 * This method must be called before calling FindProminentClusters() or
 * CalculateOutlierScores().
 *
 * @param clusters A vector of Clusters forming a cluster tree
 * @return true if there are any clusters with infinite stability, false
 * otherwise
 */
bool PropagateTree(const Vector* const  clusters);

/**
 * @brief Produces a flat clustering result using constraint satisfaction
 * and cluster stability, and returns an array of labels. PropagateTree()
 * must be called before calling this method.
 *
 * @param clusters A list of Clusters forming a cluster tree which has
 * already been propagated
 * @param hierarchy_file The path to the hierarchy input file
 * @param flat_output_file The path to the flat clustering output file
 * @param delimiter The delimiter for both files
 * @param num_points The number of points in the original data set
 * @param infinite_stability True if there are any clusters with infinite
 * stability, false otherwise
 * @return An array of labels for the flat clustering result
 */
void FindProminentClusters(const Vector* const clusters,
    const std::string& hierarchy_file, const std::string& flat_output_file,
    const char delimiter, size_t num_points, bool infinite_stability,
    vector* result);

/**
 * @brief Produces the outlier score for each point in the data set, and
 * returns a sorted list of outlier scores. PropagateTree() must be called
 * before calling this method.
 *
 * @param clusters A list of Clusters forming a cluster tree which has
 * already been propagated
 * @param point_noise_levels A double[] with the levels at which each point
 * became noise
 * @param point_last_clusters A size_t[] with the last label each point had
 * before becoming noise
 * @param core_distances An array of core distances for each data point
 * @param outlier_scores_output_file The path to the outlier scores output
 * file
 * @param delimiter The delimiter for the output file
 * @param infinite_stability True if there are any clusters with infinite
 * stability, false otherwise
 * @return An vector of OutlierScores, sorted in descending order
 */
void CalculateOutlierScores(
    const Vector* const clusters, double* point_noise_levels, size_t point_noise_levels_length,
    size_t* point_last_clusters, const double* core_distances,
    const std::string& outlier_scores_outputFile, const char delimiter,
    bool infinite_stability, vector* result);

/**
 * @brief Removes the set of points from their parent Cluster, and creates
 * a new Cluster, provided the clusterId is not 0 (noise).
 *
 * @param points The set of points to be in the new Cluster
 * @param cluster_labels An array of cluster labels, which will be modified
 * @param parent_cluster The parent Cluster of the new Cluster being created
 * @param cluster_label The label of the new Cluster
 * @param edge_weight The edge weight at which to remove the points from
 * their previous Cluster
 * @return The new Cluster, or null if the clusterId was 0
 */
Cluster* CreateNewCluster(const OrderedSet* const points,
    size_t* cluster_labels, Cluster* parent_cluster, size_t cluster_label,
    double edge_weight);

/**
 * @brief Calculates the number of constraints satisfied by the new
 * clusters and virtual children of the parents of the new clusters.
 *
 * @param new_cluster_labels Labels of new clusters
 * @param clusters A vector of clusters
 * @param constraints A vector of constraints
 * @param cluster_labels An array of current cluster labels for points
 */
void CalculateNumConstraintsSatisfied(
    const OrderedSet* const new_cluster_labels, const Vector* const clusters, const Vector* const constraints, size_t* cluster_labels);


#endif
