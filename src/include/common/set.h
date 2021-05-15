#ifndef TEAM33_SET_H
#define TEAM33_SET_H

#define SET_DEFAULT_CAPACITY 10
#define SET_MAX_USAGE 0.25
#define UNDEFINED_VALUE -1

#include <cstddef>

typedef struct SetNode {
    size_t element;
} set_node;

typedef struct Set {
    size_t capacity;
    size_t size;
    void** elements;
} set;

/**
 * @brief Allocates memory and initializes a set with default capacity
 * 
 * @param s Set to initialize
 */
void set_init(set* s);

/**
 * @brief Releases memory previously allocated to set
 * 
 * @param s Set to free memory from
 */
void set_free(set* s);

/**
 * @brief Imitates iterator by returning first element of set
 * 
 * @param s Set to get first element of
 * @return size_t Value of first element of set s
 */
size_t set_begin(const set* s);

/**
 * @brief Imitates iterator by returning following element of set
 * 
 * @param s Set to get next element of
 * @param value Value to get next element of
 * @return size_t Value of element which follows value in set s
 */
size_t set_next(const set* s, size_t value);

/**
 * @brief Imitates iterator by returning last element of set
 * 
 * @param s Set to get last element of
 * @return size_t Value of last element of set s
 */
size_t set_end(const set* s);


/**
 * @brief Checks if the set has no elements
 * 
 * @param s Set to check emptyness of
 * @return true If the set is empty
 * @return false Otherwise
 */
bool set_empty(const set* s);

/**
 * @brief Returns the number of elements in the set
 * 
 * @param s Set to get number of elements of
 * @return size_t The numer of elements in the set
 */
size_t set_size(const set* s);

/**
 * @brief Erases all elements from the set. After this call, set_size()
 * returns zero
 * 
 * @param s Set to erase all elements from
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
 * @brief Erase a value from a set if present
 * 
 * @param s Set to erase value from
 * @param value Value to erase from set
 */
void set_erase(set* s, size_t value);

/**
 * @brief Find a value in a set
 * 
 * @param s Set to find value in 
 * @param value Value to find in set
 * @return size_t TBD
 */
size_t set_find(const set* s, size_t value);

#endif