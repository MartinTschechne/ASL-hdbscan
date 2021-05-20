#ifndef TEAM33_QUICKSORT_H
#define TEAM33_QUICKSORT_H

#include <cstddef>

/**
 * @brief Swaps the values between two pointer locations.
 * 
 * @param index_one The first pointer location
 * @param index_two The second pointer location
 */
inline void SwapValues(double* one, double* two) {
        double temp = *one;
        *one = *two;
        *two = temp;
}

/**
 * @brief Partitions the array in the interval [start_index, end_index] 
 * 
 * @param a The array of type double being quicksorted
 * @param start_index The lowest index to  partition
 * @param end_index The highest index to partition
 * @return The index position of the pivot after the partition
 */
long int Partition(double* a, long int start_index, long int end_index) {
    double pivot_value = a[end_index];
    long int low_index = start_index;
    for (long int i = start_index; i < end_index; i++) {
        if (a[i] <= pivot_value) {
            SwapValues(&a[low_index], &a[i]);
            low_index++;
        }
    }
    SwapValues(&a[low_index], &a[end_index]);
    return low_index;
}

/**
 * @brief Quicksorts an array of double in the interval [startIndex, endIndex] 
 * 
 * @param array The array of type double to quicksort
 * @param startIndex The lowest index to be included in the sort
 * @param endIndex The highest index to be included in the sort
 */
void Quicksort(double* a, long int start_index, long int end_index) {
    if (start_index < end_index) {
        size_t pivot_index = Partition(a, start_index, end_index);
        Quicksort(a, start_index, pivot_index - 1);
        Quicksort(a, pivot_index + 1, end_index);
    }
}

#endif
