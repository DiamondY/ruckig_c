#include "test_common.h"

#include "ruckig_c/alloc.h"
#include "ruckig_c/internal.h"

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

static void configure_soft_interruption_waypoint_input(ruckig_input_t* input) {
    double waypoint[1] = {1.0};
    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoint, 1, 1), RUCKIG_WORKING);
}

static void configure_alpha2_resume_input(ruckig_input_t* input) {
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
    double per_section_minimum_duration[3] = {0.30, 0.42, 0.36};
    size_t i;

    for (i = 0; i < dofs; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.2;
        ruckig_input_max_acceleration_data(input)[i] = 1.8;
        ruckig_input_max_jerk_data(input)[i] = 4.5;
    }
    ruckig_input_target_position_data(input)[0] = 1.12;
    ruckig_input_target_position_data(input)[1] = -0.54;
    ruckig_input_target_position_data(input)[2] = 0.72;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 3), RUCKIG_WORKING);
}

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

static void configure_alpha1_resume_stress_input(ruckig_input_t* input) {
    const size_t dofs = 4;
    double waypoints[12] = {
        0.25, -0.10, 0.15, -0.05,
        0.55, -0.28, 0.34, -0.18,
        0.90, -0.50, 0.58, -0.34
    };
    double per_section_min_velocity[16] = {
        -0.80, -0.75, -0.70, -0.65,
        -0.85, -0.80, -0.75, -0.70,
        -0.95, -0.90, -0.85, -0.80,
        -1.05, -1.00, -0.95, -0.90
    };
    double per_section_max_velocity[16] = {
        0.90, 0.85, 0.80, 0.75,
        0.98, 0.92, 0.86, 0.80,
        1.08, 1.00, 0.94, 0.88,
        1.18, 1.10, 1.02, 0.96
    };
    double per_section_min_acceleration[16] = {
        -1.5, -1.4, -1.3, -1.2,
        -1.6, -1.5, -1.4, -1.3,
        -1.7, -1.6, -1.5, -1.4,
        -1.8, -1.7, -1.6, -1.5
    };
    double per_section_max_acceleration[16] = {
        1.5, 1.4, 1.3, 1.2,
        1.6, 1.5, 1.4, 1.3,
        1.7, 1.6, 1.5, 1.4,
        1.8, 1.7, 1.6, 1.5
    };
    double per_section_max_jerk[16] = {
        3.8, 3.6, 3.4, 3.2,
        4.0, 3.8, 3.6, 3.4,
        4.2, 4.0, 3.8, 3.6,
        4.4, 4.2, 4.0, 3.8
    };
    double per_section_min_position[16] = {
        -0.05, -0.15, -0.05, -0.10,
        0.18, -0.33, 0.10, -0.22,
        0.48, -0.55, 0.28, -0.38,
        0.82, -0.75, 0.52, -0.55
    };
    double per_section_max_position[16] = {
        0.32, 0.03, 0.22, 0.02,
        0.62, -0.05, 0.40, -0.02,
        0.98, -0.22, 0.64, -0.14,
        1.35, -0.45, 0.90, -0.30
    };
    double per_section_minimum_duration[4] = {0.55, 0.45, 0.45, 0.40};
    size_t i;

    for (i = 0; i < dofs; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.4;
        ruckig_input_max_acceleration_data(input)[i] = 2.0;
        ruckig_input_max_jerk_data(input)[i] = 4.8;
    }
    ruckig_input_target_position_data(input)[0] = 1.25;
    ruckig_input_target_position_data(input)[1] = -0.70;
    ruckig_input_target_position_data(input)[2] = 0.82;
    ruckig_input_target_position_data(input)[3] = -0.50;
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, 3, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, 4, dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 4), RUCKIG_WORKING);
}

static void check_alpha1_resume_stress_trajectory(const ruckig_trajectory_t* trajectory) {
    const size_t dofs = 4;
    double waypoints[12] = {
        0.25, -0.10, 0.15, -0.05,
        0.55, -0.28, 0.34, -0.18,
        0.90, -0.50, 0.58, -0.34
    };
    double per_section_min_velocity[16] = {
        -0.80, -0.75, -0.70, -0.65,
        -0.85, -0.80, -0.75, -0.70,
        -0.95, -0.90, -0.85, -0.80,
        -1.05, -1.00, -0.95, -0.90
    };
    double per_section_max_velocity[16] = {
        0.90, 0.85, 0.80, 0.75,
        0.98, 0.92, 0.86, 0.80,
        1.08, 1.00, 0.94, 0.88,
        1.18, 1.10, 1.02, 0.96
    };
    double per_section_min_acceleration[16] = {
        -1.5, -1.4, -1.3, -1.2,
        -1.6, -1.5, -1.4, -1.3,
        -1.7, -1.6, -1.5, -1.4,
        -1.8, -1.7, -1.6, -1.5
    };
    double per_section_max_acceleration[16] = {
        1.5, 1.4, 1.3, 1.2,
        1.6, 1.5, 1.4, 1.3,
        1.7, 1.6, 1.5, 1.4,
        1.8, 1.7, 1.6, 1.5
    };
    double per_section_max_jerk[16] = {
        3.8, 3.6, 3.4, 3.2,
        4.0, 3.8, 3.6, 3.4,
        4.2, 4.0, 3.8, 3.6,
        4.4, 4.2, 4.0, 3.8
    };
    double per_section_min_position[16] = {
        -0.05, -0.15, -0.05, -0.10,
        0.18, -0.33, 0.10, -0.22,
        0.48, -0.55, 0.28, -0.38,
        0.82, -0.75, 0.52, -0.55
    };
    double per_section_max_position[16] = {
        0.32, 0.03, 0.22, 0.02,
        0.62, -0.05, 0.40, -0.02,
        0.98, -0.22, 0.64, -0.14,
        1.35, -0.45, 0.90, -0.30
    };
    double per_section_minimum_duration[4] = {0.55, 0.45, 0.45, 0.40};
    double durations[3] = {0.0, 0.0, 0.0};
    const double total_duration = ruckig_trajectory_get_duration(trajectory);
    size_t section;

    CHECK_TRUE(total_duration > 0.0);
    check_waypoint_samples(trajectory, waypoints, 3, dofs);
    check_waypoint_section_sampled_limits(
        trajectory,
        per_section_min_velocity,
        per_section_max_velocity,
        per_section_min_acceleration,
        per_section_max_acceleration,
        per_section_max_jerk,
        per_section_min_position,
        per_section_max_position,
        4,
        dofs);
    CHECK_EQ_INT(ruckig_trajectory_get_intermediate_durations(trajectory, durations, 3), RUCKIG_WORKING);
    for (section = 0; section < 4; ++section) {
        const double start = section == 0 ? 0.0 : durations[section - 1];
        const double end = section == 3 ? total_duration : durations[section];
        CHECK_TRUE(end > start);
        CHECK_TRUE(end - start >= per_section_minimum_duration[section] - 1e-9);
    }
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

static ruckig_result_t ruckig_update_under_allocation_guard(
    ruckig_t* otg,
    ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_update(otg, input, output);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

#define WAYPOINT_RESUME_QUALITY_CASES 128u

typedef struct waypoint_resume_quality_baseline {
    ruckig_result_t result;
    double duration;
} waypoint_resume_quality_baseline_t;

static const waypoint_resume_quality_baseline_t waypoint_resume_quality_baseline[WAYPOINT_RESUME_QUALITY_CASES] = {
    {RUCKIG_WORKING, 1.8761222240029074},
    {RUCKIG_WORKING, 2.0103531913159798},
    {RUCKIG_WORKING, 2.1367785811810092},
    {RUCKIG_WORKING, 2.3231046365914789},
    {RUCKIG_WORKING, 1.9563060529630401},
    {RUCKIG_WORKING, 2.1639081016881558},
    {RUCKIG_WORKING, 2.334308960646033},
    {RUCKIG_WORKING, 2.463331019114928},
    {RUCKIG_WORKING, 2.0843835602493597},
    {RUCKIG_WORKING, 2.9613331216506076},
    {RUCKIG_WORKING, 3.0447526376853462},
    {RUCKIG_WORKING, 2.9220220742529053},
    {RUCKIG_WORKING, 1.8927697784843993},
    {RUCKIG_WORKING, 2.0322608374341988},
    {RUCKIG_WORKING, 2.1567261904761903},
    {RUCKIG_WORKING, 2.2663324979114456},
    {RUCKIG_WORKING, 1.9684155547745192},
    {RUCKIG_WORKING, 2.1692990031800696},
    {RUCKIG_WORKING, 2.5305866703295874},
    {RUCKIG_WORKING, 2.4550580647027891},
    {RUCKIG_WORKING, 2.0569052768441773},
    {RUCKIG_WORKING, 2.7292669721409863},
    {RUCKIG_WORKING, 3.0123994118392505},
    {RUCKIG_WORKING, 2.8644266241023915},
    {RUCKIG_WORKING, 1.9245663979634555},
    {RUCKIG_WORKING, 2.0413240040177811},
    {RUCKIG_WORKING, 2.1612818916534553},
    {RUCKIG_WORKING, 2.4002506265664163},
    {RUCKIG_WORKING, 1.977557379612221},
    {RUCKIG_WORKING, 2.2563187996268708},
    {RUCKIG_WORKING, 2.4584710349512853},
    {RUCKIG_WORKING, 2.4222272872301414},
    {RUCKIG_WORKING, 2.1047787855405429},
    {RUCKIG_WORKING, 2.4657343587793634},
    {RUCKIG_WORKING, 3.1531469118030202},
    {RUCKIG_WORKING, 2.6061641302819289},
    {RUCKIG_WORKING, 1.9239972254420541},
    {RUCKIG_WORKING, 2.0558384224789195},
    {RUCKIG_WORKING, 2.2248443577610386},
    {RUCKIG_WORKING, 2.3223942208462334},
    {RUCKIG_WORKING, 2.0283780734336379},
    {RUCKIG_WORKING, 2.2574411257868867},
    {RUCKIG_WORKING, 2.4644611096425031},
    {RUCKIG_WORKING, 2.4457278825371329},
    {RUCKIG_WORKING, 2.0848511936006613},
    {RUCKIG_WORKING, 2.6054809401039547},
    {RUCKIG_WORKING, 3.0800045689903248},
    {RUCKIG_WORKING, 2.8832218947413302},
    {RUCKIG_WORKING, 1.9160505589883272},
    {RUCKIG_WORKING, 2.0760672687280288},
    {RUCKIG_WORKING, 2.1888431138892592},
    {RUCKIG_WORKING, 2.2999908098908737},
    {RUCKIG_WORKING, 2.0038814140165444},
    {RUCKIG_WORKING, 2.3860493642534069},
    {RUCKIG_WORKING, 2.557441802528555},
    {RUCKIG_WORKING, 2.23899929243351},
    {RUCKIG_WORKING, 2.0428579478127338},
    {RUCKIG_WORKING, 2.3215913079831605},
    {RUCKIG_WORKING, 3.4106228864090751},
    {RUCKIG_WORKING, 2.8149811574204939},
    {RUCKIG_WORKING, 1.9318541856703626},
    {RUCKIG_WORKING, 2.0522195091848081},
    {RUCKIG_WORKING, 2.0631838839490344},
    {RUCKIG_WORKING, 2.3900689223057645},
    {RUCKIG_WORKING, 2.0494105941215643},
    {RUCKIG_WORKING, 2.5094798441135246},
    {RUCKIG_WORKING, 2.2752288475778109},
    {RUCKIG_WORKING, 2.5289410155537513},
    {RUCKIG_WORKING, 2.0357220222917145},
    {RUCKIG_WORKING, 2.0774397928966808},
    {RUCKIG_WORKING, 3.0343783763481187},
    {RUCKIG_WORKING, 2.7125519658727493},
    {RUCKIG_WORKING, 1.9442812786899282},
    {RUCKIG_WORKING, 2.069495314403671},
    {RUCKIG_WORKING, 2.2236904761904763},
    {RUCKIG_WORKING, 2.3257881283373774},
    {RUCKIG_WORKING, 2.0353605455021526},
    {RUCKIG_WORKING, 2.4109810151649858},
    {RUCKIG_WORKING, 2.4176495882389277},
    {RUCKIG_WORKING, 2.4860267003990359},
    {RUCKIG_WORKING, 2.0510236404242619},
    {RUCKIG_WORKING, 2.6080382297388756},
    {RUCKIG_WORKING, 3.091791134072011},
    {RUCKIG_WORKING, 2.903038931573815},
    {RUCKIG_WORKING, 1.9702790260533687},
    {RUCKIG_WORKING, 2.083353074399418},
    {RUCKIG_WORKING, 2.1970749992119081},
    {RUCKIG_WORKING, 2.4716791979949875},
    {RUCKIG_WORKING, 1.978703111129096},
    {RUCKIG_WORKING, 2.0772403453156212},
    {RUCKIG_WORKING, 2.3591172672337084},
    {RUCKIG_WORKING, 2.3636957636010121},
    {RUCKIG_WORKING, 2.0532460636728844},
    {RUCKIG_WORKING, 2.5913216388654354},
    {RUCKIG_WORKING, 3.3428153457599308},
    {RUCKIG_WORKING, 2.54395314189289},
    {RUCKIG_WORKING, 1.9754432000429079},
    {RUCKIG_WORKING, 2.0936269995027805},
    {RUCKIG_WORKING, 2.2964285714285708},
    {RUCKIG_WORKING, 2.2466345402602599},
    {RUCKIG_WORKING, 1.9142690602950223},
    {RUCKIG_WORKING, 2.177528236611459},
    {RUCKIG_WORKING, 2.3408605072052615},
    {RUCKIG_WORKING, 2.5460348885692179},
    {RUCKIG_WORKING, 2.0512234950866777},
    {RUCKIG_WORKING, 2.7281909321239359},
    {RUCKIG_WORKING, 3.1320961226633219},
    {RUCKIG_WORKING, 2.8955176361553034},
    {RUCKIG_WORKING, 1.9944569522369353},
    {RUCKIG_WORKING, 2.1238095238095238},
    {RUCKIG_WORKING, 2.1104950014176227},
    {RUCKIG_WORKING, 2.2762819997371411},
    {RUCKIG_WORKING, 1.9421733108693535},
    {RUCKIG_WORKING, 2.4233387413110896},
    {RUCKIG_WORKING, 2.2582695351593518},
    {RUCKIG_WORKING, 2.4168334927430806},
    {RUCKIG_WORKING, 2.0789756193223998},
    {RUCKIG_WORKING, 2.5650590147225167},
    {RUCKIG_WORKING, 3.335222010769173},
    {RUCKIG_WORKING, 2.8060429882502538},
    {RUCKIG_WORKING, 1.9847952137456903},
    {RUCKIG_WORKING, 1.9957062475352765},
    {RUCKIG_WORKING, 2.1236601897753999},
    {RUCKIG_WORKING, 2.3097117794486217},
    {RUCKIG_WORKING, 1.998232074574996},
    {RUCKIG_WORKING, 2.2494421209405626},
    {RUCKIG_WORKING, 2.3453201943745858},
    {RUCKIG_WORKING, 2.5133170309898056}
};

static void configure_waypoint_resume_quality_case(
    ruckig_input_t* input,
    size_t case_id,
    double* waypoints_out
) {
    const size_t dofs = input->dofs;
    const size_t waypoint_count = input->max_number_of_waypoints;
    const size_t section_count = waypoint_count + 1;
    double waypoints[12] = {0.0};
    double per_section_min_velocity[16] = {0.0};
    double per_section_max_velocity[16] = {0.0};
    double per_section_min_acceleration[16] = {0.0};
    double per_section_max_acceleration[16] = {0.0};
    double per_section_max_jerk[16] = {0.0};
    double per_section_min_position[16] = {0.0};
    double per_section_max_position[16] = {0.0};
    double per_section_minimum_duration[4] = {0.0};
    size_t waypoint;
    size_t section;
    size_t dof;

    CHECK_EQ_INT(ruckig_input_set_synchronization(
        input,
        (ruckig_synchronization_t)(case_id % 4u)), RUCKIG_WORKING);

    for (dof = 0; dof < dofs; ++dof) {
        const bool disabled = dofs > 1 && ((case_id + dof * 7u) % 19u == 0u);
        const double sign = ((case_id + dof) % 2u) == 0u ? 1.0 : -1.0;
        const double distance = disabled ? 0.0 : sign * (0.85 + 0.18 * (double)dof + 0.015 * (double)(case_id % 11u));
        ruckig_input_current_position_data(input)[dof] = 0.0;
        ruckig_input_current_velocity_data(input)[dof] = 0.0;
        ruckig_input_current_acceleration_data(input)[dof] = 0.0;
        ruckig_input_target_position_data(input)[dof] = distance;
        ruckig_input_target_velocity_data(input)[dof] = 0.0;
        ruckig_input_target_acceleration_data(input)[dof] = 0.0;
        ruckig_input_max_velocity_data(input)[dof] = 1.05 + 0.07 * (double)((case_id + dof) % 5u);
        ruckig_input_max_acceleration_data(input)[dof] = 1.80 + 0.10 * (double)((case_id + 2u * dof) % 4u);
        ruckig_input_max_jerk_data(input)[dof] = 4.20 + 0.20 * (double)((case_id + 3u * dof) % 4u);
        ruckig_input_max_position_data(input)[dof] = distance >= 0.0 ? distance + 0.75 : 0.75;
        ruckig_input_min_position_data(input)[dof] = distance >= 0.0 ? -0.75 : distance - 0.75;
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, dof, !disabled), RUCKIG_WORKING);
    }

    for (waypoint = 0; waypoint < waypoint_count; ++waypoint) {
        const double fraction = (double)(waypoint + 1u) / (double)(waypoint_count + 1u);
        for (dof = 0; dof < dofs; ++dof) {
            const bool enabled = ruckig_input_enabled_const_data(input)[dof];
            const double target = ruckig_input_target_position_const_data(input)[dof];
            const double curvature = enabled
                ? 0.025 * (double)(((case_id + waypoint * 3u + dof) % 3u) + 1u)
                    * (target >= 0.0 ? 1.0 : -1.0)
                : 0.0;
            double value = target * fraction + curvature * sin((double)(waypoint + 1u));
            if (fabs(value) > fabs(target) && target != 0.0) {
                value = target * fraction;
            }
            waypoints[waypoint * dofs + dof] = enabled ? value : 0.0;
        }
    }
    CHECK_EQ_INT(ruckig_input_set_intermediate_positions(input, waypoints, waypoint_count, dofs), RUCKIG_WORKING);
    if (waypoints_out) {
        memcpy(waypoints_out, waypoints, sizeof(double) * waypoint_count * dofs);
    }

    for (section = 0; section < section_count; ++section) {
        const double start_fraction = (double)section / (double)section_count;
        const double end_fraction = (double)(section + 1u) / (double)section_count;
        per_section_minimum_duration[section] = 0.04 + 0.01 * (double)((case_id + section) % 3u);
        for (dof = 0; dof < dofs; ++dof) {
            const size_t index = section * dofs + dof;
            const double target = ruckig_input_target_position_const_data(input)[dof];
            const double lo = target >= 0.0 ? target * start_fraction - 0.55 : target * end_fraction - 0.55;
            const double hi = target >= 0.0 ? target * end_fraction + 0.55 : target * start_fraction + 0.55;
            const double max_velocity = ruckig_input_max_velocity_const_data(input)[dof];
            const double max_acceleration = ruckig_input_max_acceleration_const_data(input)[dof];
            per_section_min_velocity[index] = -max_velocity;
            per_section_max_velocity[index] = max_velocity;
            per_section_min_acceleration[index] = -max_acceleration;
            per_section_max_acceleration[index] = max_acceleration;
            per_section_max_jerk[index] = ruckig_input_max_jerk_const_data(input)[dof];
            per_section_min_position[index] = lo;
            per_section_max_position[index] = hi;
        }
    }

    if ((case_id % 2u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 3u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 5u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 7u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_min_position(input, per_section_min_position, section_count, dofs), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_set_per_section_max_position(input, per_section_max_position, section_count, dofs), RUCKIG_WORKING);
    }
    if ((case_id % 11u) == 0u) {
        CHECK_EQ_INT(ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, section_count), RUCKIG_WORKING);
    }
}

static ruckig_result_t waypoint_resume_quality_calculate_case(
    size_t case_id,
    ruckig_trajectory_t** trajectory_out
) {
    const size_t dofs = 1u + (case_id % 4u);
    const size_t waypoint_count = 1u + ((case_id / 4u) % 3u);
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, waypoint_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, dofs, waypoint_count), RUCKIG_WORKING);
    configure_waypoint_resume_quality_case(input, case_id, NULL);

    result = ruckig_calculate(otg, input, trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    if (trajectory_out) {
        *trajectory_out = trajectory;
    } else {
        ruckig_trajectory_destroy(trajectory);
    }
    return result;
}

void run_waypoint_resume_quality_baseline_dump(void) {
    size_t case_id;
    printf("static const waypoint_resume_quality_baseline_t waypoint_resume_quality_baseline[WAYPOINT_RESUME_QUALITY_CASES] = {\n");
    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; ++case_id) {
        ruckig_trajectory_t* trajectory = NULL;
        const ruckig_result_t result = waypoint_resume_quality_calculate_case(case_id, &trajectory);
        const double duration = result == RUCKIG_WORKING ? ruckig_trajectory_get_duration(trajectory) : -1.0;
        printf("    {%s, %.17g}%s\n",
            result == RUCKIG_WORKING ? "RUCKIG_WORKING" : "RUCKIG_ERROR",
            duration,
            case_id + 1u == WAYPOINT_RESUME_QUALITY_CASES ? "" : ",");
        ruckig_trajectory_destroy(trajectory);
    }
    printf("};\n");
}

void run_waypoint_resume_quality_audit_tests(void) {
    double max_regression = 0.0;
    double sum_ratio = 0.0;
    size_t successful_cases = 0;
    size_t publish_count = 0;
    size_t interrupted_without_publish_count = 0;
    size_t completion_count = 0;
    size_t fresh_reference_count = 0;
    size_t case_id;

    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; ++case_id) {
        ruckig_trajectory_t* trajectory = NULL;
        const ruckig_result_t result = waypoint_resume_quality_calculate_case(case_id, &trajectory);
        const waypoint_resume_quality_baseline_t baseline = waypoint_resume_quality_baseline[case_id];
        CHECK_EQ_INT(result, baseline.result);
        if (result == RUCKIG_WORKING) {
            const double duration = ruckig_trajectory_get_duration(trajectory);
            const double regression = duration - baseline.duration;
            CHECK_TRUE(duration > 0.0);
            CHECK_TRUE(duration <= baseline.duration + 1.0e-9);
            if (regression > max_regression) {
                max_regression = regression;
            }
            sum_ratio += duration / baseline.duration;
            ++successful_cases;
        }
        ruckig_trajectory_destroy(trajectory);
    }

    for (case_id = 0; case_id < WAYPOINT_RESUME_QUALITY_CASES; case_id += 4u) {
        const size_t dofs = 1u + (case_id % 4u);
        const size_t waypoint_count = 1u + ((case_id / 4u) % 3u);
        ruckig_t* otg = NULL;
        ruckig_t* fresh_otg = NULL;
        ruckig_input_t* input = NULL;
        ruckig_input_t* fresh_input = NULL;
        ruckig_output_t* output = NULL;
        ruckig_trajectory_t* fresh_trajectory = NULL;
        size_t cycle;

        CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_create_with_waypoints(&fresh_otg, dofs, 0.01, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_input_create_with_waypoints(&fresh_input, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, dofs, waypoint_count), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&fresh_trajectory, dofs, waypoint_count), RUCKIG_WORKING);
        configure_waypoint_resume_quality_case(input, case_id, NULL);
        CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
        CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
        CHECK_TRUE(ruckig_output_new_calculation(output));
        CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
        CHECK_TRUE(otg->waypoint_engine.active);

        for (cycle = 0; cycle < 40; ++cycle) {
            const double previous_time = ruckig_output_get_time(output);
            const double incumbent_remaining_duration =
                ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) - previous_time;
            const bool was_active = otg->waypoint_engine.active;
            ruckig_output_pass_to_input(output, input);
            if (cycle == 5u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
            } else if (cycle == 6u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
            } else if (cycle == 20u) {
                CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
            }
            if (was_active) {
                CHECK_EQ_INT(ruckig_input_copy_state(input, fresh_input), RUCKIG_WORKING);
                ruckig_input_clear_interrupt_calculation_duration(fresh_input);
                if (ruckig_calculate(fresh_otg, fresh_input, fresh_trajectory) == RUCKIG_WORKING) {
                    CHECK_TRUE(ruckig_trajectory_get_duration(fresh_trajectory) > 0.0);
                    ++fresh_reference_count;
                }
            }
            CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
            CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);
            if (ruckig_output_new_calculation(output)) {
                CHECK_NEAR(ruckig_output_get_time(output), 0.01, 1e-12);
                CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output))
                    < incumbent_remaining_duration - 1.0e-12);
                ++publish_count;
            } else {
                CHECK_TRUE(ruckig_output_get_time(output) > previous_time);
                if (ruckig_output_was_calculation_interrupted(output)) {
                    ++interrupted_without_publish_count;
                }
            }
            if (was_active && !otg->waypoint_engine.active) {
                ++completion_count;
                break;
            }
        }

        ruckig_trajectory_destroy(fresh_trajectory);
        ruckig_output_destroy(output);
        ruckig_input_destroy(fresh_input);
        ruckig_input_destroy(input);
        ruckig_destroy(fresh_otg);
        ruckig_destroy(otg);
    }

    CHECK_TRUE(successful_cases > 0);
    CHECK_TRUE(publish_count > 0);
    CHECK_TRUE(interrupted_without_publish_count > 0);
    CHECK_TRUE(completion_count > 0);
    CHECK_TRUE(fresh_reference_count > 0);
    printf("waypoint resume quality audit: cases %zu successful %zu avg_ratio %.12g max_regression %.12g publish %zu interrupted_without_publish %zu completion %zu fresh_reference %zu\n",
        (size_t)WAYPOINT_RESUME_QUALITY_CASES,
        successful_cases,
        sum_ratio / (double)successful_cases,
        max_regression,
        publish_count,
        interrupted_without_publish_count,
        completion_count,
        fresh_reference_count);
}

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

static void check_tracking_diagnostics_common(
    const ruckig_tracking_t* tracking,
    const ruckig_tracking_diagnostics_t* diagnostics
);

static void check_tracking_output_sequence(
    const ruckig_tracking_output_sequence_t* outputs,
    size_t dofs,
    size_t count,
    double delta_time
);

static void configure_interrupt_boundary_no_waypoint_input(ruckig_input_t* input) {
    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_velocity_data(input)[0] = 0.0;
    ruckig_input_current_acceleration_data(input)[0] = 0.0;
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[0] = 0.0;
    ruckig_input_target_acceleration_data(input)[0] = 0.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;
}

static void test_interrupt_boundary_waypoint_update_remains_interruptible(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
    configure_soft_interruption_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(otg->waypoint_engine.last_candidate_evaluations, 1);
    CHECK_TRUE(otg->waypoint_engine.active);
    CHECK_TRUE(!otg->waypoint_engine.complete);
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_interrupt_boundary_no_waypoint_update_ignores_interrupt_and_clears_resume(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* waypoint_input = NULL;
    ruckig_output_t* waypoint_output = NULL;
    ruckig_input_t* no_waypoint_input = NULL;
    ruckig_output_t* no_waypoint_output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&waypoint_input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&waypoint_output, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&no_waypoint_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&no_waypoint_output, 1), RUCKIG_WORKING);

    configure_soft_interruption_waypoint_input(waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(waypoint_input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_update(otg, waypoint_input, waypoint_output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(waypoint_output));
    CHECK_TRUE(otg->waypoint_engine.active);

    configure_interrupt_boundary_no_waypoint_input(no_waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(no_waypoint_input, 0.0), RUCKIG_WORKING);
    result = ruckig_update(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_pass_to_input(no_waypoint_output, no_waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(no_waypoint_input, 1000000000.0), RUCKIG_WORKING);
    result = ruckig_update(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_pass_to_input(no_waypoint_output, no_waypoint_input);
    ruckig_input_clear_interrupt_calculation_duration(no_waypoint_input);
    result = ruckig_update(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(no_waypoint_output);
    ruckig_input_destroy(no_waypoint_input);
    ruckig_output_destroy(waypoint_output);
    ruckig_input_destroy(waypoint_input);
    ruckig_destroy(otg);
}

static void test_interrupt_boundary_calculate_clears_waypoint_resume(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&output, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1), RUCKIG_WORKING);
    configure_soft_interruption_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_update(otg, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(otg->waypoint_engine.active);

    CHECK_EQ_INT(ruckig_calculate(otg, input, trajectory), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_trajectory_get_duration(trajectory) > 0.0);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_interrupt_boundary_tracking_ignores_interrupt(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);

    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_update(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.25), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    fill_tracking_input_1d(input);
    ruckig_input_clear_interrupt_calculation_duration(input);
    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, count);
    check_tracking_diagnostics_common(tracking, &diagnostics);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_interrupt_boundary_allocation_guard(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* waypoint_input = NULL;
    ruckig_output_t* waypoint_output = NULL;
    ruckig_input_t* no_waypoint_input = NULL;
    ruckig_output_t* no_waypoint_output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create_with_waypoints(&otg, 1, 0.05, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create_with_waypoints(&waypoint_input, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create_with_waypoints(&waypoint_output, 1, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&no_waypoint_input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&no_waypoint_output, 1), RUCKIG_WORKING);
    configure_soft_interruption_waypoint_input(waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(waypoint_input, 0.0), RUCKIG_WORKING);

    ruckig_allocation_counters_reset();
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, waypoint_input, waypoint_output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_new_calculation(waypoint_output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(waypoint_output));
    CHECK_TRUE(otg->waypoint_engine.active);

    ruckig_output_pass_to_input(waypoint_output, waypoint_input);
    otg->waypoint_engine.best_duration = -1.0;
    CHECK_EQ_INT(ruckig_update_under_allocation_guard(otg, waypoint_input, waypoint_output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_new_calculation(waypoint_output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(waypoint_output));
    CHECK_TRUE(otg->waypoint_engine.active);

    configure_interrupt_boundary_no_waypoint_input(no_waypoint_input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(no_waypoint_input, 0.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, no_waypoint_input, no_waypoint_output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(no_waypoint_output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(no_waypoint_output);
    ruckig_input_destroy(no_waypoint_input);
    ruckig_output_destroy(waypoint_output);
    ruckig_input_destroy(waypoint_input);
    ruckig_destroy(otg);
}

static void test_no_waypoint_interrupt_first_solve_publishes_complete_candidate(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) > 0.0);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_no_waypoint_interrupt_preserves_incumbent_at_boundary(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;
    double incumbent_duration;
    double old_time;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);

    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    incumbent_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));
    old_time = ruckig_output_get_time(output);
    CHECK_TRUE(incumbent_duration > old_time);

    ruckig_output_pass_to_input(output, input);
    ruckig_input_target_position_data(input)[0] = 1.8;
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) == incumbent_duration);
    CHECK_TRUE(ruckig_output_get_time(output) > old_time);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static void test_no_waypoint_interrupt_budget_matrix_and_clear(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;
    double interrupted_duration;

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.05), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    configure_interrupt_boundary_no_waypoint_input(input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);

    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));

    ruckig_output_pass_to_input(output, input);
    ruckig_input_target_position_data(input)[0] = 1.6;
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!ruckig_output_new_calculation(output));
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    interrupted_duration = ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output));

    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(ruckig_trajectory_get_duration(ruckig_output_get_trajectory(output)) != interrupted_duration);

    ruckig_output_pass_to_input(output, input);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1.0), RUCKIG_WORKING);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_pass_to_input(output, input);
    ruckig_input_clear_interrupt_calculation_duration(input);
    result = ruckig_update_under_allocation_guard(otg, input, output);
    CHECK_TRUE(result == RUCKIG_WORKING || result == RUCKIG_FINISHED);
    CHECK_TRUE(ruckig_output_new_calculation(output));
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_TRUE(!otg->waypoint_engine.active);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

static ruckig_result_t tracking_update_under_allocation_guard(
    ruckig_tracking_t* tracking,
    ruckig_target_state_t* target,
    ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_tracking_update(tracking, target, input, output);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

static ruckig_result_t tracking_update_with_lookahead_under_allocation_guard(
    ruckig_tracking_t* tracking,
    ruckig_target_state_sequence_t* targets,
    ruckig_input_t* input,
    ruckig_output_t* output
) {
    ruckig_result_t result;
    const size_t allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);
    result = ruckig_tracking_update_with_lookahead(tracking, targets, input, output);
    ruckig_allocation_forbidden_set(false);
    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    return result;
}

static void test_tracking_interrupt_fast_mode_single_candidate_not_interrupted(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_FAST), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(tracking_update_under_allocation_guard(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.budget_exhausted_count, 0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_interrupt_optimized_best_so_far_update(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_create(&target, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    fill_tracking_target_ramp(target, 0.0);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, 4), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(tracking_update_under_allocation_guard(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.calculation_status, RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, 1);
    CHECK_TRUE(diagnostics.budget_exhausted_count > 0);

    ruckig_output_pass_to_input(output, input);
    fill_tracking_target_ramp(target, 0.01);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0), RUCKIG_WORKING);
    CHECK_EQ_INT(tracking_update_under_allocation_guard(tracking, target, input, output), RUCKIG_WORKING);
    CHECK_TRUE(!ruckig_output_was_calculation_interrupted(output));
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(diagnostics.candidate_count > 1);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_interrupt_optimized_lookahead(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 2, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(tracking_update_with_lookahead_under_allocation_guard(tracking, targets, input, output), RUCKIG_WORKING);
    CHECK_TRUE(ruckig_output_was_calculation_interrupted(output));
    check_tracking_output_constraints(output, input, 1);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_EQ_INT(diagnostics.candidate_count, 1);
    CHECK_EQ_INT(diagnostics.valid_candidate_count, 1);
    CHECK_TRUE(diagnostics.budget_exhausted_count > 0);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
}

static void test_tracking_interrupt_sequence_remains_deferred(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    ruckig_tracking_diagnostics_t diagnostics;
    const size_t count = 4;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, 1, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&targets, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_output_sequence_create(&outputs, 1, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    fill_tracking_input_1d(input);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(targets, count), RUCKIG_WORKING);
    set_tracking_sequence_signal(targets, 0, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_max_optimized_candidates(tracking, 16), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_set_interrupt_calculation_duration(input, 0.0), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_tracking_calculate_sequence(tracking, targets, input, outputs), RUCKIG_WORKING);
    check_tracking_output_sequence(outputs, 1, count, 0.01);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &diagnostics), RUCKIG_WORKING);
    CHECK_TRUE(diagnostics.candidate_count >= count);

    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
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

static size_t tracking_random_audit_pick(unsigned* state, size_t count) {
    return (size_t)((tracking_random_next(state) >> 8u) % (unsigned)count);
}

static bool tracking_random_audit_bool(unsigned* state) {
    return ((tracking_random_next(state) >> 8u) & 1u) != 0u;
}

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
    const tracking_audit_case_result_t* case_result
) {
    const tracking_audit_case_config_t* config = &case_result->config;
    const ruckig_tracking_diagnostics_t* diagnostics = &case_result->diagnostics;
    const char* selected_family = case_result->selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT
        ? tracking_audit_family_names[case_result->selected_family]
        : "unknown";
    printf(
        "tracking random audit fallback_case reason=%s sample=%zu strategy=%s dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f disabled=%s disabled_dof=%s constraints=%s status=%s selected_family=%s selected_near_tie=%s strict_improved=%zu near_tie_accepted=%zu candidates=%zu fast=%zu instantaneous=%zu horizon=%zu terminal_blend=%zu derivative_damped=%zu lead_lag=%zu budget_exhausted=%zu fast_score=%.9g best_score=%.9g improvement=%.9g\n",
        reason,
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
        tracking_audit_print_case(representatives->reasons[i], &representatives->cases[i]);
    }
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

void run_tracking_random_audit_tests(size_t samples, unsigned seed) {
    const size_t dof_values[4] = {1, 2, 4, 8};
    const size_t lookahead_values[4] = {1, 2, 5, 10};
    const double reactiveness_values[4] = {0.0, 0.25, 0.5, 1.0};
    const ruckig_tracking_optimized_strategy_t strategy_values[3] = {
        RUCKIG_TRACKING_OPTIMIZED_STABLE,
        RUCKIG_TRACKING_OPTIMIZED_BALANCED,
        RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE
    };
    tracking_audit_stats_t stats;
    tracking_audit_representatives_t representatives;
    size_t sample;
    unsigned state = seed ? seed : 1u;

    memset(&stats, 0, sizeof(stats));
    memset(&representatives, 0, sizeof(representatives));

    for (sample = 0; sample < samples; ++sample) {
        tracking_audit_case_config_t config;
        tracking_audit_case_result_t case_result;
        memset(&config, 0, sizeof(config));
        config.sample_index = sample;
        config.dofs = dof_values[tracking_random_audit_pick(&state, 4)];
        config.lookahead_count = lookahead_values[tracking_random_audit_pick(&state, 4)];
        config.signal = (int)tracking_random_audit_pick(&state, 4);
        config.reactiveness = reactiveness_values[tracking_random_audit_pick(&state, 4)];
        config.strategy = strategy_values[tracking_random_audit_pick(&state, 3)];
        config.start_time = (double)(sample % 200u) * 0.01;
        config.has_disabled_dof = false;
        config.disabled_dof = 0;
        if (config.dofs > 1 && tracking_random_audit_bool(&state)) {
            config.has_disabled_dof = true;
            config.disabled_dof = tracking_random_audit_pick(&state, config.dofs);
        }
        config.tight_constraints = tracking_random_audit_bool(&state);

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

void run_tracking_quality_hardening_tests(void) {
    test_tracking_random_audit_fixed_cases();
    run_tracking_random_audit_tests(10000, 1u);
}

void run_tracking_stability_tests(void) {
    test_tracking_stability_regression_cases();
}

void run_waypoint_tests(void) {
    test_waypoint_constructors_storage_and_optimizer();
    test_waypoint_validation_and_filter();
    test_waypoint_soft_interruption_update();
    test_waypoint_soft_interruption_alpha2_hardening();
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

void run_waypoint_resume_stress_tests(void) {
    test_waypoint_resume_stress_budget_matrix();
    test_waypoint_resume_stress_long_online_loop();
    test_waypoint_resume_stress_allocation_paths();
}

void run_waypoint_resume_quality_tests(void) {
    run_waypoint_resume_quality_audit_tests();
}

void run_interrupt_boundary_audit_tests(void) {
    test_interrupt_boundary_waypoint_update_remains_interruptible();
    test_interrupt_boundary_no_waypoint_update_ignores_interrupt_and_clears_resume();
    test_interrupt_boundary_calculate_clears_waypoint_resume();
    test_interrupt_boundary_tracking_ignores_interrupt();
    test_interrupt_boundary_allocation_guard();
}

void run_no_waypoint_interrupt_audit_tests(void) {
    test_no_waypoint_interrupt_first_solve_publishes_complete_candidate();
    test_no_waypoint_interrupt_preserves_incumbent_at_boundary();
    test_no_waypoint_interrupt_budget_matrix_and_clear();
}

void run_tracking_interrupt_audit_tests(void) {
    test_tracking_interrupt_fast_mode_single_candidate_not_interrupted();
    test_tracking_interrupt_optimized_best_so_far_update();
    test_tracking_interrupt_optimized_lookahead();
    test_tracking_interrupt_sequence_remains_deferred();
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
    test_tracking_random_audit_fixed_cases();
    test_tracking_stability_regression_cases();
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
