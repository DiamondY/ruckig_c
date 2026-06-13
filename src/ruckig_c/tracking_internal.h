#ifndef RUCKIG_C_TRACKING_INTERNAL_H
#define RUCKIG_C_TRACKING_INTERNAL_H

#include "ruckig_c/internal.h"
#include "ruckig_c/interrupt_context.h"

#include <stdbool.h>
#include <stddef.h>

#define RUCKIG_TRACKING_DEFAULT_OPTIMIZED_CANDIDATES 16u
#define RUCKIG_TRACKING_MAX_OPTIMIZED_CANDIDATES 128u
#define RUCKIG_TRACKING_SCORE_EPSILON 1e-12
#define RUCKIG_TRACKING_FAMILY_SCORE_RATIO 0.998
#define RUCKIG_TRACKING_AGGRESSIVE_NEAR_TIE_RATIO 1.01

typedef struct tracking_strategy_config {
    ruckig_tracking_optimized_strategy_t strategy;
    double position_weight;
    double velocity_weight;
    double acceleration_weight;
    double jerk_weight;
    double terminal_weight;
    double horizon_weight_step;
    double acceptance_ratio;
    double candidate_family_score_ratio;
    double near_tie_ratio;
    bool use_terminal_blends;
    bool use_derivative_damping;
    bool use_lead_lag_horizons;
} tracking_strategy_config_t;

typedef enum tracking_candidate_family {
    TRACKING_CANDIDATE_FAST,
    TRACKING_CANDIDATE_INSTANTANEOUS,
    TRACKING_CANDIDATE_HORIZON,
    TRACKING_CANDIDATE_TERMINAL_BLEND,
    TRACKING_CANDIDATE_DERIVATIVE_DAMPED,
    TRACKING_CANDIDATE_LEAD_LAG
} tracking_candidate_family_t;

typedef enum tracking_sequence_optimized_phase {
    TRACKING_SEQUENCE_OPTIMIZED_IDLE = 0,
    TRACKING_SEQUENCE_OPTIMIZED_FAST,
    TRACKING_SEQUENCE_OPTIMIZED_INSTANTANEOUS,
    TRACKING_SEQUENCE_OPTIMIZED_HORIZON,
    TRACKING_SEQUENCE_OPTIMIZED_LEAD_LAG,
    TRACKING_SEQUENCE_OPTIMIZED_TERMINAL_BLEND,
    TRACKING_SEQUENCE_OPTIMIZED_DERIVATIVE_DAMPED,
    TRACKING_SEQUENCE_OPTIMIZED_FINISH_STEP
} tracking_sequence_optimized_phase_t;

size_t min_size(size_t lhs, size_t rhs);
const tracking_strategy_config_t* tracking_strategy_config(ruckig_tracking_optimized_strategy_t strategy);
bool valid_tracking_strategy(ruckig_tracking_optimized_strategy_t strategy);
void tracking_reset_diagnostics(ruckig_tracking_t* tracking);
void tracking_sync_legacy_diagnostics(ruckig_tracking_t* tracking);
void tracking_set_diagnostic_status(
    ruckig_tracking_t* tracking,
    ruckig_tracking_calculation_status_t status
);
void tracking_mark_step_status(
    ruckig_tracking_t* tracking,
    ruckig_tracking_calculation_status_t status
);
void tracking_finalize_score_diagnostics(ruckig_tracking_t* tracking);
void tracking_note_candidate_family(
    ruckig_tracking_t* tracking,
    tracking_candidate_family_t family
);
void tracking_note_valid_candidate(ruckig_tracking_t* tracking);
void tracking_note_rejected_candidate(ruckig_tracking_t* tracking);
void tracking_note_budget_exhausted(ruckig_tracking_t* tracking);
void tracking_accumulate_diagnostics(
    ruckig_tracking_diagnostics_t* aggregate,
    const ruckig_tracking_diagnostics_t* step
);
bool finite_vector(const double* values, size_t count);
void tracking_mark_error(ruckig_tracking_t* tracking);
ruckig_result_t prepare_tracking_base(ruckig_tracking_t* tracking, const ruckig_input_t* input);
ruckig_result_t set_tracking_candidate_prediction(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    double horizon
);
void copy_best_to_work_input(ruckig_tracking_t* tracking);
ruckig_result_t prepare_fast_tracking_input(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    const ruckig_input_t* input
);
ruckig_result_t run_prepared_tracking_update(
    ruckig_tracking_t* tracking,
    ruckig_output_t* output,
    ruckig_tracking_calculation_status_t success_status,
    bool force_reset
);
ruckig_result_t tracking_optimized_candidate_step(
    ruckig_tracking_t* tracking,
    const tracking_strategy_config_t* config,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t window_count,
    size_t* phase,
    size_t* index,
    double* fast_score,
    double* fast_terminal_position_error,
    double* best_score,
    bool* improved,
    bool* evaluated_candidate
);
ruckig_result_t evaluate_optimized_tracking(
    ruckig_tracking_t* tracking,
    const double* target_position,
    const double* target_velocity,
    const double* target_acceleration,
    size_t target_count,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    bool allow_interrupt
);

#endif
