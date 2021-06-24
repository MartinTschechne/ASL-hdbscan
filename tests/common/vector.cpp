#include "gtest/gtest.h"
#include <common/vector.h>

TEST(vector, init_free) {
    vector v;
    vector_init(&v);

    ASSERT_EQ(vector_size(&v), 0);

    vector_free(&v);
}

TEST(vector, push_back_size_get) {
    vector v;
    vector_init(&v);

    const char words[6][10] = {
        "This",
        "is",
        "a",
        "test",
        "of",
        "C vector"
        };

    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }

    ASSERT_EQ(vector_size(&v), 6);

    for (size_t i = 0; i < vector_size(&v); i++) {
        ASSERT_EQ((char*)vector_get(&v, i), words[i]);
    }

     for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }

    ASSERT_EQ(vector_size(&v), 12);

    vector_free(&v);
}

TEST(vector, set_insert_erase_pop_back) {
    vector v;
    vector_init(&v);

    const char words[6][10] = {
        "This",
        "is",
        "a",
        "test",
        "of",
        "C vector"
        };

    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }

    const char word[] = "astrophage";

    vector_set(&v, 0, word);
    ASSERT_EQ((char*)vector_get(&v, 0), word);

    vector_insert(&v, 20, 1, word);
    ASSERT_EQ(vector_size(&v), 6);

    vector_insert(&v, 6, 3, words[1]);
    ASSERT_EQ(vector_size(&v), 9);

    vector_insert(&v, 0, 3, words[3]);
    ASSERT_EQ(vector_size(&v), 12);

    for (int i = 0; i < 3; i++) {
        vector_erase(&v, 1);
    }
    ASSERT_EQ(vector_size(&v), 9);
    ASSERT_EQ((char*)vector_get(&v, 0), words[3]);

    for (int i = 0; i < 3; i++) {
        vector_pop_back(&v);
    }
    ASSERT_EQ(vector_size(&v), 6);
    ASSERT_EQ((char*)vector_get(&v, 5), words[5]);

    vector_free(&v);
}

TEST(vector, resize_clear) {
    vector v;
    vector_init(&v);

    const char words[6][10] = {
        "This",
        "is",
        "a",
        "test",
        "of",
        "C vector"
        };

    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }
    vector_insert(&v, 2, 300, words[5]);
    ASSERT_EQ(vector_size(&v), 306);
    ASSERT_EQ(v.capacity, 612);

    for (int i = 0; i < 300; i++) {
        vector_pop_back(&v);
    }
    ASSERT_EQ(v.capacity, 19);

    vector_clear(&v);
    ASSERT_EQ(vector_size(&v), 0);

    vector_free(&v);
}

TEST(vector, primitive_types) {
    vector v;
    vector_init(&v);

    const int a[] = {5, 4, 3, 9, 8, 7};
    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, &a[i]);
    }

    ASSERT_EQ(vector_size(&v), 6);

    for (size_t i = 0; i < vector_size(&v); i++) {
        ASSERT_EQ(*(int*)vector_get(&v, i), a[i]);
    }

    vector_clear(&v);

    const double b[] = {5., 4., 3., 9., 8., 7.};
    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, &b[i]);
    }

    ASSERT_EQ(vector_size(&v), 6);

    for (size_t i = 0; i < vector_size(&v); i++) {
        ASSERT_EQ(*(double*)vector_get(&v, i), b[i]);
    }
}

TEST(vector, insert_AVX) {
    vector* v = vector_create();

    const int a[] = {5, 4, 3, 9, 8, 7, 11, 12, 13, 14};
    for (int i = 0; i < 10; i++) {
        vector_push_back(v, &a[i]);
    }
    ASSERT_EQ(vector_size(v), 10);

    size_t count_1{3};
    vector_insert_AVX(v, 0, count_1, &a[5]);
    ASSERT_EQ(vector_size(v), 10 + count_1);
    for (size_t i = 0; i < count_1; i++) {
        ASSERT_EQ(*(int*)vector_get(v, i), a[5]);
    }

    size_t count_2{10};
    size_t start{vector_size(v)};
    vector_insert_AVX(v, start, count_2, &a[5]);
    ASSERT_EQ(vector_size(v), start + count_2);
    for (size_t i = start; i < vector_size(v); i++) {
        ASSERT_EQ(*(int*)vector_get(v, i), a[5]);
    }

    vector_free(v);
}

TEST(vector, erase_AVX) {
    vector* v = vector_create();

    const int a[] = {5, 4, 3, 9, 8, 7, 11, 12, 13};
    for (int i = 0; i < 9; i++) {
        vector_push_back(v, &a[i]);
    }
    ASSERT_EQ(vector_size(v), 9);

    vector_erase_AVX(v, 0);
    vector_erase_AVX(v, 0);
    ASSERT_EQ(vector_size(v), 7);
    ASSERT_EQ(*(int*)vector_get(v, 0), a[2]);

    vector_free(v);
}

TEST(vector, contains) {
    vector* v = vector_create();

    const int a[] = {5, 4, 3, 9, 8, 7};
    for (int i = 0; i < 6; i++) {
        vector_push_back(v, &a[i]);
    }

    for (size_t i = 0; i < 6; i++) {
        ASSERT_TRUE(vector_contains(v, (void*)&a[i]));
    }
    ASSERT_FALSE(vector_contains(v, nullptr));
}

TEST(vector, contains_AVX) {
    vector* v = vector_create();

    const int a[] = {5, 4, 3, 9, 8, 7, 11, 12, 13, 14, 15, 16, 17};
    const int b[] = {5, 4, 3, 9, 8, 7, 11, 12, 13, 14, 15, 16, 17};
    for (int i = 0; i < 13; i++) {
        vector_push_back(v, &a[i]);
    }

    for (size_t i = 0; i < 13; i++) {
        ASSERT_TRUE(vector_contains_AVX(v, (void*)&a[i]));
        ASSERT_FALSE(vector_contains_AVX(v, (void*)&b[i]));
    }
    ASSERT_FALSE(vector_contains(v, nullptr));
}
