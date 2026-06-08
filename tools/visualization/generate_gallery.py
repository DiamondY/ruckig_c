from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, Sequence

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np


REPO_ROOT = Path(__file__).resolve().parents[2]
PYTHON_PROTOTYPE = REPO_ROOT / "bindings" / "python_prototype"
if str(PYTHON_PROTOTYPE) not in sys.path:
    sys.path.insert(0, str(PYTHON_PROTOTYPE))

from ruckig_cffi import (  # noqa: E402
    ControlInterface,
    Input,
    Output,
    Result,
    Ruckig,
    Synchronization,
    TargetState,
    TargetStateSequence,
    Tracking,
    TrackingMode,
    TrackingOptimizedStrategy,
    TrackingOutputSequence,
    Trajectory,
    configure_library,
)


FIGSIZE = (11.0, 7.2)
DPI = 100
SAVE_METADATA = {"Software": "ruckig_c matplotlib visualization"}


@dataclass(frozen=True)
class MotionSamples:
    time: np.ndarray
    position: np.ndarray
    velocity: np.ndarray
    acceleration: np.ndarray
    section: np.ndarray | None = None


@dataclass(frozen=True)
class GalleryImage:
    file_name: str
    title: str
    description: str
    original_examples: tuple[str, ...]
    metrics: dict[str, float | int | str | bool]


def find_shared_library(argument: str | None) -> Path | None:
    if argument:
        return Path(argument).resolve()
    env_path = os.environ.get("RUCKIG_C_SHARED_LIBRARY")
    if env_path:
        return Path(env_path).resolve()

    candidates: list[Path]
    if sys.platform.startswith("win"):
        candidates = [
            REPO_ROOT / "out" / "build" / "windows-clang-ninja-shared" / "ruckig_c.dll",
            REPO_ROOT / "out" / "build" / "windows-clang-ninja" / "ruckig_c.dll",
        ]
    elif sys.platform == "darwin":
        candidates = [
            REPO_ROOT / "out" / "build" / "dev-shared" / "libruckig_c.dylib",
            REPO_ROOT / "out" / "build" / "dev" / "libruckig_c.dylib",
        ]
    else:
        candidates = [
            REPO_ROOT / "out" / "build" / "dev-shared" / "libruckig_c.so",
            REPO_ROOT / "out" / "build" / "dev" / "libruckig_c.so",
        ]
    for candidate in candidates:
        if candidate.exists():
            return candidate.resolve()
    return None


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def round_metric(value: float) -> float:
    return round(float(value), 9)


def close_handles(*handles) -> None:
    for handle in reversed(handles):
        handle.close()


def configure_original_position_input(input_: Input) -> None:
    input_.set_current_position([0.0, 0.0, 0.5])
    input_.set_current_velocity([0.0, -2.2, -0.5])
    input_.set_current_acceleration([0.0, 2.5, -0.5])
    input_.set_target_position([5.0, -2.0, -3.5])
    input_.set_target_velocity([0.0, -0.5, -2.0])
    input_.set_target_acceleration([0.0, 0.0, 0.5])
    input_.set_max_velocity([3.0, 1.0, 3.0])
    input_.set_max_acceleration([3.0, 2.0, 1.0])
    input_.set_max_jerk([4.0, 3.0, 2.0])


def configure_original_waypoint_input(input_: Input) -> None:
    input_.set_current_position([0.2, 0.0, -0.3])
    input_.set_current_velocity([0.0, 0.2, 0.0])
    input_.set_current_acceleration([0.0, 0.6, 0.0])
    input_.set_intermediate_positions(
        [
            [1.4, -1.6, 1.0],
            [-0.6, -0.5, 0.4],
            [-0.4, -0.35, 0.0],
            [0.8, 1.8, -0.1],
        ]
    )
    input_.set_target_position([0.5, 1.0, 0.0])
    input_.set_target_velocity([0.2, 0.0, 0.3])
    input_.set_target_acceleration([0.0, 0.1, -0.1])
    input_.set_max_velocity([1.0, 2.0, 1.0])
    input_.set_max_acceleration([3.0, 2.0, 2.0])
    input_.set_max_jerk([6.0, 10.0, 20.0])


def sample_trajectory(trajectory: Trajectory, samples: int) -> MotionSamples:
    times = np.linspace(0.0, trajectory.duration, samples)
    position: list[list[float]] = []
    velocity: list[list[float]] = []
    acceleration: list[list[float]] = []
    section: list[int] = []
    for time in times:
        sample = trajectory.at_time(float(time))
        position.append(sample["position"])
        velocity.append(sample["velocity"])
        acceleration.append(sample["acceleration"])
        section.append(int(sample["section"]))
    return MotionSamples(
        time=times,
        position=np.asarray(position, dtype=float),
        velocity=np.asarray(velocity, dtype=float),
        acceleration=np.asarray(acceleration, dtype=float),
        section=np.asarray(section, dtype=int),
    )


def calculate_trajectory(
    dofs: int,
    configure: Callable[[Input], None],
    *,
    samples: int,
    max_waypoints: int = 0,
) -> tuple[MotionSamples, float, list[float]]:
    otg = Ruckig(dofs, 0.01, max_number_of_waypoints=max_waypoints)
    input_ = Input(dofs, max_number_of_waypoints=max_waypoints)
    trajectory = Trajectory(dofs, max_number_of_waypoints=max_waypoints)
    try:
        configure(input_)
        result = otg.calculate(input_, trajectory)
        if result != Result.WORKING:
            raise RuntimeError(f"ruckig_calculate returned {result}")
        section_times = trajectory.intermediate_durations()
        return sample_trajectory(trajectory, samples), trajectory.duration, section_times
    finally:
        close_handles(trajectory, input_, otg)


def run_online(
    dofs: int,
    configure: Callable[[Input], None],
    *,
    max_waypoints: int = 0,
    max_steps: int = 5000,
    delta_time: float = 0.01,
    after_step: Callable[[int, float, Input, Output], None] | None = None,
) -> tuple[MotionSamples, Result, list[int]]:
    otg = Ruckig(dofs, delta_time, max_number_of_waypoints=max_waypoints)
    input_ = Input(dofs, max_number_of_waypoints=max_waypoints)
    output = Output(dofs, max_number_of_waypoints=max_waypoints)
    try:
        configure(input_)
        time: list[float] = []
        position: list[list[float]] = []
        velocity: list[list[float]] = []
        acceleration: list[list[float]] = []
        sections: list[int] = []
        section_changes: list[int] = []
        result = Result.WORKING
        for step in range(max_steps):
            result = otg.update(input_, output)
            loop_time = step * delta_time
            time.append(loop_time)
            position.append(output.new_position())
            velocity.append(output.new_velocity())
            acceleration.append(output.new_acceleration())
            sections.append(output.new_section)
            if output.did_section_change:
                section_changes.append(step)
            if after_step is not None:
                after_step(step, loop_time, input_, output)
            output.pass_to_input(input_)
            if result == Result.FINISHED:
                break
        else:
            raise RuntimeError("ruckig_update did not finish within max_steps")

        return (
            MotionSamples(
                time=np.asarray(time, dtype=float),
                position=np.asarray(position, dtype=float),
                velocity=np.asarray(velocity, dtype=float),
                acceleration=np.asarray(acceleration, dtype=float),
                section=np.asarray(sections, dtype=int),
            ),
            result,
            section_changes,
        )
    finally:
        close_handles(output, input_, otg)


def style_axis(ax, title: str, ylabel: str) -> None:
    ax.set_title(title, loc="left", fontsize=11, fontweight="bold")
    ax.set_ylabel(ylabel)
    ax.grid(True, color="#e3e7ed", linewidth=0.8)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)


def plot_motion(
    path: Path,
    *,
    title: str,
    subtitle: str,
    samples: MotionSamples,
    markers: Sequence[tuple[float, str]] = (),
    phase_spans: Sequence[tuple[float, float, str, str]] = (),
    note: str | None = None,
) -> None:
    dofs = samples.position.shape[1]
    fig, axes = plt.subplots(3, 1, figsize=FIGSIZE, dpi=DPI, sharex=True)
    fig.suptitle(title, x=0.06, y=0.975, ha="left", fontsize=17, fontweight="bold")
    fig.text(0.06, 0.935, subtitle, ha="left", va="top", fontsize=10, color="#52606d")

    for ax in axes:
        for start, end, label, color in phase_spans:
            ax.axvspan(start, end, color=color, alpha=0.12, lw=0)
        for marker_time, marker_label in markers:
            ax.axvline(marker_time, color="#8d5fb8", linestyle="--", linewidth=1.1)
            ax.text(marker_time, 0.97, marker_label, rotation=90, transform=ax.get_xaxis_transform(),
                    ha="right", va="top", fontsize=8, color="#69468a")

    labels = [f"DoF {index}" for index in range(dofs)]
    for index, label in enumerate(labels):
        axes[0].plot(samples.time, samples.position[:, index], label=label, linewidth=2.0)
        axes[1].plot(samples.time, samples.velocity[:, index], label=label, linewidth=1.8)
        axes[2].plot(samples.time, samples.acceleration[:, index], label=label, linewidth=1.8)

    style_axis(axes[0], "Position", "position")
    style_axis(axes[1], "Velocity", "velocity")
    style_axis(axes[2], "Acceleration", "acceleration")
    axes[2].set_xlabel("time [s]")
    axes[0].legend(ncol=min(dofs, 4), loc="upper right", frameon=False)

    if phase_spans:
        handles = [
            plt.Line2D([0], [0], color=color, lw=6, alpha=0.4, label=label)
            for _, _, label, color in phase_spans
        ]
        axes[1].legend(handles=handles, loc="upper right", frameon=False)

    if note:
        fig.text(0.98, 0.935, note, ha="right", va="top", fontsize=9, color="#52606d")

    fig.tight_layout(rect=(0.04, 0.04, 0.98, 0.91))
    fig.savefig(path, dpi=DPI, metadata=SAVE_METADATA)
    plt.close(fig)


def plot_tracking(
    path: Path,
    *,
    title: str,
    subtitle: str,
    time: np.ndarray,
    target: np.ndarray,
    output: np.ndarray,
    velocity: np.ndarray | None = None,
    note: str | None = None,
) -> None:
    rows = 2 if velocity is not None else 1
    fig, axes_obj = plt.subplots(rows, 1, figsize=FIGSIZE, dpi=DPI, sharex=True)
    axes = np.atleast_1d(axes_obj)
    fig.suptitle(title, x=0.06, y=0.975, ha="left", fontsize=17, fontweight="bold")
    fig.text(0.06, 0.935, subtitle, ha="left", va="top", fontsize=10, color="#52606d")

    axes[0].plot(time, target, label="target", color="#333c47", linewidth=2.0)
    axes[0].plot(time, output, label="ruckig_c output", color="#176fb5", linewidth=2.0)
    style_axis(axes[0], "Position tracking", "position")
    axes[0].legend(loc="upper right", frameon=False)

    if velocity is not None:
        axes[1].plot(time, velocity, label="output velocity", color="#20916c", linewidth=2.0)
        style_axis(axes[1], "Output velocity", "velocity")
        axes[1].legend(loc="upper right", frameon=False)

    axes[-1].set_xlabel("time [s]")
    if note:
        fig.text(0.98, 0.935, note, ha="right", va="top", fontsize=9, color="#52606d")

    fig.tight_layout(rect=(0.04, 0.04, 0.98, 0.91))
    fig.savefig(path, dpi=DPI, metadata=SAVE_METADATA)
    plt.close(fig)


def position_extent(samples: MotionSamples) -> float:
    return float(np.max(samples.position) - np.min(samples.position))


def save_motion_image(
    output: Path,
    file_name: str,
    title: str,
    subtitle: str,
    original_examples: Sequence[str],
    samples: MotionSamples,
    *,
    duration: float,
    markers: Sequence[tuple[float, str]] = (),
    phase_spans: Sequence[tuple[float, float, str, str]] = (),
    extra_metrics: dict[str, float | int | str | bool] | None = None,
) -> GalleryImage:
    path = output / file_name
    plot_motion(
        path,
        title=title,
        subtitle=subtitle,
        samples=samples,
        markers=markers,
        phase_spans=phase_spans,
        note=f"duration {duration:.3f} s",
    )
    metrics: dict[str, float | int | str | bool] = {
        "duration": round_metric(duration),
        "samples": int(samples.time.size),
        "dofs": int(samples.position.shape[1]),
        "position_extent": round_metric(position_extent(samples)),
    }
    if extra_metrics:
        metrics.update(extra_metrics)
    return GalleryImage(
        file_name=file_name,
        title=title,
        description=subtitle,
        original_examples=tuple(original_examples),
        metrics=metrics,
    )


def render_01_position(output: Path, samples: int) -> GalleryImage:
    motion, result, _ = run_online(3, configure_original_position_input)
    return save_motion_image(
        output,
        "01_position.png",
        "01 Position",
        "Online 3-DoF position trajectory equivalent to original example 01.",
        ("01_position",),
        motion,
        duration=float(motion.time[-1]),
        extra_metrics={"online_result": result.name, "sampled_online": True},
    )


def render_02_position_offline(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        configure_original_position_input(input_)
        input_.set_min_velocity([-2.0, -0.5, -3.0])
        input_.set_min_acceleration([-2.0, -2.0, -2.0])

    motion, duration, _ = calculate_trajectory(3, configure, samples=samples)
    return save_motion_image(
        output,
        "02_position_offline.png",
        "02 Position Offline",
        "Offline 3-DoF trajectory with directional min velocity and acceleration limits.",
        ("02_position_offline",),
        motion,
        duration=duration,
        extra_metrics={"directional_limits": True},
    )


def render_03_waypoints(output: Path, samples: int) -> GalleryImage:
    motion, duration, section_times = calculate_trajectory(
        3,
        configure_original_waypoint_input,
        samples=samples,
        max_waypoints=10,
    )
    return save_motion_image(
        output,
        "03_waypoints_local.png",
        "03 Local Waypoints",
        "Local C ABI waypoint trajectory equivalent for original Pro/cloud waypoint example 03.",
        ("03_waypoints",),
        motion,
        duration=duration,
        markers=[(time, f"wp {index + 1}") for index, time in enumerate(section_times)],
        extra_metrics={"waypoint_count": 4, "pro_cloud_equivalence_claim": False},
    )


def render_04_waypoints_online(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        configure_original_waypoint_input(input_)
        input_.set_interrupt_calculation_duration(500.0)

    motion, result, changes = run_online(3, configure, max_waypoints=10)
    return save_motion_image(
        output,
        "04_waypoints_online_local.png",
        "04 Local Waypoints Online",
        "Online local waypoint update path equivalent for original waypoint online example 04.",
        ("04_waypoints_online",),
        motion,
        duration=float(motion.time[-1]),
        markers=[(float(index) * 0.01, "section") for index in changes[:6]],
        extra_metrics={
            "online_result": result.name,
            "section_change_count": len(changes),
            "interrupt_duration_storage_only": True,
            "pro_cloud_equivalence_claim": False,
        },
    )


def render_05_velocity(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        input_.set_control_interface(ControlInterface.VELOCITY)
        input_.set_current_position([0.0, 0.0, 0.5])
        input_.set_current_velocity([3.0, -2.2, -0.5])
        input_.set_current_acceleration([0.0, 2.5, -0.5])
        input_.set_target_velocity([0.0, -0.5, -1.5])
        input_.set_target_acceleration([0.0, 0.0, 0.5])
        input_.set_max_acceleration([3.0, 2.0, 1.0])
        input_.set_max_jerk([6.0, 6.0, 4.0])

    motion, result, _ = run_online(3, configure)
    return save_motion_image(
        output,
        "05_velocity.png",
        "05 Velocity Control",
        "Online 3-DoF velocity-control trajectory equivalent to original example 05.",
        ("05_velocity",),
        motion,
        duration=float(motion.time[-1]),
        extra_metrics={"online_result": result.name, "control_interface": "velocity"},
    )


def render_06_stop(output: Path, samples: int) -> GalleryImage:
    state = {"stop_step": -1}

    def after_step(step: int, loop_time: float, input_: Input, output: Output) -> None:
        if loop_time >= 1.0 and state["stop_step"] < 0:
            state["stop_step"] = step
            input_.set_control_interface(ControlInterface.VELOCITY)
            input_.set_synchronization(Synchronization.NONE)
            input_.set_target_velocity([0.0, 0.0, 0.0])
            input_.set_target_acceleration([0.0, 0.0, 0.0])
            input_.set_max_jerk([12.0, 10.0, 8.0])

    motion, result, _ = run_online(3, configure_original_position_input, after_step=after_step)
    stop_time = state["stop_step"] * 0.01 if state["stop_step"] >= 0 else 0.0
    return save_motion_image(
        output,
        "06_stop.png",
        "06 Stop",
        "Online stop trajectory equivalent with a velocity-mode stop command after 1 second.",
        ("06_stop",),
        motion,
        duration=float(motion.time[-1]),
        markers=[(stop_time, "stop command")],
        extra_metrics={"online_result": result.name, "stop_time": round_metric(stop_time)},
    )


def render_07_minimum_duration(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        configure_original_position_input(input_)
        input_.set_target_position([-5.0, -2.0, -3.5])
        input_.set_minimum_duration(5.0)

    motion, duration, _ = calculate_trajectory(3, configure, samples=samples)
    return save_motion_image(
        output,
        "07_minimum_duration.png",
        "07 Minimum Duration",
        "Trajectory constrained by a 5-second minimum duration.",
        ("07_minimum_duration",),
        motion,
        duration=duration,
        markers=[(5.0, "minimum")],
        extra_metrics={"minimum_duration": 5.0},
    )


def render_08_per_section_minimum_duration(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        input_.set_current_position([0.8, 0.0, 0.5])
        input_.set_current_velocity([0.0, 0.0, 0.0])
        input_.set_current_acceleration([0.0, 0.0, 0.0])
        input_.set_intermediate_positions(
            [
                [1.4, -1.6, 1.0],
                [-0.6, -0.5, 0.4],
                [-0.4, -0.35, 0.0],
                [-0.2, 0.35, -0.1],
                [0.2, 0.5, -0.1],
                [0.8, 1.8, -0.1],
            ]
        )
        input_.set_target_position([0.5, 1.2, 0.0])
        input_.set_target_velocity([0.0, 0.0, 0.0])
        input_.set_target_acceleration([0.0, 0.0, 0.0])
        input_.set_max_velocity([3.0, 2.0, 2.0])
        input_.set_max_acceleration([6.0, 4.0, 4.0])
        input_.set_max_jerk([16.0, 10.0, 20.0])
        input_.set_per_section_minimum_duration([0.0, 2.0, 0.0, 1.0, 0.0, 2.0, 0.0])

    motion, duration, section_times = calculate_trajectory(3, configure, samples=samples, max_waypoints=10)
    return save_motion_image(
        output,
        "08_per_section_minimum_duration.png",
        "08 Per-section Minimum Duration",
        "Local waypoint trajectory with per-section minimum duration constraints.",
        ("08_per_section_minimum_duration",),
        motion,
        duration=duration,
        markers=[(time, f"wp {index + 1}") for index, time in enumerate(section_times)],
        extra_metrics={"waypoint_count": 6, "per_section_minimum_duration": True},
    )


def render_09_dynamic_dofs(output: Path, samples: int) -> GalleryImage:
    motion, result, _ = run_online(3, configure_original_position_input)
    return save_motion_image(
        output,
        "09_dynamic_dofs.png",
        "09 Dynamic DoFs",
        "C ABI dynamic-DoF handle trajectory equivalent to original DynamicDOFs example 09.",
        ("09_dynamic_dofs",),
        motion,
        duration=float(motion.time[-1]),
        extra_metrics={"online_result": result.name, "dynamic_dof_c_abi": True},
    )


def render_10_dynamic_dofs_waypoints(output: Path, samples: int) -> GalleryImage:
    motion, duration, section_times = calculate_trajectory(
        3,
        configure_original_waypoint_input,
        samples=samples,
        max_waypoints=10,
    )
    return save_motion_image(
        output,
        "10_dynamic_dofs_waypoints_local.png",
        "10 Dynamic DoFs Local Waypoints",
        "C ABI dynamic-DoF local waypoint equivalent to original DynamicDOFs waypoint example 10.",
        ("10_dynamic_dofs_waypoints",),
        motion,
        duration=duration,
        markers=[(time, f"wp {index + 1}") for index, time in enumerate(section_times)],
        extra_metrics={"dynamic_dof_c_abi": True, "waypoint_count": 4, "pro_cloud_equivalence_claim": False},
    )


def tracking_ramp_signal(time: float, ramp_velocity: float = 0.5, ramp_position: float = 1.0) -> tuple[float, float, float]:
    on_ramp = time < ramp_position / abs(ramp_velocity)
    return (
        time * ramp_velocity if on_ramp else ramp_position,
        ramp_velocity if on_ramp else 0.0,
        0.0,
    )


def tracking_half_sinus_signal(time: float, ramp_velocity: float = 1.2) -> tuple[float, float, float]:
    if time < 2.5:
        return (
            math.sin(ramp_velocity * time),
            ramp_velocity * math.cos(ramp_velocity * time),
            -ramp_velocity * ramp_velocity * math.sin(ramp_velocity * time),
        )
    return 0.0, 0.0, 0.0


def configure_tracking_input(input_: Input, *, high_limits: bool = False) -> None:
    if high_limits:
        input_.set_max_velocity([4.0])
        input_.set_max_acceleration([5.0])
        input_.set_max_jerk([15.0])
    else:
        input_.set_max_velocity([0.8])
        input_.set_max_acceleration([2.0])
        input_.set_max_jerk([5.0])
        input_.set_min_position([-2.5])
        input_.set_max_position([2.5])


def run_tracking_online(steps: int = 500, dt: float = 0.01) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, dict[str, float | int | str]]:
    with Tracking(1, dt) as tracking, TargetStateSequence(1, 8) as targets, Input(1) as input_, Output(1) as output:
        configure_tracking_input(input_)
        tracking.set_mode(TrackingMode.OPTIMIZED)
        tracking.set_optimized_strategy(TrackingOptimizedStrategy.BALANCED)
        tracking.set_look_ahead_cycles(8)
        targets.set_count(8)
        time: list[float] = []
        target_position: list[float] = []
        output_position: list[float] = []
        output_velocity: list[float] = []
        for step in range(steps):
            for sample in range(8):
                sample_time = (step + sample) * dt
                position, velocity, acceleration = tracking_ramp_signal(sample_time)
                targets.set_state(sample, [position], [velocity], [acceleration])
            current_time = step * dt
            target, _, _ = tracking_ramp_signal(current_time)
            tracking.update_with_lookahead(targets, input_, output)
            time.append(current_time)
            target_position.append(target)
            output_position.append(output.new_position()[0])
            output_velocity.append(output.new_velocity()[0])
            output.pass_to_input(input_)
        diagnostics = tracking.last_diagnostics
        return (
            np.asarray(time),
            np.asarray(target_position),
            np.asarray(output_position),
            np.asarray(output_velocity),
            {
                "status": diagnostics.calculation_status.name,
                "strategy": diagnostics.optimized_strategy.name,
                "candidate_count": diagnostics.candidate_count,
                "optimized_step_count": diagnostics.optimized_step_count,
                "fallback_step_count": diagnostics.fallback_step_count,
            },
        )


def run_tracking_offline(count: int = 400, dt: float = 0.01) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, dict[str, float | int | str]]:
    with (
        Tracking(1, dt) as tracking,
        TargetStateSequence(1, count) as targets,
        Input(1) as input_,
        TrackingOutputSequence(1, count) as outputs,
    ):
        configure_tracking_input(input_, high_limits=True)
        tracking.set_mode(TrackingMode.OPTIMIZED)
        tracking.set_optimized_strategy(TrackingOptimizedStrategy.BALANCED)
        tracking.set_look_ahead_cycles(32)
        targets.set_count(count)
        target_position: list[float] = []
        for step in range(count):
            time = step * dt
            position, velocity, acceleration = tracking_half_sinus_signal(time)
            targets.set_state(step, [position], [velocity], [acceleration])
            target_position.append(position)
        tracking.calculate_sequence(targets, input_, outputs)
        diagnostics = tracking.last_diagnostics
        return (
            np.asarray(outputs.times(), dtype=float),
            np.asarray(target_position, dtype=float),
            np.asarray([point[0] for point in outputs.new_positions()], dtype=float),
            np.asarray([point[0] for point in outputs.new_velocities()], dtype=float),
            {
                "status": diagnostics.calculation_status.name,
                "strategy": diagnostics.optimized_strategy.name,
                "candidate_count": diagnostics.candidate_count,
                "optimized_step_count": diagnostics.optimized_step_count,
                "fallback_step_count": diagnostics.fallback_step_count,
                "look_ahead_cycles": 32,
            },
        )


def mean_abs_error(target: np.ndarray, output: np.ndarray) -> float:
    return float(np.mean(np.abs(target - output)))


def render_14_tracking(output: Path, samples: int) -> GalleryImage:
    time, target, follow, velocity, diagnostics = run_tracking_online()
    path = output / "14_tracking_online_local.png"
    error = mean_abs_error(target, follow)
    plot_tracking(
        path,
        title="14 Online Tracking Local",
        subtitle="Local bounded Optimized tracking equivalent for original Pro tracking example 14.",
        time=time,
        target=target,
        output=follow,
        velocity=velocity,
        note=f"MAE {error:.5f}; candidates {diagnostics['candidate_count']}",
    )
    return GalleryImage(
        file_name=path.name,
        title="14 Online Tracking Local",
        description="Local bounded Optimized online tracking over the original ramp target model.",
        original_examples=("14_tracking",),
        metrics={"steps": int(time.size), "mean_absolute_error": round_metric(error), **diagnostics},
    )


def render_15_tracking_offline(output: Path, samples: int) -> GalleryImage:
    time, target, follow, velocity, diagnostics = run_tracking_offline()
    path = output / "15_tracking_offline_local.png"
    error = mean_abs_error(target, follow)
    plot_tracking(
        path,
        title="15 Offline Tracking Local",
        subtitle="Local bounded Optimized offline tracking equivalent for original half-sinus example 15.",
        time=time,
        target=target,
        output=follow,
        velocity=velocity,
        note=f"MAE {error:.5f}; lookahead {diagnostics['look_ahead_cycles']}",
    )
    return GalleryImage(
        file_name=path.name,
        title="15 Offline Tracking Local",
        description="Offline Optimized tracking over the original half-sinus target model.",
        original_examples=("15_tracking_offline",),
        metrics={"steps": int(time.size), "mean_absolute_error": round_metric(error), **diagnostics},
    )


def render_16_speed_brake_phases(output: Path, samples: int) -> GalleryImage:
    otg = Ruckig(3, 0.01)
    input_ = Input(3)
    trajectory = Trajectory(3)
    try:
        configure_original_position_input(input_)
        if otg.calculate(input_, trajectory) != Result.WORKING:
            raise RuntimeError("speed phase source trajectory calculation failed")
        duration = trajectory.duration
        result = render_16_speed_brake_phases_from_trajectory(output, trajectory, duration)
    finally:
        close_handles(trajectory, input_, otg)
    return result


def render_16_speed_brake_phases_from_trajectory(output: Path, trajectory: Trajectory, duration: float) -> GalleryImage:
    dt = 0.01
    loop_time: list[float] = []
    trajectory_time: list[float] = []
    position: list[list[float]] = []
    velocity: list[list[float]] = []
    acceleration: list[list[float]] = []
    phases: list[str] = []
    speed = 1.0
    traj_time = 0.0
    phase = "start"
    pause_end: float | None = None
    for step in range(2000):
        current_loop_time = step * dt
        if traj_time > 1.8 and phase == "start":
            phase = "brake"
        if phase == "brake":
            speed = max(speed - dt / 1.0, 0.0)
            if math.isclose(speed, 0.0, abs_tol=1e-12):
                phase = "paused"
                pause_end = current_loop_time + 0.5
        elif phase == "paused":
            speed = 0.0
            if pause_end is not None and current_loop_time >= pause_end:
                phase = "accelerate"
        elif phase == "accelerate":
            speed = min(speed + dt / 1.0, 1.0)
            if math.isclose(speed, 1.0, abs_tol=1e-12):
                phase = "end"
        traj_time = min(traj_time + speed * dt, duration)
        sample = trajectory.at_time(traj_time)
        loop_time.append(current_loop_time)
        trajectory_time.append(traj_time)
        position.append(sample["position"])
        velocity.append(sample["velocity"])
        acceleration.append(sample["acceleration"])
        phases.append(phase)
        if traj_time >= duration and phase == "end":
            break

    phase_spans = compute_phase_spans(np.asarray(loop_time), phases)
    local_motion = MotionSamples(
        time=np.asarray(loop_time),
        position=np.asarray(position),
        velocity=np.asarray(velocity),
        acceleration=np.asarray(acceleration),
    )
    path = output / "16_speed_brake_phases.png"
    plot_motion(
        path,
        title="16 Speed Brake Phases",
        subtitle="Local public C trajectory sampling with external start/brake/pause/accelerate/end phase markers.",
        samples=local_motion,
        phase_spans=phase_spans,
        note="local retiming; no speed-control ABI claim",
    )
    return GalleryImage(
        file_name=path.name,
        title="16 Speed Brake Phases",
        description="Local C ABI trajectory sampling visualizing the original speed-control phase concept.",
        original_examples=("16_speed",),
        metrics={
            "base_duration": round_metric(duration),
            "loop_duration": round_metric(loop_time[-1]),
            "local_external_retiming": True,
            "speed_control_abi_claim": False,
            "samples": len(loop_time),
        },
    )


def compute_phase_spans(times: np.ndarray, phases: Sequence[str]) -> list[tuple[float, float, str, str]]:
    colors = {
        "start": "#7aa6d8",
        "brake": "#d07b53",
        "paused": "#9aa3ad",
        "accelerate": "#73b88f",
        "end": "#b58bd2",
    }
    spans: list[tuple[float, float, str, str]] = []
    if not phases:
        return spans
    start_index = 0
    current = phases[0]
    for index, phase in enumerate(phases[1:], start=1):
        if phase != current:
            spans.append((float(times[start_index]), float(times[index]), current, colors[current]))
            start_index = index
            current = phase
    spans.append((float(times[start_index]), float(times[-1]), current, colors[current]))
    return spans


def write_manifest(output: Path, images: Sequence[GalleryImage]) -> None:
    manifest_images = []
    for image in images:
        path = output / image.file_name
        manifest_images.append(
            {
                "file": image.file_name,
                "title": image.title,
                "description": image.description,
                "original_examples": list(image.original_examples),
                "bytes": path.stat().st_size,
                "sha256": sha256_file(path),
                "metrics": image.metrics,
            }
        )
    manifest = {
        "label": "0.8.0-alpha.2 matplotlib visualization evidence",
        "generated_by": "tools/visualization/generate_gallery.py",
        "dependencies": {
            "renderer": "Matplotlib Agg",
            "data": "NumPy arrays sampled from public C ABI calls",
            "data_source": "bindings/python_prototype/ruckig_cffi.py",
            "shared_library": "RUCKIG_C_SHARED_LIBRARY or local shared build",
        },
        "excluded_original_examples": {
            "11_eigen_vector_type": "C++ vector ergonomics, not C ABI visualization surface",
            "12_custom_vector_type": "C++ vector ergonomics, not C ABI visualization surface",
            "13_custom_vector_type_dynamic_dofs": "C++ vector ergonomics, not C ABI visualization surface",
        },
        "images": manifest_images,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate local ruckig_c Matplotlib visualization PNG assets.")
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO_ROOT / "docs" / "assets" / "visualization",
        help="Output directory for PNG assets and manifest.json.",
    )
    parser.add_argument(
        "--library",
        type=str,
        default=None,
        help="Path to ruckig_c shared library. Defaults to RUCKIG_C_SHARED_LIBRARY or the local shared build.",
    )
    parser.add_argument("--samples", type=int, default=360, help="Trajectory samples per offline plot.")
    return parser.parse_args()


def clean_png_assets(output: Path) -> None:
    for path in output.glob("*.png"):
        path.unlink()


def main() -> int:
    args = parse_args()
    if args.samples < 32:
        raise SystemExit("--samples must be at least 32")

    library = find_shared_library(args.library)
    configure_library(library if library is None else str(library))

    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    clean_png_assets(output)

    renderers = [
        render_01_position,
        render_02_position_offline,
        render_03_waypoints,
        render_04_waypoints_online,
        render_05_velocity,
        render_06_stop,
        render_07_minimum_duration,
        render_08_per_section_minimum_duration,
        render_09_dynamic_dofs,
        render_10_dynamic_dofs_waypoints,
        render_14_tracking,
        render_15_tracking_offline,
        render_16_speed_brake_phases,
    ]
    images = [renderer(output, args.samples) for renderer in renderers]
    write_manifest(output, images)

    for image in images:
        path = output / image.file_name
        print(f"generated {path.relative_to(REPO_ROOT)} {path.stat().st_size} bytes")
    print(f"generated {(output / 'manifest.json').relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
