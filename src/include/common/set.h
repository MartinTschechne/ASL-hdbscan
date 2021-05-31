#ifndef TEAM33_SET_H
#define TEAM33_SET_H

#define SET_DEFAULT_CAPACITY 512
#define SET_MAX_USAGE_RATIO 0.25
#define UNDEFINED_VALUE -1

#include <cstddef>
#include <cinttypes>

// bitset - required for set:

typedef uint64_t bitword_t;
enum {
    bitword_size = 8ULL * sizeof(bitword_t)
};
typedef struct alignas(32) BitSet {
    bitword_t* words;
    size_t num_bits;
    size_t num_words;
} bitset;

/**
 * @brief Allocates memory and initializes an empty zeroed bitset with set number of bits
 *
 * @param num_bits Number of bits in the bitset
 * @return bitset* Pointer to the initialized bitset
 */
bitset* bitset_create(size_t num_bits);

/**
 * @brief Releases memory allocated to a bitset
 *
 * @param b Bitset to release memory from
 */
void bitset_free(bitset* b);

/**
 * @brief Sets a bit in a bitset
 *
 * @param b Bitset to set bit in
 * @param bit Bit to set
 */
void bitset_set(bitset* b, size_t bit);

/**
 * @brief Clears a it in a bitset
 *
 * @param b Bitset to clear bit in
 * @param bit Bit to clear
 */
void bitset_clear(bitset* b, size_t bit);

/**
 * @brief Retrieves a bit from a bitset
 *
 * @param b Bitset to retrieve bit from
 * @param bit Bit to retrieve
 * @return size_t Content of bit
 */
size_t bitset_get(bitset* b, size_t bit);

/**
 * @brief Clears (i.e.) zeroes a bitset
 *
 * @param b Bitset to clear
 */
void bitset_clear_all(bitset* b);

/**
 * @brief Resizes a bitset to a given capacity
 *
 * @param b Bitset to resize
 * @param capacity New capacity of bitset
 */
void bitset_resize(bitset* b, size_t capacity);

/**
 * @brief Returns the bitword index of a given bit
 *
 * @param bit Bit to get the bitword index of
 * @return size_t Index of bitword which bit belongs to
 */
static inline size_t bit_idx(size_t bit);

/**
 * @brief Returns the offset of a bit within a bitword
 *
 * @param bit Bit to get the offset within a word of
 * @return size_t Offset of bit within the bitword bit belongs to
 */
static inline size_t bit_offset(size_t bit);

// set:

typedef struct alignas(32) Set {
    size_t capacity;
    size_t size;
    size_t* elements;
    bitset* bit_elements;
} set;

/**
 * @brief Allocates memory and initializes an empty set with default capacity
 *
 * @return set*
 */
set* set_create();

/**
 * @brief Releases memory previously allocated to set
 *
 * @param s Set to free memory from
 */
void set_free(set* s);

/**
 * @brief Imitates iterator by returning index of first member of set
 *
 * @param s Set to get the index of the first member of
 * @return size_t Index of first member of set s
 */
size_t set_begin(const set* s);

/**
 * @brief Imitates iterator by returning next index for given index
 * of member of set
 *
 * @param s Set to get next index of given index from
 * @param idx Index to get followup index of
 * @return size_t Index of member which follows index idx in set s
 */
size_t set_next(const set* s, size_t idx);

/**
 * @brief Imitates reverse iterator by returning next index for given index
 * of member of set
 *
 * @param s Set to get previous index of given index from
 * @param idx Index to get preceeding index of
 * @return size_t Index of member which precedes index idx in set s
 */
size_t set_prev(const set* s, size_t idx);

/**
 * @brief Imitates iterator by returning index+1 of last member of set
 *
 * @param s Set to get last index of member of
 * @return size_t Index+1 of last member of set s
 */
size_t set_end(const set* s);

/**
 * @brief Imitates iterator by getting the key at a given index in a set
 *
 * @param s Set to get key from
 * @param idx Index from which to get the key from
 * @return size_t Key at index idx in set s
 */
size_t set_get(const set* s, size_t idx);

/**
 * @brief Checks if the set has no members
 *
 * @param s Set to check emptyness of
 * @return true If the set is empty
 * @return false Otherwise
 */
bool set_empty(const set* s);

/**
 * @brief Returns the number of members in the set
 *
 * @param s Set to get number of members of
 * @return size_t The numer of members in set s
 */
size_t set_size(const set* s);

/**
 * @brief Erases all members from the set. After this call, set_size()
 * returns zero
 *
 * @param s Set to erase all members from
 */
void set_clear(set* s);

/**
 * @brief Inserts a value into a set if its not already present
 *
 * @param s Set to insert value into
 * @param value Value to insert into set
 */
void set_insert(set* s, size_t value);

/**
 * @brief Erases a key from a set if present
 *
 * @param s Set to erase key from
 * @param key Key to erase from set
 */
void set_erase(set* s, size_t key);

/**
 * @brief Finds a key in a set
 *
 * @param s Set to find key in
 * @param key Key to find in set
 * @return size_t Iterator-imitating index of element in set if found
 */
size_t set_find(const set* s, size_t key);

/**
 * @brief  Checks if a key is member of a set
 *
 * @param s Set to check if key is member of
 * @param key Key to check set for
 * @return true If the key is contained in set s
 * @return false Otherwise
 */
bool set_contains(const set* s, size_t value);

/**
 * @brief Resize the set to capacity
 *
 * @param s Set to resize
 * @param capacity Target capacity of resized set
 */
void set_resize(set* s, size_t capacity);

/**
 * @brief Reorder the set starting from given index to keep ordering
 *
 * @param s Set to reorder
 * @param idx Index to start reordering from
 */
static void set_reorder(set* s, size_t idx);

/**
 * @brief Removes the key at an index from a set
 *
 * @param s Set to remove key from
 * @param idx Index at which to remove key
 */
static void set_clear_idx(set* s, size_t idx);

/**
 * @brief Finds the next occupied index in a set
 *
 * @param s Set to find next occupied index in
 * @param idx Start index from which the next occupied index is wanted
 * @return size_t Next occupied index in set s
 */
static size_t set_next_busy_idx(const set* s, size_t idx);

/**
 * @brief Finds the next unoccupied index in a set
 *
 * @param s Set to find next unoccupied index in
 * @param idx Start index from which the next unoccupied index is wanted
 * @return size_t Next unoccupied index in set s
 */
static size_t set_next_open_idx(const set* s, size_t idx);

#endif
