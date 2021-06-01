#ifndef TEAM33_COMMON_BITSET_H
#define TEAM33_COMMON_BITSET_H

#include <stdlib.h>
#include <cstdio>

#define BITS_PER_SLOT (8 * sizeof(u_int8_t))

typedef u_int8_t* BitSet_t;

inline BitSet_t CreateBitset(size_t size) {
    return (BitSet_t)calloc(size / 8 + 2, sizeof(u_int8_t));
}

inline BitSet_t CreateBitset(size_t size, bool value) {
    BitSet_t bitset = (BitSet_t)calloc(size / 8 + 2, sizeof(u_int8_t));
    u_int8_t default_value = value ? 0xff : 0; 
    for(size_t i = 0; i < size / 8 + 1; ++i) {
        bitset[i] = default_value;
    }
    return bitset;
}

inline void SetBit(BitSet_t bitset, size_t index, bool value) {
    if(value) bitset[index / BITS_PER_SLOT] |= (1 << (index % BITS_PER_SLOT));
    else bitset[index / BITS_PER_SLOT] &= ~(1 << (index % BITS_PER_SLOT));
}

inline bool GetBit(BitSet_t bitset, size_t index) {
    return bitset[index / BITS_PER_SLOT] & (1 << (index % BITS_PER_SLOT));
}

inline uint8_t GetMask(BitSet_t bitset, size_t index) {
    if(index % 8 == 0) {
        return bitset[index / BITS_PER_SLOT];
    }
    if(index % 8 == 4) {
        return bitset[index / BITS_PER_SLOT] >> 4;
    }
    printf("\n\n\n\n\nILLEGAL STATE\n\n\n\n");
    return 0;
}

inline void PrintBitset(BitSet_t bitset, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        printf("%d", GetBit(bitset, i));
    }
    printf("\n");
}




#endif