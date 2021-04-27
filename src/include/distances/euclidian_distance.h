#ifndef TEAM33_DISTANCES_EUCLIDIAN_DISTANCES
#define TEAM33_DISTANCES_EUCLIDIAN_DISTANCES

#include <cmath>

inline double CosineSimilarity(double* a, double* b, size_t n) {
    double distance = 0.0;
    for(size_t i = 0; i < n; ++i) {
        distance += a[i] * b[i];
    }

    return std::sqrt(distance);
}

#endif