#include <hdbscan/HDBSCAN_star.h>

// Example how we could select the function on compile time using flags
// cmake -D CMAKE_CXX_FLAGS="-DOPTIMIZATION_SYMMETRY" ..
//
// #if defined (OPTIMIZATION_VECTORISE)
// #elif defined (OPTIMIZATION_UNROLL)
// #elif defined (OPTIMIZATION_SYMMETRY)
// void CalculateCoreDistances(const std::vector<std::vector<double>>& data_set, size_t k,
//     DistanceCalculator distance_function, std::vector<double>& result) {

//     CalculateCoreDistancesSymmetry(data_set, k, distance_function, result); 
// }
// #else // No Optimization
// void CalculateCoreDistances(const std::vector<std::vector<double>>& data_set, size_t k,
//     DistanceCalculator distance_function, std::vector<double>& result) {

//     CalculateCoreDistancesNoOptimization(data_set, k, distance_function, result); 
// }
// #endif

CalculateCoreDistances_t GetCalculateCoreDistancesFunction(const std::string& optimization_level) {
    if(optimization_level == "symmetry") {
        return CalculateCoreDistancesSymmetry;
    }

    if(optimization_level != "no_optimization") {
        std::cout << "\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n";
        std::cout << "Optimization " << optimization_level << " not supported for CalculcateCoreDistances. Fallback to unoptimized version\n\n";
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n" << std::endl;
    }

    return CalculateCoreDistancesNoOptimization;
}

void CalculateCoreDistancesNoOptimization(const std::vector<std::vector<double>>& data_set, size_t k,
    DistanceCalculator distance_function, std::vector<double>& result) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    result.clear();
    result.resize(data_set.size());

    if(k == 1) {
        for(size_t point = 0; point < data_set.size(); ++point) {
            result[point] = 0;
        }
        return;
    }

    double* knn_distances = new double[num_neighbors];
    for(size_t point = 0; point < data_set.size(); ++point) {
        for(size_t i = 0; i < num_neighbors; ++i) {
            knn_distances[i] = DOUBLE_MAX;
        }

        // TODO use symmetry
        for(size_t neighbor = 0; neighbor < data_set.size(); ++neighbor) {
            if(point == neighbor) {
                continue;
            }

            double distance = distance_function(data_set[point].data(), data_set[neighbor].data(), data_set[0].size());

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
    delete[] knn_distances;
}

void CalculateCoreDistancesSymmetry(const std::vector<std::vector<double>>& data_set, size_t k,
    DistanceCalculator distance_function, std::vector<double>& result) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    result.clear();
    result.resize(data_set.size());

    if(k == 1) {
        for(size_t point = 0; point < data_set.size(); ++point) {
            result[point] = 0;
        }
        return;
    }

    // initialize distance matrix with DOUBLE_MAX
    std::vector<std::vector<double>> distance_matrix(data_set.size(),std::vector<double>(data_set.size(), DOUBLE_MAX));
    double d;
    // populate distance matrix
    for (size_t point = 0; point < data_set.size(); point++) {
        for (size_t neighbor = point + 1; neighbor < data_set.size(); neighbor++) {
            d = distance_function(data_set[point].data(), data_set[neighbor].data(), data_set[0].size());
            distance_matrix[point][neighbor] = d;
            distance_matrix[neighbor][point] = d;
        }
    }

    // calculate core distance
    double* knn_distances = new double[num_neighbors];
    for(size_t point = 0; point < data_set.size(); ++point) {
        for(size_t i = 0; i < num_neighbors; ++i) {
            knn_distances[i] = DOUBLE_MAX;
        }

        for(size_t neighbor = 0; neighbor < data_set.size(); ++neighbor) {

            //Check at which position in the nearest distances the current distance would fit:
            size_t neighbor_index = num_neighbors;
            while(neighbor_index >= 1 && distance_matrix[point][neighbor] < knn_distances[neighbor_index-1]) {
                neighbor_index--;
            }

            //Shift elements in the array to make room for the current distance:
            if(neighbor_index < num_neighbors) {
                for(size_t shift_index = num_neighbors - 1; shift_index > neighbor_index; shift_index--) {
                    knn_distances[shift_index] = knn_distances[shift_index - 1];
                }
                knn_distances[neighbor_index] = distance_matrix[point][neighbor];
            }
        }
        result[point] = knn_distances[num_neighbors-1];
    }
    delete[] knn_distances;
}