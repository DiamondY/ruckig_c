#include "test_api_internal.h"

static void test_input_defaults_and_accessors(void) {
    ruckig_input_t* input = NULL;
    double* current_velocity;
    double* current_acceleration;
    double* target_velocity;
    double* target_acceleration;
    double* max_acceleration;
    double* max_jerk;
    bool* enabled;
    size_t i;

    CHECK_EQ_INT(ruckig_input_create(&input, 4), RUCKIG_WORKING);

    current_velocity = ruckig_input_current_velocity_data(input);
    current_acceleration = ruckig_input_current_acceleration_data(input);
    target_velocity = ruckig_input_target_velocity_data(input);
    target_acceleration = ruckig_input_target_acceleration_data(input);
    max_acceleration = ruckig_input_max_acceleration_data(input);
    max_jerk = ruckig_input_max_jerk_data(input);
    enabled = ruckig_input_enabled_data(input);

    for (i = 0; i < 4; ++i) {
        CHECK_NEAR(current_velocity[i], 0.0, 0.0);
        CHECK_NEAR(current_acceleration[i], 0.0, 0.0);
        CHECK_NEAR(target_velocity[i], 0.0, 0.0);
        CHECK_NEAR(target_acceleration[i], 0.0, 0.0);
        CHECK_TRUE(isinf(max_acceleration[i]));
        CHECK_TRUE(isinf(max_jerk[i]));
        CHECK_TRUE(enabled[i]);
    }

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_control_interface(input, (ruckig_control_interface_t)99), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_PHASE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_synchronization(input, (ruckig_synchronization_t)99), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, (ruckig_duration_discretization_t)99), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 2, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 4, false), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_TRUE(!ruckig_input_enabled_const_data(input)[2]);

    ruckig_input_destroy(input);
}

static void test_per_dof_setters_and_clear(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_control_interface_t per_control[2] = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY};
    ruckig_control_interface_t invalid_control[2] = {RUCKIG_CONTROL_POSITION, (ruckig_control_interface_t)99};
    ruckig_synchronization_t per_sync[2] = {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE};
    ruckig_synchronization_t invalid_sync[2] = {RUCKIG_SYNCHRONIZATION_TIME, (ruckig_synchronization_t)99};
    double per_duration = 0.0;
    double global_duration = 0.0;

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(NULL, per_control, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, NULL, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, invalid_control, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(NULL, per_sync, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, NULL, 2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, per_sync, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, invalid_sync, 2), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_per_dof_control_interface(NULL);
    ruckig_input_clear_per_dof_synchronization(NULL);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 0.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_velocity_data(input)[1] = 0.8;
    ruckig_input_max_velocity_data(input)[0] = 1.5;
    ruckig_input_max_velocity_data(input)[1] = 0.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[1] = 1.1;
    ruckig_input_max_jerk_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[1] = 1.7;

    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    per_duration = ruckig_trajectory_get_duration(trajectory);
    CHECK_TRUE(per_duration > 0.0);

    ruckig_input_clear_per_dof_control_interface(input);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    ruckig_input_clear_per_dof_control_interface(input);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_input_target_position_data(input)[1] = 3.0;
    ruckig_input_target_velocity_data(input)[1] = 0.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[1] = 1.0;
    ruckig_input_max_jerk_data(input)[1] = 2.0;
    ruckig_reset(otg);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, per_sync, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    per_duration = ruckig_trajectory_get_duration(trajectory);
    ruckig_input_clear_per_dof_synchronization(input);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    global_duration = ruckig_trajectory_get_duration(trajectory);
    CHECK_TRUE(global_duration >= per_duration);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_per_dof_clear_restores_global_sync_behavior(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_synchronization_t per_sync[2] = {RUCKIG_SYNCHRONIZATION_NONE, RUCKIG_SYNCHRONIZATION_TIME};
    double global_duration = 0.0;
    double per_duration = 0.0;
    double restored_duration = 0.0;

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 3.0;
    ruckig_input_max_velocity_data(input)[0] = 1.4;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.1;
    ruckig_input_max_acceleration_data(input)[1] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.5;
    ruckig_input_max_jerk_data(input)[1] = 2.0;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    global_duration = ruckig_trajectory_get_duration(trajectory);

    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, per_sync, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    per_duration = ruckig_trajectory_get_duration(trajectory);
    CHECK_TRUE(per_duration > 0.0);

    ruckig_input_clear_per_dof_synchronization(input);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    restored_duration = ruckig_trajectory_get_duration(trajectory);
    CHECK_NEAR(restored_duration, global_duration, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_per_dof_update_recalculation_stability(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_control_interface_t per_control[2] = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY};
    ruckig_synchronization_t per_sync[2] = {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_dof_synchronization(input, per_sync, 2), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[1] = 0.8;
    ruckig_input_max_velocity_data(input)[0] = 1.5;
    ruckig_input_max_acceleration_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[1] = 1.1;
    ruckig_input_max_jerk_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[1] = 1.7;

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    ruckig_output_pass_to_input(output, input);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    ruckig_output_pass_to_input(output, input);

    ruckig_input_target_position_data(input)[0] = 1.4;
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_optional_setters_and_pass_to_input(void) {
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    double min_velocity[2] = {-2.0, -3.0};
    double min_acceleration[2] = {-4.0, -5.0};
    double* target_position;
    double* max_velocity;

    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, min_velocity, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, min_velocity, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, min_velocity, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, NULL, 2), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_min_velocity(input);

    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, min_acceleration, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, min_acceleration, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, min_acceleration, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, NULL, 2), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_min_acceleration(input);

    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 1.25), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, -1.0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, NAN), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_minimum_duration(input);

    target_position = ruckig_input_target_position_data(input);
    max_velocity = ruckig_input_max_velocity_data(input);
    target_position[0] = 10.0;
    max_velocity[0] = 7.0;

    ((double*)ruckig_output_new_position_data(output))[0] = 1.0;
    ((double*)ruckig_output_new_velocity_data(output))[0] = 2.0;
    ((double*)ruckig_output_new_acceleration_data(output))[0] = 3.0;
    ruckig_output_pass_to_input(output, input);
    CHECK_NEAR(ruckig_input_current_position_const_data(input)[0], 1.0, 0.0);
    CHECK_NEAR(ruckig_input_current_velocity_const_data(input)[0], 2.0, 0.0);
    CHECK_NEAR(ruckig_input_current_acceleration_const_data(input)[0], 3.0, 0.0);
    CHECK_NEAR(ruckig_input_target_position_const_data(input)[0], 10.0, 0.0);
    CHECK_NEAR(ruckig_input_max_velocity_const_data(input)[0], 7.0, 0.0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
}

static void test_dof_mismatch_and_invalid_discrete_duration(void) {
    ruckig_t* otg = NULL;
    ruckig_t* zero_delta_otg = NULL;
    ruckig_input_t* input1 = NULL;
    ruckig_input_t* input2 = NULL;
    ruckig_output_t* output1 = NULL;
    ruckig_output_t* output2 = NULL;
    ruckig_trajectory_t* trajectory1 = NULL;
    ruckig_trajectory_t* trajectory2 = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create(&zero_delta_otg, 1, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input2, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output2, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory2, 2), RUCKIG_WORKING);

    ruckig_input_target_position_data(input1)[0] = 1.0;
    ruckig_input_max_velocity_data(input1)[0] = 1.0;
    ruckig_input_target_position_data(input2)[0] = 1.0;
    ruckig_input_target_position_data(input2)[1] = 1.0;
    ruckig_input_max_velocity_data(input2)[0] = 1.0;
    ruckig_input_max_velocity_data(input2)[1] = 1.0;

    CHECK_EQ_INT(ruckig_validate_input(otg, input2, false, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(otg, input2, trajectory2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(otg, input1, trajectory2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(otg, input2, output2), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(otg, input1, output2), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_output_pass_to_input(output2, input1);

    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input1, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(zero_delta_otg, input1, false, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(zero_delta_otg, input1, trajectory1), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_trajectory_destroy(trajectory2);
    ruckig_trajectory_destroy(trajectory1);
    ruckig_output_destroy(output2);
    ruckig_output_destroy(output1);
    ruckig_input_destroy(input2);
    ruckig_input_destroy(input1);
    ruckig_destroy(zero_delta_otg);
    ruckig_destroy(otg);
}

void run_public_input_api_tests(void) {
    test_input_defaults_and_accessors();
    test_per_dof_setters_and_clear();
    test_per_dof_clear_restores_global_sync_behavior();
    test_per_dof_update_recalculation_stability();
    test_optional_setters_and_pass_to_input();
    test_dof_mismatch_and_invalid_discrete_duration();
}
