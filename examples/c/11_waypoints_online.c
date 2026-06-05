#include <math.h>
#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    double waypoint[1] = {1.0};
    ruckig_result_t result = RUCKIG_WORKING;
    bool saw_section_change = false;
    size_t guard = 0;

    if (ruckig_create_with_waypoints(&otg, 1, 0.05, 1) != RUCKIG_WORKING
        || ruckig_input_create_with_waypoints(&input, 1, 1) != RUCKIG_WORKING
        || ruckig_output_create_with_waypoints(&output, 1, 1) != RUCKIG_WORKING) {
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 2.0;
    ruckig_input_max_velocity_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 4.0;

    if (ruckig_input_set_intermediate_positions(input, waypoint, 1, 1) != RUCKIG_WORKING) {
        return 1;
    }

    while (result == RUCKIG_WORKING && guard < 200) {
        result = ruckig_update(otg, input, output);
        if (result < 0) {
            break;
        }
        saw_section_change = saw_section_change || ruckig_output_did_section_change(output);
        ruckig_output_pass_to_input(output, input);
        ++guard;
    }

    if (result != RUCKIG_FINISHED
        || !saw_section_change
        || fabs(ruckig_output_new_position_data(output)[0] - 2.0) > 1e-7
        || fabs(ruckig_output_new_velocity_data(output)[0]) > 1e-7) {
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    printf("finished at %.6f in section %zu\n", ruckig_output_get_time(output), ruckig_output_get_new_section(output));

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
