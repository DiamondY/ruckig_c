#include "test_common.h"

#include "ruckig_c/alloc.h"
#include "ruckig_c/roots.h"

#include <float.h>
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

static void check_resolvent_root(double a, double b, double c, double root, double tolerance) {
    CHECK_NEAR(((root + a) * root + b) * root + c, 0.0, tolerance);
}

static void check_cubic_roots_residual(double a, double b, double c, double d, double tolerance) {
    ruckig_root_set3_t roots = ruckig_solve_cubic(a, b, c, d);
    size_t i;
    for (i = 0; i < roots.count; ++i) {
        CHECK_TRUE(roots.values[i] >= 0.0);
        CHECK_NEAR(cubic_eval(a, b, c, d, roots.values[i]), 0.0, tolerance);
        if (i > 0) {
            CHECK_TRUE(roots.values[i - 1] <= roots.values[i]);
        }
    }
}

static void check_quartic_roots_residual(double a, double b, double c, double d, double tolerance) {
    ruckig_root_set4_t roots = ruckig_solve_quart_monic(a, b, c, d);
    size_t i;
    for (i = 0; i < roots.count; ++i) {
        CHECK_TRUE(roots.values[i] >= 0.0);
        CHECK_NEAR(quartic_eval(a, b, c, d, roots.values[i]), 0.0, tolerance);
        if (i > 0) {
            CHECK_TRUE(roots.values[i - 1] <= roots.values[i]);
        }
    }
}

static void test_roots_numeric_resolvent_edges(void) {
    double roots[3] = {0.0, 0.0, 0.0};
    int count;

    count = ruckig_solve_resolvent(roots, 0.0, 0.0, 0.0);
    CHECK_TRUE(count >= 1);
    CHECK_NEAR(roots[0], 0.0, 0.0);
    check_resolvent_root(0.0, 0.0, 0.0, roots[0], 0.0);

    count = ruckig_solve_resolvent(roots, -3.0e-8, 3.0e-16, -1.0e-24);
    CHECK_TRUE(count >= 1);
    check_resolvent_root(-3.0e-8, 3.0e-16, -1.0e-24, roots[0], 1e-36);

    count = ruckig_solve_resolvent(roots, 0.0, -3.0e-12, 2.0e-18);
    CHECK_TRUE(count >= 1);
    check_resolvent_root(0.0, -3.0e-12, 2.0e-18, roots[0], 1e-24);
    if (count > 1) {
        check_resolvent_root(0.0, -3.0e-12, 2.0e-18, roots[1], 1e-24);
    }
}

static void test_roots_numeric_cubic_quartic_edges(void) {
    ruckig_root_set3_t cubic_roots;
    ruckig_root_set4_t quartic_roots;

    check_cubic_roots_residual(1.0, -3.0e-8, 3.0e-16, -1.0e-24, 1e-22);
    check_cubic_roots_residual(1.0, -1.0, -2.0, 0.0, 1e-12);
    cubic_roots = ruckig_solve_cubic(1.0, -1.0, -2.0, 0.0);
    CHECK_EQ_INT(cubic_roots.count, 2);
    CHECK_NEAR(cubic_roots.values[0], 0.0, 1e-14);
    CHECK_NEAR(cubic_roots.values[1], 2.0, 1e-12);

    check_quartic_roots_residual(-4.0e-6, 6.0e-12, -4.0e-18, 1.0e-24, 1e-22);
    check_quartic_roots_residual(-2.0, -1.0, 2.0, 0.0, 1e-10);
    quartic_roots = ruckig_solve_quart_monic(-2.0, -1.0, 2.0, 0.0);
    CHECK_TRUE(quartic_roots.count >= 2);
    CHECK_NEAR(quartic_roots.values[0], 0.0, 1e-14);
}

static void test_roots_numeric_audit_does_not_allocate(void) {
    size_t allocations_before;

    ruckig_allocation_counters_reset();
    allocations_before = ruckig_allocation_count();
    ruckig_allocation_forbidden_set(true);

    test_roots_numeric_resolvent_edges();
    test_roots_numeric_cubic_quartic_edges();

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

void run_roots_numeric_audit_tests(void) {
    test_roots_numeric_resolvent_edges();
    test_roots_numeric_cubic_quartic_edges();
    test_roots_numeric_audit_does_not_allocate();
}

void run_api_tests(void);
void run_brake_tests(void);
void run_profile_tests(void);
void run_roots_numeric_audit_tests(void);
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
void run_property_invariant_tests(void);
void run_state_machine_branch_coverage_tests(void);
void run_solver_branch_coverage_tests(void);
void run_tracking_api_tests(void);
void run_tracking_sequence_continuation_api_tests(void);
void run_tracking_sequence_fast_continuation_tests(void);
void run_tracking_sequence_optimized_continuation_tests(void);
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
void run_tracking_random_replay_tests(size_t sample, unsigned seed);
void run_tracking_random_audit_replay_tests(size_t sample, unsigned seed);
void run_tracking_random_audit_shrink_tests(size_t sample, unsigned seed);

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
    if (argc == 5 && strcmp(argv[1], "--tracking-random-replay") == 0 && strcmp(argv[3], "--seed") == 0) {
        const unsigned long sample = strtoul(argv[2], NULL, 10);
        const unsigned long seed = strtoul(argv[4], NULL, 10);
        run_tracking_random_replay_tests((size_t)sample, (unsigned)seed);
        return ruckig_c_test_failures == 0 ? 0 : 1;
    }
    if (argc == 5 && strcmp(argv[1], "--tracking-random-audit-replay") == 0 && strcmp(argv[3], "--seed") == 0) {
        const unsigned long sample = strtoul(argv[2], NULL, 10);
        const unsigned long seed = strtoul(argv[4], NULL, 10);
        run_tracking_random_audit_replay_tests((size_t)sample, (unsigned)seed);
        return ruckig_c_test_failures == 0 ? 0 : 1;
    }
    if (argc == 5 && strcmp(argv[1], "--tracking-random-audit-shrink") == 0 && strcmp(argv[3], "--seed") == 0) {
        const unsigned long sample = strtoul(argv[2], NULL, 10);
        const unsigned long seed = strtoul(argv[4], NULL, 10);
        run_tracking_random_audit_shrink_tests((size_t)sample, (unsigned)seed);
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
        if (strcmp(argv[1], "--property-invariants") == 0) {
            run_property_invariant_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--roots-numeric-audit") == 0) {
            run_roots_numeric_audit_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--state-machine-branch-coverage") == 0) {
            run_state_machine_branch_coverage_tests();
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
        if (strcmp(argv[1], "--tracking-sequence-continuation-api") == 0) {
            run_tracking_sequence_continuation_api_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-sequence-fast-continuation") == 0) {
            run_tracking_sequence_fast_continuation_tests();
            return ruckig_c_test_failures == 0 ? 0 : 1;
        }
        if (strcmp(argv[1], "--tracking-sequence-optimized-continuation") == 0) {
            run_tracking_sequence_optimized_continuation_tests();
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
        fprintf(stderr, "usage: ruckig_c_tests [--waypoint|--per-section|--waypoint-quality|--waypoint-resume-stress|--waypoint-resume-quality-audit|--interrupt-boundary-audit|--no-waypoint-interrupt-audit|--interrupt-post-release-quality|--property-invariants|--roots-numeric-audit|--state-machine-branch-coverage|--solver-branch-coverage|--tracking|--tracking-api|--tracking-sequence-continuation-api|--tracking-validation|--tracking-online|--tracking-interrupt-audit|--tracking-fixed-corpus|--tracking-offline|--tracking-optimized|--tracking-quality|--tracking-quality-hardening|--tracking-stability|--tracking-no-allocation|--tracking-random N --seed S|--tracking-random-audit N --seed S|--tracking-random-replay SAMPLE --seed S|--tracking-random-audit-replay SAMPLE --seed S|--tracking-random-audit-shrink SAMPLE --seed S]\n");
        return 2;
    }

    run_api_tests();
    run_brake_tests();
    run_profile_tests();
    run_utils_tests();
    run_roots_tests();
    return ruckig_c_test_failures == 0 ? 0 : 1;
}
