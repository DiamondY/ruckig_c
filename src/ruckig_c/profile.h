#ifndef RUCKIG_C_PROFILE_H
#define RUCKIG_C_PROFILE_H

#include "ruckig_c/brake.h"

#include <stdbool.h>

typedef struct ruckig_bound {
    double min;
    double max;
    double t_min;
    double t_max;
} ruckig_bound_t;

typedef enum ruckig_profile_control_signs {
    RUCKIG_PROFILE_SIGNS_UDDU = 0,
    RUCKIG_PROFILE_SIGNS_UDUD = 1
} ruckig_profile_control_signs_t;

typedef enum ruckig_profile_reached_limits {
    RUCKIG_PROFILE_LIMITS_ACC0_ACC1_VEL = 0,
    RUCKIG_PROFILE_LIMITS_VEL = 1,
    RUCKIG_PROFILE_LIMITS_ACC0 = 2,
    RUCKIG_PROFILE_LIMITS_ACC1 = 3,
    RUCKIG_PROFILE_LIMITS_ACC0_ACC1 = 4,
    RUCKIG_PROFILE_LIMITS_ACC0_VEL = 5,
    RUCKIG_PROFILE_LIMITS_ACC1_VEL = 6,
    RUCKIG_PROFILE_LIMITS_NONE = 7
} ruckig_profile_reached_limits_t;

typedef enum ruckig_profile_direction {
    RUCKIG_PROFILE_DIRECTION_UP = 0,
    RUCKIG_PROFILE_DIRECTION_DOWN = 1
} ruckig_profile_direction_t;

typedef struct ruckig_profile {
    double t[7];
    double t_sum[7];
    double j[7];
    double a[8];
    double v[8];
    double p[8];
    ruckig_brake_profile_t brake;
    ruckig_brake_profile_t accel;
    double pf;
    double vf;
    double af;
    ruckig_profile_reached_limits_t limits;
    ruckig_profile_direction_t direction;
    ruckig_profile_control_signs_t control_signs;
} ruckig_profile_t;

typedef struct ruckig_profile_third_order_check {
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    bool set_limits;
    double tf;
    double jf;
    double j_max;
    double v_max;
    double v_min;
    double a_max;
    double a_min;
} ruckig_profile_third_order_check_t;

typedef struct ruckig_profile_third_order_velocity_check {
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double tf;
    double jf;
    double j_max;
    double a_max;
    double a_min;
} ruckig_profile_third_order_velocity_check_t;

typedef struct ruckig_profile_second_order_check {
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double tf;
    double a_up;
    double a_down;
    double v_max;
    double v_min;
    double a_max;
    double a_min;
} ruckig_profile_second_order_check_t;

typedef struct ruckig_profile_second_order_velocity_check {
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double tf;
    double a_up;
    double a_max;
    double a_min;
} ruckig_profile_second_order_velocity_check_t;

typedef struct ruckig_profile_first_order_check {
    ruckig_profile_control_signs_t signs;
    ruckig_profile_reached_limits_t limits;
    double tf;
    double v_up;
    double v_max;
    double v_min;
} ruckig_profile_first_order_check_t;

void ruckig_profile_init(ruckig_profile_t* profile);
void ruckig_profile_set_boundary(ruckig_profile_t* profile, double p0, double v0, double a0, double pf, double vf, double af);
void ruckig_profile_set_boundary_for_velocity(ruckig_profile_t* profile, double p0, double v0, double a0, double vf, double af);
void ruckig_profile_copy_boundary(ruckig_profile_t* profile, const ruckig_profile_t* source);

bool ruckig_profile_check_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_check_t* check);
bool ruckig_profile_check_with_timing_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_check_t* check);
bool ruckig_profile_check_with_timing_guarded_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_check_t* check);

bool ruckig_profile_check_for_velocity_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_velocity_check_t* check);
bool ruckig_profile_check_for_velocity_with_timing_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_velocity_check_t* check);
bool ruckig_profile_check_for_velocity_with_timing_guarded_ctx(ruckig_profile_t* profile, const ruckig_profile_third_order_velocity_check_t* check);

bool ruckig_profile_check_for_second_order_ctx(ruckig_profile_t* profile, const ruckig_profile_second_order_check_t* check);
bool ruckig_profile_check_for_second_order_with_timing_ctx(ruckig_profile_t* profile, const ruckig_profile_second_order_check_t* check);
bool ruckig_profile_check_for_second_order_with_timing_guarded_ctx(ruckig_profile_t* profile, const ruckig_profile_second_order_check_t* check);

bool ruckig_profile_check_for_second_order_velocity_ctx(ruckig_profile_t* profile, const ruckig_profile_second_order_velocity_check_t* check);
bool ruckig_profile_check_for_second_order_velocity_with_timing_ctx(ruckig_profile_t* profile, const ruckig_profile_second_order_velocity_check_t* check);
bool ruckig_profile_check_for_second_order_velocity_with_timing_guarded_ctx(ruckig_profile_t* profile, const ruckig_profile_second_order_velocity_check_t* check);

bool ruckig_profile_check_for_first_order_ctx(ruckig_profile_t* profile, const ruckig_profile_first_order_check_t* check);
bool ruckig_profile_check_for_first_order_with_timing_ctx(ruckig_profile_t* profile, const ruckig_profile_first_order_check_t* check);
bool ruckig_profile_check_for_first_order_with_timing_guarded_ctx(ruckig_profile_t* profile, const ruckig_profile_first_order_check_t* check);

#define ruckig_profile_check(profile, signs, limits, set_limits, jf, v_max, v_min, a_max, a_min) \
    ruckig_profile_check_ctx((profile), &(ruckig_profile_third_order_check_t){(signs), (limits), (set_limits), 0.0, (jf), 0.0, (v_max), (v_min), (a_max), (a_min)})
#define ruckig_profile_check_with_timing(profile, signs, limits, tf, jf, v_max, v_min, a_max, a_min) \
    ruckig_profile_check_with_timing_ctx((profile), &(ruckig_profile_third_order_check_t){(signs), (limits), false, (tf), (jf), 0.0, (v_max), (v_min), (a_max), (a_min)})
#define ruckig_profile_check_with_timing_guarded(profile, signs, limits, tf, jf, v_max, v_min, a_max, a_min, j_max) \
    ruckig_profile_check_with_timing_guarded_ctx((profile), &(ruckig_profile_third_order_check_t){(signs), (limits), false, (tf), (jf), (j_max), (v_max), (v_min), (a_max), (a_min)})

#define ruckig_profile_check_for_velocity(profile, signs, limits, jf, a_max, a_min) \
    ruckig_profile_check_for_velocity_ctx((profile), &(ruckig_profile_third_order_velocity_check_t){(signs), (limits), 0.0, (jf), 0.0, (a_max), (a_min)})
#define ruckig_profile_check_for_velocity_with_timing(profile, signs, limits, tf, jf, a_max, a_min) \
    ruckig_profile_check_for_velocity_with_timing_ctx((profile), &(ruckig_profile_third_order_velocity_check_t){(signs), (limits), (tf), (jf), 0.0, (a_max), (a_min)})
#define ruckig_profile_check_for_velocity_with_timing_guarded(profile, signs, limits, tf, jf, a_max, a_min, j_max) \
    ruckig_profile_check_for_velocity_with_timing_guarded_ctx((profile), &(ruckig_profile_third_order_velocity_check_t){(signs), (limits), (tf), (jf), (j_max), (a_max), (a_min)})

#define ruckig_profile_check_for_second_order(profile, signs, limits, a_up, a_down, v_max, v_min) \
    ruckig_profile_check_for_second_order_ctx((profile), &(ruckig_profile_second_order_check_t){(signs), (limits), 0.0, (a_up), (a_down), (v_max), (v_min), 0.0, 0.0})
#define ruckig_profile_check_for_second_order_with_timing(profile, signs, limits, tf, a_up, a_down, v_max, v_min) \
    ruckig_profile_check_for_second_order_with_timing_ctx((profile), &(ruckig_profile_second_order_check_t){(signs), (limits), (tf), (a_up), (a_down), (v_max), (v_min), 0.0, 0.0})
#define ruckig_profile_check_for_second_order_with_timing_guarded(profile, signs, limits, tf, a_up, a_down, v_max, v_min, a_max, a_min) \
    ruckig_profile_check_for_second_order_with_timing_guarded_ctx((profile), &(ruckig_profile_second_order_check_t){(signs), (limits), (tf), (a_up), (a_down), (v_max), (v_min), (a_max), (a_min)})

#define ruckig_profile_check_for_second_order_velocity(profile, signs, limits, a_up) \
    ruckig_profile_check_for_second_order_velocity_ctx((profile), &(ruckig_profile_second_order_velocity_check_t){(signs), (limits), 0.0, (a_up), 0.0, 0.0})
#define ruckig_profile_check_for_second_order_velocity_with_timing(profile, signs, limits, tf, a_up) \
    ruckig_profile_check_for_second_order_velocity_with_timing_ctx((profile), &(ruckig_profile_second_order_velocity_check_t){(signs), (limits), (tf), (a_up), 0.0, 0.0})
#define ruckig_profile_check_for_second_order_velocity_with_timing_guarded(profile, signs, limits, tf, a_up, a_max, a_min) \
    ruckig_profile_check_for_second_order_velocity_with_timing_guarded_ctx((profile), &(ruckig_profile_second_order_velocity_check_t){(signs), (limits), (tf), (a_up), (a_max), (a_min)})

#define ruckig_profile_check_for_first_order(profile, signs, limits, v_up) \
    ruckig_profile_check_for_first_order_ctx((profile), &(ruckig_profile_first_order_check_t){(signs), (limits), 0.0, (v_up), 0.0, 0.0})
#define ruckig_profile_check_for_first_order_with_timing(profile, signs, limits, tf, v_up) \
    ruckig_profile_check_for_first_order_with_timing_ctx((profile), &(ruckig_profile_first_order_check_t){(signs), (limits), (tf), (v_up), 0.0, 0.0})
#define ruckig_profile_check_for_first_order_with_timing_guarded(profile, signs, limits, tf, v_up, v_max, v_min) \
    ruckig_profile_check_for_first_order_with_timing_guarded_ctx((profile), &(ruckig_profile_first_order_check_t){(signs), (limits), (tf), (v_up), (v_max), (v_min)})

ruckig_bound_t ruckig_profile_get_position_extrema(const ruckig_profile_t* profile);
bool ruckig_profile_get_first_state_at_position(const ruckig_profile_t* profile, double position, double* time, double time_after);

#endif
