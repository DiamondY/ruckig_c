#ifndef RUCKIG_C_TEST_API_INTERNAL_H
#define RUCKIG_C_TEST_API_INTERNAL_H

#include "test_common.h"

#include "ruckig_c/alloc.h"
#include "ruckig_c/internal.h"

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ruckig_c/ruckig.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

void run_api_tests(void);
void run_public_api_tests(void);
void run_public_api_post_tracking_tests(void);
void run_allocation_api_tests(void);
void run_public_diagnostics_tests(void);
void run_brake_tests(void);
void run_profile_tests(void);
void run_roots_tests(void);
void run_roots_numeric_audit_tests(void);
void run_utils_tests(void);
void run_waypoint_tests(void);
void run_waypoint_per_section_tests(void);
void run_waypoint_quality_tests(void);
void run_waypoint_all_regression_tests(void);
void run_waypoint_per_section_regression_tests(void);
void run_waypoint_quality_regression_tests(void);
void run_waypoint_resume_stress_tests(void);
void run_waypoint_resume_quality_tests(void);
void run_waypoint_resume_quality_baseline_dump(void);
void run_waypoint_resume_quality_audit_tests(void);
void run_interrupt_boundary_audit_tests(void);
void run_no_waypoint_interrupt_audit_tests(void);
void run_interrupt_post_release_quality_tests(void);
void run_constructor_boundary_tests(void);
void run_property_invariant_tests(void);
void run_state_machine_branch_coverage_tests(void);
void run_solver_branch_coverage_tests(void);
void run_tracking_api_tests(void);
void run_tracking_api_lifecycle_tests(void);
void run_tracking_public_diagnostics_tests(void);
void run_tracking_sequence_continuation_api_tests(void);
void run_tracking_sequence_fast_continuation_tests(void);
void run_tracking_sequence_optimized_continuation_tests(void);
void run_tracking_validation_tests(void);
void run_tracking_online_tests(void);
void run_tracking_interrupt_audit_tests(void);
void run_tracking_fixed_corpus_tests(void);
void run_tracking_offline_tests(void);
void run_tracking_optimized_tests(void);
void run_tracking_optimized_quality_regression_tests(void);
void run_tracking_quality_tests(void);
void run_tracking_quality_hardening_tests(void);
void run_tracking_stability_tests(void);
void run_tracking_no_allocation_tests(void);
void run_tracking_tests(void);
void run_tracking_random_tests(size_t samples, unsigned seed);
void run_tracking_random_audit_tests(size_t samples, unsigned seed);
void run_tracking_random_audit_fixed_case_tests(void);
void run_tracking_random_replay_tests(size_t sample, unsigned seed);
void run_tracking_random_audit_replay_tests(size_t sample, unsigned seed);
void run_tracking_random_audit_shrink_tests(size_t sample, unsigned seed);
void run_tracking_random_audit_failure_shrink_tests(size_t sample, unsigned seed);

void configure_soft_interruption_waypoint_input(ruckig_input_t* input);
void configure_alpha2_resume_input(ruckig_input_t* input);
void configure_alpha1_resume_stress_input(ruckig_input_t* input);
void configure_interrupt_boundary_no_waypoint_input(ruckig_input_t* input);
void check_alpha1_resume_stress_trajectory(const ruckig_trajectory_t* trajectory);
ruckig_result_t ruckig_update_under_allocation_guard(ruckig_t* otg, ruckig_input_t* input, ruckig_output_t* output);
void check_waypoint_samples(
    const ruckig_trajectory_t* trajectory,
    const double* waypoints,
    size_t waypoint_count,
    size_t dofs
);
void check_waypoint_section_sampled_limits(
    const ruckig_trajectory_t* trajectory,
    const double* per_section_min_velocity,
    const double* per_section_max_velocity,
    const double* per_section_min_acceleration,
    const double* per_section_max_acceleration,
    const double* per_section_max_jerk,
    const double* per_section_min_position,
    const double* per_section_max_position,
    size_t section_count,
    size_t dofs
);

void fill_tracking_input_1d(ruckig_input_t* input);
void fill_tracking_input_nd(ruckig_input_t* input, size_t dofs);
void fill_tracking_target_ramp(ruckig_target_state_t* target, double time);
void tracking_signal_value(int signal, size_t dof, double time, double* position, double* velocity, double* acceleration);
void set_tracking_target_signal(ruckig_target_state_t* target, int signal, size_t dofs, double time);
void set_tracking_sequence_signal(ruckig_target_state_sequence_t* targets, int signal, size_t dofs, size_t count, double delta_time);
void check_tracking_output_constraints(const ruckig_output_t* output, const ruckig_input_t* input, size_t dofs);
void check_tracking_diagnostics_common(const ruckig_tracking_t* tracking, const ruckig_tracking_diagnostics_t* diagnostics);
void check_tracking_output_sequence(const ruckig_tracking_output_sequence_t* outputs, size_t dofs, size_t count, double delta_time);
bool tracking_optimized_status_is_success(ruckig_tracking_calculation_status_t status);
ruckig_result_t tracking_calculate_sequence_interruptible_under_allocation_guard(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* targets,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* outputs,
    ruckig_tracking_sequence_continuation_t* continuation
);
ruckig_result_t tracking_resume_sequence_under_allocation_guard(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* outputs
);
void run_tracking_sequence_optimized_continuation_equivalence_case(
    ruckig_tracking_optimized_strategy_t strategy,
    size_t dofs,
    size_t count,
    size_t lookahead_count,
    size_t max_candidates,
    bool disable_last_dof,
    double interrupt_duration
);
void test_tracking_sequence_fast_continuation_delta_time_contract(void);
void configure_tracking_sequence_optimized_continuation(
    ruckig_tracking_t* tracking,
    size_t lookahead_count,
    size_t max_candidates
);

#endif
