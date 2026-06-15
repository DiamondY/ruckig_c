# Post-v0.16.0 Tooling Maintenance Checklist

Status: local verification complete; ordinary remote push CI is observed after
the checklist commit is pushed.

This checklist records the `post-v0.16.0-tooling-maintenance` slice. It extends
local failure-oriented shrinker tooling to tracking random audit cases and
clarifies oracle failure shrinker output. It does not change library runtime
behavior, public API, public ABI, version metadata, release/tag state, workflow
behavior, wrapper publication status, upstream baseline, or visualization
assets.

## Scope

- [x] Added
  `ruckig_c_tests --tracking-random-audit-shrink-failure SAMPLE --seed S`.
- [x] Required the original tracking audit seed/sample to fail before failure
  shrinking starts.
- [x] Rejected non-failing tracking samples with a development diagnostic and
  a pointer to pass-preserving `--tracking-random-audit-shrink`.
- [x] Accepted tracking simplifications only when they preserve the same
  coarse tracking audit failure class.
- [x] Reused the existing tracking audit generator, replay fixture printer,
  and shrink simplification order.
- [x] Added deterministic CTest smoke coverage for the expected non-failing
  tracking sample rejection path using `WILL_FAIL`.
- [x] Clarified oracle failure shrinker output with explicit reduced fixed-case
  instructions while preserving existing oracle CLIs.
- [x] Wrote no generated fixture file automatically.

## Failure Predicate

The tracking failure shrinker classifies failed audit cases with stable coarse
classes such as result, calculation status, diagnostics accounting, candidate
count, or generic tracking-audit invariant. Candidate simplifications are
accepted only when the reduced case still fails and the class is unchanged.

This remains developer tooling:

- It does not synthesize failing samples.
- It does not write source files.
- It does not enter default heavy random CI.
- It does not expose private solver, profile, candidate, or queue internals as
  public API.

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
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_random_shrink_smoke\|ruckig_c_oracle_random_per_dof_shrink_smoke"` | Pass, 4/4 including expected-failure oracle smoke paths |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_tracking_random_replay_smoke\|ruckig_c_tracking_random_audit_replay_smoke\|ruckig_c_tracking_quality_hardening\|ruckig_c_tracking_random_audit_shrink_failure_smoke"` | Pass, 4/4 |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random 17 --seed 1` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof 10 --seed 1` | Pass |
| `out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-shrink 22 --seed 1` | Pass |
| `out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit-shrink-failure 22 --seed 1` | Expected failure for non-failing sample; diagnostic points to pass-preserving shrink mode |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
