# Post-v0.15.0 Random Shrinker MVP Checklist

Status: local evidence complete; ordinary remote push CI is observed after the
checklist commit is pushed.

This checklist records the `post-v0.15.0-random-shrinker-mvp` slice after the
quality evidence refresh. The slice turns the existing replay/export commands
into a small pass-preserving shrink workflow for local development. It does not
write generated fixtures automatically, target coverage growth, start
`0.16.0-design`, change version metadata, expand the 184-symbol public C ABI,
create a tag, publish wrappers, change workflow behavior, edit ABI allowlists,
update the upstream baseline, or touch visualization assets.

## Scope

- [x] Added oracle shrink commands:
  `ruckig_c_oracle_tests --shrink-random SAMPLE --seed S` and
  `ruckig_c_oracle_tests --shrink-random-per-dof SAMPLE --seed S`.
- [x] Added tracking audit shrink command:
  `ruckig_c_tests --tracking-random-audit-shrink SAMPLE --seed S`.
- [x] Reused the existing random generators and replay config generation paths
  so seed/sample materialization keeps the same RNG consumption order.
- [x] Kept existing replay and random commands behavior-compatible.
- [x] Added only deterministic single-sample CTest smoke entries.
- [x] Kept shrink output on stdout/stderr only; no source fixture is written
  automatically.

## Shrink Strategy

The MVP shrinker is deliberately conservative. It confirms the original
seed/sample passes the same comparison or tracking audit runner, then accepts
only simplifications that still pass the same single-case check.

| Area | Simplification order |
| --- | --- |
| Oracle random | DoF count, enabled mask, per-DoF control/synchronization overrides, min velocity/acceleration vectors, synchronization, duration discretization, minimum duration, numeric quarter rounding. |
| Oracle per-DoF random | Same pass-preserving strategy, using the existing `run_case(test_case, false)` comparison mode. |
| Tracking audit | DoF count, lookahead count, disabled-DoF mask, tight constraints, strategy, signal, reactiveness, and start time. |

The output includes the original seed/sample, reduced-case summary,
fixture-ready initializer, and the replay command for the original generated
sample. Full automatic failure shrinkage and source-file materialization remain
deferred.

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

## Local Gates

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Pass |
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_random_replay_smoke\|ruckig_c_oracle_random_per_dof_replay_smoke\|ruckig_c_oracle_random_shrink_smoke"` | Pass, 3/3 |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_random_per_dof_shrink_smoke"` | Pass, 1/1 |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tracking_random_replay_smoke\|ruckig_c_tracking_random_audit_replay_smoke\|ruckig_c_tracking_quality_hardening"` | Pass, 3/3 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random 17 --seed 1` | Pass; accepted synchronization, duration, and numeric simplifications |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof 10 --seed 1` | Pass; accepted DoF, per-DoF override, duration, and numeric simplifications |
| `out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-shrink 22 --seed 1` | Pass; reduced sample `22` from 8 DoFs / lookahead 5 / aggressive / tight to 1 DoF / lookahead 1 / stable / default |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure` | Pass, 67/67 |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_tests\|ruckig_c_oracle_random_smoke\|ruckig_c_oracle_random_per_dof_smoke"` | Pass, 3/3 |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
