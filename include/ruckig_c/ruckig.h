#ifndef RUCKIG_C_RUCKIG_H
#define RUCKIG_C_RUCKIG_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RUCKIG_C_VERSION_MAJOR 0
#define RUCKIG_C_VERSION_MINOR 2
#define RUCKIG_C_VERSION_PATCH 1
#define RUCKIG_C_VERSION_STRING "0.2.1"

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

typedef enum ruckig_control_interface {
    RUCKIG_CONTROL_POSITION = 0,
    RUCKIG_CONTROL_VELOCITY = 1
} ruckig_control_interface_t;

typedef enum ruckig_synchronization {
    RUCKIG_SYNCHRONIZATION_TIME = 0,
    RUCKIG_SYNCHRONIZATION_TIME_IF_NECESSARY = 1,
    RUCKIG_SYNCHRONIZATION_PHASE = 2,
    RUCKIG_SYNCHRONIZATION_NONE = 3
} ruckig_synchronization_t;

typedef enum ruckig_duration_discretization {
    RUCKIG_DURATION_CONTINUOUS = 0,
    RUCKIG_DURATION_DISCRETE = 1
} ruckig_duration_discretization_t;

typedef struct ruckig_position_extrema {
    double min_position;
    double max_position;
    double time_min;
    double time_max;
} ruckig_position_extrema_t;

#ifdef __cplusplus
typedef struct ruckig_c_input_handle ruckig_input_t;
typedef struct ruckig_c_output_handle ruckig_output_t;
typedef struct ruckig_c_trajectory_handle ruckig_trajectory_t;
typedef struct ruckig_c_handle ruckig_t;
#else
typedef struct ruckig_input ruckig_input_t;
typedef struct ruckig_output ruckig_output_t;
typedef struct ruckig_trajectory ruckig_trajectory_t;
typedef struct ruckig ruckig_t;
#endif

/*
 * Current unsupported scope:
 * - No intermediate waypoints.
 * - No per-section constraints.
 * - No cloud API.
 * - No Python or Rust bindings in this C ABI.
 *
 * These unsupported features are intentionally not exposed as public setters.
 * If compatibility entry points are added later, they must fail explicitly with
 * RUCKIG_ERROR_UNSUPPORTED instead of silently ignoring caller input.
 */

RUCKIG_C_API ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time);
RUCKIG_C_API void ruckig_destroy(ruckig_t* otg);

RUCKIG_C_API ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs);
RUCKIG_C_API void ruckig_input_destroy(ruckig_input_t* input);

RUCKIG_C_API ruckig_result_t ruckig_output_create(ruckig_output_t** output, size_t dofs);
RUCKIG_C_API void ruckig_output_destroy(ruckig_output_t* output);

RUCKIG_C_API ruckig_result_t ruckig_trajectory_create(ruckig_trajectory_t** trajectory, size_t dofs);
RUCKIG_C_API void ruckig_trajectory_destroy(ruckig_trajectory_t* trajectory);

RUCKIG_C_API ruckig_result_t ruckig_validate_input(
    const ruckig_t* otg,
    const ruckig_input_t* input,
    bool check_current_state_within_limits,
    bool check_target_state_within_limits
);

RUCKIG_C_API ruckig_result_t ruckig_calculate(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_trajectory_t* trajectory
);

RUCKIG_C_API ruckig_result_t ruckig_update(
    ruckig_t* otg,
    const ruckig_input_t* input,
    ruckig_output_t* output
);

RUCKIG_C_API void ruckig_reset(ruckig_t* otg);

RUCKIG_C_API void ruckig_output_pass_to_input(
    const ruckig_output_t* output,
    ruckig_input_t* input
);

RUCKIG_C_API size_t ruckig_output_get_dof_count(const ruckig_output_t* output);

RUCKIG_C_API const double* ruckig_output_new_position_data(const ruckig_output_t* output);
RUCKIG_C_API const double* ruckig_output_new_velocity_data(const ruckig_output_t* output);
RUCKIG_C_API const double* ruckig_output_new_acceleration_data(const ruckig_output_t* output);
RUCKIG_C_API const double* ruckig_output_new_jerk_data(const ruckig_output_t* output);

RUCKIG_C_API double ruckig_output_get_time(const ruckig_output_t* output);
RUCKIG_C_API size_t ruckig_output_get_new_section(const ruckig_output_t* output);
RUCKIG_C_API bool ruckig_output_did_section_change(const ruckig_output_t* output);
RUCKIG_C_API bool ruckig_output_new_calculation(const ruckig_output_t* output);
RUCKIG_C_API bool ruckig_output_was_calculation_interrupted(const ruckig_output_t* output);
RUCKIG_C_API double ruckig_output_get_calculation_duration(const ruckig_output_t* output);

RUCKIG_C_API const ruckig_trajectory_t* ruckig_output_get_trajectory(const ruckig_output_t* output);

RUCKIG_C_API size_t ruckig_input_get_dof_count(const ruckig_input_t* input);

RUCKIG_C_API double* ruckig_input_current_position_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_current_velocity_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_current_acceleration_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_target_position_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_target_velocity_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_target_acceleration_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_max_velocity_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_max_acceleration_data(ruckig_input_t* input);
RUCKIG_C_API double* ruckig_input_max_jerk_data(ruckig_input_t* input);
RUCKIG_C_API bool* ruckig_input_enabled_data(ruckig_input_t* input);

RUCKIG_C_API const double* ruckig_input_current_position_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_current_velocity_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_current_acceleration_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_target_position_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_target_velocity_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_target_acceleration_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_max_velocity_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_max_acceleration_const_data(const ruckig_input_t* input);
RUCKIG_C_API const double* ruckig_input_max_jerk_const_data(const ruckig_input_t* input);
RUCKIG_C_API const bool* ruckig_input_enabled_const_data(const ruckig_input_t* input);

RUCKIG_C_API ruckig_result_t ruckig_input_set_control_interface(
    ruckig_input_t* input,
    ruckig_control_interface_t control_interface
);

RUCKIG_C_API ruckig_result_t ruckig_input_set_synchronization(
    ruckig_input_t* input,
    ruckig_synchronization_t synchronization
);

RUCKIG_C_API ruckig_result_t ruckig_input_set_per_dof_control_interface(
    ruckig_input_t* input,
    const ruckig_control_interface_t* values,
    size_t count
);

RUCKIG_C_API void ruckig_input_clear_per_dof_control_interface(ruckig_input_t* input);

RUCKIG_C_API ruckig_result_t ruckig_input_set_per_dof_synchronization(
    ruckig_input_t* input,
    const ruckig_synchronization_t* values,
    size_t count
);

RUCKIG_C_API void ruckig_input_clear_per_dof_synchronization(ruckig_input_t* input);

RUCKIG_C_API ruckig_result_t ruckig_input_set_duration_discretization(
    ruckig_input_t* input,
    ruckig_duration_discretization_t duration_discretization
);

RUCKIG_C_API ruckig_result_t ruckig_input_set_dof_enabled(
    ruckig_input_t* input,
    size_t dof,
    bool enabled
);

RUCKIG_C_API ruckig_result_t ruckig_input_set_min_velocity(
    ruckig_input_t* input,
    const double* min_velocity,
    size_t count
);

RUCKIG_C_API void ruckig_input_clear_min_velocity(ruckig_input_t* input);

RUCKIG_C_API ruckig_result_t ruckig_input_set_min_acceleration(
    ruckig_input_t* input,
    const double* min_acceleration,
    size_t count
);

RUCKIG_C_API void ruckig_input_clear_min_acceleration(ruckig_input_t* input);

RUCKIG_C_API ruckig_result_t ruckig_input_set_minimum_duration(
    ruckig_input_t* input,
    double minimum_duration
);

RUCKIG_C_API void ruckig_input_clear_minimum_duration(ruckig_input_t* input);

RUCKIG_C_API size_t ruckig_trajectory_get_dof_count(const ruckig_trajectory_t* trajectory);

RUCKIG_C_API double ruckig_trajectory_get_duration(const ruckig_trajectory_t* trajectory);

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_independent_min_durations(
    const ruckig_trajectory_t* trajectory,
    double* durations,
    size_t duration_count
);

RUCKIG_C_API ruckig_result_t ruckig_trajectory_at_time(
    const ruckig_trajectory_t* trajectory,
    double time,
    double* position,
    double* velocity,
    double* acceleration,
    double* jerk,
    size_t* section
);

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_position_extrema(
    const ruckig_trajectory_t* trajectory,
    ruckig_position_extrema_t* extrema,
    size_t extrema_count
);

RUCKIG_C_API ruckig_result_t ruckig_trajectory_get_first_time_at_position(
    const ruckig_trajectory_t* trajectory,
    size_t dof,
    double position,
    double time_after,
    double* time,
    bool* found
);

#ifdef __cplusplus
}
#endif

#endif
