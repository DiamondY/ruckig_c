#ifndef RUCKIG_C_TEST_PLATFORM_CLOCK_CUSTOM_PROVIDER_H
#define RUCKIG_C_TEST_PLATFORM_CLOCK_CUSTOM_PROVIDER_H

static unsigned long long ruckig_test_platform_clock_us(void) {
    return 123456789ull;
}

#endif
