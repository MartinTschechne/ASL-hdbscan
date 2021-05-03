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

#endif