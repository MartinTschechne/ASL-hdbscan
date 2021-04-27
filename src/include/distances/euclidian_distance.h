#ifndef TEAM33_DISTANCES_EUCLIDIAN_DISTANCE_H
#define TEAM33_DISTANCES_EUCLIDIAN_DISTANCE_H

#include <cmath>

/**
 * @brief Computes the Euclidian distance between a and b
 * 
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Euclidian distance
 */
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