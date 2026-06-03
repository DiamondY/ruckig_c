#include <stdio.h>
#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_result_t result = RUCKIG_WORKING;
    size_t guard = 0;

    if (ruckig_create(&otg, 1, 0.1) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_output_create(&output, 1) != RUCKIG_WORKING) {
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_max_velocity_data(input)[0] = 1.0;

    while (result == RUCKIG_WORKING && guard < 100) {
        result = ruckig_update(otg, input, output);
        if (result < 0) {
            break;
        }
        printf("%.3f %.6f\n",
            ruckig_output_get_time(output),
            ruckig_output_new_position_data(output)[0]);
        ruckig_output_pass_to_input(output, input);
        ++guard;
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return result == RUCKIG_FINISHED ? 0 : 1;
}
