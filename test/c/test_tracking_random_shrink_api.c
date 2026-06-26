#include "test_tracking_random_internal.h"

typedef struct tracking_audit_failure_signature {
    bool failed;
    int failure_count;
    const char* failure_class;
    tracking_audit_case_result_t case_result;
} tracking_audit_failure_signature_t;

static bool tracking_audit_case_passes(
    const tracking_audit_case_config_t* config,
    tracking_audit_case_result_t* case_result
) {
    const int failures_before = ruckig_c_test_failures;
    ruckig_c_test_failures = 0;
    run_tracking_audit_case(config, case_result);
    {
        const bool passed = ruckig_c_test_failures == 0;
        ruckig_c_test_failures = failures_before;
        return passed;
    }
}

static bool try_tracking_audit_shrink_candidate(
    tracking_audit_case_config_t* current,
    const tracking_audit_case_config_t* candidate,
    const char* label,
    size_t* accepted_count
) {
    tracking_audit_case_result_t case_result;
    if (tracking_audit_case_passes(candidate, &case_result)) {
        *current = *candidate;
        ++*accepted_count;
        printf("tracking random audit shrink accepted %s\n", label);
        return true;
    }
    return false;
}

static const char* tracking_audit_failure_class(
    const tracking_audit_case_config_t* config,
    const tracking_audit_case_result_t* case_result,
    int failure_count
) {
    if (failure_count == 0) {
        return "none";
    }
    if (case_result->result != RUCKIG_WORKING && case_result->result != RUCKIG_FINISHED) {
        return "result";
    }
    if (!tracking_optimized_status_is_success(case_result->status)) {
        return "calculation-status";
    }
    if (case_result->diagnostics.mode != RUCKIG_TRACKING_OPTIMIZED) {
        return "diagnostics-mode";
    }
    if (case_result->diagnostics.optimized_strategy != config->strategy) {
        return "diagnostics-strategy";
    }
    if (case_result->diagnostics.error_step_count != 0) {
        return "error-step";
    }
    if (case_result->diagnostics.candidate_count == 0) {
        return "candidate-count";
    }
    if (case_result->diagnostics.valid_candidate_count > case_result->diagnostics.candidate_count) {
        return "candidate-accounting";
    }
    return "tracking-audit-invariant";
}

static void tracking_audit_case_failure_signature(
    const tracking_audit_case_config_t* config,
    tracking_audit_failure_signature_t* signature
) {
    const int failures_before = ruckig_c_test_failures;
    memset(signature, 0, sizeof(*signature));
    ruckig_c_test_failures = 0;
    run_tracking_audit_case(config, &signature->case_result);
    signature->failure_count = ruckig_c_test_failures;
    signature->failed = signature->failure_count != 0;
    signature->failure_class = tracking_audit_failure_class(config, &signature->case_result, signature->failure_count);
    ruckig_c_test_failures = failures_before;
}

static bool try_tracking_audit_failure_shrink_candidate(
    tracking_audit_case_config_t* current,
    const tracking_audit_case_config_t* candidate,
    const char* label,
    const char* expected_failure_class,
    size_t* accepted_count
) {
    tracking_audit_failure_signature_t signature;
    tracking_audit_case_failure_signature(candidate, &signature);
    if (signature.failed && strcmp(signature.failure_class, expected_failure_class) == 0) {
        *current = *candidate;
        ++*accepted_count;
        printf(
            "tracking random audit failure shrink accepted %s failure_class=\"%s\" failure_count=%d\n",
            label,
            signature.failure_class,
            signature.failure_count
        );
        return true;
    }
    return false;
}

static void shrink_tracking_audit_case(
    tracking_audit_case_config_t* config,
    size_t* accepted_count
) {
    size_t i;
    static const size_t dof_targets[] = {1, 2, 4};
    static const size_t lookahead_targets[] = {1, 2, 5};
    static const double reactiveness_targets[] = {0.0, 0.25, 0.5};

    for (i = 0; i < sizeof(dof_targets) / sizeof(dof_targets[0]); ++i) {
        if (dof_targets[i] < config->dofs) {
            tracking_audit_case_config_t candidate = *config;
            candidate.dofs = dof_targets[i];
            if (candidate.has_disabled_dof && candidate.disabled_dof >= candidate.dofs) {
                candidate.has_disabled_dof = false;
                candidate.disabled_dof = 0;
            }
            if (try_tracking_audit_shrink_candidate(config, &candidate, "dofs", accepted_count)) {
                break;
            }
        }
    }

    for (i = 0; i < sizeof(lookahead_targets) / sizeof(lookahead_targets[0]); ++i) {
        if (lookahead_targets[i] < config->lookahead_count) {
            tracking_audit_case_config_t candidate = *config;
            candidate.lookahead_count = lookahead_targets[i];
            if (try_tracking_audit_shrink_candidate(config, &candidate, "lookahead-count", accepted_count)) {
                break;
            }
        }
    }

    if (config->has_disabled_dof) {
        tracking_audit_case_config_t candidate = *config;
        candidate.has_disabled_dof = false;
        candidate.disabled_dof = 0;
        try_tracking_audit_shrink_candidate(config, &candidate, "disabled-dof-mask", accepted_count);
    }

    if (config->tight_constraints) {
        tracking_audit_case_config_t candidate = *config;
        candidate.tight_constraints = false;
        try_tracking_audit_shrink_candidate(config, &candidate, "tight-constraints", accepted_count);
    }

    if (config->strategy != RUCKIG_TRACKING_OPTIMIZED_STABLE) {
        tracking_audit_case_config_t candidate = *config;
        candidate.strategy = RUCKIG_TRACKING_OPTIMIZED_STABLE;
        try_tracking_audit_shrink_candidate(config, &candidate, "strategy-stable", accepted_count);
    }

    if (config->signal != 0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.signal = 0;
        try_tracking_audit_shrink_candidate(config, &candidate, "signal-ramp", accepted_count);
    }

    for (i = 0; i < sizeof(reactiveness_targets) / sizeof(reactiveness_targets[0]); ++i) {
        if (reactiveness_targets[i] < config->reactiveness) {
            tracking_audit_case_config_t candidate = *config;
            candidate.reactiveness = reactiveness_targets[i];
            if (try_tracking_audit_shrink_candidate(config, &candidate, "reactiveness", accepted_count)) {
                break;
            }
        }
    }

    if (config->start_time != 0.0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.start_time = 0.0;
        try_tracking_audit_shrink_candidate(config, &candidate, "start-time", accepted_count);
    }
}

static void shrink_tracking_audit_failure_case(
    tracking_audit_case_config_t* config,
    const char* expected_failure_class,
    size_t* accepted_count
) {
    size_t i;
    static const size_t dof_targets[] = {1, 2, 4};
    static const size_t lookahead_targets[] = {1, 2, 5};
    static const double reactiveness_targets[] = {0.0, 0.25, 0.5};

    for (i = 0; i < sizeof(dof_targets) / sizeof(dof_targets[0]); ++i) {
        if (dof_targets[i] < config->dofs) {
            tracking_audit_case_config_t candidate = *config;
            candidate.dofs = dof_targets[i];
            if (candidate.has_disabled_dof && candidate.disabled_dof >= candidate.dofs) {
                candidate.has_disabled_dof = false;
                candidate.disabled_dof = 0;
            }
            if (try_tracking_audit_failure_shrink_candidate(
                    config,
                    &candidate,
                    "dofs",
                    expected_failure_class,
                    accepted_count)) {
                break;
            }
        }
    }

    for (i = 0; i < sizeof(lookahead_targets) / sizeof(lookahead_targets[0]); ++i) {
        if (lookahead_targets[i] < config->lookahead_count) {
            tracking_audit_case_config_t candidate = *config;
            candidate.lookahead_count = lookahead_targets[i];
            if (try_tracking_audit_failure_shrink_candidate(
                    config,
                    &candidate,
                    "lookahead-count",
                    expected_failure_class,
                    accepted_count)) {
                break;
            }
        }
    }

    if (config->has_disabled_dof) {
        tracking_audit_case_config_t candidate = *config;
        candidate.has_disabled_dof = false;
        candidate.disabled_dof = 0;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "disabled-dof-mask",
            expected_failure_class,
            accepted_count
        );
    }

    if (config->tight_constraints) {
        tracking_audit_case_config_t candidate = *config;
        candidate.tight_constraints = false;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "tight-constraints",
            expected_failure_class,
            accepted_count
        );
    }

    if (config->strategy != RUCKIG_TRACKING_OPTIMIZED_STABLE) {
        tracking_audit_case_config_t candidate = *config;
        candidate.strategy = RUCKIG_TRACKING_OPTIMIZED_STABLE;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "strategy-stable",
            expected_failure_class,
            accepted_count
        );
    }

    if (config->signal != 0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.signal = 0;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "signal-ramp",
            expected_failure_class,
            accepted_count
        );
    }

    for (i = 0; i < sizeof(reactiveness_targets) / sizeof(reactiveness_targets[0]); ++i) {
        if (reactiveness_targets[i] < config->reactiveness) {
            tracking_audit_case_config_t candidate = *config;
            candidate.reactiveness = reactiveness_targets[i];
            if (try_tracking_audit_failure_shrink_candidate(
                    config,
                    &candidate,
                    "reactiveness",
                    expected_failure_class,
                    accepted_count)) {
                break;
            }
        }
    }

    if (config->start_time != 0.0) {
        tracking_audit_case_config_t candidate = *config;
        candidate.start_time = 0.0;
        try_tracking_audit_failure_shrink_candidate(
            config,
            &candidate,
            "start-time",
            expected_failure_class,
            accepted_count
        );
    }
}

void run_tracking_random_audit_shrink_tests(size_t sample_index, unsigned seed) {
    tracking_audit_case_config_t original;
    tracking_audit_case_config_t reduced;
    tracking_audit_case_result_t original_result;
    tracking_audit_case_result_t reduced_result;
    size_t sample = 0;
    size_t accepted_count = 0;
    const int failures_before = ruckig_c_test_failures;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        make_tracking_audit_case_config(&state, sample, &original);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    run_tracking_audit_case(&original, &original_result);
    if (ruckig_c_test_failures != failures_before) {
        fprintf(
            stderr,
            "tracking random audit shrink original failed seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g\n",
            seed,
            sample_index,
            original.dofs,
            tracking_signal_name(original.signal),
            original.lookahead_count,
            original.reactiveness,
            tracking_strategy_name(original.strategy),
            original.has_disabled_dof ? "yes" : "no",
            original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
            original.tight_constraints ? "tight_valid" : "default",
            original.start_time
        );
        return;
    }

    reduced = original;
    shrink_tracking_audit_case(&reduced, &accepted_count);
    run_tracking_audit_case(&reduced, &reduced_result);

    printf(
        "tracking random audit shrink original seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g\n",
        seed,
        sample_index,
        original.dofs,
        tracking_signal_name(original.signal),
        original.lookahead_count,
        original.reactiveness,
        tracking_strategy_name(original.strategy),
        original.has_disabled_dof ? "yes" : "no",
        original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
        original.tight_constraints ? "tight_valid" : "default",
        original.start_time
    );
    printf(
        "tracking random audit shrink reduced seed=%u sample=%zu accepted=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g\n",
        seed,
        sample_index,
        accepted_count,
        reduced.dofs,
        tracking_signal_name(reduced.signal),
        reduced.lookahead_count,
        reduced.reactiveness,
        tracking_strategy_name(reduced.strategy),
        reduced.has_disabled_dof ? "yes" : "no",
        reduced.has_disabled_dof ? tracking_dof_number_name(reduced.disabled_dof) : "none",
        reduced.tight_constraints ? "tight_valid" : "default",
        reduced.start_time
    );
    printf(
        "tracking random audit shrink replay command: ruckig_c_tests.exe --tracking-random-audit-replay %zu --seed %u\n",
        sample_index,
        seed
    );
    tracking_audit_print_fixture(&reduced, &reduced_result, seed);
}

void run_tracking_random_audit_failure_shrink_tests(size_t sample_index, unsigned seed) {
    tracking_audit_case_config_t original;
    tracking_audit_case_config_t reduced;
    tracking_audit_failure_signature_t original_signature;
    tracking_audit_failure_signature_t reduced_signature;
    size_t sample = 0;
    size_t accepted_count = 0;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        make_tracking_audit_case_config(&state, sample, &original);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    tracking_audit_case_failure_signature(&original, &original_signature);
    if (!original_signature.failed) {
        fprintf(
            stderr,
            "tracking random audit failure shrink original case did not fail seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g; use --tracking-random-audit-shrink for pass-preserving reduction\n",
            seed,
            sample_index,
            original.dofs,
            tracking_signal_name(original.signal),
            original.lookahead_count,
            original.reactiveness,
            tracking_strategy_name(original.strategy),
            original.has_disabled_dof ? "yes" : "no",
            original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
            original.tight_constraints ? "tight_valid" : "default",
            original.start_time
        );
        fprintf(
            stderr,
            "tracking random audit failure shrink replay command: ruckig_c_tests.exe --tracking-random-audit-replay %zu --seed %u\n",
            sample_index,
            seed
        );
        ++ruckig_c_test_failures;
        return;
    }

    reduced = original;
    shrink_tracking_audit_failure_case(&reduced, original_signature.failure_class, &accepted_count);
    tracking_audit_case_failure_signature(&reduced, &reduced_signature);
    if (!reduced_signature.failed || strcmp(reduced_signature.failure_class, original_signature.failure_class) != 0) {
        fprintf(
            stderr,
            "tracking random audit failure shrink reduced case no longer preserves failure class seed=%u sample=%zu original_class=\"%s\" reduced_class=\"%s\"\n",
            seed,
            sample_index,
            original_signature.failure_class,
            reduced_signature.failure_class
        );
        ++ruckig_c_test_failures;
        return;
    }

    printf(
        "tracking random audit failure shrink original seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g failure_class=\"%s\" failure_count=%d\n",
        seed,
        sample_index,
        original.dofs,
        tracking_signal_name(original.signal),
        original.lookahead_count,
        original.reactiveness,
        tracking_strategy_name(original.strategy),
        original.has_disabled_dof ? "yes" : "no",
        original.has_disabled_dof ? tracking_dof_number_name(original.disabled_dof) : "none",
        original.tight_constraints ? "tight_valid" : "default",
        original.start_time,
        original_signature.failure_class,
        original_signature.failure_count
    );
    printf(
        "tracking random audit failure shrink reduced seed=%u sample=%zu accepted=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s constraints=%s start_time=%.9g failure_class=\"%s\" failure_count=%d\n",
        seed,
        sample_index,
        accepted_count,
        reduced.dofs,
        tracking_signal_name(reduced.signal),
        reduced.lookahead_count,
        reduced.reactiveness,
        tracking_strategy_name(reduced.strategy),
        reduced.has_disabled_dof ? "yes" : "no",
        reduced.has_disabled_dof ? tracking_dof_number_name(reduced.disabled_dof) : "none",
        reduced.tight_constraints ? "tight_valid" : "default",
        reduced.start_time,
        reduced_signature.failure_class,
        reduced_signature.failure_count
    );
    printf(
        "tracking random audit failure shrink original replay command: ruckig_c_tests.exe --tracking-random-audit-replay %zu --seed %u\n",
        sample_index,
        seed
    );
    printf("tracking random audit failure shrink reduced replay: paste the initializer below into the fixed tracking audit corpus\n");
    tracking_audit_print_fixture(&reduced, &reduced_signature.case_result, seed);
}
