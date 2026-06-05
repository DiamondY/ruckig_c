#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoint[1] = {1.0};
    double per_section_minimum_duration[2] = {2.0, 1.0};
    double intermediate_duration[1] = {0.0};
    ruckig_result_t result;

    if (ruckig_create_with_waypoints(&otg, 1, 0.01, 1) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, 1, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create_with_waypoints(&trajectory, 1, 1) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.5;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    ruckig_input_max_position_data(input)[0] = 3.0;
    ruckig_input_min_position_data(input)[0] = -1.0;

    if (ruckig_input_set_intermediate_positions(input, waypoint, 1, 1) != RUCKIG_WORKING
        || ruckig_input_set_per_section_minimum_duration(input, per_section_minimum_duration, 2) != RUCKIG_WORKING) {
        return 1;
    }

    result = ruckig_calculate(otg, input, trajectory);
    if (result != RUCKIG_WORKING) {
        return 1;
    }
    if (ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1) != RUCKIG_WORKING) {
        return 1;
    }
    if (intermediate_duration[0] < 2.0 || ruckig_trajectory_get_duration(trajectory) < 3.0) {
        return 1;
    }

    printf("duration: %.6f\n", ruckig_trajectory_get_duration(trajectory));

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
