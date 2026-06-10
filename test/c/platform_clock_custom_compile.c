#define RUCKIG_C_PLATFORM_CLOCK_HEADER "platform_clock_custom_provider.h"
#define RUCKIG_C_CUSTOM_MONOTONIC_TIME_US ruckig_test_platform_clock_us

#include "ruckig_c/platform_clock.h"

int main(void) {
    return ruckig_platform_monotonic_time_us() == 123456789ull ? 0 : 1;
}
