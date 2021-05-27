#include "gtest/gtest.h"
#include <distances/supremum_distance.h>

TEST(distances, supremum_similar) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum1) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_similar_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum1_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistanceUnrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_similar_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum1_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_4Unrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, supremum_similar_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = SupremumDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, supremum1_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = SupremumDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 6.5, 0.0001);

    double sim2 = SupremumDistance_Vectorized(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
