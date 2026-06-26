#include "test_api_internal.h"

static void check_alpha2_resume_trajectory(const ruckig_trajectory_t* trajectory) {
    const size_t dofs = 3;
    double waypoints[6] = {
        0.35, -0.12, 0.20,
        0.82, -0.35, 0.48
    };
    double per_section_min_velocity[9] = {
        -0.75, -0.70, -0.65,
        -0.85, -0.80, -0.75,
        -0.95, -0.90, -0.85
    };
    double per_section_max_velocity[9] = {
        0.85, 0.80, 0.75,
        0.95, 0.90, 0.85,
        1.05, 1.00, 0.95
    };
    double per_section_min_acceleration[9] = {
        -1.4, -1.3, -1.2,
        -1.5, -1.4, -1.3,
        -1.6, -1.5, -1.4
    };
    double per_section_max_acceleration[9] = {
        1.4, 1.3, 1.2,
        1.5, 1.4, 1.3,
        1.6, 1.5, 1.4
    };
    double per_section_max_jerk[9] = {
        3.6, 3.4, 3.2,
        3.8, 3.6, 3.4,
        4.0, 3.8, 3.6
    };
    double per_section_min_position[9] = {
        -0.05, -0.18, -0.05,
        0.25, -0.42, 0.12,
        0.72, -0.62, 0.40
    };
    double per_section_max_position[9] = {
        0.42, 0.05, 0.25,
        0.90, -0.05, 0.55,
        1.25, -0.30, 0.82
    };

    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
    check_waypoint_samples(trajectory, waypoints, 2, dofs);
    check_waypoint_section_sampled_limits(
        trajectory,
        per_section_min_velocity,
        per_section_max_velocity,
        per_section_min_acceleration,
        per_section_max_acceleration,
        per_section_max_jerk,
        per_section_min_position,
        per_section_max_position,
        3,
        dofs);
}

static void test_waypoint_soft_interruption_update(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* reference = NULL;
        ruckig_result_t result;
        double reference_duration = 0.0;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&reference, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        ruckig_input_clear_interrupt_calculation_duration(input);
        CHECK_EQ_INT(ruckig_calculate(otg, input, reference), RUCKIG_WORKING);
        reference_duration = ruckig_trajectory_get_duration(reference);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-12);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_reset(otg);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-12);
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_trajectory_destroy(reference);
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        otg->waypoint_engine.best_duration = -1.0;
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        ruckig_input_clear_interrupt_calculation_duration(input);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        ruckig_output_pass_to_input(output, input);
        ruckig_input_current_position_data(input)[0] += 0.01;
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_reset(otg);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        const ruckig_trajectory_t* trajectory = NULL;
        double waypoint[1] = {1.0};
        double position[1] = {0.0};
        double duration = 0.0;
        size_t allocations_before = 0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        ruckig_allocation_counters_reset();
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        result = ruckig_update(otg, input, output);
        ruckig_allocation_forbidden_set(false);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_TRUE(!otg->waypoint_engine.complete);

        trajectory = ruckig_output_get_trajectory(output);
        duration = ruckig_trajectory_get_duration(trajectory);
        CHECK_TRUE(duration > 0.0);
        check_waypoint_samples(trajectory, waypoint, 1, 1);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, duration, position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 2.0, 1e-7);

        ruckig_output_pass_to_input(output, input);
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        result = ruckig_update(otg, input, output);
        ruckig_allocation_forbidden_set(false);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_NEAR(ruckig_output_get_time(output), 0.05, 1e-12);
        trajectory = ruckig_output_get_trajectory(output);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
        check_waypoint_samples(trajectory, waypoint, 1, 1);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double waypoint[1] = {1.0};
        double per_section_max_position[2] = {0.5, 2.5};
        double per_section_min_position[2] = {-0.5, 0.5};
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_jerk_data(input)[0] = 4.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_ERROR_EXECUTION_TIME_CALCULATION);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), 0.0, 0.0);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;

        CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 1.0;
        ruckig_input_max_velocity_data(input)[0] = 1.0;
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[1] = {1.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);
        check_waypoint_samples(trajectory, waypoint, 1, 1);

        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

typedef enum waypoint_alpha2_invalidation_kind {
    WAYPOINT_ALPHA2_INVALIDATE_TARGET = 0,
    WAYPOINT_ALPHA2_INVALIDATE_WAYPOINTS,
    WAYPOINT_ALPHA2_INVALIDATE_WAYPOINT_COUNT,
    WAYPOINT_ALPHA2_INVALIDATE_LIMITS,
    WAYPOINT_ALPHA2_INVALIDATE_PER_SECTION,
    WAYPOINT_ALPHA2_INVALIDATE_ENABLED_DOF,
    WAYPOINT_ALPHA2_INVALIDATE_SYNCHRONIZATION,
    WAYPOINT_ALPHA2_INVALIDATE_CONTROL_INTERFACE,
    WAYPOINT_ALPHA2_INVALIDATE_DURATION_DISCRETIZATION,
    WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT
} waypoint_alpha2_invalidation_kind_t;

static void configure_alpha2_invalidation_input(ruckig_input_t* input) {
    double waypoints[4] = {0.40, 0.0, 0.82, 0.0};
    size_t i;
    for (i = 0; i < 2; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.2;
        ruckig_input_max_acceleration_data(input)[i] = 2.0;
        ruckig_input_max_jerk_data(input)[i] = 4.0;
    }
    ruckig_input_target_position_data(input)[0] = 1.20;
    ruckig_input_target_position_data(input)[1] = 0.0;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 2), RUCKIG_WORKING);
}

static void apply_alpha2_invalidation(ruckig_input_t* input, waypoint_alpha2_invalidation_kind_t kind) {
    switch (kind) {
    case WAYPOINT_ALPHA2_INVALIDATE_TARGET:
        ruckig_input_target_position_data(input)[0] += 0.10;
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_WAYPOINTS: {
        double waypoints[4] = {0.45, 0.0, 0.90, 0.0};
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 2), RUCKIG_WORKING);
        break;
    }
    case WAYPOINT_ALPHA2_INVALIDATE_WAYPOINT_COUNT: {
        double waypoint[2] = {0.55, 0.0};
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 2), RUCKIG_WORKING);
        break;
    }
    case WAYPOINT_ALPHA2_INVALIDATE_LIMITS:
        ruckig_input_max_velocity_data(input)[0] = 1.05;
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_PER_SECTION: {
        double per_section_minimum_duration[3] = {0.08, 0.12, 0.08};
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
        break;
    }
    case WAYPOINT_ALPHA2_INVALIDATE_ENABLED_DOF:
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_SYNCHRONIZATION:
        CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_NONE), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_CONTROL_INTERFACE:
        CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_DURATION_DISCRETIZATION:
        CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
        break;
    case WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT:
        ruckig_input_clear_interrupt_calculation_duration(input);
        break;
    }
}

static void check_alpha2_invalidation_case(
    waypoint_alpha2_invalidation_kind_t kind,
    ruckig_result_t expected_result
) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.02, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 2, 2), RUCKIG_WORKING);
    configure_alpha2_invalidation_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(otg->waypoint_engine.active);
    ruckig_output_pass_to_input(output, input);

    apply_alpha2_invalidation(input, kind);
    result = ruckig_update(otg, input, output);
    CHECK_EQ_INT(result, expected_result);
    if (kind == WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT) {
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
    } else if (expected_result == RUCKIG_WORKING) {
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
    } else {
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_waypoint_soft_interruption_alpha2_hardening(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double reference_duration = 0.0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 3, 2), RUCKIG_WORKING);
        configure_alpha2_resume_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 3);
        reference_duration = ruckig_trajectory_get_duration(trajectory);
        check_alpha2_resume_trajectory(trajectory);

        ruckig_input_clear_interrupt_calculation_duration(input);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_NEAR(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)), reference_duration, 1e-9);
        check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));

        ruckig_trajectory_destroy(trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_t* fresh_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* fresh_input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* fresh_trajectory = NULL;
        double incumbent_remaining_duration;
        double fresh_duration;
        double published_duration;
        size_t allocations_before;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, 3, 2), RUCKIG_WORKING);
        configure_alpha2_resume_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
        check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
        ruckig_input_clear_interrupt_calculation_duration(fresh_input);
        CHECK_EQ_INT(ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory), RUCKIG_WORKING);
        fresh_duration = ruckig_trajectory_get_duration(fresh_trajectory);
        CHECK_TRUE(fresh_duration > 0.0);
        CHECK_TRUE(fresh_duration <= incumbent_remaining_duration + 1e-9);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        ruckig_allocation_counters_reset();
        allocations_before = ruckig_allocation_count();
        ruckig_allocation_forbidden_set(true);
        result = ruckig_update(otg, input, output);
        ruckig_allocation_forbidden_set(false);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
        CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_NEAR(ruckig_output_get_time(output), 0.02, 1e-12);
        published_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
        CHECK_TRUE(published_duration < incumbent_remaining_duration - 1e-12);
        check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));

        ruckig_trajectory_destroy(fresh_trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(fresh_input);
        ruckig_input_destroy(input);
        ruckig_destroy(fresh_otg);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        size_t cycle;
        bool saw_background_publish = false;
        bool saw_interrupted_without_publish = false;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 3, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 3, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 3, 2), RUCKIG_WORKING);
        configure_alpha2_resume_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);

        for (cycle = 0; cycle < 24; ++cycle) {
            const double previous_time = ruckig_output_get_time(output);
            const double incumbent_remaining_duration =
                ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
            ruckig_output_pass_to_input(output, input);
            CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
            CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations <= 1);
            if (ruckig_output_new_calculation(output)) {
                CHECK_NEAR(ruckig_output_get_time(output), 0.02, 1e-12);
                CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                    < incumbent_remaining_duration - 1e-12);
                saw_background_publish = true;
            } else {
                CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
                if (ruckig_output_was_calculation_interrupted(output)) {
                    saw_interrupted_without_publish = true;
                }
            }
            check_alpha2_resume_trajectory(ruckig_output_get_trajectory(output));
        }
        CHECK_TRUE(saw_background_publish);
        CHECK_TRUE(saw_interrupted_without_publish);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double* target_position;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
        configure_soft_interruption_waypoint_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(ruckig_calculate(otg, input, output->trajectory), RUCKIG_WORKING);
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations > 1);

        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, NULL, 0, 1), RUCKIG_WORKING);
        target_position = ruckig_input_target_position_data(input);
        target_position[0] = 2.5;
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_TARGET, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_WAYPOINTS, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_WAYPOINT_COUNT, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_LIMITS, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_PER_SECTION, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_ENABLED_DOF, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_SYNCHRONIZATION, RUCKIG_WORKING);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_CONTROL_INTERFACE, RUCKIG_ERROR_INVALID_INPUT);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_DURATION_DISCRETIZATION, RUCKIG_ERROR_INVALID_INPUT);
    check_alpha2_invalidation_case(WAYPOINT_ALPHA2_INVALIDATE_CLEAR_INTERRUPT, RUCKIG_WORKING);
}

void run_waypoint_interrupt_quality_tests(void) {
    test_waypoint_soft_interruption_update();
    test_waypoint_soft_interruption_alpha2_hardening();
}
