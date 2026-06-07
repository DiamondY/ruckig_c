from __future__ import annotations

import enum
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Optional, Sequence

from cffi import FFI


ffi = FFI()
ffi.cdef(
    """
    typedef struct ruckig ruckig_t;
    typedef struct ruckig_input ruckig_input_t;
    typedef struct ruckig_output ruckig_output_t;
    typedef struct ruckig_trajectory ruckig_trajectory_t;
    typedef struct ruckig_tracking ruckig_tracking_t;
    typedef struct ruckig_target_state ruckig_target_state_t;
    typedef struct ruckig_target_state_sequence ruckig_target_state_sequence_t;
    typedef struct ruckig_tracking_output_sequence ruckig_tracking_output_sequence_t;
    typedef int ruckig_result_t;
    typedef _Bool bool;

    typedef struct ruckig_position_extrema {
        double min_position;
        double max_position;
        double time_min;
        double time_max;
    } ruckig_position_extrema_t;

    ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time);
    ruckig_result_t ruckig_create_with_waypoints(
        ruckig_t** otg,
        size_t dofs,
        double delta_time,
        size_t max_number_of_waypoints
    );
    void ruckig_destroy(ruckig_t* otg);
    size_t ruckig_get_max_number_of_waypoints(const ruckig_t* otg);

    ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs);
    ruckig_result_t ruckig_input_create_with_waypoints(
        ruckig_input_t** input,
        size_t dofs,
        size_t max_number_of_waypoints
    );
    void ruckig_input_destroy(ruckig_input_t* input);

    ruckig_result_t ruckig_output_create(ruckig_output_t** output, size_t dofs);
    ruckig_result_t ruckig_output_create_with_waypoints(
        ruckig_output_t** output,
        size_t dofs,
        size_t max_number_of_waypoints
    );
    void ruckig_output_destroy(ruckig_output_t* output);

    ruckig_result_t ruckig_trajectory_create(ruckig_trajectory_t** trajectory, size_t dofs);
    ruckig_result_t ruckig_trajectory_create_with_waypoints(
        ruckig_trajectory_t** trajectory,
        size_t dofs,
        size_t max_number_of_waypoints
    );
    void ruckig_trajectory_destroy(ruckig_trajectory_t* trajectory);

    ruckig_result_t ruckig_calculate(
        ruckig_t* otg,
        const ruckig_input_t* input,
        ruckig_trajectory_t* trajectory
    );
    ruckig_result_t ruckig_update(
        ruckig_t* otg,
        const ruckig_input_t* input,
        ruckig_output_t* output
    );
    void ruckig_output_pass_to_input(const ruckig_output_t* output, ruckig_input_t* input);
    void ruckig_reset(ruckig_t* otg);

    size_t ruckig_input_get_dof_count(const ruckig_input_t* input);
    double* ruckig_input_current_position_data(ruckig_input_t* input);
    double* ruckig_input_current_velocity_data(ruckig_input_t* input);
    double* ruckig_input_current_acceleration_data(ruckig_input_t* input);
    double* ruckig_input_target_position_data(ruckig_input_t* input);
    double* ruckig_input_target_velocity_data(ruckig_input_t* input);
    double* ruckig_input_target_acceleration_data(ruckig_input_t* input);
    double* ruckig_input_max_velocity_data(ruckig_input_t* input);
    double* ruckig_input_max_acceleration_data(ruckig_input_t* input);
    double* ruckig_input_max_jerk_data(ruckig_input_t* input);
    double* ruckig_input_max_position_data(ruckig_input_t* input);
    double* ruckig_input_min_position_data(ruckig_input_t* input);

    ruckig_result_t ruckig_input_set_control_interface(ruckig_input_t* input, int control_interface);
    ruckig_result_t ruckig_input_set_synchronization(ruckig_input_t* input, int synchronization);
    ruckig_result_t ruckig_input_set_duration_discretization(ruckig_input_t* input, int duration_discretization);
    ruckig_result_t ruckig_input_set_min_velocity(ruckig_input_t* input, const double* min_velocity, size_t count);
    void ruckig_input_clear_min_velocity(ruckig_input_t* input);
    ruckig_result_t ruckig_input_set_min_acceleration(ruckig_input_t* input, const double* min_acceleration, size_t count);
    void ruckig_input_clear_min_acceleration(ruckig_input_t* input);
    ruckig_result_t ruckig_input_set_minimum_duration(ruckig_input_t* input, double minimum_duration);
    void ruckig_input_clear_minimum_duration(ruckig_input_t* input);

    ruckig_result_t ruckig_input_set_intermediate_positions(
        ruckig_input_t* input,
        const double* flat_positions,
        size_t waypoint_count,
        size_t dofs
    );
    void ruckig_input_clear_intermediate_positions(ruckig_input_t* input);
    size_t ruckig_input_get_intermediate_position_count(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_intermediate_positions(
        const ruckig_input_t* input,
        double* flat_positions,
        size_t capacity
    );

    ruckig_result_t ruckig_input_set_per_section_max_velocity(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_max_velocity(ruckig_input_t* input);
    bool ruckig_input_has_per_section_max_velocity(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_max_velocity(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_min_velocity(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_min_velocity(ruckig_input_t* input);
    bool ruckig_input_has_per_section_min_velocity(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_min_velocity(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_max_acceleration(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_max_acceleration(ruckig_input_t* input);
    bool ruckig_input_has_per_section_max_acceleration(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_max_acceleration(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_min_acceleration(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_min_acceleration(ruckig_input_t* input);
    bool ruckig_input_has_per_section_min_acceleration(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_min_acceleration(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_max_jerk(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_max_jerk(ruckig_input_t* input);
    bool ruckig_input_has_per_section_max_jerk(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_max_jerk(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_max_position(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_max_position(ruckig_input_t* input);
    bool ruckig_input_has_per_section_max_position(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_max_position(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_min_position(ruckig_input_t* input, const double* values, size_t section_count, size_t dofs);
    void ruckig_input_clear_per_section_min_position(ruckig_input_t* input);
    bool ruckig_input_has_per_section_min_position(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_min_position(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_per_section_minimum_duration(ruckig_input_t* input, const double* values, size_t section_count);
    void ruckig_input_clear_per_section_minimum_duration(ruckig_input_t* input);
    bool ruckig_input_has_per_section_minimum_duration(const ruckig_input_t* input);
    ruckig_result_t ruckig_input_get_per_section_minimum_duration(const ruckig_input_t* input, double* values, size_t capacity);
    ruckig_result_t ruckig_input_set_interrupt_calculation_duration(ruckig_input_t* input, double interrupt_calculation_duration);
    void ruckig_input_clear_interrupt_calculation_duration(ruckig_input_t* input);

    const double* ruckig_output_new_position_data(const ruckig_output_t* output);
    const double* ruckig_output_new_velocity_data(const ruckig_output_t* output);
    const double* ruckig_output_new_acceleration_data(const ruckig_output_t* output);
    double ruckig_output_get_time(const ruckig_output_t* output);
    size_t ruckig_output_get_new_section(const ruckig_output_t* output);
    bool ruckig_output_did_section_change(const ruckig_output_t* output);
    bool ruckig_output_new_calculation(const ruckig_output_t* output);
    bool ruckig_output_was_calculation_interrupted(const ruckig_output_t* output);

    double ruckig_trajectory_get_duration(const ruckig_trajectory_t* trajectory);
    size_t ruckig_trajectory_get_section_count(const ruckig_trajectory_t* trajectory);
    size_t ruckig_trajectory_get_intermediate_duration_count(const ruckig_trajectory_t* trajectory);
    ruckig_result_t ruckig_trajectory_get_intermediate_durations(
        const ruckig_trajectory_t* trajectory,
        double* durations,
        size_t duration_count
    );
    ruckig_result_t ruckig_trajectory_at_time(
        const ruckig_trajectory_t* trajectory,
        double time,
        double* position,
        double* velocity,
        double* acceleration,
        double* jerk,
        size_t* section
    );
    ruckig_result_t ruckig_trajectory_get_position_extrema(
        const ruckig_trajectory_t* trajectory,
        ruckig_position_extrema_t* extrema,
        size_t extrema_count
    );
    ruckig_result_t ruckig_trajectory_get_first_time_at_position(
        const ruckig_trajectory_t* trajectory,
        size_t dof,
        double position,
        double time_after,
        double* time,
        bool* found
    );
    ruckig_result_t ruckig_filter_intermediate_positions(
        const ruckig_t* otg,
        const ruckig_input_t* input,
        const double* threshold_distance,
        size_t threshold_count,
        double* filtered_positions,
        size_t capacity,
        size_t* written_waypoints
    );

    ruckig_result_t ruckig_tracking_create(ruckig_tracking_t** tracking, size_t dofs, double delta_time);
    void ruckig_tracking_destroy(ruckig_tracking_t* tracking);
    size_t ruckig_tracking_get_dof_count(const ruckig_tracking_t* tracking);
    double ruckig_tracking_get_delta_time(const ruckig_tracking_t* tracking);
    ruckig_result_t ruckig_tracking_set_mode(ruckig_tracking_t* tracking, int mode);
    int ruckig_tracking_get_mode(const ruckig_tracking_t* tracking);
    ruckig_result_t ruckig_tracking_set_reactiveness(ruckig_tracking_t* tracking, double reactiveness);
    double ruckig_tracking_get_reactiveness(const ruckig_tracking_t* tracking);
    ruckig_result_t ruckig_tracking_set_look_ahead_cycles(ruckig_tracking_t* tracking, size_t look_ahead_cycles);
    size_t ruckig_tracking_get_look_ahead_cycles(const ruckig_tracking_t* tracking);
    ruckig_result_t ruckig_tracking_set_max_optimized_candidates(ruckig_tracking_t* tracking, size_t max_candidates);
    size_t ruckig_tracking_get_max_optimized_candidates(const ruckig_tracking_t* tracking);
    int ruckig_tracking_get_last_calculation_status(const ruckig_tracking_t* tracking);
    size_t ruckig_tracking_get_last_candidate_count(const ruckig_tracking_t* tracking);
    ruckig_result_t ruckig_tracking_update(
        ruckig_tracking_t* tracking,
        const ruckig_target_state_t* target_state,
        const ruckig_input_t* input,
        ruckig_output_t* output
    );
    ruckig_result_t ruckig_tracking_update_with_lookahead(
        ruckig_tracking_t* tracking,
        const ruckig_target_state_sequence_t* target_sequence,
        const ruckig_input_t* input,
        ruckig_output_t* output
    );
    ruckig_result_t ruckig_tracking_calculate_sequence(
        ruckig_tracking_t* tracking,
        const ruckig_target_state_sequence_t* target_sequence,
        const ruckig_input_t* input,
        ruckig_tracking_output_sequence_t* output_sequence
    );

    ruckig_result_t ruckig_target_state_create(ruckig_target_state_t** target_state, size_t dofs);
    void ruckig_target_state_destroy(ruckig_target_state_t* target_state);
    size_t ruckig_target_state_get_dof_count(const ruckig_target_state_t* target_state);
    double* ruckig_target_state_position_data(ruckig_target_state_t* target_state);
    double* ruckig_target_state_velocity_data(ruckig_target_state_t* target_state);
    double* ruckig_target_state_acceleration_data(ruckig_target_state_t* target_state);
    const double* ruckig_target_state_position_const_data(const ruckig_target_state_t* target_state);
    const double* ruckig_target_state_velocity_const_data(const ruckig_target_state_t* target_state);
    const double* ruckig_target_state_acceleration_const_data(const ruckig_target_state_t* target_state);

    ruckig_result_t ruckig_target_state_sequence_create(ruckig_target_state_sequence_t** sequence, size_t dofs, size_t capacity);
    void ruckig_target_state_sequence_destroy(ruckig_target_state_sequence_t* sequence);
    size_t ruckig_target_state_sequence_get_dof_count(const ruckig_target_state_sequence_t* sequence);
    size_t ruckig_target_state_sequence_get_capacity(const ruckig_target_state_sequence_t* sequence);
    size_t ruckig_target_state_sequence_get_count(const ruckig_target_state_sequence_t* sequence);
    ruckig_result_t ruckig_target_state_sequence_set_count(ruckig_target_state_sequence_t* sequence, size_t count);
    void ruckig_target_state_sequence_clear(ruckig_target_state_sequence_t* sequence);
    double* ruckig_target_state_sequence_position_data(ruckig_target_state_sequence_t* sequence);
    double* ruckig_target_state_sequence_velocity_data(ruckig_target_state_sequence_t* sequence);
    double* ruckig_target_state_sequence_acceleration_data(ruckig_target_state_sequence_t* sequence);
    const double* ruckig_target_state_sequence_position_const_data(const ruckig_target_state_sequence_t* sequence);
    const double* ruckig_target_state_sequence_velocity_const_data(const ruckig_target_state_sequence_t* sequence);
    const double* ruckig_target_state_sequence_acceleration_const_data(const ruckig_target_state_sequence_t* sequence);

    ruckig_result_t ruckig_tracking_output_sequence_create(ruckig_tracking_output_sequence_t** sequence, size_t dofs, size_t capacity);
    void ruckig_tracking_output_sequence_destroy(ruckig_tracking_output_sequence_t* sequence);
    size_t ruckig_tracking_output_sequence_get_dof_count(const ruckig_tracking_output_sequence_t* sequence);
    size_t ruckig_tracking_output_sequence_get_capacity(const ruckig_tracking_output_sequence_t* sequence);
    size_t ruckig_tracking_output_sequence_get_count(const ruckig_tracking_output_sequence_t* sequence);
    void ruckig_tracking_output_sequence_clear(ruckig_tracking_output_sequence_t* sequence);
    const double* ruckig_tracking_output_sequence_new_position_const_data(const ruckig_tracking_output_sequence_t* sequence);
    const double* ruckig_tracking_output_sequence_new_velocity_const_data(const ruckig_tracking_output_sequence_t* sequence);
    const double* ruckig_tracking_output_sequence_new_acceleration_const_data(const ruckig_tracking_output_sequence_t* sequence);
    const double* ruckig_tracking_output_sequence_new_jerk_const_data(const ruckig_tracking_output_sequence_t* sequence);
    const double* ruckig_tracking_output_sequence_time_const_data(const ruckig_tracking_output_sequence_t* sequence);
    const size_t* ruckig_tracking_output_sequence_section_const_data(const ruckig_tracking_output_sequence_t* sequence);
    const ruckig_result_t* ruckig_tracking_output_sequence_result_const_data(const ruckig_tracking_output_sequence_t* sequence);
    """
)


class Result(enum.IntEnum):
    WORKING = 0
    FINISHED = 1
    ERROR = -1
    ERROR_INVALID_INPUT = -100
    ERROR_TRAJECTORY_DURATION = -101
    ERROR_POSITIONAL_LIMITS = -102
    ERROR_ZERO_LIMITS = -104
    ERROR_EXECUTION_TIME_CALCULATION = -110
    ERROR_SYNCHRONIZATION_CALCULATION = -111
    ERROR_UNSUPPORTED = -200


class ControlInterface(enum.IntEnum):
    POSITION = 0
    VELOCITY = 1


class Synchronization(enum.IntEnum):
    TIME = 0
    TIME_IF_NECESSARY = 1
    PHASE = 2
    NONE = 3


class DurationDiscretization(enum.IntEnum):
    CONTINUOUS = 0
    DISCRETE = 1


class TrackingMode(enum.IntEnum):
    FAST = 0
    OPTIMIZED = 1


class TrackingCalculationStatus(enum.IntEnum):
    NONE = 0
    FAST = 1
    OPTIMIZED = 2
    FAST_FALLBACK = 3
    ERROR = 4


@dataclass(frozen=True)
class Bound:
    min: float
    max: float
    t_min: float
    t_max: float


class RuckigError(RuntimeError):
    def __init__(self, result: int, operation: str):
        self.result = int(result)
        self.operation = operation
        super().__init__(f"{operation} failed with ruckig result {result}")


class RuckigInvalidInputError(RuckigError):
    pass


class RuckigTrajectoryDurationError(RuckigError):
    pass


class RuckigPositionalLimitsError(RuckigError):
    pass


class RuckigZeroLimitsError(RuckigError):
    pass


class RuckigExecutionTimeCalculationError(RuckigError):
    pass


class RuckigSynchronizationCalculationError(RuckigError):
    pass


class RuckigUnsupportedError(RuckigError):
    pass


class RuckigLifecycleError(RuntimeError):
    pass


_ERROR_TYPES = {
    Result.ERROR_INVALID_INPUT: RuckigInvalidInputError,
    Result.ERROR_TRAJECTORY_DURATION: RuckigTrajectoryDurationError,
    Result.ERROR_POSITIONAL_LIMITS: RuckigPositionalLimitsError,
    Result.ERROR_ZERO_LIMITS: RuckigZeroLimitsError,
    Result.ERROR_EXECUTION_TIME_CALCULATION: RuckigExecutionTimeCalculationError,
    Result.ERROR_SYNCHRONIZATION_CALCULATION: RuckigSynchronizationCalculationError,
    Result.ERROR_UNSUPPORTED: RuckigUnsupportedError,
}


def _default_library_names() -> List[str]:
    if sys.platform.startswith("win"):
        return ["ruckig_c.dll"]
    if sys.platform == "darwin":
        return ["libruckig_c.dylib", "ruckig_c"]
    return ["libruckig_c.so", "ruckig_c"]


def load_library(path: Optional[os.PathLike[str] | str] = None):
    if path is None:
        env_path = os.environ.get("RUCKIG_C_SHARED_LIBRARY")
        if env_path:
            path = env_path

    if path is not None:
        return ffi.dlopen(str(Path(path)))

    last_error: Optional[Exception] = None
    for name in _default_library_names():
        try:
            return ffi.dlopen(name)
        except OSError as exc:
            last_error = exc
    raise OSError("could not load ruckig_c shared library") from last_error


_lib = None


def configure_library(path: Optional[os.PathLike[str] | str] = None):
    global _lib
    _lib = load_library(path)
    return _lib


def _library():
    global _lib
    if _lib is None:
        _lib = load_library()
    return _lib


def _result(value: int, operation: str) -> Result:
    result = Result(int(value))
    if result in (Result.WORKING, Result.FINISHED):
        return result
    error_type = _ERROR_TYPES.get(result, RuckigError)
    raise error_type(result, operation)


def _copy_in(ptr, values: Iterable[float], dofs: int) -> None:
    items = list(values)
    if len(items) != dofs:
        raise ValueError(f"expected {dofs} values, got {len(items)}")
    for index, value in enumerate(items):
        ptr[index] = float(value)


def _copy_out(ptr, dofs: int) -> List[float]:
    return [float(ptr[index]) for index in range(dofs)]


def _copy_in_array(values: Iterable[float]):
    items = [float(value) for value in values]
    c_values = ffi.new("double[]", len(items))
    for index, value in enumerate(items):
        c_values[index] = value
    return items, c_values


def _is_scalar(value) -> bool:
    return isinstance(value, (int, float))


def _flatten_points(points: Iterable[Sequence[float] | float], dofs: int, label: str):
    items = list(points)
    if not items:
        return [], 0
    if all(_is_scalar(item) for item in items):
        if dofs != 1:
            raise ValueError(f"{label} must contain {dofs}-value points")
        return [float(item) for item in items], len(items)

    flat: List[float] = []
    for index, point in enumerate(items):
        values = list(point)  # type: ignore[arg-type]
        if len(values) != dofs:
            raise ValueError(f"{label}[{index}] expected {dofs} values, got {len(values)}")
        flat.extend(float(value) for value in values)
    return flat, len(items)


def _unflatten_points(flat, count: int, dofs: int) -> List[List[float]]:
    return [
        [float(flat[point * dofs + dof]) for dof in range(dofs)]
        for point in range(count)
    ]


class _Handle:
    _destroy_name = ""

    def __init__(self, ptr, dofs: int):
        self._ptr = ptr
        self._dofs = int(dofs)
        self._closed = False

    @property
    def dofs(self) -> int:
        return self._dofs

    def _ensure_open(self):
        if self._closed or self._ptr == ffi.NULL:
            raise RuckigLifecycleError(f"{type(self).__name__} is closed")

    @property
    def ptr(self):
        self._ensure_open()
        return self._ptr

    def close(self) -> None:
        if not self._closed and self._ptr != ffi.NULL:
            getattr(_library(), self._destroy_name)(self._ptr)
            self._ptr = ffi.NULL
        self._closed = True

    def __enter__(self):
        self._ensure_open()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def __del__(self):
        try:
            self.close()
        except Exception:
            pass


class Ruckig(_Handle):
    _destroy_name = "ruckig_destroy"

    def __init__(self, dofs: int, delta_time: float, max_number_of_waypoints: int = 0):
        out = ffi.new("ruckig_t**")
        if max_number_of_waypoints:
            _result(
                _library().ruckig_create_with_waypoints(
                    out,
                    dofs,
                    float(delta_time),
                    int(max_number_of_waypoints),
                ),
                "ruckig_create_with_waypoints",
            )
        else:
            _result(_library().ruckig_create(out, dofs, float(delta_time)), "ruckig_create")
        super().__init__(out[0], dofs)
        self._max_number_of_waypoints = int(max_number_of_waypoints)

    @property
    def max_number_of_waypoints(self) -> int:
        return int(_library().ruckig_get_max_number_of_waypoints(self.ptr))

    def calculate(self, input_: "Input", trajectory: "Trajectory") -> Result:
        return _result(
            _library().ruckig_calculate(self.ptr, input_.ptr, trajectory.ptr),
            "ruckig_calculate",
        )

    def update(self, input_: "Input", output: "Output") -> Result:
        return _result(
            _library().ruckig_update(self.ptr, input_.ptr, output.ptr),
            "ruckig_update",
        )

    def reset(self) -> None:
        _library().ruckig_reset(self.ptr)

    def filter_intermediate_positions(
        self,
        input_: "Input",
        threshold_distance: Iterable[float],
    ) -> List[List[float]]:
        threshold_items, threshold = _copy_in_array(threshold_distance)
        if len(threshold_items) != self.dofs:
            raise ValueError(f"expected {self.dofs} threshold values, got {len(threshold_items)}")
        waypoint_count = input_.intermediate_position_count
        filtered = ffi.new("double[]", waypoint_count * self.dofs)
        written = ffi.new("size_t*")
        _result(
            _library().ruckig_filter_intermediate_positions(
                self.ptr,
                input_.ptr,
                threshold,
                len(threshold_items),
                filtered,
                waypoint_count * self.dofs,
                written,
            ),
            "ruckig_filter_intermediate_positions",
        )
        return _unflatten_points(filtered, int(written[0]), self.dofs)


class TargetState(_Handle):
    _destroy_name = "ruckig_target_state_destroy"

    def __init__(self, dofs: int):
        out = ffi.new("ruckig_target_state_t**")
        _result(_library().ruckig_target_state_create(out, dofs), "ruckig_target_state_create")
        super().__init__(out[0], dofs)

    def _set_vector(self, accessor: str, values: Iterable[float]) -> None:
        _copy_in(getattr(_library(), accessor)(self.ptr), values, self.dofs)

    def set_position(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_target_state_position_data", values)

    def set_velocity(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_target_state_velocity_data", values)

    def set_acceleration(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_target_state_acceleration_data", values)

    def position(self) -> List[float]:
        return _copy_out(_library().ruckig_target_state_position_const_data(self.ptr), self.dofs)

    def velocity(self) -> List[float]:
        return _copy_out(_library().ruckig_target_state_velocity_const_data(self.ptr), self.dofs)

    def acceleration(self) -> List[float]:
        return _copy_out(_library().ruckig_target_state_acceleration_const_data(self.ptr), self.dofs)


class TargetStateSequence(_Handle):
    _destroy_name = "ruckig_target_state_sequence_destroy"

    def __init__(self, dofs: int, capacity: int):
        out = ffi.new("ruckig_target_state_sequence_t**")
        _result(
            _library().ruckig_target_state_sequence_create(out, dofs, int(capacity)),
            "ruckig_target_state_sequence_create",
        )
        super().__init__(out[0], dofs)
        self._capacity = int(capacity)

    @property
    def capacity(self) -> int:
        return int(_library().ruckig_target_state_sequence_get_capacity(self.ptr))

    @property
    def count(self) -> int:
        return int(_library().ruckig_target_state_sequence_get_count(self.ptr))

    def set_count(self, count: int) -> None:
        _result(
            _library().ruckig_target_state_sequence_set_count(self.ptr, int(count)),
            "ruckig_target_state_sequence_set_count",
        )

    def clear(self) -> None:
        _library().ruckig_target_state_sequence_clear(self.ptr)

    def set_state(self, index: int, position: Iterable[float], velocity: Iterable[float], acceleration: Iterable[float]) -> None:
        if index < 0 or index >= self.capacity:
            raise ValueError(f"target state index out of range: {index}")
        offset = int(index) * self.dofs
        position_values = list(position)
        velocity_values = list(velocity)
        acceleration_values = list(acceleration)
        if len(position_values) != self.dofs or len(velocity_values) != self.dofs or len(acceleration_values) != self.dofs:
            raise ValueError(f"expected {self.dofs} values for position, velocity, and acceleration")
        pos = _library().ruckig_target_state_sequence_position_data(self.ptr)
        vel = _library().ruckig_target_state_sequence_velocity_data(self.ptr)
        acc = _library().ruckig_target_state_sequence_acceleration_data(self.ptr)
        for dof in range(self.dofs):
            pos[offset + dof] = float(position_values[dof])
            vel[offset + dof] = float(velocity_values[dof])
            acc[offset + dof] = float(acceleration_values[dof])

    def positions(self) -> List[List[float]]:
        return _unflatten_points(
            _library().ruckig_target_state_sequence_position_const_data(self.ptr),
            self.count,
            self.dofs,
        )


class TrackingOutputSequence(_Handle):
    _destroy_name = "ruckig_tracking_output_sequence_destroy"

    def __init__(self, dofs: int, capacity: int):
        out = ffi.new("ruckig_tracking_output_sequence_t**")
        _result(
            _library().ruckig_tracking_output_sequence_create(out, dofs, int(capacity)),
            "ruckig_tracking_output_sequence_create",
        )
        super().__init__(out[0], dofs)
        self._capacity = int(capacity)

    @property
    def capacity(self) -> int:
        return int(_library().ruckig_tracking_output_sequence_get_capacity(self.ptr))

    @property
    def count(self) -> int:
        return int(_library().ruckig_tracking_output_sequence_get_count(self.ptr))

    def clear(self) -> None:
        _library().ruckig_tracking_output_sequence_clear(self.ptr)

    def new_positions(self) -> List[List[float]]:
        return _unflatten_points(
            _library().ruckig_tracking_output_sequence_new_position_const_data(self.ptr),
            self.count,
            self.dofs,
        )

    def new_velocities(self) -> List[List[float]]:
        return _unflatten_points(
            _library().ruckig_tracking_output_sequence_new_velocity_const_data(self.ptr),
            self.count,
            self.dofs,
        )

    def new_accelerations(self) -> List[List[float]]:
        return _unflatten_points(
            _library().ruckig_tracking_output_sequence_new_acceleration_const_data(self.ptr),
            self.count,
            self.dofs,
        )

    def new_jerks(self) -> List[List[float]]:
        return _unflatten_points(
            _library().ruckig_tracking_output_sequence_new_jerk_const_data(self.ptr),
            self.count,
            self.dofs,
        )

    def times(self) -> List[float]:
        return _copy_out(_library().ruckig_tracking_output_sequence_time_const_data(self.ptr), self.count)

    def sections(self) -> List[int]:
        ptr = _library().ruckig_tracking_output_sequence_section_const_data(self.ptr)
        return [int(ptr[index]) for index in range(self.count)]

    def results(self) -> List[Result]:
        ptr = _library().ruckig_tracking_output_sequence_result_const_data(self.ptr)
        return [Result(int(ptr[index])) for index in range(self.count)]


class Tracking(_Handle):
    _destroy_name = "ruckig_tracking_destroy"

    def __init__(self, dofs: int, delta_time: float):
        out = ffi.new("ruckig_tracking_t**")
        _result(
            _library().ruckig_tracking_create(out, dofs, float(delta_time)),
            "ruckig_tracking_create",
        )
        super().__init__(out[0], dofs)

    @property
    def delta_time(self) -> float:
        return float(_library().ruckig_tracking_get_delta_time(self.ptr))

    @property
    def mode(self) -> TrackingMode:
        return TrackingMode(int(_library().ruckig_tracking_get_mode(self.ptr)))

    def set_mode(self, mode: TrackingMode) -> None:
        _result(_library().ruckig_tracking_set_mode(self.ptr, int(mode)), "ruckig_tracking_set_mode")

    @property
    def reactiveness(self) -> float:
        return float(_library().ruckig_tracking_get_reactiveness(self.ptr))

    def set_reactiveness(self, reactiveness: float) -> None:
        _result(
            _library().ruckig_tracking_set_reactiveness(self.ptr, float(reactiveness)),
            "ruckig_tracking_set_reactiveness",
        )

    @property
    def look_ahead_cycles(self) -> int:
        return int(_library().ruckig_tracking_get_look_ahead_cycles(self.ptr))

    def set_look_ahead_cycles(self, look_ahead_cycles: int) -> None:
        _result(
            _library().ruckig_tracking_set_look_ahead_cycles(self.ptr, int(look_ahead_cycles)),
            "ruckig_tracking_set_look_ahead_cycles",
        )

    @property
    def max_optimized_candidates(self) -> int:
        return int(_library().ruckig_tracking_get_max_optimized_candidates(self.ptr))

    def set_max_optimized_candidates(self, max_candidates: int) -> None:
        _result(
            _library().ruckig_tracking_set_max_optimized_candidates(self.ptr, int(max_candidates)),
            "ruckig_tracking_set_max_optimized_candidates",
        )

    @property
    def last_calculation_status(self) -> TrackingCalculationStatus:
        return TrackingCalculationStatus(int(_library().ruckig_tracking_get_last_calculation_status(self.ptr)))

    @property
    def last_candidate_count(self) -> int:
        return int(_library().ruckig_tracking_get_last_candidate_count(self.ptr))

    def update(self, target_state: TargetState, input_: "Input", output: "Output") -> Result:
        return _result(
            _library().ruckig_tracking_update(self.ptr, target_state.ptr, input_.ptr, output.ptr),
            "ruckig_tracking_update",
        )

    def update_with_lookahead(
        self,
        target_sequence: TargetStateSequence,
        input_: "Input",
        output: "Output",
    ) -> Result:
        return _result(
            _library().ruckig_tracking_update_with_lookahead(
                self.ptr,
                target_sequence.ptr,
                input_.ptr,
                output.ptr,
            ),
            "ruckig_tracking_update_with_lookahead",
        )

    def calculate_sequence(
        self,
        target_sequence: TargetStateSequence,
        input_: "Input",
        output_sequence: TrackingOutputSequence,
    ) -> Result:
        return _result(
            _library().ruckig_tracking_calculate_sequence(
                self.ptr,
                target_sequence.ptr,
                input_.ptr,
                output_sequence.ptr,
            ),
            "ruckig_tracking_calculate_sequence",
        )


class Input(_Handle):
    _destroy_name = "ruckig_input_destroy"

    def __init__(self, dofs: int, max_number_of_waypoints: int = 0):
        out = ffi.new("ruckig_input_t**")
        if max_number_of_waypoints:
            _result(
                _library().ruckig_input_create_with_waypoints(
                    out,
                    dofs,
                    int(max_number_of_waypoints),
                ),
                "ruckig_input_create_with_waypoints",
            )
        else:
            _result(_library().ruckig_input_create(out, dofs), "ruckig_input_create")
        super().__init__(out[0], dofs)

    def _set_vector(self, accessor: str, values: Iterable[float]) -> None:
        _copy_in(getattr(_library(), accessor)(self.ptr), values, self.dofs)

    def set_current_position(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_current_position_data", values)

    def set_current_velocity(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_current_velocity_data", values)

    def set_current_acceleration(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_current_acceleration_data", values)

    def set_target_position(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_target_position_data", values)

    def set_target_velocity(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_target_velocity_data", values)

    def set_target_acceleration(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_target_acceleration_data", values)

    def set_max_velocity(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_max_velocity_data", values)

    def set_max_acceleration(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_max_acceleration_data", values)

    def set_max_jerk(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_max_jerk_data", values)

    def set_max_position(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_max_position_data", values)

    def set_min_position(self, values: Iterable[float]) -> None:
        self._set_vector("ruckig_input_min_position_data", values)

    def set_control_interface(self, value: ControlInterface) -> None:
        _result(
            _library().ruckig_input_set_control_interface(self.ptr, int(value)),
            "ruckig_input_set_control_interface",
        )

    def set_synchronization(self, value: Synchronization) -> None:
        _result(
            _library().ruckig_input_set_synchronization(self.ptr, int(value)),
            "ruckig_input_set_synchronization",
        )

    def set_duration_discretization(self, value: DurationDiscretization) -> None:
        _result(
            _library().ruckig_input_set_duration_discretization(self.ptr, int(value)),
            "ruckig_input_set_duration_discretization",
        )

    def set_min_velocity(self, values: Iterable[float]) -> None:
        items, c_values = _copy_in_array(values)
        if len(items) != self.dofs:
            raise ValueError(f"expected {self.dofs} values, got {len(items)}")
        _result(
            _library().ruckig_input_set_min_velocity(self.ptr, c_values, len(items)),
            "ruckig_input_set_min_velocity",
        )

    def clear_min_velocity(self) -> None:
        _library().ruckig_input_clear_min_velocity(self.ptr)

    def set_min_acceleration(self, values: Iterable[float]) -> None:
        items, c_values = _copy_in_array(values)
        if len(items) != self.dofs:
            raise ValueError(f"expected {self.dofs} values, got {len(items)}")
        _result(
            _library().ruckig_input_set_min_acceleration(self.ptr, c_values, len(items)),
            "ruckig_input_set_min_acceleration",
        )

    def clear_min_acceleration(self) -> None:
        _library().ruckig_input_clear_min_acceleration(self.ptr)

    def set_minimum_duration(self, value: float) -> None:
        _result(
            _library().ruckig_input_set_minimum_duration(self.ptr, float(value)),
            "ruckig_input_set_minimum_duration",
        )

    def clear_minimum_duration(self) -> None:
        _library().ruckig_input_clear_minimum_duration(self.ptr)

    def set_intermediate_positions(self, points: Iterable[Sequence[float] | float]) -> None:
        flat, waypoint_count = _flatten_points(points, self.dofs, "intermediate_positions")
        _, c_values = _copy_in_array(flat)
        _result(
            _library().ruckig_input_set_intermediate_positions(
                self.ptr,
                c_values,
                waypoint_count,
                self.dofs,
            ),
            "ruckig_input_set_intermediate_positions",
        )

    def clear_intermediate_positions(self) -> None:
        _library().ruckig_input_clear_intermediate_positions(self.ptr)

    @property
    def intermediate_position_count(self) -> int:
        return int(_library().ruckig_input_get_intermediate_position_count(self.ptr))

    def intermediate_positions(self) -> List[List[float]]:
        count = self.intermediate_position_count
        values = ffi.new("double[]", count * self.dofs)
        _result(
            _library().ruckig_input_get_intermediate_positions(
                self.ptr,
                values,
                count * self.dofs,
            ),
            "ruckig_input_get_intermediate_positions",
        )
        return _unflatten_points(values, count, self.dofs)

    def _set_per_section_vector(self, suffix: str, values: Iterable[Sequence[float] | float]) -> None:
        flat, section_count = _flatten_points(values, self.dofs, f"per_section_{suffix}")
        _, c_values = _copy_in_array(flat)
        _result(
            getattr(_library(), f"ruckig_input_set_per_section_{suffix}")(
                self.ptr,
                c_values,
                section_count,
                self.dofs,
            ),
            f"ruckig_input_set_per_section_{suffix}",
        )

    def _get_per_section_vector(self, suffix: str) -> List[List[float]]:
        count = self.intermediate_position_count + 1
        values = ffi.new("double[]", count * self.dofs)
        _result(
            getattr(_library(), f"ruckig_input_get_per_section_{suffix}")(
                self.ptr,
                values,
                count * self.dofs,
            ),
            f"ruckig_input_get_per_section_{suffix}",
        )
        return _unflatten_points(values, count, self.dofs)

    def _clear_per_section_vector(self, suffix: str) -> None:
        getattr(_library(), f"ruckig_input_clear_per_section_{suffix}")(self.ptr)

    def set_per_section_max_velocity(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("max_velocity", values)

    def clear_per_section_max_velocity(self) -> None:
        self._clear_per_section_vector("max_velocity")

    def per_section_max_velocity(self) -> List[List[float]]:
        return self._get_per_section_vector("max_velocity")

    def set_per_section_min_velocity(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("min_velocity", values)

    def clear_per_section_min_velocity(self) -> None:
        self._clear_per_section_vector("min_velocity")

    def set_per_section_max_acceleration(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("max_acceleration", values)

    def clear_per_section_max_acceleration(self) -> None:
        self._clear_per_section_vector("max_acceleration")

    def set_per_section_min_acceleration(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("min_acceleration", values)

    def clear_per_section_min_acceleration(self) -> None:
        self._clear_per_section_vector("min_acceleration")

    def set_per_section_max_jerk(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("max_jerk", values)

    def clear_per_section_max_jerk(self) -> None:
        self._clear_per_section_vector("max_jerk")

    def set_per_section_max_position(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("max_position", values)

    def clear_per_section_max_position(self) -> None:
        self._clear_per_section_vector("max_position")

    def set_per_section_min_position(self, values: Iterable[Sequence[float] | float]) -> None:
        self._set_per_section_vector("min_position", values)

    def clear_per_section_min_position(self) -> None:
        self._clear_per_section_vector("min_position")

    def set_per_section_minimum_duration(self, values: Iterable[float]) -> None:
        items, c_values = _copy_in_array(values)
        _result(
            _library().ruckig_input_set_per_section_minimum_duration(
                self.ptr,
                c_values,
                len(items),
            ),
            "ruckig_input_set_per_section_minimum_duration",
        )

    def clear_per_section_minimum_duration(self) -> None:
        _library().ruckig_input_clear_per_section_minimum_duration(self.ptr)

    def per_section_minimum_duration(self) -> List[float]:
        count = self.intermediate_position_count + 1
        values = ffi.new("double[]", count)
        _result(
            _library().ruckig_input_get_per_section_minimum_duration(
                self.ptr,
                values,
                count,
            ),
            "ruckig_input_get_per_section_minimum_duration",
        )
        return _copy_out(values, count)

    def set_interrupt_calculation_duration(self, value: float) -> None:
        _result(
            _library().ruckig_input_set_interrupt_calculation_duration(self.ptr, float(value)),
            "ruckig_input_set_interrupt_calculation_duration",
        )

    def clear_interrupt_calculation_duration(self) -> None:
        _library().ruckig_input_clear_interrupt_calculation_duration(self.ptr)


class Output(_Handle):
    _destroy_name = "ruckig_output_destroy"

    def __init__(self, dofs: int, max_number_of_waypoints: int = 0):
        out = ffi.new("ruckig_output_t**")
        if max_number_of_waypoints:
            _result(
                _library().ruckig_output_create_with_waypoints(
                    out,
                    dofs,
                    int(max_number_of_waypoints),
                ),
                "ruckig_output_create_with_waypoints",
            )
        else:
            _result(_library().ruckig_output_create(out, dofs), "ruckig_output_create")
        super().__init__(out[0], dofs)

    @property
    def time(self) -> float:
        return float(_library().ruckig_output_get_time(self.ptr))

    @property
    def new_section(self) -> int:
        return int(_library().ruckig_output_get_new_section(self.ptr))

    @property
    def did_section_change(self) -> bool:
        return bool(_library().ruckig_output_did_section_change(self.ptr))

    @property
    def new_calculation(self) -> bool:
        return bool(_library().ruckig_output_new_calculation(self.ptr))

    @property
    def was_calculation_interrupted(self) -> bool:
        return bool(_library().ruckig_output_was_calculation_interrupted(self.ptr))

    def new_position(self) -> List[float]:
        return _copy_out(_library().ruckig_output_new_position_data(self.ptr), self.dofs)

    def new_velocity(self) -> List[float]:
        return _copy_out(_library().ruckig_output_new_velocity_data(self.ptr), self.dofs)

    def new_acceleration(self) -> List[float]:
        return _copy_out(_library().ruckig_output_new_acceleration_data(self.ptr), self.dofs)

    def pass_to_input(self, input_: Input) -> None:
        _library().ruckig_output_pass_to_input(self.ptr, input_.ptr)


class Trajectory(_Handle):
    _destroy_name = "ruckig_trajectory_destroy"

    def __init__(self, dofs: int, max_number_of_waypoints: int = 0):
        out = ffi.new("ruckig_trajectory_t**")
        if max_number_of_waypoints:
            _result(
                _library().ruckig_trajectory_create_with_waypoints(
                    out,
                    dofs,
                    int(max_number_of_waypoints),
                ),
                "ruckig_trajectory_create_with_waypoints",
            )
        else:
            _result(_library().ruckig_trajectory_create(out, dofs), "ruckig_trajectory_create")
        super().__init__(out[0], dofs)

    @property
    def duration(self) -> float:
        return float(_library().ruckig_trajectory_get_duration(self.ptr))

    @property
    def section_count(self) -> int:
        return int(_library().ruckig_trajectory_get_section_count(self.ptr))

    def intermediate_durations(self) -> List[float]:
        count = int(_library().ruckig_trajectory_get_intermediate_duration_count(self.ptr))
        values = ffi.new("double[]", count)
        _result(
            _library().ruckig_trajectory_get_intermediate_durations(
                self.ptr,
                values,
                count,
            ),
            "ruckig_trajectory_get_intermediate_durations",
        )
        return _copy_out(values, count)

    def at_time(self, time: float):
        position = ffi.new("double[]", self.dofs)
        velocity = ffi.new("double[]", self.dofs)
        acceleration = ffi.new("double[]", self.dofs)
        jerk = ffi.new("double[]", self.dofs)
        section = ffi.new("size_t*")
        _result(
            _library().ruckig_trajectory_at_time(
                self.ptr,
                float(time),
                position,
                velocity,
                acceleration,
                jerk,
                section,
            ),
            "ruckig_trajectory_at_time",
        )
        return {
            "position": _copy_out(position, self.dofs),
            "velocity": _copy_out(velocity, self.dofs),
            "acceleration": _copy_out(acceleration, self.dofs),
            "jerk": _copy_out(jerk, self.dofs),
            "section": int(section[0]),
        }

    def position_extrema(self) -> List[Bound]:
        extrema = ffi.new("ruckig_position_extrema_t[]", self.dofs)
        _result(
            _library().ruckig_trajectory_get_position_extrema(
                self.ptr,
                extrema,
                self.dofs,
            ),
            "ruckig_trajectory_get_position_extrema",
        )
        return [
            Bound(
                min=float(extrema[index].min_position),
                max=float(extrema[index].max_position),
                t_min=float(extrema[index].time_min),
                t_max=float(extrema[index].time_max),
            )
            for index in range(self.dofs)
        ]

    def first_time_at_position(
        self,
        dof: int,
        position: float,
        time_after: float = 0.0,
    ) -> Optional[float]:
        time = ffi.new("double*")
        found = ffi.new("bool*")
        _result(
            _library().ruckig_trajectory_get_first_time_at_position(
                self.ptr,
                int(dof),
                float(position),
                float(time_after),
                time,
                found,
            ),
            "ruckig_trajectory_get_first_time_at_position",
        )
        return float(time[0]) if bool(found[0]) else None
