#include <common/vector.h>
#include <cstdlib>
#include <immintrin.h>
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

bool vector_contains(const vector* vec, const void* value) {
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

void vector_insert_AVX(
    vector* vec, size_t pos, size_t count, const void* value) {
    if (vec) {
        if (0 <= pos && pos <= vec->size) {
            if (vec->size + count > vec->capacity) {
                vector_resize(vec, 2 * (vec->size + count));
            }
            size_t i = vec->size + count - 1;
#ifdef __AVX__
            __m256i el_vec_0, el_vec_1;
            for (; i >= pos + count + 7; i -= 8) {
                el_vec_0 = _mm256_loadu_si256(
                    (const __m256i*)&vec->elements[i-7-count]);
                el_vec_1 = _mm256_loadu_si256(
                    (const __m256i*)&vec->elements[i-3-count]);
                _mm256_storeu_si256(
                   (__m256i*)&vec->elements[i-7], el_vec_0);
                _mm256_storeu_si256(
                   (__m256i*)&vec->elements[i-3], el_vec_1);
            }
            for (; i >= pos + count + 3; i -= 4) {
                el_vec_0 = _mm256_loadu_si256(
                    (const __m256i*)&vec->elements[i-3-count]);
                _mm256_storeu_si256(
                   (__m256i*)&vec->elements[i-3], el_vec_0);
            }
#endif
            for (; i > pos + count - 1; i--) {
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

void vector_erase_AVX(vector* vec, size_t pos) {
    if (vec) {
        if (0 <= pos && pos < vec->size) {
            long int i = (long int)pos;
            const long int size = (long int)vec->size;
#ifdef __AVX__
            __m256i el_vec_0, el_vec_1;
            for (; i < size - 8; i += 8) {
                el_vec_0 = _mm256_loadu_si256(
                    (const __m256i*)&vec->elements[i+1]);
                el_vec_1 = _mm256_loadu_si256(
                    (const __m256i*)&vec->elements[i+5]);
                _mm256_storeu_si256(
                    (__m256i*)&vec->elements[i], el_vec_0);
                _mm256_storeu_si256(
                    (__m256i*)&vec->elements[i+4], el_vec_1);
            }
            for (; i < size - 4; i += 4) {
                el_vec_0 = _mm256_loadu_si256(
                    (const __m256i*)&vec->elements[i+1]);
                _mm256_storeu_si256(
                    (__m256i*)&vec->elements[i], el_vec_0);
            }
#endif
            for (; i < size - 1; i++) {
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
