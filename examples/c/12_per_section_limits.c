#include <math.h>
#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
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

    if (ruckig_create_with_waypoints(&otg, 1, 0.01, 1) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, 1, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.5;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;

    if (ruckig_input_set_intermediate_positions(input, waypoint, 1, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_max_velocity(input, per_section_max_velocity, 2, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_min_velocity(input, per_section_min_velocity, 2, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_max_acceleration(input, per_section_max_acceleration, 2, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_min_acceleration(input, per_section_min_acceleration, 2, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_max_jerk(input, per_section_max_jerk, 2, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_max_position(input, per_section_max_position, 2, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_min_position(input, per_section_min_position, 2, 1) != RUCKIG_WORKING) {
        return 1;
    }

    if (ruckig_calculate(otg, input, trajectory) != RUCKIG_WORKING
        || ruckig_trajectory_get_position_extrema(trajectory, extrema, 1) != RUCKIG_WORKING) {
        return 1;
    }
    if (extrema[0].min_position < -1e-9 || extrema[0].max_position > 2.0 + 1e-9) {
        return 1;
    }

    printf("duration: %.6f extrema [%.6f, %.6f]\n",
        ruckig_trajectory_get_duration(trajectory),
        extrema[0].min_position,
        extrema[0].max_position);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
