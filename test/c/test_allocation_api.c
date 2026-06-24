#include "test_api_internal.h"

static void test_no_allocation_in_realtime_paths(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_control_interface_t per_control[1] = {RUCKIG_CONTROL_POSITION};
    ruckig_synchronization_t per_sync[1] = {RUCKIG_SYNCHRONIZATION_TIME};
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    size_t allocations_before;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, per_sync, 1), RUCKIG_WORKING);

    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 0.5, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
#ifdef RUCKIG_C_ENABLE_CALCULATION_DURATION
    CHECK_TRUE(ruckig_output_get_calculation_duration(output) >= 0.0);
#else
    CHECK_NEAR(ruckig_output_get_calculation_duration(output), 0.0, 0.0);
#endif
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
#ifdef RUCKIG_C_ENABLE_CALCULATION_DURATION
    CHECK_TRUE(ruckig_output_get_calculation_duration(output) >= 0.0);
#else
    CHECK_NEAR(ruckig_output_get_calculation_duration(output), 0.0, 0.0);
#endif
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_allocation_forbidden_set(false);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_allocation_audit_counter_sequence(void) {
    void* first = NULL;
    void* second = NULL;

    ruckig_allocation_counters_reset();
    CHECK_EQ_INT(ruckig_allocation_count(), 0);
    CHECK_EQ_INT(ruckig_free_count(), 0);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);

    ruckig_allocation_forbidden_set(true);
    first = ruckig_calloc(1, sizeof(double));
    CHECK_TRUE(first != NULL);
    CHECK_EQ_INT(ruckig_allocation_count(), 1);
    CHECK_EQ_INT(ruckig_free_count(), 0);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 1);

    ruckig_free(first);
    CHECK_EQ_INT(ruckig_allocation_count(), 1);
    CHECK_EQ_INT(ruckig_free_count(), 1);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 1);

    ruckig_allocation_forbidden_set(false);
    second = ruckig_calloc(1, sizeof(double));
    CHECK_TRUE(second != NULL);
    CHECK_EQ_INT(ruckig_allocation_count(), 2);
    CHECK_EQ_INT(ruckig_free_count(), 1);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 1);

    ruckig_free(second);
    CHECK_EQ_INT(ruckig_allocation_count(), 2);
    CHECK_EQ_INT(ruckig_free_count(), 2);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 1);

    ruckig_allocation_counters_reset();
    CHECK_EQ_INT(ruckig_allocation_count(), 0);
    CHECK_EQ_INT(ruckig_free_count(), 0);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
}


void run_allocation_api_tests(void) {
    test_no_allocation_in_realtime_paths();
    test_allocation_audit_counter_sequence();
}
