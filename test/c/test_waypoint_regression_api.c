#include "test_api_internal.h"

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


void run_waypoint_all_regression_tests(void) {
    test_waypoint_fixed_regression_corpus();
    test_waypoint_alpha2_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
    test_waypoint_alpha2_quality_regressions();
}

void run_waypoint_per_section_regression_tests(void) {
    test_waypoint_alpha2_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
}

void run_waypoint_quality_regression_tests(void) {
    test_waypoint_fixed_regression_corpus();
    test_waypoint_041_deep_regression_corpus();
    test_waypoint_alpha2_quality_regressions();
}
