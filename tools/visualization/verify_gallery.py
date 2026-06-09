from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "assets" / "visualization"
GENERATOR = REPO_ROOT / "tools" / "visualization" / "generate_gallery.py"

EXPECTED_SIZE = (1400, 900)
EXPECTED_LABEL = "0.10.0-alpha visualization v2 evidence"
EXPECTED_RENDERER = "Matplotlib Agg"
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

EXPECTED_IMAGES = [
    "01_position.png",
    "02_position_offline.png",
    "03_waypoints_local.png",
    "04_waypoints_online_local.png",
    "05_velocity.png",
    "06_stop.png",
    "07_minimum_duration.png",
    "08_per_section_minimum_duration.png",
    "09_dynamic_dofs.png",
    "10_dynamic_dofs_waypoints_local.png",
    "14_tracking_online_local.png",
    "15_tracking_offline_local.png",
    "16_speed_brake_phases.png",
    "17_tracking_strategy_quality.png",
    "18_tracking_candidate_families.png",
    "19_tracking_fallback_diagnostics.png",
    "20_tracking_near_tie_acceptance.png",
    "21_tracking_signal_response.png",
    "22_waypoint_sections_timeline.png",
    "23_waypoint_per_section_duration.png",
    "24_waypoint_constraint_profiles.png",
    "25_waypoint_position_bounds.png",
    "26_waypoint_online_section_changes.png",
    "27_trajectory_jerk_profile.png",
    "28_trajectory_extrema.png",
    "29_trajectory_phase_spans.png",
    "30_trajectory_synchronization.png",
    "31_gallery_coverage_matrix.png",
    "32_gallery_metrics_summary.png",
    "33_gallery_boundary_summary.png",
]

EXPECTED_MAPPING = {
    "01_position.png": ["01_position"],
    "02_position_offline.png": ["02_position_offline"],
    "03_waypoints_local.png": ["03_waypoints"],
    "04_waypoints_online_local.png": ["04_waypoints_online"],
    "05_velocity.png": ["05_velocity"],
    "06_stop.png": ["06_stop"],
    "07_minimum_duration.png": ["07_minimum_duration"],
    "08_per_section_minimum_duration.png": ["08_per_section_minimum_duration"],
    "09_dynamic_dofs.png": ["09_dynamic_dofs"],
    "10_dynamic_dofs_waypoints_local.png": ["10_dynamic_dofs_waypoints"],
    "14_tracking_online_local.png": ["14_tracking"],
    "15_tracking_offline_local.png": ["15_tracking_offline"],
    "16_speed_brake_phases.png": ["16_speed"],
    "17_tracking_strategy_quality.png": [],
    "18_tracking_candidate_families.png": [],
    "19_tracking_fallback_diagnostics.png": [],
    "20_tracking_near_tie_acceptance.png": [],
    "21_tracking_signal_response.png": [],
    "22_waypoint_sections_timeline.png": [],
    "23_waypoint_per_section_duration.png": [],
    "24_waypoint_constraint_profiles.png": [],
    "25_waypoint_position_bounds.png": [],
    "26_waypoint_online_section_changes.png": [],
    "27_trajectory_jerk_profile.png": ["01_position"],
    "28_trajectory_extrema.png": ["01_position"],
    "29_trajectory_phase_spans.png": ["01_position"],
    "30_trajectory_synchronization.png": ["01_position"],
    "31_gallery_coverage_matrix.png": [],
    "32_gallery_metrics_summary.png": [],
    "33_gallery_boundary_summary.png": [],
}

EXPECTED_CATEGORIES = {
    **{file_name: "original_mapping" for file_name in EXPECTED_IMAGES[:13]},
    **{file_name: "tracking" for file_name in EXPECTED_IMAGES[13:18]},
    **{file_name: "waypoint" for file_name in EXPECTED_IMAGES[18:23]},
    **{file_name: "trajectory" for file_name in EXPECTED_IMAGES[23:27]},
    **{file_name: "summary" for file_name in EXPECTED_IMAGES[27:30]},
}

EXPECTED_EXCLUSIONS = {
    "11_eigen_vector_type": "C++ vector ergonomics, not C ABI visualization surface",
    "12_custom_vector_type": "C++ vector ergonomics, not C ABI visualization surface",
    "13_custom_vector_type_dynamic_dofs": "C++ vector ergonomics, not C ABI visualization surface",
}

PRO_CLOUD_BOUNDARY_IMAGES = {
    "03_waypoints_local.png",
    "04_waypoints_online_local.png",
    "10_dynamic_dofs_waypoints_local.png",
}

TRACKING_IMAGES = {
    "14_tracking_online_local.png",
    "15_tracking_offline_local.png",
}

FORBIDDEN_KEY_NAMES = {
    "created_at",
    "date",
    "datetime",
    "generated_at",
    "timestamp",
    "time_generated",
}

FORBIDDEN_VALUE_SUBSTRINGS = [
    "\\_local\\",
    "/_local/",
    "visualization-venv",
    "\\Users\\",
    "/Users/",
]


class VerificationError(RuntimeError):
    pass


def fail(message: str) -> None:
    raise VerificationError(message)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        fail(f"{path} is not valid JSON: {error}")


def read_png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as handle:
        signature = handle.read(8)
        if signature != PNG_SIGNATURE:
            fail(f"{path.name} is not a PNG file")

        header = handle.read(8)
        if len(header) != 8:
            fail(f"{path.name} is truncated before IHDR")
        chunk_length, chunk_type = struct.unpack(">I4s", header)
        if chunk_type != b"IHDR":
            fail(f"{path.name} first PNG chunk is not IHDR")
        if chunk_length != 13:
            fail(f"{path.name} IHDR length is {chunk_length}, expected 13")

        data = handle.read(13)
        if len(data) != 13:
            fail(f"{path.name} has truncated IHDR data")
        width, height = struct.unpack(">II", data[:8])
        return int(width), int(height)


def require_dict(value: Any, context: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        fail(f"{context} must be an object")
    return value


def require_list(value: Any, context: str) -> list[Any]:
    if not isinstance(value, list):
        fail(f"{context} must be an array")
    return value


def scan_reproducibility_fields(value: Any, path: str = "$") -> None:
    if isinstance(value, dict):
        for key, child in value.items():
            if key.lower() in FORBIDDEN_KEY_NAMES:
                fail(f"manifest contains non-deterministic key {path}.{key}")
            scan_reproducibility_fields(child, f"{path}.{key}")
        return

    if isinstance(value, list):
        for index, child in enumerate(value):
            scan_reproducibility_fields(child, f"{path}[{index}]")
        return

    if isinstance(value, str):
        for needle in FORBIDDEN_VALUE_SUBSTRINGS:
            if needle in value:
                fail(f"manifest contains forbidden local value at {path}: {needle}")
        if re.search(r"\b[A-Za-z]:[\\/]", value):
            fail(f"manifest contains Windows absolute path at {path}")


def validate_manifest_shape(manifest: dict[str, Any]) -> list[dict[str, Any]]:
    if manifest.get("label") != EXPECTED_LABEL:
        fail(f"manifest label changed: {manifest.get('label')!r}")
    if manifest.get("generated_by") != "tools/visualization/generate_gallery.py":
        fail("manifest generated_by does not point to the gallery generator")

    dependencies = require_dict(manifest.get("dependencies"), "manifest.dependencies")
    if dependencies.get("renderer") != EXPECTED_RENDERER:
        fail(f"manifest renderer changed: {dependencies.get('renderer')!r}")
    if dependencies.get("data_source") != "bindings/python_prototype/ruckig_cffi.py":
        fail("manifest data_source changed")

    exclusions = require_dict(manifest.get("excluded_original_examples"), "manifest.excluded_original_examples")
    if exclusions != EXPECTED_EXCLUSIONS:
        fail("manifest excluded_original_examples does not match expected 11-13 exclusions")

    images = require_list(manifest.get("images"), "manifest.images")
    if len(images) != len(EXPECTED_IMAGES):
        fail(f"manifest has {len(images)} images, expected {len(EXPECTED_IMAGES)}")
    image_objects = [require_dict(image, f"manifest.images[{index}]") for index, image in enumerate(images)]
    files = [str(image.get("file")) for image in image_objects]
    if files != EXPECTED_IMAGES:
        fail(f"manifest image order/list mismatch: {files}")

    scan_reproducibility_fields(manifest)
    return image_objects


def validate_image_entry(output: Path, image: dict[str, Any]) -> None:
    file_name = str(image["file"])
    path = output / file_name
    if not path.exists():
        fail(f"missing PNG asset: {file_name}")
    if path.stat().st_size <= 0:
        fail(f"{file_name} is empty")

    width, height = read_png_size(path)
    if (width, height) != EXPECTED_SIZE:
        fail(f"{file_name} size is {(width, height)}, expected {EXPECTED_SIZE}")

    actual_bytes = path.stat().st_size
    actual_sha256 = sha256_file(path)
    if image.get("bytes") != actual_bytes:
        fail(f"{file_name} byte count mismatch: manifest {image.get('bytes')} actual {actual_bytes}")
    if image.get("sha256") != actual_sha256:
        fail(f"{file_name} sha256 mismatch")

    if image.get("original_examples") != EXPECTED_MAPPING[file_name]:
        fail(f"{file_name} original_examples mismatch: {image.get('original_examples')!r}")
    if image.get("category") != EXPECTED_CATEGORIES[file_name]:
        fail(f"{file_name} category mismatch: {image.get('category')!r}")

    metrics = require_dict(image.get("metrics"), f"{file_name}.metrics")
    dofs = metrics.get("dofs")
    if (
        file_name in EXPECTED_IMAGES[:13]
        and file_name != "16_speed_brake_phases.png"
        and file_name not in TRACKING_IMAGES
        and dofs != 3
    ):
        fail(f"{file_name} expected dofs metric 3, got {dofs!r}")

    if file_name in PRO_CLOUD_BOUNDARY_IMAGES and metrics.get("pro_cloud_equivalence_claim") is not False:
        fail(f"{file_name} must record pro_cloud_equivalence_claim=false")
    if metrics.get("pro_cloud_equivalence_claim") is True:
        fail(f"{file_name} must not claim Pro/cloud equivalence")

    if file_name == "16_speed_brake_phases.png":
        if metrics.get("speed_control_abi_claim") is not False:
            fail("16_speed_brake_phases.png must record speed_control_abi_claim=false")
        if metrics.get("local_external_retiming") is not True:
            fail("16_speed_brake_phases.png must record local_external_retiming=true")

    if file_name in TRACKING_IMAGES:
        for key in ["candidate_count", "fallback_step_count", "optimized_step_count", "status", "strategy"]:
            if key not in metrics:
                fail(f"{file_name} missing tracking metric {key}")

    category = EXPECTED_CATEGORIES[file_name]
    if category == "tracking" and metrics.get("uses_public_tracking_diagnostics") is not True and file_name != "20_tracking_near_tie_acceptance.png":
        fail(f"{file_name} must record public tracking diagnostics usage")
    if category == "waypoint" and metrics.get("pro_cloud_equivalence_claim") is True:
        fail(f"{file_name} must not claim Pro/cloud equivalence")
    if category == "summary":
        if file_name == "31_gallery_coverage_matrix.png" and metrics.get("public_c_abi_only") is not True:
            fail("31_gallery_coverage_matrix.png must record public_c_abi_only=true")
        if file_name == "32_gallery_metrics_summary.png" and metrics.get("png_count") != len(EXPECTED_IMAGES):
            fail("32_gallery_metrics_summary.png must record png_count=30")
        if file_name == "33_gallery_boundary_summary.png":
            if metrics.get("public_c_abi_unchanged") is not True:
                fail("33_gallery_boundary_summary.png must record public_c_abi_unchanged=true")
            if metrics.get("default_ci_gate") is not False:
                fail("33_gallery_boundary_summary.png must record default_ci_gate=false")
            if metrics.get("v1_provenance_tag") != "v0.9.0":
                fail("33_gallery_boundary_summary.png must record v1_provenance_tag=v0.9.0")


def validate_committed_gallery(output: Path) -> dict[str, Any]:
    manifest_path = output / "manifest.json"
    if not manifest_path.exists():
        fail(f"missing manifest: {manifest_path}")

    png_files = sorted(path.name for path in output.glob("*.png"))
    if png_files != EXPECTED_IMAGES:
        fail(f"PNG asset list mismatch: {png_files}")

    manifest = require_dict(read_json(manifest_path), "manifest")
    images = validate_manifest_shape(manifest)
    for image in images:
        validate_image_entry(output, image)
    return manifest


def canonical_json(value: Any) -> str:
    return json.dumps(value, indent=2, sort_keys=True) + "\n"


def compare_regenerated_gallery(output: Path, generated: Path) -> None:
    committed_manifest = require_dict(read_json(output / "manifest.json"), "committed manifest")
    regenerated_manifest = require_dict(read_json(generated / "manifest.json"), "regenerated manifest")
    if canonical_json(committed_manifest) != canonical_json(regenerated_manifest):
        fail("strict regeneration manifest JSON differs from committed manifest")

    mismatches: list[str] = []
    for file_name in EXPECTED_IMAGES:
        committed_path = output / file_name
        regenerated_path = generated / file_name
        if not regenerated_path.exists():
            mismatches.append(f"{file_name}: missing from regenerated output")
            continue
        committed_hash = sha256_file(committed_path)
        regenerated_hash = sha256_file(regenerated_path)
        if committed_hash != regenerated_hash:
            mismatches.append(
                f"{file_name}: sha256 committed={committed_hash[:12]} regenerated={regenerated_hash[:12]}"
            )
        if committed_path.stat().st_size != regenerated_path.stat().st_size:
            mismatches.append(
                f"{file_name}: bytes committed={committed_path.stat().st_size} "
                f"regenerated={regenerated_path.stat().st_size}"
            )
    if mismatches:
        fail("strict regeneration differs:\n" + "\n".join(mismatches))


def run_strict_regeneration(output: Path, library: str | None) -> None:
    env_library = os.environ.get("RUCKIG_C_SHARED_LIBRARY")
    if library is None and not env_library:
        fail("--strict-regenerate requires --library or RUCKIG_C_SHARED_LIBRARY")

    temp_parent = REPO_ROOT / "out" / "visualization-verify"
    temp_parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="gallery-", dir=str(temp_parent)) as temp_dir:
        generated = Path(temp_dir)
        command = [sys.executable, str(GENERATOR), "--output", str(generated)]
        if library is not None:
            command.extend(["--library", library])

        result = subprocess.run(
            command,
            cwd=REPO_ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if result.returncode != 0:
            fail(
                "strict regeneration command failed\n"
                f"command: {' '.join(command)}\n"
                f"stdout:\n{result.stdout}\n"
                f"stderr:\n{result.stderr}"
            )

        validate_committed_gallery(generated)
        compare_regenerated_gallery(output, generated)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Verify committed ruckig_c visualization gallery assets.")
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help="Directory containing committed PNG assets and manifest.json.",
    )
    parser.add_argument(
        "--strict-regenerate",
        action="store_true",
        help="Regenerate the gallery into out/ and compare the result byte-for-byte with committed assets.",
    )
    parser.add_argument(
        "--library",
        type=str,
        default=None,
        help="Path to ruckig_c shared library for --strict-regenerate.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output = args.output.resolve()

    try:
        validate_committed_gallery(output)
        strict_note = "not requested"
        if args.strict_regenerate:
            run_strict_regeneration(output, args.library)
            strict_note = "passed"
    except VerificationError as error:
        print(f"visualization gallery verification failed: {error}", file=sys.stderr)
        return 1

    print("visualization gallery verification passed")
    print(f"assets: {len(EXPECTED_IMAGES)} PNG files")
    print(f"png_size: {EXPECTED_SIZE[0]}x{EXPECTED_SIZE[1]}")
    print(f"manifest_label: {EXPECTED_LABEL}")
    print("original_examples: 01-10,14-16")
    print("excluded_original_examples: 11-13")
    print(f"strict_regenerate: {strict_note}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
