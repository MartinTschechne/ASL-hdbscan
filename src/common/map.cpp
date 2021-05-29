#include <common/map.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

Map* Map_create() {
    Map* m = (Map*)malloc(sizeof(*m));
    m->size = 0;
    m->capacity = MAP_DEFAULT_CAPACITY;
    m->keys = OS_create();
    OS_resize(m->keys, m->capacity);
    m->values = vector_create();
    vector_resize(m->values, m->capacity);
    return m;
}

void Map_free(Map* m) {
    free(m->keys);
    free(m->values);
    free(m);
}

size_t Map_begin(const Map* m) {
    size_t idx = UNDEFINED_VALUE;
    if (m) {
        idx = 0;
    }
    return idx;
}

size_t Map_next(const Map* m, size_t idx) {
    size_t next = UNDEFINED_VALUE;
    if (m && idx >= 0) {
        if (idx < Map_size(m)) {
            next = idx + 1;
        }
        else {
            next = Map_end(m);
        }
    }
    return next;
}

size_t Map_prev(const Map* m, size_t idx) {
    size_t prev = UNDEFINED_VALUE;
    if (m && idx <= Map_end(m)) {
        if (idx > 0) {
            prev = idx - 1;
        }
        else if (idx == 0) {
            prev = Map_end(m);
        }
    }
    return prev;
}

size_t Map_end(const Map* m) {
    size_t idx = UNDEFINED_VALUE;
    if (m) {
        idx = Map_size(m);
    }
    return idx;
}

void* Map_get_idx(const Map* m, size_t idx) {
    void* value = nullptr;
    if (m && idx >= 0 && idx < Map_size(m)) {
        value = vector_get(m->values, idx);
    }
    return value;
}

size_t Map_key_from_idx(const Map* m, size_t idx) {
    if(!m) return UNDEFINED_VALUE;
    return OS_get(m->keys, idx);
}

void* Map_get(const Map* m, size_t key) {
    size_t pos = Map_find(m, key);
    if (pos != Map_end(m)) {
        return Map_get_idx(m, pos);
    }
    return nullptr;
}

bool Map_empty(const Map* m) {
    return (Map_size(m) == 0);
}

size_t Map_size(const Map* m) {
    return m->size;
}

void Map_clear(Map* m) {
    if (m && Map_size(m) > 0) {
        m->size = 0;
        OS_clear(m->keys);
        vector_clear(m->values);
    }
}

size_t Map_insert(Map* m, size_t key, void* value) {
    size_t pos = UNDEFINED_VALUE;
    if (m) {
        if (!OS_contains(m->keys, key)) {
            pos = OS_insert(m->keys, key);
            vector_insert(m->values, pos, 1, value);
            m->size++;
        }
    }
    return pos;
}

size_t Map_insert_AVX(Map* m, size_t key, void* value) {
    size_t pos = UNDEFINED_VALUE;
    if (m) {
        if (!OS_contains(m->keys, key)) {
            pos = OS_insert_AVX(m->keys, key);
            vector_insert_AVX(m->values, pos, 1, value);
            m->size++;
        }
    }
    return pos;
}

size_t Map_erase(Map* m, size_t key) {
    size_t next_index = UNDEFINED_VALUE;
    if (m) {
        size_t pos = OS_find(m->keys, key);
        next_index = pos;
        if (pos != OS_end(m->keys)) {
            OS_erase_AVX(m->keys, key);
            vector_erase_AVX(m->values, pos);
            m->size--;
        }
    }
    return next_index;
}

size_t Map_erase_AVX(Map* m, size_t key) {
    size_t next_index = UNDEFINED_VALUE;
    if (m) {
        size_t pos = OS_find(m->keys, key);
        next_index = pos;
        if (pos != OS_end(m->keys)) {
            OS_erase(m->keys, key);
            vector_erase(m->values, pos);
            m->size--;
        }
    }
    return next_index;
}



size_t Map_find(const Map* m, size_t key) {
    return OS_find(m->keys, key);
}

bool Map_contains(const Map* m, size_t key) {
    bool contains = false;
    if (m && Map_size(m) > 0) {
        contains = (Map_find(m, key) != Map_end(m));
    }
    return contains;
}

void Map_print(const Map* m) {
    printf("size %lu (keys: %lu - values: %lu):\n",
        Map_size(m), OS_size(m->keys), vector_size(m->values));
    for (size_t i = 0; i < Map_size(m); i++) {
        printf("key: %lu - value: %p\n",
            OS_get(m->keys, i), (void*)vector_get(m->values, i));
    }
    printf("\n");
}