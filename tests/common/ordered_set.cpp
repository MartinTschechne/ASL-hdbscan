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