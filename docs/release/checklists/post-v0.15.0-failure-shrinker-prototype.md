# Post-v0.15.0 Failure Shrinker Prototype Checklist

Status: local evidence complete; ordinary remote push CI is observed after the
checklist commit is pushed.

This checklist records the `post-v0.15.0-failure-shrinker-prototype` slice. It
extends the oracle test executable with a local failure-oriented shrink mode for
random and per-DoF random seed/sample reproductions. It does not change
library runtime behavior, public API, public ABI, version metadata, release/tag
state, workflow behavior, wrapper publication status, upstream baseline, or
visualization assets.

## Scope

- [x] Added `ruckig_c_oracle_tests --shrink-random-failure SAMPLE --seed S`.
- [x] Added
  `ruckig_c_oracle_tests --shrink-random-per-dof-failure SAMPLE --seed S`.
- [x] Reused the existing random generators, replay materialization, oracle
  comparison runner, and shrink simplification order.
- [x] Required the original seed/sample to fail before failure shrinking starts.
- [x] Accepted only simplifications that preserve the same coarse failure
  class.
- [x] Printed original seed/sample, original failure summary, reduced failure
  summary, replay command, and fixture-ready `CaseData` initializer.
- [x] Wrote no generated fixture file automatically.
- [x] Kept tracking failure-oriented shrinking deferred.
- [x] Added deterministic CTest smoke coverage for the expected non-failing
  sample rejection path using `WILL_FAIL`.

## Failure Predicate

The prototype classifies the first oracle comparison failure by a stable coarse
message prefix, such as `result mismatch`, `duration mismatch`, or sample/update
vector mismatch. A candidate simplification is accepted only if the reduced
case still fails and the first failure stays in the same class.

This is intentionally narrower than a full shrinker:

- It does not try to synthesize failing samples.
- It does not write source files.
- It does not shrink tracking random audit failures.
- It does not enter default heavy random CI.

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
| `cmake --build --preset windows-clang-ninja-oracle` | Pass |
| `ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -R "ruckig_c_oracle_random_shrink_smoke\|ruckig_c_oracle_random_per_dof_shrink_smoke"` | Pass, including expected-failure smoke paths |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random 17 --seed 1` | Pass |
| `out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --shrink-random-per-dof 10 --seed 1` | Pass |
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
