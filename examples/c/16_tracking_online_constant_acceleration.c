#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_t* target = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    size_t step;

    if (ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_target_state_create(&target, 1) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_max_velocity_data(input)[0] = 2.0;
    ruckig_input_max_acceleration_data(input)[0] = 3.0;
    ruckig_input_max_jerk_data(input)[0] = 8.0;

    for (step = 0; step < 160; ++step) {
        const double t = (double)step * ruckig_tracking_get_delta_time(tracking);
        const double a = 0.05;
        ruckig_target_state_position_data(target)[0] = 0.5 * a * t * t;
        ruckig_target_state_velocity_data(target)[0] = a * t;
        ruckig_target_state_acceleration_data(target)[0] = a;
        if (ruckig_tracking_update(tracking, target, input, output) < 0) {
            return 2;
        }
        ruckig_output_pass_to_input(output, input);
    }

    printf("tracking constant-acceleration final position: %.6f\n", ruckig_output_new_position_data(output)[0]);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_destroy(target);
    ruckig_tracking_destroy(tracking);
    return 0;
}
