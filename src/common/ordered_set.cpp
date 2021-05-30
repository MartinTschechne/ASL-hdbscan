#include <common/ordered_set.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <immintrin.h>
#include <common/vector_reductions.h>

OrderedSet* OS_create() {
    OrderedSet* os = (OrderedSet*)malloc(sizeof(*os));
    os->capacity = ORDERED_SET_DEFAULT_CAPACITY;
    os->size = 0;
    os->elements = (size_t*)malloc(os->capacity * sizeof(*os->elements));
    return os;
}

void OS_free(OrderedSet* os) {
    free(os->elements);
    free(os);
}

size_t OS_begin(const OrderedSet* os) {
    size_t idx = UNDEFINED_VALUE;
    if (os) {
        idx = 0;
    }
    return idx;
}

size_t OS_next(const OrderedSet* os, size_t idx) {
    size_t next = UNDEFINED_VALUE;
    if (os && idx >= 0) {
        if (idx < OS_size(os)) {
            next = idx + 1;
        }
        else {
            next = OS_end(os);
        }
    }
    return next;
}

size_t OS_prev(const OrderedSet* os, size_t idx) {
    size_t prev = UNDEFINED_VALUE;
    if (os && idx <= OS_end(os)) {
        if (idx > 0) {
            prev = idx - 1;
        }
        else if (idx == 0) {
            prev = OS_end(os);
        }
    }
    return prev;
}

size_t OS_end(const OrderedSet* os) {
    size_t idx = UNDEFINED_VALUE;
    if (os) {
        idx = OS_size(os);
    }
    return idx;
}

size_t OS_get(const OrderedSet* os, size_t idx) {
    size_t key = UNDEFINED_VALUE;
    if (os && idx >= 0 && idx < OS_size(os)) {
        key = os->elements[idx];
    }
    return key;
}

bool OS_empty(const OrderedSet* os) {
    return (OS_size(os) == 0);
}

size_t OS_size(const OrderedSet* os) {
    return os->size;
}

void OS_clear(OrderedSet* os) {
    if (os && OS_size(os) > 0) {
        os->size = 0;
    }
}

size_t OS_insert(OrderedSet* os, size_t key) {
    size_t pos = UNDEFINED_VALUE;
    if (os) {
        if (OS_size(os) > 0) {
            pos = OS_bisect_right(os, key, 0, OS_end(os));
            if (pos == 0 || os->elements[pos-1] != key) {
                if (OS_size(os) == os->capacity) {
                    OS_resize(os, 2 * os->capacity);
                }
                for (size_t i = OS_size(os); i > pos; i--) {
                    os->elements[i] = os->elements[i-1];
                    }
                os->elements[pos] = key;
                os->size++;
            }
        }
        else {
            pos = 0;
            os->elements[pos] = key;
            os->size = 1;
        }
    }
    return pos;
}

size_t OS_insert_AVX(OrderedSet* os, size_t key) {
    size_t pos = UNDEFINED_VALUE;
    if (os) {
        if (OS_size(os) > 0) {
            pos = OS_bisect_right(os, key, 0, OS_end(os));
            if (pos == 0 || os->elements[pos-1] != key) {
                if (OS_size(os) == os->capacity) {
                    OS_resize(os, 2 * os->capacity);
                }
                size_t i = OS_size(os);
#ifdef __AVX__
                __m256i el_vec_0, el_vec_1;
                for (; i >= pos + 8; i -= 8) {
                    el_vec_0 = _mm256_loadu_si256(
                        (const __m256i*)&os->elements[i-8]);
                    el_vec_1 = _mm256_loadu_si256(
                        (const __m256i*)&os->elements[i-4]);
                    _mm256_storeu_si256(
                        (__m256i*)&os->elements[i-7], el_vec_0);
                    _mm256_storeu_si256(
                        (__m256i*)&os->elements[i-3], el_vec_1);
                }
                for (; i >= pos + 4; i -= 4) {
                    el_vec_0 = _mm256_loadu_si256(
                        (const __m256i*)&os->elements[i-4]);
                    _mm256_storeu_si256(
                        (__m256i*)&os->elements[i-3], el_vec_0);
                }
#endif
                for (; i > pos; i--) {
                    os->elements[i] = os->elements[i-1];
                    }
                os->elements[pos] = key;
                os->size++;
            }
        }
        else {
            pos = 0;
            os->elements[pos] = key;
            os->size = 1;
        }
    }
    return pos;
}

size_t OS_erase(OrderedSet* os, size_t key) {
    size_t next_index = UNDEFINED_VALUE;
    if (os) {
        size_t pos = 0;
        if (OS_size(os) > 0) {
            pos = OS_bisect_right(os, key, 0, OS_end(os)) - 1;
        }
        next_index = pos;
        if (os->elements[pos] == key) {
            for (; pos < OS_end(os)-1; pos++) {
                os->elements[pos] = os->elements[pos+1];
            }
            os->size--;
            if (0 < OS_size(os) &&
                (OS_size(os) == (size_t)(0.25 * os->capacity))) {
                OS_resize(os, (size_t)(0.5 * os->capacity));
            }
        }
    }
    return next_index;
}

size_t OS_erase_AVX(OrderedSet* os, size_t key) {
    size_t next_index = UNDEFINED_VALUE;
    if (os) {
        long int pos = 0;
        const long int size = (long int)OS_size(os);
        if (size > 0) {
            pos = (long int)OS_bisect_right(os, key, 0, OS_end(os)) - 1;
        }
        next_index = (size_t)pos;
        if (os->elements[pos] == key) {
#ifdef __AVX__
            __m256i el_vec_0, el_vec_1;
            for (; pos < size - 8; pos += 8) {
                el_vec_0 = _mm256_loadu_si256(
                    (const __m256i*)&os->elements[pos+1]);
                el_vec_1 = _mm256_loadu_si256(
                    (const __m256i*)&os->elements[pos+5]);
                _mm256_storeu_si256(
                    (__m256i*)&os->elements[pos], el_vec_0);
                _mm256_storeu_si256(
                    (__m256i*)&os->elements[pos+4], el_vec_1);
            }
            for (; pos < size - 4; pos += 4) {
                el_vec_0 = _mm256_loadu_si256(
                    (const __m256i*)&os->elements[pos+1]);
                _mm256_storeu_si256(
                    (__m256i*)&os->elements[pos], el_vec_0);
            }
#endif
            for (; pos < size - 1; pos++) {
                os->elements[pos] = os->elements[pos+1];
            }
            os->size--;
            if (0 < OS_size(os) &&
                (OS_size(os) == (size_t)(0.25 * os->capacity))) {
                OS_resize(os, (size_t)(0.5 * os->capacity));
            }
        }
    }
    return next_index;
}

size_t OS_find(const OrderedSet* os, size_t key) {
    return OS_find_btw(os, key, 0, OS_end(os));
}

size_t OS_find_AVX(const OrderedSet* os, size_t key) {
    return OS_find_btw_AVX(os, key, 0, OS_end(os));
}

size_t OS_find_btw(const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t idx = UNDEFINED_VALUE;
    if (os) {
        size_t pos;
        if ((hi - lo) <= LINEAR_BINARY_SEARCH_XOVER) {
            pos = OS_linear_right(os, key, lo, hi);
        }
        else {
            pos = OS_bisect_right(os, key, lo, hi);
        }
        if (pos > 0) {
            pos--;
        }
        if (os->elements[pos] == key) {
            idx = pos;
        }
        else {
            idx = OS_end(os);
        }
    }
    return idx;
}

size_t OS_find_btw_AVX(const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t idx = UNDEFINED_VALUE;
    if (os) {
        size_t pos;
        if ((hi - lo) <= LINEAR_BINARY_SEARCH_XOVER) {
            pos = OS_linear_right_AVX(os, key, lo, hi);
        }
        else {
            pos = OS_bisect_right_AVX(os, key, lo, hi);
        }
        if (pos > 0) {
            pos--;
        }
        if (os->elements[pos] == key) {
            idx = pos;
        }
        else {
            idx = OS_end(os);
        }
    }
    return idx;
}

bool OS_contains(const OrderedSet* os, size_t key) {
    bool contains = false;
    if (os && OS_size(os) > 0) {
        contains = (OS_find(os, key) != OS_end(os));
    }
    return contains;
}

bool OS_contains_AVX(const OrderedSet* os, size_t key) {
    bool contains = false;
    if (os && OS_size(os) > 0) {
        contains = (OS_find_AVX(os, key) != OS_end(os));
    }
    return contains;
}

size_t OS_bisect_right(
    const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t mid;
    while (lo < hi) {
        mid = (lo + hi) >> 1;
        if (key < OS_get(os, mid)) {
            hi = mid;
        }
        else {
            lo = mid + 1;
        }
    }
    return lo;
}

size_t OS_bisect_right_AVX(
    const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t mid;
    while (lo < hi) {
        mid = (lo + hi) >> 1;
        if (key < OS_get(os, mid)) {
            hi = mid;
        }
        else {
            lo = mid + 1;
        }
    }
    return lo;
}

size_t OS_linear_right(
    const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t i = lo;
    for (; i < hi; i++) {
        lo += (key >= os->elements[i]);
    }
    return lo;
    // const size_t tmp = os->elements[hi];
    // os->elements[hi] = __SIZE_MAX__;
    // size_t i = lo;
    // while (true) {
    //     if (os->elements[i] > key) {
    //         os->elements[hi] = tmp;
    //         return i;
    //     }
    //     i++;
    // }
}

size_t OS_linear_right_AVX(
    const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t i = lo;
#ifdef __AVX2__
    if ((hi - lo) > 11) {
        const __m256i key_vec = _mm256_set1_epi64x((long long int)key);
        __m256i el_vec_0, el_vec_1, cmp_0, cmp_1,
            cmp_accum_0 = _mm256_setzero_si256(),
            cmp_accum_1 = _mm256_setzero_si256();
        for (; i < hi - 7; i += 8) {
            el_vec_0 = _mm256_loadu_si256(
                (const __m256i*)&os->elements[i]);
            el_vec_1 = _mm256_loadu_si256(
                (const __m256i*)&os->elements[i+4]);
            cmp_0 = _mm256_or_si256(_mm256_cmpgt_epi64(key_vec, el_vec_0),
                _mm256_cmpeq_epi64(key_vec, el_vec_0));
            cmp_1 = _mm256_or_si256(_mm256_cmpgt_epi64(key_vec, el_vec_1),
                _mm256_cmpeq_epi64(key_vec, el_vec_1));
            cmp_accum_0 = _mm256_add_epi64(cmp_accum_0, cmp_0);
            cmp_accum_1 = _mm256_add_epi64(cmp_accum_1, cmp_1);
        }
        cmp_accum_0 = _mm256_add_epi64(cmp_accum_0, cmp_accum_1);
        for (; i < hi - 3; i += 4) {
            el_vec_0 = _mm256_loadu_si256(
                (const __m256i*)&os->elements[i]);
            cmp_0 = _mm256_or_si256(_mm256_cmpgt_epi64(key_vec, el_vec_0),
                _mm256_cmpeq_epi64(key_vec, el_vec_0));
            cmp_accum_0 = _mm256_add_epi64(cmp_accum_0, cmp_0);
        }
        lo = -_mm256_reduce_sum_epi64(cmp_accum_0);
    }
#endif
    for (; i < hi; i++) {
        lo += (key >= os->elements[i]);
    }
    return lo;
}

void OS_resize(OrderedSet* os, size_t capacity) {
    if (os) {
        os->elements = (size_t*)realloc(
            os->elements, capacity * sizeof(*os->elements));
        os->capacity = capacity;
    }
}

void OS_print(const OrderedSet* os) {
    printf("size %lu: ",OS_size(os));
    for (size_t i = 0; i < OS_size(os); i++) {
        printf("%lu ", os->elements[i]);
    }
    printf("\n");
}
