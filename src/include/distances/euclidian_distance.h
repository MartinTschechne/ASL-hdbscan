#ifndef TEAM33_DISTANCES_EUCLIDIAN_DISTANCES
#define TEAM33_DISTANCES_EUCLIDIAN_DISTANCES

#include <cmath>

inline double EuclidianDistance(double* a, double* b, size_t n) {
    double distance = 0.0;
    double diff = 0.0;
    for(size_t i = 0; i < n; ++i) {
        diff = a[i]-b[i];
        distance += diff * diff;
    }

    return std::sqrt(distance);
}

#endif