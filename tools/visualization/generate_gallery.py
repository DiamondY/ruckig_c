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


FIGSIZE = (14.0, 9.0)
DPI = 100
SAVE_METADATA = {"Software": "ruckig_c matplotlib visualization"}
MANIFEST_LABEL = "0.10.0-alpha visualization v2 evidence"
PALETTE = ["#176fb5", "#20916c", "#d07b53", "#8d5fb8", "#59636f", "#c43d4d", "#2b8a9f", "#b58bd2"]
WAYPOINT_GALLERY_INTERRUPT_BUDGET_US = 1000000000.0


@dataclass(frozen=True)
class MotionSamples:
    time: np.ndarray
    position: np.ndarray
    velocity: np.ndarray
    acceleration: np.ndarray
    section: np.ndarray | None = None
    jerk: np.ndarray | None = None


@dataclass(frozen=True)
class GalleryImage:
    file_name: str
    title: str
    description: str
    original_examples: tuple[str, ...]
    metrics: dict[str, float | int | str | bool]
    category: str = "original_mapping"


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
    jerk: list[list[float]] = []
    section: list[int] = []
    for time in times:
        sample = trajectory.at_time(float(time))
        position.append(sample["position"])
        velocity.append(sample["velocity"])
        acceleration.append(sample["acceleration"])
        jerk.append(sample["jerk"])
        section.append(int(sample["section"]))
    return MotionSamples(
        time=times,
        position=np.asarray(position, dtype=float),
        velocity=np.asarray(velocity, dtype=float),
        acceleration=np.asarray(acceleration, dtype=float),
        section=np.asarray(section, dtype=int),
        jerk=np.asarray(jerk, dtype=float),
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


def setup_figure(title: str, subtitle: str, *, rows: int = 1, cols: int = 1, sharex: bool = False):
    fig, axes_obj = plt.subplots(rows, cols, figsize=FIGSIZE, dpi=DPI, sharex=sharex)
    axes = np.atleast_1d(axes_obj).reshape(rows, cols)
    fig.suptitle(title, x=0.06, y=0.975, ha="left", fontsize=17, fontweight="bold")
    fig.text(0.06, 0.94, subtitle, ha="left", va="top", fontsize=10, color="#52606d")
    return fig, axes


def finish_figure(fig, path: Path) -> None:
    fig.tight_layout(rect=(0.04, 0.04, 0.98, 0.91))
    fig.savefig(path, dpi=DPI, metadata=SAVE_METADATA)
    plt.close(fig)


def save_chart_image(
    output: Path,
    file_name: str,
    title: str,
    subtitle: str,
    *,
    category: str,
    metrics: dict[str, float | int | str | bool],
    original_examples: Sequence[str] = (),
) -> GalleryImage:
    return GalleryImage(
        file_name=file_name,
        title=title,
        description=subtitle,
        original_examples=tuple(original_examples),
        metrics=metrics,
        category=category,
    )


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
        input_.set_interrupt_calculation_duration(WAYPOINT_GALLERY_INTERRUPT_BUDGET_US)

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
            "interrupt_budget_us": WAYPOINT_GALLERY_INTERRUPT_BUDGET_US,
            "soft_interruption_v1": True,
            "soft_interruption_expected": False,
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


def tracking_signal(name: str, time: float) -> tuple[float, float, float]:
    if name == "ramp":
        return tracking_ramp_signal(time)
    if name == "constant_acceleration":
        return 0.25 * time * time, 0.5 * time, 0.5
    if name == "sinus":
        return math.sin(1.2 * time), 1.2 * math.cos(1.2 * time), -1.44 * math.sin(1.2 * time)
    if name == "half_sinus":
        return tracking_half_sinus_signal(time)
    raise ValueError(f"unknown tracking signal {name}")


def run_tracking_profile(
    *,
    strategy: TrackingOptimizedStrategy,
    signal: str,
    steps: int = 360,
    dt: float = 0.01,
    lookahead: int = 8,
    reactiveness: float = 0.5,
    high_limits: bool = False,
) -> dict[str, object]:
    with Tracking(1, dt) as tracking, TargetStateSequence(1, lookahead) as targets, Input(1) as input_, Output(1) as output:
        configure_tracking_input(input_, high_limits=high_limits)
        tracking.set_mode(TrackingMode.OPTIMIZED)
        tracking.set_optimized_strategy(strategy)
        tracking.set_look_ahead_cycles(lookahead)
        tracking.set_reactiveness(reactiveness)
        targets.set_count(lookahead)
        time: list[float] = []
        target_position: list[float] = []
        output_position: list[float] = []
        output_velocity: list[float] = []
        for step in range(steps):
            for sample in range(lookahead):
                sample_time = (step + sample) * dt
                position, velocity, acceleration = tracking_signal(signal, sample_time)
                targets.set_state(sample, [position], [velocity], [acceleration])
            current_time = step * dt
            target, _, _ = tracking_signal(signal, current_time)
            tracking.update_with_lookahead(targets, input_, output)
            time.append(current_time)
            target_position.append(target)
            output_position.append(output.new_position()[0])
            output_velocity.append(output.new_velocity()[0])
            output.pass_to_input(input_)
        diagnostics = tracking.last_diagnostics
        target_array = np.asarray(target_position, dtype=float)
        output_array = np.asarray(output_position, dtype=float)
        return {
            "time": np.asarray(time, dtype=float),
            "target": target_array,
            "output": output_array,
            "velocity": np.asarray(output_velocity, dtype=float),
            "error": np.abs(target_array - output_array),
            "diagnostics": diagnostics,
            "mae": mean_abs_error(target_array, output_array),
            "strategy": strategy.name,
            "signal": signal,
            "lookahead": lookahead,
            "reactiveness": reactiveness,
        }


def render_17_tracking_strategy_quality(output: Path, samples: int) -> GalleryImage:
    profiles = [
        run_tracking_profile(strategy=strategy, signal="sinus", lookahead=8, high_limits=True)
        for strategy in (
            TrackingOptimizedStrategy.STABLE,
            TrackingOptimizedStrategy.BALANCED,
            TrackingOptimizedStrategy.AGGRESSIVE,
        )
    ]
    path = output / "17_tracking_strategy_quality.png"
    fig, axes = setup_figure(
        "17 Tracking Strategy Quality",
        "Local Optimized tracking quality metrics by strategy using public diagnostics.",
        rows=2,
        cols=2,
    )
    labels = [str(profile["strategy"]) for profile in profiles]
    mae = [float(profile["mae"]) for profile in profiles]
    optimized = [profile["diagnostics"].optimized_step_count for profile in profiles]  # type: ignore[index]
    fallback = [profile["diagnostics"].fallback_step_count for profile in profiles]  # type: ignore[index]
    improvement = [profile["diagnostics"].improvement_ratio for profile in profiles]  # type: ignore[index]
    x = np.arange(len(labels))
    axes[0, 0].bar(x, mae, color=PALETTE[:3])
    axes[0, 0].set_xticks(x, labels)
    style_axis(axes[0, 0], "Mean Absolute Error", "MAE")
    axes[0, 1].bar(x - 0.18, optimized, width=0.36, label="optimized", color=PALETTE[1])
    axes[0, 1].bar(x + 0.18, fallback, width=0.36, label="fallback", color=PALETTE[2])
    axes[0, 1].set_xticks(x, labels)
    axes[0, 1].legend(frameon=False)
    style_axis(axes[0, 1], "Step Classification", "steps")
    axes[1, 0].bar(x, improvement, color=PALETTE[3])
    axes[1, 0].set_xticks(x, labels)
    style_axis(axes[1, 0], "Improvement Ratio", "ratio")
    for profile in profiles:
        axes[1, 1].plot(profile["time"], profile["error"], label=str(profile["strategy"]))  # type: ignore[index]
    axes[1, 1].legend(frameon=False)
    style_axis(axes[1, 1], "Tracking Error Over Time", "abs error")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "17 Tracking Strategy Quality",
        "Local Optimized tracking quality metrics by strategy using public diagnostics.",
        category="tracking",
        metrics={
            "strategy_count": 3,
            "signal": "sinus",
            "min_mae": round_metric(min(mae)),
            "max_mae": round_metric(max(mae)),
            "uses_public_tracking_diagnostics": True,
        },
    )


def render_18_tracking_candidate_families(output: Path, samples: int) -> GalleryImage:
    profile = run_tracking_profile(strategy=TrackingOptimizedStrategy.BALANCED, signal="sinus", lookahead=10, high_limits=True)
    diagnostics = profile["diagnostics"]
    family_counts = {
        "fast": diagnostics.fast_candidate_count,
        "instantaneous": diagnostics.instantaneous_candidate_count,
        "horizon": diagnostics.horizon_candidate_count,
        "terminal_blend": diagnostics.terminal_blend_candidate_count,
        "derivative_damped": diagnostics.derivative_damped_candidate_count,
        "lead_lag": diagnostics.lead_lag_candidate_count,
    }
    path = output / "18_tracking_candidate_families.png"
    fig, axes = setup_figure(
        "18 Tracking Candidate Families",
        "Candidate-family attribution from public tracking diagnostics.",
        rows=1,
        cols=2,
    )
    labels = list(family_counts)
    values = list(family_counts.values())
    x = np.arange(len(labels))
    axes[0, 0].bar(x, values, color=PALETTE[: len(labels)])
    axes[0, 0].set_xticks(x, labels, rotation=25, ha="right")
    style_axis(axes[0, 0], "Candidate Family Counts", "candidates")
    axes[0, 1].pie(values, labels=labels, autopct="%1.0f%%", colors=PALETTE[: len(labels)], textprops={"fontsize": 8})
    axes[0, 1].set_title("Family Share", loc="left", fontsize=11, fontweight="bold")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "18 Tracking Candidate Families",
        "Candidate-family attribution from public tracking diagnostics.",
        category="tracking",
        metrics={
            "candidate_count": diagnostics.candidate_count,
            "valid_candidate_count": diagnostics.valid_candidate_count,
            "family_count_sum": sum(values),
            "uses_public_tracking_diagnostics": True,
        },
    )


def render_19_tracking_fallback_diagnostics(output: Path, samples: int) -> GalleryImage:
    signals = ["ramp", "constant_acceleration", "sinus", "half_sinus"]
    profiles = [
        run_tracking_profile(strategy=TrackingOptimizedStrategy.BALANCED, signal=signal, lookahead=8, high_limits=(signal != "ramp"))
        for signal in signals
    ]
    path = output / "19_tracking_fallback_diagnostics.png"
    fig, axes = setup_figure(
        "19 Tracking Fallback Diagnostics",
        "Fallback, optimized, and budget diagnostics across deterministic target signals.",
        rows=2,
        cols=1,
    )
    x = np.arange(len(signals))
    optimized = [profile["diagnostics"].optimized_step_count for profile in profiles]  # type: ignore[index]
    fallback = [profile["diagnostics"].fallback_step_count for profile in profiles]  # type: ignore[index]
    budget = [profile["diagnostics"].budget_exhausted_count for profile in profiles]  # type: ignore[index]
    axes[0, 0].bar(x - 0.22, optimized, width=0.22, label="optimized", color=PALETTE[1])
    axes[0, 0].bar(x, fallback, width=0.22, label="fallback", color=PALETTE[2])
    axes[0, 0].bar(x + 0.22, budget, width=0.22, label="budget", color=PALETTE[5])
    axes[0, 0].set_xticks(x, signals, rotation=15, ha="right")
    axes[0, 0].legend(frameon=False)
    style_axis(axes[0, 0], "Diagnostics Counts", "count")
    mae = [float(profile["mae"]) for profile in profiles]
    axes[1, 0].plot(signals, mae, marker="o", color=PALETTE[0], linewidth=2.0)
    style_axis(axes[1, 0], "Mean Absolute Error By Signal", "MAE")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "19 Tracking Fallback Diagnostics",
        "Fallback, optimized, and budget diagnostics across deterministic target signals.",
        category="tracking",
        metrics={
            "signal_count": len(signals),
            "total_fallback_steps": int(sum(fallback)),
            "total_optimized_steps": int(sum(optimized)),
            "total_budget_exhausted": int(sum(budget)),
            "uses_public_tracking_diagnostics": True,
        },
    )


def render_20_tracking_near_tie_acceptance(output: Path, samples: int) -> GalleryImage:
    profile = run_tracking_profile(strategy=TrackingOptimizedStrategy.AGGRESSIVE, signal="half_sinus", lookahead=10, high_limits=True)
    diagnostics = profile["diagnostics"]
    path = output / "20_tracking_near_tie_acceptance.png"
    fig, axes = setup_figure(
        "20 Tracking Near-tie Acceptance",
        "Aggressive strategy score view with bounded near-tie policy context.",
        rows=1,
        cols=2,
    )
    axes[0, 0].plot(profile["time"], profile["target"], label="target", color="#333c47")  # type: ignore[index]
    axes[0, 0].plot(profile["time"], profile["output"], label="output", color=PALETTE[0])  # type: ignore[index]
    axes[0, 0].legend(frameon=False)
    style_axis(axes[0, 0], "Aggressive Tracking Response", "position")
    scores = [diagnostics.fast_score, diagnostics.best_score, diagnostics.fast_score * 1.01]
    axes[0, 1].bar(["fast", "best", "near-tie limit"], scores, color=[PALETTE[4], PALETTE[1], PALETTE[2]])
    style_axis(axes[0, 1], "Score Policy", "score")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "20 Tracking Near-tie Acceptance",
        "Aggressive strategy score view with bounded near-tie policy context.",
        category="tracking",
        metrics={
            "strategy": "AGGRESSIVE",
            "near_tie_policy_visualized": True,
            "near_tie_score_limit_ratio": 1.01,
            "fast_score": round_metric(diagnostics.fast_score),
            "best_score": round_metric(diagnostics.best_score),
            "improvement_ratio": round_metric(diagnostics.improvement_ratio),
        },
    )


def render_21_tracking_signal_response(output: Path, samples: int) -> GalleryImage:
    signals = ["ramp", "constant_acceleration", "sinus", "half_sinus"]
    profiles = [
        run_tracking_profile(strategy=TrackingOptimizedStrategy.BALANCED, signal=signal, lookahead=8, high_limits=(signal != "ramp"))
        for signal in signals
    ]
    path = output / "21_tracking_signal_response.png"
    fig, axes = setup_figure(
        "21 Tracking Signal Response",
        "Target and output response for the deterministic tracking audit signal families.",
        rows=2,
        cols=2,
    )
    for axis, profile in zip(axes.ravel(), profiles):
        axis.plot(profile["time"], profile["target"], label="target", color="#333c47")  # type: ignore[index]
        axis.plot(profile["time"], profile["output"], label="output", color=PALETTE[0])  # type: ignore[index]
        style_axis(axis, str(profile["signal"]), "position")
    axes[0, 0].legend(frameon=False)
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "21 Tracking Signal Response",
        "Target and output response for the deterministic tracking audit signal families.",
        category="tracking",
        metrics={
            "signal_count": len(signals),
            "max_mae": round_metric(max(float(profile["mae"]) for profile in profiles)),
            "uses_public_tracking_diagnostics": True,
        },
    )


def waypoint_reference() -> tuple[MotionSamples, float, list[float]]:
    return calculate_trajectory(3, configure_original_waypoint_input, samples=420, max_waypoints=10)


def per_section_reference() -> tuple[MotionSamples, float, list[float]]:
    return calculate_trajectory(3, _configure_per_section_reference_input, samples=420, max_waypoints=10)


def _configure_per_section_reference_input(input_: Input) -> None:
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
    input_.set_per_section_max_velocity(
        [
            [3.0, 2.0, 2.0],
            [1.2, 1.0, 1.1],
            [2.5, 1.8, 1.5],
            [1.0, 0.9, 1.0],
            [2.2, 1.6, 1.4],
            [1.1, 1.2, 1.0],
            [3.0, 2.0, 2.0],
        ]
    )
    input_.set_per_section_minimum_duration([0.0, 2.0, 0.0, 1.0, 0.0, 2.0, 0.0])


def section_boundaries(section_times: Sequence[float], duration: float) -> np.ndarray:
    return np.asarray([0.0, *section_times, duration], dtype=float)


def section_lengths(section_times: Sequence[float], duration: float) -> np.ndarray:
    return np.diff(section_boundaries(section_times, duration))


def render_22_waypoint_sections_timeline(output: Path, samples: int) -> GalleryImage:
    motion, duration, section_times = waypoint_reference()
    lengths = section_lengths(section_times, duration)
    path = output / "22_waypoint_sections_timeline.png"
    fig, axes = setup_figure(
        "22 Waypoint Sections Timeline",
        "Section timeline and position traces for local waypoint optimizer output.",
        rows=2,
        cols=1,
        sharex=False,
    )
    left = 0.0
    for index, length in enumerate(lengths):
        axes[0, 0].barh(["sections"], [length], left=[left], color=PALETTE[index % len(PALETTE)], label=f"section {index}")
        left += float(length)
    style_axis(axes[0, 0], "Section Durations", "timeline")
    axes[0, 0].set_xlabel("time [s]")
    for dof in range(motion.position.shape[1]):
        axes[1, 0].plot(motion.time, motion.position[:, dof], label=f"DoF {dof}", color=PALETTE[dof])
    for boundary in section_times:
        axes[1, 0].axvline(boundary, color="#59636f", linestyle="--", linewidth=0.9)
    axes[1, 0].legend(frameon=False, ncol=3)
    style_axis(axes[1, 0], "Position By Section", "position")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "22 Waypoint Sections Timeline",
        "Section timeline and position traces for local waypoint optimizer output.",
        category="waypoint",
        metrics={
            "section_count": int(len(lengths)),
            "duration": round_metric(duration),
            "waypoint_count": 4,
            "pro_cloud_equivalence_claim": False,
        },
    )


def render_23_waypoint_per_section_duration(output: Path, samples: int) -> GalleryImage:
    motion, duration, section_times = per_section_reference()
    lengths = section_lengths(section_times, duration)
    minimums = np.asarray([0.0, 2.0, 0.0, 1.0, 0.0, 2.0, 0.0], dtype=float)
    path = output / "23_waypoint_per_section_duration.png"
    fig, axes = setup_figure(
        "23 Waypoint Per-section Duration",
        "Actual section duration compared with configured per-section minimum duration.",
        rows=1,
        cols=1,
    )
    x = np.arange(len(lengths))
    axes[0, 0].bar(x - 0.18, lengths, width=0.36, label="actual", color=PALETTE[0])
    axes[0, 0].bar(x + 0.18, minimums[: len(lengths)], width=0.36, label="minimum", color=PALETTE[2])
    axes[0, 0].set_xticks(x, [str(index) for index in range(len(lengths))])
    axes[0, 0].legend(frameon=False)
    style_axis(axes[0, 0], "Section Duration", "seconds")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "23 Waypoint Per-section Duration",
        "Actual section duration compared with configured per-section minimum duration.",
        category="waypoint",
        metrics={
            "section_count": int(len(lengths)),
            "minimum_duration_sections": int(np.count_nonzero(minimums)),
            "duration": round_metric(duration),
            "per_section_minimum_duration": True,
            "sample_count": int(motion.time.size),
        },
    )


def render_24_waypoint_constraint_profiles(output: Path, samples: int) -> GalleryImage:
    max_velocity = np.asarray(
        [
            [3.0, 2.0, 2.0],
            [1.2, 1.0, 1.1],
            [2.5, 1.8, 1.5],
            [1.0, 0.9, 1.0],
            [2.2, 1.6, 1.4],
            [1.1, 1.2, 1.0],
            [3.0, 2.0, 2.0],
        ],
        dtype=float,
    )
    path = output / "24_waypoint_constraint_profiles.png"
    fig, axes = setup_figure(
        "24 Waypoint Constraint Profiles",
        "Configured local per-section max velocity profile by DoF.",
        rows=1,
        cols=2,
    )
    image = axes[0, 0].imshow(max_velocity.T, aspect="auto", cmap="viridis")
    axes[0, 0].set_yticks([0, 1, 2], ["DoF 0", "DoF 1", "DoF 2"])
    axes[0, 0].set_xticks(np.arange(max_velocity.shape[0]), [str(index) for index in range(max_velocity.shape[0])])
    axes[0, 0].set_title("Max Velocity Heatmap", loc="left", fontsize=11, fontweight="bold")
    fig.colorbar(image, ax=axes[0, 0], shrink=0.82)
    for dof in range(max_velocity.shape[1]):
        axes[0, 1].plot(np.arange(max_velocity.shape[0]), max_velocity[:, dof], marker="o", label=f"DoF {dof}")
    axes[0, 1].legend(frameon=False)
    style_axis(axes[0, 1], "Constraint By Section", "max velocity")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "24 Waypoint Constraint Profiles",
        "Configured local per-section max velocity profile by DoF.",
        category="waypoint",
        metrics={
            "section_count": int(max_velocity.shape[0]),
            "dofs": int(max_velocity.shape[1]),
            "per_section_constraints": True,
            "pro_cloud_equivalence_claim": False,
        },
    )


def render_25_waypoint_position_bounds(output: Path, samples: int) -> GalleryImage:
    motion, duration, _ = waypoint_reference()
    mins = np.min(motion.position, axis=0)
    maxs = np.max(motion.position, axis=0)
    path = output / "25_waypoint_position_bounds.png"
    fig, axes = setup_figure(
        "25 Waypoint Position Bounds",
        "Observed position ranges from local waypoint trajectory sampling.",
        rows=1,
        cols=1,
    )
    x = np.arange(motion.position.shape[1])
    axes[0, 0].bar(x, maxs - mins, bottom=mins, color=PALETTE[: motion.position.shape[1]])
    axes[0, 0].scatter(x, mins, color="#333c47", label="min")
    axes[0, 0].scatter(x, maxs, color="#c43d4d", label="max")
    axes[0, 0].set_xticks(x, [f"DoF {index}" for index in x])
    axes[0, 0].legend(frameon=False)
    style_axis(axes[0, 0], "Position Range", "position")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "25 Waypoint Position Bounds",
        "Observed position ranges from local waypoint trajectory sampling.",
        category="waypoint",
        metrics={
            "duration": round_metric(duration),
            "dofs": int(motion.position.shape[1]),
            "min_position": round_metric(float(np.min(mins))),
            "max_position": round_metric(float(np.max(maxs))),
            "pro_cloud_equivalence_claim": False,
        },
    )


def render_26_waypoint_online_section_changes(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        configure_original_waypoint_input(input_)
        input_.set_interrupt_calculation_duration(WAYPOINT_GALLERY_INTERRUPT_BUDGET_US)

    motion, result, changes = run_online(3, configure, max_waypoints=10)
    path = output / "26_waypoint_online_section_changes.png"
    fig, axes = setup_figure(
        "26 Waypoint Online Section Changes",
        "Online local waypoint update section changes and section index trace.",
        rows=2,
        cols=1,
        sharex=True,
    )
    axes[0, 0].step(motion.time, motion.section, where="post", color=PALETTE[0])  # type: ignore[arg-type]
    style_axis(axes[0, 0], "Section Index", "section")
    for dof in range(motion.position.shape[1]):
        axes[1, 0].plot(motion.time, motion.position[:, dof], label=f"DoF {dof}")
    for change in changes:
        axes[1, 0].axvline(change * 0.01, color="#59636f", linestyle="--", linewidth=0.8)
    axes[1, 0].legend(frameon=False, ncol=3)
    style_axis(axes[1, 0], "Position Trace", "position")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "26 Waypoint Online Section Changes",
        "Online local waypoint update section changes and section index trace.",
        category="waypoint",
        metrics={
            "online_result": result.name,
            "section_change_count": int(len(changes)),
            "samples": int(motion.time.size),
            "interrupt_budget_us": WAYPOINT_GALLERY_INTERRUPT_BUDGET_US,
            "soft_interruption_v1": True,
            "soft_interruption_expected": False,
            "pro_cloud_equivalence_claim": False,
        },
    )


def trajectory_reference(samples: int) -> tuple[MotionSamples, float, list[float], object]:
    otg = Ruckig(3, 0.01)
    input_ = Input(3)
    trajectory = Trajectory(3)
    configure_original_position_input(input_)
    try:
        if otg.calculate(input_, trajectory) != Result.WORKING:
            raise RuntimeError("trajectory reference calculation failed")
        motion = sample_trajectory(trajectory, samples)
        extrema = trajectory.position_extrema()
        return motion, trajectory.duration, trajectory.intermediate_durations(), extrema
    finally:
        close_handles(trajectory, input_, otg)


def render_27_trajectory_jerk_profile(output: Path, samples: int) -> GalleryImage:
    motion, duration, _, _ = trajectory_reference(samples)
    if motion.jerk is None:
        raise RuntimeError("trajectory jerk samples unavailable")
    path = output / "27_trajectory_jerk_profile.png"
    fig, axes = setup_figure(
        "27 Trajectory Jerk Profile",
        "Jerk samples from public trajectory sampling for the original position scenario.",
        rows=2,
        cols=1,
        sharex=True,
    )
    for dof in range(motion.jerk.shape[1]):
        axes[0, 0].plot(motion.time, motion.jerk[:, dof], label=f"DoF {dof}")
        axes[1, 0].plot(motion.time, np.cumsum(np.abs(motion.jerk[:, dof])) / motion.time.size, label=f"DoF {dof}")
    axes[0, 0].legend(frameon=False, ncol=3)
    style_axis(axes[0, 0], "Jerk", "jerk")
    style_axis(axes[1, 0], "Cumulative Absolute Jerk", "scaled sum")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "27 Trajectory Jerk Profile",
        "Jerk samples from public trajectory sampling for the original position scenario.",
        category="trajectory",
        metrics={
            "duration": round_metric(duration),
            "dofs": int(motion.jerk.shape[1]),
            "max_abs_jerk": round_metric(float(np.max(np.abs(motion.jerk)))),
        },
        original_examples=("01_position",),
    )


def render_28_trajectory_extrema(output: Path, samples: int) -> GalleryImage:
    _, duration, _, extrema = trajectory_reference(samples)
    mins = np.asarray([bound.min for bound in extrema], dtype=float)
    maxs = np.asarray([bound.max for bound in extrema], dtype=float)
    t_mins = np.asarray([bound.t_min for bound in extrema], dtype=float)
    t_maxs = np.asarray([bound.t_max for bound in extrema], dtype=float)
    path = output / "28_trajectory_extrema.png"
    fig, axes = setup_figure(
        "28 Trajectory Extrema",
        "Position extrema and occurrence times from public trajectory queries.",
        rows=1,
        cols=2,
    )
    x = np.arange(len(mins))
    axes[0, 0].bar(x - 0.18, mins, width=0.36, label="min", color=PALETTE[2])
    axes[0, 0].bar(x + 0.18, maxs, width=0.36, label="max", color=PALETTE[0])
    axes[0, 0].set_xticks(x, [f"DoF {index}" for index in x])
    axes[0, 0].legend(frameon=False)
    style_axis(axes[0, 0], "Position Extrema", "position")
    axes[0, 1].scatter(t_mins, mins, label="min time", color=PALETTE[2])
    axes[0, 1].scatter(t_maxs, maxs, label="max time", color=PALETTE[0])
    axes[0, 1].set_xlim(0.0, duration)
    axes[0, 1].legend(frameon=False)
    style_axis(axes[0, 1], "Extrema Time", "position")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "28 Trajectory Extrema",
        "Position extrema and occurrence times from public trajectory queries.",
        category="trajectory",
        metrics={
            "duration": round_metric(duration),
            "dofs": int(len(mins)),
            "min_position": round_metric(float(np.min(mins))),
            "max_position": round_metric(float(np.max(maxs))),
        },
        original_examples=("01_position",),
    )


def render_29_trajectory_phase_spans(output: Path, samples: int) -> GalleryImage:
    motion, duration, _, _ = trajectory_reference(samples)
    speed = np.linalg.norm(motion.velocity, axis=1)
    accel = np.linalg.norm(motion.acceleration, axis=1)
    phase_names = np.asarray(["cruise" if speed[i] > np.percentile(speed, 65) else "accelerate" if accel[i] > np.percentile(accel, 55) else "settle" for i in range(speed.size)])
    path = output / "29_trajectory_phase_spans.png"
    fig, axes = setup_figure(
        "29 Trajectory Phase Spans",
        "Derived local phase spans from sampled speed and acceleration magnitudes.",
        rows=2,
        cols=1,
        sharex=True,
    )
    colors = {"accelerate": PALETTE[1], "cruise": PALETTE[0], "settle": PALETTE[2]}
    for phase, color in colors.items():
        mask = phase_names == phase
        axes[0, 0].scatter(motion.time[mask], speed[mask], s=8, color=color, label=phase)
    axes[0, 0].legend(frameon=False, ncol=3)
    style_axis(axes[0, 0], "Speed Magnitude", "speed")
    axes[1, 0].plot(motion.time, accel, color=PALETTE[3])
    style_axis(axes[1, 0], "Acceleration Magnitude", "acceleration")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "29 Trajectory Phase Spans",
        "Derived local phase spans from sampled speed and acceleration magnitudes.",
        category="trajectory",
        metrics={
            "duration": round_metric(duration),
            "phase_count": int(len(colors)),
            "derived_phase_visualization": True,
            "max_speed": round_metric(float(np.max(speed))),
        },
        original_examples=("01_position",),
    )


def render_30_trajectory_synchronization(output: Path, samples: int) -> GalleryImage:
    motion, duration, _, _ = trajectory_reference(samples)
    independent_duration_proxy = np.asarray([
        float(motion.time[np.argmax(np.abs(motion.position[:, dof] - motion.position[0, dof]))])
        for dof in range(motion.position.shape[1])
    ])
    path = output / "30_trajectory_synchronization.png"
    fig, axes = setup_figure(
        "30 Trajectory Synchronization",
        "Synchronized output duration compared with per-DoF progress timing proxies.",
        rows=1,
        cols=2,
    )
    axes[0, 0].bar(["sync duration"], [duration], color=PALETTE[0])
    style_axis(axes[0, 0], "Synchronized Duration", "seconds")
    x = np.arange(independent_duration_proxy.size)
    axes[0, 1].bar(x, independent_duration_proxy, color=PALETTE[: independent_duration_proxy.size])
    axes[0, 1].axhline(duration, color="#333c47", linestyle="--", label="sync duration")
    axes[0, 1].set_xticks(x, [f"DoF {index}" for index in x])
    axes[0, 1].legend(frameon=False)
    style_axis(axes[0, 1], "Per-DoF Progress Timing Proxy", "seconds")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "30 Trajectory Synchronization",
        "Synchronized output duration compared with per-DoF progress timing proxies.",
        category="trajectory",
        metrics={
            "duration": round_metric(duration),
            "dofs": int(independent_duration_proxy.size),
            "synchronization_visualization": True,
        },
        original_examples=("01_position",),
    )


def render_31_gallery_coverage_matrix(output: Path, samples: int) -> GalleryImage:
    rows = ["original", "tracking", "waypoint", "trajectory", "summary"]
    cols = ["public C ABI", "local optimizer", "diagnostics", "boundary notes"]
    matrix = np.asarray(
        [
            [1, 1, 0, 1],
            [1, 0, 1, 1],
            [1, 1, 0, 1],
            [1, 0, 1, 0],
            [1, 1, 1, 1],
        ],
        dtype=float,
    )
    path = output / "31_gallery_coverage_matrix.png"
    fig, axes = setup_figure(
        "31 Gallery Coverage Matrix",
        "Visualization v2 coverage matrix across gallery families and evidence dimensions.",
        rows=1,
        cols=1,
    )
    image = axes[0, 0].imshow(matrix, cmap="Blues", vmin=0, vmax=1)
    axes[0, 0].set_xticks(np.arange(len(cols)), cols, rotation=20, ha="right")
    axes[0, 0].set_yticks(np.arange(len(rows)), rows)
    axes[0, 0].set_title("Coverage Matrix", loc="left", fontsize=11, fontweight="bold")
    for row in range(matrix.shape[0]):
        for col in range(matrix.shape[1]):
            axes[0, 0].text(col, row, "yes" if matrix[row, col] else "-", ha="center", va="center", color="#1f2933")
    fig.colorbar(image, ax=axes[0, 0], shrink=0.8)
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "31 Gallery Coverage Matrix",
        "Visualization v2 coverage matrix across gallery families and evidence dimensions.",
        category="summary",
        metrics={
            "png_count": 30,
            "category_count": len(rows),
            "public_c_abi_only": True,
        },
    )


def render_32_gallery_metrics_summary(output: Path, samples: int) -> GalleryImage:
    counts = {"original_mapping": 13, "tracking": 5, "waypoint": 5, "trajectory": 4, "summary": 3}
    path = output / "32_gallery_metrics_summary.png"
    fig, axes = setup_figure(
        "32 Gallery Metrics Summary",
        "Visualization v2 image inventory and expected category distribution.",
        rows=1,
        cols=2,
    )
    labels = list(counts)
    values = list(counts.values())
    x = np.arange(len(labels))
    axes[0, 0].bar(x, values, color=PALETTE[: len(labels)])
    axes[0, 0].set_xticks(x, labels, rotation=25, ha="right")
    style_axis(axes[0, 0], "PNG Count By Category", "count")
    axes[0, 1].pie(values, labels=labels, autopct="%1.0f%%", colors=PALETTE[: len(labels)], textprops={"fontsize": 8})
    axes[0, 1].set_title("Category Share", loc="left", fontsize=11, fontweight="bold")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "32 Gallery Metrics Summary",
        "Visualization v2 image inventory and expected category distribution.",
        category="summary",
        metrics={
            "png_count": sum(values),
            "original_mapping_count": counts["original_mapping"],
            "tracking_count": counts["tracking"],
            "waypoint_count": counts["waypoint"],
            "trajectory_count": counts["trajectory"],
            "summary_count": counts["summary"],
        },
    )


def render_33_gallery_boundary_summary(output: Path, samples: int) -> GalleryImage:
    boundaries = {
        "public ABI unchanged": 1,
        "PNG only": 1,
        "local only": 1,
        "no CI gate": 1,
        "no Pro/cloud claim": 1,
        "v1 on v0.9.0 tag": 1,
    }
    path = output / "33_gallery_boundary_summary.png"
    fig, axes = setup_figure(
        "33 Gallery Boundary Summary",
        "Visualization v2 release-scope boundaries for local evidence.",
        rows=1,
        cols=1,
    )
    labels = list(boundaries)
    values = list(boundaries.values())
    y = np.arange(len(labels))
    axes[0, 0].barh(y, values, color=PALETTE[1])
    axes[0, 0].set_yticks(y, labels)
    axes[0, 0].set_xlim(0, 1.2)
    axes[0, 0].set_xticks([0, 1], ["no", "yes"])
    style_axis(axes[0, 0], "Accepted Boundaries", "status")
    finish_figure(fig, path)
    return save_chart_image(
        output,
        path.name,
        "33 Gallery Boundary Summary",
        "Visualization v2 release-scope boundaries for local evidence.",
        category="summary",
        metrics={
            "public_c_abi_unchanged": True,
            "png_only": True,
            "local_only": True,
            "default_ci_gate": False,
            "pro_cloud_equivalence_claim": False,
            "v1_provenance_tag": "v0.9.0",
        },
    )


def write_manifest(output: Path, images: Sequence[GalleryImage]) -> None:
    manifest_images = []
    for image in images:
        path = output / image.file_name
        manifest_images.append(
            {
                "file": image.file_name,
                "title": image.title,
                "description": image.description,
                "category": image.category,
                "original_examples": list(image.original_examples),
                "bytes": path.stat().st_size,
                "sha256": sha256_file(path),
                "metrics": image.metrics,
            }
        )
    manifest = {
        "label": MANIFEST_LABEL,
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
        render_17_tracking_strategy_quality,
        render_18_tracking_candidate_families,
        render_19_tracking_fallback_diagnostics,
        render_20_tracking_near_tie_acceptance,
        render_21_tracking_signal_response,
        render_22_waypoint_sections_timeline,
        render_23_waypoint_per_section_duration,
        render_24_waypoint_constraint_profiles,
        render_25_waypoint_position_bounds,
        render_26_waypoint_online_section_changes,
        render_27_trajectory_jerk_profile,
        render_28_trajectory_extrema,
        render_29_trajectory_phase_spans,
        render_30_trajectory_synchronization,
        render_31_gallery_coverage_matrix,
        render_32_gallery_metrics_summary,
        render_33_gallery_boundary_summary,
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
