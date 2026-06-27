#include "ruckig_c/alloc.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef RUCKIG_C_TESTING
static size_t allocation_count = 0;
static size_t free_count = 0;
static size_t forbidden_allocation_count = 0;
static bool allocation_forbidden = false;
#endif

void* ruckig_calloc(size_t count, size_t size) {
    void* ptr;
    if (count != 0u && size > SIZE_MAX / count) {
        return NULL;
    }
    ptr = calloc(count, size);
#ifdef RUCKIG_C_TESTING
    if (ptr) {
        ++allocation_count;
        if (allocation_forbidden) {
            ++forbidden_allocation_count;
        }
    }
#endif
    return ptr;
}

void ruckig_free(void* ptr) {
#ifdef RUCKIG_C_TESTING
    if (ptr) {
        ++free_count;
    }
#endif
    free(ptr);
}

size_t ruckig_allocation_count(void) {
#ifdef RUCKIG_C_TESTING
    return allocation_count;
#else
    return 0;
#endif
}

size_t ruckig_free_count(void) {
#ifdef RUCKIG_C_TESTING
    return free_count;
#else
    return 0;
#endif
}

void ruckig_allocation_counters_reset(void) {
#ifdef RUCKIG_C_TESTING
    allocation_count = 0;
    free_count = 0;
    forbidden_allocation_count = 0;
    allocation_forbidden = false;
#endif
}

void ruckig_allocation_forbidden_set(bool forbidden) {
#ifdef RUCKIG_C_TESTING
    allocation_forbidden = forbidden;
#else
    (void)forbidden;
#endif
}

size_t ruckig_allocation_forbidden_count(void) {
#ifdef RUCKIG_C_TESTING
    return forbidden_allocation_count;
#else
    return 0;
#endif
}
