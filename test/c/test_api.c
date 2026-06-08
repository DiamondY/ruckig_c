#include "test_common.h"

#include "ruckig_c/alloc.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ruckig_c/ruckig.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

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
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 3.210714722108343, 1e-9);
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
        CHECK_NEAR(ruckig_trajectory_get_duration(trajectory), 2.617255105467443, 1e-9);
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

static void fill_tracking_input_1d(ruckig_input_t* input) {
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

static void fill_tracking_target_ramp(ruckig_target_state_t* target, double time) {
    const double ramp_velocity = 0.5;
    const double ramp_limit = 1.0;
    const bool on_ramp = time < ramp_limit / ramp_velocity;
    ruckig_target_state_position_data(target)[0] = on_ramp ? time * ramp_velocity : ramp_limit;
    ruckig_target_state_velocity_data(target)[0] = on_ramp ? ramp_velocity : 0.0;
    ruckig_target_state_acceleration_data(target)[0] = 0.0;
}

static void fill_tracking_input_nd(ruckig_input_t* input, size_t dofs) {
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

static void set_tracking_target_signal(ruckig_target_state_t* target, int signal, size_t dofs, double time) {
    size_t dof;
    double* position = ruckig_target_state_position_data(target);
    double* velocity = ruckig_target_state_velocity_data(target);
    double* acceleration = ruckig_target_state_acceleration_data(target);
    for (dof = 0; dof < dofs; ++dof) {
        tracking_signal_value(signal, dof, time, &position[dof], &velocity[dof], &acceleration[dof]);
    }
}

static void set_tracking_sequence_signal(ruckig_target_state_sequence_t* targets, int signal, size_t dofs, size_t count, double delta_time) {
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

static void check_tracking_output_constraints(const ruckig_output_t* output, const ruckig_input_t* input, size_t dofs) {
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

static void check_tracking_output_sequence(
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

static bool tracking_optimized_status_is_success(ruckig_tracking_calculation_status_t status) {
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

static void check_tracking_diagnostics_common(
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

    ruckig_tracking_output_sequence_destroy(output_sequence);
    ruckig_target_state_sequence_destroy(target_sequence);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
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

void run_tracking_random_tests(size_t samples, unsigned seed) {
    const size_t dof_values[4] = {1, 2, 4, 8};
    const size_t lookahead_values[4] = {1, 2, 5, 10};
    const double reactiveness_values[4] = {0.0, 0.25, 0.5, 1.0};
    const ruckig_tracking_optimized_strategy_t strategy_values[3] = {
        RUCKIG_TRACKING_OPTIMIZED_STABLE,
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE
    };
    size_t sample;
    size_t optimized_count = 0;
    size_t fallback_count = 0;
    size_t candidate_count = 0;
    unsigned state = seed ? seed : 1u;

    for (sample = 0; sample < samples; ++sample) {
        const size_t dofs = dof_values[tracking_random_pick(&state, 4)];
        const size_t lookahead_count = lookahead_values[tracking_random_pick(&state, 4)];
        const int signal = (int)tracking_random_pick(&state, 4);
        const double reactiveness = reactiveness_values[tracking_random_pick(&state, 4)];
        const ruckig_tracking_optimized_strategy_t strategy = strategy_values[tracking_random_pick(&state, 3)];
        const double dt = 0.01;
        const double start_time = (double)(sample % 200u) * dt;
        ruckig_tracking_t* tracking = NULL;
        ruckig_target_state_sequence_t* lookahead = NULL;
        ruckig_input_t* input = NULL;
        ruckig_output_t* output = NULL;
        size_t ahead;

        CHECK_EQ_INT(ruckig_tracking_create(&tracking, dofs, dt), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, dofs, lookahead_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create(&input, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create(&output, dofs), RUCKIG_WORKING);
        fill_tracking_input_nd(input, dofs);
        if (dofs > 1 && (tracking_random_next(&state) & 1u) != 0u) {
            const size_t disabled_dof = tracking_random_pick(&state, dofs);
            CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, disabled_dof, false), RUCKIG_WORKING);
        }
        CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, strategy), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, reactiveness), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, lookahead_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, lookahead_count), RUCKIG_WORKING);
        for (ahead = 0; ahead < lookahead_count; ++ahead) {
            size_t dof;
            const double time = start_time + (double)ahead * dt;
            for (dof = 0; dof < dofs; ++dof) {
                double position;
                double velocity;
                double acceleration;
                tracking_signal_value(signal, dof, time, &position, &velocity, &acceleration);
                ruckig_target_state_sequence_position_data(lookahead)[ahead * dofs + dof] = position;
                ruckig_target_state_sequence_velocity_data(lookahead)[ahead * dofs + dof] = velocity;
                ruckig_target_state_sequence_acceleration_data(lookahead)[ahead * dofs + dof] = acceleration;
            }
        }

        {
            const ruckig_result_t result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
            CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
        }
        CHECK_TRUE(tracking_optimized_status_is_success(ruckig_tracking_get_last_calculation_status(tracking)));
        CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= 1);
        CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) <= ruckig_tracking_get_max_optimized_candidates(tracking));
        check_tracking_output_constraints(output, input, dofs);
        candidate_count += ruckig_tracking_get_last_candidate_count(tracking);
        if (ruckig_tracking_get_last_calculation_status(tracking) == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
            ++optimized_count;
        } else if (ruckig_tracking_get_last_calculation_status(tracking) == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
            ++fallback_count;
        }

        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_target_state_sequence_destroy(lookahead);
        ruckig_tracking_destroy(tracking);
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

void run_tracking_api_tests(void) {
    test_tracking_api_lifecycle_and_accessors();
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
}

void run_tracking_quality_tests(void) {
    test_tracking_quality_against_instantaneous_chasing();
    test_tracking_optimized_quality_against_fast_baseline();
    test_tracking_optimized_strategy_quality_corpus();
}

void run_tracking_no_allocation_tests(void) {
    test_tracking_no_allocation();
}

void run_tracking_tests(void) {
    run_tracking_api_tests();
    run_tracking_validation_tests();
    run_tracking_online_tests();
    run_tracking_offline_tests();
    run_tracking_optimized_tests();
    run_tracking_quality_tests();
    run_tracking_no_allocation_tests();
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
    run_tracking_tests();
    test_position_third_order_nonzero_target_velocity();
}
