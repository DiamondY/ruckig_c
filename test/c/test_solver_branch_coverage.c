#include "test_common.h"

#include "ruckig_c/block.h"
#include "ruckig_c/brake.h"
#include "ruckig_c/position_first.h"
#include "ruckig_c/velocity_second.h"

#include <stddef.h>

static ruckig_profile_t make_profile(double duration, ruckig_profile_direction_t direction, double marker) {
    ruckig_profile_t profile;
    ruckig_profile_init(&profile);
    profile.t_sum[6] = duration;
    profile.direction = direction;
    profile.pf = marker;
    return profile;
}

static void expect_profile_marker(const ruckig_profile_t* profile, double marker) {
    CHECK_TRUE(profile != NULL);
    if (profile) {
        CHECK_NEAR(profile->pf, marker, 0.0);
    }
}

static void expect_position_profile_end(
    const ruckig_profile_t* profile,
    double pf,
    double vf,
    double af
);

static void test_block_invalid_and_single_profile(void) {
    ruckig_block_t block;
    ruckig_profile_t profiles[1];

    ruckig_block_init(&block);
    profiles[0] = make_profile(1.25, RUCKIG_PROFILE_DIRECTION_UP, 101.0);

    CHECK_TRUE(!ruckig_block_calculate(NULL, profiles, 1));
    CHECK_TRUE(!ruckig_block_calculate(&block, NULL, 1));
    CHECK_TRUE(!ruckig_block_calculate(&block, profiles, 0));
    CHECK_TRUE(ruckig_block_is_blocked(NULL, 0.0));
    CHECK_TRUE(ruckig_block_get_profile(NULL, 0.0) == NULL);

    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 1));
    CHECK_TRUE(block.valid);
    CHECK_NEAR(block.t_min, 1.25, 0.0);
    CHECK_TRUE(!block.a.valid);
    CHECK_TRUE(!block.b.valid);
    CHECK_TRUE(ruckig_block_is_blocked(&block, 1.0));
    CHECK_TRUE(!ruckig_block_is_blocked(&block, 1.25));
    expect_profile_marker(ruckig_block_get_profile(&block, 1.25), 101.0);
}

static void test_block_two_profile_tie_and_interval(void) {
    ruckig_block_t block;
    ruckig_profile_t profiles[2];

    profiles[0] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 201.0);
    profiles[1] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_DOWN, 202.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 2));
    CHECK_NEAR(block.t_min, 2.0, 0.0);
    CHECK_TRUE(!block.a.valid);
    expect_profile_marker(ruckig_block_get_profile(&block, 3.0), 201.0);

    profiles[0] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 203.0);
    profiles[1] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_DOWN, 204.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 2));
    CHECK_NEAR(block.t_min, 1.0, 0.0);
    CHECK_TRUE(block.a.valid);
    CHECK_NEAR(block.a.left, 1.0, 0.0);
    CHECK_NEAR(block.a.right, 3.0, 0.0);
    CHECK_TRUE(ruckig_block_is_blocked(&block, 2.0));
    CHECK_TRUE(!ruckig_block_is_blocked(&block, 3.0));
    expect_profile_marker(ruckig_block_get_profile(&block, 2.0), 203.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 3.0), 204.0);
}

static void test_block_three_profile_interval(void) {
    ruckig_block_t block;
    ruckig_profile_t profiles[3];

    profiles[0] = make_profile(4.0, RUCKIG_PROFILE_DIRECTION_UP, 301.0);
    profiles[1] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_DOWN, 302.0);
    profiles[2] = make_profile(2.5, RUCKIG_PROFILE_DIRECTION_UP, 303.0);

    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 3));
    CHECK_NEAR(block.t_min, 1.0, 0.0);
    CHECK_TRUE(block.a.valid);
    CHECK_TRUE(!block.b.valid);
    CHECK_NEAR(block.a.left, 2.5, 0.0);
    CHECK_NEAR(block.a.right, 4.0, 0.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 1.0), 302.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 4.0), 301.0);
}

static void test_block_four_profile_duplicate_removal(void) {
    ruckig_block_t block;
    ruckig_profile_t profiles[4];

    profiles[0] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 401.0);
    profiles[1] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_DOWN, 402.0);
    profiles[2] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 403.0);
    profiles[3] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_UP, 404.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 4));
    CHECK_NEAR(block.t_min, 1.0, 0.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 3.0), 404.0);

    profiles[0] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 405.0);
    profiles[1] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_UP, 406.0);
    profiles[2] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 407.0);
    profiles[3] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_DOWN, 408.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 4));
    CHECK_NEAR(block.t_min, 1.0, 0.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 3.0), 406.0);

    profiles[0] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 409.0);
    profiles[1] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 410.0);
    profiles[2] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_UP, 411.0);
    profiles[3] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_DOWN, 412.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 4));
    CHECK_NEAR(block.t_min, 1.0, 0.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 3.0), 411.0);

    profiles[0] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 413.0);
    profiles[1] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 414.0);
    profiles[2] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_UP, 415.0);
    profiles[3] = make_profile(4.0, RUCKIG_PROFILE_DIRECTION_UP, 416.0);
    CHECK_TRUE(!ruckig_block_calculate(&block, profiles, 4));
}

static void test_block_five_profile_interval_pairing(void) {
    ruckig_block_t block;
    ruckig_profile_t profiles[5];

    profiles[0] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 501.0);
    profiles[1] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 502.0);
    profiles[2] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_UP, 503.0);
    profiles[3] = make_profile(4.0, RUCKIG_PROFILE_DIRECTION_DOWN, 504.0);
    profiles[4] = make_profile(5.0, RUCKIG_PROFILE_DIRECTION_DOWN, 505.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 5));
    CHECK_TRUE(block.a.valid);
    CHECK_TRUE(block.b.valid);
    CHECK_NEAR(block.a.left, 2.0, 0.0);
    CHECK_NEAR(block.a.right, 3.0, 0.0);
    CHECK_NEAR(block.b.left, 4.0, 0.0);
    CHECK_NEAR(block.b.right, 5.0, 0.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 5.0), 505.0);

    profiles[0] = make_profile(1.0, RUCKIG_PROFILE_DIRECTION_UP, 506.0);
    profiles[1] = make_profile(2.0, RUCKIG_PROFILE_DIRECTION_UP, 507.0);
    profiles[2] = make_profile(3.0, RUCKIG_PROFILE_DIRECTION_DOWN, 508.0);
    profiles[3] = make_profile(4.0, RUCKIG_PROFILE_DIRECTION_DOWN, 509.0);
    profiles[4] = make_profile(5.0, RUCKIG_PROFILE_DIRECTION_UP, 510.0);
    CHECK_TRUE(ruckig_block_calculate(&block, profiles, 5));
    CHECK_TRUE(block.a.valid);
    CHECK_TRUE(block.b.valid);
    CHECK_NEAR(block.a.left, 2.0, 0.0);
    CHECK_NEAR(block.a.right, 5.0, 0.0);
    CHECK_NEAR(block.b.left, 3.0, 0.0);
    CHECK_NEAR(block.b.right, 4.0, 0.0);
    expect_profile_marker(ruckig_block_get_profile(&block, 5.0), 509.0);
}

static void expect_empty_brake(const ruckig_brake_profile_t* brake) {
    CHECK_TRUE(brake != NULL);
    if (!brake) {
        return;
    }

    CHECK_NEAR(brake->duration, 0.0, 0.0);
    CHECK_NEAR(brake->t[0], 0.0, 0.0);
    CHECK_NEAR(brake->t[1], 0.0, 0.0);
}

static void test_brake_solver_adjacent_branches(void) {
    ruckig_brake_profile_t brake;
    double position = 0.0;
    double velocity = 0.0;
    double acceleration = 0.0;

    ruckig_brake_profile_init(NULL);
    ruckig_brake_get_position_trajectory(NULL, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 2.0);
    ruckig_brake_get_second_order_position_trajectory(NULL, 0.0, 1.0, -1.0, 1.0, -1.0);
    ruckig_brake_get_velocity_trajectory(NULL, 0.0, 1.0, -1.0, 2.0);

    ruckig_brake_profile_init(&brake);
    ruckig_brake_get_position_trajectory(&brake, 0.0, 0.0, 1.0, -1.0, 0.0, -1.0, 2.0);
    expect_empty_brake(&brake);
    ruckig_brake_get_position_trajectory(&brake, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 0.0);
    expect_empty_brake(&brake);

    ruckig_brake_get_position_trajectory(&brake, 0.0, 2.0, 5.0, -5.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], -2.0, 0.0);
    position = 0.0;
    velocity = 0.0;
    acceleration = 2.0;
    ruckig_brake_finalize(&brake, &position, &velocity, &acceleration);
    CHECK_TRUE(brake.duration > 0.0);
    CHECK_TRUE(acceleration < 2.0);

    ruckig_brake_get_position_trajectory(&brake, 0.0, -2.0, 5.0, -5.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], 2.0, 0.0);

    ruckig_brake_get_position_trajectory(&brake, 2.0, 0.0, 1.0, -1.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], -2.0, 0.0);

    ruckig_brake_get_position_trajectory(&brake, -2.0, 0.0, 1.0, -1.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], 2.0, 0.0);

    ruckig_brake_get_second_order_position_trajectory(&brake, 0.0, 1.0, -1.0, 0.0, -1.0);
    expect_empty_brake(&brake);
    ruckig_brake_get_second_order_position_trajectory(&brake, 2.0, 1.0, -1.0, 1.0, -1.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.a[0], -1.0, 0.0);
    ruckig_brake_get_second_order_position_trajectory(&brake, -2.0, 1.0, -1.0, 1.0, -1.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.a[0], 1.0, 0.0);

    ruckig_brake_get_velocity_trajectory(&brake, 0.0, 1.0, -1.0, 0.0);
    expect_empty_brake(&brake);
    ruckig_brake_get_velocity_trajectory(&brake, 2.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], -2.0, 0.0);
    ruckig_brake_get_velocity_trajectory(&brake, -2.0, 1.0, -1.0, 2.0);
    CHECK_TRUE(brake.t[0] > 0.0);
    CHECK_NEAR(brake.j[0], 2.0, 0.0);

    ruckig_brake_finalize(NULL, &position, &velocity, &acceleration);
    ruckig_brake_finalize(&brake, NULL, &velocity, &acceleration);
    ruckig_brake_finalize(&brake, &position, NULL, &acceleration);
    ruckig_brake_finalize(&brake, &position, &velocity, NULL);

    ruckig_brake_profile_init(&brake);
    position = 1.0;
    velocity = 2.0;
    acceleration = 3.0;
    ruckig_brake_finalize(&brake, &position, &velocity, &acceleration);
    CHECK_NEAR(brake.duration, 0.0, 0.0);
    CHECK_NEAR(position, 1.0, 0.0);
    CHECK_NEAR(velocity, 2.0, 0.0);
    CHECK_NEAR(acceleration, 3.0, 0.0);

    ruckig_brake_profile_init(&brake);
    brake.t[0] = 0.25;
    brake.j[0] = -2.0;
    position = 0.0;
    velocity = 0.0;
    acceleration = 1.0;
    ruckig_brake_finalize(&brake, &position, &velocity, &acceleration);
    CHECK_NEAR(brake.duration, 0.25, 1e-12);

    ruckig_brake_profile_init(&brake);
    brake.t[0] = 0.25;
    brake.t[1] = 0.5;
    brake.j[0] = -2.0;
    brake.j[1] = 0.0;
    position = 0.0;
    velocity = 0.0;
    acceleration = 1.0;
    ruckig_brake_finalize(&brake, &position, &velocity, &acceleration);
    CHECK_NEAR(brake.duration, 0.75, 1e-12);

    ruckig_brake_finalize_second_order(NULL, &position, &velocity, &acceleration);
    ruckig_brake_finalize_second_order(&brake, NULL, &velocity, &acceleration);
    ruckig_brake_finalize_second_order(&brake, &position, NULL, &acceleration);
    ruckig_brake_finalize_second_order(&brake, &position, &velocity, NULL);

    ruckig_brake_profile_init(&brake);
    position = 1.0;
    velocity = 2.0;
    acceleration = 0.0;
    ruckig_brake_finalize_second_order(&brake, &position, &velocity, &acceleration);
    CHECK_NEAR(brake.duration, 0.0, 0.0);
    CHECK_NEAR(position, 1.0, 0.0);

    ruckig_brake_profile_init(&brake);
    brake.t[0] = 0.5;
    brake.a[0] = -1.0;
    position = 0.0;
    velocity = 2.0;
    acceleration = 0.0;
    ruckig_brake_finalize_second_order(&brake, &position, &velocity, &acceleration);
    CHECK_NEAR(brake.duration, 0.5, 1e-12);
    CHECK_TRUE(velocity < 2.0);
}

static void test_position_first_step_direct_branches(void) {
    ruckig_profile_t input;
    ruckig_profile_t output;
    ruckig_block_t block;
    double duration = 0.0;

    ruckig_profile_init(&input);
    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_first_step1_get_profile(NULL, &output, &block, &duration, 0.0, 2.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_first_step1_get_profile(&input, NULL, &block, &duration, 0.0, 2.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_first_step1_get_profile(&input, &output, NULL, &duration, 0.0, 2.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_first_step1_get_profile(&input, &output, &block, NULL, 0.0, 2.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_first_step1_get_profile(&input, &output, &block, &duration, 0.0, 2.0, 0.0, 0.0));

    CHECK_TRUE(ruckig_position_first_step1_get_profile(&input, &output, &block, &duration, 0.0, 2.0, 1.0, -1.0));
    CHECK_TRUE(block.valid);
    CHECK_NEAR(duration, 2.0, 1e-12);
    expect_position_profile_end(&output, 2.0, 0.0, 0.0);

    ruckig_profile_set_boundary(&input, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_first_step1_get_profile(&input, &output, &block, &duration, 2.0, 0.0, 1.0, -2.0));
    CHECK_TRUE(block.valid);
    CHECK_NEAR(duration, 1.0, 1e-12);
    expect_position_profile_end(&output, 0.0, 0.0, 0.0);

    CHECK_TRUE(!ruckig_position_first_step2_get_profile(NULL, 1.0, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_first_step2_get_profile(&output, 0.0, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_first_step2_get_profile(&output, INFINITY, 0.0, 1.0, 1.0, -1.0));

    ruckig_profile_set_boundary(&output, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_first_step2_get_profile(&output, 2.0, 0.0, 2.0, 1.0, -1.0));
    expect_position_profile_end(&output, 2.0, 0.0, 0.0);

    ruckig_profile_set_boundary(&output, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_first_step2_get_profile(&output, 0.5, 0.0, 2.0, 1.0, -1.0));

    ruckig_profile_set_boundary(&output, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_first_step2_get_profile(&output, 1.0, 2.0, 0.0, 1.0, -2.0));
    expect_position_profile_end(&output, 0.0, 0.0, 0.0);
}

static void test_position_second_step_direct_branches(void) {
    ruckig_profile_t input;
    ruckig_profile_t output;
    ruckig_block_t block;
    double duration = 0.0;

    ruckig_profile_init(&input);
    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(NULL, &output, &block, &duration, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(&input, NULL, &block, &duration, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(&input, &output, NULL, &duration, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(&input, &output, &block, NULL, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));

    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_second_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0));
    CHECK_TRUE(!block.a.valid);
    CHECK_NEAR(duration, 0.0, 1e-12);

    ruckig_profile_set_boundary(&input, 0.0, 0.5, 0.0, 1.0, 0.5, 0.0);
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0, 1.0, -1.0));

    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_second_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 1.0, 0.0, 2.0, -2.0, 1.0, -1.0));
    CHECK_TRUE(block.valid);
    CHECK_TRUE(duration > 0.0);
    expect_position_profile_end(&output, 1.0, 0.0, 0.0);

    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 0.25, 0.5, 0.0);
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.25, 0.5, 0.0, 0.0, 1.0, -1.0));

    CHECK_TRUE(!ruckig_position_second_step2_get_profile(NULL, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    ruckig_profile_set_boundary(&output, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_second_step2_get_profile(&output, -1.0, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_second_step2_get_profile(&output, INFINITY, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_position_second_step2_get_profile(&output, 0.25, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));

    ruckig_profile_set_boundary(&output, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_second_step2_get_profile(&output, 2.0, 0.0, 0.0, 1.0, 0.0, 2.0, -2.0, 1.0, -1.0));
    expect_position_profile_end(&output, 1.0, 0.0, 0.0);

    ruckig_profile_set_boundary(&output, 1.0, 0.0, 0.0, -0.5, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_second_step2_get_profile(&output, 2.5, 1.0, 0.0, -0.5, 0.0, 1.0, -1.5, 1.0, -1.0));
    expect_position_profile_end(&output, -0.5, 0.0, 0.0);
}

static void expect_position_profile_end(
    const ruckig_profile_t* profile,
    double pf,
    double vf,
    double af
) {
    CHECK_TRUE(profile != NULL);
    if (!profile) {
        return;
    }

    CHECK_NEAR(profile->p[7], pf, 1e-8);
    CHECK_NEAR(profile->v[7], vf, 1e-8);
    CHECK_NEAR(profile->a[7], af, 1e-10);
    CHECK_TRUE(profile->t_sum[6] >= 0.0);
}

static void init_position_boundary(
    ruckig_profile_t* profile,
    double p0,
    double v0,
    double a0,
    double pf,
    double vf,
    double af
) {
    ruckig_profile_init(profile);
    ruckig_profile_set_boundary(profile, p0, v0, a0, pf, vf, af);
}

static void test_position_third_step_direct_branches(void) {
    ruckig_profile_t input;
    ruckig_profile_t output;
    ruckig_profile_t sync_profile;
    ruckig_block_t block;
    double duration = -1.0;
    double stretched_duration = 0.0;

    init_position_boundary(&input, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_third_step1_get_profile(NULL, &output, &block, &duration, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(!ruckig_position_third_step1_get_profile(&input, NULL, &block, &duration, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(!ruckig_position_third_step1_get_profile(&input, &output, NULL, &duration, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(!ruckig_position_third_step1_get_profile(&input, &output, &block, NULL, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0));

    CHECK_TRUE(ruckig_position_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(block.valid);
    CHECK_TRUE(!block.a.valid);
    CHECK_NEAR(duration, 0.0, 1e-12);
    expect_position_profile_end(&output, 0.0, 0.0, 0.0);

    init_position_boundary(&input, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 2.0));
    CHECK_TRUE(block.valid);
    CHECK_TRUE(duration > 0.0);
    expect_position_profile_end(&output, 1.25, 0.0, 0.0);

    init_position_boundary(&input, 1.0, -0.15, 0.05, -0.75, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_third_step1_get_profile(&input, &output, &block, &duration, 1.0, -0.15, 0.05, -0.75, 0.0, 0.0, 1.6, -1.2, 1.4, -0.9, 1.7));
    CHECK_TRUE(block.valid);
    CHECK_TRUE(duration > 0.0);
    expect_position_profile_end(&output, -0.75, 0.0, 0.0);

    init_position_boundary(&input, 0.0, 0.5, 0.0, 1.0, 0.5, 0.0);
    CHECK_TRUE(ruckig_position_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.5, 0.0, 1.0, 0.5, 0.0, 2.0, -2.0, 1.0, -1.0, 0.0));
    CHECK_TRUE(block.valid);
    CHECK_TRUE(block.a.valid);
    CHECK_NEAR(duration, 2.0, 1e-12);
    expect_position_profile_end(&output, 1.0, 0.5, 0.0);

    init_position_boundary(&input, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0));
    CHECK_TRUE(!ruckig_position_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0));

    CHECK_TRUE(!ruckig_position_third_step2_get_profile(NULL, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 2.0));
    init_position_boundary(&sync_profile, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_third_step2_get_profile(&sync_profile, -1.0, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 2.0));
    CHECK_TRUE(!ruckig_position_third_step2_get_profile(&sync_profile, INFINITY, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 2.0));
    CHECK_TRUE(!ruckig_position_third_step2_get_profile(&sync_profile, 0.25, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 2.0));
    CHECK_TRUE(!ruckig_position_third_step2_get_profile(&sync_profile, 4.0, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 0.0));

    init_position_boundary(&sync_profile, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0);
    stretched_duration = duration + 0.75;
    if (stretched_duration < 4.0) {
        stretched_duration = 4.0;
    }
    CHECK_TRUE(ruckig_position_third_step2_get_profile(&sync_profile, stretched_duration, 0.0, 0.0, 0.0, 1.25, 0.0, 0.0, 2.0, -2.0, 1.5, -1.5, 2.0));
    expect_position_profile_end(&sync_profile, 1.25, 0.0, 0.0);

    init_position_boundary(&sync_profile, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_third_step2_get_profile(&sync_profile, 4.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.5, -1.5, 1.2, -1.2, 1.6));
    expect_position_profile_end(&sync_profile, -1.0, 0.0, 0.0);
}

static void test_velocity_third_step_direct_branches(void) {
    ruckig_profile_t input;
    ruckig_profile_t output;
    ruckig_block_t block;
    double duration = 0.0;

    ruckig_profile_init(&input);
    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.5, 1.0, 0.5);
    CHECK_TRUE(ruckig_velocity_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.5, 1.0, 0.5, 1.0, -1.0, 0.0));
    CHECK_TRUE(block.a.valid);
    CHECK_NEAR(duration, 2.0, 1e-12);

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_velocity_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 0.0));
    CHECK_TRUE(!block.a.valid);
    CHECK_NEAR(duration, 0.0, 1e-12);

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.0, 0.5, 0.25);
    CHECK_TRUE(!ruckig_velocity_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.5, 0.25, 1.0, -1.0, 0.0));

    CHECK_TRUE(!ruckig_velocity_third_step1_get_profile(NULL, &output, &block, &duration, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(!ruckig_velocity_third_step1_get_profile(&input, NULL, &block, &duration, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(!ruckig_velocity_third_step1_get_profile(&input, &output, NULL, &duration, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0));
    CHECK_TRUE(!ruckig_velocity_third_step1_get_profile(&input, &output, &block, NULL, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0));

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.5, 1.0, 0.5);
    CHECK_TRUE(ruckig_velocity_third_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.5, 1.0, 0.5, 1.0, -1.0, 0.0));
    CHECK_TRUE(block.a.valid);
    CHECK_NEAR(duration, 2.0, 1e-12);

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 1.0, 0.0, -0.5, 0.0);
    CHECK_TRUE(ruckig_velocity_third_step1_get_profile(&input, &output, &block, &duration, 1.0, 0.0, -0.5, 0.0, 1.2, -1.2, 1.5));
    CHECK_TRUE(block.valid);
    CHECK_TRUE(duration > 0.0);

    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(NULL, 1.0, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));
    ruckig_profile_set_boundary_for_velocity(&output, 0.0, 0.0, 1.0, 0.5, -1.0);
    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(&output, 0.0, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));
    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(&output, 1.0, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));
    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(&output, INFINITY, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));

    ruckig_profile_set_boundary_for_velocity(&output, 0.0, 0.0, 0.0, 1.0, 0.0);
    CHECK_TRUE(ruckig_velocity_third_step2_get_profile(&output, 2.0, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 2.0));
    CHECK_NEAR(output.v[7], 1.0, 1e-8);
    CHECK_NEAR(output.a[7], 0.0, 1e-10);

    ruckig_profile_set_boundary_for_velocity(&output, 0.0, 1.0, 0.0, -0.5, 0.0);
    CHECK_TRUE(ruckig_velocity_third_step2_get_profile(&output, 2.5, 1.0, 0.0, -0.5, 0.0, 1.2, -1.2, 1.5));
    CHECK_NEAR(output.v[7], -0.5, 1e-8);
    CHECK_NEAR(output.a[7], 0.0, 1e-10);
}

static void test_velocity_second_step_direct_branches(void) {
    ruckig_profile_t input;
    ruckig_profile_t output;
    double duration = 0.0;

    ruckig_profile_init(&input);
    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.0, 1.0, 0.0);
    CHECK_TRUE(!ruckig_velocity_second_step1_get_profile(NULL, &output, &duration, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_velocity_second_step1_get_profile(&input, NULL, &duration, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_velocity_second_step1_get_profile(&input, &output, NULL, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_velocity_second_step1_get_profile(&input, &output, &duration, 0.0, 1.0, 0.0, 0.0));

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_velocity_second_step1_get_profile(&input, &output, &duration, 0.0, 0.0, 1.0, -1.0));
    CHECK_NEAR(duration, 0.0, 1e-12);
    CHECK_NEAR(output.v[7], 0.0, 1e-12);

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 0.0, 0.0, 1.0, 0.0);
    CHECK_TRUE(ruckig_velocity_second_step1_get_profile(&input, &output, &duration, 0.0, 1.0, 1.0, -1.0));
    CHECK_NEAR(duration, 1.0, 1e-12);
    CHECK_NEAR(output.v[7], 1.0, 1e-12);

    ruckig_profile_set_boundary_for_velocity(&input, 0.0, 1.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_velocity_second_step1_get_profile(&input, &output, &duration, 1.0, 0.0, 1.0, -1.0));
    CHECK_NEAR(duration, 1.0, 1e-12);
    CHECK_NEAR(output.v[7], 0.0, 1e-12);

    CHECK_TRUE(!ruckig_velocity_second_step2_get_profile(NULL, 1.0, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_velocity_second_step2_get_profile(&output, 0.0, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_velocity_second_step2_get_profile(&output, INFINITY, 0.0, 1.0, 1.0, -1.0));
    CHECK_TRUE(!ruckig_velocity_second_step2_get_profile(&output, 0.25, 0.0, 1.0, 1.0, -1.0));

    ruckig_profile_set_boundary_for_velocity(&output, 0.0, 0.0, 0.0, 1.0, 0.0);
    CHECK_TRUE(ruckig_velocity_second_step2_get_profile(&output, 1.0, 0.0, 1.0, 1.0, -1.0));
    CHECK_NEAR(output.v[7], 1.0, 1e-12);

    ruckig_profile_set_boundary_for_velocity(&output, 0.0, 1.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_velocity_second_step2_get_profile(&output, 1.0, 1.0, 0.0, 1.0, -1.0));
    CHECK_NEAR(output.v[7], 0.0, 1e-12);
}

void run_solver_branch_coverage_tests(void) {
    test_block_invalid_and_single_profile();
    test_block_two_profile_tie_and_interval();
    test_block_three_profile_interval();
    test_block_four_profile_duplicate_removal();
    test_block_five_profile_interval_pairing();
    test_brake_solver_adjacent_branches();
    test_position_first_step_direct_branches();
    test_position_second_step_direct_branches();
    test_position_third_step_direct_branches();
    test_velocity_second_step_direct_branches();
    test_velocity_third_step_direct_branches();
}
