#include "gtest/gtest.h"
#include <common/ordered_set.h>
#include <cstdio>
#include <set>
#include <random>


TEST(ordered_set, ordered_OS_create_empty_free) {
    OrderedSet* os = OS_create();

    ASSERT_EQ(OS_size(os), 0);

    OS_free(os);
}

TEST(ordered_set, ordered_set_resize) {
    OrderedSet* os = OS_create();

    size_t num_samples = ORDERED_SET_DEFAULT_CAPACITY;

    ASSERT_EQ(OS_size(os), 0);
    ASSERT_EQ(os->capacity, num_samples);

    num_samples *= 4;
    OS_resize(os, num_samples);
    ASSERT_EQ(os->capacity, num_samples);

    num_samples *= 4;
    OS_resize(os, num_samples);
    ASSERT_EQ(os->capacity, num_samples);

    num_samples *= 4;
    OS_resize(os, num_samples);
    ASSERT_EQ(os->capacity, num_samples);

    OS_free(os);
}

TEST(ordered_set, ordered_OS_insert_AVX) {
    OrderedSet* os = OS_create();
    OS_insert(os, 1);
    OS_insert(os, 2);
    OS_insert(os, 3);
    OS_insert(os, 4);
    OS_insert(os, 5);
    OS_insert(os, 6);
    OS_insert(os, 7);
    OS_insert(os, 8);
    OS_insert(os, 10);

    OS_insert_AVX(os, 0);
    ASSERT_EQ(os->size, 10);
    ASSERT_EQ(OS_get(os, 0), 0);

    OS_insert_AVX(os, 9);
    ASSERT_EQ(os->size, 11);
    ASSERT_EQ(OS_get(os, 9), 9);

    OS_insert_AVX(os, 11);
    ASSERT_EQ(os->size, 12);
    ASSERT_EQ(OS_get(os, OS_end(os)-1), 11);

    OS_free(os);

}

TEST(ordered_set, ordered_OS_erase_AVX) {
    OrderedSet* os = OS_create();
    OS_insert(os, 1);
    OS_insert(os, 2);
    OS_insert(os, 3);
    OS_insert(os, 4);
    OS_insert(os, 5);
    OS_insert(os, 6);
    OS_insert(os, 7);
    OS_insert(os, 8);
    OS_insert(os, 9);
    OS_insert(os, 10);
    OS_insert(os, 11);

    OS_erase_AVX(os, 1);
    ASSERT_EQ(os->size, 10);
    ASSERT_EQ(OS_get(os, 0), 2);

    OS_erase_AVX(os, 3);
    ASSERT_EQ(os->size, 9);
    ASSERT_EQ(OS_get(os, 1), 4);

    OS_erase_AVX(os, 11);
    ASSERT_EQ(os->size, 8);
    ASSERT_EQ(OS_get(os, OS_end(os)-1), 10);

    OS_clear(os);
    OS_insert(os, 1);
    OS_insert(os, 2);
    OS_insert(os, 3);
    OS_insert(os, 4);
    OS_insert(os, 5);
    OS_insert(os, 6);
    OS_insert(os, 7);
    OS_insert(os, 8);
    OS_insert(os, 9);
    OS_erase_AVX(os, 1);
    OS_erase_AVX(os, 2);
    ASSERT_EQ(os->size, 7);

    OS_free(os);
}

TEST(ordered_set, ordered_OS_insert_size_contains) {
    OrderedSet* os = OS_create();

    // OS_print(os);
    ASSERT_EQ(OS_size(os), 0);

    OS_insert(os, 10);
    // OS_print(os);
    ASSERT_TRUE(OS_contains(os, 10));
    ASSERT_EQ(OS_size(os), 1);

    OS_insert(os, 10);
    // OS_print(os);
    ASSERT_TRUE(OS_contains(os, 10));
    ASSERT_EQ(OS_size(os), 1);

    OS_insert(os, 20);
    // OS_print(os);
    ASSERT_TRUE(OS_contains(os, 20));
    ASSERT_EQ(OS_size(os), 2);

    OS_insert(os, 532);
    // OS_print(os);
    ASSERT_TRUE(OS_contains(os, 532));
    ASSERT_EQ(OS_size(os), 3);

    OS_free(os);
}

TEST(ordered_set, ordered_OS_erase_empty) {
    OrderedSet* os = OS_create();

    ASSERT_TRUE(OS_empty(os));

    OS_insert(os, 10);
    OS_insert(os, 20);
    OS_insert(os, 532);

    ASSERT_FALSE(OS_empty(os));

    OS_erase(os, 10);
    ASSERT_FALSE(OS_contains(os, 10));
    ASSERT_EQ(OS_size(os), 2);

    OS_erase(os, 20);
    ASSERT_FALSE(OS_contains(os, 20));
    ASSERT_EQ(OS_size(os), 1);

    OS_erase(os, 532);
    ASSERT_FALSE(OS_contains(os, 532));
    ASSERT_EQ(OS_size(os), 0);

    OS_erase(os, 10);
    ASSERT_FALSE(OS_contains(os, 10));
    ASSERT_EQ(OS_size(os), 0);

    ASSERT_TRUE(OS_empty(os));

    OS_free(os);
}

TEST(ordered_set, ordered_OS_next_get_no_resize) {
    const size_t b{2048};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::set<size_t> std_set;
    OrderedSet* os = OS_create();


    size_t sample, num_samples{ORDERED_SET_DEFAULT_CAPACITY / 4};
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        OS_insert(os, sample);
        std_set.insert(0);
        OS_insert(os, 0);
    }

    ASSERT_EQ(std_set.size(), OS_size(os));

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(OS_contains(os, *it));
    }

    for (size_t i = OS_begin(os); i != OS_end(os); i = OS_next(os, i)) {
        ASSERT_TRUE(std_set.contains(OS_get(os, i)));
    }


    size_t count{0};
    for (const size_t& sample : std_set) {
        OS_erase(os, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, OS_size(os));

    OS_free(os);
}

TEST(ordered_set, OS_next_get_resize) {
    const size_t b{16384};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::set<size_t> std_set;
    OrderedSet* os = OS_create();

    size_t sample, num_samples{ORDERED_SET_DEFAULT_CAPACITY * 4};
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        OS_insert(os, sample);
    }

    ASSERT_EQ(std_set.size(), OS_size(os));

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(OS_contains(os, *it));
    }

    for (size_t i = OS_begin(os); i != OS_end(os); i = OS_next(os, i)) {
        ASSERT_TRUE(std_set.contains(OS_get(os, i)));
    }

    size_t count{0};
    for (const size_t& sample : std_set) {
        OS_erase(os, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, OS_size(os));

    std_set.clear();
    num_samples *= 4;
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        OS_insert(os, sample);
    }

    ASSERT_EQ(std_set.size(), OS_size(os));

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(OS_contains(os, *it));
    }

    for (size_t i = OS_begin(os); i != OS_end(os); i = OS_next(os, i)) {
        ASSERT_TRUE(std_set.contains(OS_get(os, i)));
    }


    count = 0;
    for (const size_t& sample : std_set) {
        OS_erase(os, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, OS_size(os));

    std_set.clear();
    num_samples *= 4;
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        OS_insert(os, sample);
    }

    ASSERT_EQ(std_set.size(), OS_size(os));

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(OS_contains(os, *it));
    }

    for (size_t i = OS_begin(os); i != OS_end(os); i = OS_next(os, i)) {
        ASSERT_TRUE(std_set.contains(OS_get(os, i)));
    }

    count = 0;
    for (const size_t& sample : std_set) {
        OS_erase(os, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, OS_size(os));

    OS_free(os);
}

TEST(ordered_set, ordered_OS_prev_get_resize) {
    const size_t b{16384};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::set<size_t> std_set;
    OrderedSet* os = OS_create();

    size_t sample, num_samples{ORDERED_SET_DEFAULT_CAPACITY * 4};
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        OS_insert(os, sample);
    }

    for (auto it = std_set.rbegin(); it != std_set.rend(); it++) {
        if (!OS_contains(os, *it)) {
        }
        ASSERT_TRUE(OS_contains(os, *it));

    }

    for (size_t i = OS_prev(os, OS_end(os));
        i != OS_end(os); i = OS_prev(os, i)) {
        if (!std_set.contains(OS_get(os, i))) {
        }
        ASSERT_TRUE(std_set.contains(OS_get(os, i)));
    }

    ASSERT_EQ(std_set.size(), OS_size(os));


    size_t count{0};
    for (const size_t& sample : std_set) {
        OS_erase(os, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, OS_size(os));

    OS_free(os);
}

int compare_os(const void* a, const void* b) {
    if(*(size_t*)a > *(size_t*)b) return 1;
    if(*(size_t*)a < *(size_t*)b) return -1;
    return 0;
}

TEST(ordered_set, iterate_and_erase) {
    size_t values[] = {13, 5, 29, 11, 15};
    size_t n = 5;
    size_t count = 0;

    OrderedSet* s = OS_create();

    for(size_t i = 0; i < n; ++i) {
        OS_insert(s, values[i]);
    }

    qsort((void*)values, n, sizeof(size_t), compare_os);
    size_t set_index = OS_begin(s);
    for(size_t i = 0; i < n; ++i) {
        ASSERT_EQ(values[i], OS_get(s, set_index));
        set_index = OS_next(s, set_index);
    }

    for(size_t i = OS_begin(s); i < OS_end(s); ) {
        size_t value = OS_get(s, i);
        ASSERT_EQ(values[count], value);
        i = OS_erase(s, value);
        count++;
    }

    ASSERT_EQ(count, n);
}