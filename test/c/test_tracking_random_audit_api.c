#include "test_tracking_random_internal.h"

typedef struct tracking_audit_bucket {
    size_t samples;
    size_t optimized;
    size_t fallback;
    size_t candidates;
    size_t valid;
    size_t rejected;
    size_t budget_exhausted;
    size_t family_attempted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_valid[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_strict_improved[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_near_tie_accepted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_selected[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t strict_improved_count;
    size_t near_tie_accepted_count;
    double improvement_sum;
} tracking_audit_bucket_t;

typedef struct tracking_audit_stats {
    tracking_audit_bucket_t overall;
    tracking_audit_bucket_t by_strategy[3];
    tracking_audit_bucket_t by_dof[4];
    tracking_audit_bucket_t by_signal[4];
    tracking_audit_bucket_t by_lookahead[4];
    tracking_audit_bucket_t by_reactiveness[4];
    tracking_audit_bucket_t by_disabled[2];
    tracking_audit_bucket_t by_constraints[2];
} tracking_audit_stats_t;

typedef struct tracking_audit_representatives {
    tracking_audit_case_result_t cases[8];
    const char* reasons[8];
    size_t count;
    bool strategy_seen[3];
    bool disabled_seen;
    bool tight_seen;
    bool budget_seen;
} tracking_audit_representatives_t;

typedef struct tracking_audit_threshold {
    size_t samples;
    unsigned seed;
    size_t baseline_optimized[3];
    size_t required_optimized[3];
    double baseline_average_improvement[3];
    double required_average_improvement[3];
} tracking_audit_threshold_t;

static const tracking_audit_threshold_t tracking_audit_thresholds[] = {
    {
        10000,
        1u,
        {268, 254, 262},
        {335, 318, 328},
        {0.00696885785, 0.00573055088, 0.00735498978},
        {0.007665743635, 0.006303605968, 0.008090488758}
    },
    {
        100000,
        1u,
        {2628, 2601, 2573},
        {3285, 3252, 3217},
        {0.00654911563, 0.00679519282, 0.00721271345},
        {0.007204027193, 0.007474712102, 0.007933984795}
    },
    {
        100000,
        2u,
        {2648, 2702, 2526},
        {3310, 3378, 3158},
        {0.00587577617, 0.00614356450, 0.00642797412},
        {0.006463353787, 0.006757920950, 0.007070771532}
    },
    {
        100000,
        41u,
        {2638, 2711, 2499},
        {3298, 3389, 3124},
        {0.00792016481, 0.00763398601, 0.00693869317},
        {0.008712181291, 0.008397384611, 0.007632562487}
    },
    {
        1000000,
        1u,
        {26631, 26171, 25308},
        {33289, 32714, 31635},
        {0.00679055094, 0.00672526897, 0.00712030876},
        {0.007469606034, 0.007397795867, 0.007832339636}
    }
};

static const tracking_audit_threshold_t* tracking_audit_find_threshold(size_t samples, unsigned seed) {
    size_t i;
    for (i = 0; i < sizeof(tracking_audit_thresholds) / sizeof(tracking_audit_thresholds[0]); ++i) {
        if (tracking_audit_thresholds[i].samples == samples && tracking_audit_thresholds[i].seed == seed) {
            return &tracking_audit_thresholds[i];
        }
    }
    return NULL;
}

static void apply_tracking_audit_constraints(ruckig_input_t* input, size_t dofs, bool tight_constraints) {
    size_t dof;
    if (!tight_constraints) {
        return;
    }
    for (dof = 0; dof < dofs; ++dof) {
        ruckig_input_max_velocity_data(input)[dof] *= 0.55;
        ruckig_input_max_acceleration_data(input)[dof] *= 0.65;
        ruckig_input_max_jerk_data(input)[dof] *= 0.70;
    }
}

static void fill_tracking_audit_lookahead(
    const tracking_audit_case_config_t* config,
    ruckig_target_state_sequence_t* lookahead
) {
    size_t ahead;
    double* position = ruckig_target_state_sequence_position_data(lookahead);
    double* velocity = ruckig_target_state_sequence_velocity_data(lookahead);
    double* acceleration = ruckig_target_state_sequence_acceleration_data(lookahead);
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, config->lookahead_count), RUCKIG_WORKING);
    for (ahead = 0; ahead < config->lookahead_count; ++ahead) {
        size_t dof;
        const double time = config->start_time + (double)ahead * 0.01;
        for (dof = 0; dof < config->dofs; ++dof) {
            tracking_signal_value(
                config->signal,
                dof,
                time,
                &position[ahead * config->dofs + dof],
                &velocity[ahead * config->dofs + dof],
                &acceleration[ahead * config->dofs + dof]
            );
        }
    }
}

void make_tracking_audit_case_config(unsigned* state, size_t sample_index, tracking_audit_case_config_t* config) {
    memset(config, 0, sizeof(*config));
    config->sample_index = sample_index;
    config->dofs = tracking_random_dof_values[tracking_random_audit_pick(state, 4)];
    config->lookahead_count = tracking_random_lookahead_values[tracking_random_audit_pick(state, 4)];
    config->signal = (int)tracking_random_audit_pick(state, 4);
    config->reactiveness = tracking_random_reactiveness_values[tracking_random_audit_pick(state, 4)];
    config->strategy = tracking_random_strategy_values[tracking_random_audit_pick(state, 3)];
    config->start_time = (double)(sample_index % 200u) * 0.01;
    if (config->dofs > 1 && tracking_random_audit_bool(state)) {
        config->has_disabled_dof = true;
        config->disabled_dof = tracking_random_audit_pick(state, config->dofs);
    }
    config->tight_constraints = tracking_random_audit_bool(state);
}

void run_tracking_audit_case(
    const tracking_audit_case_config_t* config,
    tracking_audit_case_result_t* case_result
) {
    ruckig_tracking_t* tracking = NULL;
    ruckig_target_state_sequence_t* lookahead = NULL;
    ruckig_input_t* input = NULL;
    ruckig_output_t* output = NULL;

    memset(case_result, 0, sizeof(*case_result));
    case_result->config = *config;

    CHECK_EQ_INT(ruckig_tracking_create(&tracking, config->dofs, 0.01), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_target_state_sequence_create(&lookahead, config->dofs, config->lookahead_count), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_input_create(&input, config->dofs), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_output_create(&output, config->dofs), RUCKIG_WORKING);
    fill_tracking_input_nd(input, config->dofs);
    apply_tracking_audit_constraints(input, config->dofs, config->tight_constraints);
    if (config->has_disabled_dof) {
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, config->disabled_dof, false), RUCKIG_WORKING);
    }
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, config->strategy), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, config->reactiveness), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, config->lookahead_count), RUCKIG_WORKING);
    fill_tracking_audit_lookahead(config, lookahead);

    case_result->result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
    CHECK_TRUE(case_result->result == RUCKIG_WORKING || case_result->result == RUCKIG_FINISHED);
    case_result->status = ruckig_tracking_get_last_calculation_status(tracking);
    CHECK_TRUE(tracking_optimized_status_is_success(case_result->status));
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) >= 1);
    CHECK_TRUE(ruckig_tracking_get_last_candidate_count(tracking) <= ruckig_tracking_get_max_optimized_candidates(tracking));
    check_tracking_output_constraints(output, input, config->dofs);
    CHECK_EQ_INT(ruckig_tracking_get_last_diagnostics(tracking, &case_result->diagnostics), RUCKIG_WORKING);
    check_tracking_diagnostics_common(tracking, &case_result->diagnostics);
    CHECK_EQ_INT(case_result->diagnostics.mode, RUCKIG_TRACKING_OPTIMIZED);
    CHECK_EQ_INT(case_result->diagnostics.optimized_strategy, config->strategy);
    CHECK_TRUE(case_result->diagnostics.fallback_step_count + case_result->diagnostics.optimized_step_count == 1);
    CHECK_EQ_INT(case_result->diagnostics.error_step_count, 0);
    memcpy(case_result->family_attempted, tracking->audit_family_attempted, sizeof(case_result->family_attempted));
    memcpy(case_result->family_valid, tracking->audit_family_valid, sizeof(case_result->family_valid));
    memcpy(case_result->family_strict_improved, tracking->audit_family_strict_improved, sizeof(case_result->family_strict_improved));
    memcpy(case_result->family_near_tie_accepted, tracking->audit_family_near_tie_accepted, sizeof(case_result->family_near_tie_accepted));
    memcpy(case_result->family_selected, tracking->audit_family_selected, sizeof(case_result->family_selected));
    case_result->selected_family = tracking->audit_best_candidate_family;
    case_result->selected_near_tie = tracking->audit_best_candidate_near_tie;
    case_result->strict_improved_count = tracking->audit_strict_improved_count;
    case_result->near_tie_accepted_count = tracking->audit_near_tie_accepted_count;
    {
        size_t family;
        size_t attempted = 0;
        size_t valid = 0;
        size_t selected = 0;
        size_t strict_improved = 0;
        size_t near_tie = 0;
        for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
            attempted += case_result->family_attempted[family];
            valid += case_result->family_valid[family];
            selected += case_result->family_selected[family];
            strict_improved += case_result->family_strict_improved[family];
            near_tie += case_result->family_near_tie_accepted[family];
        }
        CHECK_EQ_INT(attempted, case_result->diagnostics.candidate_count);
        CHECK_EQ_INT(valid, case_result->diagnostics.valid_candidate_count);
        CHECK_EQ_INT(selected, 1);
        CHECK_EQ_INT(strict_improved, case_result->strict_improved_count);
        CHECK_EQ_INT(near_tie, case_result->near_tie_accepted_count);
    }

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_tracking_destroy(tracking);
}

static void tracking_audit_add_bucket(
    tracking_audit_bucket_t* bucket,
    const tracking_audit_case_result_t* case_result
) {
    size_t family;
    ++bucket->samples;
    if (case_result->status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
        ++bucket->optimized;
    } else if (case_result->status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
        ++bucket->fallback;
    }
    bucket->candidates += case_result->diagnostics.candidate_count;
    bucket->valid += case_result->diagnostics.valid_candidate_count;
    bucket->rejected += case_result->diagnostics.rejected_candidate_count;
    bucket->budget_exhausted += case_result->diagnostics.budget_exhausted_count;
    bucket->strict_improved_count += case_result->strict_improved_count;
    bucket->near_tie_accepted_count += case_result->near_tie_accepted_count;
    for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
        bucket->family_attempted[family] += case_result->family_attempted[family];
        bucket->family_valid[family] += case_result->family_valid[family];
        bucket->family_strict_improved[family] += case_result->family_strict_improved[family];
        bucket->family_near_tie_accepted[family] += case_result->family_near_tie_accepted[family];
        bucket->family_selected[family] += case_result->family_selected[family];
    }
    bucket->improvement_sum += case_result->diagnostics.improvement_ratio;
}

static void tracking_audit_add_stats(
    tracking_audit_stats_t* stats,
    const tracking_audit_case_result_t* case_result
) {
    const tracking_audit_case_config_t* config = &case_result->config;
    tracking_audit_add_bucket(&stats->overall, case_result);
    tracking_audit_add_bucket(&stats->by_strategy[tracking_strategy_index(config->strategy)], case_result);
    tracking_audit_add_bucket(&stats->by_dof[tracking_dof_index(config->dofs)], case_result);
    tracking_audit_add_bucket(&stats->by_signal[(size_t)config->signal], case_result);
    tracking_audit_add_bucket(&stats->by_lookahead[tracking_lookahead_index(config->lookahead_count)], case_result);
    tracking_audit_add_bucket(&stats->by_reactiveness[tracking_reactiveness_index(config->reactiveness)], case_result);
    tracking_audit_add_bucket(&stats->by_disabled[config->has_disabled_dof ? 1 : 0], case_result);
    tracking_audit_add_bucket(&stats->by_constraints[config->tight_constraints ? 1 : 0], case_result);
}

static void tracking_audit_print_bucket(const char* group, const char* name, const tracking_audit_bucket_t* bucket) {
    const double average_improvement = bucket->samples > 0 ? bucket->improvement_sum / (double)bucket->samples : 0.0;
    printf(
        "tracking random audit %s %s: samples %zu optimized %zu fallback %zu candidates %zu valid %zu rejected %zu budget_exhausted %zu strict_improved %zu near_tie_accepted %zu average_improvement %.9g\n",
        group,
        name,
        bucket->samples,
        bucket->optimized,
        bucket->fallback,
        bucket->candidates,
        bucket->valid,
        bucket->rejected,
        bucket->budget_exhausted,
        bucket->strict_improved_count,
        bucket->near_tie_accepted_count,
        average_improvement
    );
}

static bool tracking_audit_case_recorded(
    const tracking_audit_representatives_t* representatives,
    const tracking_audit_case_result_t* case_result
) {
    size_t i;
    for (i = 0; i < representatives->count; ++i) {
        if (representatives->cases[i].config.sample_index == case_result->config.sample_index) {
            return true;
        }
    }
    return false;
}

static void tracking_audit_record_representative(
    tracking_audit_representatives_t* representatives,
    const tracking_audit_case_result_t* case_result,
    const char* reason
) {
    if (representatives->count >= sizeof(representatives->cases) / sizeof(representatives->cases[0])
        || tracking_audit_case_recorded(representatives, case_result)) {
        return;
    }
    representatives->cases[representatives->count] = *case_result;
    representatives->reasons[representatives->count] = reason;
    ++representatives->count;
}

static void tracking_audit_maybe_record_fallback(
    tracking_audit_representatives_t* representatives,
    const tracking_audit_case_result_t* case_result
) {
    const size_t strategy_index = tracking_strategy_index(case_result->config.strategy);
    if (case_result->status != RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
        return;
    }
    if (!representatives->strategy_seen[strategy_index]) {
        representatives->strategy_seen[strategy_index] = true;
        tracking_audit_record_representative(
            representatives,
            case_result,
            strategy_index == 0 ? "stable_fallback" : (strategy_index == 1 ? "balanced_fallback" : "aggressive_fallback")
        );
    }
    if (case_result->config.has_disabled_dof && !representatives->disabled_seen) {
        representatives->disabled_seen = true;
        tracking_audit_record_representative(representatives, case_result, "disabled_fallback");
    }
    if (case_result->config.tight_constraints && !representatives->tight_seen) {
        representatives->tight_seen = true;
        tracking_audit_record_representative(representatives, case_result, "tight_valid_fallback");
    }
    if (case_result->diagnostics.budget_exhausted_count > 0 && !representatives->budget_seen) {
        representatives->budget_seen = true;
        tracking_audit_record_representative(representatives, case_result, "budget_exhausted_fallback");
    }
}

static void tracking_audit_print_case(
    const char* reason,
    const tracking_audit_case_result_t* case_result,
    unsigned seed
) {
    const tracking_audit_case_config_t* config = &case_result->config;
    const ruckig_tracking_diagnostics_t* diagnostics = &case_result->diagnostics;
    const char* selected_family = case_result->selected_family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT
        ? tracking_audit_family_names[case_result->selected_family]
        : "unknown";
    printf(
        "tracking random audit fallback_case reason=%s seed=%u sample=%zu strategy=%s dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f disabled=%s disabled_dof=%s constraints=%s status=%s selected_family=%s selected_near_tie=%s strict_improved=%zu near_tie_accepted=%zu candidates=%zu fast=%zu instantaneous=%zu horizon=%zu terminal_blend=%zu derivative_damped=%zu lead_lag=%zu budget_exhausted=%zu fast_score=%.9g best_score=%.9g improvement=%.9g\n",
        reason,
        seed,
        config->sample_index,
        tracking_strategy_name(config->strategy),
        config->dofs,
        tracking_signal_name(config->signal),
        config->lookahead_count,
        config->reactiveness,
        config->has_disabled_dof ? "yes" : "no",
        config->has_disabled_dof ? tracking_dof_number_name(config->disabled_dof) : "none",
        config->tight_constraints ? "tight_valid" : "default",
        tracking_status_name(case_result->status),
        selected_family,
        case_result->selected_near_tie ? "yes" : "no",
        case_result->strict_improved_count,
        case_result->near_tie_accepted_count,
        diagnostics->candidate_count,
        diagnostics->fast_candidate_count,
        diagnostics->instantaneous_candidate_count,
        diagnostics->horizon_candidate_count,
        diagnostics->terminal_blend_candidate_count,
        diagnostics->derivative_damped_candidate_count,
        diagnostics->lead_lag_candidate_count,
        diagnostics->budget_exhausted_count,
        diagnostics->fast_score,
        diagnostics->best_score,
        diagnostics->improvement_ratio
    );
}

void tracking_audit_print_fixture(
    const tracking_audit_case_config_t* config,
    const tracking_audit_case_result_t* case_result,
    unsigned seed
) {
    printf("tracking random audit replay fixture seed=%u sample=%zu\n", seed, config->sample_index);
    printf(
        "const tracking_audit_case_config_t case_config = {%zu, %zu, %zu, %d, %.17g, %s, %s, %zu, %s, %.17g};\n",
        config->sample_index,
        config->dofs,
        config->lookahead_count,
        config->signal,
        config->reactiveness,
        tracking_strategy_initializer(config->strategy),
        config->has_disabled_dof ? "true" : "false",
        config->disabled_dof,
        config->tight_constraints ? "true" : "false",
        config->start_time
    );
    tracking_audit_print_case("replay", case_result, seed);
}

static double tracking_audit_average_improvement(const tracking_audit_bucket_t* bucket) {
    return bucket->samples > 0 ? bucket->improvement_sum / (double)bucket->samples : 0.0;
}

static void tracking_audit_print_family_summary(const tracking_audit_bucket_t* bucket) {
    size_t family;
    for (family = 0; family < RUCKIG_TRACKING_AUDIT_FAMILY_COUNT; ++family) {
        printf(
            "tracking random audit by_family %s: attempted %zu valid %zu strict_improved %zu near_tie_accepted %zu selected %zu\n",
            tracking_audit_family_names[family],
            bucket->family_attempted[family],
            bucket->family_valid[family],
            bucket->family_strict_improved[family],
            bucket->family_near_tie_accepted[family],
            bucket->family_selected[family]
        );
    }
}

static void tracking_audit_check_thresholds(
    const tracking_audit_stats_t* stats,
    const tracking_audit_threshold_t* threshold
) {
    static const char* strategy_names[3] = {"stable", "balanced", "aggressive"};
    size_t i;
    if (!threshold) {
        printf("tracking random audit threshold: samples unregistered result SKIP\n");
        return;
    }
    for (i = 0; i < 3; ++i) {
        const tracking_audit_bucket_t* bucket = &stats->by_strategy[i];
        const double average_improvement = tracking_audit_average_improvement(bucket);
        const bool optimized_pass = bucket->optimized >= threshold->required_optimized[i];
        const bool average_pass = average_improvement + 1e-12 >= threshold->required_average_improvement[i];
        printf(
            "tracking random audit threshold strategy %s: baseline_optimized %zu optimized %zu required_optimized %zu baseline_average_improvement %.12g average_improvement %.12g required_average_improvement %.12g result %s\n",
            strategy_names[i],
            threshold->baseline_optimized[i],
            bucket->optimized,
            threshold->required_optimized[i],
            threshold->baseline_average_improvement[i],
            average_improvement,
            threshold->required_average_improvement[i],
            optimized_pass && average_pass ? "PASS" : "FAIL"
        );
        CHECK_TRUE(optimized_pass);
        CHECK_TRUE(average_pass);
    }
}

static void tracking_audit_print_stats(
    const tracking_audit_stats_t* stats,
    const tracking_audit_representatives_t* representatives,
    size_t samples,
    unsigned seed,
    const tracking_audit_threshold_t* threshold
) {
    static const char* strategy_names[3] = {"stable", "balanced", "aggressive"};
    static const char* dof_names[4] = {"1", "2", "4", "8"};
    static const char* signal_names[4] = {"ramp", "constant_acceleration", "sinus", "half_sinus"};
    static const char* lookahead_names[4] = {"1", "2", "5", "10"};
    static const char* reactiveness_names[4] = {"0", "0.25", "0.5", "1"};
    static const char* disabled_names[2] = {"enabled_only", "has_disabled_dof"};
    static const char* constraint_names[2] = {"default", "tight_valid"};
    size_t i;

    printf("tracking random audit: samples %zu seed %u\n", samples, seed);
    tracking_audit_print_bucket("overall", "all", &stats->overall);
    for (i = 0; i < 3; ++i) {
        tracking_audit_print_bucket("by_strategy", strategy_names[i], &stats->by_strategy[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_dof", dof_names[i], &stats->by_dof[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_signal", signal_names[i], &stats->by_signal[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_lookahead", lookahead_names[i], &stats->by_lookahead[i]);
    }
    for (i = 0; i < 4; ++i) {
        tracking_audit_print_bucket("by_reactiveness", reactiveness_names[i], &stats->by_reactiveness[i]);
    }
    for (i = 0; i < 2; ++i) {
        tracking_audit_print_bucket("by_disabled", disabled_names[i], &stats->by_disabled[i]);
    }
    for (i = 0; i < 2; ++i) {
        tracking_audit_print_bucket("by_constraints", constraint_names[i], &stats->by_constraints[i]);
    }
    tracking_audit_print_family_summary(&stats->overall);
    tracking_audit_check_thresholds(stats, threshold);
    for (i = 0; i < representatives->count; ++i) {
        tracking_audit_print_case(representatives->reasons[i], &representatives->cases[i], seed);
    }
}

void run_tracking_random_audit_tests(size_t samples, unsigned seed) {
    tracking_audit_stats_t stats;
    tracking_audit_representatives_t representatives;
    size_t sample;
    unsigned state = seed ? seed : 1u;

    memset(&stats, 0, sizeof(stats));
    memset(&representatives, 0, sizeof(representatives));

    for (sample = 0; sample < samples; ++sample) {
        tracking_audit_case_config_t config;
        tracking_audit_case_result_t case_result;
        make_tracking_audit_case_config(&state, sample, &config);
        run_tracking_audit_case(&config, &case_result);
        tracking_audit_add_stats(&stats, &case_result);
        tracking_audit_maybe_record_fallback(&representatives, &case_result);
    }

    tracking_audit_print_stats(
        &stats,
        &representatives,
        samples,
        seed,
        tracking_audit_find_threshold(samples, seed)
    );
}

void run_tracking_random_audit_replay_tests(size_t sample_index, unsigned seed) {
    tracking_audit_case_config_t config;
    tracking_audit_case_result_t case_result;
    size_t sample = 0;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        make_tracking_audit_case_config(&state, sample, &config);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    run_tracking_audit_case(&config, &case_result);
    tracking_audit_print_fixture(&config, &case_result, seed);
}
