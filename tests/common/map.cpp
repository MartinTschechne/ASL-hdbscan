#include "gtest/gtest.h"
#include <common/map.h>
#include <cstdio>
#include <map>
#include <set>
#include <random>


TEST(map, Map_create_empty_free) {
    Map* m = Map_create();

    ASSERT_EQ(Map_size(m), 0);
    ASSERT_EQ(m->keys->size, 0);
    ASSERT_EQ(m->values->size, 0);
    
    Map_free(m);    
}

TEST(map, Map_insert_size_contains) {
    Map* m = Map_create();

    // Map_print(m);
    ASSERT_EQ(Map_size(m), 0);

    Map_insert(m, 10, (void*)1);
    // Map_print(m);
    ASSERT_TRUE(Map_contains(m, 10));
    ASSERT_EQ(Map_size(m), 1);

    Map_insert(m, 10, (void*)1);
    // Map_print(m);
    ASSERT_TRUE(Map_contains(m, 10));
    ASSERT_EQ(Map_size(m), 1);

    Map_insert(m, 20, (void*)2);
    // Map_print(m);
    ASSERT_TRUE(Map_contains(m, 20));
    ASSERT_EQ(Map_size(m), 2);

    Map_insert(m, 532, (void*)3);
    // Map_print(m);
    ASSERT_TRUE(Map_contains(m, 532));
    ASSERT_EQ(Map_size(m), 3);

    Map_insert(m, 325, (void*)4);
    // Map_print(m);
    ASSERT_TRUE(Map_contains(m, 325));
    ASSERT_EQ(Map_size(m), 4);

    Map_insert(m, 0, (void*)5);
    // Map_print(m);
    ASSERT_TRUE(Map_contains(m, 0));
    ASSERT_EQ(Map_size(m), 5);

    Map_free(m);
}

TEST(map, Map_erase_empty) {
    Map* m = Map_create();

    ASSERT_TRUE(Map_empty(m));

    Map_insert(m, 10, (void*)1);
    Map_insert(m, 20, (void*)2);
    Map_insert(m, 532, (void*)3);

    ASSERT_FALSE(Map_empty(m));

    Map_erase(m, 10);
    ASSERT_FALSE(Map_contains(m, 10));
    ASSERT_EQ(Map_size(m), 2);

    Map_erase(m, 20);
    ASSERT_FALSE(Map_contains(m, 20));
    ASSERT_EQ(Map_size(m), 1);

    Map_erase(m, 532);
    ASSERT_FALSE(Map_contains(m, 532));
    ASSERT_EQ(Map_size(m), 0);

    Map_erase(m, 10);
    ASSERT_FALSE(Map_contains(m, 10));
    ASSERT_EQ(Map_size(m), 0);

    ASSERT_TRUE(Map_empty(m));

    Map_free(m);
}

TEST(map, Map_next_get_no_resize) {
    const size_t b{2048};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::map<size_t, size_t> std_map;
    Map* m = Map_create();


    size_t sample_key, sample_value, num_samples{ORDERED_SET_DEFAULT_CAPACITY / 4};
     for (int i = 0; i < num_samples; i++) {
        sample_key= distrib(gen);
        sample_value= distrib(gen);
        std_map.insert({sample_key, sample_value});
        Map_insert(m, sample_key, (void*)sample_value);
        ASSERT_EQ(std_map.size(), Map_size(m));
    }
    std_map.insert({0, 0});
    Map_insert(m, 0, (void*)0);

    ASSERT_EQ(std_map.size(), Map_size(m));

    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        ASSERT_TRUE(Map_contains(m, it->first));
    }

    for (size_t i = Map_begin(m); i != Map_end(m); i = Map_next(m, i)) {
        ASSERT_TRUE(std_map.contains(OS_get(m->keys, i)));
    }

    size_t count{0};
    for (auto sample: std_map) {
        Map_erase(m, sample.first);
        count++;
    }
    ASSERT_EQ(std_map.size() - count, Map_size(m));

    Map_free(m);
}

TEST(map, Map_next_get_resize) {
    const size_t b{16384};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::map<size_t, size_t> std_map;
    Map* m = Map_create();

    size_t sample_key, sample_value, num_samples{ORDERED_SET_DEFAULT_CAPACITY * 4};
    for (int i = 0; i < num_samples; i++) {
        sample_key= distrib(gen);
        sample_value= distrib(gen);
        std_map.insert({sample_key, sample_value});
        Map_insert(m, sample_key, (void*)sample_value);
    }

    ASSERT_EQ(std_map.size(), Map_size(m));

    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        ASSERT_TRUE(Map_contains(m, it->first));
    }

    for (size_t i = Map_begin(m); i != Map_end(m); i = Map_next(m, i)) {
        ASSERT_TRUE(std_map.contains(OS_get(m->keys, i)));
    }

    size_t count{0};
    for (auto sample: std_map) {
        Map_erase(m, sample.first);
        count++;
    }
    ASSERT_EQ(std_map.size() - count, Map_size(m));
    
    std_map.clear();
    num_samples *= 4;
    for (int i = 0; i < num_samples; i++) {
        sample_key= distrib(gen);
        sample_value= distrib(gen);
        std_map.insert({sample_key, sample_value});
        Map_insert(m, sample_key, (void*)sample_value);
    }

    ASSERT_EQ(std_map.size(), Map_size(m));

    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        ASSERT_TRUE(Map_contains(m, it->first));
    }

    for (size_t i = Map_begin(m); i != Map_end(m); i = Map_next(m, i)) {
        ASSERT_TRUE(std_map.contains(OS_get(m->keys, i)));
    }

    count = 0;
    for (auto sample: std_map) {
        Map_erase(m, sample.first);
        count++;
    }
    ASSERT_EQ(std_map.size() - count, Map_size(m));

    std_map.clear();
    num_samples *= 4;
    for (int i = 0; i < num_samples; i++) {
        sample_key= distrib(gen);
        sample_value= distrib(gen);
        std_map.insert({sample_key, sample_value});
        Map_insert(m, sample_key, (void*)sample_value);
    }

    ASSERT_EQ(std_map.size(), Map_size(m));

    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        ASSERT_TRUE(Map_contains(m, it->first));
    }

    for (size_t i = Map_begin(m); i != Map_end(m); i = Map_next(m, i)) {
        ASSERT_TRUE(std_map.contains(OS_get(m->keys, i)));
    }

    count = 0;
    for (auto sample: std_map) {
        Map_erase(m, sample.first);
        count++;
    }
    ASSERT_EQ(std_map.size() - count, Map_size(m));

    Map_free(m);
}

TEST(map, Map_prev_get_resize) {
    const size_t b{16384};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::map<size_t, size_t> std_map;
    Map* m = Map_create();



    size_t sample_key, sample_value, num_samples{ORDERED_SET_DEFAULT_CAPACITY * 4};
     for (int i = 0; i < num_samples; i++) {
        sample_key= distrib(gen);
        sample_value= distrib(gen);
        std_map.insert({sample_key, sample_value});
        Map_insert(m, sample_key, (void*)sample_value);
    }

    ASSERT_EQ(std_map.size(), Map_size(m));

    for (auto it = std_map.rbegin(); it != std_map.rend(); it++) {
        ASSERT_TRUE(Map_contains(m, it->first));
    }

    for (size_t i = Map_begin(m); i != Map_end(m); i = Map_next(m, i)) {
        ASSERT_TRUE(std_map.contains(OS_get(m->keys, i)));
    }

    
    for (size_t i = Map_prev(m, Map_end(m));
        i != Map_end(m); i = Map_prev(m, i)) {
        ASSERT_TRUE(std_map.contains(OS_get(m->keys, i)));
    }

    ASSERT_EQ(std_map.size(), Map_size(m));

    
    size_t count{0};
    for (auto sample: std_map) {
        Map_erase(m, sample.first);
        count++;
    }
    ASSERT_EQ(std_map.size() - count, Map_size(m));
    
    Map_free(m);
}

int compare_m(const void* a, const void* b) {
    if(*(size_t*)a > *(size_t*)b) return 1;
    if(*(size_t*)a < *(size_t*)b) return -1;
    return 0;
}

TEST(map, iterate_and_erase) {
    size_t keys[] = {13, 5, 29, 11, 15};
    void* values[] = {(void*)3, (void*)1, (void*)5, (void*)2, (void*)4};
    size_t n = 5;
    size_t count = 0;

    Map* m = Map_create();

    for(size_t i = 0; i < n; ++i) {
        Map_insert(m, keys[i], values[i]);
    }

    qsort((void*)keys, n, sizeof(size_t), compare_m);
    qsort((void*)values, n, sizeof(size_t), compare_m);
    size_t map_index = Map_begin(m);
    for(size_t i = 0; i < n; ++i) {
        ASSERT_EQ(keys[i], OS_get(m->keys, map_index));
        map_index = Map_next(m, map_index);
    }

    for(size_t i = Map_begin(m); i < Map_end(m); ) {
        void* value = Map_get_idx(m, i);
        ASSERT_EQ(values[count], value);
        i = Map_erase(m, OS_get(m->keys, i));
        count++;
    }

    ASSERT_EQ(count, n);
}