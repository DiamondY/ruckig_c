#include <math.h>
#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoint[2] = {1.0, -0.5};
    double position[2] = {0.0, 0.0};
    double velocity[2] = {0.0, 0.0};
    double acceleration[2] = {0.0, 0.0};
    double intermediate_duration[1] = {0.0};
    ruckig_result_t result;

    if (ruckig_create_with_waypoints(&otg, 2, 0.01, 1) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, 2, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create_with_waypoints(&trajectory, 2, 1) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_target_position_data(input)[1] = -1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[1] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    ruckig_input_max_jerk_data(input)[1] = 4.0;
    ruckig_input_max_position_data(input)[0] = 3.0;
    ruckig_input_max_position_data(input)[1] = 1.0;
    ruckig_input_min_position_data(input)[0] = -1.0;
    ruckig_input_min_position_data(input)[1] = -2.0;

    if (ruckig_input_set_intermediate_positions(input, waypoint, 1, 2) != RUCKIG_WORKING) {
        return 1;
    }

    result = ruckig_calculate(otg, input, trajectory);
    if (result != RUCKIG_WORKING) {
        return 1;
    }
    if (ruckig_trajectory_get_intermediate_durations(trajectory, intermediate_duration, 1) != RUCKIG_WORKING) {
        return 1;
    }
    if (ruckig_trajectory_at_time(trajectory, intermediate_duration[0], position, velocity, acceleration, NULL, NULL) != RUCKIG_WORKING) {
        return 1;
    }
    if (fabs(position[0] - waypoint[0]) > 1e-7 || fabs(position[1] - waypoint[1]) > 1e-7) {
        return 1;
    }

    printf("duration: %.6f\n", ruckig_trajectory_get_duration(trajectory));

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
