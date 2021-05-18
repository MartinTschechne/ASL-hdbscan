#include <common/set.h>
#include <cstdlib>
#include <cstring>

// bitset - required for set:

bitset* bitset_create(size_t num_bits) {
    bitset* b = (bitset*)malloc(sizeof(*b));
    b->num_bits = num_bits;
    b->num_words = (size_t)(num_bits / bitword_size) + 1;
    b->words = (bitword_t*)malloc(b->num_words * sizeof(*b->words));
    bitset_clear_all(b);
    return b;
}

void bitset_free(bitset* b) {
    free(b->words);
    free(b);
}

void bitset_set(bitset* b, size_t bit) {
    b->words[bit_idx(bit)] |= (1ULL << bit_offset(bit));
}

void bitset_clear(bitset* b, size_t bit) {
    b->words[bit_idx(bit)] &= ~(1ULL << bit_offset(bit));
}

size_t bitset_get(bitset* b, size_t bit) {
    return b->words[bit_idx(bit)] & (1ULL << bit_offset(bit));
}

void bitset_clear_all(bitset* b) {
    memset(b->words, 0, b->num_words * sizeof(*b->words));
}

void bitset_resize(bitset* b, size_t capacity) {
    b->words = (bitword_t*)realloc(
            b->words, capacity * sizeof(*b->words));
    for (size_t i = b->num_bits; i < capacity; i++) {
        bitset_clear(b, i);
    }
    b->num_bits = capacity;
    b->num_words = (size_t)(capacity / bitword_size) + 1; 
}

static inline size_t bit_idx(size_t bit) {
    return (size_t)(bit / bitword_size);
}

static inline size_t bit_offset(size_t bit) {
    return (size_t)(bit % bitword_size);
}

// set:

set* set_create() {
    set* s = (set*)malloc(sizeof(*s));
    s->capacity = SET_DEFAULT_CAPACITY;
    s->size = 0;
    s->elements = (size_t*)malloc(s->capacity * sizeof(size_t));
    s->bit_elements = bitset_create(s->capacity);
    return s;
}

void set_free(set* s) {
    free(s->elements);
    free(s->bit_elements);
    free(s);
}

size_t set_begin(const set* s) {
    size_t idx = UNDEFINED_VALUE;
    if (s) {
        if (s->size > 0) {
            idx = 0;
            while (idx < s->capacity && !bitset_get(s->bit_elements, idx)) {
                idx++;
            }
        }
        else {
            idx = set_end(s);
        }
    }
    return idx;
}

size_t set_next(const set* s, size_t idx) {
    size_t next = UNDEFINED_VALUE;
    if (s && s->capacity > 0) {
        next = idx + 1;
        while (next < s->capacity) {
            if (bitset_get(s->bit_elements, next)) {
                return next;
            }
            next++;
        }
        next = idx + 1;
    }
    return next;
}

// size_t set_prev(const set* s, size_t idx) {
//     size_t prev = UNDEFINED_VALUE;
//     if (s && s->capacity > 0) {
//         prev = idx - 1;
//         while (prev >= 0) {
//             if (bitset_get(s->bit_elements, prev)) {
//                 return prev;
//             }
//             prev--;
//         }
//         prev = set_end(s);
//     }
//     return prev;
// }

size_t set_end(const set* s) {
    size_t idx = UNDEFINED_VALUE;
    if (s) {
        if (s->size > 0) {
            idx = s->capacity;
            while (0 < idx && !bitset_get(s->bit_elements, idx-1)) {
                idx--;
            }
        }
        else {
            idx = 0;
        }
    }
    return idx;
}

size_t set_get(const set* s, size_t idx) {
    size_t key = UNDEFINED_VALUE;
    if (s && s->size > 0) {
        if (bitset_get(s->bit_elements, idx)) {
            key = s->elements[idx];
        }
    }
    return key;
}

bool set_empty(const set* s) {
    return (s->size == 0);
}

size_t set_size(const set* s) {
    return s->size;
}

void set_clear(set* s) {
    if (s && s->size > 0)  {
        s->size = 0;
        bitset_clear_all(s->bit_elements);
    }
}

void set_insert(set* s, size_t key) {
    if (s) {
        if (!set_contains(s, key)) {
            if ((float)(s->size / s->capacity) > SET_MAX_USAGE_RATIO) {
                set_resize(s, 2 * s-> capacity);
                }
            size_t idx = key % s->capacity;
            idx = set_next_open_idx(s, idx);
            s->elements[idx] = key;
            bitset_set(s->bit_elements, idx);
            s->size++;
        }
    }
}

void set_erase(set* s, size_t key) {
    if (s && s->size > 0) {
        size_t idx = set_find(s, key);
        if (idx != set_end(s)) {
            set_clear_idx(s, idx);
            s->size--;
        }
    }
}

size_t set_find(const set* s, size_t key) {
    size_t idx = UNDEFINED_VALUE;
    if (s && s->size > 0) {
        idx = key % s->capacity; 
        if (!bitset_get(s->bit_elements, idx)) {
            idx = set_next_busy_idx(s, idx);
        }
        size_t start_idx = idx;
        do {
            idx = set_next_busy_idx(s, idx);
            if (s->elements[idx] == key) {
                return idx;
            }
        }  while (idx != start_idx);
        idx = set_end(s);
    }
    return idx;
}

bool set_contains(const set* s, size_t key) {
    bool contains = false;
    if (s && s->size > 0) {
        contains = (set_find(s, key) != set_end(s));
    }
    return contains;
}

void set_resize(set* s, size_t capacity) {
    if (s && (capacity > s->capacity)) {
        s->elements = (size_t*)realloc(
            s->elements, capacity * sizeof(size_t));
        bitset_resize(s->bit_elements, capacity);
        s->capacity = capacity;
        set_reorder(s, 0);
    }
}

static void set_reorder(set* s, size_t idx) {
    if (s) {
        size_t next = idx;
        while (next < s->capacity) {
            if (bitset_get(s->bit_elements, next)) {
                size_t key = s->elements[next];
                size_t target_idx = key % s->capacity;
                if (next != target_idx) {
                    target_idx = set_next_open_idx(s, target_idx);
                    if (target_idx < s->capacity) {
                        s->elements[target_idx] = s->elements[next];
                        bitset_set(s->bit_elements, target_idx);
                        set_clear_idx(s, next);
                        bitset_clear(s->bit_elements, next);
                    }
                }
            }
            next++;
        }
    }
}

static void set_clear_idx(set* s, size_t idx) {
    if (s) {
        s->elements[idx] = UNDEFINED_VALUE;
        bitset_clear(s->bit_elements, idx);
    }
}

static size_t set_next_busy_idx(const set* s, size_t idx) {
    size_t next = UNDEFINED_VALUE;
    if (s && s->capacity > 0) {
        next = idx + 1;
        while (true) {
            if (next == s->capacity) {
                next = 0;
            }
            if (next == idx) {
                break;
            }
            if (bitset_get(s->bit_elements, next)) {
                break;
            }
            next++;
        }
    }
    return next;
}

static size_t set_next_open_idx(const set* s, size_t idx) {
    size_t next = idx;
    while (bitset_get(s->bit_elements, next) && next < s->capacity) {
        next++;
        if (next == s->capacity) {
            next = 0;
        }
    }
    return next;
}
