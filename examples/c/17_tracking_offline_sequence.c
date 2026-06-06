#include <math.h>
#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    const size_t sample_count = 200;
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* targets = NULL;
    ruckig_tracking_output_sequence_t* outputs = NULL;
    ruckig_input_t* input = NULL;
    double* position;
    double* velocity;
    double* acceleration;
    const double* output_position;
    size_t step;

    if (ruckig_tracking_create(&tracking, 1, 0.01) != RUCKIG_WORKING
        || ruckig_target_state_sequence_create(&targets, 1, sample_count) != RUCKIG_WORKING
        || ruckig_tracking_output_sequence_create(&outputs, 1, sample_count) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING) {
        return 1;
    }

    ruckig_input_max_velocity_data(input)[0] = 4.0;
    ruckig_input_max_acceleration_data(input)[0] = 5.0;
    ruckig_input_max_jerk_data(input)[0] = 15.0;

    if (ruckig_target_state_sequence_set_count(targets, sample_count) != RUCKIG_WORKING) {
        return 2;
    }
    position = ruckig_target_state_sequence_position_data(targets);
    velocity = ruckig_target_state_sequence_velocity_data(targets);
    acceleration = ruckig_target_state_sequence_acceleration_data(targets);
    for (step = 0; step < sample_count; ++step) {
        const double t = (double)step * ruckig_tracking_get_delta_time(tracking);
        const double ramp_velocity = 1.2;
        if (t < 2.0) {
            position[step] = sin(ramp_velocity * t);
            velocity[step] = ramp_velocity * cos(ramp_velocity * t);
            acceleration[step] = -ramp_velocity * ramp_velocity * sin(ramp_velocity * t);
        } else {
            position[step] = 0.0;
            velocity[step] = 0.0;
            acceleration[step] = 0.0;
        }
    }

    if (ruckig_tracking_calculate_sequence(tracking, targets, input, outputs) != RUCKIG_WORKING) {
        return 3;
    }
    output_position = ruckig_tracking_output_sequence_new_position_const_data(outputs);
    printf("offline tracking samples: %zu final position: %.6f\n",
        ruckig_tracking_output_sequence_get_count(outputs),
        output_position[sample_count - 1]);

    ruckig_input_destroy(input);
    ruckig_tracking_output_sequence_destroy(outputs);
    ruckig_target_state_sequence_destroy(targets);
    ruckig_tracking_destroy(tracking);
    return 0;
}
