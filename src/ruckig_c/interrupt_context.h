#ifndef RUCKIG_C_INTERRUPT_CONTEXT_H
#define RUCKIG_C_INTERRUPT_CONTEXT_H

#include "ruckig_c/platform_clock.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct ruckig_interrupt_context {
    bool enabled;
    bool interrupted;
    uint64_t start_us;
    double duration_us;
} ruckig_interrupt_context_t;

static inline ruckig_interrupt_context_t ruckig_interrupt_context_start(
    const ruckig_input_t* input,
    bool allow_interrupt
) {
    ruckig_interrupt_context_t context;
    context.enabled = allow_interrupt && input && input->has_interrupt_calculation_duration;
    context.interrupted = false;
    context.start_us = context.enabled ? ruckig_platform_monotonic_time_us() : 0u;
    context.duration_us = context.enabled ? input->interrupt_calculation_duration : 0.0;
    return context;
}

static inline bool ruckig_interrupt_context_elapsed(const ruckig_interrupt_context_t* context) {
    uint64_t now_us;
    double elapsed_us;
    if (!context || !context->enabled) {
        return false;
    }
    now_us = ruckig_platform_monotonic_time_us();
    elapsed_us = now_us >= context->start_us ? (double)(now_us - context->start_us) : 0.0;
    return elapsed_us >= context->duration_us;
}

static inline bool ruckig_interrupt_context_check(ruckig_interrupt_context_t* context) {
    if (!context || context->interrupted || !ruckig_interrupt_context_elapsed(context)) {
        return false;
    }
    context->interrupted = true;
    return true;
}

#endif
