from __future__ import annotations

import enum
import os
import sys
from pathlib import Path
from typing import Iterable, List, Optional

from cffi import FFI


ffi = FFI()
ffi.cdef(
    """
    typedef struct ruckig ruckig_t;
    typedef struct ruckig_input ruckig_input_t;
    typedef struct ruckig_output ruckig_output_t;
    typedef struct ruckig_trajectory ruckig_trajectory_t;
    typedef int ruckig_result_t;

    ruckig_result_t ruckig_create(ruckig_t** otg, size_t dofs, double delta_time);
    void ruckig_destroy(ruckig_t* otg);
    ruckig_result_t ruckig_input_create(ruckig_input_t** input, size_t dofs);
    void ruckig_input_destroy(ruckig_input_t* input);
    ruckig_result_t ruckig_output_create(ruckig_output_t** output, size_t dofs);
    void ruckig_output_destroy(ruckig_output_t* output);
    ruckig_result_t ruckig_trajectory_create(ruckig_trajectory_t** trajectory, size_t dofs);
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

    const double* ruckig_output_new_position_data(const ruckig_output_t* output);
    const double* ruckig_output_new_velocity_data(const ruckig_output_t* output);
    const double* ruckig_output_new_acceleration_data(const ruckig_output_t* output);
    double ruckig_output_get_time(const ruckig_output_t* output);

    double ruckig_trajectory_get_duration(const ruckig_trajectory_t* trajectory);
    ruckig_result_t ruckig_trajectory_at_time(
        const ruckig_trajectory_t* trajectory,
        double time,
        double* position,
        double* velocity,
        double* acceleration,
        double* jerk,
        size_t* section
    );
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

    def __init__(self, dofs: int, delta_time: float):
        out = ffi.new("ruckig_t**")
        _result(_library().ruckig_create(out, dofs, float(delta_time)), "ruckig_create")
        super().__init__(out[0], dofs)

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


class Input(_Handle):
    _destroy_name = "ruckig_input_destroy"

    def __init__(self, dofs: int):
        out = ffi.new("ruckig_input_t**")
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


class Output(_Handle):
    _destroy_name = "ruckig_output_destroy"

    def __init__(self, dofs: int):
        out = ffi.new("ruckig_output_t**")
        _result(_library().ruckig_output_create(out, dofs), "ruckig_output_create")
        super().__init__(out[0], dofs)

    @property
    def time(self) -> float:
        return float(_library().ruckig_output_get_time(self.ptr))

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

    def __init__(self, dofs: int):
        out = ffi.new("ruckig_trajectory_t**")
        _result(_library().ruckig_trajectory_create(out, dofs), "ruckig_trajectory_create")
        super().__init__(out[0], dofs)

    @property
    def duration(self) -> float:
        return float(_library().ruckig_trajectory_get_duration(self.ptr))

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
