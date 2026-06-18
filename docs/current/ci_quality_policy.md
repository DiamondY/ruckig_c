# CI Quality Policy

This document records the current CI quality boundary after the post-`v0.16.0`
quality follow-up work. It is a policy and evidence index, not a request to
expand the default workflow.

## Current Default CI Coverage

The default GitHub Actions workflow already covers a broad routine matrix:

| Area | Current coverage |
| --- | --- |
| Operating systems | Windows, Linux, and macOS C builds. |
| Windows consumers | clang-cl C-only static/shared paths and MSYS2 MinGW static/DLL consumer smokes. |
| Unix consumers | Installed CMake, pkg-config, shared install-tree, and Valgrind paths. |
| Sanitizers | Linux Clang ASan/UBSan routine job. |
| Memory tooling | Linux Valgrind job over C tests and an example. |
| ABI/export | Linux, Windows, and macOS shared-build public symbol/export checks. |
| Performance | Linux no-waypoint and waypoint benchmark jobs with threshold evidence. |
| Wrappers | Python prototype smoke on Windows/Linux/macOS and Rust alpha wrapper smoke on Linux. |
| Visualization | Manual artifact-style Visualization v2 generation/verification job. |

## Local Or Manual Evidence

The following remain local/manual or event-driven evidence unless a future CI
policy slice explicitly accepts their cost:

| Area | Current policy |
| --- | --- |
| `.clang-format` | Local configuration only; do not reformat the repository by default. |
| `.clang-tidy` | Local/manual targeted analysis only; no default CI gate. |
| cppcheck / CodeQL | Not enabled by default. |
| Coverage upload | Local LLVM coverage evidence only; no routine upload/report gate. |
| MSVC `cl` full matrix | Optional local standalone smoke where Visual Studio environment is available. |
| Windows ASan/UBSan | Not routine CI. |
| macOS sanitizer/oracle/performance | Not routine CI. |
| Heavy random / release random | Manual release or event-triggered evidence, not default push CI. |

The `post-v0.16.0-ci-static-analysis-evidence-policy` slice keeps this
boundary unchanged. The current `.clang-tidy` configuration is usable for
targeted local runs after the path-portable `HeaderFilterRegex` update. Local
evidence from this series:

```sh
clang-tidy test/c/linked_library_smoke.c --quiet -- -std=c99 -Iinclude
```

The command exited successfully locally and emitted only the normal warning
summary. This is recorded as manual evidence, not as a default CI gate.

## Promotion Criteria

Do not expand default CI merely because a tool exists. A new routine CI gate
needs all of the following:

- A concrete risk it catches better than current tests.
- A maintainer who accepts triage ownership.
- Stable tool availability on the target platform.
- Runtime cost that does not materially slow ordinary push feedback.
- A documented failure policy that distinguishes real regressions from tool
  noise or environment drift.

Static analysis, sanitizer expansion, coverage upload, MSVC full-matrix
coverage, and macOS oracle/performance work should each be opened as separate
policy slices if their triggers are met.

The same rule applies to formatter gates, cppcheck, CodeQL, and coverage upload:
do not add them to `.github/workflows/ci.yml` without an accepted owner, runtime
budget, noise policy, and failure triage path.

## Non-Goals

- Do not modify `.github/workflows/ci.yml` as part of documentation or local
  configuration polish.
- Do not add coverage-percentage gates for solver or tracking work.
- Do not add heavy random, performance, static analyzer, or visualization
  churn to ordinary CI without an accepted owner and cost budget.
- Do not publish patch releases for CI policy documentation alone.
