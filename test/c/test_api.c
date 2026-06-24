#include "test_api_internal.h"

void run_api_tests(void) {
    run_public_api_tests();
    run_allocation_api_tests();
    run_waypoint_tests();
    run_tracking_tests();
    run_public_api_post_tracking_tests();
}
