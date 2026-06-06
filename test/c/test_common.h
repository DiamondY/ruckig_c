#ifndef RUCKIG_C_TEST_COMMON_H
#define RUCKIG_C_TEST_COMMON_H

#include <math.h>
#include <stdio.h>

extern int ruckig_c_test_failures;

#define CHECK_TRUE(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #expr); \
            ++ruckig_c_test_failures; \
        } \
    } while (0)

#define CHECK_EQ_INT(actual, expected) \
    do { \
        long long actual_value = (long long)(actual); \
        long long expected_value = (long long)(expected); \
        if (actual_value != expected_value) { \
            fprintf(stderr, "%s:%d: check failed: %s == %s (%lld != %lld)\n", __FILE__, __LINE__, #actual, #expected, actual_value, expected_value); \
            ++ruckig_c_test_failures; \
        } \
    } while (0)

#define CHECK_NEAR(actual, expected, tolerance) \
    do { \
        double actual_value = (double)(actual); \
        double expected_value = (double)(expected); \
        double tolerance_value = (double)(tolerance); \
        if (fabs(actual_value - expected_value) > tolerance_value) { \
            fprintf(stderr, "%s:%d: check failed: %s ~= %s (%.17g != %.17g, tol %.17g)\n", __FILE__, __LINE__, #actual, #expected, actual_value, expected_value, tolerance_value); \
            ++ruckig_c_test_failures; \
        } \
    } while (0)

#endif
