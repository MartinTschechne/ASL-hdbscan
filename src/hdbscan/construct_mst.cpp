#include <hdbscan/HDBSCAN_star.h>

UndirectedGraph ConstructMST(const std::vector<std::vector<double>>& data_set,
        const std::vector<double>& core_distances, bool self_edges,
        DistanceCalculator distance_function) {

    size_t self_edge_capacity = self_edges ? data_set.size() : 0;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    const size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();

    //One bit is set (true) for each attached point, or unset (false) for unattached points:
    std::vector<bool> attached_points(data_set.size()); // bool vector is specialized to act like bitset

    //Each point has a current neighbor point in the tree, and a current nearest distance:
    size_t* nearest_mrd_neighbors = new size_t[data_set.size()-1 + self_edge_capacity];
    double* nearest_mrd_distances = new double[data_set.size()-1 + self_edge_capacity];

    for(size_t i = 0; i < data_set.size()-1; ++i) {
        nearest_mrd_distances[i] = DOUBLE_MAX;
    }

    //The MST is expanded starting with the last point in the data set:
    size_t current_point = data_set.size() - 1;
    size_t num_attached_points = 1;
    attached_points[current_point] = true;

    //Continue attaching points to the MST until all points are attached:
    while(num_attached_points < data_set.size()) {
        size_t nearest_mrd_point = SIZE_T_MAX;
        double nearest_mrd_distance = DOUBLE_MAX;

        //Iterate through all unattached points, updating distances using the current point:
        for(size_t neighbor = 0; neighbor < data_set.size(); ++neighbor) {
            if(current_point == neighbor) {
                continue;
            }
            if(attached_points[neighbor]) { // point already attached
                continue;
            }

            double distance = distance_function(data_set[current_point].data(), data_set[neighbor].data(), data_set[0].size());
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
        attached_points[nearest_mrd_point] = true;
        ++num_attached_points;
        current_point = nearest_mrd_point;
    }

    //Create an array for vertices in the tree that each point attached to:
    size_t* other_vertex_indices = new size_t[data_set.size()-1 + self_edge_capacity];
    for(size_t i = 0; i < data_set.size(); ++i) {
        other_vertex_indices[i] = i;
    }

    //If necessary, attach self edges:
    if(self_edges) {
        for(size_t i = data_set.size() - 1; i < data_set.size()*2 - 1; ++i) {
            size_t vertex = i - (data_set.size() - 1);
            nearest_mrd_neighbors[i] = vertex;
            other_vertex_indices[i] = vertex;
            nearest_mrd_distances[i] = core_distances[vertex];
        }
    }

    return UndirectedGraph(data_set.size(), nearest_mrd_neighbors, other_vertex_indices, nearest_mrd_distances, data_set.size()-1 + self_edge_capacity);
}
