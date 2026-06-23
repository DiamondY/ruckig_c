#include "test_common.h"

#include "ruckig_c/brake.h"
#include "ruckig_c/precision.h"

static void test_position_velocity_brake(void) {
    ruckig_brake_profile_t brake;
    double p = 0.0;
    double v = 2.0;
    double a = 0.0;

    ruckig_brake_get_position_trajectory(&brake, v, a, 1.0, -1.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], -2.0, 0.0);

    ruckig_brake_finalize(&brake, &p, &v, &a);
    CHECK_TRUE(brake.duration > 0.0);
    CHECK_TRUE(v <= 1.0 + 1e-12);
}

static void test_acceleration_brake(void) {
    ruckig_brake_profile_t brake;
    double p = 0.0;
    double v = 0.0;
    double a = 2.0;

    ruckig_brake_get_velocity_trajectory(&brake, a, 1.0, -1.0, 2.0);
    CHECK_NEAR(brake.j[0], -2.0, 0.0);
    CHECK_NEAR(brake.t[0], 0.5 + RUCKIG_C_BRAKE_TIME_EPS, 1e-15);

    ruckig_brake_finalize(&brake, &p, &v, &a);
    CHECK_NEAR(a, 1.0 - 2.0 * RUCKIG_C_BRAKE_TIME_EPS, 1e-13);
}

static void test_second_order_brake(void) {
    ruckig_brake_profile_t brake;
    double p = 0.0;
    double v = 2.0;
    double a = 0.0;

    ruckig_brake_get_second_order_position_trajectory(&brake, v, 1.0, -1.0, 1.0, -1.0);
    CHECK_NEAR(brake.a[0], -1.0, 0.0);
    CHECK_NEAR(brake.t[0], 1.0 + RUCKIG_C_BRAKE_TIME_EPS, 1e-15);

    ruckig_brake_finalize_second_order(&brake, &p, &v, &a);
    CHECK_TRUE(brake.duration > 1.0);
    CHECK_NEAR(a, -1.0, 0.0);
    CHECK_TRUE(v <= 1.0 + 1e-12);
}

void run_brake_tests(void) {
    test_position_velocity_brake();
    test_acceleration_brake();
    test_second_order_brake();
}
