#ifndef RUCKIG_C_RUCKIG_H
#define RUCKIG_C_RUCKIG_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RUCKIG_C_VERSION_MAJOR 0
#define RUCKIG_C_VERSION_MINOR 16
#define RUCKIG_C_VERSION_PATCH 0
#define RUCKIG_C_VERSION_STRING "0.16.0"

/**
 * @file ruckig.h
 * Public C API for the local `ruckig_c` trajectory generation library.
 *
 * Handles are opaque and must be created and destroyed with their matching
 * `ruckig_*_create` and `ruckig_*_destroy` functions. Destroy functions accept
 * `NULL`. Pointers returned by `*_data` accessors are owned by their handle and
 * remain valid until the handle is destroyed or reconfigured by another API
 * call documented to change the associated storage.
 */

#ifndef RUCKIG_C_API
#  if defined(RUCKIG_C_STATIC_DEFINE)
#    define RUCKIG_C_API
#  elif defined(_WIN32) || defined(__CYGWIN__)
#    if defined(RUCKIG_C_BUILDING_LIBRARY)
#      define RUCKIG_C_API __declspec(dllexport)
#    else
#      define RUCKIG_C_API __declspec(dllimport)
#    endif
#  else
#    if defined(RUCKIG_C_BUILDING_LIBRARY) && defined(__GNUC__)
#      define RUCKIG_C_API __attribute__((visibility("default")))
#    else
#      define RUCKIG_C_API
#    endif
#  endif
#endif

/** Result codes returned by operations that can fail or finish. */
typedef enum ruckig_result {
    RUCKIG_WORKING = 0,
    RUCKIG_FINISHED = 1,
    RUCKIG_ERROR = -1,
    RUCKIG_ERROR_INVALID_INPUT = -100,
    RUCKIG_ERROR_TRAJECTORY_DURATION = -101,
    RUCKIG_ERROR_POSITIONAL_LIMITS = -102,
    RUCKIG_ERROR_ZERO_LIMITS = -104,
    RUCKIG_ERROR_EXECUTION_TIME_CALCULATION = -110,
    RUCKIG_ERROR_SYNCHRONIZATION_CALCULATION = -111,
    RUCKIG_ERROR_UNSUPPORTED = -200
} ruckig_result_t;

/** Return true for non-error result codes such as `RUCKIG_WORKING` and `RUCKIG_FINISHED`. */
#define RUCKIG_RESULT_IS_OK(result) ((result) >= 0)

/** Stable high-level area associated with a public diagnostics record. */
typedef enum ruckig_diagnostic_scope {
    RUCKIG_DIAGNOSTIC_SCOPE_NONE = 0,
    RUCKIG_DIAGNOSTIC_SCOPE_INPUT = 1,
    RUCKIG_DIAGNOSTIC_SCOPE_CALCULATION = 2,
    RUCKIG_DIAGNOSTIC_SCOPE_UPDATE = 3,
    RUCKIG_DIAGNOSTIC_SCOPE_WAYPOINT = 4,
    RUCKIG_DIAGNOSTIC_SCOPE_TRACKING = 5,
    RUCKIG_DIAGNOSTIC_SCOPE_TRACKING_SEQUENCE = 6
} ruckig_diagnostic_scope_t;

/** Stable coarse diagnostic reason associated with a failed or observed operation. */
typedef enum ruckig_diagnostic_code {
    RUCKIG_DIAGNOSTIC_NONE = 0,
    RUCKIG_DIAGNOSTIC_NULL_ARGUMENT = 1,
    RUCKIG_DIAGNOSTIC_DOF_MISMATCH = 2,
    RUCKIG_DIAGNOSTIC_CAPACITY_MISMATCH = 3,
    RUCKIG_DIAGNOSTIC_INVALID_ENUM = 4,
    RUCKIG_DIAGNOSTIC_NONFINITE_VALUE = 5,
    RUCKIG_DIAGNOSTIC_NEGATIVE_LIMIT = 6,
    RUCKIG_DIAGNOSTIC_ZERO_LIMIT = 7,
    RUCKIG_DIAGNOSTIC_CURRENT_STATE_OUT_OF_LIMITS = 8,
    RUCKIG_DIAGNOSTIC_TARGET_STATE_OUT_OF_LIMITS = 9,
    RUCKIG_DIAGNOSTIC_TRAJECTORY_DURATION = 10,
    RUCKIG_DIAGNOSTIC_SYNCHRONIZATION = 11,
    RUCKIG_DIAGNOSTIC_INTERRUPTED = 12,
    RUCKIG_DIAGNOSTIC_RESUME_IDENTITY_MISMATCH = 13,
    RUCKIG_DIAGNOSTIC_UNSUPPORTED = 14
} ruckig_diagnostic_code_t;

/**
 * Caller-owned diagnostics record for opt-in public diagnostics APIs.
 *
 * Initialize the object with `ruckig_diagnostics_init` before passing it to an
 * API. `struct_size` is used for source-level forward compatibility. If
 * `struct_size` is smaller than the stable prefix required by the library, the
 * API returns `RUCKIG_ERROR_INVALID_INPUT` and does not write a diagnostics
 * record because the caller-provided storage is not valid for the stable
 * fields. Reserved fields are read-only and currently zeroed.
 */
typedef struct ruckig_diagnostics {
    size_t struct_size;
    ruckig_result_t result;
    ruckig_diagnostic_scope_t scope;
    ruckig_diagnostic_code_t code;
    size_t dof;
    size_t section;
    size_t expected_count;
    size_t actual_count;
    double value;
    double limit;
    size_t reserved_size[8];
    double reserved_value[8];
} ruckig_diagnostics_t;

/** Global or per-DoF control interface. */
typedef enum ruckig_control_interface {
    RUCKIG_CONTROL_POSITION = 0,
    RUCKIG_CONTROL_VELOCITY = 1
} ruckig_control_interface_t;

/** Synchronization strategy for enabled degrees of freedom. */
typedef enum ruckig_synchronization {
    RUCKIG_SYNCHRONIZATION_TIME = 0,
    RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY = 1,
    RUCKIG_SYNCHRONIZATION_PHASE = 2,
    RUCKIG_SYNCHRONIZATION_NONE = 3
} ruckig_synchronization_t;

/** Duration discretization policy used during validation and calculation. */
typedef enum ruckig_duration_discretization {
    RUCKIG_DURATION_CONTINUOUS = 0,
    RUCKIG_DURATION_DISCRETE = 1
} ruckig_duration_discretization_t;

/** Tracking algorithm mode. */
typedef enum ruckig_tracking_mode {
    RUCKIG_TRACKING_FAST = 0,
    RUCKIG_TRACKING_OPTIMIZED = 1
} ruckig_tracking_mode_t;

/** Last tracking calculation family used by tracking diagnostics. */
typedef enum ruckig_tracking_calculation_status {
    RUCKIG_TRACKING_CALCULATION_NONE = 0,
    RUCKIG_TRACKING_CALCULATION_FAST = 1,
    RUCKIG_TRACKING_CALCULATION_OPTIMIZED = 2,
    RUCKIG_TRACKING_CALCULATION_FAST_FALLBACK = 3,
    RUCKIG_TRACKING_CALCULATION_ERROR = 4
} ruckig_tracking_calculation_status_t;

/** Optimized tracking candidate search strategy preset. */
typedef enum ruckig_tracking_optimized_strategy {
    RUCKIG_TRACKING_OPTIMIZED_STABLE = 0,
    RUCKIG_TRACKING_OPTIMIZED_BALANCED = 1,
    RUCKIG_TRACKING_OPTIMIZED_AGGRESSIVE = 2
} ruckig_tracking_optimized_strategy_t;

/** Minimum and maximum position extrema for one DoF of a trajectory. */
typedef struct ruckig_position_extrema {
    double min_position;
    double max_position;
    double time_min;
    double time_max;
} ruckig_position_extrema_t;

/**
 * Specialized tracking diagnostics for developer-facing tracking quality
 * inspection. These diagnostics expose tracking candidate counts and scores;
 * use `ruckig_tracking_get_last_public_diagnostics` for stable coarse public
 * diagnostics.
 */
typedef struct ruckig_tracking_diagnostics {
    ruckig_tracking_calculation_status_t calculation_status;
    ruckig_tracking_mode_t mode;
    ruckig_tracking_optimized_strategy_t optimized_strategy;

    size_t candidate_count;
    size_t valid_candidate_count;
    size_t rejected_candidate_count;
    size_t fallback_step_count;
    size_t optimized_step_count;
    size_t error_step_count;
    size_t budget_exhausted_count;

    size_t fast_candidate_count;
    size_t instantaneous_candidate_count;
    size_t horizon_candidate_count;
    size_t terminal_blend_candidate_count;
    size_t derivative_damped_candidate_count;
    size_t lead_lag_candidate_count;

    double fast_score;
    double best_score;
    double improvement_ratio;

    /* Reserved fields are read-only, currently zeroed, and may gain defined semantics in future releases. */
    size_t reserved_size[4];
    double reserved_value[4];
} ruckig_tracking_diagnostics_t;

#ifdef __cplusplus
/**
 * Opaque handle typedefs for C++ consumers. Use only the `ruckig_*_t` typedef
 * names in user code; struct tag names are intentionally not public API.
 */
typedef struct ruckig_c_input_handle ruckig_input_t;
typedef struct ruckig_c_output_handle ruckig_output_t;
typedef struct ruckig_c_trajectory_handle ruckig_trajectory_t;
typedef struct ruckig_c_handle ruckig_t;
typedef struct ruckig_c_tracking_handle ruckig_tracking_t;
typedef struct ruckig_c_target_state_handle ruckig_target_state_t;
typedef struct ruckig_c_target_state_sequence_handle ruckig_target_state_sequence_t;
typedef struct ruckig_c_tracking_output_sequence_handle ruckig_tracking_output_sequence_t;
typedef struct ruckig_c_tracking_sequence_continuation_handle ruckig_tracking_sequence_continuation_t;
#else
/** Opaque input parameter handle. */
typedef struct ruckig_input ruckig_input_t;
/** Opaque online output handle. */
typedef struct ruckig_output ruckig_output_t;
/** Opaque offline trajectory handle. */
typedef struct ruckig_trajectory ruckig_trajectory_t;
/** Opaque OTG solver handle. */
typedef struct ruckig ruckig_t;
/** Opaque tracking solver handle. */
typedef struct ruckig_tracking ruckig_tracking_t;
/** Opaque single tracking target-state handle. */
typedef struct ruckig_target_state ruckig_target_state_t;
/** Opaque fixed-capacity tracking target-state sequence handle. */
typedef struct ruckig_target_state_sequence ruckig_target_state_sequence_t;
/** Opaque fixed-capacity tracking output sequence handle. */
typedef struct ruckig_tracking_output_sequence ruckig_tracking_output_sequence_t;
/** Opaque interruptible tracking sequence continuation handle. */
typedef struct ruckig_tracking_sequence_continuation ruckig_tracking_sequence_continuation_t;
#endif

/*
 * Public C ABI scope:
 * - Intermediate waypoints and per-section constraints are exposed through the
 *   C ABI and solved locally by the waypoint optimizer.
 * - Tracking exposes local Fast mode and a bounded local Optimized mode with
 *   deterministic candidate search, strategy presets, and Fast fallback
 *   diagnostics.
 * - Algorithm visualization evidence is generated from this public C ABI, but
 *   it does not add visualization-specific public functions.
 * - Tracking quality and stability evidence is release-reviewed through local
 *   audit, hardening, and fixed regression gates without expanding the public
 *   C ABI.
 * - No cloud API, remote fallback, or Pro/cloud equivalence claim is provided.
 * - Python and Rust bindings remain separate layers over this C ABI.
 */

/**
 * Create an OTG solver for `dofs` degrees of freedom and online step
 * `delta_time`.
 *
 * `otg` receives a new handle on success and is set to `NULL` before
 * validation. `dofs` must be non-zero. Negative, NaN, and infinite
 * `delta_time` values are invalid; `0.0` remains accepted for offline
 * compatibility but is rejected later with discrete duration.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid arguments or unsupported
 * capacities, and `RUCKIG_ERROR` for failed internal storage allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time);
/** Create an OTG solver with intermediate waypoint capacity; see `ruckig_create` for return-code conventions. */
RUCKIG_C_API ruckig_result_t ruckig_create_with_waypoints(
    ruckig_t** otg,
    size_t dofs,
    double delta_time,
    size_t max_number_of_waypoints
);
/** Destroy an OTG solver; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_destroy(ruckig_t* otg);
/** Return the waypoint capacity of an OTG solver, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_get_max_number_of_waypoints(const ruckig_t* otg);

/**
 * Create an input handle for `dofs` degrees of freedom.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for a `NULL` output pointer, zero DoFs, or
 * unsupported capacity, and `RUCKIG_ERROR` for failed internal storage
 * allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs);
/** Create an input handle with intermediate waypoint capacity; see `ruckig_input_create` for return-code conventions. */
RUCKIG_C_API ruckig_result_t ruckig_input_create_with_waypoints(
    ruckig_input_t** input,
    size_t dofs,
    size_t max_number_of_waypoints
);
/** Destroy an input handle; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_destroy(ruckig_input_t* input);

/**
 * Create an online output handle for `dofs` degrees of freedom.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for a `NULL` output pointer, zero DoFs, or
 * unsupported capacity, and `RUCKIG_ERROR` for failed internal storage
 * allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_output_create(ruckig_output_t** output, size_t dofs);
/** Create an online output handle with trajectory waypoint capacity; see `ruckig_output_create` for return-code conventions. */
RUCKIG_C_API ruckig_result_t ruckig_output_create_with_waypoints(
    ruckig_output_t** output,
    size_t dofs,
    size_t max_number_of_waypoints
);
/** Destroy an output handle; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_output_destroy(ruckig_output_t* output);

/**
 * Create an offline trajectory handle for `dofs` degrees of freedom.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for a `NULL` output pointer, zero DoFs, or
 * unsupported capacity, and `RUCKIG_ERROR` for failed internal storage
 * allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_create(ruckig_trajectory_t** trajectory, size_t dofs);
/** Create an offline trajectory handle with waypoint section capacity; see `ruckig_trajectory_create` for return-code conventions. */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_create_with_waypoints(
    ruckig_trajectory_t** trajectory,
    size_t dofs,
    size_t max_number_of_waypoints
);
/** Destroy a trajectory handle; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_trajectory_destroy(ruckig_trajectory_t* trajectory);

/** Initialize a caller-owned diagnostics record; passing `NULL` is a no-op. */
RUCKIG_C_API void ruckig_diagnostics_init(ruckig_diagnostics_t* diagnostics);

/**
 * Validate an input against an OTG handle.
 *
 * `check_current_state_within_limits` checks the current state against limits.
 * `check_target_state_within_limits` checks the target state against limits.
 * Returns `RUCKIG_WORKING` when validation passes. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT`, `RUCKIG_ERROR_ZERO_LIMITS`, and
 * `RUCKIG_ERROR_POSITIONAL_LIMITS`.
 */
RUCKIG_C_API ruckig_result_t ruckig_validate_input(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits
);

/** Validate an input and fill stable coarse diagnostics when provided; return codes match `ruckig_validate_input`. */
RUCKIG_C_API ruckig_result_t ruckig_validate_input_with_diagnostics(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits,
    ruckig_diagnostics_t* diagnostics
);

/**
 * Calculate a complete offline trajectory into `trajectory`.
 *
 * Returns `RUCKIG_WORKING` when the trajectory is valid. Common failures
 * include invalid input or limits, trajectory-duration failures,
 * execution-time calculation failures, and synchronization calculation
 * failures.
 */
RUCKIG_C_API ruckig_result_t ruckig_calculate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
);

/** Calculate a complete offline trajectory and fill diagnostics when provided; return codes match `ruckig_calculate`. */
RUCKIG_C_API ruckig_result_t ruckig_calculate_with_diagnostics(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory,
    ruckig_diagnostics_t* diagnostics
);

/**
 * Advance one online step into `output`.
 *
 * Returns `RUCKIG_WORKING` while the online trajectory is still in progress
 * and `RUCKIG_FINISHED` when the sampled trajectory has completed. Common
 * failures match the calculation path and include invalid input, limit,
 * trajectory-duration, execution-time, and synchronization errors.
 */
RUCKIG_C_API ruckig_result_t ruckig_update(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output
);

/** Advance one online step and fill diagnostics when provided; return codes match `ruckig_update`. */
RUCKIG_C_API ruckig_result_t ruckig_update_with_diagnostics(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output,
    ruckig_diagnostics_t* diagnostics
);

/** Reset private OTG state such as cached online calculation/resume state. */
RUCKIG_C_API void ruckig_reset(ruckig_t* otg);

/**
 * Copy the latest output state into an input's current state.
 *
 * This is a no-op when either handle is `NULL` or when DoF counts differ.
 */
RUCKIG_C_API void ruckig_output_pass_to_input(
    const ruckig_output_t* output,
    ruckig_input_t* input
);

/** Return the DoF count for an output handle, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_output_get_dof_count(const ruckig_output_t* output);

/** Return output position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_output_new_position_data(const ruckig_output_t* output);
/** Return output velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_output_new_velocity_data(const ruckig_output_t* output);
/** Return output acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_output_new_acceleration_data(const ruckig_output_t* output);
/** Return output jerk array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_output_new_jerk_data(const ruckig_output_t* output);

/** Return the sampled output time in seconds, or `0.0` for `NULL`. */
RUCKIG_C_API double ruckig_output_get_time(const ruckig_output_t* output);
/** Return the active waypoint section index, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_output_get_new_section(const ruckig_output_t* output);
/** Return whether the output moved to a new section in the last update. */
RUCKIG_C_API bool ruckig_output_did_section_change(const ruckig_output_t* output);
/** Return whether the last update started a new calculation. */
RUCKIG_C_API bool ruckig_output_new_calculation(const ruckig_output_t* output);
/** Return whether the last waypoint calculation was interrupted. */
RUCKIG_C_API bool ruckig_output_was_calculation_interrupted(const ruckig_output_t* output);
/** Return optional monotonic elapsed calculation time in microseconds when enabled; returns `0.0` when disabled or when no monotonic timestamp is available. */
RUCKIG_C_API double ruckig_output_get_calculation_duration(const ruckig_output_t* output);

/** Return the trajectory owned by `output`; the pointer remains owned by `output`. */
RUCKIG_C_API const ruckig_trajectory_t* ruckig_output_get_trajectory(const ruckig_output_t* output);

/** Return the DoF count for an input handle, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_input_get_dof_count(const ruckig_input_t* input);

/** Return mutable current position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_current_position_data(ruckig_input_t* input);
/** Return mutable current velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_current_velocity_data(ruckig_input_t* input);
/** Return mutable current acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_current_acceleration_data(ruckig_input_t* input);
/** Return mutable target position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_target_position_data(ruckig_input_t* input);
/** Return mutable target velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_target_velocity_data(ruckig_input_t* input);
/** Return mutable target acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_target_acceleration_data(ruckig_input_t* input);
/** Return mutable maximum velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_max_velocity_data(ruckig_input_t* input);
/** Return mutable maximum acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_max_acceleration_data(ruckig_input_t* input);
/** Return mutable maximum jerk array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_max_jerk_data(ruckig_input_t* input);
/** Return mutable maximum position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_max_position_data(ruckig_input_t* input);
/** Return mutable minimum position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_input_min_position_data(ruckig_input_t* input);
/** Return mutable enabled-DoF array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API bool* ruckig_input_enabled_data(ruckig_input_t* input);

/** Return const current position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_current_position_const_data(const ruckig_input_t* input);
/** Return const current velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_current_velocity_const_data(const ruckig_input_t* input);
/** Return const current acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_current_acceleration_const_data(const ruckig_input_t* input);
/** Return const target position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_target_position_const_data(const ruckig_input_t* input);
/** Return const target velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_target_velocity_const_data(const ruckig_input_t* input);
/** Return const target acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_target_acceleration_const_data(const ruckig_input_t* input);
/** Return const maximum velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_max_velocity_const_data(const ruckig_input_t* input);
/** Return const maximum acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_max_acceleration_const_data(const ruckig_input_t* input);
/** Return const maximum jerk array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_max_jerk_const_data(const ruckig_input_t* input);
/** Return const maximum position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_max_position_const_data(const ruckig_input_t* input);
/** Return const minimum position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_input_min_position_const_data(const ruckig_input_t* input);
/** Return const enabled-DoF array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const bool* ruckig_input_enabled_const_data(const ruckig_input_t* input);

/** Set the global control interface for all DoFs. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_control_interface(
    ruckig_input_t* input,
    ruckig_control_interface_t control_interface
);

/** Set the global synchronization policy for all DoFs. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_synchronization(
    ruckig_input_t* input,
    ruckig_synchronization_t synchronization
);

/** Set per-DoF control-interface overrides; `count` must equal input DoF count. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_dof_control_interface(
    ruckig_input_t* input,
    const ruckig_control_interface_t* values,
    size_t count
);

/** Clear per-DoF control-interface overrides; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_dof_control_interface(ruckig_input_t* input);

/** Set per-DoF synchronization overrides; `count` must equal input DoF count. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_dof_synchronization(
    ruckig_input_t* input,
    const ruckig_synchronization_t* values,
    size_t count
);

/** Clear per-DoF synchronization overrides; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_dof_synchronization(ruckig_input_t* input);

/** Set duration discretization mode. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_duration_discretization(
    ruckig_input_t* input,
    ruckig_duration_discretization_t duration_discretization
);

/** Enable or disable one DoF by index. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_dof_enabled(
    ruckig_input_t* input,
    size_t dof,
    bool enabled
);

/** Set optional minimum velocity limits; `count` must equal input DoF count. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_min_velocity(
    ruckig_input_t* input,
    const double* min_velocity,
    size_t count
);

/** Clear optional minimum velocity limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_min_velocity(ruckig_input_t* input);

/** Set optional minimum acceleration limits; `count` must equal input DoF count. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_min_acceleration(
    ruckig_input_t* input,
    const double* min_acceleration,
    size_t count
);

/** Clear optional minimum acceleration limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_min_acceleration(ruckig_input_t* input);

/** Set a global minimum trajectory duration. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_minimum_duration(
    ruckig_input_t* input,
    double minimum_duration
);

/** Clear the global minimum trajectory duration; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_minimum_duration(ruckig_input_t* input);

/** Set flattened intermediate waypoint positions as `waypoint_count * dofs` doubles. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_intermediate_positions(
    ruckig_input_t* input,
    const double* flat_positions,
    size_t waypoint_count,
    size_t dofs
);
/** Clear intermediate waypoint positions; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_intermediate_positions(ruckig_input_t* input);
/** Return the active intermediate waypoint count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_input_get_intermediate_position_count(const ruckig_input_t* input);
/** Copy flattened intermediate waypoint positions into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_intermediate_positions(
    const ruckig_input_t* input,
    double* flat_positions,
    size_t capacity
);

/** Set flattened per-section maximum velocity limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_max_velocity(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section maximum velocity limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_max_velocity(ruckig_input_t* input);
/** Return whether per-section maximum velocity limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_max_velocity(const ruckig_input_t* input);
/** Copy flattened per-section maximum velocity limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_max_velocity(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set flattened per-section minimum velocity limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_min_velocity(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section minimum velocity limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_min_velocity(ruckig_input_t* input);
/** Return whether per-section minimum velocity limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_min_velocity(const ruckig_input_t* input);
/** Copy flattened per-section minimum velocity limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_min_velocity(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set flattened per-section maximum acceleration limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_max_acceleration(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section maximum acceleration limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_max_acceleration(ruckig_input_t* input);
/** Return whether per-section maximum acceleration limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_max_acceleration(const ruckig_input_t* input);
/** Copy flattened per-section maximum acceleration limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_max_acceleration(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set flattened per-section minimum acceleration limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_min_acceleration(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section minimum acceleration limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_min_acceleration(ruckig_input_t* input);
/** Return whether per-section minimum acceleration limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_min_acceleration(const ruckig_input_t* input);
/** Copy flattened per-section minimum acceleration limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_min_acceleration(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set flattened per-section maximum jerk limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_max_jerk(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section maximum jerk limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_max_jerk(ruckig_input_t* input);
/** Return whether per-section maximum jerk limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_max_jerk(const ruckig_input_t* input);
/** Copy flattened per-section maximum jerk limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_max_jerk(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set flattened per-section maximum position limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_max_position(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section maximum position limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_max_position(ruckig_input_t* input);
/** Return whether per-section maximum position limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_max_position(const ruckig_input_t* input);
/** Copy flattened per-section maximum position limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_max_position(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set flattened per-section minimum position limits. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_min_position(
    ruckig_input_t* input,
    const double* values,
    size_t section_count,
    size_t dofs
);
/** Clear per-section minimum position limits; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_min_position(ruckig_input_t* input);
/** Return whether per-section minimum position limits are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_min_position(const ruckig_input_t* input);
/** Copy flattened per-section minimum position limits into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_min_position(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set per-section minimum durations; `section_count` must match the active section count. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_per_section_minimum_duration(
    ruckig_input_t* input,
    const double* values,
    size_t section_count
);
/** Clear per-section minimum durations; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_per_section_minimum_duration(ruckig_input_t* input);
/** Return whether per-section minimum durations are active. */
RUCKIG_C_API bool ruckig_input_has_per_section_minimum_duration(const ruckig_input_t* input);
/** Copy per-section minimum durations into caller storage. */
RUCKIG_C_API ruckig_result_t ruckig_input_get_per_section_minimum_duration(
    const ruckig_input_t* input,
    double* values,
    size_t capacity
);

/** Set an optional calculation interrupt budget in microseconds. */
RUCKIG_C_API ruckig_result_t ruckig_input_set_interrupt_calculation_duration(
    ruckig_input_t* input,
    double interrupt_calculation_duration
);
/** Clear the optional calculation interrupt budget; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_input_clear_interrupt_calculation_duration(ruckig_input_t* input);

/** Return trajectory DoF count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_trajectory_get_dof_count(const ruckig_trajectory_t* trajectory);

/** Return trajectory duration in seconds, or `0.0` for `NULL`. */
RUCKIG_C_API double ruckig_trajectory_get_duration(const ruckig_trajectory_t* trajectory);
/** Return trajectory section count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_trajectory_get_section_count(const ruckig_trajectory_t* trajectory);
/** Return number of intermediate section durations. */
RUCKIG_C_API size_t ruckig_trajectory_get_intermediate_duration_count(const ruckig_trajectory_t* trajectory);
/**
 * Copy intermediate section durations into caller storage.
 *
 * Returns `RUCKIG_WORKING` on success and commonly
 * `RUCKIG_ERROR_INVALID_INPUT` for `NULL` handles/storage or mismatched
 * `duration_count`.
 */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_intermediate_durations(
    const ruckig_trajectory_t* trajectory,
    double* durations,
    size_t duration_count
);

/** Copy per-DoF independent minimum durations into caller storage; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_independent_min_durations(
    const ruckig_trajectory_t* trajectory,
    double* durations,
    size_t duration_count
);

/**
 * Sample trajectory state at `time`; `position` is required, other outputs may
 * be `NULL`.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid handles/storage and
 * `RUCKIG_ERROR_TRAJECTORY_DURATION` for invalid sample times.
 */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_at_time(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
);

/** Copy per-DoF position extrema into caller storage; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_position_extrema(
    const ruckig_trajectory_t* trajectory,
    ruckig_position_extrema_t* extrema,
    size_t extrema_count
);

/** Find the first time at which one DoF reaches `position` after `time_after`; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_first_time_at_position(
    const ruckig_trajectory_t* trajectory,
    size_t dof,
    double position,
    double time_after,
    double* time,
    bool* found
);

/**
 * Filter flattened intermediate positions by per-DoF distance threshold.
 *
 * Returns `RUCKIG_WORKING` on success and commonly
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid handles, counts, capacity, storage,
 * or non-finite/negative threshold values.
 */
RUCKIG_C_API ruckig_result_t ruckig_filter_intermediate_positions(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    const double* threshold_distance,
    size_t threshold_count,
    double* filtered_positions,
    size_t capacity,
    size_t* written_waypoints
);

/**
 * Create a tracking solver for `dofs` degrees of freedom and positive
 * `delta_time`.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid arguments and `RUCKIG_ERROR` for
 * failed internal storage allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_tracking_create(ruckig_tracking_t** tracking, size_t dofs, double delta_time);
/** Destroy a tracking solver; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_tracking_destroy(ruckig_tracking_t* tracking);
/** Return tracking DoF count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_get_dof_count(const ruckig_tracking_t* tracking);
/** Return tracking control-cycle duration, or `0.0` for `NULL`. */
RUCKIG_C_API double ruckig_tracking_get_delta_time(const ruckig_tracking_t* tracking);
/** Set tracking mode; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_set_mode(ruckig_tracking_t* tracking, ruckig_tracking_mode_t mode);
/** Return current tracking mode, or Fast for `NULL`. */
RUCKIG_C_API ruckig_tracking_mode_t ruckig_tracking_get_mode(const ruckig_tracking_t* tracking);
/** Set tracking reactiveness; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_set_reactiveness(ruckig_tracking_t* tracking, double reactiveness);
/** Return tracking reactiveness, or `0.0` for `NULL`. */
RUCKIG_C_API double ruckig_tracking_get_reactiveness(const ruckig_tracking_t* tracking);
/** Set tracking look-ahead cycle count; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_set_look_ahead_cycles(ruckig_tracking_t* tracking, size_t look_ahead_cycles);
/** Return tracking look-ahead cycle count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_get_look_ahead_cycles(const ruckig_tracking_t* tracking);
/** Set maximum optimized tracking candidate count; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_set_max_optimized_candidates(
    ruckig_tracking_t* tracking,
    size_t max_candidates
);
/** Return maximum optimized tracking candidate count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_get_max_optimized_candidates(const ruckig_tracking_t* tracking);
/** Set optimized tracking strategy; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_set_optimized_strategy(
    ruckig_tracking_t* tracking,
    ruckig_tracking_optimized_strategy_t strategy
);
/** Return optimized tracking strategy, or Balanced for `NULL`. */
RUCKIG_C_API ruckig_tracking_optimized_strategy_t ruckig_tracking_get_optimized_strategy(
    const ruckig_tracking_t* tracking
);
/** Return the last tracking calculation status. */
RUCKIG_C_API ruckig_tracking_calculation_status_t ruckig_tracking_get_last_calculation_status(
    const ruckig_tracking_t* tracking
);
/** Return last tracking candidate count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_get_last_candidate_count(const ruckig_tracking_t* tracking);
/** Copy specialized tracking diagnostics into caller storage; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_get_last_diagnostics(
    const ruckig_tracking_t* tracking,
    ruckig_tracking_diagnostics_t* diagnostics
);
/** Copy stable coarse tracking diagnostics into caller storage; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_get_last_public_diagnostics(
    const ruckig_tracking_t* tracking,
    ruckig_diagnostics_t* diagnostics
);
/**
 * Run one tracking update against a single target state.
 *
 * Returns `RUCKIG_WORKING` or `RUCKIG_FINISHED` for normal online control flow.
 * Common failures include invalid input, limit, execution-time, and
 * synchronization errors from the underlying OTG calculation.
 */
RUCKIG_C_API ruckig_result_t ruckig_tracking_update(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_t* target_state,
    const ruckig_input_t* input,
    ruckig_output_t* output
);
/** Run one tracking update using a look-ahead target sequence; return codes match `ruckig_tracking_update`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_update_with_lookahead(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_output_t* output
);
/**
 * Calculate a complete tracking output sequence.
 *
 * Returns `RUCKIG_WORKING` on successful sequence calculation. Common failures
 * include invalid handles/capacity and calculation errors from the underlying
 * tracking update path.
 */
RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence
);
/** Start an interruptible tracking sequence calculation; returns `RUCKIG_WORKING` on a complete or resumable start, or commonly invalid-input/calculation errors. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_calculate_sequence_interruptible(
    ruckig_tracking_t* tracking,
    const ruckig_target_state_sequence_t* target_sequence,
    const ruckig_input_t* input,
    ruckig_tracking_output_sequence_t* output_sequence,
    ruckig_tracking_sequence_continuation_t* continuation
);
/** Resume an interrupted tracking sequence calculation; returns `RUCKIG_WORKING` on successful progress or commonly invalid-input/calculation errors. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_resume_sequence(
    ruckig_tracking_t* tracking,
    ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_tracking_output_sequence_t* output_sequence
);

/**
 * Create a single tracking target-state handle.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid arguments and `RUCKIG_ERROR` for
 * failed internal storage allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_target_state_create(ruckig_target_state_t** target_state, size_t dofs);
/** Destroy a target-state handle; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_target_state_destroy(ruckig_target_state_t* target_state);
/** Return target-state DoF count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_target_state_get_dof_count(const ruckig_target_state_t* target_state);
/** Return mutable target position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_target_state_position_data(ruckig_target_state_t* target_state);
/** Return mutable target velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_target_state_velocity_data(ruckig_target_state_t* target_state);
/** Return mutable target acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API double* ruckig_target_state_acceleration_data(ruckig_target_state_t* target_state);
/** Return const target position array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_target_state_position_const_data(const ruckig_target_state_t* target_state);
/** Return const target velocity array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_target_state_velocity_const_data(const ruckig_target_state_t* target_state);
/** Return const target acceleration array of length DoF, or `NULL` for `NULL`. */
RUCKIG_C_API const double* ruckig_target_state_acceleration_const_data(const ruckig_target_state_t* target_state);

/**
 * Create a fixed-capacity target-state sequence.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid arguments or unsupported capacity,
 * and `RUCKIG_ERROR` for failed internal storage allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_target_state_sequence_create(
    ruckig_target_state_sequence_t** sequence,
    size_t dofs,
    size_t capacity
);
/** Destroy a target-state sequence; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_target_state_sequence_destroy(ruckig_target_state_sequence_t* sequence);
/** Return target-state sequence DoF count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_target_state_sequence_get_dof_count(const ruckig_target_state_sequence_t* sequence);
/** Return target-state sequence capacity, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_target_state_sequence_get_capacity(const ruckig_target_state_sequence_t* sequence);
/** Return active target-state sequence count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_target_state_sequence_get_count(const ruckig_target_state_sequence_t* sequence);
/** Set active target-state sequence count; must not exceed capacity; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_target_state_sequence_set_count(ruckig_target_state_sequence_t* sequence, size_t count);
/** Clear active target-state sequence count and storage; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_target_state_sequence_clear(ruckig_target_state_sequence_t* sequence);
/** Return mutable flattened sequence position array of `capacity * dofs` doubles. */
RUCKIG_C_API double* ruckig_target_state_sequence_position_data(ruckig_target_state_sequence_t* sequence);
/** Return mutable flattened sequence velocity array of `capacity * dofs` doubles. */
RUCKIG_C_API double* ruckig_target_state_sequence_velocity_data(ruckig_target_state_sequence_t* sequence);
/** Return mutable flattened sequence acceleration array of `capacity * dofs` doubles. */
RUCKIG_C_API double* ruckig_target_state_sequence_acceleration_data(ruckig_target_state_sequence_t* sequence);
/** Return const flattened sequence position array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_target_state_sequence_position_const_data(const ruckig_target_state_sequence_t* sequence);
/** Return const flattened sequence velocity array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_target_state_sequence_velocity_const_data(const ruckig_target_state_sequence_t* sequence);
/** Return const flattened sequence acceleration array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_target_state_sequence_acceleration_const_data(const ruckig_target_state_sequence_t* sequence);

/**
 * Create a fixed-capacity tracking output sequence.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid arguments or unsupported capacity,
 * and `RUCKIG_ERROR` for failed internal storage allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_tracking_output_sequence_create(
    ruckig_tracking_output_sequence_t** sequence,
    size_t dofs,
    size_t capacity
);
/** Destroy a tracking output sequence; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_tracking_output_sequence_destroy(ruckig_tracking_output_sequence_t* sequence);
/** Return tracking output sequence DoF count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_output_sequence_get_dof_count(const ruckig_tracking_output_sequence_t* sequence);
/** Return tracking output sequence capacity, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_output_sequence_get_capacity(const ruckig_tracking_output_sequence_t* sequence);
/** Return active tracking output sequence count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_output_sequence_get_count(const ruckig_tracking_output_sequence_t* sequence);
/** Clear active tracking output sequence state; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_tracking_output_sequence_clear(ruckig_tracking_output_sequence_t* sequence);
/** Return const flattened output position array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_position_const_data(const ruckig_tracking_output_sequence_t* sequence);
/** Return const flattened output velocity array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_velocity_const_data(const ruckig_tracking_output_sequence_t* sequence);
/** Return const flattened output acceleration array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_acceleration_const_data(const ruckig_tracking_output_sequence_t* sequence);
/** Return const flattened output jerk array of `capacity * dofs` doubles. */
RUCKIG_C_API const double* ruckig_tracking_output_sequence_new_jerk_const_data(const ruckig_tracking_output_sequence_t* sequence);
/** Return const output time array of length capacity. */
RUCKIG_C_API const double* ruckig_tracking_output_sequence_time_const_data(const ruckig_tracking_output_sequence_t* sequence);
/** Return const output section array of length capacity. */
RUCKIG_C_API const size_t* ruckig_tracking_output_sequence_section_const_data(const ruckig_tracking_output_sequence_t* sequence);
/** Return const output result-code array of length capacity. */
RUCKIG_C_API const ruckig_result_t* ruckig_tracking_output_sequence_result_const_data(const ruckig_tracking_output_sequence_t* sequence);

/**
 * Create an interruptible tracking sequence continuation handle.
 *
 * Returns `RUCKIG_WORKING` on success. Common failures include
 * `RUCKIG_ERROR_INVALID_INPUT` for invalid arguments or unsupported capacity,
 * and `RUCKIG_ERROR` for failed internal storage allocation.
 */
RUCKIG_C_API ruckig_result_t ruckig_tracking_sequence_continuation_create(
    ruckig_tracking_sequence_continuation_t** continuation,
    size_t dofs,
    size_t capacity
);
/** Destroy a tracking sequence continuation; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_tracking_sequence_continuation_destroy(
    ruckig_tracking_sequence_continuation_t* continuation
);
/** Reset a tracking sequence continuation state; `NULL` is a no-op. */
RUCKIG_C_API void ruckig_tracking_sequence_continuation_reset(
    ruckig_tracking_sequence_continuation_t* continuation
);
/** Return continuation DoF count, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_dof_count(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Return continuation capacity, or `0` for `NULL`. */
RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_capacity(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Return whether the continuation currently has resumable state. */
RUCKIG_C_API bool ruckig_tracking_sequence_continuation_is_active(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Return whether the last interruptible calculation was interrupted. */
RUCKIG_C_API bool ruckig_tracking_sequence_continuation_was_interrupted(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Return whether the tracked sequence calculation is complete. */
RUCKIG_C_API bool ruckig_tracking_sequence_continuation_is_complete(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Return completed output count for the current continuation state. */
RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_completed_count(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Return target count for the current continuation state. */
RUCKIG_C_API size_t ruckig_tracking_sequence_continuation_get_target_count(
    const ruckig_tracking_sequence_continuation_t* continuation
);
/** Copy stable coarse continuation diagnostics into caller storage; returns `RUCKIG_WORKING` or commonly `RUCKIG_ERROR_INVALID_INPUT`. */
RUCKIG_C_API ruckig_result_t ruckig_tracking_sequence_continuation_get_last_diagnostics(
    const ruckig_tracking_sequence_continuation_t* continuation,
    ruckig_diagnostics_t* diagnostics
);

#ifdef __cplusplus
}
#endif

#endif
