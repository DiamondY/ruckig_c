#include "ruckig_c/tracking_internal.h"

#include <float.h>
#include <math.h>
#include <string.h>

void tracking_mark_error(ruckig_tracking_t* tracking) {
    if (tracking) {
        tracking_mark_step_status(tracking, RUCKIG_TRACKING_CALCULATION_ERROR);
    }
}

ruckig_result_t prepare_tracking_base(ruckig_tracking_t* tracking, const ruckig_input_t* input) {
    if (!tracking || !input || !ruckig_input_same_dofs(input, tracking->dofs)
        || input->control_interface != RUCKIG_CONTROL_POSITION || input->has_per_dof_control_interface) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (input != tracking->work_input) {
        if (ruckig_input_copy_state(input, tracking->work_input) != RUCKIG_WORKING) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
    }
    tracking->work_input->has_interrupt_calculation_duration = false;
    tracking->work_input->interrupt_calculation_duration = 0.0;
    return RUCKIG_WORKING;
}

ruckig_result_t set_tracking_candidate_prediction(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    double horizon
) {
    size_t dof;
    if (!finite_vector(target_position, tracking->dofs)
        || !finite_vector(target_velocity, tracking->dofs)
        || !finite_vector(target_acceleration, tracking->dofs)) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (!isfinite(horizon) || horizon < 0.0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    for (dof = 0; dof < tracking->dofs; ++dof) {
        const double p = target_position[dof];
        const double v = target_velocity[dof];
        const double a = target_acceleration[dof];
        const double predicted_position = p + v * horizon + 0.5 * a * horizon * horizon;
        const double predicted_velocity = v + a * horizon;
        if (!isfinite(predicted_position) || !isfinite(predicted_velocity)) {
            return RUCKIG_ERROR_INVALID_INPUT;
        }
        tracking->optimized_candidate_position[dof] = predicted_position;
        tracking->optimized_candidate_velocity[dof] = predicted_velocity;
        tracking->optimized_candidate_acceleration[dof] = a;
    }
    return RUCKIG_WORKING;
}

static void copy_candidate_to_work_input(ruckig_tracking_t* tracking) {
    memcpy(tracking->work_input->target_position, tracking->optimized_candidate_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_velocity, tracking->optimized_candidate_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_acceleration, tracking->optimized_candidate_acceleration, sizeof(double) * tracking->dofs);
}

static void copy_work_input_target_to_best(ruckig_tracking_t* tracking) {
    memcpy(tracking->optimized_best_position, tracking->work_input->target_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->optimized_best_velocity, tracking->work_input->target_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->optimized_best_acceleration, tracking->work_input->target_acceleration, sizeof(double) * tracking->dofs);
}

void copy_best_to_work_input(ruckig_tracking_t* tracking) {
    memcpy(tracking->work_input->target_position, tracking->optimized_best_position, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_velocity, tracking->optimized_best_velocity, sizeof(double) * tracking->dofs);
    memcpy(tracking->work_input->target_acceleration, tracking->optimized_best_acceleration, sizeof(double) * tracking->dofs);
}

ruckig_result_t prepare_fast_tracking_input(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    const ruckig_input_t* input
) {
    ruckig_result_t result = prepare_tracking_base(tracking, input);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    result = set_tracking_candidate_prediction(
        tracking,
        target_position,
        target_velocity,
        target_acceleration,
        (double)tracking->look_ahead_cycles * tracking->delta_time * tracking->reactiveness
    );
    if (result != RUCKIG_WORKING) {
        return result;
    }
    copy_candidate_to_work_input(tracking);
    return RUCKIG_WORKING;
}

ruckig_result_t run_prepared_tracking_update(
    ruckig_tracking_t* tracking,
    ruckig_output_t* output,
    ruckig_tracking_calculation_status_t success_status,
    bool force_reset
) {
    ruckig_result_t result;
    if (force_reset) {
        ruckig_reset(tracking->otg);
    }
    result = ruckig_update(tracking->otg, tracking->work_input, output);
    if (result != RUCKIG_WORKING && result != RUCKIG_FINISHED) {
        tracking_mark_step_status(tracking, RUCKIG_TRACKING_CALCULATION_ERROR);
        return result;
    }
    if (success_status == RUCKIG_TRACKING_CALCULATION_FAST && tracking->diagnostics.candidate_count == 0) {
        tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
        tracking_note_valid_candidate(tracking);
    }
    tracking_mark_step_status(tracking, success_status);
    return result;
}

static ruckig_result_t score_current_tracking_candidate(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double* score,
    double* terminal_position_error
) {
    size_t sample;
    ruckig_result_t result;
    double value = 0.0;
    if (!score || !terminal_position_error || target_count == 0) {
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    *terminal_position_error = 0.0;
    result = ruckig_calculate(tracking->otg, tracking->work_input, tracking->work_output->trajectory);
    if (result != RUCKIG_WORKING) {
        return result;
    }
    for (sample = 0; sample < target_count; ++sample) {
        size_t dof;
        size_t section = 0;
        const size_t offset = sample * tracking->dofs;
        const double time = (double)(sample + 1) * tracking->delta_time;
        double weight = 1.0 + config->horizon_weight_step * (double)sample;
        result = ruckig_trajectory_at_time(
            tracking->work_output->trajectory,
            time,
            tracking->optimized_candidate_position,
            tracking->optimized_candidate_velocity,
            tracking->optimized_candidate_acceleration,
            tracking->optimized_candidate_jerk,
            &section
        );
        (void)section;
        if (result != RUCKIG_WORKING) {
            return result;
        }
        if (sample + 1 == target_count) {
            weight *= config->terminal_weight;
        }
        for (dof = 0; dof < tracking->dofs; ++dof) {
            const double position_error = tracking->optimized_candidate_position[dof] - target_position[offset + dof];
            const double velocity_error = tracking->optimized_candidate_velocity[dof] - target_velocity[offset + dof];
            const double acceleration_error = tracking->optimized_candidate_acceleration[dof] - target_acceleration[offset + dof];
            const double jerk = tracking->optimized_candidate_jerk[dof];
            if (!isfinite(position_error) || !isfinite(velocity_error) || !isfinite(acceleration_error) || !isfinite(jerk)) {
                return RUCKIG_ERROR_INVALID_INPUT;
            }
            if (tracking->work_input->enabled[dof]) {
                value += weight * (
                    config->position_weight * position_error * position_error
                    + config->velocity_weight * velocity_error * velocity_error
                    + config->acceleration_weight * acceleration_error * acceleration_error
                );
                value += weight * config->jerk_weight * jerk * jerk;
                if (sample + 1 == target_count) {
                    *terminal_position_error += position_error * position_error;
                }
            }
        }
    }
    *score = value;
    return RUCKIG_WORKING;
}

static ruckig_result_t try_tracking_candidate(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    tracking_candidate_family_t family,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double fast_score,
    double fast_terminal_position_error,
    double* best_score,
    bool* improved
) {
    double raw_score = DBL_MAX;
    double score = DBL_MAX;
    double terminal_position_error = DBL_MAX;
    ruckig_result_t result;
    if (tracking->last_candidate_count >= tracking->max_optimized_candidates) {
        tracking_note_budget_exhausted(tracking);
        return RUCKIG_WORKING;
    }
    tracking_note_candidate_family(tracking, family);
    copy_candidate_to_work_input(tracking);
    result = score_current_tracking_candidate(
        tracking,
        config,
        target_position,
        target_velocity,
        target_acceleration,
        target_count,
        &raw_score,
        &terminal_position_error
    );
    if (result != RUCKIG_WORKING) {
        tracking_note_rejected_candidate(tracking);
        return RUCKIG_WORKING;
    }
    tracking_note_valid_candidate(tracking);
    score = raw_score * (family == TRACKING_CANDIDATE_FAST ? 1.0 : config->candidate_family_score_ratio);
    if (score + RUCKIG_TRACKING_SCORE_EPSILON < *best_score * config->acceptance_ratio) {
        *best_score = score;
        *improved = true;
        if ((size_t)family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
            ++tracking->audit_family_strict_improved[(size_t)family];
        }
        ++tracking->audit_strict_improved_count;
        tracking->audit_best_candidate_family = (size_t)family;
        tracking->audit_best_candidate_near_tie = false;
        copy_work_input_target_to_best(tracking);
    } else if (family != TRACKING_CANDIDATE_FAST
        && config->near_tie_ratio > 1.0
        && raw_score <= fast_score * config->near_tie_ratio
        && terminal_position_error <= 0.5 * fast_terminal_position_error) {
        *best_score = score;
        *improved = true;
        if ((size_t)family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
            ++tracking->audit_family_near_tie_accepted[(size_t)family];
        }
        ++tracking->audit_near_tie_accepted_count;
        tracking->audit_best_candidate_family = (size_t)family;
        tracking->audit_best_candidate_near_tie = true;
        copy_work_input_target_to_best(tracking);
    }
    return RUCKIG_WORKING;
}

static bool tracking_candidate_budget_available(const ruckig_tracking_t* tracking) {
    return tracking->last_candidate_count < tracking->max_optimized_candidates;
}

static ruckig_result_t try_tracking_prediction_candidate(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    tracking_candidate_family_t family,
    const double* candidate_position,
    const double* candidate_velocity,
    const double* candidate_acceleration,
    double horizon,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    double fast_score,
    double fast_terminal_position_error,
    double* best_score,
    bool* improved
) {
    ruckig_result_t result;
    if (!tracking_candidate_budget_available(tracking)) {
        tracking_note_budget_exhausted(tracking);
        return RUCKIG_WORKING;
    }
    result = set_tracking_candidate_prediction(
        tracking,
        candidate_position,
        candidate_velocity,
        candidate_acceleration,
        horizon
    );
    if (result != RUCKIG_WORKING) {
        return result;
    }
    return try_tracking_candidate(
        tracking,
        config,
        family,
        target_position,
        target_velocity,
        target_acceleration,
        target_count,
        fast_score,
        fast_terminal_position_error,
        best_score,
        improved
    );
}

ruckig_result_t tracking_optimized_candidate_step(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t window_count,
    size_t* phase,
    size_t* index,
    double* fast_score,
    double* fast_terminal_position_error,
    double* best_score,
    bool* improved,
    bool* evaluated_candidate
) {
    ruckig_result_t result = RUCKIG_WORKING;
    if (evaluated_candidate) {
        *evaluated_candidate = false;
    }

    switch ((tracking_sequence_optimized_phase_t)*phase) {
    case TRACKING_SEQUENCE_OPTIMIZED_FAST:
        result = set_tracking_candidate_prediction(
            tracking,
            target_position,
            target_velocity,
            target_acceleration,
            (double)window_count * tracking->delta_time * tracking->reactiveness
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        copy_candidate_to_work_input(tracking);
        tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
        result = score_current_tracking_candidate(
            tracking,
            config,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            fast_score,
            fast_terminal_position_error
        );
        if (result != RUCKIG_WORKING) {
            tracking_note_rejected_candidate(tracking);
            return result;
        }
        tracking_note_valid_candidate(tracking);
        *best_score = *fast_score;
        tracking->diagnostics.fast_score = *fast_score;
        tracking->diagnostics.best_score = *best_score;
        copy_work_input_target_to_best(tracking);
        *phase = TRACKING_SEQUENCE_OPTIMIZED_INSTANTANEOUS;
        *index = 0;
        if (evaluated_candidate) {
            *evaluated_candidate = true;
        }
        break;

    case TRACKING_SEQUENCE_OPTIMIZED_INSTANTANEOUS:
        if (*index >= window_count) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_HORIZON;
            *index = 0;
            break;
        }
        {
            const size_t offset = *index * tracking->dofs;
            result = try_tracking_prediction_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_INSTANTANEOUS,
                &target_position[offset],
                &target_velocity[offset],
                &target_acceleration[offset],
                0.0,
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                *fast_score,
                *fast_terminal_position_error,
                best_score,
                improved
            );
        }
        if (result != RUCKIG_WORKING) {
            return result;
        }
        ++(*index);
        if (*index >= window_count) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_HORIZON;
            *index = 0;
        }
        if (evaluated_candidate) {
            *evaluated_candidate = true;
        }
        break;

    case TRACKING_SEQUENCE_OPTIMIZED_HORIZON:
        if (*index >= window_count) {
            *phase = config->use_lead_lag_horizons
                ? TRACKING_SEQUENCE_OPTIMIZED_LEAD_LAG
                : TRACKING_SEQUENCE_OPTIMIZED_TERMINAL_BLEND;
            *index = 0;
            break;
        }
        result = try_tracking_prediction_candidate(
            tracking,
            config,
            TRACKING_CANDIDATE_HORIZON,
            target_position,
            target_velocity,
            target_acceleration,
            (double)(*index + 1) * tracking->delta_time * tracking->reactiveness,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            *fast_score,
            *fast_terminal_position_error,
            best_score,
            improved
        );
        if (result != RUCKIG_WORKING) {
            return result;
        }
        ++(*index);
        if (*index >= window_count) {
            *phase = config->use_lead_lag_horizons
                ? TRACKING_SEQUENCE_OPTIMIZED_LEAD_LAG
                : TRACKING_SEQUENCE_OPTIMIZED_TERMINAL_BLEND;
            *index = 0;
        }
        if (evaluated_candidate) {
            *evaluated_candidate = true;
        }
        break;

    case TRACKING_SEQUENCE_OPTIMIZED_LEAD_LAG:
        if (!config->use_lead_lag_horizons || *index >= 4) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_TERMINAL_BLEND;
            *index = 0;
            break;
        }
        {
            const double horizon_values[4] = {
                0.5 * tracking->delta_time * tracking->reactiveness,
                ((double)window_count + 0.5) * tracking->delta_time * tracking->reactiveness,
                ((double)window_count + 1.0) * tracking->delta_time * tracking->reactiveness,
                ((double)window_count + 2.0) * tracking->delta_time * tracking->reactiveness
            };
            result = try_tracking_prediction_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_LEAD_LAG,
                target_position,
                target_velocity,
                target_acceleration,
                horizon_values[*index],
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                *fast_score,
                *fast_terminal_position_error,
                best_score,
                improved
            );
        }
        if (result != RUCKIG_WORKING) {
            return result;
        }
        ++(*index);
        if (*index >= 4) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_TERMINAL_BLEND;
            *index = 0;
        }
        if (evaluated_candidate) {
            *evaluated_candidate = true;
        }
        break;

    case TRACKING_SEQUENCE_OPTIMIZED_TERMINAL_BLEND:
        if (!config->use_terminal_blends || *index >= 4) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_DERIVATIVE_DAMPED;
            *index = 0;
            break;
        }
        {
            static const double blend_values[4] = {0.25, 0.5, 0.75, 1.0};
            const size_t terminal_offset = (window_count - 1) * tracking->dofs;
            const double blend = blend_values[*index];
            size_t dof;
            result = set_tracking_candidate_prediction(
                tracking,
                target_position,
                target_velocity,
                target_acceleration,
                (double)tracking->look_ahead_cycles * tracking->delta_time * tracking->reactiveness
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            for (dof = 0; dof < tracking->dofs; ++dof) {
                tracking->optimized_candidate_position[dof] =
                    (1.0 - blend) * tracking->optimized_candidate_position[dof]
                    + blend * target_position[terminal_offset + dof];
                tracking->optimized_candidate_velocity[dof] =
                    (1.0 - blend) * tracking->optimized_candidate_velocity[dof]
                    + blend * target_velocity[terminal_offset + dof];
                tracking->optimized_candidate_acceleration[dof] =
                    (1.0 - blend) * tracking->optimized_candidate_acceleration[dof]
                    + blend * target_acceleration[terminal_offset + dof];
            }
            result = try_tracking_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_TERMINAL_BLEND,
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                *fast_score,
                *fast_terminal_position_error,
                best_score,
                improved
            );
        }
        if (result != RUCKIG_WORKING) {
            return result;
        }
        ++(*index);
        if (*index >= 4) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_DERIVATIVE_DAMPED;
            *index = 0;
        }
        if (evaluated_candidate) {
            *evaluated_candidate = true;
        }
        break;

    case TRACKING_SEQUENCE_OPTIMIZED_DERIVATIVE_DAMPED:
        if (!config->use_derivative_damping || *index >= 3) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP;
            *index = 0;
            break;
        }
        {
            const size_t terminal_offset = (window_count - 1) * tracking->dofs;
            const double scale = *index == 0 ? 0.75 : (*index == 1 ? 0.5 : 0.0);
            size_t dof;
            result = set_tracking_candidate_prediction(
                tracking,
                &target_position[terminal_offset],
                &target_velocity[terminal_offset],
                &target_acceleration[terminal_offset],
                0.0
            );
            if (result != RUCKIG_WORKING) {
                return result;
            }
            for (dof = 0; dof < tracking->dofs; ++dof) {
                tracking->optimized_candidate_velocity[dof] *= scale;
                tracking->optimized_candidate_acceleration[dof] *= scale;
            }
            result = try_tracking_candidate(
                tracking,
                config,
                TRACKING_CANDIDATE_DERIVATIVE_DAMPED,
                target_position,
                target_velocity,
                target_acceleration,
                window_count,
                *fast_score,
                *fast_terminal_position_error,
                best_score,
                improved
            );
        }
        if (result != RUCKIG_WORKING) {
            return result;
        }
        ++(*index);
        if (*index >= 3) {
            *phase = TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP;
            *index = 0;
        }
        if (evaluated_candidate) {
            *evaluated_candidate = true;
        }
        break;

    case TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP:
    case TRACKING_SEQUENCE_OPTIMIZED_IDLE:
    default:
        break;
    }

    return RUCKIG_WORKING;
}

ruckig_result_t evaluate_optimized_tracking(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    bool allow_interrupt
) {
    size_t phase = TRACKING_SEQUENCE_OPTIMIZED_FAST;
    size_t index = 0;
    double fast_score = DBL_MAX;
    double fast_terminal_position_error = DBL_MAX;
    double best_score = DBL_MAX;
    const tracking_strategy_config_t* config = tracking_strategy_config(tracking->optimized_strategy);
    bool improved = false;
    bool interrupted = false;
    ruckig_result_t result;
    const size_t window_count = min_size(target_count, tracking->look_ahead_cycles);
    ruckig_interrupt_context_t interrupt_context = ruckig_interrupt_context_start(input, allow_interrupt);
    tracking_reset_diagnostics(tracking);
    if (!config || target_count == 0 || window_count == 0
        || !finite_vector(target_position, target_count * tracking->dofs)
        || !finite_vector(target_velocity, target_count * tracking->dofs)
        || !finite_vector(target_acceleration, target_count * tracking->dofs)) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_tracking_base(tracking, input);
    if (result != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return result;
    }

    while (phase != TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP) {
        bool evaluated_candidate = false;
        result = tracking_optimized_candidate_step(
            tracking,
            config,
            target_position,
            target_velocity,
            target_acceleration,
            window_count,
            &phase,
            &index,
            &fast_score,
            &fast_terminal_position_error,
            &best_score,
            &improved,
            &evaluated_candidate
        );
        if (result != RUCKIG_WORKING) {
            tracking_mark_error(tracking);
            return result;
        }
        if (evaluated_candidate && ruckig_interrupt_context_check(&interrupt_context)) {
            interrupted = true;
            tracking_note_budget_exhausted(tracking);
            goto finish_optimized_tracking;
        }
    }

finish_optimized_tracking:
    copy_best_to_work_input(tracking);
    if (tracking->audit_best_candidate_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT) {
        ++tracking->audit_family_selected[tracking->audit_best_candidate_family];
    }
    tracking->diagnostics.best_score = best_score;
    tracking_finalize_score_diagnostics(tracking);
    result = run_prepared_tracking_update(
        tracking,
        output,
        improved ? RUCKIG_TRACKING_CALCULATION_OPTIMIZED : RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
        true
    );
    if ((result == RUCKIG_WORKING || result == RUCKIG_FINISHED) && interrupted) {
        output->was_calculation_interrupted = true;
    }
    return result;
}

RUCKIG_C_API ruckig_result_t ruckig_tracking_update(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_t* target_state,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_state || !input || !output || target_state->dofs != tracking->dofs || output->dofs != tracking->dofs) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return evaluate_optimized_tracking(
            tracking,
            target_state->position,
            target_state->velocity,
            target_state->acceleration,
            1,
            input,
            output,
            true
        );
    }
    if (tracking->mode != RUCKIG_TRACKING_FAST) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    result = prepare_fast_tracking_input(tracking, target_state->position, target_state->velocity, target_state->acceleration, input);
    if (result != RUCKIG_WORKING) {
        tracking_mark_error(tracking);
        return result;
    }
    tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
    tracking_note_valid_candidate(tracking);
    return run_prepared_tracking_update(tracking, output, RUCKIG_TRACKING_CALCULATION_FAST, false);
}
RUCKIG_C_API ruckig_result_t ruckig_tracking_update_with_lookahead(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_output_t* output
) {
    if (tracking) {
        tracking_reset_diagnostics(tracking);
    }
    if (!tracking || !target_sequence || !input || !output
        || target_sequence->dofs != tracking->dofs || output->dofs != tracking->dofs
        || target_sequence->count == 0 || target_sequence->count > target_sequence->capacity) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    if (tracking->mode == RUCKIG_TRACKING_OPTIMIZED) {
        return evaluate_optimized_tracking(
            tracking,
            target_sequence->position,
            target_sequence->velocity,
            target_sequence->acceleration,
            target_sequence->count,
            input,
            output,
            true
        );
    }
    if (tracking->mode != RUCKIG_TRACKING_FAST) {
        tracking_mark_error(tracking);
        return RUCKIG_ERROR_INVALID_INPUT;
    }
    {
        ruckig_result_t result = prepare_fast_tracking_input(
            tracking,
            target_sequence->position,
            target_sequence->velocity,
            target_sequence->acceleration,
            input
        );
        if (result != RUCKIG_WORKING) {
            tracking_mark_error(tracking);
            return result;
        }
    }
    tracking_note_candidate_family(tracking, TRACKING_CANDIDATE_FAST);
    tracking_note_valid_candidate(tracking);
    return run_prepared_tracking_update(tracking, output, RUCKIG_TRACKING_CALCULATION_FAST, false);
}
