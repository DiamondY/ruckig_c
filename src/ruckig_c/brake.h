#ifndef RUCKIG_C_BRAKE_H
#define RUCKIG_C_BRAKE_H

typedef struct ruckig_brake_profile {
    double duration;
    double t[2];
    double j[2];
    double a[2];
    double v[2];
    double p[2];
} ruckig_brake_profile_t;

void ruckig_brake_profile_init(ruckig_brake_profile_t* brake);

void ruckig_brake_get_position_trajectory(
    ruckig_brake_profile_t* brake,
    double v0,
    double a0,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
);

void ruckig_brake_get_second_order_position_trajectory(
    ruckig_brake_profile_t* brake,
    double v0,
    double v_max,
    double v_min,
    double a_max,
    double a_min
);

void ruckig_brake_get_velocity_trajectory(
    ruckig_brake_profile_t* brake,
    double a0,
    double a_max,
    double a_min,
    double j_max
);

void ruckig_brake_get_second_order_velocity_trajectory(ruckig_brake_profile_t* brake);

void ruckig_brake_finalize(
    ruckig_brake_profile_t* brake,
    double* position,
    double* velocity,
    double* acceleration
);

void ruckig_brake_finalize_second_order(
    ruckig_brake_profile_t* brake,
    double* position,
    double* velocity,
    double* acceleration
);

#endif
