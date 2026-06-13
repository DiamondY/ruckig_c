#include "ruckig_c/tracking_internal.h"

#include <float.h>
#include <string.h>

static void tracking_sequence_continuation_clear_state(ruckig_tracking_sequence_continuation_t* continuation) {
    if (!continuation) {
        return;
    }
    continuation->target_count = 0;
    continuation->completed_count = 0;
    continuation->active = false;
    continuation->was_interrupted = false;
    continuation->complete = false;
    continuation->optimized_step_active = false;
    continuation->optimized_improved = false;
    continuation->delta_time = 0.0;
    continuation->mode = RUCKIG_TRACKING_FAST;
    continuation->optimized_strategy = RUCKIG_TRACKING_OPTIMIZED_BALANCED;
    continuation->reactiveness = 1.0;
    continuation->look_ahead_cycles = 1;
    continuation->max_optimized_candidates = RUCKIG_TRACKING_DEFAULT_OPTIMIZED_CANDIDATES;
    continuation->optimized_phase = TRACKING_SEQUENCE_OPTIMIZED_IDLE;
    continuation->optimized_index = 0;
    continuation->optimized_window_count = 0;
    continuation->optimized_fast_score = 0.0;
    continuation->optimized_fast_terminal_position_error = 0.0;
    continuation->optimized_best_score = 0.0;
    memset(&continuation->diagnostics, 0, sizeof(continuation->diagnostics));
    memset(&continuation->optimized_step_diagnostics, 0, sizeof(continuation->optimized_step_diagnostics));
    continuation->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    continuation->diagnostics.mode = continuation->mode;
    continuation->diagnostics.optimized_strategy = continuation->optimized_strategy;
    continuation->optimized_step_diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    continuation->optimized_step_diagnostics.mode = continuation->mode;
    continuation->optimized_step_diagnostics.optimized_strategy = continuation->optimized_strategy;
    if (continuation->target_sequence) {
        ruckig_target_state_sequence_clear(continuation->target_sequence);
    }
    if (continuation->output_prefix) {
        ruckig_tracking_output_sequence_clear(continuation->output_prefix);
    }
}

static void tracking_sequence_assert_diagnostics_consistent(const ruckig_tracking_diagnostics_t* diagnostics) {
    const size_t family_count = diagnostics
        ? diagnostics->fast_candidate_count
            + diagnostics->instantaneous_candidate_count
            + diagnostics->horizon_candidate_count
            + diagnostics->terminal_blend_candidate_count
            + diagnostics->derivative_damped_candidate_count
            + diagnostics->lead_lag_candidate_count
        : 0;
    if (!diagnostics) {
        return;
    }
    RUCKIG_C_INTERNAL_ASSERT(family_count == diagnostics->candidate_count);
    RUCKIG_C_INTERNAL_ASSERT(diagnostics->valid_candidate_count + diagnostics->rejected_candidate_count <= diagnostics->candidate_count);
}

static void tracking_sequence_assert_continuation_consistent(const ruckig_tracking_sequence_continuation_t* continuation) {
    if (!continuation) {
        return;
    }
    RUCKIG_C_INTERNAL_ASSERT(continuation->dofs > 0);
    RUCKIG_C_INTERNAL_ASSERT(continuation->capacity > 0);
    RUCKIG_C_INTERNAL_ASSERT(continuation->target_count <= continuation->capacity);
    RUCKIG_C_INTERNAL_ASSERT(continuation->completed_count <= continuation->target_count);
    RUCKIG_C_INTERNAL_ASSERT(!continuation->active || continuation->completed_count < continuation->target_count);
    RUCKIG_C_INTERNAL_ASSERT(!continuation->complete || (!continuation->active && continuation->completed_count == continuation->target_count));
    RUCKIG_C_INTERNAL_ASSERT(!continuation->was_interrupted || (continuation->active && !continuation->complete));
    RUCKIG_C_INTERNAL_ASSERT(!continuation->optimized_step_active || continuation->mode == RUCKIG_TRACKING_OPTIMIZED);
    if (continuation->target_sequence) {
        RUCKIG_C_INTERNAL_ASSERT(continuation->target_sequence->dofs == continuation->dofs);
        RUCKIG_C_INTERNAL_ASSERT(continuation->target_sequence->capacity == continuation->capacity);
        RUCKIG_C_INTERNAL_ASSERT(continuation->target_sequence->count == continuation->target_count);
    }
    if (continuation->output_prefix) {
        RUCKIG_C_INTERNAL_ASSERT(continuation->output_prefix->dofs == continuation->dofs);
        RUCKIG_C_INTERNAL_ASSERT(continuation->output_prefix->capacity == continuation->capacity);
        RUCKIG_C_INTERNAL_ASSERT(continuation->output_prefix->count <= continuation->completed_count);
    }
    tracking_sequence_assert_diagnostics_consistent(&continuation->diagnostics);
    tracking_sequence_assert_diagnostics_consistent(&continuation->optimized_step_diagnostics);
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_sequence_continuation_create(
    ruckig_tracking_sequence_continuation_t** continuation,
    size_t dofs,
    size_t capacity
) {
    ruckig_tracking_sequence_continuation_t* value;
    if (!continuation || dofs == 0 || capacity == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *continuation = NULL;
    value = (ruckig_tracking_sequence_continuation_t*)ruckig_calloc(1, sizeof(*value));
    if (!value) {
        return RUCKIG_ERROR;
    }
    value->dofs = dofs;
    value->capacity = capacity;
    if (ruckig_input_create(&value->input, dofs) != RUCKIG_WORKING
        || ruckig_target_state_sequence_create(&value->target_sequence, dofs, capacity) != RUCKIG_WORKING
        || ruckig_tracking_output_sequence_create(&value->output_prefix, dofs, capacity) != RUCKIG_WORKING) {
        ruckig_tracking_sequence_continuation_destroy(value);
        return RUCKIG_ERROR;
    }
    value->optimized_best_position = ruckig_allocate_double_vector(dofs);
    value->optimized_best_velocity = ruckig_allocate_double_vector(dofs);
    value->optimized_best_acceleration = ruckig_allocate_double_vector(dofs);
    if (!value->optimized_best_position || !value->optimized_best_velocity || !value->optimized_best_acceleration) {
        ruckig_tracking_sequence_continuation_destroy(value);
        return RUCKIG_ERROR;
    }
    tracking_sequence_continuation_clear_state(value);
    tracking_sequence_assert_continuation_consistent(value);
    *continuation = value;
    return RUCKIG_WORKING;
}

RUCKIG_C_API void ruckig_tracking_sequence_continuation_destroy(
    ruckig_tracking_sequence_continuation_t* continuation
) {
    if (!continuation) {
        return;
    }
    ruckig_input_destroy(continuation->input);
    ruckig_target_state_sequence_destroy(continuation->target_sequence);
    ruckig_tracking_output_sequence_destroy(continuation->output_prefix);
    ruckig_free(continuation->optimized_best_position);
    ruckig_free(continuation->optimized_best_velocity);
    ruckig_free(continuation->optimized_best_acceleration);
    ruckig_free(continuation);
}

RUCKIG_C_API void ruckig_tracking_sequence_continuation_reset(
    ruckig_tracking_sequence_continuation_t* continuation
) {
    tracking_sequence_continuation_clear_state(continuation);
    tracking_sequence_assert_continuation_consistent(continuation);
}

RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_dof_count(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->dofs : 0;
}

RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_capacity(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->capacity : 0;
}

RUCKIG_C_API bool ruckig_tracking_sequence_continuation_is_active(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->active : false;
}

RUCKIG_C_API bool ruckig_tracking_sequence_continuation_was_interrupted(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->was_interrupted : false;
}

RUCKIG_C_API bool ruckig_tracking_sequence_continuation_is_complete(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->complete : false;
}

RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_completed_count(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->completed_count : 0;
}

RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_target_count(
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    return continuation ? continuation->target_count : 0;
}

static void tracking_sequence_copy_prefix(
    ruckig_tracking_output_sequence_t* dst,
    const ruckig_tracking_output_sequence_t* src
) {
    const size_t value_count = src && dst ? src->count * src->dofs : 0;
    if (!dst || !src || dst->dofs != src->dofs || src->count > dst->capacity) {
        return;
    }
    if (value_count > 0) {
        memcpy(dst->new_position, src->new_position, sizeof(double) * value_count);
        memcpy(dst->new_velocity, src->new_velocity, sizeof(double) * value_count);
        memcpy(dst->new_acceleration, src->new_acceleration, sizeof(double) * value_count);
        memcpy(dst->new_jerk, src->new_jerk, sizeof(double) * value_count);
        memcpy(dst->time, src->time, sizeof(double) * src->count);
        memcpy(dst->section, src->section, sizeof(size_t) * src->count);
        memcpy(dst->result, src->result, sizeof(ruckig_result_t) * src->count);
    }
    dst->count = src->count;
}

static void tracking_sequence_store_work_output(
    ruckig_tracking_output_sequence_t* sequence,
    size_t step,
    const ruckig_tracking_t* tracking,
    double delta_time,
    ruckig_result_t result
) {
    const size_t offset = step * tracking->dofs;
    memcpy(&sequence->new_position[offset], tracking->work_output->new_position, sizeof(double) * tracking->dofs);
    memcpy(&sequence->new_velocity[offset], tracking->work_output->new_velocity, sizeof(double) * tracking->dofs);
    memcpy(&sequence->new_acceleration[offset], tracking->work_output->new_acceleration, sizeof(double) * tracking->dofs);
    memcpy(&sequence->new_jerk[offset], tracking->work_output->new_jerk, sizeof(double) * tracking->dofs);
    sequence->time[step] = (double)(step + 1) * delta_time;
    sequence->section[step] = tracking->work_output->new_section;
    sequence->result[step] = result;
    sequence->count = step + 1;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    size_t step;
    ruckig_tracking_diagnostics_t aggregate;
    bool optimized_any_fallback = false;
    bool optimized_all_optimized = tracking && tracking->mode == RUCKIG_TRACKING_OPTIMIZED;
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_sequence || !input || !output_sequence
        || target_sequence->dofs != tracking->dofs || output_sequence->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > output_sequence->capacity
        || !ruckig_input_same_dofs(input, tracking->dofs)) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    memset(&aggregate, 0, sizeof(aggregate));
    aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    aggregate.mode = tracking->mode;
    aggregate.optimized_strategy = tracking->optimized_strategy;
    output_sequence->count = 0;
    ruckig_reset(tracking->otg);
    if (ruckig_input_copy_state(input, tracking->work_input) != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }

    for (step = 0; step < target_sequence->count; ++step) {
        const size_t offset = step * tracking->dofs;
        const size_t remaining = target_sequence->count - step;
        const size_t window_count = tracking->mode == RUCKIG_TRACKING_OPTIMIZED
            ? min_size(remaining, tracking->look_ahead_cycles)
            : 1;
        ruckig_result_t result;
        tracking_reset_diagnostics(tracking);
        if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
            result = evaluate_optimized_tracking(
                tracking,
                &target_sequence->position[offset],
                &target_sequence->velocity[offset],
                &target_sequence->acceleration[offset],
                window_count,
                tracking->work_input,
                tracking->work_output,
                false
            );
        } else if (tracking->mode == RUCKIG_TRACKING_FAST) {
            result = prepare_fast_tracking_input(
                tracking,
                &target_sequence->position[offset],
                &target_sequence->velocity[offset],
                &target_sequence->acceleration[offset],
                tracking->work_input
            );
            if (result == RUCKIG_WORKING) {
                tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
                tracking_note_valid_candidate(tracking);
                result = run_prepared_tracking_update(tracking, tracking->work_output, RUCKIG_TRACKING_CALCULATION_FAST, false);
            }
        } else {
            result = RUCKIG_ERROR_INVALID_INPUT;
        }
        {
            const bool step_marked_error = tracking->diagnostics.error_step_count > 0;
            tracking_accumulate_diagnostics(&aggregate, &tracking->diagnostics);
            if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
                tracking_mark_error(tracking);
                aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
                if (!step_marked_error) {
                    ++aggregate.error_step_count;
                }
                tracking->diagnostics = aggregate;
                tracking_sync_legacy_diagnostics(tracking);
                return result;
            }
        }
        if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
            if (tracking->diagnostics.calculation_status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
                optimized_any_fallback = true;
                optimized_all_optimized = false;
            } else if (tracking->diagnostics.calculation_status != RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
                optimized_all_optimized = false;
            }
        }
        memcpy(&output_sequence->new_position[offset], tracking->work_output->new_position, sizeof(double) * tracking->dofs);
        memcpy(&output_sequence->new_velocity[offset], tracking->work_output->new_velocity, sizeof(double) * tracking->dofs);
        memcpy(&output_sequence->new_acceleration[offset], tracking->work_output->new_acceleration, sizeof(double) * tracking->dofs);
        memcpy(&output_sequence->new_jerk[offset], tracking->work_output->new_jerk, sizeof(double) * tracking->dofs);
        output_sequence->time[step] = (double)(step + 1) * tracking->delta_time;
        output_sequence->section[step] = tracking->work_output->new_section;
        output_sequence->result[step] = result;
        output_sequence->count = step + 1;
        ruckig_output_pass_to_input(tracking->work_output, tracking->work_input);
    }
    if (tracking->mode == RUCKIG_TRACKING_FAST) {
        aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST;
    } else if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        aggregate.calculation_status = optimized_any_fallback || !optimized_all_optimized
            ? RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK
            : RUCKIG_TRACKING_CALCULATION_OPTIMIZED;
    }
    if (aggregate.fast_score > 0.0) {
        aggregate.improvement_ratio = (aggregate.fast_score - aggregate.best_score) / aggregate.fast_score;
    }
    tracking->diagnostics = aggregate;
    tracking_sync_legacy_diagnostics(tracking);
    return RUCKIG_WORKING;
}

static ruckig_result_t tracking_sequence_continuation_capture_start(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence,
    ruckig_tracking_sequence_continuation_t* continuation
) {
    const size_t value_count = target_sequence->count * target_sequence->dofs;
    tracking_sequence_continuation_clear_state(continuation);
    if (ruckig_input_copy_state(input, continuation->input) != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (ruckig_target_state_sequence_set_count(continuation->target_sequence, target_sequence->count) != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    memcpy(continuation->target_sequence->position, target_sequence->position, sizeof(double) * value_count);
    memcpy(continuation->target_sequence->velocity, target_sequence->velocity, sizeof(double) * value_count);
    memcpy(continuation->target_sequence->acceleration, target_sequence->acceleration, sizeof(double) * value_count);
    continuation->target_count = target_sequence->count;
    continuation->completed_count = 0;
    continuation->active = false;
    continuation->was_interrupted = false;
    continuation->complete = false;
    continuation->delta_time = tracking->delta_time;
    continuation->mode = tracking->mode;
    continuation->optimized_strategy = tracking->optimized_strategy;
    continuation->reactiveness = tracking->reactiveness;
    continuation->look_ahead_cycles = tracking->look_ahead_cycles;
    continuation->max_optimized_candidates = tracking->max_optimized_candidates;
    continuation->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    continuation->diagnostics.mode = tracking->mode;
    continuation->diagnostics.optimized_strategy = tracking->optimized_strategy;
    continuation->optimized_step_diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    continuation->optimized_step_diagnostics.mode = tracking->mode;
    continuation->optimized_step_diagnostics.optimized_strategy = tracking->optimized_strategy;
    ruckig_tracking_output_sequence_clear(output_sequence);
    tracking_sequence_assert_continuation_consistent(continuation);
    return RUCKIG_WORKING;
}

static void tracking_sequence_set_diagnostics(
    ruckig_tracking_t* tracking,
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    if (!tracking || !continuation) {
        return;
    }
    tracking->diagnostics = continuation->diagnostics;
    tracking_sync_legacy_diagnostics(tracking);
}

static ruckig_result_t tracking_sequence_process_fast(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    ruckig_result_t result;
    ruckig_tracking_diagnostics_t aggregate = continuation->diagnostics;
    ruckig_tracking_mode_t saved_mode = tracking->mode;
    ruckig_tracking_optimized_strategy_t saved_strategy = tracking->optimized_strategy;
    ruckig_interrupt_context_t interrupt_context = ruckig_interrupt_context_start(continuation->input, true);

    tracking->mode = RUCKIG_TRACKING_FAST;
    tracking->optimized_strategy = continuation->optimized_strategy;
    continuation->active = continuation->completed_count < continuation->target_count;
    continuation->complete = continuation->completed_count == continuation->target_count && continuation->target_count > 0;
    continuation->was_interrupted = false;
    output_sequence->count = 0;
    tracking_sequence_copy_prefix(output_sequence, continuation->output_prefix);

    if (continuation->complete) {
        aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST;
        aggregate.mode = RUCKIG_TRACKING_FAST;
        aggregate.optimized_strategy = continuation->optimized_strategy;
        continuation->diagnostics = aggregate;
        tracking_sequence_set_diagnostics(tracking, continuation);
        tracking->mode = saved_mode;
        tracking->optimized_strategy = saved_strategy;
        return RUCKIG_WORKING;
    }

    if (ruckig_input_copy_state(continuation->input, tracking->work_input) != RUCKIG_WORKING) {
        continuation->active = false;
        continuation->complete = false;
        aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
        aggregate.mode = RUCKIG_TRACKING_FAST;
        aggregate.optimized_strategy = continuation->optimized_strategy;
        continuation->diagnostics = aggregate;
        tracking_sequence_set_diagnostics(tracking, continuation);
        tracking->mode = saved_mode;
        tracking->optimized_strategy = saved_strategy;
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    ruckig_reset(tracking->otg);

    while (continuation->completed_count < continuation->target_count) {
        const size_t step = continuation->completed_count;
        const size_t offset = step * tracking->dofs;
        tracking_reset_diagnostics(tracking);
        result = prepare_fast_tracking_input(
            tracking,
            &continuation->target_sequence->position[offset],
            &continuation->target_sequence->velocity[offset],
            &continuation->target_sequence->acceleration[offset],
            tracking->work_input
        );
        if (result == RUCKIG_WORKING) {
            tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
            tracking_note_valid_candidate(tracking);
            result = run_prepared_tracking_update(
                tracking,
                tracking->work_output,
                RUCKIG_TRACKING_CALCULATION_FAST,
                false
            );
        }
        if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
            tracking_mark_error(tracking);
            tracking_accumulate_diagnostics(&aggregate, &tracking->diagnostics);
            aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
            aggregate.mode = RUCKIG_TRACKING_FAST;
            aggregate.optimized_strategy = continuation->optimized_strategy;
            continuation->active = false;
            continuation->complete = false;
            continuation->was_interrupted = false;
            continuation->diagnostics = aggregate;
            tracking_sequence_set_diagnostics(tracking, continuation);
            tracking->mode = saved_mode;
            tracking->optimized_strategy = saved_strategy;
            return result;
        }

        tracking_accumulate_diagnostics(&aggregate, &tracking->diagnostics);
        tracking_sequence_store_work_output(continuation->output_prefix, step, tracking, continuation->delta_time, result);
        ++continuation->completed_count;
        ruckig_output_pass_to_input(tracking->work_output, tracking->work_input);
        ruckig_output_pass_to_input(tracking->work_output, continuation->input);

        if (continuation->completed_count < continuation->target_count
            && ruckig_interrupt_context_check(&interrupt_context)) {
            ++aggregate.budget_exhausted_count;
            aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST;
            aggregate.mode = RUCKIG_TRACKING_FAST;
            aggregate.optimized_strategy = continuation->optimized_strategy;
            continuation->active = true;
            continuation->complete = false;
            continuation->was_interrupted = true;
            continuation->diagnostics = aggregate;
            tracking_sequence_copy_prefix(output_sequence, continuation->output_prefix);
            tracking_sequence_set_diagnostics(tracking, continuation);
            tracking->mode = saved_mode;
            tracking->optimized_strategy = saved_strategy;
            return RUCKIG_WORKING;
        }
    }

    aggregate.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST;
    aggregate.mode = RUCKIG_TRACKING_FAST;
    aggregate.optimized_strategy = continuation->optimized_strategy;
    continuation->active = false;
    continuation->complete = true;
    continuation->was_interrupted = false;
    continuation->diagnostics = aggregate;
    tracking_sequence_copy_prefix(output_sequence, continuation->output_prefix);
    tracking_sequence_set_diagnostics(tracking, continuation);
    tracking->mode = saved_mode;
    tracking->optimized_strategy = saved_strategy;
    return RUCKIG_WORKING;
}

static void tracking_sequence_copy_best_to_continuation(
    ruckig_tracking_sequence_continuation_t* continuation,
    const ruckig_tracking_t* tracking
) {
    memcpy(continuation->optimized_best_position, tracking->optimized_best_position, sizeof(double) * tracking->dofs);
    memcpy(continuation->optimized_best_velocity, tracking->optimized_best_velocity, sizeof(double) * tracking->dofs);
    memcpy(continuation->optimized_best_acceleration, tracking->optimized_best_acceleration, sizeof(double) * tracking->dofs);
}

static void tracking_sequence_copy_best_to_tracking(
    ruckig_tracking_t* tracking,
    const ruckig_tracking_sequence_continuation_t* continuation
) {
    memcpy(tracking->optimized_best_position, continuation->optimized_best_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->optimized_best_velocity, continuation->optimized_best_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->optimized_best_acceleration, continuation->optimized_best_acceleration, sizeof(double) * tracking->dofs);
}

static void tracking_sequence_publish_optimized_diagnostics(
    ruckig_tracking_t* tracking,
    const ruckig_tracking_sequence_continuation_t* continuation,
    bool include_step
) {
    ruckig_tracking_diagnostics_t diagnostics = continuation->diagnostics;
    if (include_step) {
        tracking_accumulate_diagnostics(&diagnostics, &continuation->optimized_step_diagnostics);
    }
    if (diagnostics.calculation_status == RUCKIG_TRACKING_CALCULATION_NONE && include_step) {
        diagnostics.calculation_status = continuation->optimized_step_diagnostics.calculation_status;
    }
    diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
    diagnostics.optimized_strategy = continuation->optimized_strategy;
    if (diagnostics.fast_score > 0.0) {
        diagnostics.improvement_ratio = (diagnostics.fast_score - diagnostics.best_score) / diagnostics.fast_score;
    } else {
        diagnostics.improvement_ratio = 0.0;
    }
    tracking->diagnostics = diagnostics;
    tracking_sync_legacy_diagnostics(tracking);
}

static void tracking_sequence_reset_optimized_step(
    ruckig_tracking_sequence_continuation_t* continuation
) {
    continuation->optimized_step_active = false;
    continuation->optimized_improved = false;
    continuation->optimized_phase = TRACKING_SEQUENCE_OPTIMIZED_IDLE;
    continuation->optimized_index = 0;
    continuation->optimized_window_count = 0;
    continuation->optimized_fast_score = DBL_MAX;
    continuation->optimized_fast_terminal_position_error = DBL_MAX;
    continuation->optimized_best_score = DBL_MAX;
    memset(&continuation->optimized_step_diagnostics, 0, sizeof(continuation->optimized_step_diagnostics));
    continuation->optimized_step_diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_NONE;
    continuation->optimized_step_diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
    continuation->optimized_step_diagnostics.optimized_strategy = continuation->optimized_strategy;
}

static ruckig_result_t tracking_sequence_begin_optimized_step(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation
) {
    const size_t remaining = continuation->target_count - continuation->completed_count;
    const size_t offset = continuation->completed_count * tracking->dofs;
    const tracking_strategy_config_t* config = tracking_strategy_config(continuation->optimized_strategy);
    ruckig_result_t result;
    tracking_sequence_reset_optimized_step(continuation);
    continuation->optimized_window_count = min_size(remaining, continuation->look_ahead_cycles);
    if (!config || continuation->optimized_window_count == 0
        || !finite_vector(&continuation->target_sequence->position[offset], continuation->optimized_window_count * tracking->dofs)
        || !finite_vector(&continuation->target_sequence->velocity[offset], continuation->optimized_window_count * tracking->dofs)
        || !finite_vector(&continuation->target_sequence->acceleration[offset], continuation->optimized_window_count * tracking->dofs)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (ruckig_input_copy_state(continuation->input, tracking->work_input) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    ruckig_reset(tracking->otg);
    tracking_reset_diagnostics(tracking);
    result = prepare_tracking_base(tracking, tracking->work_input);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    continuation->optimized_step_active = true;
    continuation->optimized_phase = TRACKING_SEQUENCE_OPTIMIZED_FAST;
    continuation->optimized_index = 0;
    continuation->optimized_fast_score = DBL_MAX;
    continuation->optimized_fast_terminal_position_error = DBL_MAX;
    continuation->optimized_best_score = DBL_MAX;
    continuation->optimized_improved = false;
    continuation->optimized_step_diagnostics = tracking->diagnostics;
    continuation->optimized_step_diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
    continuation->optimized_step_diagnostics.optimized_strategy = continuation->optimized_strategy;
    return RUCKIG_WORKING;
}

static ruckig_result_t tracking_sequence_step_optimized_candidate(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation
) {
    const size_t step_offset = continuation->completed_count * tracking->dofs;
    const double* target_position = &continuation->target_sequence->position[step_offset];
    const double* target_velocity = &continuation->target_sequence->velocity[step_offset];
    const double* target_acceleration = &continuation->target_sequence->acceleration[step_offset];
    const size_t window_count = continuation->optimized_window_count;
    const tracking_strategy_config_t* config = tracking_strategy_config(continuation->optimized_strategy);
    ruckig_result_t result = RUCKIG_WORKING;

    if (!config) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    tracking->diagnostics = continuation->optimized_step_diagnostics;
    tracking_sync_legacy_diagnostics(tracking);
    if (ruckig_input_copy_state(continuation->input, tracking->work_input) != RUCKIG_WORKING) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_tracking_base(tracking, tracking->work_input);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    tracking_sequence_copy_best_to_tracking(tracking, continuation);

    result = tracking_optimized_candidate_step(
        tracking,
        config,
        target_position,
        target_velocity,
        target_acceleration,
        window_count,
        &continuation->optimized_phase,
        &continuation->optimized_index,
        &continuation->optimized_fast_score,
        &continuation->optimized_fast_terminal_position_error,
        &continuation->optimized_best_score,
        &continuation->optimized_improved,
        NULL
    );
    if (result != RUCKIG_WORKING) {
        return result;
    }
    tracking_sequence_copy_best_to_continuation(continuation, tracking);

    continuation->optimized_step_diagnostics = tracking->diagnostics;
    continuation->optimized_step_diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
    continuation->optimized_step_diagnostics.optimized_strategy = continuation->optimized_strategy;
    return RUCKIG_WORKING;
}

static ruckig_result_t tracking_sequence_finish_optimized_step(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation
) {
    ruckig_result_t result;
    tracking->diagnostics = continuation->optimized_step_diagnostics;
    tracking_sync_legacy_diagnostics(tracking);
    tracking_sequence_copy_best_to_tracking(tracking, continuation);
    copy_best_to_work_input(tracking);
    if (tracking->audit_best_candidate_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
        ++tracking->audit_family_selected[tracking->audit_best_candidate_family];
    }
    tracking->diagnostics.best_score = continuation->optimized_best_score;
    tracking_finalize_score_diagnostics(tracking);
    result = run_prepared_tracking_update(
        tracking,
        tracking->work_output,
        continuation->optimized_improved
            ? RUCKIG_TRACKING_CALCULATION_OPTIMIZED
            : RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
        true
    );
    continuation->optimized_step_diagnostics = tracking->diagnostics;
    continuation->optimized_step_diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
    continuation->optimized_step_diagnostics.optimized_strategy = continuation->optimized_strategy;
    return result;
}

static ruckig_result_t tracking_sequence_process_optimized(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    ruckig_result_t result = RUCKIG_WORKING;
    ruckig_tracking_mode_t saved_mode = tracking->mode;
    ruckig_tracking_optimized_strategy_t saved_strategy = tracking->optimized_strategy;
    double saved_reactiveness = tracking->reactiveness;
    size_t saved_look_ahead_cycles = tracking->look_ahead_cycles;
    size_t saved_max_optimized_candidates = tracking->max_optimized_candidates;
    ruckig_interrupt_context_t interrupt_context = ruckig_interrupt_context_start(continuation->input, true);

    tracking->mode = RUCKIG_TRACKING_OPTIMIZED;
    tracking->optimized_strategy = continuation->optimized_strategy;
    tracking->reactiveness = continuation->reactiveness;
    tracking->look_ahead_cycles = continuation->look_ahead_cycles;
    tracking->max_optimized_candidates = continuation->max_optimized_candidates;
    continuation->active = continuation->completed_count < continuation->target_count;
    continuation->complete = continuation->completed_count == continuation->target_count && continuation->target_count > 0;
    continuation->was_interrupted = false;
    output_sequence->count = 0;
    tracking_sequence_copy_prefix(output_sequence, continuation->output_prefix);

    while (continuation->completed_count < continuation->target_count) {
        if (!continuation->optimized_step_active) {
            result = tracking_sequence_begin_optimized_step(tracking, continuation);
            if (result != RUCKIG_WORKING) {
                tracking_mark_error(tracking);
                continuation->active = false;
                continuation->complete = false;
                continuation->was_interrupted = false;
                continuation->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_ERROR;
                tracking_sequence_publish_optimized_diagnostics(tracking, continuation, true);
                goto restore_and_return;
            }
        }

        while (continuation->optimized_phase != TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP) {
            result = tracking_sequence_step_optimized_candidate(tracking, continuation);
            if (result != RUCKIG_WORKING) {
                tracking_mark_error(tracking);
                continuation->active = false;
                continuation->complete = false;
                continuation->was_interrupted = false;
                continuation->optimized_step_diagnostics = tracking->diagnostics;
                tracking_sequence_publish_optimized_diagnostics(tracking, continuation, true);
                goto restore_and_return;
            }
            if (continuation->optimized_phase != TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP
                && ruckig_interrupt_context_check(&interrupt_context)) {
                ++continuation->optimized_step_diagnostics.budget_exhausted_count;
                continuation->optimized_step_diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK;
                continuation->active = true;
                continuation->complete = false;
                continuation->was_interrupted = true;
                tracking_sequence_copy_prefix(output_sequence, continuation->output_prefix);
                tracking_sequence_publish_optimized_diagnostics(tracking, continuation, true);
                result = RUCKIG_WORKING;
                goto restore_and_return;
            }
        }

        result = tracking_sequence_finish_optimized_step(tracking, continuation);
        if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
            tracking_mark_error(tracking);
            continuation->active = false;
            continuation->complete = false;
            continuation->was_interrupted = false;
            continuation->optimized_step_diagnostics = tracking->diagnostics;
            tracking_sequence_publish_optimized_diagnostics(tracking, continuation, true);
            goto restore_and_return;
        }
        tracking_accumulate_diagnostics(&continuation->diagnostics, &continuation->optimized_step_diagnostics);
        tracking_sequence_store_work_output(
            continuation->output_prefix,
            continuation->completed_count,
            tracking,
            continuation->delta_time,
            result
        );
        ++continuation->completed_count;
        ruckig_output_pass_to_input(tracking->work_output, continuation->input);
        tracking_sequence_reset_optimized_step(continuation);
        tracking_sequence_copy_prefix(output_sequence, continuation->output_prefix);

        if (continuation->completed_count < continuation->target_count
            && ruckig_interrupt_context_check(&interrupt_context)) {
            ++continuation->diagnostics.budget_exhausted_count;
            continuation->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK;
            continuation->diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
            continuation->diagnostics.optimized_strategy = continuation->optimized_strategy;
            continuation->active = true;
            continuation->complete = false;
            continuation->was_interrupted = true;
            tracking_sequence_publish_optimized_diagnostics(tracking, continuation, false);
            result = RUCKIG_WORKING;
            goto restore_and_return;
        }
    }

    continuation->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_OPTIMIZED;
    if (continuation->diagnostics.fallback_step_count > 0
        || continuation->diagnostics.optimized_step_count != continuation->target_count) {
        continuation->diagnostics.calculation_status = RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK;
    }
    continuation->diagnostics.mode = RUCKIG_TRACKING_OPTIMIZED;
    continuation->diagnostics.optimized_strategy = continuation->optimized_strategy;
    if (continuation->diagnostics.fast_score > 0.0) {
        continuation->diagnostics.improvement_ratio =
            (continuation->diagnostics.fast_score - continuation->diagnostics.best_score)
            / continuation->diagnostics.fast_score;
    }
    continuation->active = false;
    continuation->complete = true;
    continuation->was_interrupted = false;
    tracking_sequence_publish_optimized_diagnostics(tracking, continuation, false);
    result = RUCKIG_WORKING;

restore_and_return:
    tracking->mode = saved_mode;
    tracking->optimized_strategy = saved_strategy;
    tracking->reactiveness = saved_reactiveness;
    tracking->look_ahead_cycles = saved_look_ahead_cycles;
    tracking->max_optimized_candidates = saved_max_optimized_candidates;
    return result;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence_interruptible(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence,
    ruckig_tracking_sequence_continuation_t* continuation
) {
    ruckig_result_t result;
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_sequence || !input || !output_sequence || !continuation
        || target_sequence->dofs != tracking->dofs || output_sequence->dofs != tracking->dofs
        || continuation->dofs != tracking->dofs || target_sequence->count == 0
        || target_sequence->count > output_sequence->capacity || target_sequence->count > continuation->capacity
        || !ruckig_input_same_dofs(input, tracking->dofs)) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = tracking_sequence_continuation_capture_start(
        tracking,
        target_sequence,
        input,
        output_sequence,
        continuation
    );
    if (result != RUCKIG_WORKING) {
        return result;
    }
    if (continuation->mode == RUCKIG_TRACKING_FAST) {
        result = tracking_sequence_process_fast(tracking, continuation, output_sequence);
        tracking_sequence_assert_continuation_consistent(continuation);
        return result;
    }
    if (continuation->mode == RUCKIG_TRACKING_OPTIMIZED) {
        result = tracking_sequence_process_optimized(tracking, continuation, output_sequence);
        tracking_sequence_assert_continuation_consistent(continuation);
        return result;
    }
    return RUCKIG_ERROR_UNSUPPORTED;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_resume_sequence(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* output_sequence
) {
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !continuation || !output_sequence || continuation->dofs != tracking->dofs
        || output_sequence->dofs != tracking->dofs || continuation->target_count > output_sequence->capacity
        || tracking->delta_time != continuation->delta_time) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (continuation->mode == RUCKIG_TRACKING_FAST) {
        if (continuation->target_count == 0 && !continuation->complete) {
            tracking_mark_error(tracking);
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        {
            const ruckig_result_t result = tracking_sequence_process_fast(tracking, continuation, output_sequence);
            tracking_sequence_assert_continuation_consistent(continuation);
            return result;
        }
    }
    if (continuation->mode == RUCKIG_TRACKING_OPTIMIZED) {
        if (continuation->target_count == 0 && !continuation->complete) {
            tracking_mark_error(tracking);
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        {
            const ruckig_result_t result = tracking_sequence_process_optimized(tracking, continuation, output_sequence);
            tracking_sequence_assert_continuation_consistent(continuation);
            return result;
        }
    }
    return RUCKIG_ERROR_UNSUPPORTED;
}
