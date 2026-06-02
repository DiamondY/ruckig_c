#include <stdio.h>
#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double velocity[1] = {0.0};
    double acceleration[1] = {0.0};

    if (ruckig_create(&otg, 1, 0.01) != RUCKIG_WORKING
        || ruckig_input_create(&input, 1) != RUCKIG_WORKING
        || ruckig_trajectory_create(&trajectory, 1) != RUCKIG_WORKING) {
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_input_set_control_interface(input, RUCKIG_CONTROL_VELOCITY);
    ruckig_input_target_velocity_data(input)[0] = 1.0;
    ruckig_input_max_acceleration_data(input)[0] = 1.0;
    ruckig_input_max_jerk_data(input)[0] = 1.0;

    if (ruckig_calculate(otg, input, trajectory) != RUCKIG_WORKING) {
        fprintf(stderr, "calculate failed\n");
        ruckig_trajectory_destroy(trajectory);
        ruckig_input_destroy(input);
        ruckig_destroy(otg);
        return 1;
    }

    ruckig_trajectory_at_time(trajectory, ruckig_trajectory_get_duration(trajectory), position, velocity, acceleration, NULL, NULL);
    printf("duration %.6f final %.6f %.6f %.6f\n",
        ruckig_trajectory_get_duration(trajectory),
        position[0],
        velocity[0],
        acceleration[0]);

    ruckig_trajectory_destroy(trajectory);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    return 0;
}
