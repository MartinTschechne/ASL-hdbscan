#ifndef TEAM33_VECTOR_H
#define TEAM33_VECTOR_H

#define VECTOR_DEFAULT_CAPACITY 10

#include <cstddef>

typedef struct Vector {
    size_t capacity;
    size_t size;
    void** elements;
} vector;

/**
 * @brief Allocates memory for payload and initializes a vector with default
 * capacity 
 * 
 * @param vec Vector to initialize
 */
void vector_init(vector* vec);

/**
 * @brief Releases memory previously allocated for vector payload
 * 
 * @param vec Vector to free memory from
 */
void vector_free(vector* vec);

/**
 * @brief Returns the size of the vector
 * 
 * @param vec Vector to get size of
 * @return size_t Size of vector vec
 */
size_t vector_size(const vector* vec);

/**
 * @brief Gets pointer to value stored at given position in vector
 * 
 * @param vec Vector to get value from
 * @param pos Position of value in vector
 * @return void* Pointer to value at desired position in vector
 */
void* vector_get(const vector* vec, size_t pos);

/**
 * @brief Sets value at given position in vector 
 * 
 * @param vec Vector to set value in
 * @param pos Position of value in vector
 * @param value Pointer to value at desired position in vector
 */
void vector_set(vector* vec, size_t pos, const void* value);

/**
 * @brief Erases all elements from the vector. After this call, vector has
 * size zero
 * 
 * @param vec Vector to erase elements from
 */
void vector_clear(vector* vec);

/**
 * @brief Inserts count copies of given element at the specified position in 
 * the vector, shifting the element at the position and all subsequent 
 * elements to the right
 * 
 * @param vec Vector to insert element into
 * @param pos Position before which content will be inserted
 * @param count Number of copies of element to insert
 * @param value Pointer to value to insert
 */
void vector_insert(vector* vec, size_t pos, size_t count, const void* value);

bool vector_contains(vector* vec, const void* value);

/**
 * @brief Erases specified element from the vector
 * 
 * @param vec Vector to erase element from
 * @param pos Position at which to remove element
 */
void vector_erase(vector* vec, size_t pos);

/**
 * @brief Appends the given element to the end of the vector
 * 
 * @param vec Vector to append element to
 * @param value Value to be copied into appended element
 */
void vector_push_back(vector* vec, const void* value);

/**
 * @brief Removes the last element from the vector
 * 
 * @param vec Vector to remove last element from
 */
void vector_pop_back(vector* vec);

/**
 * @brief Resizes the vector to have a capacity of count elements
 * 
 * @param vec Vector to resize
 * @param count New size of the vector
 */
void vector_resize(vector* vec, size_t count);

#endif