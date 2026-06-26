#include "test_api_internal.h"

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

void run_public_trajectory_api_tests(void) {
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
}

void run_public_trajectory_post_tracking_tests(void) {
    test_position_third_order_nonzero_target_velocity();
}
