#include "test_api_internal.h"

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

void run_tracking_optimized_quality_regression_tests(void) {
    test_tracking_optimized_quality_against_fast_baseline();
    test_tracking_optimized_strategy_quality_corpus();
}

void run_tracking_quality_tests(void) {
    test_tracking_quality_against_instantaneous_chasing();
    run_tracking_optimized_quality_regression_tests();
}
