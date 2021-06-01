#include <hdbscan/HDBSCAN_star.h>
#include <common/memory.h>

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
    if(optimization_level == "no_optimization") {
        return CalculateCoreDistancesNoOptimization;
    } else {
        return CalculateCoreDistancesSymmetry;
    }

    throw std::invalid_argument("Optimization level not supported");
}

double* CalculateCoreDistancesNoOptimization(const double* const * const data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    double* knn_distances = (double*)malloc(num_neighbors*sizeof(double));
    for(size_t point = 0; point < num_points; ++point) {
        for(size_t i = 0; i < num_neighbors; ++i) {
            knn_distances[i] = DOUBLE_MAX;
        }

        // TODO use symmetry
        for(size_t neighbor = 0; neighbor < num_points; ++neighbor) {
            if(point == neighbor) {
                continue;
            }

            double distance = distance_function(data_set[point], data_set[neighbor], num_dimensions);

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
    return result;
}

double* CalculateCoreDistancesSymmetry(const double* const * const data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    // initialize distance matrix with DOUBLE_MAX
    distance_matrix = CreateAlignedDouble2D(num_points, num_points);
    double d;
    // populate distance matrix
    for (size_t point = 0; point < num_points; point++) {
        distance_matrix[point][point] = DOUBLE_MAX;
        for (size_t neighbor = point + 1; neighbor < num_points; neighbor++) {
            d = distance_function(data_set[point], data_set[neighbor], num_dimensions);
            distance_matrix[point][neighbor] = d;
            distance_matrix[neighbor][point] = d;
        }
    }

    // calculate core distance
    double* knn_distances = (double*)malloc(num_neighbors*sizeof(double));;
    for(size_t point = 0; point < num_points; ++point) {
        for(size_t i = 0; i < num_neighbors; ++i) {
            knn_distances[i] = DOUBLE_MAX;
        }

        for(size_t neighbor = 0; neighbor < num_points; ++neighbor) {

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

    free(knn_distances);

    return result;
}
