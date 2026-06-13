#ifndef RUCKIG_C_ALLOC_H
#define RUCKIG_C_ALLOC_H

#include <stdbool.h>
#include <stddef.h>

void* ruckig_calloc(size_t count, size_t size);
void ruckig_free(void* ptr);

size_t ruckig_allocation_count(void);
size_t ruckig_free_count(void);
void ruckig_allocation_counters_reset(void);
void ruckig_allocation_forbidden_set(bool forbidden);
size_t ruckig_allocation_forbidden_count(void);

static inline double* ruckig_allocate_double_vector(size_t count) {
    return (double*)ruckig_calloc(count, sizeof(double));
}

#endif
