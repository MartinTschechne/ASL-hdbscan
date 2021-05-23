#include <common/vector.h>
#include <cstdlib>
#include <cstdio>

void vector_init(vector* vec) {
    if (vec) {
        vec->capacity = VECTOR_DEFAULT_CAPACITY;
        vec->size = 0;
        vec->elements = (void**)malloc(vec->capacity * sizeof(void*));
    }
}

vector* vector_create() {
    vector* v = (vector*)malloc(sizeof(*v));
    v->capacity = VECTOR_DEFAULT_CAPACITY;
    v->size = 0;
    v->elements = (void**)malloc(v->capacity * sizeof(*v->elements));
    return v;
}

void vector_free(vector* vec) {
    if (vec) {
        vec->capacity = 0;
        vec->size = 0;
        free(vec->elements);
        vec->elements = nullptr;
    }
}

bool vector_empty(const vector* const vec) {
    if(!vec) return true;
    return vec->size == 0;
}

size_t vector_size(const vector* vec) {
    size_t size = -1;
    if (vec) {
        size = vec->size;
    }
    return size;
}

void* vector_get(const vector* vec, size_t pos) {
    void* value = nullptr;
    if (vec) {
        if (0 <= pos && pos < vec->size) {
            value = vec->elements[pos];
        }
    }
    return value;
}

void vector_set(vector* vec, size_t pos, const void* value) {
    if (vec) {
        if (0 <= pos && pos < vec->size) {
            vec->elements[pos] = (void*)value;
        }
    }
}

void vector_clear(vector* vec) {
    if (vec) {
        vec->size = 0;
    }

}

bool vector_contains(vector* vec, const void* value) {
    for(size_t i = 0; i < vec->size; ++i) {
        if(vec->elements[i] == value) {
            return true;
        }
    }
    return false;
}

void vector_insert(vector* vec, size_t pos, size_t count, const void* value) {
    if (vec) {
        if (0 <= pos && pos <= vec->size) {
            if (vec->size + count > vec->capacity) {
                vector_resize(vec, 2 * (vec->size + count));
            }
            for (size_t i = vec->size+count-1; i > pos+count-1; i--) {
                vec->elements[i] = vec->elements[i-count];
            }
            for (size_t i = 0; i < count; i++) {
                vec->elements[pos+i] = (void*)value;
            }
            vec->size += count;
        }
    }
}

void vector_erase(vector* vec, size_t pos) {
    if (vec) {
        if (0 <= pos && pos < vec->size) {
            for (size_t i = pos; i < vec->size-1; i++) {
                vec->elements[i] = vec->elements[i+1];
            }
            vec->size--;
        }
        if (0 < vec->size && vec->size == (size_t)(vec->capacity / 4)) {
            vector_resize(vec, (size_t)(vec->capacity / 2));
        } 
    }
}

void vector_push_back(vector* vec, const void* value) {
    if (vec) {
        if (vec->size == vec->capacity) {
            vector_resize(vec, 2 * vec->size);
        }
        vec->elements[vec->size++] = (void*)value;
    }
}

void vector_pop_back(vector* vec) {
    if (vec) {
        if (vec->size > 0) {
            vec->size--;
        }
        if (0 < vec->size && vec->size == (size_t)(vec->capacity / 4)) {
            vector_resize(vec, (size_t)(vec->capacity / 2));
        }
    }
}

void vector_resize(vector* vec, size_t count) {
    if (vec) {
        void** elements = (void**)realloc(
            vec->elements, count * sizeof(void*));
        if (elements) {
            vec->elements = elements;
            vec->capacity = count;
        }
    }
}
