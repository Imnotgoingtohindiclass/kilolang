#include "gc.h"
#include <stdlib.h>
#include <string.h>

typedef struct Obj {
    uint8_t marked;           // mark bit for gc
    struct Obj *next;         // linked list of all objects
    size_t size;              // size of allocated data
    char data[];              // flexible array member
} Obj;

static Obj *head = NULL;      // head of allocation list

void gc_init(void) { head = NULL; } // reset gc state

void *gc_alloc(size_t sz) {
    // todo: add null check for malloc
    Obj *o = malloc(sizeof(Obj) + sz);
    o->marked = 0;
    o->size = sz;
    o->next = head;
    head = o;
    return o->data;
}

static void mark_range(void **start, void **end) {
    // todo: reverse args if needed depending on stack growth
    for (void **p = start; p < end; ++p)
        for (Obj *o = head; o; o = o->next)
            if ((char*)*p >= (char*)o->data && (char*)*p < (char*)o->data + o->size)
                o->marked = 1; // conservative mark if pointer is within object
}

void gc_collect(void) {
    void *dummy;
    // todo: scan full stack, not just 1 word
    mark_range((void**)&dummy, (void**)(&dummy + 1));

    Obj **prev = &head;
    while (*prev) {
        Obj *cur = *prev;
        if (!cur->marked) {
            *prev = cur->next;
            free(cur); // free unreachable
        } else {
            cur->marked = 0; // unmark for next round
            prev = &cur->next;
        }
    }
    // todo: add stats/logging for freed objects
    // todo: protect against double free or memory corruption
}