#ifndef TEAM33_LIST_H
#define TEAM33_LIST_H

#include <cstddef>

#define UNDEFINED_VALUE -1

typedef struct Node {
    struct Node* prev;
    struct Node* next;
    size_t element;
} node;

typedef struct List {
    node* head;
    node* tail;
} list;

/**
 * @brief Creates and initializes a list of primitive type size_t
 * 
 * @return list* Pointer to list
 */
list* list_create();

/**
 * @brief Frees memory previously allocated to a list
 * 
 * @param l List to free memory from
 */
void list_free(list* l);

/**
 * @brief Creates and initializes a list node with a value of type size_t
 * 
 * @param value Value of list node
 * @return node* Pointer to node
 */
node* node_create(size_t value);

/**
 * @brief Frees memory previously allocated to a node
 * 
 * @param n Node to free memory from
 */
void node_free(node* n);

/**
 * @brief Erases all elements from the list
 * 
 * @param l List to erase elements from
 */
void list_clear(list* l);

/**
 * @brief Accesses the first element of a list.
 *  Calling on an empty list is undefined
 * 
 * @param l List to get the first element from
 * @return size_t First element of list l
 */
size_t list_front(const list* l);

/**
 * @brief Accesses the last element of a list.
 * Calling on an empty list is undefined 
 * 
 * @param l List to get the last element from
 * @return size_t Last element of list l
 */
size_t list_back(const list* l);

/**
 * @brief Checks if the list contains no elements
 * 
 * @param l The list to check emptyness of
 * @return true If the list l is empty
 * @return false Otherwise
 */
bool list_empty(const list* l);

/**
 * @brief Returns the number of elements in the list
 * 
 * @param l List to get size of
 * @return size_t The number of elements in list l
 */
size_t list_size(const list* l);

/**
 * @brief Prepends the given element value to the beginning of the list
 * 
 * @param l List to prepend element to
 * @param value The element to prepend to list l
 */
void list_push_front(list* l, size_t value);

/**
 * @brief Appends the given element value to the back of the list
 * 
 * @param l List to append element to
 * @param value The element to append to list l
 */
void list_push_back(list* l, size_t value);

/**
 * @brief Removes the first element of the list
 * 
 * @param l List to remove first element from
 */
void list_pop_front(list* l);

/**
 * @brief Removes the last element of the list
 * 
 * @param l List to remove last element from
 */
void list_pop_back(list* l);

#endif