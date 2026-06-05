#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    double waypoints[6] = {
        0.2, 0.1,
        1.0, 0.5,
        1.6, 1.4
    };
    double threshold[2] = {0.25, 0.25};
    double filtered[6] = {0.0};
    size_t written = 0;

    if (ruckig_create_with_waypoints(&otg, 2, 0.01, 3) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, 2, 3) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_target_position_data(input)[1] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[1] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;
    ruckig_input_max_jerk_data(input)[1] = 4.0;

    if (ruckig_input_set_intermediate_positions(input, waypoints, 3, 2) != RUCKIG_WORKING
        || ruckig_filter_intermediate_positions(otg, input, threshold, 2, filtered, 6, &written) != RUCKIG_WORKING) {
        return 1;
    }
    if (written > 3) {
        return 1;
    }

    printf("filtered waypoints: %zu\n", written);

    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
