#ifndef TEAM33_ORDERED_SET_H
#define TEAM33_ORDERED_SET_H

#define ORDERED_SET_DEFAULT_CAPACITY 1024
// #define ORDERED_SET_MAX_USAGE_RATIO 1.0
// #define ORDERED_SET_MIN_USAGE_RATIO 0.25
#define UNDEFINED_VALUE -1

#include <cstddef>

typedef struct OrderedSet {
    size_t capacity;
    size_t size;
    size_t* elements;
} OrderedSet;

/**
 * @brief Allocates memory and initializes an empty ordered set with default 
 * capacity
 * 
 * @return OrderedSet* 
 */
OrderedSet* OS_create();

/**
 * @brief Releases memory previously allocated to ordered set
 * 
 * @param os Ordered set to free memory from
 */
void OS_free(OrderedSet* os);

/**
 * @brief Imitates iterator by returning index of first member of ordered set
 * 
 * @param os Set to get the index of the first member of
 * @return size_t Index of first member of set s
 */
size_t OS_begin(const OrderedSet* os);

/**
 * @brief Imitates iterator by returning next index for given index
 * of member of set
 * 
 * @param os Set to get next index of given index from
 * @param idx Index to get followup index of
 * @return size_t Index of member which follows index idx in set s
 */
size_t OS_next(const OrderedSet* os, size_t idx);

/**
 * @brief Imitates reverse iterator by returning next index for given index
 * of member of set
 * 
 * @param os Set to get previous index of given index from
 * @param idx Index to get preceeding index of
 * @return size_t Index of member which precedes index idx in set s
 */
size_t OS_prev(const OrderedSet* os, size_t idx);

/**
 * @brief Imitates iterator by returning index+1 of last member of set
 * 
 * @param os Set to get last index of member of
 * @return size_t Index+1 of last member of set s
 */
size_t OS_end(const OrderedSet* os);

/**
 * @brief Imitates iterator by getting the key at a given index in a
 * ordered set
 * 
 * @param os Set to get key from 
 * @param idx Index from which to get the key from
 * @return size_t Key at index idx in set s
 */
size_t OS_get(const OrderedSet* os, size_t idx);

/**
 * @brief Checks if the set has no members
 * 
 * @param os Set to check emptyness of
 * @return true If the set is empty
 * @return false Otherwise
 */
bool OS_empty(const OrderedSet* os);

/**
 * @brief Returns the number of members in the set
 * 
 * @param os Set to get number of members of
 * @return size_t The numer of members in set s
 */
size_t OS_size(const OrderedSet* os);

/**
 * @brief Erases all members from the ordered set. After this call, OS_size()
 * returns zero
 * 
 * @param os Set to erase all members from
 */
void OS_clear(OrderedSet* os);

/**
 * @brief Inserts a key into an ordered set if it's not already present
 * 
 * @param os Set to insert key into
 * @param value Key to insert into set
 * @return Index of the inserted key or the key that prevented the insertion
 */
size_t OS_insert(OrderedSet* os, size_t key);

/**
 * @brief Erases a key from an ordered set if present
 * 
 * @param os Set to erase key from
 * @param key Key to erase from set
 * @return Index following the last removed key
 */
size_t OS_erase(OrderedSet* os, size_t key);

/**
 * @brief Finds a key in an ordered set
 * 
 * @param os Set to find key in 
 * @param key Key to find in set
 * @return size_t Iterator-imitating index of element in set if found
 */
size_t OS_find(const OrderedSet* os, size_t key);

/**
 * @brief Finds a key in an ordered set in a given index range 
 *
 * @param os Set to find key in 
 * @param key Key to find in set
 * @param lo Low end of search range index
 * @param hi Post high end of search range index
 * @return size_t Iterator-imitating index of element in set if found
 */
size_t OS_find_btw(const OrderedSet* os, size_t key, size_t lo, size_t hi);

/**
 * @brief  Checks if a key is member of an ordered set
 * 
 * @param os Set to check if key is member of
 * @param key Key to check set for
 * @return true If the key is contained in set s
 * @return false Otherwise
 */
bool OS_contains(const OrderedSet* os, size_t value); 

/**
 * @brief Returns the index where to insert key into ordered set
 * 
 * @param set Set to obtain insertion index for key of
 * @param key Key to insert
 * @param lo Index of start of search range for insertion index
 * @param hi Index past end of search range for insertion index
 * @return size_t 
 */
static size_t OS_bisect_right(
    const OrderedSet* set, size_t key, size_t lo, size_t hi);

/**
 * @brief Resizes the ordered set to capacity
 * 
 * @param os Set to resize
 * @param capacity Target capacity of resized set 
 */
void OS_resize(OrderedSet* os, size_t capacity);

/**
 * @brief Prints the ordered set to screen for monkey brains
 * 
 * @param os Set to print
 */
void OS_print(const OrderedSet* os);

#endif 
