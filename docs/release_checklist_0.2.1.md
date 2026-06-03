# Ruckig C 0.2.1 Release Checklist

This checklist is the patch-release evidence template for `v0.2.1`. Fill every
item from the final release commit before tagging.

## Scope

- [ ] Final commit hash recorded.
- [ ] Version is `0.2.1` in `CMakeLists.txt` if a `v0.2.1` tag is being cut.
- [ ] Version macros in `include/ruckig_c/ruckig.h` are `0.2.1` if a `v0.2.1`
  tag is being cut.
- [ ] `CHANGELOG.md` has a `0.2.1` release entry.
- [ ] No public C API additions unless separately approved.
- [ ] Public header diff reviewed.
- [ ] Exported symbols reviewed for shared-library builds.
- [ ] Result-code and enum numeric values unchanged.
- [ ] Intermediate waypoints remain unsupported.
- [ ] Per-section constraints remain unsupported.
- [ ] Cloud calculation remains unsupported.
- [ ] Python and Rust bindings remain deferred.
- [ ] `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  oracle baseline.

## Required Gates

- [ ] `git status --short --branch` is clean before final local gates.
- [ ] Static CMake release tests pass on Windows clang/Ninja.
- [ ] Shared-library release tests pass on Windows clang/Ninja.
- [ ] Fixed C++ oracle suite passes and reports `Oracle comparisons passed: 59`
  or more.
- [ ] Routine per-DoF random smoke passes with `--random-per-dof 100 --seed 1`.
- [ ] Ordinary development random oracle passes with `--random 100000 --seed 1`.
- [ ] Ordinary development random oracle passes with `--random 100000 --seed 2`.
- [ ] Ordinary development random oracle passes with `--random 100000 --seed 41`.
- [ ] Per-DoF development random oracle passes with
  `--random-per-dof 100000 --seed 1`.
- [ ] Release random oracle passes with `--random 1000000 --seed 1`.
- [ ] Windows release performance benchmark average ratio is `<= 1.5`.
- [ ] Linux release performance benchmark average ratio is `<= 1.5`.
- [ ] GitHub Actions push CI passes for the release evidence commit.
- [ ] GitHub Actions manual release random oracle passes with
  `release_random=true`.
- [ ] Linux Clang ASan+UBSan passes.
- [ ] Linux Valgrind passes.
- [ ] Installed CMake consumer smoke test passes.
- [ ] Linux pkg-config consumer smoke test passes.
- [ ] Unix shared install-tree consumer CTest
  `ruckig_c_shared_install_consumer` passes where `pkg-config` is available.
- [ ] Windows static/manual link path is reviewed or smoke-tested.
- [ ] Windows DLL consumer path is reviewed or smoke-tested.
- [ ] Linux shared install-tree consumer path is reviewed or smoke-tested.
- [ ] `v0.2.1` annotated tag points at the final release evidence commit.
- [ ] GitHub Release `ruckig_c 0.2.1` is published.

## Evidence To Record

```text
Final release commit:
Annotated tag object:
Tag target commit:
Push CI run id:
Push CI URL:
Push CI conclusion:
Manual release random run id:
Manual release random URL:
Manual release random conclusion:
Windows static CTest result:
Windows shared CTest result:
Fixed oracle suite result:
Ordinary random seed 1 result:
Ordinary random seed 2 result:
Ordinary random seed 41 result:
Per-DoF random seed 1 result:
Release random oracle result:
Windows performance result:
Linux performance result:
Exported symbol review result:
GitHub Release URL:
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
