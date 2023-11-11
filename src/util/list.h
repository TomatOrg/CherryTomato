#pragma once

#include <stdbool.h>

typedef struct list_entry {
    struct list_entry* next;
    struct list_entry* prev;
} list_entry_t;

typedef list_entry_t list_t;

#define INIT_LIST(list) ((list_t){ .next = &(list), .prev = &(list) })

#define CR(Record, TYPE, Field)  ((TYPE *) ((uint8_t*) (Record) - offsetof(TYPE, Field)))

static inline bool list_is_empty(list_t* list) {
    return list->next == list;
}

static inline void list_insert_tail(list_t* head, list_entry_t* entry) {
    entry->next = head;
    entry->prev = head->prev;
    entry->prev->next = entry;
    head->prev = entry;
}

static inline void list_remove(list_entry_t* entry) {
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = NULL;
    entry->prev = NULL;
}
