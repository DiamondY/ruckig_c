#include "test_common.h"

#include "ruckig_c/alloc.h"
#include "ruckig_c/roots.h"

#include <stdlib.h>
#include <string.h>

int ruckig_c_test_failures = 0;

static double cubic_eval(double a, double b, double c, double d, double x) {
    return ((a * x + b) * x + c) * x + d;
}

static double quartic_eval(double a, double b, double c, double d, double x) {
    return (((x + a) * x + b) * x + c) * x + d;
}

static void test_cubic(void) {
    ruckig_root_set3_t roots = ruckig_solve_cubic(1.0, -6.0, 11.0, -6.0);
    CHECK_EQ_INT(roots.count, 3);
    CHECK_NEAR(roots.values[0], 1.0, 1e-12);
    CHECK_NEAR(roots.values[1], 2.0, 1e-12);
    CHECK_NEAR(roots.values[2], 3.0, 1e-12);
    CHECK_NEAR(cubic_eval(1.0, -6.0, 11.0, -6.0, roots.values[0]), 0.0, 1e-10);

    roots = ruckig_solve_cubic(0.0, 1.0, -5.0, 6.0);
    CHECK_EQ_INT(roots.count, 2);
    CHECK_NEAR(roots.values[0], 2.0, 1e-12);
    CHECK_NEAR(roots.values[1], 3.0, 1e-12);

    roots = ruckig_solve_cubic(1.0, 0.0, -1.0, 0.0);
    CHECK_EQ_INT(roots.count, 2);
    CHECK_NEAR(roots.values[0], 0.0, 1e-12);
    CHECK_NEAR(roots.values[1], 1.0, 1e-12);
}

static void test_quartic(void) {
    ruckig_root_set4_t roots = ruckig_solve_quart_monic(-10.0, 35.0, -50.0, 24.0);
    CHECK_EQ_INT(roots.count, 4);
    CHECK_NEAR(roots.values[0], 1.0, 1e-10);
    CHECK_NEAR(roots.values[1], 2.0, 1e-10);
    CHECK_NEAR(roots.values[2], 3.0, 1e-10);
    CHECK_NEAR(roots.values[3], 4.0, 1e-10);
    CHECK_NEAR(quartic_eval(-10.0, 35.0, -50.0, 24.0, roots.values[2]), 0.0, 1e-8);

    roots = ruckig_solve_quart_monic(0.0, -5.0, 0.0, 4.0);
    CHECK_EQ_INT(roots.count, 2);
    CHECK_NEAR(roots.values[0], 1.0, 1e-10);
    CHECK_NEAR(roots.values[1], 2.0, 1e-10);
}

static void test_polynomial_helpers(void) {
    const double p[3] = {1.0, -3.0, 2.0};
    double d[2] = {0.0, 0.0};
    const double q[2] = {1.0, -2.0};
    double root;

    CHECK_NEAR(ruckig_poly_eval(p, 3, 0.0), 2.0, 0.0);
    CHECK_NEAR(ruckig_poly_eval(p, 3, 1.0), 0.0, 0.0);
    CHECK_NEAR(ruckig_poly_eval(p, 3, 2.0), 0.0, 0.0);

    ruckig_poly_derivative(p, 3, d);
    CHECK_NEAR(d[0], 2.0, 0.0);
    CHECK_NEAR(d[1], -3.0, 0.0);

    root = ruckig_shrink_interval(q, 2, 0.0, 4.0);
    CHECK_NEAR(root, 2.0, 1e-14);
}

static void test_roots_do_not_allocate(void) {
    double p[2] = {1.0, -2.0};
    size_t allocations_before;

    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);

    (void)ruckig_solve_cubic(1.0, -6.0, 11.0, -6.0);
    (void)ruckig_solve_quart_monic(-10.0, 35.0, -50.0, 24.0);
    (void)ruckig_shrink_interval(p, 2, 0.0, 4.0);

    CHECK_EQ_INT(ruckig_allocation_count(), allocations_before);
    CHECK_EQ_INT(ruckig_allocation_forbidden_count(), 0);
    ruckig_allocation_forbidden_set(false);
}

void run_roots_tests(void) {
    test_cubic();
    test_quartic();
    test_polynomial_helpers();
    test_roots_do_not_allocate();
}

void run_api_tests(void);
void run_brake_tests(void);
void run_profile_tests(void);
void run_utils_tests(void);
void run_waypoint_tests(void);
void run_waypoint_per_section_tests(void);
void run_waypoint_quality_tests(void);
void run_waypoint_resume_stress_tests(void);
void run_waypoint_resume_quality_tests(void);
void run_waypoint_resume_quality_baseline_dump(void);
void run_interrupt_boundary_audit_tests(void);
void run_no_waypoint_interrupt_audit_tests(void);
void run_interrupt_post_release_quality_tests(void);
void run_solver_branch_coverage_tests(void);
void run_tracking_api_tests(void);
void run_tracking_validation_tests(void);
void run_tracking_online_tests(void);
void run_tracking_interrupt_audit_tests(void);
void run_tracking_fixed_corpus_tests(void);
void run_tracking_offline_tests(void);
void run_tracking_optimized_tests(void);
void run_tracking_quality_tests(void);
void run_tracking_quality_hardening_tests(void);
void run_tracking_stability_tests(void);
void run_tracking_no_allocation_tests(void);
void run_tracking_tests(void);
void run_tracking_random_tests(size_t samples, unsigned seed);
void run_tracking_random_audit_tests(size_t samples, unsigned seed);

int main(int argc, char** argv) {
    if (argc == 5 && strcmp(argv[1], "--tracking-random") == 0 && strcmp(argv[3], "--seed") == 0) {
        const unsigned long samples = strtoul(argv[2], NULL, 10);
        const unsigned long seed = strtoul(argv[4], NULL, 10);
        run_tracking_random_tests((size_t)samples, (unsigned)seed);
        return ruckig_c_test_failures == 0 ? 0 : 1;
    }
    if (argc == 5 && strcmp(argv[1], "--tracking-random-audit") == 0 && strcmp(argv[3], "--seed") == 0) {
        const unsigned long samples = strtoul(argv[2], NULL, 10);
        const unsigned long seed = strtoul(argv[4], NULL, 10);
        run_tracking_random_audit_tests((size_t)samples, (unsigned)seed);
        return ruckig_c_test_failures == 0 ? 0 : 1;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "--waypoint-resume-quality-dump") == 0) {
            run_waypoint_resume_quality_baseline_dump();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--waypoint") == 0) {
            run_waypoint_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--per-section") == 0) {
            run_waypoint_per_section_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--waypoint-quality") == 0) {
            run_waypoint_quality_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--waypoint-resume-stress") == 0) {
            run_waypoint_resume_stress_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--waypoint-resume-quality-audit") == 0) {
            run_waypoint_resume_quality_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--interrupt-boundary-audit") == 0) {
            run_interrupt_boundary_audit_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--no-waypoint-interrupt-audit") == 0) {
            run_no_waypoint_interrupt_audit_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--interrupt-post-release-quality") == 0) {
            run_interrupt_post_release_quality_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--solver-branch-coverage") == 0) {
            run_solver_branch_coverage_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking") == 0) {
            run_tracking_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-api") == 0) {
            run_tracking_api_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-validation") == 0) {
            run_tracking_validation_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-online") == 0) {
            run_tracking_online_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-interrupt-audit") == 0) {
            run_tracking_interrupt_audit_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-fixed-corpus") == 0) {
            run_tracking_fixed_corpus_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-offline") == 0) {
            run_tracking_offline_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-optimized") == 0) {
            run_tracking_optimized_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-quality") == 0) {
            run_tracking_quality_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-quality-hardening") == 0) {
            run_tracking_quality_hardening_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-stability") == 0) {
            run_tracking_stability_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-no-allocation") == 0) {
            run_tracking_no_allocation_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        fprintf(stderr, "unknown test selection: %s\n", argv[1]);
        return 2;
    }
    if (argc > 2) {
        fprintf(stderr, "usage: ruckig_c_tests [--waypoint|--per-section|--waypoint-quality|--waypoint-resume-stress|--waypoint-resume-quality-audit|--interrupt-boundary-audit|--no-waypoint-interrupt-audit|--interrupt-post-release-quality|--solver-branch-coverage|--tracking|--tracking-api|--tracking-validation|--tracking-online|--tracking-interrupt-audit|--tracking-fixed-corpus|--tracking-offline|--tracking-optimized|--tracking-quality|--tracking-quality-hardening|--tracking-stability|--tracking-no-allocation|--tracking-random N --seed S|--tracking-random-audit N --seed S]\n");
        return 2;
    }

    run_api_tests();
    run_brake_tests();
    run_profile_tests();
    run_utils_tests();
    run_roots_tests();
    return ruckig_c_test_failures == 0 ? 0 : 1;
}
