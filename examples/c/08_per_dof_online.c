#include <stdio.h>
#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_control_interface_t control[2] = {RUCKIG_CONTROL_POSITION, RUCKIG_CONTROL_VELOCITY};
    ruckig_synchronization_t synchronization[2] = {RUCKIG_SYNCHRONIZATION_TIME, RUCKIG_SYNCHRONIZATION_NONE};
    ruckig_result_t result = RUCKIG_WORKING;
    size_t guard = 0;

    if (ruckig_create(&otg, 2, 0.05) != RUCKIG_WORKING
        || ruckig_input_create(&input, 2) != RUCKIG_WORKING
        || ruckig_output_create(&output, 2) != RUCKIG_WORKING) {
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    if (ruckig_input_set_per_dof_control_interface(input, control, 2) != RUCKIG_WORKING
        || ruckig_input_set_per_dof_synchronization(input, synchronization, 2) != RUCKIG_WORKING) {
        ruckig_output_destroy(output);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_input_target_position_data(input)[0] = 1.0;
    ruckig_input_target_velocity_data(input)[1] = 0.8;
    ruckig_input_max_velocity_data(input)[0] = 1.5;
    ruckig_input_max_acceleration_data(input)[0] = 1.2;
    ruckig_input_max_acceleration_data(input)[1] = 1.1;
    ruckig_input_max_jerk_data(input)[0] = 2.0;
    ruckig_input_max_jerk_data(input)[1] = 1.7;

    while (result == RUCKIG_WORKING && guard < 200) {
        result = ruckig_update(otg, input, output);
        if (result < 0) {
            break;
        }
        printf("%.3f %.6f %.6f %.6f %.6f\n",
            ruckig_output_get_time(output),
            ruckig_output_new_position_data(output)[0],
            ruckig_output_new_position_data(output)[1],
            ruckig_output_new_velocity_data(output)[0],
            ruckig_output_new_velocity_data(output)[1]);
        ruckig_output_pass_to_input(output, input);
        ++guard;
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return result == RUCKIG_FINISHED ? 0 : 1;
}
