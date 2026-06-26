#include "test_api_internal.h"

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

void run_public_validation_api_tests(void) {
    test_validation();
    test_invalid_input_diagnostics();
    test_zero_limit_error_paths();
    test_finite_infinite_limit_semantics();
}
