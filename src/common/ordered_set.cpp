#include <common/ordered_set.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>


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

void OS_insert(OrderedSet* os, size_t key) {
    if (os) {
        if (OS_size(os) > 0) {
            size_t pos = OS_bisect_right(os, key, 0, OS_end(os));
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
            os->elements[0] = key;
            os->size = 1;
        }
    }
}

void OS_erase(OrderedSet* os, size_t key) {
    if (os) {
        size_t pos = 0;
        if (OS_size(os) > 0) {
            pos = OS_bisect_right(os, key, 0, OS_end(os)) - 1;
        }
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
}

size_t OS_find(const OrderedSet* os, size_t key) {
    return OS_find_btw(os, key, 0, OS_end(os));
}

size_t OS_find_btw(const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t idx = UNDEFINED_VALUE;
    if (os) {
        size_t pos = OS_bisect_right(os, key, lo, hi) - 1;
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

static size_t OS_bisect_right(
    const OrderedSet* os, size_t key, size_t lo, size_t hi) {
    size_t mid;
    while (lo < hi) {
        mid = (lo + hi) / 2;
        if (key < OS_get(os, mid)) {
            hi = mid;
        }
        else {
            lo = mid + 1;
        }
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
    printf("size %d: ",OS_size(os));
    for (size_t i = 0; i<OS_size(os);i++) {
        printf("%d ", os->elements[i]);
    }
    printf("\n");
}