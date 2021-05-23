#include <hdbscan/HDBSCAN_star.h>
#include <float.h>
#include <stdint.h>
#include <common/bitset.h>

UndirectedGraph_C* ConstructMST(const double* const * const data_set,
    const double* core_distances, bool self_edges,
    DistanceCalculator distance_function, size_t n_pts, size_t point_dimension) {

    size_t self_edge_capacity = self_edges ? n_pts : 0;
    const double DOUBLE_MAX = DBL_MAX;
    const size_t SIZE_T_MAX = SIZE_MAX;

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    BitSet_t attached_points = CreateBitset(n_pts, false);

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    double* nearest_mrd_distances = (double*)calloc(n_pts-1 + self_edge_capacity, sizeof(double));

    for(size_t i = 0; i < n_pts-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = n_pts - 1;
    size_t num_attached_points = 1;
    SetBit(attached_points, current_point, true);

    //Continue attaching points to the MST until all points are attached:
    for(num_attached_points = 1; num_attached_points < n_pts; ++num_attached_points) {
        size_t nearest_mrd_point = SIZE_T_MAX;
        double nearest_mrd_distance = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        for(size_t neighbor = 0; neighbor < n_pts; ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(GetBit(attached_points, neighbor)) { // point already attached
                continue;
            }

            double distance = distance_function(data_set[current_point], data_set[neighbor], point_dimension);
            double mutual_reachability_distance = distance;

            if(core_distances[current_point] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[current_point];
            }
            if(core_distances[neighbor] > mutual_reachability_distance) {
                mutual_reachability_distance = core_distances[neighbor];
            }

            if(mutual_reachability_distance < nearest_mrd_distances[neighbor]) {
                nearest_mrd_distances[neighbor] = mutual_reachability_distance;
                nearest_mrd_neighbors[neighbor] = current_point;
            }

            //Check if the unattached point being updated is the closest to the tree:
            if(nearest_mrd_distances[neighbor] <= nearest_mrd_distance) {
                nearest_mrd_distance = nearest_mrd_distances[neighbor];
                nearest_mrd_point = neighbor;
            }
        }

        //Attach the closest point found in this iteration to the tree:
        SetBit(attached_points, nearest_mrd_point, true);
        current_point = nearest_mrd_point;
    }

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = (size_t*)calloc(n_pts-1 + self_edge_capacity, sizeof(size_t));
    for(size_t i = 0; i < n_pts; ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = n_pts - 1; i < n_pts*2 - 1; ++i) {
            size_t vertex = i - (n_pts - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    return UDG_Create(n_pts, nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, n_pts-1 + self_edge_capacity);
}