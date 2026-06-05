#include <math.h>
#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    const size_t dofs = 3;
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double waypoints[6] = {
        0.5, -0.2, 0.25,
        1.0, -0.4, 0.50
    };
    double position[3] = {0.0, 0.0, 0.0};
    double durations[2] = {0.0, 0.0};

    if (ruckig_create_with_waypoints(&otg, dofs, 0.02, 2) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, dofs, 2) != RUCKIG_WORKING
        || ruckig_trajectory_create_with_waypoints(&trajectory, dofs, 2) != RUCKIG_WORKING) {
        return 1;
    }

    for (size_t i = 0; i < dofs; ++i) {
        ruckig_input_max_velocity_data(input)[i] = 1.5;
        ruckig_input_max_acceleration_data(input)[i] = 2.0;
        ruckig_input_max_jerk_data(input)[i] = 4.0;
    }
    ruckig_input_target_position_data(input)[0] = 1.5;
    ruckig_input_target_position_data(input)[1] = -0.6;
    ruckig_input_target_position_data(input)[2] = 0.75;

    if (ruckig_input_set_intermediate_positions(input, waypoints, 2, dofs) != RUCKIG_WORKING
        || ruckig_calculate(otg, input, trajectory) != RUCKIG_WORKING
        || ruckig_trajectory_get_intermediate_durations(trajectory, durations, 2) != RUCKIG_WORKING) {
        return 1;
    }

    if (ruckig_trajectory_at_time(trajectory, durations[1], position, NULL, NULL, NULL, NULL) != RUCKIG_WORKING
        || fabs(position[0] - waypoints[3]) > 1e-7
        || fabs(position[1] - waypoints[4]) > 1e-7
        || fabs(position[2] - waypoints[5]) > 1e-7) {
        return 1;
    }

    printf("dynamic dofs waypoint duration: %.6f\n", ruckig_trajectory_get_duration(trajectory));

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
