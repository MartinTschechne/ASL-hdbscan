#include "gtest/gtest.h"
#include <common/bitset.h>

TEST(BitSet, basics) {
    size_t size = 20;
    BitSet bitset = CreateBitset(size, false);

    for(size_t i = 0; i < size; ++i) {
        ASSERT_FALSE(GetBit(bitset, i));
    }

    for(size_t i = 0; i < size; ++i) {
        SetBit(bitset, i, true);

        for(size_t j = 0; j <= i; ++j) {
            ASSERT_TRUE(GetBit(bitset, j)) << j << " " << i;
        }
        for(size_t j = i+1; j < size; ++j) {
            ASSERT_FALSE(GetBit(bitset, j)) << j << " " << i;
        }
    }
}