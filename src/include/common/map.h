#ifndef TEAM33_MAP_H
#define TEAM33_MAP_H

#define MAP_DEFAULT_CAPACITY 1024
#define UNDEFINED_VALUE -1

#include <cstddef>
#include <common/ordered_set.h>
#include <common/vector.h>

typedef struct alignas(32) Map {
    size_t size;
    size_t capacity;
    OrderedSet* keys;
    vector* values;
} Map;

/**
 * @brief Allocates memory and initializes an empty map with default
 * capacity
 *
 * @return Map*
 */
Map* Map_create();

/**
 * @brief Releases memory previously allocated to map
 *
 * @param m Ordered map to free memory from
 */
void Map_free(Map* m);

/**
 * @brief Imitates iterator by returning index of first member of map
 *
 * @param m Map to get the index of the first member of
 * @return size_t Index of first member of map s
 */
size_t Map_begin(const Map* m);

/**
 * @brief Imitates iterator by returning next index for given index
 * of member of set
 *
 * @param m Map to get next index of given index from
 * @param idx Index to get followup index of
 * @return size_t Index of member which follows index idx in map s
 */
size_t Map_next(const Map* m, size_t idx);

/**
 * @brief Imitates reverse iterator by returning next index for given index
 * of member of set
 *
 * @param m Map to get previous index of given index from
 * @param idx Index to get preceeding index of
 * @return size_t Index of member which precedes index idx in map s
 */
size_t Map_prev(const Map* m, size_t idx);

/**
 * @brief Imitates iterator by returning index+1 of last member of set
 *
 * @param m Map to get last index of member of
 * @return size_t Index+1 of last member of map s
 */
size_t Map_end(const Map* m);

/**
 * @brief Gets value for a given index in a map
 *
 * @param m Map to get value from
 * @param idx Index to get value of
 * @return void* Value at given index
 */
void* Map_get_idx(const Map* m, size_t idx);

/**
 * @brief Gets key at a given index in a map
 *
 * @param m Map to get key from
 * @param idx Index at which to get key
 * @return size_t Key at index idx in map m
 */
size_t Map_key_from_idx(const Map* m, size_t idx);

/**
 * @brief Imitates operator[] by getting the value for a given key in a map
 *
 * @param m Map to get value from
 * @param idx Index from which to get the key from
 * @return size_t Key at index idx in map s
 */
void* Map_get(const Map* m, size_t key);

/**
 * @brief Checks if the map has no members
 *
 * @param m Map to check emptyness of
 * @return true If the map is empty
 * @return false Otherwise
 */
bool Map_empty(const Map* m);

/**
 * @brief Returns the number of members in the set
 *
 * @param m Map to get number of members of
 * @return size_t The numer of members in map s
 */
size_t Map_size(const Map* m);

/**
 * @brief Erases all members from the map. After this call, Map_size()
 * returns zero
 *
 * @param m Map to erase all members from
 */
void Map_clear(Map* m);

/**
 * @brief Inserts a key-value pair into an map if it's not already present
 *
 * @param m Map to insert key into
 * @param key Key to insert into map
 * @param value Value to insert into map
 * @return Index of the inserted key or the key that prevented the insertion
 */
size_t Map_insert(Map* m, size_t key, void* value);

/**
 * @brief Using AVX: Inserts a key-value pair into an map if it's not already
 * present
 *
 * @param m Map to insert key into
 * @param key Key to insert into map
 * @param value Value to insert into map
 * @return Index of the inserted key or the key that prevented the insertion
 */
size_t Map_insert_AVX(Map* m, size_t key, void* value);

/**
 * @brief Erases a key-value pair from an map if present
 *
 * @param m Map to erase key from
 * @param key Key to erase from map
 * @return Index following the last removed key
 */
size_t Map_erase(Map* m, size_t key);

/**
 * @brief Using AVX: Erases a key-value pair from an map if present
 *
 * @param m Map to erase key from
 * @param key Key to erase from map
 * @return Index following the last removed key
 */
size_t Map_erase_AVX(Map* m, size_t key);

/**
 * @brief Finds a key in an map
 *
 * @param m Map to find key in
 * @param key Key to find in map
 * @return size_t Iterator-imitating index of element in map if found
 */
size_t Map_find(const Map* m, size_t key);

/**
 * @brief  Checks if a key is member of an map
 *
 * @param m Map to check if key is member of
 * @param key Key to check map for
 * @return true If the key is contained in map s
 * @return false Otherwise
 */
bool Map_contains(const Map* m, size_t key);

/**
 * @brief Prints the map to screen for monkey brains
 *
 * @param m Map to print
 */
void Map_print(const Map* m);

#endif
