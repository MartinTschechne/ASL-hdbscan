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
inline double EuclidianDistance(const double* a, const double* b, size_t n) {
    double distance = 0.0;
    double diff = 0.0;
    for(size_t i = 0; i < n; ++i) {
        diff = a[i]-b[i];
        distance += diff * diff;
    }

    return std::sqrt(distance);
}


/**
 * @brief Unrolled version of function EuclideanDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Euclidian distance
 */
inline double EuclidianDistanceUnrolled(const double* a, const double* b, size_t n) {
    double distance_0 = 0.0;
    double distance_1 = 0.0;
    double diff_0 = 0.0;
    double diff_1 = 0.0;
    size_t i = 0;

    for(; i < n-1; i += 2) {
        diff_0 = a[i]-b[i];
        distance_0 += diff_0 * diff_0;

        diff_1 = a[i+1]-b[i+1];
        distance_1 += diff_1 * diff_1;
    }

    // scalar clean-up
    for (; i < n; i++) {
        diff_0 = a[i]-b[i];
        distance_0 += diff_0 * diff_0;
    }

    return std::sqrt(distance_0 + distance_1);
}

#endif
