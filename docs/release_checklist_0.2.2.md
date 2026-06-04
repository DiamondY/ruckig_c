# Ruckig C 0.2.2 Release Checklist

This checklist records the `v0.2.2` patch-release closeout evidence. Local
release-candidate gates were rerun after the version bump and dated changelog
entry. Remote CI, manual workflow, tag, and GitHub Release evidence has been
recorded for the final release commit.

## Scope

- [x] Final commit hash recorded.
- [x] Version is `0.2.2` in `CMakeLists.txt` if a `v0.2.2` tag is being cut.
- [x] Version macros in `include/ruckig_c/ruckig.h` are `0.2.2` if a
  `v0.2.2` tag is being cut.
- [x] `CHANGELOG.md` has a dated `0.2.2` release entry.
- [x] No public C API additions unless separately approved.
- [x] Public header diff reviewed against `v0.2.1`.
- [x] Linux exported-symbol snapshot reviewed.
- [x] Windows exported-symbol snapshot reviewed.
- [x] Result-code and enum numeric values unchanged.
- [x] No unintended public API additions.
- [x] Intermediate waypoints remain unsupported.
- [x] Per-section constraints remain unsupported.
- [x] Cloud calculation remains unsupported.
- [x] Python and Rust bindings remain deferred.
- [x] `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  oracle baseline.

## Required Gates

- [x] `git status --short --branch` is clean before release closeout edits.
- [x] `git status --short --branch` is clean after the release closeout commit
  and before tagging.
- [x] Static CMake release tests pass on Windows clang/Ninja.
- [x] Shared-library release tests pass on Windows clang/Ninja.
- [x] Fixed C++ oracle suite passes and reports the final case count.
- [x] Routine per-DoF random smoke passes with `--random-per-dof 100 --seed 1`.
- [x] Ordinary development random oracle passes with `--random 100000 --seed 1`.
- [x] Ordinary development random oracle passes with `--random 100000 --seed 2`.
- [x] Ordinary development random oracle passes with `--random 100000 --seed 41`.
- [x] Per-DoF development random oracle passes with
  `--random-per-dof 100000 --seed 1`.
- [x] Release random oracle passes with `--random 1000000 --seed 1`.
- [x] Windows release performance benchmark average ratio is `<= 1.5`.
- [x] Linux release performance benchmark average ratio is `<= 1.5`.
- [x] Windows performance trend is compared against the `0.2.1` Windows
  release baseline.
- [x] Linux performance trend is compared against the `0.2.1` Linux release
  baseline.
- [x] GitHub Actions push CI passes for the release evidence commit.
- [x] GitHub Actions manual release random oracle passes with
  `release_random=true`.
- [x] Linux Clang ASan+UBSan passes.
- [x] Linux Valgrind passes.
- [x] Installed CMake consumer smoke test passes.
- [x] Linux pkg-config consumer smoke test passes.
- [x] Unix shared install-tree consumer CTest
  `ruckig_c_shared_install_consumer` passes where `pkg-config` is available.
- [x] Windows manual static consumer smoke test
  `ruckig_c_windows_manual_static_consumer` passes where enabled.
- [x] Windows DLL consumer smoke test `ruckig_c_windows_dll_consumer` passes
  where enabled.
- [x] Shared build exported-symbol target `ruckig_c_exported_symbols` produces
  a review artifact.
- [x] GitHub Actions `Linux exported symbols` and `Windows exported symbols`
  jobs upload ABI review artifacts.
- [x] `v0.2.2` annotated tag points at the final release evidence commit.
- [x] GitHub Release `ruckig_c 0.2.2` is published.

## Evidence To Record

```text
Final release commit:
15c896497fc5973fc19129c6fe59b2fd4da9533f.
Annotated tag object:
49b04e26776c6c787e97fb6223a03240031dd97a.
Tag target commit:
15c896497fc5973fc19129c6fe59b2fd4da9533f.
Push CI run id:
26935069765.
Push CI URL:
https://github.com/DiamondY/ruckig_c/actions/runs/26935069765.
Push CI conclusion:
success.
Manual release random run id:
26935519342.
Manual release random URL:
https://github.com/DiamondY/ruckig_c/actions/runs/26935519342.
Manual release random conclusion:
success.
Windows static CTest result:
100% tests passed, 0 tests failed out of 20.
Windows shared CTest result:
100% tests passed, 0 tests failed out of 20.
Fixed oracle suite result:
Oracle comparisons passed: 64.
Ordinary random seed 1 result:
Oracle comparisons passed: 64; Random oracle comparisons passed: 100000 seed 1.
Ordinary random seed 2 result:
Oracle comparisons passed: 64; Random oracle comparisons passed: 100000 seed 2.
Ordinary random seed 41 result:
Oracle comparisons passed: 64; Random oracle comparisons passed: 100000 seed 41.
Per-DoF random seed 1 result:
Oracle comparisons passed: 64; Random per-DoF oracle comparisons passed: 100000 seed 1.
Release random oracle result:
100% tests passed, 0 tests failed out of 1; `--random 1000000 --seed 1` completed in 50.04 seconds.
Windows performance result:
average_ratio_c_over_oracle: 1.08231; threshold: 1.5.
Linux performance result:
Final push CI artifact `7404574237` and manual workflow artifact `7404750529`; both Linux Clang performance jobs completed successfully and the benchmark program enforces `average_ratio_c_over_oracle <= 1.5`.
Windows/Linux performance trend versus 0.2.1:
Windows local release ratio changed from `0.328289` in `0.2.1` to a final-commit post-publication rerun ratio of `1.39707` in `0.2.2`; both are below the `1.5` threshold. Linux performance artifacts `7404574237` and `7404750529` are final-commit evidence.
Consumer automation result:
Installed CMake consumer, Windows manual static consumer, and Windows DLL consumer passed in local release-check CTest. Linux pkg-config and shared install-tree consumers passed in final push CI.
Linux exported-symbol artifact:
Push CI artifact `7404571215`; manual workflow artifact `7404748851`.
Windows exported-symbol artifact:
`build_release_check_shared/artifacts/abi/0.2.2/windows-exports.txt`.
Linux exported-symbol CI artifact:
7404571215.
Windows exported-symbol CI artifact:
7404578412.
Exported symbol review result:
Windows DLL exports reviewed with `llvm-readobj --coff-exports`; 66 public `ruckig_*` exports found, including lifecycle, input, output, trajectory, validation, calculate, update, reset, and per-DoF APIs.
Public header diff review result:
Public header diff from `v0.2.1` contains only version macro changes from `0.2.1` to `0.2.2`; no public functions, enum values, or result-code values changed.
Enum/result-code review result:
Enum and result-code numeric values unchanged from `v0.2.1`.
GitHub Release URL:
https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.2.
```

## ABI Review Commands

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c diff v0.2.1 -- include/ruckig_c/ruckig.h
cmake --build build_release_check_shared --target ruckig_c_exported_symbols
```

The generated snapshot is written under the build tree, for example:

```text
build_release_check_shared/artifacts/abi/0.2.2/windows-exports.txt
build-shared/artifacts/abi/0.2.2/linux-exports.txt
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
0.2.1 same-platform baseline ratio:
Release threshold:
Result:
```
