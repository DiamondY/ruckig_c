#include "ruckig_c/alloc.h"

#include <stdlib.h>

static size_t allocation_count = 0;
static size_t free_count = 0;
static size_t forbidden_allocation_count = 0;
static bool allocation_forbidden = false;

void* ruckig_calloc(size_t count, size_t size) {
    void* ptr = calloc(count, size);
    if (ptr) {
        ++allocation_count;
        if (allocation_forbidden) {
            ++forbidden_allocation_count;
        }
    }
    return ptr;
}

void ruckig_free(void* ptr) {
    if (ptr) {
        ++free_count;
    }
    free(ptr);
}

size_t ruckig_allocation_count(void) {
    return allocation_count;
}

size_t ruckig_free_count(void) {
    return free_count;
}

void ruckig_allocation_counters_reset(void) {
    allocation_count = 0;
    free_count = 0;
    forbidden_allocation_count = 0;
    allocation_forbidden = false;
}

void ruckig_allocation_forbidden_set(bool forbidden) {
    allocation_forbidden = forbidden;
}

size_t ruckig_allocation_forbidden_count(void) {
    return forbidden_allocation_count;
}
