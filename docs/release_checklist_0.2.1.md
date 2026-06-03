# Ruckig C 0.2.1 Release Checklist

This checklist records the local patch-release closeout evidence for `v0.2.1`.
Remote CI, manual workflow, tag, and GitHub Release evidence must be filled
after the final release commit is pushed and tagged.

## Scope

- [ ] Final commit hash recorded after commit creation.
- [x] Version is `0.2.1` in `CMakeLists.txt` if a `v0.2.1` tag is being cut.
- [x] Version macros in `include/ruckig_c/ruckig.h` are `0.2.1` if a `v0.2.1`
  tag is being cut.
- [x] `CHANGELOG.md` has a `0.2.1` release entry.
- [x] No public C API additions unless separately approved.
- [x] Public header diff reviewed.
- [x] Exported symbols reviewed for shared-library builds.
- [x] Result-code and enum numeric values unchanged.
- [x] Intermediate waypoints remain unsupported.
- [x] Per-section constraints remain unsupported.
- [x] Cloud calculation remains unsupported.
- [x] Python and Rust bindings remain deferred.
- [x] `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  oracle baseline.

## Required Gates

- [x] `git status --short --branch` was clean before release closeout edits.
- [ ] `git status --short --branch` is clean after the release closeout commit
  and before tagging.
- [x] Static CMake release tests pass on Windows clang/Ninja.
- [x] Shared-library release tests pass on Windows clang/Ninja.
- [x] Fixed C++ oracle suite passes and reports `Oracle comparisons passed: 59`
  or more.
- [x] Routine per-DoF random smoke passes with `--random-per-dof 100 --seed 1`.
- [x] Ordinary development random oracle passes with `--random 100000 --seed 1`.
- [x] Ordinary development random oracle passes with `--random 100000 --seed 2`.
- [x] Ordinary development random oracle passes with `--random 100000 --seed 41`.
- [x] Per-DoF development random oracle passes with
  `--random-per-dof 100000 --seed 1`.
- [x] Release random oracle passes with `--random 1000000 --seed 1`.
- [x] Windows release performance benchmark average ratio is `<= 1.5`.
- [ ] Linux release performance benchmark average ratio is `<= 1.5`.
- [ ] GitHub Actions push CI passes for the release evidence commit.
- [ ] GitHub Actions manual release random oracle passes with
  `release_random=true`.
- [ ] Linux Clang ASan+UBSan passes.
- [ ] Linux Valgrind passes.
- [x] Installed CMake consumer smoke test passes.
- [ ] Linux pkg-config consumer smoke test passes.
- [ ] Unix shared install-tree consumer CTest
  `ruckig_c_shared_install_consumer` passes where `pkg-config` is available.
- [x] Windows static/manual link path is reviewed or smoke-tested.
- [x] Windows DLL consumer path is reviewed or smoke-tested.
- [ ] Linux shared install-tree consumer path is reviewed or smoke-tested.
- [ ] `v0.2.1` annotated tag points at the final release evidence commit.
- [ ] GitHub Release `ruckig_c 0.2.1` is published.

## Evidence To Record

```text
Final release commit:
To be recorded after the release closeout commit is created.
Annotated tag object:
Tag target commit:
Push CI run id:
Push CI URL:
Push CI conclusion:
Manual release random run id:
Manual release random URL:
Manual release random conclusion:
Windows static CTest result:
100% tests passed, 0 tests failed out of 19.
Windows shared CTest result:
100% tests passed, 0 tests failed out of 18.
Fixed oracle suite result:
Oracle comparisons passed: 59.
Ordinary random seed 1 result:
Oracle comparisons passed: 59; Random oracle comparisons passed: 100000 seed 1.
Ordinary random seed 2 result:
Oracle comparisons passed: 59; Random oracle comparisons passed: 100000 seed 2.
Ordinary random seed 41 result:
Oracle comparisons passed: 59; Random oracle comparisons passed: 100000 seed 41.
Per-DoF random seed 1 result:
Oracle comparisons passed: 59; Random per-DoF oracle comparisons passed: 100000 seed 1.
Release random oracle result:
100% tests passed, 0 tests failed out of 1; `--random 1000000 --seed 1` completed in 340.14 seconds.
Windows performance result:
average_ratio_c_over_oracle: 0.328289; threshold: 1.5.
Linux performance result:
Pending final push CI Linux performance artifact.
Exported symbol review result:
Windows DLL exports reviewed with `llvm-readobj --coff-exports`; 66 public `ruckig_*` exports found, including lifecycle, input, output, trajectory, validation, calculate, update, reset, and per-DoF APIs. Public header diff from `v0.2.0` contains version macro changes and existing unsupported-scope comment cleanup only; no public functions, enum values, or result-code values changed.
GitHub Release URL:
Pending GitHub Release publication.
```

## Performance Template

```text
Source:
Command:
OS:
CPU identifier:
C compiler:
C++ compiler:
CMake build type:
Generator:
Samples:
Seed:
C average ns:
C p99 ns:
C worst ns:
Oracle average ns:
Oracle p99 ns:
Oracle worst ns:
Average C/oracle ratio:
Release threshold:
Result:
```
