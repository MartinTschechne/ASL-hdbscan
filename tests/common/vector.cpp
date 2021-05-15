#include "gtest/gtest.h"
#include <common/vector.h>

TEST(vector, init_free) {
    vector v;
    vector_init(&v);

    ASSERT_EQ(vector_size(&v), 0);

    vector_free(&v);
}    

TEST(vector, push_back_size_get) {
    vector v;
    vector_init(&v);

    const char words[6][10] = {
        "This",
        "is",
        "a",
        "test",
        "of",
        "C vector"
        };
    
    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }
   
    ASSERT_EQ(vector_size(&v), 6);

    for (size_t i = 0; i < vector_size(&v); i++) {
        ASSERT_EQ((char*)vector_get(&v, i), words[i]);
    }

     for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }

    ASSERT_EQ(vector_size(&v), 12);
    
    vector_free(&v);
}        

TEST(vector, set_insert_erase_pop_back) {
    vector v;
    vector_init(&v);

    const char words[6][10] = {
        "This",
        "is",
        "a",
        "test",
        "of",
        "C vector"
        };
    
    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }

    const char word[] = "astrophage";
    
    vector_set(&v, 0, word);
    ASSERT_EQ((char*)vector_get(&v, 0), word);

    vector_insert(&v, 20, 1, word);
    ASSERT_EQ(vector_size(&v), 6); 

    vector_insert(&v, 6, 3, words[1]);
    ASSERT_EQ(vector_size(&v), 9);  

    vector_insert(&v, 0, 3, words[3]);
    ASSERT_EQ(vector_size(&v), 12);  

    for (int i = 0; i < 3; i++) {
        vector_erase(&v, 1);
    }
    ASSERT_EQ(vector_size(&v), 9);
    ASSERT_EQ((char*)vector_get(&v, 0), words[3]); 

    for (int i = 0; i < 3; i++) {
        vector_pop_back(&v);
    }
    ASSERT_EQ(vector_size(&v), 6); 
    ASSERT_EQ((char*)vector_get(&v, 5), words[5]); 

    vector_free(&v);
}        

TEST(vector, resize_clear) {
    vector v;
    vector_init(&v);

    const char words[6][10] = {
        "This",
        "is",
        "a",
        "test",
        "of",
        "C vector"
        };

    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, words[i]);
    }
    vector_insert(&v, 2, 300, words[5]);
    ASSERT_EQ(vector_size(&v), 306);
    ASSERT_EQ(v.capacity, 612);

    for (int i = 0; i < 300; i++) {
        vector_pop_back(&v);
    }
    ASSERT_EQ(v.capacity, 19);

    vector_clear(&v);
    ASSERT_EQ(vector_size(&v), 0); 

    vector_free(&v);
}        

TEST(vector, primitive_types) {
    vector v;
    vector_init(&v);

    const int a[] = {5, 4, 3, 9, 8, 7};
    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, &a[i]);
    }
   
    ASSERT_EQ(vector_size(&v), 6);

    for (size_t i = 0; i < vector_size(&v); i++) {
        ASSERT_EQ(*(int*)vector_get(&v, i), a[i]);
    }

    vector_clear(&v);

    const double b[] = {5., 4., 3., 9., 8., 7.};
    for (int i = 0; i < 6; i++) {
        vector_push_back(&v, &b[i]);
    }
   
    ASSERT_EQ(vector_size(&v), 6);

    for (size_t i = 0; i < vector_size(&v); i++) {
        ASSERT_EQ(*(double*)vector_get(&v, i), b[i]);
    }

    vector_free(&v);
}
