#ifndef TEAM33_DISTANCES_MANHATTAN_DISTANCE_H
#define TEAM33_DISTANCES_MANHATTAN_DISTANCE_H

#include <cmath>

/**
 * @brief Computes the Manhattan distance between a and b
 * 
 * @param a Vector of length >= n
 * @param b Vector of length >= n
 * @param n Size of the vectors
 * @return Manhattan distance 
 */
inline double ManhattanDistance(double* a, double* b, size_t n) {
    double distance = 0.0;
    for(size_t i = 0; i < n; ++i) {
        distance += std::abs(a[i] - b[i]);
    }

    return distance;
}

#endif