#include "test_common.h"

#include "ruckig_c/profile.h"

static void test_first_order_profile(void) {
    ruckig_profile_t profile;
    ruckig_bound_t extrema;
    double time = 0.0;

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary(&profile, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    profile.t[3] = 2.0;

    CHECK_TRUE(ruckig_profile_check_for_first_order(&profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_VEL, 1.0));
    CHECK_NEAR(profile.p[7], 2.0, 1e-12);
    CHECK_NEAR(profile.t_sum[6], 2.0, 0.0);

    extrema = ruckig_profile_get_position_extrema(&profile);
    CHECK_NEAR(extrema.min, 0.0, 0.0);
    CHECK_NEAR(extrema.max, 2.0, 0.0);
    CHECK_NEAR(extrema.t_max, 2.0, 0.0);

    CHECK_TRUE(ruckig_profile_get_first_state_at_position(&profile, 1.0, &time, 0.0));
    CHECK_NEAR(time, 1.0, 1e-12);
}

static void test_second_order_velocity_profile(void) {
    ruckig_profile_t profile;

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary_for_velocity(&profile, 0.0, 0.0, 0.0, 1.0, 0.0);
    profile.t[1] = 1.0;

    CHECK_TRUE(ruckig_profile_check_for_second_order_velocity(&profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_ACC0, 1.0));
    CHECK_NEAR(profile.v[7], 1.0, 1e-12);
    CHECK_NEAR(profile.p[7], 0.5, 1e-12);
}

static void test_third_order_velocity_profile(void) {
    ruckig_profile_t profile;

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary_for_velocity(&profile, 0.0, 0.0, 0.0, 1.0, 0.0);
    profile.t[0] = 1.0;
    profile.t[2] = 1.0;

    CHECK_TRUE(ruckig_profile_check_for_velocity(&profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 1.0, 2.0, -2.0));
    CHECK_NEAR(profile.v[7], 1.0, 1e-12);
    CHECK_NEAR(profile.a[7], 0.0, 1e-12);
    CHECK_NEAR(profile.p[7], 1.0, 1e-12);
}

static void test_third_order_position_profile(void) {
    ruckig_profile_t profile;
    double time = 0.0;

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary(&profile, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    profile.t[0] = 1.0;
    profile.t[2] = 1.0;
    profile.t[4] = 1.0;
    profile.t[6] = 1.0;

    CHECK_TRUE(ruckig_profile_check(&profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, false, 1.0, 2.0, -2.0, 2.0, -2.0));
    CHECK_NEAR(profile.p[7], 2.0, 1e-12);
    CHECK_NEAR(profile.v[7], 0.0, 1e-12);
    CHECK_NEAR(profile.a[7], 0.0, 1e-12);

    CHECK_TRUE(ruckig_profile_check_with_timing_guarded(&profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 4.0, 1.0, 2.0, -2.0, 2.0, -2.0, 1.0));
    CHECK_TRUE(!ruckig_profile_check_with_timing_guarded(&profile, RUCKIG_PROFILE_SIGNS_UDDU, RUCKIG_PROFILE_LIMITS_NONE, 4.0, 2.0, 2.0, -2.0, 2.0, -2.0, 1.0));

    CHECK_TRUE(ruckig_profile_get_first_state_at_position(&profile, 1.0, &time, 0.0));
    CHECK_NEAR(time, 2.0, 1e-12);
}

static void test_profile_context_entrypoints(void) {
    ruckig_profile_t profile;
    ruckig_profile_first_order_check_t first_order = {
        RUCKIG_PROFILE_SIGNS_UDDU,
        RUCKIG_PROFILE_LIMITS_VEL,
        2.0,
        1.0,
        2.0,
        -2.0
    };
    ruckig_profile_second_order_velocity_check_t second_order_velocity = {
        RUCKIG_PROFILE_SIGNS_UDDU,
        RUCKIG_PROFILE_LIMITS_ACC0,
        1.0,
        1.0,
        2.0,
        -2.0
    };
    ruckig_profile_third_order_velocity_check_t third_order_velocity = {
        RUCKIG_PROFILE_SIGNS_UDDU,
        RUCKIG_PROFILE_LIMITS_NONE,
        2.0,
        1.0,
        2.0,
        2.0,
        -2.0
    };
    ruckig_profile_second_order_check_t second_order = {
        RUCKIG_PROFILE_SIGNS_UDDU,
        RUCKIG_PROFILE_LIMITS_NONE,
        2.0,
        1.0,
        -1.0,
        2.0,
        -2.0,
        2.0,
        -2.0
    };
    ruckig_profile_third_order_check_t third_order = {
        RUCKIG_PROFILE_SIGNS_UDDU,
        RUCKIG_PROFILE_LIMITS_NONE,
        false,
        4.0,
        1.0,
        1.0,
        2.0,
        -2.0,
        2.0,
        -2.0
    };
    ruckig_profile_third_order_check_t guarded_failure = third_order;

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary(&profile, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    profile.t[3] = 2.0;
    CHECK_TRUE(ruckig_profile_check_for_first_order_with_timing_guarded_ctx(&profile, &first_order));
    CHECK_NEAR(profile.p[7], 2.0, 1e-12);

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary_for_velocity(&profile, 0.0, 0.0, 0.0, 1.0, 0.0);
    profile.t[1] = 1.0;
    CHECK_TRUE(ruckig_profile_check_for_second_order_velocity_with_timing_guarded_ctx(&profile, &second_order_velocity));
    CHECK_NEAR(profile.v[7], 1.0, 1e-12);

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary_for_velocity(&profile, 0.0, 0.0, 0.0, 1.0, 0.0);
    profile.t[0] = 1.0;
    profile.t[2] = 1.0;
    CHECK_TRUE(ruckig_profile_check_for_velocity_with_timing_guarded_ctx(&profile, &third_order_velocity));
    CHECK_NEAR(profile.v[7], 1.0, 1e-12);

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary(&profile, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    profile.t[0] = 1.0;
    profile.t[2] = 1.0;
    CHECK_TRUE(ruckig_profile_check_for_second_order_with_timing_guarded_ctx(&profile, &second_order));
    CHECK_NEAR(profile.p[7], 1.0, 1e-12);

    ruckig_profile_init(&profile);
    ruckig_profile_set_boundary(&profile, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    profile.t[0] = 1.0;
    profile.t[2] = 1.0;
    profile.t[4] = 1.0;
    profile.t[6] = 1.0;
    CHECK_TRUE(ruckig_profile_check_with_timing_guarded_ctx(&profile, &third_order));
    guarded_failure.jf = 2.0;
    CHECK_TRUE(!ruckig_profile_check_with_timing_guarded_ctx(&profile, &guarded_failure));
}

void run_profile_tests(void) {
    test_first_order_profile();
    test_second_order_velocity_profile();
    test_third_order_velocity_profile();
    test_third_order_position_profile();
    test_profile_context_entrypoints();
}
