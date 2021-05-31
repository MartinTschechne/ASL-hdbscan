#include "gtest/gtest.h"
#include <distances/manhattan_distance.h>

TEST(distances, manhattan_similar) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = ManhattanDistance(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, manhattan1) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = ManhattanDistance(a, b, 5);

    ASSERT_NEAR(sim, 14.0, 0.0001);

    double sim2 = ManhattanDistance(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, manhattan_similar_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = ManhattanDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, manhattan1_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = ManhattanDistanceUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 14.0, 0.0001);

    double sim2 = ManhattanDistanceUnrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, manhattan_similar_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = ManhattanDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, manhattan1_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = ManhattanDistance_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 14.0, 0.0001);

    double sim2 = ManhattanDistance_4Unrolled(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

#ifdef __AVX2__
TEST(distances, manhattan_similar_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = ManhattanDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, manhattan_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = ManhattanDistance_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 14.0, 0.0001);

    double sim2 = ManhattanDistance_Vectorized(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, manhattan_vectorized_size_17) {
    double a[17] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0};
    double b[17] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};

    double sim = ManhattanDistance_Vectorized(a, b, 17);
    ASSERT_NEAR(sim, 56, 0.0001);

    double sim2 = ManhattanDistance_Vectorized(b, a, 17);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, manhattan_vectorized_size_21) {
    double a[21] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};
    double b[21] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0};

    double sim = ManhattanDistance_Vectorized(a, b, 21);
    ASSERT_NEAR(sim, 73.5, 0.0001);

    double sim2 = ManhattanDistance_Vectorized(b, a, 21);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, manhattan_vectorized_size_42) {
    double a[42] = {6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.0, 3.0, 4.0, 5.0, 3.5, 10.0};
    double b[42] = {2.5, 8.5, 2.0, 1.0, 3.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 3.0, 7.5, 8.5};

    double sim = ManhattanDistance_Vectorized(a, b, 42);
    ASSERT_NEAR(sim, 114, 0.0001);

    double sim2 = ManhattanDistance_Vectorized(b, a, 42);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
#endif //__AVX2__
