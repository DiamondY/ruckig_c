#include <stdio.h>
#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result = RUCKIG_WORKING;
    size_t steps = 0;

    if (ruckig_create(&otg, 3, 0.005) != RUCKIG_WORKING) {
        return 1;
    }
    if (ruckig_input_create(&input, 3) != RUCKIG_WORKING) {
        ruckig_destroy(otg);
        return 1;
    }
    if (ruckig_output_create(&output, 3) != RUCKIG_WORKING) {
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_input_current_position_data(input)[0] = 0.0;
    ruckig_input_current_position_data(input)[1] = 0.0;
    ruckig_input_current_position_data(input)[2] = 0.0;
    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_position_data(input)[1] = -0.5;
    ruckig_input_target_position_data(input)[2] = 0.25;
    ruckig_input_max_velocity_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[1] = 1.0;
    ruckig_input_max_velocity_data(input)[2] = 1.0;

    while (result == RUCKIG_WORKING && steps < 1000) {
        result = ruckig_update(otg, input, output);
        if (result < 0) {
            fprintf(stderr, "update failed: %d\n", (int)result);
            break;
        }
        printf("%.3f %.6f %.6f\n",
            ruckig_output_get_time(output),
            ruckig_output_new_position_data(output)[0],
            ruckig_output_new_velocity_data(output)[0]);
        ruckig_output_pass_to_input(output, input);
        ++steps;
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return result < 0 ? 1 : 0;
}
