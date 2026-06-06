#include "test_common.h"

#include "ruckig_c/alloc.h"

#include <float.h>
#include <math.h>
#include <ruckig_c/ruckig.h>

static void test_create_destroy(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create(NULL, 3, 0.005), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_create(&otg, 0, 0.005), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_create(&otg, 3, 0.005), RUCKIG_WORKING);
    CHECK_TRUE(otg != NULL);

    CHECK_EQ_INT(ruckig_input_create(NULL, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_create(&input, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_create(&input, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_get_dof_count(input), 3);

    CHECK_EQ_INT(ruckig_output_create(&output, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_get_dof_count(output), 3);

    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_get_dof_count(trajectory), 3);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    ruckig_trajectory_destroy(NULL);
    ruckig_output_destroy(NULL);
    ruckig_input_destroy(NULL);
    ruckig_destroy(NULL);
}

static void test_null_handles_and_invalid_queries(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double duration[1] = {0.0};
    ruckig_position_extrema_t extrema[1];
    double time = 0.0;
    bool found = false;

    CHECK_EQ_INT(ruckig_calculate(NULL, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(NULL, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_validate_input(NULL, NULL, false, false), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_reset(NULL);
    ruckig_output_pass_to_input(NULL, NULL);

    CHECK_EQ_INT(ruckig_input_get_dof_count(NULL), 0);
    CHECK_TRUE(ruckig_input_current_position_data(NULL) == NULL);
    CHECK_TRUE(ruckig_input_current_position_const_data(NULL) == NULL);
    CHECK_TRUE(ruckig_input_enabled_data(NULL) == NULL);
    CHECK_TRUE(ruckig_input_enabled_const_data(NULL) == NULL);
    CHECK_EQ_INT(ruckig_output_get_dof_count(NULL), 0);
    CHECK_TRUE(ruckig_output_new_position_data(NULL) == NULL);
    CHECK_EQ_INT(ruckig_trajectory_get_dof_count(NULL), 0);
    CHECK_NEAR(ruckig_trajectory_get_duration(NULL), 0.0, 0.0);

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(NULL, input, trajectory), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(otg, NULL, trajectory), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(otg, input, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(NULL, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(otg, NULL, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(otg, input, NULL), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 0.0, position, NULL, NULL, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_independent_min_durations(trajectory, duration, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, 0.0, 0.0, &time, &found), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

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

static void test_validation(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    double* current_position;
    double* current_velocity;
    double* current_acceleration;
    double* target_position;
    double* target_velocity;
    double* target_acceleration;
    double* max_velocity;
    double* max_acceleration;
    double* max_jerk;

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.005), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);

    current_position = ruckig_input_current_position_data(input);
    current_velocity = ruckig_input_current_velocity_data(input);
    current_acceleration = ruckig_input_current_acceleration_data(input);
    target_position = ruckig_input_target_position_data(input);
    target_velocity = ruckig_input_target_velocity_data(input);
    target_acceleration = ruckig_input_target_acceleration_data(input);
    max_velocity = ruckig_input_max_velocity_data(input);
    max_acceleration = ruckig_input_max_acceleration_data(input);
    max_jerk = ruckig_input_max_jerk_data(input);

    current_position[0] = 0.0; current_position[1] = -2.0;
    current_velocity[0] = 0.0; current_velocity[1] = 0.0;
    current_acceleration[0] = 0.0; current_acceleration[1] = 0.0;
    target_position[0] = 1.0; target_position[1] = -3.0;
    target_velocity[0] = 0.0; target_velocity[1] = 0.3;
    target_acceleration[0] = 0.0; target_acceleration[1] = 0.0;
    max_velocity[0] = 1.0; max_velocity[1] = 1.0;
    max_acceleration[0] = 1.0; max_acceleration[1] = 1.0;
    max_jerk[0] = 1.0; max_jerk[1] = 1.0;

    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_WORKING);

    max_jerk[1] = NAN;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    max_jerk[1] = 1.0;

    target_velocity[1] = 1.3;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    target_velocity[1] = 0.3;

    current_velocity[0] = 2.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, false), RUCKIG_ERROR_INVALID_INPUT);
    current_velocity[0] = 1.0;

    current_acceleration[0] = 1.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_invalid_input_diagnostics(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    double min_values[2] = {-1.0, -1.0};
    double positive_min_values[2] = {-1.0, 0.1};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = -1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[1] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[1] = 1.0;

    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_WORKING);

    ruckig_input_current_position_data(input)[0] = NAN;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_current_position_data(input)[0] = 0.0;

    ruckig_input_target_acceleration_data(input)[1] = NAN;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_target_acceleration_data(input)[1] = 0.0;

    ruckig_input_max_velocity_data(input)[0] = -1.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_max_velocity_data(input)[0] = 1.0;

    ruckig_input_max_acceleration_data(input)[0] = -1.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_max_acceleration_data(input)[0] = 1.0;

    ruckig_input_max_jerk_data(input)[0] = -1.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, positive_min_values, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_min_velocity(input, min_values, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, positive_min_values, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(input, min_values, 2), RUCKIG_WORKING);

    ruckig_input_target_velocity_data(input)[0] = 2.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_target_velocity_data(input)[0] = 0.0;

    ruckig_input_current_velocity_data(input)[0] = 2.0;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, true, true), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_zero_limit_error_paths(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 0.0;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_ERROR_ZERO_LIMITS);

    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_ERROR_ZERO_LIMITS);

    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 0.0;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_ERROR_ZERO_LIMITS);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_finite_infinite_limit_semantics(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 1.0, 1e-12);

    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);

    ruckig_input_max_jerk_data(input)[0] = 1.0;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 2.0);

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 0.0;
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 1.0, 1e-12);

    ruckig_input_max_jerk_data(input)[0] = 1.0;
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_first_order_calculate_and_trajectory(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};
    double jerk[2] = {0.0, 0.0};
    double independent_min_durations[2] = {0.0, 0.0};
    ruckig_position_extrema_t extrema[2];
    size_t section = 99;
    double time = 0.0;
    bool found = false;

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_target_position_data(input)[1] = -3.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.5;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_get_independent_min_durations(trajectory, independent_min_durations, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_independent_min_durations(trajectory, independent_min_durations, 2), RUCKIG_WORKING);
    CHECK_NEAR(independent_min_durations[0], 2.0, 1e-12);
    CHECK_NEAR(independent_min_durations[1], 2.0, 1e-12);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, jerk, &section), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-12);
    CHECK_NEAR(position[1], -1.5, 1e-12);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(velocity[1], -1.5, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 0.0);
    CHECK_NEAR(jerk[0], 0.0, 0.0);
    CHECK_EQ_INT(section, 0);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 2.0, position, velocity, acceleration, jerk, &section), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 2.0, 1e-12);
    CHECK_NEAR(position[1], -3.0, 1e-12);
    CHECK_EQ_INT(section, 1);

    CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 2), RUCKIG_WORKING);
    CHECK_NEAR(extrema[0].min_position, 0.0, 0.0);
    CHECK_NEAR(extrema[0].max_position, 2.0, 0.0);
    CHECK_NEAR(extrema[1].min_position, -3.0, 0.0);
    CHECK_NEAR(extrema[1].max_position, 0.0, 0.0);

    CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, 1.5, 0.0, &time, &found), RUCKIG_WORKING);
    CHECK_TRUE(found);
    CHECK_NEAR(time, 1.5, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_first_order_update(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result = RUCKIG_WORKING;
    size_t steps = 0;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.5), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;

    while (result == RUCKIG_WORKING && steps < 4) {
        result = ruckig_update(otg, input, output);
        ruckig_output_pass_to_input(output, input);
        ++steps;
    }

    CHECK_EQ_INT(result, RUCKIG_FINISHED);
    CHECK_NEAR(ruckig_output_new_position_data(output)[0], 1.0, 1e-12);
    CHECK_NEAR(ruckig_output_new_velocity_data(output)[0], 0.0, 1e-12);
    CHECK_TRUE(steps == 3);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_update_recalculates_on_changed_target(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.5), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    ruckig_output_pass_to_input(output, input);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    ruckig_output_pass_to_input(output, input);

    ruckig_input_target_position_data(input)[0] = 2.0;
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_NEAR(ruckig_output_get_time(output), 0.5, 1e-12);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_reset_forces_recalculation(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.25), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    ruckig_output_pass_to_input(output, input);

    ruckig_reset(otg);
    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_NEAR(ruckig_output_get_time(output), 0.25, 1e-12);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_all_disabled_dofs(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    ruckig_input_current_position_data(input)[0] = 1.0;
    ruckig_input_current_position_data(input)[1] = -2.0;
    ruckig_input_current_velocity_data(input)[0] = 0.5;
    ruckig_input_current_velocity_data(input)[1] = -0.25;
    ruckig_input_current_acceleration_data(input)[0] = 0.1;
    ruckig_input_current_acceleration_data(input)[1] = -0.2;
    ruckig_input_target_position_data(input)[0] = 100.0;
    ruckig_input_target_position_data(input)[1] = -100.0;
    ruckig_input_max_velocity_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[1] = 0.0;
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 0, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 0.0, 0.0);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0 + 0.5 * 1.0 + 0.5 * 0.1, 1e-12);
    CHECK_NEAR(position[1], -2.0 - 0.25 * 1.0 - 0.5 * 0.2, 1e-12);
    CHECK_NEAR(velocity[0], 0.6, 1e-12);
    CHECK_NEAR(velocity[1], -0.45, 1e-12);
    CHECK_NEAR(acceleration[0], 0.1, 1e-12);
    CHECK_NEAR(acceleration[1], -0.2, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_position_third_order_nonzero_target_velocity(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    double duration;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[0] = 0.2;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    duration = ruckig_trajectory_get_duration(trajectory);
    CHECK_TRUE(duration > 0.0);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, duration, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-8);
    CHECK_NEAR(velocity[0], 0.2, 1e-8);
    CHECK_NEAR(acceleration[0], 0.0, 1e-10);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_position_third_order_calculate(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    double jerk[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.5;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 4.0, 1e-12);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, jerk, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0 / 6.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.5, 1e-12);
    CHECK_NEAR(acceleration[0], 1.0, 1e-12);
    CHECK_NEAR(jerk[0], -1.0, 0.0);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 2.0, position, velocity, acceleration, jerk, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-12);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 4.0, position, velocity, acceleration, jerk, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 2.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_position_third_order_minimum_duration(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.5;
    ruckig_input_max_jerk_data(input)[0] = 1.0;
    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 5.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 5.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 2.5, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.5739108254637659, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 5.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 2.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_position_third_order_velocity_limit(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 5.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 7.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 3.5, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 2.5, 1e-12);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 7.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 5.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_position_third_order_velocity_limit_discrete_nonzero_current_velocity(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
    ruckig_input_current_velocity_data(input)[0] = 0.191846;
    ruckig_input_target_position_data(input)[0] = 5.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory) / 2.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_TRUE(velocity[0] <= 1.0 + 1e-8);
    CHECK_TRUE(velocity[0] >= 0.0);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 5.0, 1e-8);
    CHECK_NEAR(velocity[0], 0.0, 1e-8);
    CHECK_NEAR(acceleration[0], 0.0, 1e-10);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_calculate_and_trajectory(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    double jerk[1] = {0.0};
    double time = 0.0;
    bool found = false;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, jerk, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 0.5, 1e-12);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(acceleration[0], -1.0, 1e-12);
    CHECK_NEAR(jerk[0], 0.0, 0.0);

    CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, 0.5, 0.0, &time, &found), RUCKIG_WORKING);
    CHECK_TRUE(found);
    CHECK_NEAR(time, 1.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_minimum_duration(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 3.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.5, position, velocity, NULL, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 0.5, 1e-12);
    CHECK_TRUE(velocity[0] >= 0.0);
    CHECK_TRUE(velocity[0] <= 1.0 + 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 3.0, position, velocity, NULL, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_discrete_duration(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.1, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_first_order_none_discrete_duration_keeps_independent_time(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_NONE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 2.05135;
    ruckig_input_target_position_data(input)[1] = 0.603709;
    ruckig_input_max_velocity_data(input)[0] = 2.3748;
    ruckig_input_max_velocity_data(input)[1] = 1.93036;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.05135 / 2.3748, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_synchronization_none_with_disabled_dof(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double independent[3] = {0.0, 0.0, 0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 3, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 3), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_NONE), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 3.0;
    ruckig_input_target_position_data(input)[2] = 100.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_velocity_data(input)[2] = 0.5;
    ruckig_input_current_velocity_data(input)[2] = 0.2;
    ruckig_input_current_acceleration_data(input)[2] = 0.1;
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 2, false), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_get_independent_min_durations(trajectory, independent, 3), RUCKIG_WORKING);
    CHECK_NEAR(independent[0], 1.0, 1e-12);
    CHECK_NEAR(independent[1], 3.0, 1e-12);
    CHECK_NEAR(independent[2], 0.0, 0.0);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_phase_sync(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_PHASE), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[1] = 2.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[1] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[1] = INFINITY;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 0.5, 1e-12);
    CHECK_NEAR(position[1], 1.0, 1e-12);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(velocity[1], 2.0, 1e-12);
    CHECK_NEAR(acceleration[0], -1.0, 1e-12);
    CHECK_NEAR(acceleration[1], -2.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_time_if_necessary_zero_target(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 4.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[1] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[1] = INFINITY;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 5.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 2.5, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_time_if_necessary_nonzero_target(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_synchronization(input, RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 4.0;
    ruckig_input_target_velocity_data(input)[0] = 0.2;
    ruckig_input_max_velocity_data(input)[0] = 1.5;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[1] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[1] = INFINITY;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 5.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 2.5, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_TRUE(fabs(position[0] - 1.0) > 1e-3);
    CHECK_TRUE(fabs(velocity[0] - 0.2) > 1e-3);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_second_order_disabled_dof(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 2, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 2), RUCKIG_WORKING);

    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_position_data(input)[1] = 1.0;
    ruckig_input_current_velocity_data(input)[1] = 0.5;
    ruckig_input_current_acceleration_data(input)[1] = 0.2;
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = 100.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[1] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    ruckig_input_max_jerk_data(input)[1] = INFINITY;
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[1], 1.6, 1e-12);
    CHECK_NEAR(velocity[1], 0.7, 1e-12);
    CHECK_NEAR(acceleration[1], 0.2, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_velocity_second_order_calculate(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 1.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 0.5, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 0.125, 1e-12);
    CHECK_NEAR(velocity[0], 0.5, 1e-12);
    CHECK_NEAR(acceleration[0], 1.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 0.5, 1e-12);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_velocity_second_order_minimum_duration(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 2.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, NULL, velocity, acceleration, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(velocity[0], 0.5, 1e-12);
    CHECK_NEAR(acceleration[0], 0.5, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_velocity_second_order_update(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.5), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = INFINITY;

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_output_new_velocity_data(output)[0], 0.5, 1e-12);
    CHECK_NEAR(ruckig_output_new_acceleration_data(output)[0], 1.0, 1e-12);
    ruckig_output_pass_to_input(output, input);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_output_new_velocity_data(output)[0], 1.0, 1e-12);
    ruckig_output_pass_to_input(output, input);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_FINISHED);
    CHECK_NEAR(ruckig_output_new_velocity_data(output)[0], 1.0, 1e-12);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_velocity_third_order_calculate(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    double jerk[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 1.0, position, velocity, acceleration, jerk, NULL), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0 / 6.0, 1e-12);
    CHECK_NEAR(velocity[0], 0.5, 1e-12);
    CHECK_NEAR(acceleration[0], 1.0, 1e-12);
    CHECK_NEAR(jerk[0], -1.0, 0.0);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 2.0, position, velocity, acceleration, jerk, NULL), RUCKIG_WORKING);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_velocity_third_order_minimum_duration(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY), RUCKIG_WORKING);
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;
    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 3.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.0, 1e-12);
    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 3.0, position, velocity, acceleration, NULL, NULL), RUCKIG_WORKING);
    CHECK_NEAR(velocity[0], 1.0, 1e-12);
    CHECK_NEAR(acceleration[0], 0.0, 1e-12);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

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

static void test_waypoint_constructors_storage_and_optimizer(void) {
    ruckig_t* otg = NULL;
    ruckig_t* section_otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_input_t* section_input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_trajectory_t* section_trajectory = NULL;
    double waypoint[1] = {1.0};
    double per_section_max_velocity[2] = {1.2, 1.0};
    double per_section_min_velocity[2] = {-1.2, -1.0};
    double per_section_max_acceleration[2] = {2.0, 2.0};
    double per_section_min_acceleration[2] = {-2.0, -2.0};
    double per_section_max_jerk[2] = {4.0, 4.0};
    double per_section_max_position[2] = {1.5, 2.5};
    double per_section_min_position[2] = {-0.5, 0.5};
    double per_section_minimum_duration[2] = {0.0, 0.0};
    double waypoint_readback[1] = {0.0};
    double per_section_readback[2] = {0.0, 0.0};
    double intermediate_durations[1] = {0.0};
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};
    ruckig_position_extrema_t extrema[1];
    double first_time = 0.0;
    bool found = false;
    size_t section = 99;
    size_t allocations_before = 0;
    double zero_derivative_segment_duration = 0.0;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_create(&section_otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_get_max_number_of_waypoints(otg), 2);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&section_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 2), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&section_trajectory, 1), RUCKIG_WORKING);

    CHECK_TRUE(ruckig_input_max_position_data(input) != NULL);
    CHECK_TRUE(ruckig_input_min_position_data(input) != NULL);
    CHECK_TRUE(isinf(ruckig_input_max_position_const_data(input)[0]));
    CHECK_TRUE(isinf(ruckig_input_min_position_const_data(input)[0]));
    ruckig_input_max_position_data(input)[0] = 3.0;
    ruckig_input_min_position_data(input)[0] = -1.0;

    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_get_intermediate_position_count(input), 1);
    CHECK_EQ_INT(ruckig_input_get_intermediate_positions(input, waypoint_readback, 1), RUCKIG_WORKING);
    CHECK_NEAR(waypoint_readback[0], 1.0, 0.0);

    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 2), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_input_has_per_section_max_velocity(input));
    CHECK_TRUE(ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_get_per_section_max_velocity(input, per_section_readback, 2), RUCKIG_WORKING);
    CHECK_NEAR(per_section_readback[0], 1.2, 0.0);
    CHECK_NEAR(per_section_readback[1], 1.0, 0.0);

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;

    ruckig_input_current_position_data(section_input)[0] = 0.0;
    ruckig_input_target_position_data(section_input)[0] = 1.0;
    ruckig_input_max_velocity_data(section_input)[0] = per_section_max_velocity[0];
    CHECK_EQ_INT(ruckig_input_set_min_velocity(section_input, per_section_min_velocity, 1), RUCKIG_WORKING);
    ruckig_input_max_acceleration_data(section_input)[0] = per_section_max_acceleration[0];
    CHECK_EQ_INT(ruckig_input_set_min_acceleration(section_input, per_section_min_acceleration, 1), RUCKIG_WORKING);
    ruckig_input_max_jerk_data(section_input)[0] = per_section_max_jerk[0];
    CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
    zero_derivative_segment_duration += ruckig_trajectory_get_duration(section_trajectory);

    ruckig_input_current_position_data(section_input)[0] = 1.0;
    ruckig_input_target_position_data(section_input)[0] = 2.0;
    ruckig_input_max_velocity_data(section_input)[0] = per_section_max_velocity[1];
    {
        double min_velocity[1] = {per_section_min_velocity[1]};
        double min_acceleration[1] = {per_section_min_acceleration[1]};
        CHECK_EQ_INT(ruckig_input_set_min_velocity(section_input, min_velocity, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_min_acceleration(section_input, min_acceleration, 1), RUCKIG_WORKING);
    }
    ruckig_input_max_acceleration_data(section_input)[0] = per_section_max_acceleration[1];
    ruckig_input_max_jerk_data(section_input)[0] = per_section_max_jerk[1];
    CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
    zero_derivative_segment_duration += ruckig_trajectory_get_duration(section_trajectory);

    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_WORKING);
    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), 2);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(trajectory), 1);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_durations, 1), RUCKIG_WORKING);
    CHECK_TRUE(intermediate_durations[0] > 0.0);
    CHECK_TRUE(intermediate_durations[0] < ruckig_trajectory_get_duration(trajectory));

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, intermediate_durations[0], position, velocity, acceleration, NULL, &section), RUCKIG_WORKING);
    CHECK_NEAR(position[0], 1.0, 1e-7);
    CHECK_EQ_INT(section, 1);
    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) <= zero_derivative_segment_duration + 1e-9);
    CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 1), RUCKIG_WORKING);
    CHECK_TRUE(extrema[0].min_position >= -1e-9);
    CHECK_TRUE(extrema[0].max_position <= 2.0 + 1e-9);
    CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, 1.0, 0.0, &first_time, &found), RUCKIG_WORKING);
    CHECK_TRUE(found);
    CHECK_NEAR(first_time, intermediate_durations[0], 1e-7);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));

    ruckig_trajectory_destroy(section_trajectory);
    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(section_input);
    ruckig_input_destroy(input);
    ruckig_destroy(section_otg);
    ruckig_destroy(otg);
}

static void test_waypoint_validation_and_filter(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoints[3] = {1.0, 2.0, 3.0};
    double filtered[3] = {0.0, 0.0, 0.0};
    double threshold[1] = {0.25};
    double short_waypoint[1] = {1.0};
    double valid_minimum_durations[4] = {0.0, 0.0, 0.0, 0.0};
    size_t written = 99;
    ruckig_control_interface_t per_control[1] = {RUCKIG_CONTROL_POSITION};

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 3), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, 1), RUCKIG_WORKING);
    ruckig_input_target_position_data(input)[0] = 4.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 2.0;

    CHECK_EQ_INT(ruckig_filter_intermediate_positions(otg, input, threshold, 1, filtered, 3, &written), RUCKIG_WORKING);
    CHECK_EQ_INT(written, 0);

    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_DISCRETE), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_duration_discretization(input, RUCKIG_DURATION_CONTINUOUS), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_minimum_duration(input, 1.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_minimum_duration(input);

    CHECK_EQ_INT(ruckig_input_set_per_dof_control_interface(input, per_control, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_clear_per_dof_control_interface(input);

    ruckig_input_max_jerk_data(input)[0] = INFINITY;
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_input_max_jerk_data(input)[0] = 2.0;

    {
        double invalid_max_velocity[4] = {1.0, 1.0, -1.0, 1.0};
        double invalid_min_velocity[4] = {-1.0, -1.0, 0.5, -1.0};
        double invalid_max_jerk[4] = {2.0, 2.0, INFINITY, 2.0};
        double invalid_max_position[4] = {2.0, 2.0, 2.0, 2.0};
        double invalid_min_position[4] = {-1.0, -1.0, 3.0, -1.0};
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, invalid_max_velocity, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_max_velocity(input);

        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, invalid_min_velocity, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_min_velocity(input);

        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, invalid_max_jerk, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_max_jerk(input);

        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, invalid_max_position, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, invalid_min_position, 4, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
        ruckig_input_clear_per_section_max_position(input);
        ruckig_input_clear_per_section_min_position(input);
    }

    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, valid_minimum_durations, 4), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, short_waypoint, 1, 1), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_input_has_per_section_minimum_duration(input));
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 0, false), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_validate_input(otg, input, false, true), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 0, true), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void check_waypoint_samples(
    const ruckig_trajectory_t* trajectory,
    const double* waypoints,
    size_t waypoint_count,
    size_t dofs
) {
    double durations[4] = {0.0, 0.0, 0.0, 0.0};
    double position[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    size_t waypoint;
    size_t dof;

    CHECK_TRUE(waypoint_count <= 4);
    CHECK_TRUE(dofs <= 8);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), waypoint_count + 1);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_duration_count(trajectory), waypoint_count);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, waypoint_count), RUCKIG_WORKING);
    for (waypoint = 0; waypoint < waypoint_count; ++waypoint) {
        size_t section = 99;
        CHECK_TRUE(durations[waypoint] > 0.0);
        CHECK_TRUE(durations[waypoint] < ruckig_trajectory_get_duration(trajectory));
        if (waypoint > 0) {
            CHECK_TRUE(durations[waypoint] > durations[waypoint - 1]);
        }
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, durations[waypoint], position, NULL, NULL, NULL, &section), RUCKIG_WORKING);
        CHECK_EQ_INT(section, waypoint + 1);
        for (dof = 0; dof < dofs; ++dof) {
            CHECK_NEAR(position[dof], waypoints[waypoint * dofs + dof], 1e-7);
        }
    }
}

static void check_waypoint_section_sampled_limits(
    const ruckig_trajectory_t* trajectory,
    const double* per_section_min_velocity,
    const double* per_section_max_velocity,
    const double* per_section_min_acceleration,
    const double* per_section_max_acceleration,
    const double* per_section_max_jerk,
    const double* per_section_min_position,
    const double* per_section_max_position,
    size_t section_count,
    size_t dofs
) {
    double durations[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double position[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double velocity[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double acceleration[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double jerk[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    const double total_duration = ruckig_trajectory_get_duration(trajectory);
    size_t section;

    CHECK_TRUE(section_count <= 8);
    CHECK_TRUE(dofs <= 8);
    CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), section_count);
    if (section_count > 1) {
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, section_count - 1), RUCKIG_WORKING);
    }

    for (section = 0; section < section_count; ++section) {
        const double start = section == 0 ? 0.0 : durations[section - 1];
        const double end = section + 1 == section_count ? total_duration : durations[section];
        size_t sample_index;

        CHECK_TRUE(end > start);
        for (sample_index = 1; sample_index <= 5; ++sample_index) {
            const double t = start + (end - start) * ((double)sample_index / 6.0);
            size_t sampled_section = 99;
            size_t dof;
            CHECK_EQ_INT(ruckig_trajectory_at_time(
                             trajectory,
                             t,
                             position,
                             velocity,
                             acceleration,
                             jerk,
                             &sampled_section),
                         RUCKIG_WORKING);
            CHECK_EQ_INT(sampled_section, section);
            for (dof = 0; dof < dofs; ++dof) {
                const size_t index = section * dofs + dof;
                if (per_section_min_position) {
                    CHECK_TRUE(position[dof] >= per_section_min_position[index] - 1e-7);
                }
                if (per_section_max_position) {
                    CHECK_TRUE(position[dof] <= per_section_max_position[index] + 1e-7);
                }
                if (per_section_min_velocity) {
                    CHECK_TRUE(velocity[dof] >= per_section_min_velocity[index] - 1e-7);
                }
                if (per_section_max_velocity) {
                    CHECK_TRUE(velocity[dof] <= per_section_max_velocity[index] + 1e-7);
                }
                if (per_section_min_acceleration) {
                    CHECK_TRUE(acceleration[dof] >= per_section_min_acceleration[index] - 1e-7);
                }
                if (per_section_max_acceleration) {
                    CHECK_TRUE(acceleration[dof] <= per_section_max_acceleration[index] + 1e-7);
                }
                if (per_section_max_jerk && isfinite(per_section_max_jerk[index])) {
                    CHECK_TRUE(fabs(jerk[dof]) <= per_section_max_jerk[index] + 1e-7);
                }
            }
        }
    }
}

static void test_waypoint_fixed_regression_corpus(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[2] = {1.0, -0.5};
        double intermediate_duration[1] = {0.0};
        double position[2] = {0.0, 0.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.01, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 2, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_target_position_data(input)[1] = -1.0;
        ruckig_input_max_velocity_data(input)[0] = 1.0;
        ruckig_input_max_velocity_data(input)[1] = 1.0;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_acceleration_data(input)[1] = 2.0;
        ruckig_input_max_jerk_data(input)[0] = 4.0;
        ruckig_input_max_jerk_data(input)[1] = 4.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.0, 1e-9);
        CHECK_EQ_INT(ruckig_trajectory_get_section_count(trajectory), 2);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, intermediate_duration[0], position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], waypoint[0], 1e-7);
        CHECK_NEAR(position[1], waypoint[1], 1e-7);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[1] = {1.0};
        double per_section_max_velocity[2] = {0.8, 1.4};
        double per_section_min_velocity[2] = {-0.8, -1.4};
        double per_section_max_acceleration[2] = {1.2, 2.0};
        double per_section_min_acceleration[2] = {-1.2, -2.0};
        double per_section_max_jerk[2] = {3.0, 5.0};
        double per_section_max_position[2] = {1.1, 2.1};
        double per_section_min_position[2] = {-0.1, 0.9};
        ruckig_position_extrema_t extrema[1];

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_max_velocity_data(input)[0] = 1.5;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_jerk_data(input)[0] = 5.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.211521, 1e-5);
        CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 1), RUCKIG_WORKING);
        CHECK_TRUE(extrema[0].min_position >= -1e-9);
        CHECK_TRUE(extrema[0].max_position <= 2.0 + 1e-9);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        const size_t dofs = 3;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[6] = {0.5, -0.2, 0.25, 1.0, -0.4, 0.50};
        double durations[2] = {0.0, 0.0};
        double position[3] = {0.0, 0.0, 0.0};
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.02, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.5;
            ruckig_input_max_acceleration_data(input)[i] = 2.0;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_target_position_data(input)[0] = 1.5;
        ruckig_input_target_position_data(input)[1] = -0.6;
        ruckig_input_target_position_data(input)[2] = 0.75;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.426124, 1e-5);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2), RUCKIG_WORKING);
        CHECK_TRUE(durations[0] > 0.0 && durations[0] < durations[1]);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, durations[1], position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], waypoints[3], 1e-7);
        CHECK_NEAR(position[1], waypoints[4], 1e-7);
        CHECK_NEAR(position[2], waypoints[5], 1e-7);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[2] = {1.0, 5.0};
        double intermediate_duration[1] = {0.0};
        double position[2] = {0.0, 0.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 2, 0.05, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 2, 1), RUCKIG_WORKING);
        ruckig_input_current_position_data(input)[1] = 5.0;
        ruckig_input_target_position_data(input)[0] = 2.0;
        ruckig_input_target_position_data(input)[1] = 5.0;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_velocity_data(input)[1] = 1.0;
        ruckig_input_max_acceleration_data(input)[0] = 2.0;
        ruckig_input_max_acceleration_data(input)[1] = 1.0;
        ruckig_input_max_jerk_data(input)[0] = 4.0;
        ruckig_input_max_jerk_data(input)[1] = 2.0;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 1, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, intermediate_duration[0], position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 1.0, 1e-7);
        CHECK_NEAR(position[1], 5.0, 1e-12);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 2.0, 1e-7);
        CHECK_NEAR(position[1], 5.0, 1e-12);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_alpha2_fixed_regression_corpus(void) {
    {
        const size_t dofs = 4;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[8] = {
            0.25, -0.15, 0.20, -0.10,
            0.75, -0.45, 0.45, -0.30
        };
        double per_section_minimum_duration[3] = {0.35, 0.60, 0.45};
        double per_section_max_position[12] = {
            0.30, 0.05, 0.25, 0.05,
            0.80, -0.10, 0.50, -0.05,
            1.25, -0.35, 0.80, -0.20
        };
        double per_section_min_position[12] = {
            -0.05, -0.20, -0.05, -0.15,
            0.20, -0.50, 0.15, -0.35,
            0.70, -0.75, 0.40, -0.55
        };
        ruckig_position_extrema_t extrema[4];
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.4;
            ruckig_input_max_acceleration_data(input)[i] = 2.0;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_target_position_data(input)[0] = 1.10;
        ruckig_input_target_position_data(input)[1] = -0.65;
        ruckig_input_target_position_data(input)[2] = 0.70;
        ruckig_input_target_position_data(input)[3] = -0.50;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) >= 1.40);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 6.00);
        check_waypoint_samples(trajectory, waypoints, 2, dofs);
        CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, dofs), RUCKIG_WORKING);
        CHECK_TRUE(extrema[0].min_position >= -0.05 - 1e-9);
        CHECK_TRUE(extrema[0].max_position <= 1.25 + 1e-9);
        CHECK_TRUE(extrema[1].min_position >= -0.75 - 1e-9);
        CHECK_TRUE(extrema[1].max_position <= 0.05 + 1e-9);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        const size_t dofs = 6;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoint[6] = {0.20, -0.10, 0.30, -0.20, 5.0, -3.0};
        double position[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.02, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 1), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.5;
            ruckig_input_max_acceleration_data(input)[i] = 2.0;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_current_position_data(input)[4] = 5.0;
        ruckig_input_current_position_data(input)[5] = -3.0;
        ruckig_input_target_position_data(input)[0] = 0.50;
        ruckig_input_target_position_data(input)[1] = -0.25;
        ruckig_input_target_position_data(input)[2] = 0.75;
        ruckig_input_target_position_data(input)[3] = -0.45;
        ruckig_input_target_position_data(input)[4] = 5.0;
        ruckig_input_target_position_data(input)[5] = -3.0;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 4, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 5, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        check_waypoint_samples(trajectory, waypoint, 1, dofs);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[4], 5.0, 1e-12);
        CHECK_NEAR(position[5], -3.0, 1e-12);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_041_deep_regression_corpus(void) {
    {
        const size_t dofs = 8;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[24] = {
            0.10, -0.05, 0.12, -0.08, 0.06, -0.04, 7.0, -2.0,
            0.35, -0.20, 0.30, -0.25, 0.18, -0.12, 7.0, -2.0,
            0.70, -0.45, 0.55, -0.40, 0.32, -0.24, 7.0, -2.0
        };
        double position[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 3), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.8;
            ruckig_input_max_acceleration_data(input)[i] = 2.5;
            ruckig_input_max_jerk_data(input)[i] = 5.0;
        }
        ruckig_input_current_position_data(input)[6] = 7.0;
        ruckig_input_current_position_data(input)[7] = -2.0;
        ruckig_input_target_position_data(input)[0] = 1.00;
        ruckig_input_target_position_data(input)[1] = -0.65;
        ruckig_input_target_position_data(input)[2] = 0.75;
        ruckig_input_target_position_data(input)[3] = -0.55;
        ruckig_input_target_position_data(input)[4] = 0.45;
        ruckig_input_target_position_data(input)[5] = -0.33;
        ruckig_input_target_position_data(input)[6] = 7.0;
        ruckig_input_target_position_data(input)[7] = -2.0;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 6, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, 7, false), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 8.0);
        check_waypoint_samples(trajectory, waypoints, 3, dofs);
        CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, NULL, NULL, NULL, NULL), RUCKIG_WORKING);
        CHECK_NEAR(position[0], 1.00, 1e-7);
        CHECK_NEAR(position[5], -0.33, 1e-7);
        CHECK_NEAR(position[6], 7.0, 1e-12);
        CHECK_NEAR(position[7], -2.0, 1e-12);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        const size_t dofs = 4;
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[8] = {
            0.30, -0.15, 0.22, -0.10,
            0.82, -0.42, 0.46, -0.30
        };
        double per_section_min_velocity[12] = {
            -0.65, -0.75, -0.70, -0.60,
            -0.80, -0.85, -0.78, -0.70,
            -0.90, -0.95, -0.85, -0.80
        };
        double per_section_max_velocity[12] = {
            0.70, 0.75, 0.70, 0.65,
            0.85, 0.90, 0.82, 0.75,
            1.00, 1.05, 0.92, 0.88
        };
        double per_section_min_acceleration[12] = {
            -1.2, -1.2, -1.1, -1.0,
            -1.4, -1.4, -1.3, -1.2,
            -1.6, -1.6, -1.5, -1.4
        };
        double per_section_max_acceleration[12] = {
            1.2, 1.2, 1.1, 1.0,
            1.4, 1.4, 1.3, 1.2,
            1.6, 1.6, 1.5, 1.4
        };
        double per_section_max_jerk[12] = {
            3.0, 3.0, 2.8, 2.6,
            3.4, 3.4, 3.2, 3.0,
            3.8, 3.8, 3.5, 3.3
        };
        double per_section_min_position[12] = {
            -0.05, -0.20, -0.05, -0.15,
            0.25, -0.50, 0.15, -0.35,
            0.75, -0.75, 0.38, -0.55
        };
        double per_section_max_position[12] = {
            0.35, 0.05, 0.25, 0.05,
            0.88, -0.10, 0.50, -0.05,
            1.25, -0.38, 0.78, -0.22
        };
        double per_section_minimum_duration[3] = {0.40, 0.70, 0.50};
        ruckig_position_extrema_t extrema[4];
        size_t i;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2), RUCKIG_WORKING);
        for (i = 0; i < dofs; ++i) {
            ruckig_input_max_velocity_data(input)[i] = 1.1;
            ruckig_input_max_acceleration_data(input)[i] = 1.8;
            ruckig_input_max_jerk_data(input)[i] = 4.0;
        }
        ruckig_input_target_position_data(input)[0] = 1.15;
        ruckig_input_target_position_data(input)[1] = -0.62;
        ruckig_input_target_position_data(input)[2] = 0.70;
        ruckig_input_target_position_data(input)[3] = -0.48;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 3, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) >= 1.60);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 7.50);
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
        CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, dofs), RUCKIG_WORKING);
        CHECK_TRUE(extrema[0].min_position >= -0.05 - 1e-9);
        CHECK_TRUE(extrema[0].max_position <= 1.25 + 1e-9);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }

    {
        ruckig_t* otg = NULL;
        ruckig_t* section_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* section_input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        ruckig_trajectory_t* section_trajectory = NULL;
        double waypoints[2] = {0.45, 0.95};
        double durations[2] = {0.0, 0.0};
        double baseline_duration = 0.0;
        double first_time = 0.0;
        bool found = false;
        size_t section_index;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create(&section_otg, 1, 0.01), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&section_input, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create(&section_trajectory, 1), RUCKIG_WORKING);
        ruckig_input_current_velocity_data(input)[0] = 0.20;
        ruckig_input_target_position_data(input)[0] = 1.40;
        ruckig_input_target_velocity_data(input)[0] = -0.10;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_acceleration_data(input)[0] = 2.4;
        ruckig_input_max_jerk_data(input)[0] = 5.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 1), RUCKIG_WORKING);

        ruckig_input_max_velocity_data(section_input)[0] = 1.2;
        ruckig_input_max_acceleration_data(section_input)[0] = 2.4;
        ruckig_input_max_jerk_data(section_input)[0] = 5.0;
        ruckig_input_current_velocity_data(section_input)[0] = 0.20;
        ruckig_input_target_position_data(section_input)[0] = waypoints[0];
        CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
        baseline_duration += ruckig_trajectory_get_duration(section_trajectory);
        for (section_index = 1; section_index < 3; ++section_index) {
            ruckig_input_current_position_data(section_input)[0] = waypoints[section_index - 1];
            ruckig_input_current_velocity_data(section_input)[0] = 0.0;
            ruckig_input_target_position_data(section_input)[0] = section_index == 2 ? 1.40 : waypoints[section_index];
            ruckig_input_target_velocity_data(section_input)[0] = section_index == 2 ? -0.10 : 0.0;
            CHECK_EQ_INT(ruckig_calculate(section_otg, section_input, section_trajectory), RUCKIG_WORKING);
            baseline_duration += ruckig_trajectory_get_duration(section_trajectory);
        }

        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) <= baseline_duration + 1e-9);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2), RUCKIG_WORKING);
        CHECK_TRUE(durations[0] < durations[1]);
        CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, waypoints[1], durations[0] + 1e-9, &first_time, &found), RUCKIG_WORKING);
        CHECK_TRUE(found);
        CHECK_NEAR(first_time, durations[1], 1e-7);
        ruckig_trajectory_destroy(section_trajectory);
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(section_input);
        ruckig_input_destroy(input);
        ruckig_destroy(section_otg);
        ruckig_destroy(otg);
    }
}

static void test_waypoint_alpha2_quality_regressions(void) {
    {
        ruckig_t* otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_trajectory_t* trajectory = NULL;
        double waypoints[2] = {0.45, 0.95};
        double durations[2] = {0.0, 0.0};

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.01, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 2), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 2), RUCKIG_WORKING);
        ruckig_input_target_position_data(input)[0] = 1.40;
        ruckig_input_current_velocity_data(input)[0] = 0.10;
        ruckig_input_target_velocity_data(input)[0] = -0.05;
        ruckig_input_max_velocity_data(input)[0] = 1.2;
        ruckig_input_max_acceleration_data(input)[0] = 2.4;
        ruckig_input_max_jerk_data(input)[0] = 5.0;
        CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, 1), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) < 4.50);
        CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2), RUCKIG_WORKING);
        CHECK_TRUE(durations[0] < durations[1]);
        CHECK_TRUE(durations[1] < ruckig_trajectory_get_duration(trajectory));
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
    }
}

void run_waypoint_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_validation_and_filter();
    test_waypoint_fixed_regression_corpus();
    test_waypoint_alpha2_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
    test_waypoint_alpha2_quality_regressions();
}

void run_waypoint_per_section_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_alpha2_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
}

void run_waypoint_quality_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
    test_waypoint_alpha2_quality_regressions();
}

void run_api_tests(void) {
    test_create_destroy();
    test_null_handles_and_invalid_queries();
    test_input_defaults_and_accessors();
    test_per_dof_setters_and_clear();
    test_per_dof_clear_restores_global_sync_behavior();
    test_per_dof_update_recalculation_stability();
    test_optional_setters_and_pass_to_input();
    test_dof_mismatch_and_invalid_discrete_duration();
    test_validation();
    test_invalid_input_diagnostics();
    test_zero_limit_error_paths();
    test_finite_infinite_limit_semantics();
    test_first_order_calculate_and_trajectory();
    test_first_order_update();
    test_update_recalculates_on_changed_target();
    test_reset_forces_recalculation();
    test_all_disabled_dofs();
    test_second_order_calculate_and_trajectory();
    test_second_order_minimum_duration();
    test_second_order_discrete_duration();
    test_first_order_none_discrete_duration_keeps_independent_time();
    test_synchronization_none_with_disabled_dof();
    test_second_order_phase_sync();
    test_second_order_time_if_necessary_zero_target();
    test_second_order_time_if_necessary_nonzero_target();
    test_second_order_disabled_dof();
    test_position_third_order_calculate();
    test_position_third_order_minimum_duration();
    test_position_third_order_velocity_limit();
    test_position_third_order_velocity_limit_discrete_nonzero_current_velocity();
    test_velocity_second_order_calculate();
    test_velocity_second_order_minimum_duration();
    test_velocity_second_order_update();
    test_velocity_third_order_calculate();
    test_velocity_third_order_minimum_duration();
    test_no_allocation_in_realtime_paths();
    run_waypoint_tests();
    test_position_third_order_nonzero_target_velocity();
}
