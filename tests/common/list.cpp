#include "gtest/gtest.h"
#include <common/list.h>

TEST(list, list_init_free) {
    list* l = list_create();
    ASSERT_EQ(l->head, nullptr);
    ASSERT_EQ(l->tail, nullptr);

    list_free(l);
}    

TEST(list, node_init_free) {
    node* n = node_create(99);
    ASSERT_EQ(n->element, 99);
    ASSERT_EQ(n->prev, nullptr);
    ASSERT_EQ(n->next, nullptr);

    node_free(n);
}    

TEST(list, size_push_front_back) {
    list* l = list_create();
    ASSERT_EQ(list_size(l), 0);

    list_push_front(l, 99);
    ASSERT_EQ(l->head->next, nullptr);
    ASSERT_EQ(l->tail->prev, nullptr);
    ASSERT_EQ(list_size(l), 1);
    ASSERT_EQ(list_front(l), 99); 
    ASSERT_EQ(list_back(l), 99);

    list_push_front(l, 77);
    ASSERT_EQ(list_front(l), 77); 
    ASSERT_EQ(list_back(l), 99);

    list_push_back(l, 11);
    ASSERT_EQ(list_back(l), 11);

    list_free(l);
} 

TEST(list, clear_pop_front_back) {
    list* l = list_create();
    list_clear(l);
    ASSERT_EQ(l->head, nullptr);
    ASSERT_EQ(l->tail, nullptr);
    ASSERT_EQ(list_size(l), 0); 

    list_push_front(l, 99);
    list_clear(l);
    ASSERT_EQ(l->head, nullptr);
    ASSERT_EQ(l->tail, nullptr);
    ASSERT_EQ(list_size(l), 0); 

    list_push_front(l, 99);
    list_push_front(l, 77);
    list_push_back(l, 11);
    ASSERT_EQ(list_size(l), 3);

    list_clear(l);
    ASSERT_EQ(l->head, nullptr);
    ASSERT_EQ(l->tail, nullptr);
    ASSERT_EQ(list_size(l), 0); 

    list_push_front(l, 99);
    list_push_front(l, 77);
    list_push_back(l, 11);
    list_pop_front(l);
    ASSERT_EQ(list_front(l), 99);
    ASSERT_EQ(list_size(l), 2);

    list_pop_back(l);
    ASSERT_EQ(list_front(l), 99);
    ASSERT_EQ(list_size(l), 1);

    list_pop_front(l);
    ASSERT_EQ(list_size(l), 0);

    list_pop_front(l);
    ASSERT_EQ(list_size(l), 0);

    list_pop_back(l);
    ASSERT_EQ(list_size(l), 0);

    list_pop_back(l);
    ASSERT_EQ(list_size(l), 0);

    list_push_front(l, 99);
    list_pop_back(l); 
    ASSERT_EQ(list_size(l), 0);

    list_pop_back(l);
    ASSERT_EQ(list_size(l), 0);

    list_pop_front(l);
    ASSERT_EQ(list_size(l), 0);

    list_free(l);
}
