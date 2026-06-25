#include "test_api_internal.h"

void fill_tracking_input_1d(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_position_data(input)[0] = 0.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;
}

void fill_tracking_target_ramp(ruckig_target_state_t* target, double time) {
    const double ramp_velocity = 0.5;
    const double ramp_limit = 1.0;
    const bool on_ramp = time < ramp_limit / ramp_velocity;
    ruckig_target_state_position_data(target)[0] = on_ramp ? time * ramp_velocity : ramp_limit;
    ruckig_target_state_velocity_data(target)[0] = on_ramp ? ramp_velocity : 0.0;
    ruckig_target_state_acceleration_data(target)[0] = 0.0;
}

void fill_tracking_input_nd(ruckig_input_t* input, size_t dofs) {
    size_t dof;
    double* current_position = ruckig_input_current_position_data(input);
    double* current_velocity = ruckig_input_current_velocity_data(input);
    double* current_acceleration = ruckig_input_current_acceleration_data(input);
    double* target_position = ruckig_input_target_position_data(input);
    double* target_velocity = ruckig_input_target_velocity_data(input);
    double* target_acceleration = ruckig_input_target_acceleration_data(input);
    double* max_velocity = ruckig_input_max_velocity_data(input);
    double* max_acceleration = ruckig_input_max_acceleration_data(input);
    double* max_jerk = ruckig_input_max_jerk_data(input);
    double* min_position = ruckig_input_min_position_data(input);
    double* max_position = ruckig_input_max_position_data(input);
    for (dof = 0; dof < dofs; ++dof) {
        current_position[dof] = 0.0;
        current_velocity[dof] = 0.0;
        current_acceleration[dof] = 0.0;
        target_position[dof] = 0.0;
        target_velocity[dof] = 0.0;
        target_acceleration[dof] = 0.0;
        max_velocity[dof] = 1.25 + 0.05 * (double)dof;
        max_acceleration[dof] = 2.25 + 0.10 * (double)dof;
        max_jerk[dof] = 6.0 + 0.25 * (double)dof;
        min_position[dof] = -2.0;
        max_position[dof] = 2.0;
    }
}

void tracking_signal_value(int signal, size_t dof, double time, double* position, double* velocity, double* acceleration) {
    const double phase = 0.17 * (double)dof;
    if (signal == 0) {
        const double v = 0.35 + 0.03 * (double)dof;
        *position = v * time;
        *velocity = v;
        *acceleration = 0.0;
    } else if (signal == 1) {
        const double a = 0.12 + 0.01 * (double)dof;
        *position = 0.5 * a * time * time;
        *velocity = a * time;
        *acceleration = a;
    } else if (signal == 2) {
        const double w = 0.45 + 0.04 * (double)dof;
        const double amplitude = 0.20 + 0.02 * (double)dof;
        const double x = w * time + phase;
        *position = amplitude * sin(x);
        *velocity = amplitude * w * cos(x);
        *acceleration = -amplitude * w * w * sin(x);
    } else {
        const double duration = 2.20 + 0.10 * (double)dof;
        const double amplitude = 0.24 + 0.015 * (double)dof;
        const double shifted_time = time + 0.03 * (double)dof;
        if (shifted_time < duration) {
            const double scale = M_PI / duration;
            const double angle = scale * shifted_time;
            *position = 0.5 * amplitude * (1.0 - cos(angle));
            *velocity = 0.5 * amplitude * scale * sin(angle);
            *acceleration = 0.5 * amplitude * scale * scale * cos(angle);
        } else {
            *position = amplitude;
            *velocity = 0.0;
            *acceleration = 0.0;
        }
    }
}

void set_tracking_target_signal(ruckig_target_state_t* target, int signal, size_t dofs, double time) {
    size_t dof;
    double* position = ruckig_target_state_position_data(target);
    double* velocity = ruckig_target_state_velocity_data(target);
    double* acceleration = ruckig_target_state_acceleration_data(target);
    for (dof = 0; dof < dofs; ++dof) {
        tracking_signal_value(signal, dof, time, &position[dof], &velocity[dof], &acceleration[dof]);
    }
}

void set_tracking_sequence_signal(ruckig_target_state_sequence_t* targets, int signal, size_t dofs, size_t count, double delta_time) {
    size_t step;
    double* position = ruckig_target_state_sequence_position_data(targets);
    double* velocity = ruckig_target_state_sequence_velocity_data(targets);
    double* acceleration = ruckig_target_state_sequence_acceleration_data(targets);
    for (step = 0; step < count; ++step) {
        size_t dof;
        const double time = (double)step * delta_time;
        for (dof = 0; dof < dofs; ++dof) {
            tracking_signal_value(
                signal,
                dof,
                time,
                &position[step * dofs + dof],
                &velocity[step * dofs + dof],
                &acceleration[step * dofs + dof]
            );
        }
    }
}

void check_tracking_output_constraints(const ruckig_output_t* output, const ruckig_input_t* input, size_t dofs) {
    size_t dof;
    const double* position = ruckig_output_new_position_data(output);
    const double* velocity = ruckig_output_new_velocity_data(output);
    const double* acceleration = ruckig_output_new_acceleration_data(output);
    const double* jerk = ruckig_output_new_jerk_data(output);
    const double* max_velocity = ruckig_input_max_velocity_const_data(input);
    const double* max_acceleration = ruckig_input_max_acceleration_const_data(input);
    const double* max_jerk = ruckig_input_max_jerk_const_data(input);
    const double* min_position = ruckig_input_min_position_const_data(input);
    const double* max_position = ruckig_input_max_position_const_data(input);
    for (dof = 0; dof < dofs; ++dof) {
        CHECK_TRUE(isfinite(position[dof]));
        CHECK_TRUE(isfinite(velocity[dof]));
        CHECK_TRUE(isfinite(acceleration[dof]));
        CHECK_TRUE(isfinite(jerk[dof]));
        CHECK_TRUE(position[dof] >= min_position[dof] - 1e-9);
        CHECK_TRUE(position[dof] <= max_position[dof] + 1e-9);
        CHECK_TRUE(fabs(velocity[dof]) <= max_velocity[dof] + 1e-9);
        CHECK_TRUE(fabs(acceleration[dof]) <= max_acceleration[dof] + 1e-9);
        CHECK_TRUE(fabs(jerk[dof]) <= max_jerk[dof] + 1e-7);
    }
}


void check_tracking_output_sequence(
    const ruckig_tracking_output_sequence_t* outputs,
    size_t dofs,
    size_t count,
    double delta_time
) {
    size_t step;
    const double* position = ruckig_tracking_output_sequence_new_position_const_data(outputs);
    const double* velocity = ruckig_tracking_output_sequence_new_velocity_const_data(outputs);
    const double* acceleration = ruckig_tracking_output_sequence_new_acceleration_const_data(outputs);
    const double* jerk = ruckig_tracking_output_sequence_new_jerk_const_data(outputs);
    const double* time = ruckig_tracking_output_sequence_time_const_data(outputs);
    const size_t* section = ruckig_tracking_output_sequence_section_const_data(outputs);
    const ruckig_result_t* results = ruckig_tracking_output_sequence_result_const_data(outputs);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), count);
    for (step = 0; step < count; ++step) {
        size_t dof;
        CHECK_NEAR(time[step], (double)(step + 1) * delta_time, 1e-12);
        if (step > 0) {
            CHECK_TRUE(time[step] > time[step - 1]);
        }
        CHECK_TRUE(results[step] == RUCKIG_WORKING || results[step] == RUCKIG_FINISHED);
        CHECK_TRUE(section[step] < 16);
        for (dof = 0; dof < dofs; ++dof) {
            const size_t offset = step * dofs + dof;
            CHECK_TRUE(isfinite(position[offset]));
            CHECK_TRUE(isfinite(velocity[offset]));
            CHECK_TRUE(isfinite(acceleration[offset]));
            CHECK_TRUE(isfinite(jerk[offset]));
        }
    }
}

static void run_tracking_loop_final(
    size_t dofs,
    int signal,
    double reactiveness,
    size_t look_ahead_cycles,
    size_t steps,
    double* final_position
) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    size_t step;
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, dofs, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, dofs), RUCKIG_WORKING);
    fill_tracking_input_nd(input, dofs);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, reactiveness), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, look_ahead_cycles), RUCKIG_WORKING);
    for (step = 0; step < steps; ++step) {
        set_tracking_target_signal(target, signal, dofs, (double)step * 0.01);
        CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
        check_tracking_output_constraints(output, input, dofs);
        ruckig_output_pass_to_input(output, input);
    }
    memcpy(final_position, ruckig_output_new_position_data(output), sizeof(double) * dofs);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

bool tracking_optimized_status_is_success(ruckig_tracking_calculation_status_t status) {
    return status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED
        || status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK;
}

static size_t tracking_diagnostics_family_count(const ruckig_tracking_diagnostics_t* diagnostics) {
    return diagnostics->fast_candidate_count
        + diagnostics->instantaneous_candidate_count
        + diagnostics->horizon_candidate_count
        + diagnostics->terminal_blend_candidate_count
        + diagnostics->derivative_damped_candidate_count
        + diagnostics->lead_lag_candidate_count;
}

void check_tracking_diagnostics_common(
    const ruckig_tracking_t* tracking,
    const ruckig_tracking_diagnostics_t* diagnostics
) {
    size_t i;
    CHECK_EQ_INT(ruckig_tracking_get_last_calculation_status(tracking), diagnostics->calculation_status);
    CHECK_EQ_INT(ruckig_tracking_get_last_candidate_count(tracking), diagnostics->candidate_count);
    CHECK_EQ_INT(tracking_diagnostics_family_count(diagnostics), diagnostics->candidate_count);
    CHECK_TRUE(diagnostics->valid_candidate_count + diagnostics->rejected_candidate_count <= diagnostics->candidate_count);
    CHECK_TRUE(isfinite(diagnostics->fast_score));
    CHECK_TRUE(isfinite(diagnostics->best_score));
    CHECK_TRUE(isfinite(diagnostics->improvement_ratio));
    for (i = 0; i < 4; ++i) {
        CHECK_EQ_INT(diagnostics->reserved_size[i], 0);
        CHECK_NEAR(diagnostics->reserved_value[i], 0.0, 0.0);
    }
    if (diagnostics->fast_score > 0.0) {
        CHECK_NEAR(
            diagnostics->improvement_ratio,
            (diagnostics->fast_score - diagnostics->best_score) / diagnostics->fast_score,
            1e-12
        );
    } else {
        CHECK_NEAR(diagnostics->improvement_ratio, 0.0, 0.0);
    }
}

static void test_tracking_api_lifecycle_and_accessors(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* target_sequence = NULL;
    ruckig_tracking_output_sequence_t* output_sequence = NULL;
    ruckig_tracking_sequence_continuation_t* continuation = NULL;
    ruckig_tracking_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_tracking_create(NULL, 1, 0.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 0, 0.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_dof_count(tracking), 2);
    CHECK_NEAR(ruckig_tracking_get_delta_time(tracking), 0.01, 0.0);
    CHECK_EQ_INT(ruckig_tracking_get_mode(tracking), RUCKIG_TRACKING_FAST);
    CHECK_NEAR(ruckig_tracking_get_reactiveness(tracking), 1.0, 0.0);
    CHECK_EQ_INT(ruckig_tracking_get_look_ahead_cycles(tracking), 1);
    CHECK_EQ_INT(ruckig_tracking_get_max_optimized_candidates(tracking), 16);
    CHECK_EQ_INT(ruckig_tracking_get_optimized_strategy(tracking), RUCKIG_TRACKING_OPTIMIZED_BALANCED);
    CHECK_EQ_INT(ruckig_tracking_get_last_calculation_status(tracking), RUCKIG_TRACKING_CALCULATION_NONE);
    CHECK_EQ_INT(ruckig_tracking_get_last_candidate_count(tracking), 0);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(NULL, &diagnostics), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_NONE);
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_FAST);
    CHECK_EQ_INT(diagnostics.optimized_strategy, RUCKIG_TRACKING_OPTIMIZED_BALANCED);
    CHECK_EQ_INT(diagnostics.candidate_count, 0);
    CHECK_NEAR(diagnostics.fast_score, 0.0, 0.0);
    CHECK_NEAR(diagnostics.best_score, 0.0, 0.0);
    check_tracking_diagnostics_common(tracking, &diagnostics);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, 0.25), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_tracking_get_reactiveness(tracking), 0.25, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_look_ahead_cycles(tracking), 3);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 8), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_max_optimized_candidates(tracking), 8);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 129), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_STABLE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_optimized_strategy(tracking), RUCKIG_TRACKING_OPTIMIZED_STABLE);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_BALANCED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_optimized_strategy(tracking), RUCKIG_TRACKING_OPTIMIZED_BALANCED);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_optimized_strategy(tracking), RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, (ruckig_tracking_optimized_strategy_t)99), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_optimized_strategy(NULL), RUCKIG_TRACKING_OPTIMIZED_BALANCED);

    CHECK_EQ_INT(ruckig_target_state_create(&target, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_get_dof_count(target), 2);
    ruckig_target_state_position_data(target)[0] = 1.0;
    ruckig_target_state_velocity_data(target)[1] = -0.5;
    ruckig_target_state_acceleration_data(target)[0] = 0.2;
    CHECK_NEAR(ruckig_target_state_position_const_data(target)[0], 1.0, 0.0);
    CHECK_NEAR(ruckig_target_state_velocity_const_data(target)[1], -0.5, 0.0);
    CHECK_NEAR(ruckig_target_state_acceleration_const_data(target)[0], 0.2, 0.0);

    CHECK_EQ_INT(ruckig_target_state_sequence_create(&target_sequence, 2, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_get_dof_count(target_sequence), 2);
    CHECK_EQ_INT(ruckig_target_state_sequence_get_capacity(target_sequence), 4);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(target_sequence, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_get_count(target_sequence), 3);
    ruckig_target_state_sequence_position_data(target_sequence)[5] = 1.25;
    CHECK_NEAR(ruckig_target_state_sequence_position_const_data(target_sequence)[5], 1.25, 0.0);
    ruckig_target_state_sequence_clear(target_sequence);
    CHECK_EQ_INT(ruckig_target_state_sequence_get_count(target_sequence), 0);

    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&output_sequence, 2, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_dof_count(output_sequence), 2);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_capacity(output_sequence), 4);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(output_sequence), 0);
    CHECK_TRUE(ruckig_tracking_output_sequence_new_position_const_data(output_sequence) != NULL);
    CHECK_TRUE(ruckig_tracking_output_sequence_new_velocity_const_data(output_sequence) != NULL);
    CHECK_TRUE(ruckig_tracking_output_sequence_new_acceleration_const_data(output_sequence) != NULL);
    CHECK_TRUE(ruckig_tracking_output_sequence_new_jerk_const_data(output_sequence) != NULL);
    CHECK_TRUE(ruckig_tracking_output_sequence_time_const_data(output_sequence) != NULL);
    CHECK_TRUE(ruckig_tracking_output_sequence_section_const_data(output_sequence) != NULL);
    CHECK_TRUE(ruckig_tracking_output_sequence_result_const_data(output_sequence) != NULL);
    ruckig_tracking_output_sequence_clear(output_sequence);

    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(NULL, 2, 4), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 0, 4), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 2, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_create(&continuation, 2, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_dof_count(continuation), 2);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_capacity(continuation), 4);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(continuation));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_complete(continuation));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), 0);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_target_count(continuation), 0);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_dof_count(NULL), 0);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_capacity(NULL), 0);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(NULL));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_was_interrupted(NULL));
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_complete(NULL));
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(NULL), 0);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_target_count(NULL), 0);

    ruckig_tracking_sequence_continuation_reset(continuation);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_target_count(continuation), 0);
    CHECK_EQ_INT(ruckig_tracking_sequence_continuation_get_completed_count(continuation), 0);
    CHECK_TRUE(!ruckig_tracking_sequence_continuation_is_active(continuation));

    ruckig_tracking_sequence_continuation_destroy(continuation);
    ruckig_tracking_output_sequence_destroy(output_sequence);
    ruckig_target_state_sequence_destroy(target_sequence);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
    ruckig_tracking_sequence_continuation_destroy(NULL);
    ruckig_tracking_output_sequence_destroy(NULL);
    ruckig_target_state_sequence_destroy(NULL);
    ruckig_target_state_destroy(NULL);
    ruckig_tracking_destroy(NULL);
}


static void test_tracking_diagnostics_snapshots(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 6;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, 1);
    CHECK_EQ_INT(diagnostics.fast_candidate_count, 1);
    CHECK_EQ_INT(diagnostics.fallback_step_count, 0);
    CHECK_EQ_INT(diagnostics.optimized_step_count, 0);
    CHECK_EQ_INT(diagnostics.error_step_count, 0);
    CHECK_NEAR(diagnostics.fast_score, 0.0, 0.0);
    CHECK_NEAR(diagnostics.best_score, 0.0, 0.0);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, 3), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 0, 1, 3, 0.01);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 3);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, 3);
    CHECK_EQ_INT(diagnostics.fast_candidate_count, 3);
    CHECK_NEAR(diagnostics.fast_score, 0.0, 0.0);
    CHECK_NEAR(diagnostics.best_score, 0.0, 0.0);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    {
        const ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(tracking_optimized_status_is_success(diagnostics.calculation_status));
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_OPTIMIZED);
    CHECK_EQ_INT(diagnostics.optimized_strategy, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE);
    CHECK_TRUE(diagnostics.candidate_count >= 1);
    CHECK_TRUE(diagnostics.candidate_count <= 16);
    CHECK_EQ_INT(diagnostics.fast_candidate_count, 1);
    CHECK_TRUE(diagnostics.fast_score >= diagnostics.best_score - 1e-12);
    CHECK_TRUE(diagnostics.fallback_step_count + diagnostics.optimized_step_count == 1);
    CHECK_EQ_INT(diagnostics.error_step_count, 0);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 2), RUCKIG_WORKING);
    {
        const ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.candidate_count, 2);
    CHECK_TRUE(diagnostics.budget_exhausted_count > 0);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 8), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(tracking_optimized_status_is_success(diagnostics.calculation_status));
    CHECK_EQ_INT(diagnostics.mode, RUCKIG_TRACKING_OPTIMIZED);
    CHECK_TRUE(diagnostics.candidate_count >= count);
    CHECK_TRUE(diagnostics.candidate_count <= count * 8);
    CHECK_TRUE(diagnostics.fallback_step_count + diagnostics.optimized_step_count == count);
    CHECK_EQ_INT(diagnostics.error_step_count, 0);
    CHECK_TRUE(diagnostics.fast_score >= diagnostics.best_score - 1e-12);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}


static void test_tracking_validation(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_t* target_2d = NULL;
    ruckig_target_state_sequence_t* target_sequence = NULL;
    ruckig_target_state_sequence_t* empty_target_sequence = NULL;
    ruckig_tracking_output_sequence_t* output_sequence = NULL;
    ruckig_tracking_output_sequence_t* small_output_sequence = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* input_2d = NULL;
    ruckig_output_t* output = NULL;
    ruckig_control_interface_t per_dof_control[1] = {RUCKIG_CONTROL_POSITION};

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target_2d, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input_2d, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&target_sequence, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&empty_target_sequence, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&output_sequence, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&small_output_sequence, 1, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);

    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, -0.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, 1.01), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, NAN), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, (ruckig_tracking_mode_t)99), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_tracking_update(NULL, target, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, NULL, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, NULL, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target_2d, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input_2d, output), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_target_state_position_data(target)[0] = NAN;
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_ERROR_INVALID_INPUT);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_POSITION), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_dof_control, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_per_dof_control_interface(input);

    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    {
        ruckig_result_t optimized_result = ruckig_tracking_update(tracking, target, input, output);
        CHECK_TRUE(optimized_result == RUCKIG_WORKING || optimized_result == RUCKIG_FINISHED);
        CHECK_TRUE(
            ruckig_tracking_get_last_calculation_status(tracking) == RUCKIG_TRACKING_CALCULATION_OPTIMIZED
            || ruckig_tracking_get_last_calculation_status(tracking) == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK
        );
        CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= 1);
    }
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(target_sequence, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, target_sequence, input, output_sequence), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(output_sequence), 2);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, target_sequence, input, small_output_sequence), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, empty_target_sequence, input, output_sequence), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_target_state_sequence_position_data(target_sequence)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, target_sequence, input, output_sequence), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_last_calculation_status(tracking), RUCKIG_TRACKING_CALCULATION_ERROR);

    ruckig_tracking_output_sequence_destroy(small_output_sequence);
    ruckig_tracking_output_sequence_destroy(output_sequence);
    ruckig_target_state_sequence_destroy(empty_target_sequence);
    ruckig_target_state_sequence_destroy(target_sequence);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input_2d);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target_2d);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_online_fast(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    double original_target_position = 0.0;
    size_t step;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, 1.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 1), RUCKIG_WORKING);

    for (step = 0; step < 50; ++step) {
        fill_tracking_target_ramp(target, (double)step * 0.01);
        original_target_position = ruckig_input_target_position_const_data(input)[0];
        CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_input_target_position_const_data(input)[0], original_target_position, 0.0);
        CHECK_TRUE(ruckig_output_get_time(output) >= 0.0);
        CHECK_TRUE(isfinite(ruckig_output_new_position_data(output)[0]));
        CHECK_TRUE(isfinite(ruckig_output_new_velocity_data(output)[0]));
        check_tracking_output_constraints(output, input, 1);
        ruckig_output_pass_to_input(output, input);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_fixed_corpus(void) {
    const double reactiveness_values[4] = {0.0, 0.25, 0.5, 1.0};
    const size_t look_ahead_values[4] = {1, 2, 5, 10};
    size_t signal;
    size_t r;
    size_t l;

    for (signal = 0; signal < 3; ++signal) {
        for (r = 0; r < 4; ++r) {
            for (l = 0; l < 4; ++l) {
                double final_a[4] = {0.0, 0.0, 0.0, 0.0};
                double final_b[4] = {0.0, 0.0, 0.0, 0.0};
                run_tracking_loop_final(1, (int)signal, reactiveness_values[r], look_ahead_values[l], 120, final_a);
                run_tracking_loop_final(1, (int)signal, reactiveness_values[r], look_ahead_values[l], 120, final_b);
                CHECK_NEAR(final_a[0], final_b[0], 0.0);
            }
        }
    }

    {
        double final_a[4] = {0.0, 0.0, 0.0, 0.0};
        double final_b[4] = {0.0, 0.0, 0.0, 0.0};
        run_tracking_loop_final(2, 2, 1.0, 2, 180, final_a);
        run_tracking_loop_final(2, 2, 1.0, 2, 180, final_b);
        CHECK_NEAR(final_a[0], final_b[0], 0.0);
        CHECK_NEAR(final_a[1], final_b[1], 0.0);
    }

    {
        double final_a[4] = {0.0, 0.0, 0.0, 0.0};
        double final_b[4] = {0.0, 0.0, 0.0, 0.0};
        run_tracking_loop_final(4, 1, 0.5, 5, 160, final_a);
        run_tracking_loop_final(4, 1, 0.5, 5, 160, final_b);
        CHECK_NEAR(final_a[0], final_b[0], 0.0);
        CHECK_NEAR(final_a[1], final_b[1], 0.0);
        CHECK_NEAR(final_a[2], final_b[2], 0.0);
        CHECK_NEAR(final_a[3], final_b[3], 0.0);
    }

    {
        ruckig_tracking_t* tracking = NULL;
        ruckig_target_state_t* target = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        size_t step;
        CHECK_EQ_INT(ruckig_tracking_create(&tracking, 2, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_create(&target, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output, 2), RUCKIG_WORKING);
        fill_tracking_input_nd(input, 2);
        ruckig_input_current_position_data(input)[1] = -0.25;
        ruckig_input_target_position_data(input)[1] = -0.25;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
        for (step = 0; step < 80; ++step) {
            set_tracking_target_signal(target, 0, 2, (double)step * 0.01);
            CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
            CHECK_TRUE(isfinite(ruckig_output_new_position_data(output)[0]));
            CHECK_NEAR(ruckig_output_new_position_data(output)[1], -0.25, 1e-12);
            CHECK_NEAR(ruckig_output_new_velocity_data(output)[1], 0.0, 1e-12);
            CHECK_NEAR(ruckig_output_new_acceleration_data(output)[1], 0.0, 1e-12);
            ruckig_output_pass_to_input(output, input);
        }
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_target_state_destroy(target);
        ruckig_tracking_destroy(tracking);
    }
}

static void test_tracking_offline_sequence(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    double* position;
    double* velocity;
    double* acceleration;
    const double* output_position;
    const double* output_time;
    const ruckig_result_t* results;
    size_t step;
    const size_t count = 64;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    position = ruckig_target_state_sequence_position_data(targets);
    velocity = ruckig_target_state_sequence_velocity_data(targets);
    acceleration = ruckig_target_state_sequence_acceleration_data(targets);
    for (step = 0; step < count; ++step) {
        const double t = (double)step * 0.01;
        position[step] = sin(0.4 * t);
        velocity[step] = 0.4 * cos(0.4 * t);
        acceleration[step] = -0.16 * sin(0.4 * t);
    }

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), count);
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    output_position = ruckig_tracking_output_sequence_new_position_const_data(outputs);
    output_time = ruckig_tracking_output_sequence_time_const_data(outputs);
    results = ruckig_tracking_output_sequence_result_const_data(outputs);
    for (step = 0; step < count; ++step) {
        CHECK_TRUE(isfinite(output_position[step]));
        CHECK_NEAR(output_time[step], (double)(step + 1) * 0.01, 1e-12);
        CHECK_TRUE(results[step] == RUCKIG_WORKING || results[step] == RUCKIG_FINISHED);
        if (step > 0) {
            CHECK_TRUE(output_time[step] > output_time[step - 1]);
        }
    }

    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_offline_invariants(void) {
    const size_t dofs = 2;
    const size_t count = 96;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    CHECK_EQ_INT(ruckig_tracking_create(&tracking, dofs, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, dofs), RUCKIG_WORKING);
    fill_tracking_input_nd(input, dofs);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, dofs, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    check_tracking_output_sequence(outputs, dofs, count, 0.01);
    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);

    {
        ruckig_tracking_t* partial_tracking = NULL;
        ruckig_target_state_sequence_t* partial_targets = NULL;
        ruckig_tracking_output_sequence_t* partial_outputs = NULL;
        ruckig_input_t* partial_input = NULL;
        double* positions;
        CHECK_EQ_INT(ruckig_tracking_create(&partial_tracking, 1, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_sequence_create(&partial_targets, 1, 5), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&partial_outputs, 1, 5), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&partial_input, 1), RUCKIG_WORKING);
        fill_tracking_input_1d(partial_input);
        CHECK_EQ_INT(ruckig_target_state_sequence_set_count(partial_targets, 5), RUCKIG_WORKING);
        set_tracking_sequence_signal(partial_targets, 0, 1, 5, 0.01);
        positions = ruckig_target_state_sequence_position_data(partial_targets);
        positions[3] = NAN;
        CHECK_EQ_INT(
            ruckig_tracking_calculate_sequence(partial_tracking, partial_targets, partial_input, partial_outputs),
            RUCKIG_ERROR_INVALID_INPUT
        );
        CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(partial_outputs), 3);
        ruckig_input_destroy(partial_input);
        ruckig_tracking_output_sequence_destroy(partial_outputs);
        ruckig_target_state_sequence_destroy(partial_targets);
        ruckig_tracking_destroy(partial_tracking);
    }
}

static void test_tracking_optimized_single_target_and_lookahead(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    size_t step;
    const size_t lookahead_count = 4;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, 1, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);

    for (step = 0; step < 24; ++step) {
        set_tracking_target_signal(target, 0, 1, (double)step * 0.01);
        {
            ruckig_result_t result = ruckig_tracking_update(tracking, target, input, output);
            CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
        }
        CHECK_TRUE(tracking_optimized_status_is_success(ruckig_tracking_get_last_calculation_status(tracking)));
        CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= 1);
        CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) <= 16);
        check_tracking_output_constraints(output, input, 1);
        ruckig_output_pass_to_input(output, input);
    }

    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, lookahead_count), RUCKIG_WORKING);
    set_tracking_sequence_signal(lookahead, 1, 1, lookahead_count, 0.01);
    {
        ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_TRUE(tracking_optimized_status_is_success(ruckig_tracking_get_last_calculation_status(tracking)));
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= 1);
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) <= 16);
    check_tracking_output_constraints(output, input, 1);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_optimized_offline_sequence(void) {
    const size_t dofs = 2;
    const size_t count = 48;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, dofs, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, dofs, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, dofs), RUCKIG_WORKING);
    fill_tracking_input_nd(input, dofs);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 5), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 12), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, dofs, count, 0.01);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_get_count(outputs), count);
    CHECK_TRUE(tracking_optimized_status_is_success(ruckig_tracking_get_last_calculation_status(tracking)));
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= count);
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) <= count * 12);
    check_tracking_output_sequence(outputs, dofs, count, 0.01);

    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_optimized_validation_and_diagnostics(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* empty_lookahead = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&empty_lookahead, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_tracking_update_with_lookahead(tracking, empty_lookahead, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_last_calculation_status(tracking), RUCKIG_TRACKING_CALCULATION_ERROR);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, 2), RUCKIG_WORKING);
    set_tracking_sequence_signal(lookahead, 0, 1, 2, 0.01);
    ruckig_target_state_sequence_position_data(lookahead)[1] = NAN;
    CHECK_EQ_INT(ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_last_calculation_status(tracking), RUCKIG_TRACKING_CALCULATION_ERROR);

    ruckig_target_state_position_data(target)[0] = NAN;
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_tracking_get_last_calculation_status(tracking), RUCKIG_TRACKING_CALCULATION_ERROR);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_target_state_sequence_destroy(empty_lookahead);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

void run_tracking_api_lifecycle_tests(void) {
    test_tracking_api_lifecycle_and_accessors();
}

void run_tracking_api_tests(void) {
    run_tracking_api_lifecycle_tests();
    test_tracking_diagnostics_snapshots();
}

void run_tracking_validation_tests(void) {
    test_tracking_validation();
}

void run_tracking_online_tests(void) {
    test_tracking_online_fast();
    test_tracking_fixed_corpus();
}

void run_tracking_fixed_corpus_tests(void) {
    test_tracking_fixed_corpus();
}

void run_tracking_offline_tests(void) {
    test_tracking_offline_sequence();
    test_tracking_offline_invariants();
}

void run_tracking_optimized_tests(void) {
    test_tracking_optimized_single_target_and_lookahead();
    test_tracking_optimized_offline_sequence();
    test_tracking_optimized_validation_and_diagnostics();
    run_tracking_optimized_quality_regression_tests();
    run_tracking_random_audit_fixed_case_tests();
    run_tracking_stability_tests();
}

void run_tracking_tests(void) {
    run_tracking_api_tests();
    run_tracking_public_diagnostics_tests();
    run_tracking_validation_tests();
    run_tracking_online_tests();
    run_tracking_offline_tests();
    run_tracking_optimized_tests();
    run_tracking_quality_tests();
    run_tracking_no_allocation_tests();
}
