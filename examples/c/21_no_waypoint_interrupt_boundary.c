#include <stdio.h>

#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result;

    if (ruckig_create(&otg, 1, 0.05) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING) {
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[0] = 5.0;

    if (ruckig_input_set_interrupt_calculation_duration(input, 1000000000.0) != RUCKIG_WORKING) {
        return 2;
    }

    result = ruckig_update(otg, input, output);
    if (result < 0 || !ruckig_output_new_calculation(output)
        || ruckig_output_was_calculation_interrupted(output)) {
        return 3;
    }

    ruckig_output_pass_to_input(output, input);
    ruckig_input_target_position_data(input)[0] = 1.8;
    if (ruckig_input_set_interrupt_calculation_duration(input, 0.0) != RUCKIG_WORKING) {
        return 4;
    }

    result = ruckig_update(otg, input, output);
    if (result < 0 || ruckig_output_new_calculation(output)
        || !ruckig_output_was_calculation_interrupted(output)) {
        return 5;
    }

    printf(
        "no-waypoint interrupt kept incumbent: time %.6f position %.6f\n",
        ruckig_output_get_time(output),
        ruckig_output_new_position_data(output)[0]
    );

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
