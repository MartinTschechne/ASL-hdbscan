#include "gtest/gtest.h"
#include <distances/pearson_correlation.h>

TEST(distances, pearson_max_positive) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = PearsonCorrelation(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, pearson_in_between) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = PearsonCorrelation(a, b, 5);

    ASSERT_NEAR(sim, 1.34893828, 0.0001);

    double sim2 = PearsonCorrelation(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
