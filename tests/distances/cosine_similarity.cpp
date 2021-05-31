#include "gtest/gtest.h"
#include <distances/cosine_similarity.h>

TEST(distances, cosine_similarity_similar) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarity(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_similarity_ortho) {
    double a[3] = {2.0, -4.2, -3.15};
    double b[3] = {-21.84, -21.155, 14.34};

    double sim = CosineSimilarity(a, b, 5);

    ASSERT_NEAR(sim, 1.0, 0.0001);
}

TEST(distances, cosine_similarity1) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarity(a, b, 5);

    ASSERT_NEAR(sim, 0.376110322, 0.0001);
}

TEST(distances, cosine_similarity_similar_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarityUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_similarity_ortho_unrolled) {
    double a[3] = {2.0, -4.2, -3.15};
    double b[3] = {-21.84, -21.155, 14.34};

    double sim = CosineSimilarityUnrolled(a, b, 3);

    ASSERT_NEAR(sim, 1.0, 0.0001);
}

TEST(distances, cosine_similarity1_unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarityUnrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.376110322, 0.0001);
}

TEST(distances, cosine_similarity_similar_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarity_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_similarity_ortho_4unrolled) {
    double a[3] = {2.0, -4.2, -3.15};
    double b[3] = {-21.84, -21.155, 14.34};

    double sim = CosineSimilarity_4Unrolled(a, b, 3);

    ASSERT_NEAR(sim, 1.0, 0.0001);
}

TEST(distances, cosine_similarity1_4unrolled) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarity_4Unrolled(a, b, 5);

    ASSERT_NEAR(sim, 0.376110322, 0.0001);
}

#ifdef __AVX2__
TEST(distances, cosine_similarity_similar_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarity_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_similarity_ortho_vectorized) {
    double a[3] = {2.0, -4.2, -3.15};
    double b[3] = {-21.84, -21.155, 14.34};

    double sim = CosineSimilarity_Vectorized(a, b, 3);

    ASSERT_NEAR(sim, 1.0, 0.0001);
}

TEST(distances, cosine_similarity1_vectorized) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarity_Vectorized(a, b, 5);

    ASSERT_NEAR(sim, 0.376110322, 0.0001);
}

TEST(distances, cosine_similar_fma) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

    double sim = CosineSimilarity_Vectorized_FMA(a, b, 5);

    ASSERT_NEAR(sim, 0.0, 0.0001);
}

TEST(distances, cosine_fma) {
    double a[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double b[5] = {2.5, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarity_Vectorized_FMA(a, b, 5);
    ASSERT_NEAR(sim, 0.3761103228628243, 0.0001);

    double sim2 = CosineSimilarity_Vectorized_FMA(b, a, 5);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, cosine_fma_size_17) {
    double a[17] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0};
    double b[17] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};

    double sim = CosineSimilarity_Vectorized_FMA(a, b, 17);
    ASSERT_NEAR(sim, 0.2441844109269068, 0.0001);

    double sim2 = CosineSimilarity_Vectorized_FMA(b, a, 17);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, cosine_fma_size_21) {
    double a[21] = {1.0, 2.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0};
    double b[21] = {2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0};

    double sim = CosineSimilarity_Vectorized_FMA(a, b, 21);
    ASSERT_NEAR(sim, 0.276780303870508, 0.0001);

    double sim2 = CosineSimilarity_Vectorized_FMA(b, a, 21);
    ASSERT_NEAR(sim2, sim, 0.0001);
}

TEST(distances, cosine_fma_size_42) {
    double a[42] = {6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.0, 3.0, 4.0, 5.0, 3.5, 10.0};
    double b[42] = {2.5, 8.5, 2.0, 1.0, 3.0, 5.0, 3.5, 6.5, 7.0, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 2.5, 8.5, 2.0, 1.0, 3.0, 4.0, 5.0, 3.5, 1.0, 10.0, 5.0, 3.5, 6.5, 7.0, 1.0, 8.5, 2.0, 1.0, 3.0, 3.0, 7.5, 8.5};

    double sim = CosineSimilarity_Vectorized_FMA(a, b, 42);
    ASSERT_NEAR(sim, 0.21548184293912998, 0.0001);

    double sim2 = CosineSimilarity_Vectorized_FMA(b, a, 42);
    ASSERT_NEAR(sim2, sim, 0.0001);
}
#endif //__AVX2__
