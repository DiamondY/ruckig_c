# Post-v0.15.0 Random Repro Materialization Checklist

Status: local implementation and gates complete; ordinary remote push CI is
observed after the checklist commit is pushed.

This checklist records the `post-v0.15.0-random-repro-materialization` quality
slice after `v0.15.0`. It adds developer/test replay commands that turn seeded
random oracle and tracking samples into single-sample replays with
fixture-ready initializer output. It does not start `0.16.0-design`, change
version metadata, expand the 184-symbol public C ABI, create a tag, publish
wrappers, change workflow behavior, edit ABI allowlists, update the upstream
baseline, or touch visualization assets.

## Scope

- [x] Added `ruckig_c_oracle_tests --replay-random SAMPLE --seed S`.
- [x] Added `ruckig_c_oracle_tests --replay-random-per-dof SAMPLE --seed S`.
- [x] Added `ruckig_c_tests --tracking-random-replay SAMPLE --seed S`.
- [x] Added `ruckig_c_tests --tracking-random-audit-replay SAMPLE --seed S`.
- [x] Kept existing random command behavior unchanged.
- [x] Added small deterministic CTest smoke entries for replay commands.
- [x] Deferred automatic shrinking; replay/export is stdout/stderr only and
  does not modify source files.

## Public API / ABI Boundary

- [x] No exported C function is added.
- [x] No public function signature is changed.
- [x] No enum numeric value or result-code numeric value is changed.
- [x] No public struct layout or public diagnostics layout is changed.
- [x] `include/ruckig_c/ruckig.h` is unchanged.
- [x] No ABI allowlist or public-symbol exception file is changed.
- [x] No version metadata, tag, GitHub Release, package-manager recipe,
  workflow, upstream baseline, or visualization asset is changed.
- [x] Python and Rust wrappers remain prototype-only.

## Tooling Additions

| Area | Added replay/materialization support |
| --- | --- |
| Oracle random | `--replay-random SAMPLE --seed S` fast-forwards the existing generator to one sample, runs the same C++ oracle comparison, and prints a `CaseData` initializer. |
| Oracle per-DoF random | `--replay-random-per-dof SAMPLE --seed S` preserves the existing per-DoF comparison mode and prints override vectors in the initializer. |
| Tracking random stress | `--tracking-random-replay SAMPLE --seed S` uses the same generated config as `--tracking-random`, runs one sample, and prints a `tracking_random_case_config_t` initializer plus context. |
| Tracking random audit | `--tracking-random-audit-replay SAMPLE --seed S` uses the same audit config generator as `--tracking-random-audit`, runs one sample, and prints a `tracking_audit_case_config_t` initializer plus diagnostics/family summary. |
| CTest smoke | Adds single-sample replay smoke tests for normal and oracle builds without adding heavy random cost. |

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `cmake --build --preset windows-clang-ninja-shared` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tracking_random_replay_smoke\|ruckig_c_tracking_random_audit_replay_smoke\|ruckig_c_tracking_random_audit\|ruckig_c_tracking_quality_hardening"` | Pass, 4/4 |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_random_replay_smoke\|ruckig_c_oracle_random_per_dof_replay_smoke\|ruckig_c_oracle_random_smoke\|ruckig_c_oracle_random_per_dof_smoke"` | Pass, 4/4 |
| `out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-replay 22 --seed 1` | Pass, prints `tracking_random_case_config_t` initializer |
| `out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-replay 22 --seed 1` | Pass, prints `tracking_audit_case_config_t` initializer and diagnostics |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --replay-random 17 --seed 1` | Pass, prints `CaseData` initializer |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --replay-random-per-dof 10 --seed 1` | Pass, prints per-DoF `CaseData` initializer |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Pass, 65/65 |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_tests\|ruckig_c_oracle_random_smoke\|ruckig_c_oracle_random_per_dof_smoke"` | Pass, 3/3 |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure` | Pass, 65/65 |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Pass |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Pass |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Empty |
| `git diff --check` | Pass |

## Materialization Workflow

1. Run the failing random command and capture the reported seed and sample.
2. Re-run the matching replay command with that seed and sample.
3. Copy the emitted initializer into the relevant fixed-case corpus.
4. Reduce the case manually only after it is fixed and reproducible.
5. Keep large random commands as local/manual evidence; routine CTest should
   receive only deterministic reduced regressions.

## Remote CI

Ordinary remote push CI is observed after pushing the checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.

## Deferred

- Automatic random failure shrinker.
- JSON fixture schema or generated source-file writing.
- Large random gates in default push CI.
- Public API, wrapper publication, package recipes, upstream baseline upgrades,
  release/tag work, or `0.16.0-design` transition.
