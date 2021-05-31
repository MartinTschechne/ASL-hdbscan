#include <common/list.h>
#include <cstdlib>


list* list_create() {
    list* new_list = (list*)aligned_alloc(32, sizeof(list));
    new_list->head = nullptr;
    new_list->tail = nullptr;
    return new_list;
}

void list_free(list* l) {
    if (l) {
        node* current = l->head;
        node* next = nullptr;
        while (current) {
            next = current->next;
            node_free(current);
            current = next;
        }
        l->head = nullptr;
        l->tail = nullptr;
        free(l);
    }
}

node* node_create(size_t value) {
    node* new_node = (node*)aligned_alloc(32, sizeof(node));
    new_node->prev = nullptr;
    new_node->next = nullptr;
    new_node->element = value;
    return new_node;
}

void node_free(node* n) {
    if (n) {
        n->prev = nullptr;
        n->next = nullptr;
        n->element = 0;
        free(n);
    }
}

void list_clear(list* l) {
    if (l) {
       while (l->head) {
           list_pop_front(l);
       }
       l->tail = nullptr;
    }
}

size_t list_front(const list* l) {
    size_t front = UNDEFINED_VALUE;
    if (l) {
        front = l->head->element;
    }
    return front;
}

size_t list_back(const list* l) {
    size_t back = UNDEFINED_VALUE;
    if (l) {
        back = l->tail->element;
    }
    return back;
}

bool list_empty(const list* l) {
    bool is_empty = true;
    if (l) {
        is_empty = !(l->head);
    }
    return is_empty;
}

size_t list_size(const list* l) {
    size_t size = UNDEFINED_VALUE;
    if (l) {
        size = 0;
        node* current = l->head;
        while(current) {
            current = current->next;
            size++;
        }
    }
    return size;
}

void list_push_front(list* l, size_t value) {
    if (l) {
        node* new_node = node_create(value);
        new_node->next = l->head;
        if (l->head) {
            l->head->prev = new_node;
        }
        l->head = new_node;
        if (!l->tail) {
            l->tail = new_node;
        }
    }
}

void list_push_back(list* l, size_t value) {
    if (l) {
        node* new_node = node_create(value);
        new_node->prev = l->tail;
        if (l->tail) {
            l->tail->next = new_node;
        }
        l->tail = new_node;
        if (!l->head) {
            l->head = new_node;
        }
    }
}

void list_pop_front(list* l) {
    if (l) {
        if (l->head) {
            node* pop_node = l->head;
            if (l->head->next) {
                l->head->next->prev = nullptr;
                l->head = l->head->next;
            }
            else {
                l->head = nullptr;
                l->tail = nullptr;
            }
            node_free(pop_node);
        }
    }
}

void list_pop_back(list* l) {
    if (l) {
        if (l->tail) {
            node* pop_node = l->tail;
            if (l->tail->prev) {
                l->tail->prev->next = nullptr;
                l->tail = l->tail->prev;
            }
            else {
                l->tail = nullptr;
                l->head = nullptr;
            }
            node_free(pop_node);
        }
    }
}
