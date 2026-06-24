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

static void tracking_signal_value(int signal, size_t dof, double time, double* position, double* velocity, double* acceleration) {
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

static void test_tracking_optimized_quality_against_fast_baseline(void) {
    ruckig_tracking_t* fast_tracking = NULL;
    ruckig_tracking_t* optimized_tracking = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* fast_input = NULL;
    ruckig_input_t* optimized_input = NULL;
    ruckig_output_t* fast_output = NULL;
    ruckig_output_t* optimized_output = NULL;
    const double dt = 0.01;
    const size_t steps = 120;
    const size_t lookahead_count = 5;
    size_t step;
    double fast_error_sum = 0.0;
    double optimized_error_sum = 0.0;

    CHECK_EQ_INT(ruckig_tracking_create(&fast_tracking, 1, dt), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_create(&optimized_tracking, 1, dt), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, 1, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&fast_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&optimized_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&fast_output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&optimized_output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(fast_input);
    fill_tracking_input_1d(optimized_input);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(fast_tracking, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(optimized_tracking, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_mode(optimized_tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(optimized_tracking, 16), RUCKIG_WORKING);

    for (step = 0; step < steps; ++step) {
        size_t sample;
        double target_position;
        double target_velocity;
        double target_acceleration;
        CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, lookahead_count), RUCKIG_WORKING);
        for (sample = 0; sample < lookahead_count; ++sample) {
            tracking_signal_value(1, 0, (double)(step + sample) * dt, &target_position, &target_velocity, &target_acceleration);
            ruckig_target_state_sequence_position_data(lookahead)[sample] = target_position;
            ruckig_target_state_sequence_velocity_data(lookahead)[sample] = target_velocity;
            ruckig_target_state_sequence_acceleration_data(lookahead)[sample] = target_acceleration;
        }
        CHECK_EQ_INT(ruckig_tracking_update_with_lookahead(fast_tracking, lookahead, fast_input, fast_output), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_update_with_lookahead(optimized_tracking, lookahead, optimized_input, optimized_output), RUCKIG_WORKING);
        tracking_signal_value(1, 0, (double)(step + 1) * dt, &target_position, &target_velocity, &target_acceleration);
        (void)target_velocity;
        (void)target_acceleration;
        fast_error_sum += fabs(target_position - ruckig_output_new_position_data(fast_output)[0]);
        optimized_error_sum += fabs(target_position - ruckig_output_new_position_data(optimized_output)[0]);
        ruckig_output_pass_to_input(fast_output, fast_input);
        ruckig_output_pass_to_input(optimized_output, optimized_input);
    }

    printf(
        "tracking optimized quality constant_acceleration: optimized %.9g fast %.9g candidates %zu status %d\n",
        optimized_error_sum,
        fast_error_sum,
        ruckig_tracking_get_last_candidate_count(optimized_tracking),
        (int)ruckig_tracking_get_last_calculation_status(optimized_tracking)
    );
    CHECK_TRUE(optimized_error_sum <= fast_error_sum + 1e-9);

    ruckig_output_destroy(optimized_output);
    ruckig_output_destroy(fast_output);
    ruckig_input_destroy(optimized_input);
    ruckig_input_destroy(fast_input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_tracking_destroy(optimized_tracking);
    ruckig_tracking_destroy(fast_tracking);
}

static void tracking_strategy_metric_weights(
    ruckig_tracking_optimized_strategy_t strategy,
    double* position_weight,
    double* velocity_weight,
    double* acceleration_weight,
    double* jerk_weight,
    double* terminal_weight,
    double* horizon_step
) {
    if (strategy == RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE) {
        *position_weight = 2.0;
        *velocity_weight = 0.10;
        *acceleration_weight = 0.004;
        *jerk_weight = 0.00002;
        *terminal_weight = 8.0;
        *horizon_step = 1.5;
    } else if (strategy == RUCKIG_TRACKING_OPTIMIZED_STABLE) {
        *position_weight = 1.0;
        *velocity_weight = 0.05;
        *acceleration_weight = 0.005;
        *jerk_weight = 0.0001;
        *terminal_weight = 4.0;
        *horizon_step = 1.0;
    } else {
        *position_weight = 1.25;
        *velocity_weight = 0.08;
        *acceleration_weight = 0.006;
        *jerk_weight = 0.00008;
        *terminal_weight = 5.0;
        *horizon_step = 1.25;
    }
}

static double score_tracking_output_horizon(
    const ruckig_output_t* output,
    const ruckig_target_state_sequence_t* lookahead,
    size_t dofs,
    size_t count,
    double delta_time,
    ruckig_tracking_optimized_strategy_t metric_strategy
) {
    double position[8];
    double velocity[8];
    double acceleration[8];
    double jerk[8];
    const double* target_position = ruckig_target_state_sequence_position_const_data(lookahead);
    const double* target_velocity = ruckig_target_state_sequence_velocity_const_data(lookahead);
    const double* target_acceleration = ruckig_target_state_sequence_acceleration_const_data(lookahead);
    const ruckig_trajectory_t* trajectory = ruckig_output_get_trajectory(output);
    double position_weight;
    double velocity_weight;
    double acceleration_weight;
    double jerk_weight;
    double terminal_weight;
    double horizon_step;
    double score = 0.0;
    size_t sample;
    CHECK_TRUE(dofs <= 8);
    tracking_strategy_metric_weights(
        metric_strategy,
        &position_weight,
        &velocity_weight,
        &acceleration_weight,
        &jerk_weight,
        &terminal_weight,
        &horizon_step
    );
    for (sample = 0; sample < count; ++sample) {
        size_t section = 0;
        size_t dof;
        double weight = 1.0 + horizon_step * (double)sample;
        CHECK_EQ_INT(
            ruckig_trajectory_at_time(
                trajectory,
                (double)(sample + 1) * delta_time,
                position,
                velocity,
                acceleration,
                jerk,
                &section
            ),
            RUCKIG_WORKING
        );
        (void)section;
        if (sample + 1 == count) {
            weight *= terminal_weight;
        }
        for (dof = 0; dof < dofs; ++dof) {
            const size_t offset = sample * dofs + dof;
            const double position_error = position[dof] - target_position[offset];
            const double velocity_error = velocity[dof] - target_velocity[offset];
            const double acceleration_error = acceleration[dof] - target_acceleration[offset];
            CHECK_TRUE(isfinite(position_error));
            CHECK_TRUE(isfinite(velocity_error));
            CHECK_TRUE(isfinite(acceleration_error));
            CHECK_TRUE(isfinite(jerk[dof]));
            score += weight * (
                position_weight * position_error * position_error
                + velocity_weight * velocity_error * velocity_error
                + acceleration_weight * acceleration_error * acceleration_error
                + jerk_weight * jerk[dof] * jerk[dof]
            );
        }
    }
    return score;
}

static double run_tracking_strategy_quality_case(
    int signal,
    ruckig_tracking_mode_t mode,
    ruckig_tracking_optimized_strategy_t strategy,
    ruckig_tracking_optimized_strategy_t metric_strategy,
    size_t lookahead_count,
    size_t steps,
    size_t* candidate_count,
    size_t* fallback_count
) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    const double dt = 0.01;
    double score = 0.0;
    size_t step;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, dt), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, 1, lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, mode), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, lookahead_count), RUCKIG_WORKING);
    if (mode == RUCKIG_TRACKING_OPTIMIZED) {
        CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, strategy), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    }

    *candidate_count = 0;
    *fallback_count = 0;
    for (step = 0; step < steps; ++step) {
        size_t sample;
        CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, lookahead_count), RUCKIG_WORKING);
        for (sample = 0; sample < lookahead_count; ++sample) {
            double target_position;
            double target_velocity;
            double target_acceleration;
            tracking_signal_value(signal, 0, (double)(step + sample) * dt, &target_position, &target_velocity, &target_acceleration);
            ruckig_target_state_sequence_position_data(lookahead)[sample] = target_position;
            ruckig_target_state_sequence_velocity_data(lookahead)[sample] = target_velocity;
            ruckig_target_state_sequence_acceleration_data(lookahead)[sample] = target_acceleration;
        }
        {
            const ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
            CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
        }
        check_tracking_output_constraints(output, input, 1);
        score += score_tracking_output_horizon(output, lookahead, 1, lookahead_count, dt, metric_strategy);
        *candidate_count += ruckig_tracking_get_last_candidate_count(tracking);
        if (ruckig_tracking_get_last_calculation_status(tracking) == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
            ++(*fallback_count);
        }
        ruckig_output_pass_to_input(output, input);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_tracking_destroy(tracking);
    return score;
}

static void check_tracking_strategy_quality_case(
    int signal,
    const char* name,
    size_t lookahead_count,
    size_t steps,
    bool require_balanced_improvement,
    bool require_aggressive_improvement
) {
    size_t fast_candidates = 0;
    size_t fast_fallbacks = 0;
    size_t balanced_candidates = 0;
    size_t balanced_fallbacks = 0;
    size_t balanced_aggressive_metric_candidates = 0;
    size_t balanced_aggressive_metric_fallbacks = 0;
    size_t aggressive_candidates = 0;
    size_t aggressive_fallbacks = 0;
    const double fast_score = run_tracking_strategy_quality_case(
        signal,
        RUCKIG_TRACKING_FAST,
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        lookahead_count,
        steps,
        &fast_candidates,
        &fast_fallbacks
    );
    const double balanced_score = run_tracking_strategy_quality_case(
        signal,
        RUCKIG_TRACKING_OPTIMIZED,
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        lookahead_count,
        steps,
        &balanced_candidates,
        &balanced_fallbacks
    );
    printf(
        "tracking strategy quality %s balanced_metric: balanced %.9g fast %.9g candidates %zu fallbacks %zu improvement %.6f\n",
        name,
        balanced_score,
        fast_score,
        balanced_candidates,
        balanced_fallbacks,
        fast_score > 0.0 ? (fast_score - balanced_score) / fast_score : 0.0
    );
    CHECK_TRUE(balanced_score <= fast_score + 1e-9);
    if (require_balanced_improvement) {
        CHECK_TRUE(balanced_score <= 0.995 * fast_score);
    }

    if (require_aggressive_improvement) {
        const double balanced_aggressive_metric_score = run_tracking_strategy_quality_case(
            signal,
            RUCKIG_TRACKING_OPTIMIZED,
            RUCKIG_TRACKING_OPTIMIZED_BALANCED,
            RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
            lookahead_count,
            steps,
            &balanced_aggressive_metric_candidates,
            &balanced_aggressive_metric_fallbacks
        );
        const double aggressive_score = run_tracking_strategy_quality_case(
            signal,
            RUCKIG_TRACKING_OPTIMIZED,
            RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
            RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE,
            lookahead_count,
            steps,
            &aggressive_candidates,
            &aggressive_fallbacks
        );
        (void)balanced_aggressive_metric_candidates;
        (void)balanced_aggressive_metric_fallbacks;
        printf(
            "tracking strategy quality %s aggressive_metric: aggressive %.9g balanced %.9g candidates %zu fallbacks %zu improvement %.6f\n",
            name,
            aggressive_score,
            balanced_aggressive_metric_score,
            aggressive_candidates,
            aggressive_fallbacks,
            balanced_aggressive_metric_score > 0.0 ? (balanced_aggressive_metric_score - aggressive_score) / balanced_aggressive_metric_score : 0.0
        );
        CHECK_TRUE(aggressive_score <= 0.98 * balanced_aggressive_metric_score);
    }
}

static void test_tracking_optimized_strategy_quality_corpus(void) {
    check_tracking_strategy_quality_case(0, "ramp", 5, 120, true, false);
    check_tracking_strategy_quality_case(1, "constant_acceleration", 5, 120, true, false);
    check_tracking_strategy_quality_case(2, "sinus", 8, 160, false, true);
    check_tracking_strategy_quality_case(3, "half_sinus", 5, 120, false, true);
}

static void measure_tracking_quality_case(
    int signal,
    const char* name,
    double reactiveness,
    size_t look_ahead_cycles,
    bool hard_gate
) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* tracking_input = NULL;
    ruckig_input_t* naive_input = NULL;
    ruckig_output_t* tracking_output = NULL;
    ruckig_output_t* naive_output = NULL;
    ruckig_t* naive_otg = NULL;
    const double dt = 0.01;
    const size_t steps = 160;
    size_t step;
    double tracking_lag_sum = 0.0;
    double naive_lag_sum = 0.0;
    double tracking_lag_max = 0.0;
    double naive_lag_max = 0.0;
    double tracking_lag_final = 0.0;
    double naive_lag_final = 0.0;
    double improvement_ratio;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, dt), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&tracking_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&naive_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&tracking_output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&naive_output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create(&naive_otg, 1, dt), RUCKIG_WORKING);
    fill_tracking_input_1d(tracking_input);
    fill_tracking_input_1d(naive_input);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, reactiveness), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, look_ahead_cycles), RUCKIG_WORKING);

    for (step = 0; step < steps; ++step) {
        double future_position;
        double unused_velocity;
        double unused_acceleration;
        double tracking_lag;
        double naive_lag;
        const double time = (double)step * dt;
        set_tracking_target_signal(target, signal, 1, time);
        CHECK_TRUE(ruckig_tracking_update(tracking, target, tracking_input, tracking_output) >= 0);
        ruckig_input_target_position_data(naive_input)[0] = ruckig_target_state_position_const_data(target)[0];
        ruckig_input_target_velocity_data(naive_input)[0] = 0.0;
        ruckig_input_target_acceleration_data(naive_input)[0] = 0.0;
        CHECK_TRUE(ruckig_update(naive_otg, naive_input, naive_output) >= 0);
        tracking_signal_value(signal, 0, time + dt, &future_position, &unused_velocity, &unused_acceleration);
        tracking_lag = fabs(future_position - ruckig_output_new_position_data(tracking_output)[0]);
        naive_lag = fabs(future_position - ruckig_output_new_position_data(naive_output)[0]);
        tracking_lag_sum += tracking_lag;
        naive_lag_sum += naive_lag;
        if (tracking_lag > tracking_lag_max) {
            tracking_lag_max = tracking_lag;
        }
        if (naive_lag > naive_lag_max) {
            naive_lag_max = naive_lag;
        }
        tracking_lag_final = tracking_lag;
        naive_lag_final = naive_lag;
        ruckig_output_pass_to_input(tracking_output, tracking_input);
        ruckig_output_pass_to_input(naive_output, naive_input);
    }

    improvement_ratio = naive_lag_sum > 0.0 ? (naive_lag_sum - tracking_lag_sum) / naive_lag_sum : 0.0;
    printf(
        "tracking quality %s: avg_fast %.9g avg_naive %.9g max_fast %.9g max_naive %.9g final_fast %.9g final_naive %.9g improvement %.6f\n",
        name,
        tracking_lag_sum / (double)steps,
        naive_lag_sum / (double)steps,
        tracking_lag_max,
        naive_lag_max,
        tracking_lag_final,
        naive_lag_final,
        improvement_ratio
    );
    if (hard_gate) {
        CHECK_TRUE(tracking_lag_sum <= naive_lag_sum + 1e-9);
        CHECK_TRUE(tracking_lag_final <= naive_lag_final + 1e-9);
    }

    ruckig_destroy(naive_otg);
    ruckig_output_destroy(naive_output);
    ruckig_output_destroy(tracking_output);
    ruckig_input_destroy(naive_input);
    ruckig_input_destroy(tracking_input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_quality_against_instantaneous_chasing(void) {
    measure_tracking_quality_case(0, "ramp_tuned", 1.0, 20, true);
    measure_tracking_quality_case(1, "constant_acceleration", 1.0, 2, true);
    measure_tracking_quality_case(2, "sinus_trend", 1.0, 1, false);
}

static void test_tracking_no_allocation(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    size_t allocations_before;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, 2), RUCKIG_WORKING);
    ruckig_target_state_sequence_position_data(targets)[0] = 0.0;
    ruckig_target_state_sequence_position_data(targets)[1] = 0.005;
    ruckig_target_state_sequence_velocity_data(targets)[0] = 0.5;
    ruckig_target_state_sequence_velocity_data(targets)[1] = 0.5;
    ruckig_target_state_sequence_acceleration_data(targets)[0] = 0.0;
    ruckig_target_state_sequence_acceleration_data(targets)[1] = 0.0;

    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_allocation_forbidden_set(false);

    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 8), RUCKIG_WORKING);
    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    {
        ruckig_result_t result = ruckig_tracking_update(tracking, target, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    {
        ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
        CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    }
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_allocation_forbidden_set(false);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);

    {
        ruckig_tracking_t* tracking4 = NULL;
        ruckig_target_state_t* target4 = NULL;
        ruckig_target_state_sequence_t* targets4 = NULL;
        ruckig_tracking_output_sequence_t* outputs4 = NULL;
        ruckig_input_t* input4 = NULL;
        ruckig_output_t* output4 = NULL;
        size_t step;
        CHECK_EQ_INT(ruckig_tracking_create(&tracking4, 4, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_create(&target4, 4), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets4, 4, 16), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs4, 4, 16), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input4, 4), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output4, 4), RUCKIG_WORKING);
        fill_tracking_input_nd(input4, 4);
        CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets4, 16), RUCKIG_WORKING);
        set_tracking_sequence_signal(targets4, 2, 4, 16, 0.01);
        ruckig_allocation_counters_reset();
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        for (step = 0; step < 12; ++step) {
            set_tracking_target_signal(target4, 2, 4, (double)step * 0.01);
            CHECK_EQ_INT(ruckig_tracking_update(tracking4, target4, input4, output4), RUCKIG_WORKING);
            CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
            CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
            ruckig_output_pass_to_input(output4, input4);
        }
        CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking4, targets4, input4, outputs4), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        ruckig_allocation_forbidden_set(false);
        ruckig_output_destroy(output4);
        ruckig_input_destroy(input4);
        ruckig_tracking_output_sequence_destroy(outputs4);
        ruckig_target_state_sequence_destroy(targets4);
        ruckig_target_state_destroy(target4);
        ruckig_tracking_destroy(tracking4);
    }
}

static unsigned tracking_random_next(unsigned* state) {
    *state = 1664525u * (*state) + 1013904223u;
    return *state;
}

static size_t tracking_random_pick(unsigned* state, size_t count) {
    return (size_t)(tracking_random_next(state) % (unsigned)count);
}

static size_t tracking_random_audit_pick(unsigned* state, size_t count) {
    return (size_t)((tracking_random_next(state) >> 8u) % (unsigned)count);
}

static bool tracking_random_audit_bool(unsigned* state) {
    return ((tracking_random_next(state) >> 8u) & 1u) != 0u;
}

typedef struct tracking_random_case_config {
    size_t sample_index;
    size_t dofs;
    size_t lookahead_count;
    int signal;
    double reactiveness;
    ruckig_tracking_optimized_strategy_t strategy;
    bool has_disabled_dof;
    size_t disabled_dof;
    double start_time;
} tracking_random_case_config_t;

typedef struct tracking_random_case_result {
    tracking_random_case_config_t config;
    ruckig_result_t result;
    ruckig_tracking_calculation_status_t status;
    size_t candidate_count;
} tracking_random_case_result_t;

typedef struct tracking_audit_case_config {
    size_t sample_index;
    size_t dofs;
    size_t lookahead_count;
    int signal;
    double reactiveness;
    ruckig_tracking_optimized_strategy_t strategy;
    bool has_disabled_dof;
    size_t disabled_dof;
    bool tight_constraints;
    double start_time;
} tracking_audit_case_config_t;

typedef struct tracking_audit_case_result {
    tracking_audit_case_config_t config;
    ruckig_result_t result;
    ruckig_tracking_calculation_status_t status;
    ruckig_tracking_diagnostics_t diagnostics;
    size_t family_attempted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_valid[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_strict_improved[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_near_tie_accepted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_selected[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t selected_family;
    bool selected_near_tie;
    size_t strict_improved_count;
    size_t near_tie_accepted_count;
} tracking_audit_case_result_t;

typedef struct tracking_audit_bucket {
    size_t samples;
    size_t optimized;
    size_t fallback;
    size_t candidates;
    size_t valid;
    size_t rejected;
    size_t budget_exhausted;
    size_t family_attempted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_valid[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_strict_improved[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_near_tie_accepted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_selected[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t strict_improved_count;
    size_t near_tie_accepted_count;
    double improvement_sum;
} tracking_audit_bucket_t;

typedef struct tracking_audit_stats {
    tracking_audit_bucket_t overall;
    tracking_audit_bucket_t by_strategy[3];
    tracking_audit_bucket_t by_dof[4];
    tracking_audit_bucket_t by_signal[4];
    tracking_audit_bucket_t by_lookahead[4];
    tracking_audit_bucket_t by_reactiveness[4];
    tracking_audit_bucket_t by_disabled[2];
    tracking_audit_bucket_t by_constraints[2];
} tracking_audit_stats_t;

typedef struct tracking_audit_representatives {
    tracking_audit_case_result_t cases[8];
    const char* reasons[8];
    size_t count;
    bool strategy_seen[3];
    bool disabled_seen;
    bool tight_seen;
    bool budget_seen;
} tracking_audit_representatives_t;

typedef struct tracking_audit_threshold {
    size_t samples;
    unsigned seed;
    size_t baseline_optimized[3];
    size_t required_optimized[3];
    double baseline_average_improvement[3];
    double required_average_improvement[3];
} tracking_audit_threshold_t;

typedef struct tracking_stability_case {
    const char* name;
    tracking_audit_case_config_t config;
    ruckig_tracking_calculation_status_t expected_status;
    size_t expected_family;
    bool expected_near_tie;
    bool expect_budget_exhausted;
    bool expect_positive_improvement;
} tracking_stability_case_t;

static const char* tracking_audit_family_names[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT] = {
    "fast",
    "instantaneous",
    "horizon",
    "terminal_blend",
    "derivative_damped",
    "lead_lag"
};

static const size_t tracking_random_dof_values[4] = {1, 2, 4, 8};
static const size_t tracking_random_lookahead_values[4] = {1, 2, 5, 10};
static const double tracking_random_reactiveness_values[4] = {0.0, 0.25, 0.5, 1.0};
static const ruckig_tracking_optimized_strategy_t tracking_random_strategy_values[3] = {
    RUCKIG_TRACKING_OPTIMIZED_STABLE,
    RUCKIG_TRACKING_OPTIMIZED_BALANCED,
    RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE
};

static const tracking_audit_threshold_t tracking_audit_thresholds[] = {
    {
        10000,
        1u,
        {268, 254, 262},
        {335, 318, 328},
        {0.00696885785, 0.00573055088, 0.00735498978},
        {0.007665743635, 0.006303605968, 0.008090488758}
    },
    {
        100000,
        1u,
        {2628, 2601, 2573},
        {3285, 3252, 3217},
        {0.00654911563, 0.00679519282, 0.00721271345},
        {0.007204027193, 0.007474712102, 0.007933984795}
    },
    {
        100000,
        2u,
        {2648, 2702, 2526},
        {3310, 3378, 3158},
        {0.00587577617, 0.00614356450, 0.00642797412},
        {0.006463353787, 0.006757920950, 0.007070771532}
    },
    {
        100000,
        41u,
        {2638, 2711, 2499},
        {3298, 3389, 3124},
        {0.00792016481, 0.00763398601, 0.00693869317},
        {0.008712181291, 0.008397384611, 0.007632562487}
    },
    {
        1000000,
        1u,
        {26631, 26171, 25308},
        {33289, 32714, 31635},
        {0.00679055094, 0.00672526897, 0.00712030876},
        {0.007469606034, 0.007397795867, 0.007832339636}
    }
};

static const tracking_audit_threshold_t* tracking_audit_find_threshold(size_t samples, unsigned seed) {
    size_t i;
    for (i = 0; i < sizeof(tracking_audit_thresholds) / sizeof(tracking_audit_thresholds[0]); ++i) {
        if (tracking_audit_thresholds[i].samples == samples && tracking_audit_thresholds[i].seed == seed) {
            return &tracking_audit_thresholds[i];
        }
    }
    return NULL;
}

static const char* tracking_strategy_name(ruckig_tracking_optimized_strategy_t strategy) {
    switch (strategy) {
    case RUCKIG_TRACKING_OPTIMIZED_STABLE:
        return "stable";
    case RUCKIG_TRACKING_OPTIMIZED_BALANCED:
        return "balanced";
    case RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE:
        return "aggressive";
    }
    return "unknown";
}

static const char* tracking_strategy_initializer(ruckig_tracking_optimized_strategy_t strategy) {
    switch (strategy) {
    case RUCKIG_TRACKING_OPTIMIZED_STABLE:
        return "RUCKIG_TRACKING_OPTIMIZED_STABLE";
    case RUCKIG_TRACKING_OPTIMIZED_BALANCED:
        return "RUCKIG_TRACKING_OPTIMIZED_BALANCED";
    case RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE:
        return "RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE";
    }
    return "RUCKIG_TRACKING_OPTIMIZED_STABLE";
}

static size_t tracking_strategy_index(ruckig_tracking_optimized_strategy_t strategy) {
    switch (strategy) {
    case RUCKIG_TRACKING_OPTIMIZED_STABLE:
        return 0;
    case RUCKIG_TRACKING_OPTIMIZED_BALANCED:
        return 1;
    case RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE:
        return 2;
    }
    return 0;
}

static const char* tracking_signal_name(int signal) {
    switch (signal) {
    case 0:
        return "ramp";
    case 1:
        return "constant_acceleration";
    case 2:
        return "sinus";
    case 3:
        return "half_sinus";
    }
    return "unknown";
}

static const char* tracking_status_name(ruckig_tracking_calculation_status_t status) {
    switch (status) {
    case RUCKIG_TRACKING_CALCULATION_NONE:
        return "none";
    case RUCKIG_TRACKING_CALCULATION_FAST:
        return "fast";
    case RUCKIG_TRACKING_CALCULATION_OPTIMIZED:
        return "optimized";
    case RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK:
        return "fast_fallback";
    case RUCKIG_TRACKING_CALCULATION_ERROR:
        return "error";
    }
    return "unknown";
}

static const char* tracking_dof_number_name(size_t dof) {
    static const char* names[8] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    return dof < 8 ? names[dof] : "unknown";
}

static size_t tracking_dof_index(size_t dofs) {
    if (dofs == 1) {
        return 0;
    }
    if (dofs == 2) {
        return 1;
    }
    if (dofs == 4) {
        return 2;
    }
    return 3;
}

static size_t tracking_lookahead_index(size_t lookahead_count) {
    if (lookahead_count == 1) {
        return 0;
    }
    if (lookahead_count == 2) {
        return 1;
    }
    if (lookahead_count == 5) {
        return 2;
    }
    return 3;
}

static size_t tracking_reactiveness_index(double reactiveness) {
    if (reactiveness < 0.125) {
        return 0;
    }
    if (reactiveness < 0.375) {
        return 1;
    }
    if (reactiveness < 0.75) {
        return 2;
    }
    return 3;
}

static void apply_tracking_audit_constraints(ruckig_input_t* input, size_t dofs, bool tight_constraints) {
    size_t dof;
    if (!tight_constraints) {
        return;
    }
    for (dof = 0; dof < dofs; ++dof) {
        ruckig_input_max_velocity_data(input)[dof] *= 0.55;
        ruckig_input_max_acceleration_data(input)[dof] *= 0.65;
        ruckig_input_max_jerk_data(input)[dof] *= 0.70;
    }
}

static void fill_tracking_audit_lookahead(
    const tracking_audit_case_config_t* config,
    ruckig_target_state_sequence_t* lookahead
) {
    size_t ahead;
    double* position = ruckig_target_state_sequence_position_data(lookahead);
    double* velocity = ruckig_target_state_sequence_velocity_data(lookahead);
    double* acceleration = ruckig_target_state_sequence_acceleration_data(lookahead);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, config->lookahead_count), RUCKIG_WORKING);
    for (ahead = 0; ahead < config->lookahead_count; ++ahead) {
        size_t dof;
        const double time = config->start_time + (double)ahead * 0.01;
        for (dof = 0; dof < config->dofs; ++dof) {
            tracking_signal_value(
                config->signal,
                dof,
                time,
                &position[ahead * config->dofs + dof],
                &velocity[ahead * config->dofs + dof],
                &acceleration[ahead * config->dofs + dof]
            );
        }
    }
}

static tracking_random_case_config_t make_tracking_random_case_config(unsigned* state, size_t sample_index) {
    tracking_random_case_config_t config;
    memset(&config, 0, sizeof(config));
    config.sample_index = sample_index;
    config.dofs = tracking_random_dof_values[tracking_random_pick(state, 4)];
    config.lookahead_count = tracking_random_lookahead_values[tracking_random_pick(state, 4)];
    config.signal = (int)tracking_random_pick(state, 4);
    config.reactiveness = tracking_random_reactiveness_values[tracking_random_pick(state, 4)];
    config.strategy = tracking_random_strategy_values[tracking_random_pick(state, 3)];
    config.start_time = (double)(sample_index % 200u) * 0.01;
    if (config.dofs > 1 && (tracking_random_next(state) & 1u) != 0u) {
        config.has_disabled_dof = true;
        config.disabled_dof = tracking_random_pick(state, config.dofs);
    }
    return config;
}

static void make_tracking_audit_case_config(unsigned* state, size_t sample_index, tracking_audit_case_config_t* config) {
    memset(config, 0, sizeof(*config));
    config->sample_index = sample_index;
    config->dofs = tracking_random_dof_values[tracking_random_audit_pick(state, 4)];
    config->lookahead_count = tracking_random_lookahead_values[tracking_random_audit_pick(state, 4)];
    config->signal = (int)tracking_random_audit_pick(state, 4);
    config->reactiveness = tracking_random_reactiveness_values[tracking_random_audit_pick(state, 4)];
    config->strategy = tracking_random_strategy_values[tracking_random_audit_pick(state, 3)];
    config->start_time = (double)(sample_index % 200u) * 0.01;
    if (config->dofs > 1 && tracking_random_audit_bool(state)) {
        config->has_disabled_dof = true;
        config->disabled_dof = tracking_random_audit_pick(state, config->dofs);
    }
    config->tight_constraints = tracking_random_audit_bool(state);
}

static void fill_tracking_random_lookahead(
    const tracking_random_case_config_t* config,
    ruckig_target_state_sequence_t* lookahead
) {
    size_t ahead;
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, config->lookahead_count), RUCKIG_WORKING);
    for (ahead = 0; ahead < config->lookahead_count; ++ahead) {
        size_t dof;
        const double time = config->start_time + (double)ahead * 0.01;
        for (dof = 0; dof < config->dofs; ++dof) {
            double position;
            double velocity;
            double acceleration;
            tracking_signal_value(config->signal, dof, time, &position, &velocity, &acceleration);
            ruckig_target_state_sequence_position_data(lookahead)[ahead * config->dofs + dof] = position;
            ruckig_target_state_sequence_velocity_data(lookahead)[ahead * config->dofs + dof] = velocity;
            ruckig_target_state_sequence_acceleration_data(lookahead)[ahead * config->dofs + dof] = acceleration;
        }
    }
}

static void run_tracking_random_case(
    const tracking_random_case_config_t* config,
    tracking_random_case_result_t* case_result
) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    memset(case_result, 0, sizeof(*case_result));
    case_result->config = *config;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, config->dofs, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, config->dofs, config->lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, config->dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, config->dofs), RUCKIG_WORKING);
    fill_tracking_input_nd(input, config->dofs);
    if (config->has_disabled_dof) {
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, config->disabled_dof, false), RUCKIG_WORKING);
    }
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, config->strategy), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, config->reactiveness), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, config->lookahead_count), RUCKIG_WORKING);
    fill_tracking_random_lookahead(config, lookahead);

    case_result->result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
    CHECK_TRUE(case_result->result == RUCKIG_WORKING || case_result->result == RUCKIG_FINISHED);
    case_result->status = ruckig_tracking_get_last_calculation_status(tracking);
    CHECK_TRUE(tracking_optimized_status_is_success(case_result->status));
    case_result->candidate_count = ruckig_tracking_get_last_candidate_count(tracking);
    CHECK_TRUE(case_result->candidate_count >= 1);
    CHECK_TRUE(case_result->candidate_count <= ruckig_tracking_get_max_optimized_candidates(tracking));
    check_tracking_output_constraints(output, input, config->dofs);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_tracking_destroy(tracking);
}

static void run_tracking_audit_case(
    const tracking_audit_case_config_t* config,
    tracking_audit_case_result_t* case_result
) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    memset(case_result, 0, sizeof(*case_result));
    case_result->config = *config;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, config->dofs, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, config->dofs, config->lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, config->dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, config->dofs), RUCKIG_WORKING);
    fill_tracking_input_nd(input, config->dofs);
    apply_tracking_audit_constraints(input, config->dofs, config->tight_constraints);
    if (config->has_disabled_dof) {
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, config->disabled_dof, false), RUCKIG_WORKING);
    }
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, config->strategy), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, config->reactiveness), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, config->lookahead_count), RUCKIG_WORKING);
    fill_tracking_audit_lookahead(config, lookahead);

    case_result->result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
    CHECK_TRUE(case_result->result == RUCKIG_WORKING || case_result->result == RUCKIG_FINISHED);
    case_result->status = ruckig_tracking_get_last_calculation_status(tracking);
    CHECK_TRUE(tracking_optimized_status_is_success(case_result->status));
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= 1);
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) <= ruckig_tracking_get_max_optimized_candidates(tracking));
    check_tracking_output_constraints(output, input, config->dofs);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &case_result->diagnostics), RUCKIG_WORKING);
    check_tracking_diagnostics_common(tracking, &case_result->diagnostics);
    CHECK_EQ_INT(case_result->diagnostics.mode, RUCKIG_TRACKING_OPTIMIZED);
    CHECK_EQ_INT(case_result->diagnostics.optimized_strategy, config->strategy);
    CHECK_TRUE(case_result->diagnostics.fallback_step_count + case_result->diagnostics.optimized_step_count == 1);
    CHECK_EQ_INT(case_result->diagnostics.error_step_count, 0);
    memcpy(case_result->family_attempted, tracking->audit_family_attempted, sizeof(case_result->family_attempted));
    memcpy(case_result->family_valid, tracking->audit_family_valid, sizeof(case_result->family_valid));
    memcpy(case_result->family_strict_improved, tracking->audit_family_strict_improved, sizeof(case_result->family_strict_improved));
    memcpy(case_result->family_near_tie_accepted, tracking->audit_family_near_tie_accepted, sizeof(case_result->family_near_tie_accepted));
    memcpy(case_result->family_selected, tracking->audit_family_selected, sizeof(case_result->family_selected));
    case_result->selected_family = tracking->audit_best_candidate_family;
    case_result->selected_near_tie = tracking->audit_best_candidate_near_tie;
    case_result->strict_improved_count = tracking->audit_strict_improved_count;
    case_result->near_tie_accepted_count = tracking->audit_near_tie_accepted_count;
    {
        size_t family;
        size_t attempted = 0;
        size_t valid = 0;
        size_t selected = 0;
        size_t strict_improved = 0;
        size_t near_tie = 0;
        for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
            attempted += case_result->family_attempted[family];
            valid += case_result->family_valid[family];
            selected += case_result->family_selected[family];
            strict_improved += case_result->family_strict_improved[family];
            near_tie += case_result->family_near_tie_accepted[family];
        }
        CHECK_EQ_INT(attempted, case_result->diagnostics.candidate_count);
        CHECK_EQ_INT(valid, case_result->diagnostics.valid_candidate_count);
        CHECK_EQ_INT(selected, 1);
        CHECK_EQ_INT(strict_improved, case_result->strict_improved_count);
        CHECK_EQ_INT(near_tie, case_result->near_tie_accepted_count);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_tracking_destroy(tracking);
}

static void tracking_audit_add_bucket(
    tracking_audit_bucket_t* bucket,
    const tracking_audit_case_result_t* case_result
) {
    size_t family;
    ++bucket->samples;
    if (case_result->status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
        ++bucket->optimized;
    } else if (case_result->status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
        ++bucket->fallback;
    }
    bucket->candidates += case_result->diagnostics.candidate_count;
    bucket->valid += case_result->diagnostics.valid_candidate_count;
    bucket->rejected += case_result->diagnostics.rejected_candidate_count;
    bucket->budget_exhausted += case_result->diagnostics.budget_exhausted_count;
    bucket->strict_improved_count += case_result->strict_improved_count;
    bucket->near_tie_accepted_count += case_result->near_tie_accepted_count;
    for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
        bucket->family_attempted[family] += case_result->family_attempted[family];
        bucket->family_valid[family] += case_result->family_valid[family];
        bucket->family_strict_improved[family] += case_result->family_strict_improved[family];
        bucket->family_near_tie_accepted[family] += case_result->family_near_tie_accepted[family];
        bucket->family_selected[family] += case_result->family_selected[family];
    }
    bucket->improvement_sum += case_result->diagnostics.improvement_ratio;
}

static void tracking_audit_add_stats(
    tracking_audit_stats_t* stats,
    const tracking_audit_case_result_t* case_result
) {
    const tracking_audit_case_config_t* config = &case_result->config;
    tracking_audit_add_bucket(&stats->overall, case_result);
    tracking_audit_add_bucket(&stats->by_strategy[tracking_strategy_index(config->strategy)], case_result);
    tracking_audit_add_bucket(&stats->by_dof[tracking_dof_index(config->dofs)], case_result);
    tracking_audit_add_bucket(&stats->by_signal[(size_t)config->signal], case_result);
    tracking_audit_add_bucket(&stats->by_lookahead[tracking_lookahead_index(config->lookahead_count)], case_result);
    tracking_audit_add_bucket(&stats->by_reactiveness[tracking_reactiveness_index(config->reactiveness)], case_result);
    tracking_audit_add_bucket(&stats->by_disabled[config->has_disabled_dof ? 1 : 0], case_result);
    tracking_audit_add_bucket(&stats->by_constraints[config->tight_constraints ? 1 : 0], case_result);
}

static void tracking_audit_print_bucket(const char* group, const char* name, const tracking_audit_bucket_t* bucket) {
    const double average_improvement = bucket->samples > 0 ? bucket->improvement_sum / (double)bucket->samples : 0.0;
    printf(
        "tracking random audit %s %s: samples %zu optimized %zu fallback %zu candidates %zu valid %zu rejected %zu budget_exhausted %zu strict_improved %zu near_tie_accepted %zu average_improvement %.9g\n",
        group,
        name,
        bucket->samples,
        bucket->optimized,
        bucket->fallback,
        bucket->candidates,
        bucket->valid,
        bucket->rejected,
        bucket->budget_exhausted,
        bucket->strict_improved_count,
        bucket->near_tie_accepted_count,
        average_improvement
    );
}

static bool tracking_audit_case_recorded(
    const tracking_audit_representatives_t* representatives,
    const tracking_audit_case_result_t* case_result
) {
    size_t i;
    for (i = 0; i < representatives->count; ++i) {
        if (representatives->cases[i].config.sample_index == case_result->config.sample_index) {
            return true;
        }
    }
    return false;
}

static void tracking_audit_record_representative(
    tracking_audit_representatives_t* representatives,
    const tracking_audit_case_result_t* case_result,
    const char* reason
) {
    if (representatives->count >= sizeof(representatives->cases) / sizeof(representatives->cases[0])
        || tracking_audit_case_recorded(representatives, case_result)) {
        return;
    }
    representatives->cases[representatives->count] = *case_result;
    representatives->reasons[representatives->count] = reason;
    ++representatives->count;
}

static void tracking_audit_maybe_record_fallback(
    tracking_audit_representatives_t* representatives,
    const tracking_audit_case_result_t* case_result
) {
    const size_t strategy_index = tracking_strategy_index(case_result->config.strategy);
    if (case_result->status != RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
        return;
    }
    if (!representatives->strategy_seen[strategy_index]) {
        representatives->strategy_seen[strategy_index] = true;
        tracking_audit_record_representative(
            representatives,
            case_result,
            strategy_index == 0 ? "stable_fallback" : (strategy_index == 1 ? "balanced_fallback" : "aggressive_fallback")
        );
    }
    if (case_result->config.has_disabled_dof && !representatives->disabled_seen) {
        representatives->disabled_seen = true;
        tracking_audit_record_representative(representatives, case_result, "disabled_fallback");
    }
    if (case_result->config.tight_constraints && !representatives->tight_seen) {
        representatives->tight_seen = true;
        tracking_audit_record_representative(representatives, case_result, "tight_valid_fallback");
    }
    if (case_result->diagnostics.budget_exhausted_count > 0 && !representatives->budget_seen) {
        representatives->budget_seen = true;
        tracking_audit_record_representative(representatives, case_result, "budget_exhausted_fallback");
    }
}

static void tracking_audit_print_case(
    const char* reason,
    const tracking_audit_case_result_t* case_result,
    unsigned seed
) {
    const tracking_audit_case_config_t* config = &case_result->config;
    const ruckig_tracking_diagnostics_t* diagnostics = &case_result->diagnostics;
    const char* selected_family = case_result->selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT
        ? tracking_audit_family_names[case_result->selected_family]
        : "unknown";
    printf(
        "tracking random audit fallback_case reason=%s seed=%u sample=%zu strategy=%s dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f disabled=%s disabled_dof=%s constraints=%s status=%s selected_family=%s selected_near_tie=%s strict_improved=%zu near_tie_accepted=%zu candidates=%zu fast=%zu instantaneous=%zu horizon=%zu terminal_blend=%zu derivative_damped=%zu lead_lag=%zu budget_exhausted=%zu fast_score=%.9g best_score=%.9g improvement=%.9g\n",
        reason,
        seed,
        config->sample_index,
        tracking_strategy_name(config->strategy),
        config->dofs,
        tracking_signal_name(config->signal),
        config->lookahead_count,
        config->reactiveness,
        config->has_disabled_dof ? "yes" : "no",
        config->has_disabled_dof ? tracking_dof_number_name(config->disabled_dof) : "none",
        config->tight_constraints ? "tight_valid" : "default",
        tracking_status_name(case_result->status),
        selected_family,
        case_result->selected_near_tie ? "yes" : "no",
        case_result->strict_improved_count,
        case_result->near_tie_accepted_count,
        diagnostics->candidate_count,
        diagnostics->fast_candidate_count,
        diagnostics->instantaneous_candidate_count,
        diagnostics->horizon_candidate_count,
        diagnostics->terminal_blend_candidate_count,
        diagnostics->derivative_damped_candidate_count,
        diagnostics->lead_lag_candidate_count,
        diagnostics->budget_exhausted_count,
        diagnostics->fast_score,
        diagnostics->best_score,
        diagnostics->improvement_ratio
    );
}

static void tracking_random_print_fixture(
    const tracking_random_case_config_t* config,
    const tracking_random_case_result_t* case_result,
    unsigned seed
) {
    printf("tracking random replay fixture seed=%u sample=%zu\n", seed, config->sample_index);
    printf(
        "tracking random replay context seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s start_time=%.9g\n",
        seed,
        config->sample_index,
        config->dofs,
        tracking_signal_name(config->signal),
        config->lookahead_count,
        config->reactiveness,
        tracking_strategy_name(config->strategy),
        config->has_disabled_dof ? "yes" : "no",
        config->has_disabled_dof ? tracking_dof_number_name(config->disabled_dof) : "none",
        config->start_time
    );
    printf(
        "const tracking_random_case_config_t case_config = {%zu, %zu, %zu, %d, %.17g, %s, %s, %zu, %.17g};\n",
        config->sample_index,
        config->dofs,
        config->lookahead_count,
        config->signal,
        config->reactiveness,
        tracking_strategy_initializer(config->strategy),
        config->has_disabled_dof ? "true" : "false",
        config->disabled_dof,
        config->start_time
    );
    printf(
        "tracking random replay result seed=%u sample=%zu result=%d status=%s candidates=%zu\n",
        seed,
        config->sample_index,
        (int)case_result->result,
        tracking_status_name(case_result->status),
        case_result->candidate_count
    );
}

static void tracking_audit_print_fixture(
    const tracking_audit_case_config_t* config,
    const tracking_audit_case_result_t* case_result,
    unsigned seed
) {
    printf("tracking random audit replay fixture seed=%u sample=%zu\n", seed, config->sample_index);
    printf(
        "const tracking_audit_case_config_t case_config = {%zu, %zu, %zu, %d, %.17g, %s, %s, %zu, %s, %.17g};\n",
        config->sample_index,
        config->dofs,
        config->lookahead_count,
        config->signal,
        config->reactiveness,
        tracking_strategy_initializer(config->strategy),
        config->has_disabled_dof ? "true" : "false",
        config->disabled_dof,
        config->tight_constraints ? "true" : "false",
        config->start_time
    );
    tracking_audit_print_case("replay", case_result, seed);
}

static double tracking_audit_average_improvement(const tracking_audit_bucket_t* bucket) {
    return bucket->samples > 0 ? bucket->improvement_sum / (double)bucket->samples : 0.0;
}

static void tracking_audit_print_family_summary(const tracking_audit_bucket_t* bucket) {
    size_t family;
    for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
        printf(
            "tracking random audit by_family %s: attempted %zu valid %zu strict_improved %zu near_tie_accepted %zu selected %zu\n",
            tracking_audit_family_names[family],
            bucket->family_attempted[family],
            bucket->family_valid[family],
            bucket->family_strict_improved[family],
            bucket->family_near_tie_accepted[family],
            bucket->family_selected[family]
        );
    }
}

static void tracking_audit_check_thresholds(
    const tracking_audit_stats_t* stats,
    const tracking_audit_threshold_t* threshold
) {
    static const char* strategy_names[3] = {"stable", "balanced", "aggressive"};
    size_t i;
    if (!threshold) {
        printf("tracking random audit threshold: samples unregistered result SKIP\n");
        return;
    }
    for (i = 0; i < 3; ++i) {
        const tracking_audit_bucket_t* bucket = &stats->by_strategy[i];
        const double average_improvement = tracking_audit_average_improvement(bucket);
        const bool optimized_pass = bucket->optimized >= threshold->required_optimized[i];
        const bool average_pass = average_improvement + 1e-12 >= threshold->required_average_improvement[i];
        printf(
            "tracking random audit threshold strategy %s: baseline_optimized %zu optimized %zu required_optimized %zu baseline_average_improvement %.12g average_improvement %.12g required_average_improvement %.12g result %s\n",
            strategy_names[i],
            threshold->baseline_optimized[i],
            bucket->optimized,
            threshold->required_optimized[i],
            threshold->baseline_average_improvement[i],
            average_improvement,
            threshold->required_average_improvement[i],
            optimized_pass && average_pass ? "PASS" : "FAIL"
        );
        CHECK_TRUE(optimized_pass);
        CHECK_TRUE(average_pass);
    }
}

static void tracking_audit_print_stats(
    const tracking_audit_stats_t* stats,
    const tracking_audit_representatives_t* representatives,
    size_t samples,
    unsigned seed,
    const tracking_audit_threshold_t* threshold
) {
    static const char* strategy_names[3] = {"stable", "balanced", "aggressive"};
    static const char* dof_names[4] = {"1", "2", "4", "8"};
    static const char* signal_names[4] = {"ramp", "constant_acceleration", "sinus", "half_sinus"};
    static const char* lookahead_names[4] = {"1", "2", "5", "10"};
    static const char* reactiveness_names[4] = {"0", "0.25", "0.5", "1"};
    static const char* disabled_names[2] = {"enabled_only", "has_disabled_dof"};
    static const char* constraint_names[2] = {"default", "tight_valid"};
    size_t i;

    printf("tracking random audit: samples %zu seed %u\n", samples, seed);
    tracking_audit_print_bucket("overall", "all", &stats->overall);
    for (i = 0; i < 3; ++i) {
        tracking_audit_print_bucket("by_strategy", strategy_names[i], &stats->by_strategy[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_dof", dof_names[i], &stats->by_dof[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_signal", signal_names[i], &stats->by_signal[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_lookahead", lookahead_names[i], &stats->by_lookahead[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_reactiveness", reactiveness_names[i], &stats->by_reactiveness[i]);
    }
    for (i = 0; i < 2; ++i) {
        tracking_audit_print_bucket("by_disabled", disabled_names[i], &stats->by_disabled[i]);
    }
    for (i = 0; i < 2; ++i) {
        tracking_audit_print_bucket("by_constraints", constraint_names[i], &stats->by_constraints[i]);
    }
    tracking_audit_print_family_summary(&stats->overall);
    tracking_audit_check_thresholds(stats, threshold);
    for (i = 0; i < representatives->count; ++i) {
        tracking_audit_print_case(representatives->reasons[i], &representatives->cases[i], seed);
    }
}

void run_tracking_random_tests(size_t samples, unsigned seed) {
    size_t sample;
    size_t optimized_count = 0;
    size_t fallback_count = 0;
    size_t candidate_count = 0;
    unsigned state = seed ? seed : 1u;

    for (sample = 0; sample < samples; ++sample) {
        const int sample_failures_before = ruckig_c_test_failures;
        tracking_random_case_config_t config = make_tracking_random_case_config(&state, sample);
        tracking_random_case_result_t case_result;
        run_tracking_random_case(&config, &case_result);
        candidate_count += case_result.candidate_count;
        if (case_result.status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
            ++optimized_count;
        } else if (case_result.status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
            ++fallback_count;
        }
        if (ruckig_c_test_failures != sample_failures_before) {
            fprintf(
                stderr,
                "tracking random stress failure_context seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s start_time=%.9g\n",
                seed,
                sample,
                config.dofs,
                tracking_signal_name(config.signal),
                config.lookahead_count,
                config.reactiveness,
                tracking_strategy_name(config.strategy),
                config.has_disabled_dof ? "yes" : "no",
                config.has_disabled_dof ? tracking_dof_number_name(config.disabled_dof) : "none",
                config.start_time
            );
        }
    }

    printf(
        "tracking random stress: samples %zu seed %u optimized %zu fallback %zu candidates %zu\n",
        samples,
        seed,
        optimized_count,
        fallback_count,
        candidate_count
    );
}

void run_tracking_random_audit_tests(size_t samples, unsigned seed) {
    tracking_audit_stats_t stats;
    tracking_audit_representatives_t representatives;
    size_t sample;
    unsigned state = seed ? seed : 1u;

    memset(&stats, 0, sizeof(stats));
    memset(&representatives, 0, sizeof(representatives));

    for (sample = 0; sample < samples; ++sample) {
        tracking_audit_case_config_t config;
        tracking_audit_case_result_t case_result;
        make_tracking_audit_case_config(&state, sample, &config);
        run_tracking_audit_case(&config, &case_result);
        tracking_audit_add_stats(&stats, &case_result);
        tracking_audit_maybe_record_fallback(&representatives, &case_result);
    }

    tracking_audit_print_stats(
        &stats,
        &representatives,
        samples,
        seed,
        tracking_audit_find_threshold(samples, seed)
    );
}

void run_tracking_random_replay_tests(size_t sample_index, unsigned seed) {
    tracking_random_case_config_t config;
    tracking_random_case_result_t case_result;
    size_t sample = 0;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        config = make_tracking_random_case_config(&state, sample);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    run_tracking_random_case(&config, &case_result);
    tracking_random_print_fixture(&config, &case_result, seed);
}

void run_tracking_random_audit_replay_tests(size_t sample_index, unsigned seed) {
    tracking_audit_case_config_t config;
    tracking_audit_case_result_t case_result;
    size_t sample = 0;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        make_tracking_audit_case_config(&state, sample, &config);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    run_tracking_audit_case(&config, &case_result);
    tracking_audit_print_fixture(&config, &case_result, seed);
}

static bool tracking_audit_case_passes(
    const tracking_audit_case_config_t* config,
    tracking_audit_case_result_t* case_result
) {
    const int failures_before = ruckig_c_test_failures;
    ruckig_c_test_failures = 0;
    run_tracking_audit_case(config, case_result);
    {
        const bool passed = ruckig_c_test_failures == 0;
        ruckig_c_test_failures = failures_before;
        return passed;
    }
}

static bool try_tracking_audit_shrink_candidate(
    tracking_audit_case_config_t* current,
    const tracking_audit_case_config_t* candidate,
    const char* label,
    size_t* accepted_count
) {
    tracking_audit_case_result_t case_result;
    if (tracking_audit_case_passes(candidate, &case_result)) {
        *current = *candidate;
        ++*accepted_count;
        printf("tracking random audit shrink accepted %s\n", label);
        return true;
    }
    return false;
}

typedef struct tracking_audit_failure_signature {
    bool failed;
    int failure_count;
    const char* failure_class;
    tracking_audit_case_result_t case_result;
} tracking_audit_failure_signature_t;

static const char* tracking_audit_failure_class(
    const tracking_audit_case_config_t* config,
    const tracking_audit_case_result_t* case_result,
    int failure_count
) {
    if (failure_count == 0) {
        return "none";
    }
    if (case_result->result != RUCKIG_WORKING && case_result->result != RUCKIG_FINISHED) {
        return "result";
    }
    if (!tracking_optimized_status_is_success(case_result->status)) {
        return "calculation-status";
    }
    if (case_result->diagnostics.mode != RUCKIG_TRACKING_OPTIMIZED) {
        return "diagnostics-mode";
    }
    if (case_result->diagnostics.optimized_strategy != config->strategy) {
        return "diagnostics-strategy";
    }
    if (case_result->diagnostics.error_step_count != 0) {
        return "error-step";
    }
    if (case_result->diagnostics.candidate_count == 0) {
        return "candidate-count";
    }
    if (case_result->diagnostics.valid_candidate_count > case_result->diagnostics.candidate_count) {
        return "candidate-accounting";
    }
    return "tracking-audit-invariant";
}

static void tracking_audit_case_failure_signature(
    const tracking_audit_case_config_t* config,
    tracking_audit_failure_signature_t* signature
) {
    const int failures_before = ruckig_c_test_failures;
    memset(signature, 0, sizeof(*signature));
    ruckig_c_test_failures = 0;
    run_tracking_audit_case(config, &signature->case_result);
    signature->failure_count = ruckig_c_test_failures;
    signature->failed = signature->failure_count != 0;
    signature->failure_class = tracking_audit_failure_class(config, &signature->case_result, signature->failure_count);
    ruckig_c_test_failures = failures_before;
}

static bool try_tracking_audit_failure_shrink_candidate(
    tracking_audit_case_config_t* current,
    const tracking_audit_case_config_t* candidate,
    const char* label,
    const char* expected_failure_class,
    size_t* accepted_count
) {
    tracking_audit_failure_signature_t signature;
    tracking_audit_case_failure_signature(candidate, &signature);
    if (signature.failed && strcmp(signature.failure_class, expected_failure_class) == 0) {
        *current = *candidate;
        ++*accepted_count;
        printf(
            "tracking random audit failure shrink accepted %s failure_class=\"%s\" failure_count=%d\n",
            label,
            signature.failure_class,
            signature.failure_count
        );
        return true;
    }
    return false;
}

static void shrink_tracking_audit_case(
    tracking_audit_case_config_t* config,
    size_t* accepted_count
) {
    size_t i;
    static const size_t dof_targets[] = {1, 2, 4};
    static const size_t lookahead_targets[] = {1, 2, 5};
    static const double reactiveness_targets[] = {0.0, 0.25, 0.5};

    for (i = 0; i < sizeof(dof_targets) / sizeof(dof_targets[0]); ++i) {
        if (dof_targets[i] < config->dofs) {
            tracking_audit_case_config_t candidate = *config;
            candidate.dofs = dof_targets[i];
            if (candidate.has_disabled_dof && candidate.disabled_dof >= candidate.dofs) {
                candidate.has_disabled_dof = false;
                candidate.disabled_dof = 0;
            }
            if (try_tracking_audit_shrink_candidate(config, &candidate, "dofs", accepted_count)) {
                break;
            }
        }
    }

    for (i = 0; i < sizeof(lookahead_targets) / sizeof(lookahead_targets[0]); ++i) {
        if (lookahead_targets[i] < config->lookahead_count) {
            tracking_audit_case_config_t candidate = *config;
            candidate.lookahead_count = lookahead_targets[i];
            if (try_tracking_audit_shrink_candidate(config, &candidate, "lookahead-count", accepted_count)) {
                break;
            }
        }
    }

    if (config->has_disabled_dof) {
        tracking_audit_case_config_t candidate = *config;
        candidate.has_disabled_dof = false;
        candidate.disabled_dof = 0;
        try_tracking_audit_shrink_candidate(config, &candidate, "disabled-dof-mask", accepted_count);
    }

    if (config->tight_constraints) {
        tracking_audit_case_config_t candidate = *config;
        candidate.tight_constraints = false;
        try_tracking_audit_shrink_candidate(config, &candidate, "tight-constraints", accepted_count);
    }

    if (config->strategy != RUCKIG_TRACKING_OPTIMIZED_STABLE) {
        tracking_audit_case_config_t candidate = *config;
        candidate.strategy = RUCKIG_TRACKING_OPTIMIZED_STABLE;
        try_tracking_audit_shrink_candidate(config, &candidate, "strategy-stable", accepted_count);
    }

    if (config->signal != 0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.signal = 0;
        try_tracking_audit_shrink_candidate(config, &candidate, "signal-ramp", accepted_count);
    }

    for (i = 0; i < sizeof(reactiveness_targets) / sizeof(reactiveness_targets[0]); ++i) {
        if (reactiveness_targets[i] < config->reactiveness) {
            tracking_audit_case_config_t candidate = *config;
            candidate.reactiveness = reactiveness_targets[i];
            if (try_tracking_audit_shrink_candidate(config, &candidate, "reactiveness", accepted_count)) {
                break;
            }
        }
    }

    if (config->start_time != 0.0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.start_time = 0.0;
        try_tracking_audit_shrink_candidate(config, &candidate, "start-time", accepted_count);
    }
}

static void shrink_tracking_audit_failure_case(
    tracking_audit_case_config_t* config,
    const char* expected_failure_class,
    size_t* accepted_count
) {
    size_t i;
    static const size_t dof_targets[] = {1, 2, 4};
    static const size_t lookahead_targets[] = {1, 2, 5};
    static const double reactiveness_targets[] = {0.0, 0.25, 0.5};

    for (i = 0; i < sizeof(dof_targets) / sizeof(dof_targets[0]); ++i) {
        if (dof_targets[i] < config->dofs) {
            tracking_audit_case_config_t candidate = *config;
            candidate.dofs = dof_targets[i];
            if (candidate.has_disabled_dof && candidate.disabled_dof >= candidate.dofs) {
                candidate.has_disabled_dof = false;
                candidate.disabled_dof = 0;
            }
            if (try_tracking_audit_failure_shrink_candidate(
                    config,
                    &candidate,
                    "dofs",
                    expected_failure_class,
                    accepted_count)) {
                break;
            }
        }
    }

    for (i = 0; i < sizeof(lookahead_targets) / sizeof(lookahead_targets[0]); ++i) {
        if (lookahead_targets[i] < config->lookahead_count) {
            tracking_audit_case_config_t candidate = *config;
            candidate.lookahead_count = lookahead_targets[i];
            if (try_tracking_audit_failure_shrink_candidate(
                    config,
                    &candidate,
                    "lookahead-count",
                    expected_failure_class,
                    accepted_count)) {
                break;
            }
        }
    }

    if (config->has_disabled_dof) {
        tracking_audit_case_config_t candidate = *config;
        candidate.has_disabled_dof = false;
        candidate.disabled_dof = 0;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "disabled-dof-mask",
            expected_failure_class,
            accepted_count
        );
    }

    if (config->tight_constraints) {
        tracking_audit_case_config_t candidate = *config;
        candidate.tight_constraints = false;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "tight-constraints",
            expected_failure_class,
            accepted_count
        );
    }

    if (config->strategy != RUCKIG_TRACKING_OPTIMIZED_STABLE) {
        tracking_audit_case_config_t candidate = *config;
        candidate.strategy = RUCKIG_TRACKING_OPTIMIZED_STABLE;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "strategy-stable",
            expected_failure_class,
            accepted_count
        );
    }

    if (config->signal != 0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.signal = 0;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "signal-ramp",
            expected_failure_class,
            accepted_count
        );
    }

    for (i = 0; i < sizeof(reactiveness_targets) / sizeof(reactiveness_targets[0]); ++i) {
        if (reactiveness_targets[i] < config->reactiveness) {
            tracking_audit_case_config_t candidate = *config;
            candidate.reactiveness = reactiveness_targets[i];
            if (try_tracking_audit_failure_shrink_candidate(
                    config,
                    &candidate,
                    "reactiveness",
                    expected_failure_class,
                    accepted_count)) {
                break;
            }
        }
    }

    if (config->start_time != 0.0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.start_time = 0.0;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "start-time",
            expected_failure_class,
            accepted_count
        );
    }
}

void run_tracking_random_audit_shrink_tests(size_t sample_index, unsigned seed) {
    tracking_audit_case_config_t original;
    tracking_audit_case_config_t reduced;
    tracking_audit_case_result_t original_result;
    tracking_audit_case_result_t reduced_result;
    size_t sample = 0;
    size_t accepted_count = 0;
    const int failures_before = ruckig_c_test_failures;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        make_tracking_audit_case_config(&state, sample, &original);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    run_tracking_audit_case(&original, &original_result);
    if (ruckig_c_test_failures != failures_before) {
        fprintf(
            stderr,
            "tracking random audit shrink original failed seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g\n",
            seed,
            sample_index,
            original.dofs,
            tracking_signal_name(original.signal),
            original.lookahead_count,
            original.reactiveness,
            tracking_strategy_name(original.strategy),
            original.has_disabled_dof ? "yes" : "no",
            original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
            original.tight_constraints ? "tight_valid" : "default",
            original.start_time
        );
        return;
    }

    reduced = original;
    shrink_tracking_audit_case(&reduced, &accepted_count);
    run_tracking_audit_case(&reduced, &reduced_result);

    printf(
        "tracking random audit shrink original seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g\n",
        seed,
        sample_index,
        original.dofs,
        tracking_signal_name(original.signal),
        original.lookahead_count,
        original.reactiveness,
        tracking_strategy_name(original.strategy),
        original.has_disabled_dof ? "yes" : "no",
        original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
        original.tight_constraints ? "tight_valid" : "default",
        original.start_time
    );
    printf(
        "tracking random audit shrink reduced seed=%u sample=%zu accepted=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g\n",
        seed,
        sample_index,
        accepted_count,
        reduced.dofs,
        tracking_signal_name(reduced.signal),
        reduced.lookahead_count,
        reduced.reactiveness,
        tracking_strategy_name(reduced.strategy),
        reduced.has_disabled_dof ? "yes" : "no",
        reduced.has_disabled_dof ? tracking_dof_number_name(reduced.disabled_dof) : "none",
        reduced.tight_constraints ? "tight_valid" : "default",
        reduced.start_time
    );
    printf(
        "tracking random audit shrink replay command: ruckig_c_tests.exe --tracking-random-audit-replay %zu --seed %u\n",
        sample_index,
        seed
    );
    tracking_audit_print_fixture(&reduced, &reduced_result, seed);
}

void run_tracking_random_audit_failure_shrink_tests(size_t sample_index, unsigned seed) {
    tracking_audit_case_config_t original;
    tracking_audit_case_config_t reduced;
    tracking_audit_failure_signature_t original_signature;
    tracking_audit_failure_signature_t reduced_signature;
    size_t sample = 0;
    size_t accepted_count = 0;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        make_tracking_audit_case_config(&state, sample, &original);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    tracking_audit_case_failure_signature(&original, &original_signature);
    if (!original_signature.failed) {
        fprintf(
            stderr,
            "tracking random audit failure shrink original case did not fail seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g; use --tracking-random-audit-shrink for pass-preserving reduction\n",
            seed,
            sample_index,
            original.dofs,
            tracking_signal_name(original.signal),
            original.lookahead_count,
            original.reactiveness,
            tracking_strategy_name(original.strategy),
            original.has_disabled_dof ? "yes" : "no",
            original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
            original.tight_constraints ? "tight_valid" : "default",
            original.start_time
        );
        fprintf(
            stderr,
            "tracking random audit failure shrink replay command: ruckig_c_tests.exe --tracking-random-audit-replay %zu --seed %u\n",
            sample_index,
            seed
        );
        ++ruckig_c_test_failures;
        return;
    }

    reduced = original;
    shrink_tracking_audit_failure_case(&reduced, original_signature.failure_class, &accepted_count);
    tracking_audit_case_failure_signature(&reduced, &reduced_signature);
    if (!reduced_signature.failed || strcmp(reduced_signature.failure_class, original_signature.failure_class) != 0) {
        fprintf(
            stderr,
            "tracking random audit failure shrink reduced case no longer preserves failure class seed=%u sample=%zu original_class=\"%s\" reduced_class=\"%s\"\n",
            seed,
            sample_index,
            original_signature.failure_class,
            reduced_signature.failure_class
        );
        ++ruckig_c_test_failures;
        return;
    }

    printf(
        "tracking random audit failure shrink original seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g failure_class=\"%s\" failure_count=%d\n",
        seed,
        sample_index,
        original.dofs,
        tracking_signal_name(original.signal),
        original.lookahead_count,
        original.reactiveness,
        tracking_strategy_name(original.strategy),
        original.has_disabled_dof ? "yes" : "no",
        original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
        original.tight_constraints ? "tight_valid" : "default",
        original.start_time,
        original_signature.failure_class,
        original_signature.failure_count
    );
    printf(
        "tracking random audit failure shrink reduced seed=%u sample=%zu accepted=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g failure_class=\"%s\" failure_count=%d\n",
        seed,
        sample_index,
        accepted_count,
        reduced.dofs,
        tracking_signal_name(reduced.signal),
        reduced.lookahead_count,
        reduced.reactiveness,
        tracking_strategy_name(reduced.strategy),
        reduced.has_disabled_dof ? "yes" : "no",
        reduced.has_disabled_dof ? tracking_dof_number_name(reduced.disabled_dof) : "none",
        reduced.tight_constraints ? "tight_valid" : "default",
        reduced.start_time,
        reduced_signature.failure_class,
        reduced_signature.failure_count
    );
    printf(
        "tracking random audit failure shrink original replay command: ruckig_c_tests.exe --tracking-random-audit-replay %zu --seed %u\n",
        sample_index,
        seed
    );
    printf("tracking random audit failure shrink reduced replay: paste the initializer below into the fixed tracking audit corpus\n");
    tracking_audit_print_fixture(&reduced, &reduced_signature.case_result, seed);
}

static void test_tracking_random_audit_fixed_cases(void) {
    const tracking_audit_case_config_t cases[] = {
        {6, 2, 1, 1, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 0, true, 0.06},
        {12, 2, 5, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, true, 1, true, 0.12},
        {22, 8, 5, 0, 0.25, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 7, true, 0.22},
        {2600, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, false, 0.0},
        {8011, 4, 10, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 3, true, 0.11},
        {9800, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.0},
        {1614, 8, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, false, 0.14},
        {0, 8, 5, 0, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 6, true, 0.0},
        {4, 4, 2, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, true, 0.04},
        {8, 1, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, true, 0.08},
        {10, 1, 10, 1, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, true, 0.10},
        {16, 1, 5, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.16}
    };
    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        tracking_audit_case_result_t case_result;
        size_t selected = 0;
        size_t family;
        run_tracking_audit_case(&cases[i], &case_result);
        CHECK_TRUE(tracking_optimized_status_is_success(case_result.status));
        CHECK_EQ_INT(case_result.diagnostics.fallback_step_count + case_result.diagnostics.optimized_step_count, 1);
        CHECK_EQ_INT(case_result.diagnostics.error_step_count, 0);
        CHECK_TRUE(case_result.diagnostics.candidate_count >= 1);
        CHECK_TRUE(case_result.selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT);
        for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
            selected += case_result.family_selected[family];
        }
        CHECK_EQ_INT(selected, 1);
        if (case_result.selected_near_tie) {
            CHECK_EQ_INT(case_result.config.strategy, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE);
            CHECK_TRUE(case_result.near_tie_accepted_count > 0);
        }
    }
}

static void test_tracking_stability_regression_cases(void) {
    const tracking_stability_case_t cases[] = {
        {
            "stable tight disabled strict",
            {6, 2, 1, 1, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 0, true, 0.06},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            false,
            true
        },
        {
            "balanced tight disabled strict",
            {12, 2, 5, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, true, 1, true, 0.12},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "aggressive tight disabled strict",
            {22, 8, 5, 0, 0.25, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 7, true, 0.22},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "stable fallback sinus",
            {2600, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, false, 0.0},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "stable disabled tight fallback",
            {8011, 4, 10, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 3, true, 0.11},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "balanced fallback sinus",
            {9800, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.0},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "aggressive fallback ramp",
            {1614, 8, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, false, 0.14},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "aggressive disabled tight strict",
            {0, 8, 5, 0, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 6, true, 0.0},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "balanced tight strict",
            {4, 4, 2, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, true, 0.04},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            false,
            true
        },
        {
            "stable tight strict",
            {8, 1, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, true, 0.08},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "aggressive strict budget",
            {10, 1, 10, 1, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, true, 0.10},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "balanced half sinus strict",
            {16, 1, 5, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.16},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "seed1 derivative damped representative",
            {1, 4, 1, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, true, 0.01},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            4,
            false,
            false,
            true
        },
        {
            "seed1 horizon representative",
            {8, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.08},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            2,
            false,
            true,
            true
        },
        {
            "seed1 lead lag representative",
            {403, 2, 1, 3, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 1, false, 0.03},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            5,
            false,
            false,
            true
        },
        {
            "seed1 terminal blend representative",
            {602, 2, 5, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_BALANCED, true, 1, false, 0.02},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            3,
            false,
            true,
            true
        },
        {
            "seed1 aggressive near tie representative",
            {1400, 2, 5, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 1, false, 0.0},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            true,
            true,
            true
        }
    };
    bool strategy_seen[3] = {false, false, false};
    bool signal_seen[4] = {false, false, false, false};
    bool lookahead_seen[4] = {false, false, false, false};
    bool family_seen[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT] = {false, false, false, false, false, false};
    bool optimized_seen = false;
    bool fallback_seen = false;
    bool near_tie_seen = false;
    bool disabled_seen = false;
    bool tight_seen = false;
    bool budget_seen = false;
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        tracking_audit_case_result_t case_result;
        size_t selected = 0;
        size_t family;
        run_tracking_audit_case(&cases[i].config, &case_result);
        printf(
            "tracking stability case %s: status=%s selected_family=%s selected_near_tie=%s candidates=%zu budget_exhausted=%zu improvement=%.9g\n",
            cases[i].name,
            tracking_status_name(case_result.status),
            case_result.selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT
                ? tracking_audit_family_names[case_result.selected_family]
                : "unknown",
            case_result.selected_near_tie ? "yes" : "no",
            case_result.diagnostics.candidate_count,
            case_result.diagnostics.budget_exhausted_count,
            case_result.diagnostics.improvement_ratio
        );
        CHECK_EQ_INT(case_result.status, cases[i].expected_status);
        CHECK_EQ_INT(case_result.selected_family, cases[i].expected_family);
        CHECK_EQ_INT(case_result.selected_near_tie, cases[i].expected_near_tie);
        CHECK_EQ_INT(case_result.config.strategy, cases[i].config.strategy);
        CHECK_EQ_INT(case_result.config.has_disabled_dof, cases[i].config.has_disabled_dof);
        CHECK_EQ_INT(case_result.config.tight_constraints, cases[i].config.tight_constraints);
        CHECK_EQ_INT(case_result.diagnostics.fast_candidate_count, 1);
        CHECK_TRUE(case_result.diagnostics.candidate_count >= 1);
        CHECK_TRUE(case_result.diagnostics.valid_candidate_count >= 1);
        CHECK_EQ_INT(case_result.diagnostics.rejected_candidate_count, 0);
        for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
            selected += case_result.family_selected[family];
        }
        CHECK_EQ_INT(selected, 1);
        if (cases[i].expected_status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
            CHECK_EQ_INT(case_result.diagnostics.optimized_step_count, 1);
            CHECK_EQ_INT(case_result.diagnostics.fallback_step_count, 0);
            optimized_seen = true;
        } else {
            CHECK_EQ_INT(case_result.diagnostics.optimized_step_count, 0);
            CHECK_EQ_INT(case_result.diagnostics.fallback_step_count, 1);
            fallback_seen = true;
        }
        if (cases[i].expected_near_tie) {
            CHECK_EQ_INT(case_result.config.strategy, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE);
            CHECK_TRUE(case_result.near_tie_accepted_count > 0);
            near_tie_seen = true;
        } else {
            CHECK_EQ_INT(case_result.selected_near_tie, false);
        }
        if (cases[i].expect_budget_exhausted) {
            CHECK_TRUE(case_result.diagnostics.budget_exhausted_count > 0);
            budget_seen = true;
        } else {
            CHECK_EQ_INT(case_result.diagnostics.budget_exhausted_count, 0);
        }
        if (cases[i].expect_positive_improvement) {
            CHECK_TRUE(case_result.diagnostics.improvement_ratio > 0.0);
        }
        strategy_seen[tracking_strategy_index(case_result.config.strategy)] = true;
        signal_seen[(size_t)case_result.config.signal] = true;
        lookahead_seen[tracking_lookahead_index(case_result.config.lookahead_count)] = true;
        family_seen[case_result.selected_family] = true;
        disabled_seen = disabled_seen || case_result.config.has_disabled_dof;
        tight_seen = tight_seen || case_result.config.tight_constraints;
    }

    CHECK_TRUE(strategy_seen[0] && strategy_seen[1] && strategy_seen[2]);
    CHECK_TRUE(signal_seen[0] && signal_seen[1] && signal_seen[2] && signal_seen[3]);
    CHECK_TRUE(lookahead_seen[0] && lookahead_seen[1] && lookahead_seen[2] && lookahead_seen[3]);
    CHECK_TRUE(
        family_seen[0]
        && family_seen[1]
        && family_seen[2]
        && family_seen[3]
        && family_seen[4]
        && family_seen[5]
    );
    CHECK_TRUE(optimized_seen);
    CHECK_TRUE(fallback_seen);
    CHECK_TRUE(near_tie_seen);
    CHECK_TRUE(disabled_seen);
    CHECK_TRUE(tight_seen);
    CHECK_TRUE(budget_seen);
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
    test_tracking_optimized_quality_against_fast_baseline();
    test_tracking_optimized_strategy_quality_corpus();
    test_tracking_random_audit_fixed_cases();
    test_tracking_stability_regression_cases();
}

void run_tracking_quality_tests(void) {
    test_tracking_quality_against_instantaneous_chasing();
    test_tracking_optimized_quality_against_fast_baseline();
    test_tracking_optimized_strategy_quality_corpus();
}

void run_tracking_quality_hardening_tests(void) {
    test_tracking_random_audit_fixed_cases();
    run_tracking_random_audit_tests(10000, 1u);
}

void run_tracking_stability_tests(void) {
    test_tracking_stability_regression_cases();
}

void run_tracking_no_allocation_tests(void) {
    test_tracking_no_allocation();
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
