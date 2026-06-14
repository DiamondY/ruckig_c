# Post-v0.15.0 Next Design Readiness Checklist

Status: local evidence complete; ordinary remote push CI is observed after the
checklist commit is pushed.

This checklist records the docs-only readiness audit after the
post-`v0.15.0` quality closeout. It recommends starting
`0.16.0-design-public-diagnostics` but does not implement public API, change
the 184-symbol `v0.15.0` public C ABI baseline, change version metadata, create
a tag or release, publish wrappers, edit workflows, change ABI allowlists,
update the upstream baseline, or touch visualization assets.

## Readiness Result

- [x] `post-v0.15.0-quality-closeout` completed with branch coverage `73.91%`.
- [x] Recent post-release quality-slice push CI runs concluded success.
- [x] Public ABI/export evidence remains at the stable 184-symbol baseline.
- [x] No open quality item blocks a docs-only `0.16.0` design line.
- [x] `Public diagnostics` is selected as the first `0.16.0` design topic.
- [x] Wrapper stabilization remains deferred to a separate design decision.
- [x] Failure-oriented random shrinking remains a local-tool candidate.
- [x] Solver long-tail branch coverage remains regression/oracle driven.
- [x] Package-manager recipes remain frozen.

## Candidate Ranking

| Rank | Candidate | Disposition |
| ---: | --- | --- |
| 1 | Public diagnostics | Start docs-only design next. |
| 2 | Wrapper stabilization | Defer; needs separate API and release policy. |
| 3 | Failure-oriented random shrinker | Defer as local tooling; no public ABI dependency. |
| 4 | Solver long-tail oracle-backed coverage | Defer unless backed by concrete behavior. |
| 5 | Package-manager recipes | Keep frozen until demand-driven. |

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
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_property_invariants"` | Pass, 3/3 |

## Boundary Checks

| Check | Result |
| --- | --- |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Clean |
| `git diff -- original/ruckig-main docs/assets/visualization` | Clean |
| `git diff --check` | Pass; Git reported expected CRLF normalization warnings only |

## Remote CI

Ordinary remote push CI is observed after pushing this checklist commit to
`main`. The run URL and conclusion should be recorded in the delivery summary.
