#include "test_api_internal.h"

static void test_create_destroy(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;

    CHECK_TRUE(RUCKIG_RESULT_IS_OK(RUCKIG_WORKING));
    CHECK_TRUE(RUCKIG_RESULT_IS_OK(RUCKIG_FINISHED));
    CHECK_TRUE(!RUCKIG_RESULT_IS_OK(RUCKIG_ERROR_INVALID_INPUT));

    CHECK_EQ_INT(ruckig_create(NULL, 3, 0.005), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_create(&otg, 0, 0.005), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_create(&otg, 3, 0.005), RUCKIG_WORKING);
    CHECK_TRUE(otg != NULL);

    CHECK_EQ_INT(ruckig_input_create(NULL, 3), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_create(&input, 0), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_input_create(&input, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_get_dof_count(input), 3);

    CHECK_EQ_INT(ruckig_output_create(&output, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_get_dof_count(output), 3);

    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 3), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_get_dof_count(trajectory), 3);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
    ruckig_trajectory_destroy(NULL);
    ruckig_output_destroy(NULL);
    ruckig_input_destroy(NULL);
    ruckig_destroy(NULL);
}

static void test_null_handles_and_invalid_queries(void) {
    ruckig_t* otg = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;
    ruckig_trajectory_t* trajectory = NULL;
    double position[1] = {0.0};
    double duration[1] = {0.0};
    ruckig_position_extrema_t extrema[1];
    double time = 0.0;
    bool found = false;

    CHECK_EQ_INT(ruckig_calculate(NULL, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(NULL, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_validate_input(NULL, NULL, false, false), RUCKIG_ERROR_INVALID_INPUT);
    ruckig_reset(NULL);
    ruckig_output_pass_to_input(NULL, NULL);

    CHECK_EQ_INT(ruckig_input_get_dof_count(NULL), 0);
    CHECK_TRUE(ruckig_input_current_position_data(NULL) == NULL);
    CHECK_TRUE(ruckig_input_current_position_const_data(NULL) == NULL);
    CHECK_TRUE(ruckig_input_enabled_data(NULL) == NULL);
    CHECK_TRUE(ruckig_input_enabled_const_data(NULL) == NULL);
    CHECK_EQ_INT(ruckig_output_get_dof_count(NULL), 0);
    CHECK_TRUE(ruckig_output_new_position_data(NULL) == NULL);
    CHECK_EQ_INT(ruckig_trajectory_get_dof_count(NULL), 0);
    CHECK_NEAR(ruckig_trajectory_get_duration(NULL), 0.0, 0.0);

    CHECK_EQ_INT(ruckig_create(&otg, 1, 0.1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, 1), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_trajectory_create(&trajectory, 1), RUCKIG_WORKING);

    CHECK_EQ_INT(ruckig_calculate(NULL, input, trajectory), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(otg, NULL, trajectory), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_calculate(otg, input, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(NULL, input, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(otg, NULL, output), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_update(otg, input, NULL), RUCKIG_ERROR_INVALID_INPUT);

    CHECK_EQ_INT(ruckig_trajectory_at_time(trajectory, 0.0, position, NULL, NULL, NULL, NULL), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_independent_min_durations(trajectory, duration, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_position_extrema(trajectory, extrema, 1), RUCKIG_ERROR_INVALID_INPUT);
    CHECK_EQ_INT(ruckig_trajectory_get_first_time_at_position(trajectory, 0, 0.0, 0.0, &time, &found), RUCKIG_ERROR_INVALID_INPUT);

    ruckig_trajectory_destroy(trajectory);
    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_destroy(otg);
}

void run_public_core_api_tests(void) {
    test_create_destroy();
    test_null_handles_and_invalid_queries();
}
