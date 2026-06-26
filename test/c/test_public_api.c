#include "test_api_internal.h"

void run_public_api_tests(void) {
    run_public_core_api_tests();
    run_constructor_boundary_tests();
    run_public_input_api_tests();
    run_public_validation_api_tests();
    run_public_trajectory_api_tests();
}

void run_public_api_post_tracking_tests(void) {
    run_public_trajectory_post_tracking_tests();
}
