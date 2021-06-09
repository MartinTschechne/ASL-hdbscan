#include <hdbscan/HDBSCAN_star.h>
#include <common/memory.h>
#include <benchmark/tsc_x86.h>
#include <fstream>
#include <cmath>
#include <immintrin.h>
#include <common/vector_reductions.h>

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
    #ifdef __AVX2__
    } else if(optimization_level == "full") {
        return CalculateCoreDistancesBlocked_Euclidean;
    #endif //__AVX2__
    } else {
        return CalculateCoreDistancesSymmetry;
        //return CalculateCoreDistancesSymmetry;
    }

    throw std::invalid_argument("Optimization level not supported");
}

double* CalculateCoreDistancesNoOptimization(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);
    // std::ofstream timing("core_distance_base.txt");

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    uint64_t start = start_tsc();
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

            double distance = distance_function(&data_set[point*num_dimensions], &data_set[neighbor*num_dimensions], num_dimensions);

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
    uint64_t end = stop_tsc(start);
    // timing <<"Find core distance, " << end << "\n";
    free(knn_distances);
    return result;
}

double* CalculateCoreDistancesSymmetry(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);
    // std::ofstream timing("core_distance.txt");

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    // initialize distance matrix with DOUBLE_MAX
    if(distance_matrix == nullptr) {
        distance_matrix = CreateAlignedDouble2D(num_points, num_points);
    }
    double d;

    uint64_t start = start_tsc();
    // populate distance matrix
    for (size_t point = 0; point < num_points; point++) {
        distance_matrix[point][point] = DOUBLE_MAX;
        for (size_t neighbor = point + 1; neighbor < num_points; neighbor++) {
            d = distance_function(&data_set[point*num_dimensions], &data_set[neighbor*num_dimensions], num_dimensions);
            distance_matrix[point][neighbor] = d;
            distance_matrix[neighbor][point] = d;
        }
    }
    uint64_t end = stop_tsc(start);
    // timing <<"Distance matrix, " << end << "\n";

    // calculate core distance
    double* knn_distances = (double*)malloc(num_neighbors*sizeof(double));
    start = start_tsc();
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
    end = stop_tsc(start);
    // timing <<"Find core distance, " << end << "\n";

    free(knn_distances);

    return result;
}

double* CalculateCoreDistancesSymmetry_blocked(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);
    // std::ofstream timing("symmetry_blocked.txt");
    size_t block_size = 16;

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    // initialize distance matrix with DOUBLE_MAX
    if(distance_matrix == nullptr) {
        distance_matrix = CreateAlignedDouble2D(num_points, num_points);
    }
    double d;

    uint64_t start = start_tsc();
    // populate distance matrix
    for (size_t ii = 0; ii < num_points; ii += block_size) {
        for(size_t jj = ii; jj < num_points; jj += block_size) {
            for(size_t i = ii; i < min(ii + block_size, num_points); ++i) {
                distance_matrix[i][i] = DOUBLE_MAX;
                for(size_t j = jj; j < min(jj + block_size, num_points); ++j) {
                    d = distance_function(&data_set[i*num_dimensions], &data_set[j*num_dimensions], num_dimensions);
                    distance_matrix[i][j] = d;
                    distance_matrix[j][i] = d;
                }
            }
        }
    }
    uint64_t end = stop_tsc(start);
    // timing <<"Distance matrix, " << end << "\n";

    // calculate core distance
    double* knn_distances = (double*)malloc(num_neighbors*sizeof(double));
    start = start_tsc();
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
    end = stop_tsc(start);
    // timing <<"Find core distance, " << end << "\n";

    free(knn_distances);

    return result;
}


#ifdef __AVX2__
double* CalculateCoreDistancesBlocked_Euclidean(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);
    // std::ofstream timing("core_distance_blocked.txt");
    size_t block_size = 16;

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    // initialize distance matrix with DOUBLE_MAX
    if(distance_matrix == nullptr) {
        distance_matrix = CreateAlignedDouble2D(num_points, num_points);
    }
    double d;

    uint64_t start = start_tsc();
    // populate distance matrix
    for(size_t ii = 0; ii < num_points; ii += block_size) {
        for(size_t jj = ii; jj < num_points; jj += block_size) {
            for(size_t i = ii; i < min(ii + block_size, num_points); ++i) {
                if(jj+block_size <= num_points) {
                    for(size_t j = jj; j + 7 < jj + block_size; j += 8) {
                        __m256d d0 = _mm256_setzero_pd();
                        __m256d d1 = _mm256_setzero_pd();
                        __m256d d2 = _mm256_setzero_pd();
                        __m256d d3 = _mm256_setzero_pd();
                        __m256d d4 = _mm256_setzero_pd();
                        __m256d d5 = _mm256_setzero_pd();
                        __m256d d6 = _mm256_setzero_pd();
                        __m256d d7 = _mm256_setzero_pd();
                        size_t k = 0;
                        for(; k + 3 < num_dimensions; k += 4) {
                            __m256d pi = _mm256_load_pd(&data_set[i*num_dimensions + k]);

                            __m256d pj0 = _mm256_load_pd(&data_set[j*num_dimensions + k]);
                            __m256d pj1 = _mm256_load_pd(&data_set[(j+1)*num_dimensions + k]);
                            __m256d pj2 = _mm256_load_pd(&data_set[(j+2)*num_dimensions + k]);
                            __m256d pj3 = _mm256_load_pd(&data_set[(j+3)*num_dimensions + k]);
                            __m256d pj4 = _mm256_load_pd(&data_set[(j+4)*num_dimensions + k]);
                            __m256d pj5 = _mm256_load_pd(&data_set[(j+5)*num_dimensions + k]);
                            __m256d pj6 = _mm256_load_pd(&data_set[(j+6)*num_dimensions + k]);
                            __m256d pj7 = _mm256_load_pd(&data_set[(j+7)*num_dimensions + k]);

                            __m256d diff0 = _mm256_sub_pd(pi, pj0);
                            __m256d diff1 = _mm256_sub_pd(pi, pj1);
                            __m256d diff2 = _mm256_sub_pd(pi, pj2);
                            __m256d diff3 = _mm256_sub_pd(pi, pj3);
                            __m256d diff4 = _mm256_sub_pd(pi, pj4);
                            __m256d diff5 = _mm256_sub_pd(pi, pj5);
                            __m256d diff6 = _mm256_sub_pd(pi, pj6);
                            __m256d diff7 = _mm256_sub_pd(pi, pj7);

                            d0 = _mm256_fmadd_pd(diff0, diff0, d0);
                            d1 = _mm256_fmadd_pd(diff1, diff1, d1);
                            d2 = _mm256_fmadd_pd(diff2, diff2, d2);
                            d3 = _mm256_fmadd_pd(diff3, diff3, d3);
                            d4 = _mm256_fmadd_pd(diff4, diff4, d4);
                            d5 = _mm256_fmadd_pd(diff5, diff5, d5);
                            d6 = _mm256_fmadd_pd(diff6, diff6, d6);
                            d7 = _mm256_fmadd_pd(diff7, diff7, d7);
                        }
                        __m256d distances0 = _mm256_set_pd(_mm256_reduce_sum_pd(d3), _mm256_reduce_sum_pd(d2), _mm256_reduce_sum_pd(d1), _mm256_reduce_sum_pd(d0));
                        __m256d distances1 = _mm256_set_pd(_mm256_reduce_sum_pd(d7), _mm256_reduce_sum_pd(d6), _mm256_reduce_sum_pd(d5), _mm256_reduce_sum_pd(d4));
                        for(; k < num_dimensions; ++k) {
                            double pi = data_set[i*num_dimensions + k];
                            __m256d pi_intr =_mm256_set1_pd(pi);
                            __m256d diff0 = _mm256_sub_pd(
                                pi_intr,
                                _mm256_set_pd(data_set[(j+3)*num_dimensions + k], data_set[(j+2)*num_dimensions + k], data_set[(j+1)*num_dimensions + k], data_set[j*num_dimensions + k])
                            );
                            __m256d diff1 = _mm256_sub_pd(
                                pi_intr,
                                _mm256_set_pd(data_set[(j+7)*num_dimensions + k], data_set[(j+6)*num_dimensions + k], data_set[(j+5)*num_dimensions + k], data_set[(j+4)*num_dimensions + k])
                            );
                            distances0 = _mm256_fmadd_pd(diff0, diff0, distances0);
                            distances1 = _mm256_fmadd_pd(diff1, diff1, distances1);
                        }
                        __m256d root0 = _mm256_sqrt_pd(distances0);
                        __m256d root1 = _mm256_sqrt_pd(distances1);
                        _mm256_store_pd(distance_matrix[i] + j, root0);
                        _mm256_store_pd(distance_matrix[i] + j + 4, root1);
                    }
                    for(size_t j = jj; j < jj+block_size; ++j) {
                        distance_matrix[j][i] = distance_matrix[i][j];
                    }
                } else {
                    for(size_t j = jj; j < min(jj+block_size,num_points); ++j) {
                        __m256d d0 = _mm256_setzero_pd();
                        __m256d d1 = _mm256_setzero_pd();
                        __m256d d2 = _mm256_setzero_pd();
                        __m256d d3 = _mm256_setzero_pd();
                        size_t k = 0;
                        for(; k + 15 < num_dimensions; k += 16) {
                            __m256d pi0 = _mm256_load_pd(&data_set[i*num_dimensions + k]);
                            __m256d pj0 = _mm256_load_pd(&data_set[j*num_dimensions + k]);
                            __m256d pi1 = _mm256_load_pd(&data_set[i*num_dimensions + k + 4]);
                            __m256d pj1 = _mm256_load_pd(&data_set[j*num_dimensions + k + 4]);
                            __m256d pi2 = _mm256_load_pd(&data_set[i*num_dimensions + k + 8]);
                            __m256d pj2 = _mm256_load_pd(&data_set[j*num_dimensions + k + 8]);
                            __m256d pi3 = _mm256_load_pd(&data_set[i*num_dimensions + k + 12]);
                            __m256d pj3 = _mm256_load_pd(&data_set[j*num_dimensions + k + 12]);
                            __m256d diff0 = _mm256_sub_pd(pi0, pj0);
                            __m256d diff1 = _mm256_sub_pd(pi1, pj1);
                            __m256d diff2 = _mm256_sub_pd(pi2, pj2);
                            __m256d diff3 = _mm256_sub_pd(pi3, pj3);

                            d0 = _mm256_fmadd_pd(diff0, diff0, d0);
                            d1 = _mm256_fmadd_pd(diff1, diff1, d1);
                            d2 = _mm256_fmadd_pd(diff2, diff2, d2);
                            d3 = _mm256_fmadd_pd(diff3, diff3, d3);
                        }

                        d0 = _mm256_add_pd(d0, d2);
                        d1 = _mm256_add_pd(d1, d3);
                        d0 = _mm256_add_pd(d0, d1);

                        // for(; k + 3 < num_dimensions; k += 4) {
                        //     __m256d pi0 = _mm256_load_pd(&data_set[i*num_dimensions + k]);
                        //     __m256d pj0 = _mm256_load_pd(&data_set[j*num_dimensions + k]);
                        //     std::cout << i << " " << j << std::endl;
                        //     __m256d diff0 = _mm256_sub_pd(pi0, pj0);

                        //     d0 = _mm256_fmadd_pd(diff0, diff0, d0);
                        // }
                        double distance = _mm256_reduce_sum_pd(d0);
                        for(; k < num_dimensions; ++k){
                            double diff = data_set[i*num_dimensions + k] - data_set[j*num_dimensions + k];
                            distance += diff*diff;
                        }
                        distance_matrix[i][j] = sqrt(distance);
                        distance_matrix[j][i] = distance_matrix[i][j];
                    }
                }
            }
        }
    }

    uint64_t end = stop_tsc(start);
    // timing <<"Distance matrix, " << end << "\n";

    // calculate core distance
    double* knn_distances = (double*)malloc(num_neighbors*sizeof(double));
    start = start_tsc();
    for(size_t point = 0; point < num_points; ++point) {
        distance_matrix[point][point] = DOUBLE_MAX;
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
    end = stop_tsc(start);
    // timing <<"Find core distance, " << end << "\n";

    free(knn_distances);

    return result;
}


// This is just matrix-matrix multiplication
double* CalculateCoreDistancesBlocked_Euclidean_Transpose(const double* data_set, size_t k,
    DistanceCalculator distance_function, const size_t num_points, const size_t num_dimensions, double**& distance_matrix) {

    size_t num_neighbors = k - 1;
    const double DOUBLE_MAX = std::numeric_limits<double>::max();
    double* result = CreateAlignedDouble1D(num_points);
    // std::ofstream timing("core_distance_blocked_transpose.txt");
    size_t block_size = 8;

    if(k == 1) {
        for(size_t point = 0; point < num_points; ++point) {
            result[point] = 0;
        }
        return result;
    }

    // initialize distance matrix with DOUBLE_MAX
    distance_matrix = CreateAlignedDouble2D(num_points, num_points);
    double d;

    uint64_t start = start_tsc();
    for(size_t i = 0; i < num_points; ++i) {
        for(size_t j = i; j < num_points; ++j) {
            distance_matrix[i][j] = 0.0;
        }
    }
    // populate distance matrix
    for(size_t k = 0; k < num_dimensions; k += 1) {
        size_t i = 0;
        for(; i < num_points; i += 1) {
            __m256d x = _mm256_set1_pd(data_set[k*num_points + i]);
            size_t j = (i/4)*4;
            for(; j + 15 < num_points; j += 16) {
                __m256d y0 = _mm256_load_pd(&data_set[k*num_points + j]);
                __m256d y1 = _mm256_load_pd(&data_set[k*num_points + j + 4]);
                __m256d y2 = _mm256_load_pd(&data_set[k*num_points + j + 8]);
                __m256d y3 = _mm256_load_pd(&data_set[k*num_points + j + 12]);

                __m256d diff0 = _mm256_sub_pd(x, y0);
                __m256d diff1 = _mm256_sub_pd(x, y1);
                __m256d diff2 = _mm256_sub_pd(x, y2);
                __m256d diff3 = _mm256_sub_pd(x, y3);

                _mm256_store_pd(distance_matrix[i] + j, _mm256_fmadd_pd(diff0, diff0, _mm256_load_pd(distance_matrix[i] + j)));
                _mm256_store_pd(distance_matrix[i] + j + 4, _mm256_fmadd_pd(diff1, diff1, _mm256_load_pd(distance_matrix[i] + j + 4)));
                _mm256_store_pd(distance_matrix[i] + j + 8, _mm256_fmadd_pd(diff2, diff2, _mm256_load_pd(distance_matrix[i] + j + 8)));
                _mm256_store_pd(distance_matrix[i] + j + 12, _mm256_fmadd_pd(diff3, diff3, _mm256_load_pd(distance_matrix[i] + j + 12)));
            }
            for(; j + 3 < num_points; j += 4) {
                __m256d y0 = _mm256_load_pd(&data_set[k*num_points + j]);
                __m256d diff0 = _mm256_sub_pd(x, y0);

                _mm256_store_pd(distance_matrix[i] + j, _mm256_fmadd_pd(diff0, diff0, _mm256_load_pd(distance_matrix[i] + j)));
            }
            for(; j < num_points; ++j) {
                double diff = data_set[k*num_points + i] - data_set[k*num_points + j];
                distance_matrix[i][j] += diff*diff;
            }
        }
    }
    // for(size_t k = 0; k < num_dimensions; k += 1) {
    //     for(size_t i = 0; i < num_points; i += 1) {
    //         for(size_t j = i; j < num_points; j += 1) {
    //             double diff = data_set[k*num_points + i] - data_set[k*num_points + j];
    //             distance_matrix[i][j] += diff*diff;
    //         }
    //     }
    // }
    for(size_t i = 0; i < num_points; ++i) {
        for(size_t j = i; j < num_points; ++j) {
            distance_matrix[i][j] = sqrt(distance_matrix[i][j]);
            distance_matrix[j][i] = distance_matrix[i][j];
        }
    }

    uint64_t end = stop_tsc(start);
    // timing <<"Distance matrix, " << end << "\n";

    // calculate core distance
    double* knn_distances = (double*)malloc(num_neighbors*sizeof(double));
    start = start_tsc();
    for(size_t point = 0; point < num_points; ++point) {
        distance_matrix[point][point] = DOUBLE_MAX;
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
    end = stop_tsc(start);
    // timing <<"Find core distance, " << end << "\n";

    free(knn_distances);

    return result;
}
#endif //__AVX2__
