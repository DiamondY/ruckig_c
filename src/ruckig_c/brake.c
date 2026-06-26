#include "ruckig_c/brake.h"

#include "ruckig_c/precision.h"
#include "ruckig_c/utils.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

static const double brake_eps = RUCKIG_C_BRAKE_TIME_EPS;

static double v_at_t(double v0, double a0, double j, double t) {
    return v0 + t * (a0 + j * t / 2.0);
}

static bool v_at_a_zero(double v0, double a0, double j, double* result) {
    double value;
    if (fabs(j) < RUCKIG_C_PROFILE_J_EPS || !result) {
        return false;
    }
    value = v0 + (a0 * a0) / (2.0 * j);
    if (!isfinite(value)) {
        return false;
    }
    *result = value;
    return true;
}

static bool sqrt_nonnegative_or_zero(double value, double* result) {
    if (!isfinite(value)) {
        *result = 0.0;
        return false;
    }
    if (value < 0.0) {
        if (value < -DBL_EPSILON) {
            *result = 0.0;
            return false;
        }
        value = 0.0;
    }
    *result = sqrt(value);
    return isfinite(*result);
}

void ruckig_brake_profile_init(ruckig_brake_profile_t* brake) {
    if (brake) {
        memset(brake, 0, sizeof(*brake));
    }
}

static void acceleration_brake(
    ruckig_brake_profile_t* brake,
    double v0,
    double a0,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    const double t_to_a_max = (a0 - a_max) / j_max;
    const double t_to_a_zero = a0 / j_max;
    const double v_at_a_max = v_at_t(v0, a0, -j_max, t_to_a_max);
    const double v_at_zero = v_at_t(v0, a0, -j_max, t_to_a_zero);

    brake->j[0] = -j_max;
    if (!isfinite(t_to_a_max) || !isfinite(t_to_a_zero) || !isfinite(v_at_a_max) || !isfinite(v_at_zero)) {
        return;
    }

    if ((v_at_zero > v_max && j_max > 0.0) || (v_at_zero < v_max && j_max < 0.0)) {
        const double t_to_a_min = (a0 - a_min) / j_max;
        const double radicand_to_v_max = a0 * a0 + 2.0 * j_max * (v0 - v_max);
        const double radicand_to_v_min = a0 * a0 / 2.0 + j_max * (v0 - v_min);
        double sqrt_to_v_max = 0.0;
        double sqrt_to_v_min = 0.0;
        const bool has_t_to_v_max = sqrt_nonnegative_or_zero(radicand_to_v_max, &sqrt_to_v_max);
        const bool has_t_to_v_min = sqrt_nonnegative_or_zero(radicand_to_v_min, &sqrt_to_v_min);
        const double t_to_v_max = has_t_to_v_max ? a0 / j_max + sqrt_to_v_max / fabs(j_max) : INFINITY;
        const double t_to_v_min = has_t_to_v_min ? a0 / j_max + sqrt_to_v_min / fabs(j_max) : INFINITY;
        const double t_min_to_v_max = t_to_v_max < t_to_v_min ? t_to_v_max : t_to_v_min;

        if ((!has_t_to_v_max && !has_t_to_v_min) || !isfinite(t_min_to_v_max)) {
            return;
        }
        if (t_to_a_min < t_min_to_v_max) {
            const double v_at_a_min = v_at_t(v0, a0, -j_max, t_to_a_min);
            const double t_to_v_max_with_constant = -(v_at_a_min - v_max) / a_min;
            const double t_to_v_min_with_constant = a_min / (2.0 * j_max) - (v_at_a_min - v_min) / a_min;

            if (!isfinite(t_to_a_min) || !isfinite(v_at_a_min)
                || !isfinite(t_to_v_max_with_constant) || !isfinite(t_to_v_min_with_constant)) {
                return;
            }
            brake->t[0] = fmax(t_to_a_min - brake_eps, 0.0);
            brake->t[1] = fmax(fmin(t_to_v_max_with_constant, t_to_v_min_with_constant), 0.0);
        } else {
            brake->t[0] = fmax(t_min_to_v_max - brake_eps, 0.0);
        }

    } else if ((v_at_a_max < v_min && j_max > 0.0) || (v_at_a_max > v_min && j_max < 0.0)) {
        const double t_to_v_min = -(v_at_a_max - v_min) / a_max;
        const double t_to_v_max = -a_max / (2.0 * j_max) - (v_at_a_max - v_max) / a_max;

        if (!isfinite(t_to_v_min) || !isfinite(t_to_v_max)) {
            return;
        }
        brake->t[0] = t_to_a_max + brake_eps;
        brake->t[1] = fmax(fmin(t_to_v_min, t_to_v_max - brake_eps), 0.0);

    } else {
        brake->t[0] = t_to_a_max + brake_eps;
    }
}

static void velocity_brake(
    ruckig_brake_profile_t* brake,
    double v0,
    double a0,
    double v_max,
    double v_min,
    double a_min,
    double j_max
) {
    const double t_to_a_min = (a0 - a_min) / j_max;
    const double radicand_to_v_max = a0 * a0 + 2.0 * j_max * (v0 - v_max);
    const double radicand_to_v_min = a0 * a0 / 2.0 + j_max * (v0 - v_min);
    double sqrt_to_v_max = 0.0;
    double sqrt_to_v_min = 0.0;
    const bool has_t_to_v_max = sqrt_nonnegative_or_zero(radicand_to_v_max, &sqrt_to_v_max);
    const bool has_t_to_v_min = sqrt_nonnegative_or_zero(radicand_to_v_min, &sqrt_to_v_min);
    const double t_to_v_max = has_t_to_v_max ? a0 / j_max + sqrt_to_v_max / fabs(j_max) : INFINITY;
    const double t_to_v_min = has_t_to_v_min ? a0 / j_max + sqrt_to_v_min / fabs(j_max) : INFINITY;
    const double t_min_to_v_max = t_to_v_max < t_to_v_min ? t_to_v_max : t_to_v_min;

    brake->j[0] = -j_max;

    if ((!has_t_to_v_max && !has_t_to_v_min) || !isfinite(t_min_to_v_max)) {
        return;
    }
    if (t_to_a_min < t_min_to_v_max) {
        const double v_at_a_min = v_at_t(v0, a0, -j_max, t_to_a_min);
        const double t_to_v_max_with_constant = -(v_at_a_min - v_max) / a_min;
        const double t_to_v_min_with_constant = a_min / (2.0 * j_max) - (v_at_a_min - v_min) / a_min;

        if (!isfinite(t_to_a_min) || !isfinite(v_at_a_min)
            || !isfinite(t_to_v_max_with_constant) || !isfinite(t_to_v_min_with_constant)) {
            return;
        }
        brake->t[0] = fmax(t_to_a_min - brake_eps, 0.0);
        brake->t[1] = fmax(fmin(t_to_v_max_with_constant, t_to_v_min_with_constant), 0.0);
    } else {
        brake->t[0] = fmax(t_min_to_v_max - brake_eps, 0.0);
    }
}

void ruckig_brake_get_position_trajectory(
    ruckig_brake_profile_t* brake,
    double v0,
    double a0,
    double v_max,
    double v_min,
    double a_max,
    double a_min,
    double j_max
) {
    double v_zero_negative_jerk = 0.0;
    double v_zero_positive_jerk = 0.0;
    const bool has_v_zero_negative_jerk = v_at_a_zero(v0, a0, -j_max, &v_zero_negative_jerk);
    const bool has_v_zero_positive_jerk = v_at_a_zero(v0, a0, j_max, &v_zero_positive_jerk);
    if (!brake) {
        return;
    }
    ruckig_brake_profile_init(brake);

    /* Brake pre-trajectories move invalid or inevitably invalid states back inside limits before the main profile. */
    if (fabs(j_max) < RUCKIG_C_PROFILE_J_EPS || a_max == 0.0 || a_min == 0.0) {
        return;
    }

    if (a0 > a_max) {
        acceleration_brake(brake, v0, a0, v_max, v_min, a_max, a_min, j_max);
    } else if (a0 < a_min) {
        acceleration_brake(brake, v0, a0, v_min, v_max, a_min, a_max, -j_max);
    } else if ((v0 > v_max && has_v_zero_negative_jerk && v_zero_negative_jerk > v_min)
        || (a0 > 0.0 && has_v_zero_positive_jerk && v_zero_positive_jerk > v_max)) {
        velocity_brake(brake, v0, a0, v_max, v_min, a_min, j_max);
    } else if ((v0 < v_min && has_v_zero_positive_jerk && v_zero_positive_jerk < v_max)
        || (a0 < 0.0 && has_v_zero_negative_jerk && v_zero_negative_jerk < v_min)) {
        velocity_brake(brake, v0, a0, v_min, v_max, a_max, -j_max);
    }
}

void ruckig_brake_get_second_order_position_trajectory(
    ruckig_brake_profile_t* brake,
    double v0,
    double v_max,
    double v_min,
    double a_max,
    double a_min
) {
    if (!brake) {
        return;
    }
    ruckig_brake_profile_init(brake);

    /* Second-order braking has no jerk phase; keep the branch order aligned with the C++ oracle. */
    if (a_max == 0.0 || a_min == 0.0) {
        return;
    }

    if (v0 > v_max) {
        brake->a[0] = a_min;
        brake->t[0] = (v_max - v0) / a_min + brake_eps;
    } else if (v0 < v_min) {
        brake->a[0] = a_max;
        brake->t[0] = (v_min - v0) / a_max + brake_eps;
    }
}

void ruckig_brake_get_velocity_trajectory(
    ruckig_brake_profile_t* brake,
    double a0,
    double a_max,
    double a_min,
    double j_max
) {
    if (!brake) {
        return;
    }
    ruckig_brake_profile_init(brake);

    /* Velocity control only brakes acceleration violations before solving the target velocity profile. */
    if (fabs(j_max) < RUCKIG_C_PROFILE_J_EPS) {
        return;
    }

    if (a0 > a_max) {
        brake->j[0] = -j_max;
        brake->t[0] = (a0 - a_max) / j_max + brake_eps;
    } else if (a0 < a_min) {
        brake->j[0] = j_max;
        brake->t[0] = -(a0 - a_min) / j_max + brake_eps;
    }
}

void ruckig_brake_get_second_order_velocity_trajectory(ruckig_brake_profile_t* brake) {
    ruckig_brake_profile_init(brake);
}

void ruckig_brake_finalize(
    ruckig_brake_profile_t* brake,
    double* position,
    double* velocity,
    double* acceleration
) {
    if (!brake || !position || !velocity || !acceleration) {
        return;
    }

    if (brake->t[0] <= 0.0 && brake->t[1] <= 0.0) {
        brake->duration = 0.0;
        return;
    }

    brake->duration = brake->t[0];
    brake->p[0] = *position;
    brake->v[0] = *velocity;
    brake->a[0] = *acceleration;
    ruckig_integrate(brake->t[0], *position, *velocity, *acceleration, brake->j[0], position, velocity, acceleration);

    if (brake->t[1] > 0.0) {
        brake->duration += brake->t[1];
        brake->p[1] = *position;
        brake->v[1] = *velocity;
        brake->a[1] = *acceleration;
        ruckig_integrate(brake->t[1], *position, *velocity, *acceleration, brake->j[1], position, velocity, acceleration);
    }
}

void ruckig_brake_finalize_second_order(
    ruckig_brake_profile_t* brake,
    double* position,
    double* velocity,
    double* acceleration
) {
    if (!brake || !position || !velocity || !acceleration) {
        return;
    }

    if (brake->t[0] <= 0.0) {
        brake->duration = 0.0;
        return;
    }

    brake->duration = brake->t[0];
    brake->p[0] = *position;
    brake->v[0] = *velocity;
    ruckig_integrate(brake->t[0], *position, *velocity, brake->a[0], 0.0, position, velocity, acceleration);
}
