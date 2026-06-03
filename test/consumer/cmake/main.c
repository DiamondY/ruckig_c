#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    int failed = 0;

    failed |= ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING;
    failed |= ruckig_input_create(&input, 1) != RUCKIG_WORKING;
    failed |= ruckig_trajectory_create(&trajectory, 1) != RUCKIG_WORKING;
    if (!failed) {
        ruckig_input_target_position_data(input)[0] = 1.0;
        ruckig_input_max_velocity_data(input)[0] = 1.0;
        failed |= ruckig_calculate(otg, input, trajectory) != RUCKIG_WORKING;
        failed |= ruckig_trajectory_at_time(trajectory, 1.0, position, NULL, NULL, NULL, NULL) != RUCKIG_WORKING;
        failed |= position[0] < 0.0 || position[0] > 1.0;
    }

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return failed ? 1 : 0;
}
