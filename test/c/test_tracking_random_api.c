#include "test_tracking_random_internal.h"

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

const char* const tracking_audit_family_names[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT] = {
    "fast",
    "instantaneous",
    "horizon",
    "terminal_blend",
    "derivative_damped",
    "lead_lag"
};

const size_t tracking_random_dof_values[4] = {1, 2, 4, 8};
const size_t tracking_random_lookahead_values[4] = {1, 2, 5, 10};
const double tracking_random_reactiveness_values[4] = {0.0, 0.25, 0.5, 1.0};
const ruckig_tracking_optimized_strategy_t tracking_random_strategy_values[3] = {
    RUCKIG_TRACKING_OPTIMIZED_STABLE,
    RUCKIG_TRACKING_OPTIMIZED_BALANCED,
    RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE
};

unsigned tracking_random_next(unsigned* state) {
    *state = 1664525u * (*state) + 1013904223u;
    return *state;
}

size_t tracking_random_pick(unsigned* state, size_t count) {
    return (size_t)(tracking_random_next(state) % (unsigned)count);
}

size_t tracking_random_audit_pick(unsigned* state, size_t count) {
    return (size_t)((tracking_random_next(state) >> 8u) % (unsigned)count);
}

bool tracking_random_audit_bool(unsigned* state) {
    return ((tracking_random_next(state) >> 8u) & 1u) != 0u;
}

const char* tracking_strategy_name(ruckig_tracking_optimized_strategy_t strategy) {
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

const char* tracking_strategy_initializer(ruckig_tracking_optimized_strategy_t strategy) {
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

size_t tracking_strategy_index(ruckig_tracking_optimized_strategy_t strategy) {
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

const char* tracking_signal_name(int signal) {
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

const char* tracking_status_name(ruckig_tracking_calculation_status_t status) {
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

const char* tracking_dof_number_name(size_t dof) {
    static const char* names[8] = {"0", "1", "2", "3", "4", "5", "6", "7"};
    return dof < 8 ? names[dof] : "unknown";
}

size_t tracking_dof_index(size_t dofs) {
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

size_t tracking_lookahead_index(size_t lookahead_count) {
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

size_t tracking_reactiveness_index(double reactiveness) {
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
