#include "gtest/gtest.h"
#include <common/bitset.h>

TEST(BitSet, basics) {
    size_t size = 20;
    BitSet_t bitset = CreateBitset(size, false);

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

TEST(BitSet, masking) {
    size_t size = 20;
    BitSet_t bitset = CreateBitset(size, false);

    SetBit(bitset, 4, true);
    SetBit(bitset, 9, true);
    SetBit(bitset, 13, true);
    SetBit(bitset, 17, true);
    SetBit(bitset, 11, true);
    SetBit(bitset, 8, true);

    for(size_t i = 0; i < size-3; i += 4) {
        uint8_t mask = GetMask(bitset, i);
        for(size_t j = 0; j < 4; ++j) {
            bool flag = GetBit(bitset, i+j);
            bool mflag = mask & (1 << j);
            ASSERT_EQ(flag, mflag) << i << ", " << j << ": Mask is " << (int)mask << " Bitwise and with " << (1 << j) << " yields " << mflag << " and should be " << flag;
        }
    }
}