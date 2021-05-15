#ifndef TEAM33_DISTANCES_SUPREMUM_DISTANCE_H
#define TEAM33_DISTANCES_SUPREMUM_DISTANCE_H

#include <cmath>

/**
 * @brief Computes the Supremum distance between a and b
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistance(const double* a, const double* b, size_t n) {
    double distance = 0.0;
    for(size_t i = 0; i < n; ++i) {
        double diff = std::abs(a[i] - b[i]);
        if(diff > distance) {
            distance = diff;
        }
    }

    return distance;
}


/**
 * @brief Unrolled version of function SupremumDistance.
 *
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return max(|a-b|)
 */
inline double SupremumDistanceUnrolled(const double* a, const double* b, size_t n) {
    double distance = 0.0;
    double diff_0 = 0.0;
    double diff_1 = 0.0;

    size_t i = 0;
    for(; i < n-1; i+=2) {
        double diff_0 = std::abs(a[i] - b[i]);
        double diff_1 = std::abs(a[i+1] - b[i+1]);
        distance = std::fmax(distance, std::fmax(diff_0, diff_1));
    }

    // scalar clean-up
    for (; i < n; i++) {
        double diff_0 = std::abs(a[i] - b[i]);
        if(diff_0 > distance) {
            distance = diff_0;
        }
    }

    return distance;
}

#endif
