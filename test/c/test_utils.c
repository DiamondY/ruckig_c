#include "test_common.h"

#include "ruckig_c/utils.h"

static void test_integrate(void) {
    double p;
    double v;
    double a;

    ruckig_integrate(2.0, 1.0, -0.5, 0.25, 0.75, &p, &v, &a);
    CHECK_NEAR(p, 1.5, 1e-14);
    CHECK_NEAR(v, 1.5, 1e-14);
    CHECK_NEAR(a, 1.75, 1e-14);
    CHECK_NEAR(ruckig_pow2(-3.0), 9.0, 0.0);
}

void run_utils_tests(void) {
    test_integrate();
}
