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

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError as exc:  # pragma: no cover - exercised by local dependency state
    raise SystemExit(
        "Pillow is required for local gallery generation. Install it in the "
        "local documentation environment, then rerun this script."
    ) from exc


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
    TargetState,
    TargetStateSequence,
    Tracking,
    TrackingMode,
    TrackingOptimizedStrategy,
    Trajectory,
    configure_library,
)


Color = tuple[int, int, int]

BLACK: Color = (32, 36, 42)
GRAY: Color = (100, 110, 122)
LIGHT_GRAY: Color = (218, 224, 232)
GRID: Color = (236, 240, 244)
BACKGROUND: Color = (250, 251, 252)
PANEL: Color = (255, 255, 255)
POSITION: Color = (23, 111, 181)
VELOCITY: Color = (32, 145, 108)
ACCELERATION: Color = (191, 91, 42)
JERK: Color = (112, 80, 160)
TARGET: Color = (72, 80, 90)
FAST: Color = (199, 70, 52)
OPTIMIZED: Color = (33, 125, 187)
WAYPOINT: Color = (146, 82, 178)


@dataclass(frozen=True)
class Series:
    label: str
    x: list[float]
    y: list[float]
    color: Color


@dataclass(frozen=True)
class GalleryImage:
    file_name: str
    title: str
    description: str
    source: str
    metrics: dict[str, float | int | str]


def _font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    names = (
        ("arialbd.ttf", "arial.ttf")
        if sys.platform.startswith("win")
        else ("DejaVuSans-Bold.ttf", "DejaVuSans.ttf")
    )
    for name in (names[0] if bold else names[1], names[1], names[0]):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


FONT_TITLE = _font(26, bold=True)
FONT_SUBTITLE = _font(15)
FONT_LABEL = _font(13)
FONT_SMALL = _font(11)


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


def _range(values: Iterable[float]) -> tuple[float, float]:
    items = list(values)
    minimum = min(items)
    maximum = max(items)
    if math.isclose(minimum, maximum):
        padding = max(abs(minimum), 1.0) * 0.1
    else:
        padding = (maximum - minimum) * 0.08
    return minimum - padding, maximum + padding


def _line_points(
    rect: tuple[int, int, int, int],
    series: Series,
    x_range: tuple[float, float],
    y_range: tuple[float, float],
) -> list[tuple[int, int]]:
    left, top, right, bottom = rect
    x0, x1 = x_range
    y0, y1 = y_range
    width = max(right - left, 1)
    height = max(bottom - top, 1)
    x_span = x1 - x0 if not math.isclose(x0, x1) else 1.0
    y_span = y1 - y0 if not math.isclose(y0, y1) else 1.0
    points: list[tuple[int, int]] = []
    for x, y in zip(series.x, series.y):
        px = left + int(round((x - x0) / x_span * width))
        py = bottom - int(round((y - y0) / y_span * height))
        points.append((px, py))
    return points


def draw_time_plot(
    image: Image.Image,
    rect: tuple[int, int, int, int],
    title: str,
    series: Sequence[Series],
    *,
    y_label: str,
    markers: Sequence[tuple[float, str, Color]] = (),
    notes: Sequence[str] = (),
) -> None:
    draw = ImageDraw.Draw(image)
    left, top, right, bottom = rect
    draw.rounded_rectangle((left, top, right, bottom), radius=8, fill=PANEL, outline=LIGHT_GRAY)
    draw.text((left + 18, top + 14), title, font=FONT_SUBTITLE, fill=BLACK)

    plot = (left + 64, top + 58, right - 28, bottom - 68)
    px0, py0, px1, py1 = plot
    draw.rectangle(plot, fill=(255, 255, 255), outline=LIGHT_GRAY)

    all_x = [value for item in series for value in item.x]
    all_y = [value for item in series for value in item.y]
    x_range = (min(all_x), max(all_x))
    y_range = _range(all_y)

    for index in range(1, 5):
        x = px0 + int((px1 - px0) * index / 5.0)
        y = py0 + int((py1 - py0) * index / 5.0)
        draw.line((x, py0, x, py1), fill=GRID)
        draw.line((px0, y, px1, y), fill=GRID)

    for marker_x, label, color in markers:
        if x_range[0] <= marker_x <= x_range[1]:
            span = x_range[1] - x_range[0]
            x = px0 + int(round((marker_x - x_range[0]) / span * (px1 - px0)))
            draw.line((x, py0, x, py1), fill=color, width=2)
            draw.text((x + 4, py0 + 4), label, font=FONT_SMALL, fill=color)

    for item in series:
        points = _line_points(plot, item, x_range, y_range)
        if len(points) > 1:
            draw.line(points, fill=item.color, width=3)

    draw.text((px0, py1 + 12), f"t: {x_range[0]:.2f}..{x_range[1]:.2f} s", font=FONT_SMALL, fill=GRAY)
    draw.text((px0, py0 - 18), y_label, font=FONT_SMALL, fill=GRAY)
    draw.text((px0 - 48, py0 - 2), f"{y_range[1]:.2f}", font=FONT_SMALL, fill=GRAY)
    draw.text((px0 - 48, py1 - 10), f"{y_range[0]:.2f}", font=FONT_SMALL, fill=GRAY)

    legend_x = left + 18
    legend_y = bottom - 42
    for item in series:
        draw.line((legend_x, legend_y + 7, legend_x + 24, legend_y + 7), fill=item.color, width=3)
        draw.text((legend_x + 30, legend_y), item.label, font=FONT_SMALL, fill=BLACK)
        legend_x += 150

    note_y = top + 15
    for note in notes:
        w = draw.textlength(note, font=FONT_SMALL)
        draw.text((right - 18 - w, note_y), note, font=FONT_SMALL, fill=GRAY)
        note_y += 16


def sample_trajectory(trajectory: Trajectory, samples: int) -> dict[str, list[float]]:
    duration = trajectory.duration
    times = [duration * index / (samples - 1) for index in range(samples)]
    values = {
        "time": times,
        "position": [],
        "velocity": [],
        "acceleration": [],
        "jerk": [],
        "section": [],
    }
    for time in times:
        sample = trajectory.at_time(time)
        values["position"].append(float(sample["position"][0]))
        values["velocity"].append(float(sample["velocity"][0]))
        values["acceleration"].append(float(sample["acceleration"][0]))
        values["jerk"].append(float(sample["jerk"][0]))
        values["section"].append(float(sample["section"]))
    return values


def new_page(title: str, subtitle: str) -> Image.Image:
    image = Image.new("RGB", (1100, 720), BACKGROUND)
    draw = ImageDraw.Draw(image)
    draw.text((40, 26), title, font=FONT_TITLE, fill=BLACK)
    draw.text((42, 62), subtitle, font=FONT_LABEL, fill=GRAY)
    return image


def calculate_trajectory(
    dofs: int,
    configure: Callable[[Input], None],
    *,
    max_waypoints: int = 0,
) -> tuple[Trajectory, Ruckig, Input]:
    otg = Ruckig(dofs, 0.01, max_number_of_waypoints=max_waypoints)
    input_ = Input(dofs, max_number_of_waypoints=max_waypoints)
    trajectory = Trajectory(dofs, max_number_of_waypoints=max_waypoints)
    configure(input_)
    result = otg.calculate(input_, trajectory)
    if result != Result.WORKING:
        raise RuntimeError(f"ruckig_calculate returned {result}")
    return trajectory, otg, input_


def render_position(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        input_.set_target_position([2.0])
        input_.set_max_velocity([2.0])
        input_.set_max_acceleration([1.5])
        input_.set_max_jerk([1.0])

    trajectory, otg, input_ = calculate_trajectory(1, configure)
    try:
        data = sample_trajectory(trajectory, samples)
        image = new_page(
            "No-waypoint position trajectory",
            "Sampled through ruckig_trajectory_at_time from the public C ABI.",
        )
        draw_time_plot(
            image,
            (40, 96, 1060, 660),
            "Position-controlled trajectory",
            [
                Series("position", data["time"], data["position"], POSITION),
                Series("velocity", data["time"], data["velocity"], VELOCITY),
                Series("acceleration", data["time"], data["acceleration"], ACCELERATION),
                Series("jerk", data["time"], data["jerk"], JERK),
            ],
            y_label="value",
            notes=[f"duration {trajectory.duration:.3f} s"],
        )
        path = output / "no_waypoint_position.png"
        image.save(path)
        return GalleryImage(
            file_name=path.name,
            title="No-waypoint position trajectory",
            description="Position, velocity, acceleration, and jerk sampled from a 1-DoF offline trajectory.",
            source="public C trajectory API via Python cffi prototype",
            metrics={"duration": round(trajectory.duration, 9), "samples": samples},
        )
    finally:
        trajectory.close()
        input_.close()
        otg.close()


def render_velocity(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        input_.set_control_interface(ControlInterface.VELOCITY)
        input_.set_target_velocity([1.0])
        input_.set_max_acceleration([1.0])
        input_.set_max_jerk([1.0])

    trajectory, otg, input_ = calculate_trajectory(1, configure)
    try:
        data = sample_trajectory(trajectory, samples)
        image = new_page(
            "Velocity-control trajectory",
            "Velocity target with bounded acceleration and jerk.",
        )
        draw_time_plot(
            image,
            (40, 96, 1060, 660),
            "Velocity-control profile",
            [
                Series("position", data["time"], data["position"], POSITION),
                Series("velocity", data["time"], data["velocity"], VELOCITY),
                Series("acceleration", data["time"], data["acceleration"], ACCELERATION),
            ],
            y_label="value",
            notes=[f"duration {trajectory.duration:.3f} s", "target velocity 1.0"],
        )
        path = output / "velocity_control.png"
        image.save(path)
        return GalleryImage(
            file_name=path.name,
            title="Velocity-control trajectory",
            description="Velocity-control mode trajectory generated through the public C ABI.",
            source="public C trajectory API via Python cffi prototype",
            metrics={"duration": round(trajectory.duration, 9), "samples": samples},
        )
    finally:
        trajectory.close()
        input_.close()
        otg.close()


def render_stop(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        input_.set_control_interface(ControlInterface.VELOCITY)
        input_.set_current_velocity([1.0])
        input_.set_target_velocity([0.0])
        input_.set_max_acceleration([1.0])
        input_.set_max_jerk([1.0])

    trajectory, otg, input_ = calculate_trajectory(1, configure)
    try:
        data = sample_trajectory(trajectory, samples)
        image = new_page(
            "Stop trajectory",
            "Deceleration to zero velocity with jerk and acceleration limits.",
        )
        draw_time_plot(
            image,
            (40, 96, 1060, 660),
            "Stop profile",
            [
                Series("position", data["time"], data["position"], POSITION),
                Series("velocity", data["time"], data["velocity"], VELOCITY),
                Series("acceleration", data["time"], data["acceleration"], ACCELERATION),
            ],
            y_label="value",
            notes=[f"duration {trajectory.duration:.3f} s", "initial velocity 1.0"],
        )
        path = output / "stop_trajectory.png"
        image.save(path)
        return GalleryImage(
            file_name=path.name,
            title="Stop trajectory",
            description="Velocity-control stop profile generated from a non-zero current velocity.",
            source="public C trajectory API via Python cffi prototype",
            metrics={"duration": round(trajectory.duration, 9), "samples": samples},
        )
    finally:
        trajectory.close()
        input_.close()
        otg.close()


def render_minimum_duration(output: Path, samples: int) -> GalleryImage:
    def configure(input_: Input) -> None:
        input_.set_target_position([2.0])
        input_.set_max_velocity([2.0])
        input_.set_max_acceleration([1.5])
        input_.set_max_jerk([1.0])
        input_.set_minimum_duration(5.0)

    trajectory, otg, input_ = calculate_trajectory(1, configure)
    try:
        data = sample_trajectory(trajectory, samples)
        image = new_page(
            "Minimum-duration constrained trajectory",
            "The trajectory is stretched to satisfy the public minimum-duration setting.",
        )
        draw_time_plot(
            image,
            (40, 96, 1060, 660),
            "Minimum duration profile",
            [
                Series("position", data["time"], data["position"], POSITION),
                Series("velocity", data["time"], data["velocity"], VELOCITY),
                Series("acceleration", data["time"], data["acceleration"], ACCELERATION),
            ],
            y_label="value",
            markers=[(5.0, "minimum duration", WAYPOINT)],
            notes=[f"duration {trajectory.duration:.3f} s"],
        )
        path = output / "minimum_duration.png"
        image.save(path)
        return GalleryImage(
            file_name=path.name,
            title="Minimum-duration constrained trajectory",
            description="Position trajectory constrained by ruckig_input_set_minimum_duration.",
            source="public C trajectory API via Python cffi prototype",
            metrics={"duration": round(trajectory.duration, 9), "minimum_duration": 5.0, "samples": samples},
        )
    finally:
        trajectory.close()
        input_.close()
        otg.close()


def render_waypoint(output: Path, samples: int) -> GalleryImage:
    waypoint = [1.0, -0.5]

    def configure(input_: Input) -> None:
        input_.set_target_position([2.0, -1.0])
        input_.set_max_velocity([1.0, 1.0])
        input_.set_max_acceleration([2.0, 2.0])
        input_.set_max_jerk([4.0, 4.0])
        input_.set_min_position([-1.0, -2.0])
        input_.set_max_position([3.0, 1.0])
        input_.set_intermediate_positions([waypoint])

    trajectory, otg, input_ = calculate_trajectory(2, configure, max_waypoints=1)
    try:
        duration = trajectory.duration
        times = [duration * index / (samples - 1) for index in range(samples)]
        position_0: list[float] = []
        position_1: list[float] = []
        for time in times:
            sample = trajectory.at_time(time)
            position_0.append(float(sample["position"][0]))
            position_1.append(float(sample["position"][1]))
        section_times = trajectory.intermediate_durations()

        image = new_page(
            "Local waypoint trajectory",
            "Two-DoF trajectory with an intermediate section marker.",
        )
        draw_time_plot(
            image,
            (40, 96, 1060, 660),
            "Waypoint sections",
            [
                Series("position[0]", times, position_0, POSITION),
                Series("position[1]", times, position_1, VELOCITY),
            ],
            y_label="position",
            markers=[(section_times[0], "waypoint section", WAYPOINT)] if section_times else (),
            notes=[f"duration {duration:.3f} s", "waypoint [1.0, -0.5]"],
        )
        path = output / "waypoint_sections.png"
        image.save(path)
        return GalleryImage(
            file_name=path.name,
            title="Local waypoint trajectory",
            description="Two-DoF local waypoint trajectory with intermediate section timing.",
            source="public C waypoint trajectory API via Python cffi prototype",
            metrics={
                "duration": round(duration, 9),
                "intermediate_duration": round(section_times[0], 9) if section_times else 0.0,
                "samples": samples,
            },
        )
    finally:
        trajectory.close()
        input_.close()
        otg.close()


def target_signal(time: float) -> tuple[float, float, float]:
    omega = 2.0 * math.pi * 0.35
    position = 0.28 * math.sin(omega * time)
    velocity = 0.28 * omega * math.cos(omega * time)
    acceleration = -0.28 * omega * omega * math.sin(omega * time)
    return position, velocity, acceleration


def configure_tracking_input(input_: Input) -> None:
    input_.set_max_velocity([1.4])
    input_.set_max_acceleration([2.4])
    input_.set_max_jerk([6.0])
    input_.set_min_position([-2.5])
    input_.set_max_position([2.5])


def run_fast_tracking(steps: int, dt: float) -> tuple[list[float], list[float], list[float]]:
    with Tracking(1, dt) as tracking, TargetState(1) as target, Input(1) as input_, Output(1) as output:
        configure_tracking_input(input_)
        tracking.set_mode(TrackingMode.FAST)
        times: list[float] = []
        target_position: list[float] = []
        output_position: list[float] = []
        for step in range(steps):
            time = step * dt
            position, velocity, acceleration = target_signal(time)
            target.set_position([position])
            target.set_velocity([velocity])
            target.set_acceleration([acceleration])
            tracking.update(target, input_, output)
            times.append(time)
            target_position.append(position)
            output_position.append(output.new_position()[0])
            output.pass_to_input(input_)
        return times, target_position, output_position


def run_optimized_tracking(
    steps: int,
    dt: float,
    lookahead: int,
) -> tuple[list[float], list[float], list[float], dict[str, float | int | str]]:
    with (
        Tracking(1, dt) as tracking,
        TargetStateSequence(1, lookahead) as targets,
        Input(1) as input_,
        Output(1) as output,
    ):
        configure_tracking_input(input_)
        tracking.set_mode(TrackingMode.OPTIMIZED)
        tracking.set_optimized_strategy(TrackingOptimizedStrategy.BALANCED)
        tracking.set_look_ahead_cycles(lookahead)
        tracking.set_max_optimized_candidates(16)
        targets.set_count(lookahead)
        times: list[float] = []
        target_position: list[float] = []
        output_position: list[float] = []
        for step in range(steps):
            for sample in range(lookahead):
                time = (step + sample) * dt
                position, velocity, acceleration = target_signal(time)
                targets.set_state(sample, [position], [velocity], [acceleration])
            current_time = step * dt
            position, _, _ = target_signal(current_time)
            tracking.update_with_lookahead(targets, input_, output)
            times.append(current_time)
            target_position.append(position)
            output_position.append(output.new_position()[0])
            output.pass_to_input(input_)
        diagnostics = tracking.last_diagnostics
        return (
            times,
            target_position,
            output_position,
            {
                "status": diagnostics.calculation_status.name,
                "strategy": diagnostics.optimized_strategy.name,
                "candidate_count": diagnostics.candidate_count,
                "optimized_step_count": diagnostics.optimized_step_count,
                "fallback_step_count": diagnostics.fallback_step_count,
                "improvement_ratio": round(diagnostics.improvement_ratio, 9),
            },
        )


def average_abs_error(target: Sequence[float], output: Sequence[float]) -> float:
    return sum(abs(a - b) for a, b in zip(target, output)) / max(len(target), 1)


def render_tracking(output: Path) -> GalleryImage:
    dt = 0.01
    steps = 320
    lookahead = 8
    fast_time, target_position, fast_position = run_fast_tracking(steps, dt)
    opt_time, _, opt_position, diagnostics = run_optimized_tracking(steps, dt, lookahead)
    fast_error = average_abs_error(target_position, fast_position)
    optimized_error = average_abs_error(target_position, opt_position)

    image = new_page(
        "Fast vs Optimized tracking",
        "Online tracking comparison using local bounded Optimized lookahead and diagnostics.",
    )
    draw_time_plot(
        image,
        (40, 96, 1060, 660),
        "Tracking sinus target",
        [
            Series("target", fast_time, target_position, TARGET),
            Series("Fast output", fast_time, fast_position, FAST),
            Series("Optimized Balanced", opt_time, opt_position, OPTIMIZED),
        ],
        y_label="position",
        notes=[
            f"Fast MAE {fast_error:.5f}",
            f"Optimized MAE {optimized_error:.5f}",
            f"candidates {diagnostics['candidate_count']}",
        ],
    )
    path = output / "tracking_fast_vs_optimized.png"
    image.save(path)
    return GalleryImage(
        file_name=path.name,
        title="Fast vs Optimized tracking",
        description="Local Fast and bounded Optimized tracking outputs plotted against a sinus target.",
        source="public C tracking API via Python cffi prototype",
        metrics={
            "steps": steps,
            "delta_time": dt,
            "lookahead": lookahead,
            "fast_mean_absolute_error": round(fast_error, 9),
            "optimized_mean_absolute_error": round(optimized_error, 9),
            **diagnostics,
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
                "source": image.source,
                "bytes": path.stat().st_size,
                "sha256": sha256_file(path),
                "metrics": image.metrics,
            }
        )
    manifest = {
        "label": "0.8.0-design visualization evidence",
        "generated_by": "tools/visualization/generate_gallery.py",
        "dependencies": {
            "renderer": "Pillow",
            "data_source": "bindings/python_prototype/ruckig_cffi.py",
            "shared_library": "RUCKIG_C_SHARED_LIBRARY or local shared build",
        },
        "images": manifest_images,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate local ruckig_c visualization gallery PNG assets.")
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
    parser.add_argument("--samples", type=int, default=240, help="Trajectory samples per offline plot.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.samples < 16:
        raise SystemExit("--samples must be at least 16")

    library = find_shared_library(args.library)
    if library is None:
        configure_library(None)
    else:
        configure_library(library)

    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)

    images = [
        render_position(output, args.samples),
        render_velocity(output, args.samples),
        render_stop(output, args.samples),
        render_minimum_duration(output, args.samples),
        render_waypoint(output, args.samples),
        render_tracking(output),
    ]
    write_manifest(output, images)

    for image in images:
        path = output / image.file_name
        print(f"generated {path.relative_to(REPO_ROOT)} {path.stat().st_size} bytes")
    print(f"generated {(output / 'manifest.json').relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
