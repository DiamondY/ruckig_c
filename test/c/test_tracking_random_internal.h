#ifndef RUCKIG_C_TEST_TRACKING_RANDOM_INTERNAL_H
#define RUCKIG_C_TEST_TRACKING_RANDOM_INTERNAL_H

#include "test_api_internal.h"

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

extern const char* const tracking_audit_family_names[RUCKIG_TRACKING_AUDIT_FAMILY_COUNT];
extern const size_t tracking_random_dof_values[4];
extern const size_t tracking_random_lookahead_values[4];
extern const double tracking_random_reactiveness_values[4];
extern const ruckig_tracking_optimized_strategy_t tracking_random_strategy_values[3];

unsigned tracking_random_next(unsigned* state);
size_t tracking_random_pick(unsigned* state, size_t count);
size_t tracking_random_audit_pick(unsigned* state, size_t count);
bool tracking_random_audit_bool(unsigned* state);

const char* tracking_strategy_name(ruckig_tracking_optimized_strategy_t strategy);
const char* tracking_strategy_initializer(ruckig_tracking_optimized_strategy_t strategy);
size_t tracking_strategy_index(ruckig_tracking_optimized_strategy_t strategy);
const char* tracking_signal_name(int signal);
const char* tracking_status_name(ruckig_tracking_calculation_status_t status);
const char* tracking_dof_number_name(size_t dof);
size_t tracking_dof_index(size_t dofs);
size_t tracking_lookahead_index(size_t lookahead_count);
size_t tracking_reactiveness_index(double reactiveness);

void make_tracking_audit_case_config(unsigned* state, size_t sample_index, tracking_audit_case_config_t* config);
void run_tracking_audit_case(const tracking_audit_case_config_t* config, tracking_audit_case_result_t* case_result);
void tracking_audit_print_fixture(const tracking_audit_case_config_t* config, const tracking_audit_case_result_t* result, unsigned seed);

#endif
