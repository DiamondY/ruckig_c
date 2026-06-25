#include "test_api_internal.h"

static unsigned tracking_random_next(unsigned* state) {
    *state = 1664525u * (*state) + 1013904223u;
    return *state;
}

static size_t tracking_random_pick(unsigned* state, size_t count) {
    return (size_t)(tracking_random_next(state) % (unsigned)count);
}

static size_t tracking_random_audit_pick(unsigned* state, size_t count) {
    return (size_t)((tracking_random_next(state) >> 8u) % (unsigned)count);
}

static bool tracking_random_audit_bool(unsigned* state) {
    return ((tracking_random_next(state) >> 8u) & 1u) != 0u;
}

typedef struct tracking_random_case_config {
    size_t sample_index;
    size_t dofs;
    size_t lookahead_count;
    int signal;
    double reactiveness;
    ruckig_tracking_optimized_strategy_t strategy;
    bool has_disabled_dof;
    size_t disabled_dof;
    double start_time;
} tracking_random_case_config_t;

typedef struct tracking_random_case_result {
    tracking_random_case_config_t config;
    ruckig_result_t result;
    ruckig_tracking_calculation_status_t status;
    size_t candidate_count;
} tracking_random_case_result_t;

typedef struct tracking_audit_case_config {
    size_t sample_index;
    size_t dofs;
    size_t lookahead_count;
    int signal;
    double reactiveness;
    ruckig_tracking_optimized_strategy_t strategy;
    bool has_disabled_dof;
    size_t disabled_dof;
    bool tight_constraints;
    double start_time;
} tracking_audit_case_config_t;

typedef struct tracking_audit_case_result {
    tracking_audit_case_config_t config;
    ruckig_result_t result;
    ruckig_tracking_calculation_status_t status;
    ruckig_tracking_diagnostics_t diagnostics;
    size_t family_attempted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_valid[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_strict_improved[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_near_tie_accepted[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t family_selected[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
    size_t selected_family;
    bool selected_near_tie;
    size_t strict_improved_count;
    size_t near_tie_accepted_count;
} tracking_audit_case_result_t;

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

typedef struct tracking_stability_case {
    const char* name;
    tracking_audit_case_config_t config;
    ruckig_tracking_calculation_status_t expected_status;
    size_t expected_family;
    bool expected_near_tie;
    bool expect_budget_exhausted;
    bool expect_positive_improvement;
} tracking_stability_case_t;

static const char* tracking_audit_family_names[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT] = {
    "fast",
    "instantaneous",
    "horizon",
    "terminal_blend",
    "derivative_damped",
    "lead_lag"
};

static const size_t tracking_random_dof_values[4] = {1, 2, 4, 8};
static const size_t tracking_random_lookahead_values[4] = {1, 2, 5, 10};
static const double tracking_random_reactiveness_values[4] = {0.0, 0.25, 0.5, 1.0};
static const ruckig_tracking_optimized_strategy_t tracking_random_strategy_values[3] = {
    RUCKIG_TRACKING_OPTIMIZED_STABLE,
    RUCKIG_TRACKING_OPTIMIZED_BALANCED,
    RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE
};

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

static const char* tracking_strategy_name(ruckig_tracking_optimized_strategy_t strategy) {
    switch (strategy) {
    case RUCKIG_TRACKING_OPTIMIZED_STABLE:
        return "stable";
    case RUCKIG_TRACKING_OPTIMIZED_BALANCED:
        return "balanced";
    case RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE:
        return "aggressive";
    }
    return "unknown";
}

static const char* tracking_strategy_initializer(ruckig_tracking_optimized_strategy_t strategy) {
    switch (strategy) {
    case RUCKIG_TRACKING_OPTIMIZED_STABLE:
        return "RUCKIG_TRACKING_OPTIMIZED_STABLE";
    case RUCKIG_TRACKING_OPTIMIZED_BALANCED:
        return "RUCKIG_TRACKING_OPTIMIZED_BALANCED";
    case RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE:
        return "RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE";
    }
    return "RUCKIG_TRACKING_OPTIMIZED_STABLE";
}

static size_t tracking_strategy_index(ruckig_tracking_optimized_strategy_t strategy) {
    switch (strategy) {
    case RUCKIG_TRACKING_OPTIMIZED_STABLE:
        return 0;
    case RUCKIG_TRACKING_OPTIMIZED_BALANCED:
        return 1;
    case RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE:
        return 2;
    }
    return 0;
}

static const char* tracking_signal_name(int signal) {
    switch (signal) {
    case 0:
        return "ramp";
    case 1:
        return "constant_acceleration";
    case 2:
        return "sinus";
    case 3:
        return "half_sinus";
    }
    return "unknown";
}

static const char* tracking_status_name(ruckig_tracking_calculation_status_t status) {
    switch (status) {
    case RUCKIG_TRACKING_CALCULATION_NONE:
        return "none";
    case RUCKIG_TRACKING_CALCULATION_FAST:
        return "fast";
    case RUCKIG_TRACKING_CALCULATION_OPTIMIZED:
        return "optimized";
    case RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK:
        return "fast_fallback";
    case RUCKIG_TRACKING_CALCULATION_ERROR:
        return "error";
    }
    return "unknown";
}

static const char* tracking_dof_number_name(size_t dof) {
    static const char* names[8] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    return dof < 8 ? names[dof] : "unknown";
}

static size_t tracking_dof_index(size_t dofs) {
    if (dofs == 1) {
        return 0;
    }
    if (dofs == 2) {
        return 1;
    }
    if (dofs == 4) {
        return 2;
    }
    return 3;
}

static size_t tracking_lookahead_index(size_t lookahead_count) {
    if (lookahead_count == 1) {
        return 0;
    }
    if (lookahead_count == 2) {
        return 1;
    }
    if (lookahead_count == 5) {
        return 2;
    }
    return 3;
}

static size_t tracking_reactiveness_index(double reactiveness) {
    if (reactiveness < 0.125) {
        return 0;
    }
    if (reactiveness < 0.375) {
        return 1;
    }
    if (reactiveness < 0.75) {
        return 2;
    }
    return 3;
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

static tracking_random_case_config_t make_tracking_random_case_config(unsigned* state, size_t sample_index) {
    tracking_random_case_config_t config;
    memset(&config, 0, sizeof(config));
    config.sample_index = sample_index;
    config.dofs = tracking_random_dof_values[tracking_random_pick(state, 4)];
    config.lookahead_count = tracking_random_lookahead_values[tracking_random_pick(state, 4)];
    config.signal = (int)tracking_random_pick(state, 4);
    config.reactiveness = tracking_random_reactiveness_values[tracking_random_pick(state, 4)];
    config.strategy = tracking_random_strategy_values[tracking_random_pick(state, 3)];
    config.start_time = (double)(sample_index % 200u) * 0.01;
    if (config.dofs > 1 && (tracking_random_next(state) & 1u) != 0u) {
        config.has_disabled_dof = true;
        config.disabled_dof = tracking_random_pick(state, config.dofs);
    }
    return config;
}

static void make_tracking_audit_case_config(unsigned* state, size_t sample_index, tracking_audit_case_config_t* config) {
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

static void fill_tracking_random_lookahead(
    const tracking_random_case_config_t* config,
    ruckig_target_state_sequence_t* lookahead
) {
    size_t ahead;
    CHECK_EQ_INT(ruckig_target_state_sequence_set_count(lookahead, config->lookahead_count), RUCKIG_WORKING);
    for (ahead = 0; ahead < config->lookahead_count; ++ahead) {
        size_t dof;
        const double time = config->start_time + (double)ahead * 0.01;
        for (dof = 0; dof < config->dofs; ++dof) {
            double position;
            double velocity;
            double acceleration;
            tracking_signal_value(config->signal, dof, time, &position, &velocity, &acceleration);
            ruckig_target_state_sequence_position_data(lookahead)[ahead * config->dofs + dof] = position;
            ruckig_target_state_sequence_velocity_data(lookahead)[ahead * config->dofs + dof] = velocity;
            ruckig_target_state_sequence_acceleration_data(lookahead)[ahead * config->dofs + dof] = acceleration;
        }
    }
}

static void run_tracking_random_case(
    const tracking_random_case_config_t* config,
    tracking_random_case_result_t* case_result
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
    if (config->has_disabled_dof) {
        CHECK_EQ_INT(ruckig_input_set_dof_enabled(input, config->disabled_dof, false), RUCKIG_WORKING);
    }
    CHECK_EQ_INT(ruckig_tracking_set_mode(tracking, RUCKIG_TRACKING_OPTIMIZED), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_optimized_strategy(tracking, config->strategy), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_reactiveness(tracking, config->reactiveness), RUCKIG_WORKING);
    CHECK_EQ_INT(ruckig_tracking_set_look_ahead_cycles(tracking, config->lookahead_count), RUCKIG_WORKING);
    fill_tracking_random_lookahead(config, lookahead);

    case_result->result = ruckig_tracking_update_with_lookahead(tracking, lookahead, input, output);
    CHECK_TRUE(case_result->result == RUCKIG_WORKING || case_result->result == RUCKIG_FINISHED);
    case_result->status = ruckig_tracking_get_last_calculation_status(tracking);
    CHECK_TRUE(tracking_optimized_status_is_success(case_result->status));
    case_result->candidate_count = ruckig_tracking_get_last_candidate_count(tracking);
    CHECK_TRUE(case_result->candidate_count >= 1);
    CHECK_TRUE(case_result->candidate_count <= ruckig_tracking_get_max_optimized_candidates(tracking));
    check_tracking_output_constraints(output, input, config->dofs);

    ruckig_output_destroy(output);
    ruckig_input_destroy(input);
    ruckig_target_state_sequence_destroy(lookahead);
    ruckig_tracking_destroy(tracking);
}

static void run_tracking_audit_case(
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

static void tracking_random_print_fixture(
    const tracking_random_case_config_t* config,
    const tracking_random_case_result_t* case_result,
    unsigned seed
) {
    printf("tracking random replay fixture seed=%u sample=%zu\n", seed, config->sample_index);
    printf(
        "tracking random replay context seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s start_time=%.9g\n",
        seed,
        config->sample_index,
        config->dofs,
        tracking_signal_name(config->signal),
        config->lookahead_count,
        config->reactiveness,
        tracking_strategy_name(config->strategy),
        config->has_disabled_dof ? "yes" : "no",
        config->has_disabled_dof ? tracking_dof_number_name(config->disabled_dof) : "none",
        config->start_time
    );
    printf(
        "const tracking_random_case_config_t case_config = {%zu, %zu, %zu, %d, %.17g, %s, %s, %zu, %.17g};\n",
        config->sample_index,
        config->dofs,
        config->lookahead_count,
        config->signal,
        config->reactiveness,
        tracking_strategy_initializer(config->strategy),
        config->has_disabled_dof ? "true" : "false",
        config->disabled_dof,
        config->start_time
    );
    printf(
        "tracking random replay result seed=%u sample=%zu result=%d status=%s candidates=%zu\n",
        seed,
        config->sample_index,
        (int)case_result->result,
        tracking_status_name(case_result->status),
        case_result->candidate_count
    );
}

static void tracking_audit_print_fixture(
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

void run_tracking_random_tests(size_t samples, unsigned seed) {
    size_t sample;
    size_t optimized_count = 0;
    size_t fallback_count = 0;
    size_t candidate_count = 0;
    unsigned state = seed ? seed : 1u;

    for (sample = 0; sample < samples; ++sample) {
        const int sample_failures_before = ruckig_c_test_failures;
        tracking_random_case_config_t config = make_tracking_random_case_config(&state, sample);
        tracking_random_case_result_t case_result;
        run_tracking_random_case(&config, &case_result);
        candidate_count += case_result.candidate_count;
        if (case_result.status == RUCKIG_TRACKING_CALCULATION_OPTIMIZED) {
            ++optimized_count;
        } else if (case_result.status == RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK) {
            ++fallback_count;
        }
        if (ruckig_c_test_failures != sample_failures_before) {
            fprintf(
                stderr,
                "tracking random stress failure_context seed=%u sample=%zu dofs=%zu signal=%s lookahead=%zu reactiveness=%.2f strategy=%s disabled=%s disabled_dof=%s start_time=%.9g\n",
                seed,
                sample,
                config.dofs,
                tracking_signal_name(config.signal),
                config.lookahead_count,
                config.reactiveness,
                tracking_strategy_name(config.strategy),
                config.has_disabled_dof ? "yes" : "no",
                config.has_disabled_dof ? tracking_dof_number_name(config.disabled_dof) : "none",
                config.start_time
            );
        }
    }

    printf(
        "tracking random stress: samples %zu seed %u optimized %zu fallback %zu candidates %zu\n",
        samples,
        seed,
        optimized_count,
        fallback_count,
        candidate_count
    );
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

void run_tracking_random_replay_tests(size_t sample_index, unsigned seed) {
    tracking_random_case_config_t config;
    tracking_random_case_result_t case_result;
    size_t sample = 0;
    unsigned state = seed ? seed : 1u;

    for (;;) {
        config = make_tracking_random_case_config(&state, sample);
        if (sample == sample_index) {
            break;
        }
        ++sample;
    }

    run_tracking_random_case(&config, &case_result);
    tracking_random_print_fixture(&config, &case_result, seed);
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

typedef struct tracking_audit_failure_signature {
    bool failed;
    int failure_count;
    const char* failure_class;
    tracking_audit_case_result_t case_result;
} tracking_audit_failure_signature_t;

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
