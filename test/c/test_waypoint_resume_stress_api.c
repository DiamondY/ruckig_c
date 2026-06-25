#include "test_api_internal.h"

static void test_waypoint_resume_stress_budget_matrix(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double incumbent_remaining_duration = 0.0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        CHECK_TRUE(otg->waypoint_engine.active);
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_TRUE(otg->waypoint_engine.last_candidate_evaluations >= 1);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        const double previous_time = 0.0;
        ruckig_result_t result;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);

        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);
        ruckig_output_pass_to_input(output, input);
        ruckig_input_clear_interrupt_calculation_duration(input);
        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_resume_stress_long_online_loop(void) {
    ruckig_t* otg = NULL;
    ruckig_t* fresh_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* fresh_input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* fresh_trajectory = NULL;
    bool saw_publish = false;
    bool saw_fresh_quality_reference = false;
    bool saw_budget_interruption = false;
    bool saw_completion = false;
    size_t cycle;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, 4, 0.01, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, 4, 3), RUCKIG_WORKING);
    configure_alpha1_resume_stress_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(otg->waypoint_engine.active);
    check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

    for (cycle = 0; cycle < 32; ++cycle) {
        const double previous_time = ruckig_output_get_time(output);
        const size_t previous_section = ruckig_output_get_new_section(output);
        const double incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
        const bool was_active_before_update = otg->waypoint_engine.active;
        double fresh_duration = 0.0;
        ruckig_result_t result;

        ruckig_output_pass_to_input(output, input);
        if (cycle == 2) {
            CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
        } else if (cycle == 3) {
            CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        } else if (cycle == 10) {
            CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        }

        if (was_active_before_update) {
            CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
            ruckig_input_clear_interrupt_calculation_duration(fresh_input);
            CHECK_EQ_INT(ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory), RUCKIG_WORKING);
            fresh_duration = ruckig_trajectory_get_duration(fresh_trajectory);
            CHECK_TRUE(fresh_duration > 0.0);
            saw_fresh_quality_reference = true;
        }

        result = ruckig_update(otg, input, output);
        CHECK_EQ_INT(result, RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_get_new_section(output) < 4);
        if (ruckig_output_new_calculation(output)) {
            const double published_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(published_duration < incumbent_remaining_duration - 1e-12);
            CHECK_TRUE(published_duration > 0.0);
            saw_publish = true;
        } else {
            CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
            CHECK_TRUE(ruckig_output_get_new_section(output) >= previous_section);
            if (ruckig_output_was_calculation_interrupted(output)) {
                saw_budget_interruption = true;
            }
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        if (cycle >= 10 && was_active_before_update && !otg->waypoint_engine.active) {
            CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
            saw_completion = true;
            break;
        }
    }

    CHECK_TRUE(saw_fresh_quality_reference);
    CHECK_TRUE(saw_budget_interruption);
    CHECK_TRUE(saw_publish);
    CHECK_TRUE(saw_completion);

    ruckig_trajectory_destroy(fresh_trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(fresh_input);
    ruckig_input_destroy(input);
    ruckig_destroy(fresh_otg);
    ruckig_destroy(otg);
}

static void test_waypoint_resume_stress_allocation_paths(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

        ruckig_allocation_counters_reset();
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        ruckig_output_pass_to_input(output, input);
        otg->waypoint_engine.best_duration = -1.0;
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        double incumbent_remaining_duration = 0.0;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 4, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 4, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 4, 3), RUCKIG_WORKING);
        configure_alpha1_resume_stress_input(input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(otg->waypoint_engine.active);

        incumbent_remaining_duration =
            ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - ruckig_output_get_time(output);
        ruckig_output_pass_to_input(output, input);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
        ruckig_allocation_counters_reset();
        CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(!otg->waypoint_engine.active);
        if (ruckig_output_new_calculation(output)) {
            CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                < incumbent_remaining_duration - 1e-12);
        }
        check_alpha1_resume_stress_trajectory(ruckig_output_get_trajectory(output));

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

void run_waypoint_resume_stress_tests(void) {
    test_waypoint_resume_stress_budget_matrix();
    test_waypoint_resume_stress_long_online_loop();
    test_waypoint_resume_stress_allocation_paths();
}
