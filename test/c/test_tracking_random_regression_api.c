#include "test_tracking_random_internal.h"

typedef struct tracking_stability_case {
    const char* name;
    tracking_audit_case_config_t config;
    ruckig_tracking_calculation_status_t expected_status;
    size_t expected_family;
    bool expected_near_tie;
    bool expect_budget_exhausted;
    bool expect_positive_improvement;
} tracking_stability_case_t;

static void test_tracking_random_audit_fixed_cases(void) {
    const tracking_audit_case_config_t cases[] = {
        {6, 2, 1, 1, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 0, true, 0.06},
        {12, 2, 5, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, true, 1, true, 0.12},
        {22, 8, 5, 0, 0.25, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 7, true, 0.22},
        {2600, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, false, 0.0},
        {8011, 4, 10, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 3, true, 0.11},
        {9800, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.0},
        {1614, 8, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, false, 0.14},
        {0, 8, 5, 0, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 6, true, 0.0},
        {4, 4, 2, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, true, 0.04},
        {8, 1, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, true, 0.08},
        {10, 1, 10, 1, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, true, 0.10},
        {16, 1, 5, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.16}
    };
    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        tracking_audit_case_result_t case_result;
        size_t selected = 0;
        size_t family;
        run_tracking_audit_case(&cases[i], &case_result);
        CHECK_TRUE(tracking_optimized_status_is_success(case_result.status));
        CHECK_EQ_INT(case_result.diagnostics.fallback_step_count + case_result.diagnostics.optimized_step_count, 1);
        CHECK_EQ_INT(case_result.diagnostics.error_step_count, 0);
        CHECK_TRUE(case_result.diagnostics.candidate_count >= 1);
        CHECK_TRUE(case_result.selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT);
        for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
            selected += case_result.family_selected[family];
        }
        CHECK_EQ_INT(selected, 1);
        if (case_result.selected_near_tie) {
            CHECK_EQ_INT(case_result.config.strategy, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE);
            CHECK_TRUE(case_result.near_tie_accepted_count > 0);
        }
    }
}

static void test_tracking_stability_regression_cases(void) {
    const tracking_stability_case_t cases[] = {
        {
            "stable tight disabled strict",
            {6, 2, 1, 1, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 0, true, 0.06},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            false,
            true
        },
        {
            "balanced tight disabled strict",
            {12, 2, 5, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, true, 1, true, 0.12},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "aggressive tight disabled strict",
            {22, 8, 5, 0, 0.25, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 7, true, 0.22},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "stable fallback sinus",
            {2600, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, false, 0.0},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "stable disabled tight fallback",
            {8011, 4, 10, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_STABLE, true, 3, true, 0.11},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "balanced fallback sinus",
            {9800, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.0},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "aggressive fallback ramp",
            {1614, 8, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, false, 0.14},
            RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK,
            0,
            false,
            true,
            false
        },
        {
            "aggressive disabled tight strict",
            {0, 8, 5, 0, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 6, true, 0.0},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "balanced tight strict",
            {4, 4, 2, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, true, 0.04},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            false,
            true
        },
        {
            "stable tight strict",
            {8, 1, 10, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, true, 0.08},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "aggressive strict budget",
            {10, 1, 10, 1, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, false, 0, true, 0.10},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "balanced half sinus strict",
            {16, 1, 5, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.16},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            false,
            true,
            true
        },
        {
            "seed1 derivative damped representative",
            {1, 4, 1, 0, 1.0, RUCKIG_TRACKING_OPTIMIZED_STABLE, false, 0, true, 0.01},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            4,
            false,
            false,
            true
        },
        {
            "seed1 horizon representative",
            {8, 2, 10, 2, 1.0, RUCKIG_TRACKING_OPTIMIZED_BALANCED, false, 0, false, 0.08},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            2,
            false,
            true,
            true
        },
        {
            "seed1 lead lag representative",
            {403, 2, 1, 3, 0.5, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 1, false, 0.03},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            5,
            false,
            false,
            true
        },
        {
            "seed1 terminal blend representative",
            {602, 2, 5, 3, 0.25, RUCKIG_TRACKING_OPTIMIZED_BALANCED, true, 1, false, 0.02},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            3,
            false,
            true,
            true
        },
        {
            "seed1 aggressive near tie representative",
            {1400, 2, 5, 1, 0.0, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE, true, 1, false, 0.0},
            RUCKIG_TRACKING_CALCULATION_OPTIMIZED,
            1,
            true,
            true,
            true
        }
    };
    bool strategy_seen[3] = {false, false, false};
    bool signal_seen[4] = {false, false, false, false};
    bool lookahead_seen[4] = {false, false, false, false};
    bool family_seen[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT] = {false, false, false, false, false, false};
    bool optimized_seen = false;
    bool fallback_seen = false;
    bool near_tie_seen = false;
    bool disabled_seen = false;
    bool tight_seen = false;
    bool budget_seen = false;
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        tracking_audit_case_result_t case_result;
        size_t selected = 0;
        size_t family;
        run_tracking_audit_case(&cases[i].config, &case_result);
        printf(
            "tracking stability case %s: status=%s selected_family=%s selected_near_tie=%s candidates=%zu budget_exhausted=%zu improvement=%.9g\n",
            cases[i].name,
            tracking_status_name(case_result.status),
            case_result.selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT
                ? tracking_audit_family_names[case_result.selected_family]
                : "unknown",
            case_result.selected_near_tie ? "yes" : "no",
            case_result.diagnostics.candidate_count,
            case_result.diagnostics.budget_exhausted_count,
            case_result.diagnostics.improvement_ratio
        );
        CHECK_EQ_INT(case_result.status, cases[i].expected_status);
        CHECK_EQ_INT(case_result.selected_family, cases[i].expected_family);
        CHECK_EQ_INT(case_result.selected_near_tie, cases[i].expected_near_tie);
        CHECK_EQ_INT(case_result.config.strategy, cases[i].config.strategy);
        CHECK_EQ_INT(case_result.config.has_disabled_dof, cases[i].config.has_disabled_dof);
        CHECK_EQ_INT(case_result.config.tight_constraints, cases[i].config.tight_constraints);
        CHECK_EQ_INT(case_result.diagnostics.fast_candidate_count, 1);
        CHECK_TRUE(case_result.diagnostics.candidate_count >= 1);
        CHECK_TRUE(case_result.diagnostics.valid_candidate_count >= 1);
        CHECK_EQ_INT(case_result.diagnostics.rejected_candidate_count, 0);
        for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
            selected += case_result.family_selected[family];
        }
        CHECK_EQ_INT(selected, 1);
        if (cases[i].expected_status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
            CHECK_EQ_INT(case_result.diagnostics.optimized_step_count, 1);
            CHECK_EQ_INT(case_result.diagnostics.fallback_step_count, 0);
            optimized_seen = true;
        } else {
            CHECK_EQ_INT(case_result.diagnostics.optimized_step_count, 0);
            CHECK_EQ_INT(case_result.diagnostics.fallback_step_count, 1);
            fallback_seen = true;
        }
        if (cases[i].expected_near_tie) {
            CHECK_EQ_INT(case_result.config.strategy, RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE);
            CHECK_TRUE(case_result.near_tie_accepted_count > 0);
            near_tie_seen = true;
        } else {
            CHECK_EQ_INT(case_result.selected_near_tie, false);
        }
        if (cases[i].expect_budget_exhausted) {
            CHECK_TRUE(case_result.diagnostics.budget_exhausted_count > 0);
            budget_seen = true;
        } else {
            CHECK_EQ_INT(case_result.diagnostics.budget_exhausted_count, 0);
        }
        if (cases[i].expect_positive_improvement) {
            CHECK_TRUE(case_result.diagnostics.improvement_ratio > 0.0);
        }
        strategy_seen[tracking_strategy_index(case_result.config.strategy)] = true;
        signal_seen[(size_t)case_result.config.signal] = true;
        lookahead_seen[tracking_lookahead_index(case_result.config.lookahead_count)] = true;
        family_seen[case_result.selected_family] = true;
        disabled_seen = disabled_seen || case_result.config.has_disabled_dof;
        tight_seen = tight_seen || case_result.config.tight_constraints;
    }

    CHECK_TRUE(strategy_seen[0] && strategy_seen[1] && strategy_seen[2]);
    CHECK_TRUE(signal_seen[0] && signal_seen[1] && signal_seen[2] && signal_seen[3]);
    CHECK_TRUE(lookahead_seen[0] && lookahead_seen[1] && lookahead_seen[2] && lookahead_seen[3]);
    CHECK_TRUE(
        family_seen[0]
        && family_seen[1]
        && family_seen[2]
        && family_seen[3]
        && family_seen[4]
        && family_seen[5]
    );
    CHECK_TRUE(optimized_seen);
    CHECK_TRUE(fallback_seen);
    CHECK_TRUE(near_tie_seen);
    CHECK_TRUE(disabled_seen);
    CHECK_TRUE(tight_seen);
    CHECK_TRUE(budget_seen);
}

void run_tracking_random_audit_fixed_case_tests(void) {
    test_tracking_random_audit_fixed_cases();
}

void run_tracking_quality_hardening_tests(void) {
    test_tracking_random_audit_fixed_cases();
    run_tracking_random_audit_tests(10000, 1u);
}

void run_tracking_stability_tests(void) {
    test_tracking_stability_regression_cases();
}
