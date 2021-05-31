#include "gtest/gtest.h"
#include <common/quicksort.h>

#include <algorithm>
#include <array>
#include <random>

TEST(QuickSort, basics) {
    const double a{1000.};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib(-a, a);

    const size_t num_samples{2048};

    double* array_1 = (double*)malloc(num_samples * sizeof(*array_1));
    std::array<double, num_samples> array_2;

    double sample;
    for (size_t i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        array_1[i] = sample;
        array_2[i] = sample;
    }

    Quicksort(array_1, 0, num_samples-1);
    std::sort(array_2.begin(), array_2.end());

    for (size_t i = 0; i < num_samples; i++) {
        ASSERT_EQ(array_2[i], array_1[i]);
    }
    free(array_1);
}
