#include "gtest/gtest.h"
#include <common/set.h>
#include <cstdio>
#include <set>
#include <random>

TEST(set, bitset_create_free) {
    const size_t num_bits{512};
    bitset* b = bitset_create(num_bits);

    ASSERT_EQ(num_bits, b->num_bits);
    ASSERT_EQ(9, b->num_words);

    bitset_free(b);
}

TEST(set, bitset_resize) {
    size_t num_bits{512};
    bitset* b = bitset_create(num_bits);

    ASSERT_EQ(num_bits, b->num_bits);

    num_bits *= 4;
    bitset_resize(b, num_bits);
    ASSERT_EQ(num_bits, b->num_bits);
    for (size_t i = 0; i < num_bits; i++) {
        ASSERT_FALSE(bitset_get(b, i));
    }

    num_bits *= 4;
    bitset_resize(b, num_bits);
    ASSERT_EQ(num_bits, b->num_bits);
    for (size_t i = 0; i < num_bits; i++) {
        ASSERT_FALSE(bitset_get(b, i));
    }

    num_bits *= 4;
    bitset_resize(b, num_bits);
    ASSERT_EQ(num_bits, b->num_bits);
    for (size_t i = 0; i < num_bits; i++) {
        ASSERT_FALSE(bitset_get(b, i));
    }

    num_bits *= 4;
    bitset_resize(b, num_bits);
    ASSERT_EQ(num_bits, b->num_bits);
    for (size_t i = 0; i < num_bits; i++) {
        ASSERT_FALSE(bitset_get(b, i));
    }

    bitset_free(b);
}

TEST(set, bitset_set_clear_get) {
    const size_t num_bits{512};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, num_bits-1);
    std::set<size_t> std_set;

    bitset* b = bitset_create(num_bits);
    size_t sample, num_samples{num_bits / 4};

    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        bitset_set(b, sample);
    }

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(bitset_get(b, *it));
        bitset_clear(b, *it);
        ASSERT_FALSE(bitset_get(b, *it)); 
    }

    for (int i = 0; i < num_bits; i++) {
        ASSERT_FALSE(bitset_get(b, i));
    }

    std_set.clear();

    num_samples = num_bits * 4;

    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        bitset_set(b, sample);
    }

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(bitset_get(b, *it));
        bitset_clear(b, *it);
        ASSERT_FALSE(bitset_get(b, *it)); 
    }

    for (int i = 0; i < num_bits; i++) {
        ASSERT_FALSE(bitset_get(b, i));
    }

    bitset_free(b);
}


TEST(set, set_create_empty_free) {
    set* s = set_create();

    ASSERT_EQ(set_size(s), 0);
    
    set_free(s);    
}

TEST(set, set_resize) {
    set* s = set_create();

    size_t num_samples = SET_DEFAULT_CAPACITY;

    ASSERT_EQ(set_size(s), 0);
    ASSERT_EQ(s->capacity, num_samples);

    num_samples *= 4;
    set_resize(s, num_samples);
    ASSERT_EQ(s->capacity, num_samples);

    num_samples *= 4;
    set_resize(s, num_samples);
    ASSERT_EQ(s->capacity, num_samples);

    num_samples *= 4;
    set_resize(s, num_samples);
    ASSERT_EQ(s->capacity, num_samples);
    
    set_free(s);    
}


TEST(set, set_insert_size_contains) {
    set* s = set_create();

    ASSERT_EQ(set_size(s), 0);

    set_insert(s, 10);
    ASSERT_TRUE(set_contains(s, 10));
    ASSERT_EQ(set_size(s), 1);

    set_insert(s, 10);
    ASSERT_TRUE(set_contains(s, 10));
    ASSERT_EQ(set_size(s), 1);

    set_insert(s, 20);
    ASSERT_TRUE(set_contains(s, 20));
    ASSERT_EQ(set_size(s), 2);

    set_insert(s, 532);
    ASSERT_TRUE(set_contains(s, 532));
    ASSERT_EQ(set_size(s), 3);

    set_free(s);
}

TEST(set, set_erase_empty) {
    set* s = set_create();

    ASSERT_TRUE(set_empty(s));

    set_insert(s, 10);
    set_insert(s, 20);
    set_insert(s, 532);

    ASSERT_FALSE(set_empty(s));

    set_erase(s, 10);
    ASSERT_FALSE(set_contains(s, 10));
    ASSERT_EQ(set_size(s), 2);

    set_erase(s, 20);
    ASSERT_FALSE(set_contains(s, 20));
    ASSERT_EQ(set_size(s), 1);

    set_erase(s, 532);
    ASSERT_FALSE(set_contains(s, 532));
    ASSERT_EQ(set_size(s), 0);

    set_erase(s, 10);
    ASSERT_FALSE(set_contains(s, 10));
    ASSERT_EQ(set_size(s), 0);

    ASSERT_TRUE(set_empty(s));

    set_free(s);
}

TEST(set, set_next_get_no_resize) {
    const size_t b{2048};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::set<size_t> std_set;
    set* s = set_create();


    size_t sample, num_samples{SET_DEFAULT_CAPACITY / 4};
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        set_insert(s, sample);
    }

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(set_contains(s, *it));
    }

    for (size_t i = set_begin(s); i != set_end(s); i = set_next(s, i)) {
        ASSERT_TRUE(std_set.contains(set_get(s, i)));
    }

    ASSERT_EQ(std_set.size(), set_size(s));

    size_t count{0};
    for (const size_t& sample : std_set) {
        set_erase(s, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, set_size(s));

    set_free(s);
}

TEST(set, set_next_get_resize) {
    const size_t b{16384};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(0, b);

    std::set<size_t> std_set;
    set* s = set_create();

    size_t sample, num_samples{SET_DEFAULT_CAPACITY * 4};
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        set_insert(s, sample);
    }

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(set_contains(s, *it));
    }

    for (size_t i = set_begin(s); i != set_end(s); i = set_next(s, i)) {
        ASSERT_TRUE(std_set.contains(set_get(s, i)));
    }

    ASSERT_EQ(std_set.size(), set_size(s));

    
    size_t count{0};
    for (const size_t& sample : std_set) {
        set_erase(s, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, set_size(s));
    
    std_set.clear();
    num_samples *= 4;
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        set_insert(s, sample);
    }

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(set_contains(s, *it));
    }

    for (size_t i = set_begin(s); i != set_end(s); i = set_next(s, i)) {
        ASSERT_TRUE(std_set.contains(set_get(s, i)));
    }

    ASSERT_EQ(std_set.size(), set_size(s));

    count = 0;
    for (const size_t& sample : std_set) {
        set_erase(s, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, set_size(s));
   
    std_set.clear();
    num_samples *= 4;
    for (int i = 0; i < num_samples; i++) {
        sample = distrib(gen);
        std_set.insert(sample);
        set_insert(s, sample);
    }

    for (auto it = std_set.begin(); it != std_set.end(); it++) {
        ASSERT_TRUE(set_contains(s, *it));
    }

    for (size_t i = set_begin(s); i != set_end(s); i = set_next(s, i)) {
        ASSERT_TRUE(std_set.contains(set_get(s, i)));
    }

    ASSERT_EQ(std_set.size(), set_size(s));

    count = 0;
    for (const size_t& sample : std_set) {
        set_erase(s, sample);
        count++;
    }
    ASSERT_EQ(std_set.size() - count, set_size(s));

    set_free(s);
}
