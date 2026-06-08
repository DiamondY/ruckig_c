#include "test_common.h"

#include "ruckig_c/block.h"
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

static void test_position_second_step_direct_branches(void) {
    ruckig_profile_t input;
    ruckig_profile_t output;
    ruckig_block_t block;
    double duration = 0.0;

    ruckig_profile_init(&input);
    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_TRUE(ruckig_position_second_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0));
    CHECK_TRUE(!block.a.valid);
    CHECK_NEAR(duration, 0.0, 1e-12);

    ruckig_profile_set_boundary(&input, 0.0, 0.0, 0.0, 0.25, 0.5, 0.0);
    CHECK_TRUE(!ruckig_position_second_step1_get_profile(&input, &output, &block, &duration, 0.0, 0.0, 0.25, 0.5, 0.0, 0.0, 1.0, -1.0));

    CHECK_TRUE(!ruckig_position_second_step2_get_profile(NULL, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
    ruckig_profile_set_boundary(&output, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    CHECK_TRUE(!ruckig_position_second_step2_get_profile(&output, -1.0, 0.0, 0.0, 1.0, 0.0, 1.0, -1.0, 1.0, -1.0));
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

    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(NULL, 1.0, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));
    ruckig_profile_set_boundary_for_velocity(&output, 0.0, 0.0, 1.0, 0.5, -1.0);
    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(&output, 0.0, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));
    CHECK_TRUE(!ruckig_velocity_third_step2_get_profile(&output, 1.0, 0.0, 1.0, 0.5, -1.0, 2.0, -2.0, 2.0));
}

void run_solver_branch_coverage_tests(void) {
    test_block_invalid_and_single_profile();
    test_block_two_profile_tie_and_interval();
    test_block_three_profile_interval();
    test_block_four_profile_duplicate_removal();
    test_block_five_profile_interval_pairing();
    test_position_second_step_direct_branches();
    test_velocity_third_step_direct_branches();
}
