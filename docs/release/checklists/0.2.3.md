# Ruckig C 0.2.3 Release Checklist

This checklist is the future `v0.2.3` patch-release evidence template. It does
not claim that `v0.2.3` is released. All release evidence must be rerun from the
final `0.2.3` release candidate commit before tagging.

## Scope

- [ ] Final commit hash recorded.
- [ ] Version is `0.2.3` in `CMakeLists.txt` if a `v0.2.3` tag is being cut.
- [ ] Version macros in `include/ruckig_c/ruckig.h` are `0.2.3` if a
  `v0.2.3` tag is being cut.
- [ ] `CHANGELOG.md` has a dated `0.2.3` release entry.
- [ ] No public C API additions unless separately approved.
- [ ] Public header diff reviewed against `v0.2.2`.
- [ ] Linux exported-symbol baseline diff reviewed against
  `docs/abi/v0.2.2/linux-symbols.txt`.
- [ ] Windows exported-symbol baseline diff reviewed against
  `docs/abi/v0.2.2/windows-symbols.txt`.
- [ ] Result-code and enum numeric values unchanged.
- [ ] No unintended public API additions.
- [ ] Intermediate waypoints remain unsupported.
- [ ] Per-section constraints remain unsupported.
- [ ] Cloud calculation remains unsupported.
- [ ] Python binding implementation and Rust bindings remain deferred.
- [ ] `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  oracle baseline.

## Required Gates

- [ ] `git status --short --branch` is clean before release closeout edits.
- [ ] `git status --short --branch` is clean after the release closeout commit
  and before tagging.
- [ ] Static CMake release tests pass on Windows clang/Ninja.
- [ ] Shared-library release tests pass on Windows clang/Ninja.
- [ ] Fixed C++ oracle suite passes and reports the final case count.
- [ ] Routine per-DoF random smoke passes with `--random-per-dof 100 --seed 1`.
- [ ] Ordinary development random oracle passes with `--random 100000 --seed 1`.
- [ ] Ordinary development random oracle passes with `--random 100000 --seed 2`.
- [ ] Ordinary development random oracle passes with `--random 100000 --seed 41`.
- [ ] Per-DoF development random oracle passes with
  `--random-per-dof 100000 --seed 1`.
- [ ] Release random oracle passes with `--random 1000000 --seed 1`.
- [ ] Windows release performance benchmark average ratio is `<= 1.5`.
- [ ] Linux release performance benchmark average ratio is `<= 1.5`.
- [ ] Windows performance trend is compared against the `0.2.2` Windows
  release baseline.
- [ ] Linux performance trend is compared against the `0.2.2` Linux release
  baseline.
- [ ] GitHub Actions push CI passes for the release evidence commit.
- [ ] GitHub Actions manual release random oracle passes with
  `release_random=true`.
- [ ] Linux Clang ASan+UBSan passes.
- [ ] Linux Valgrind passes.
- [ ] Installed CMake consumer smoke test passes.
- [ ] Linux pkg-config consumer smoke test passes.
- [ ] Unix shared install-tree consumer CTest
  `ruckig_c_shared_install_consumer` passes where `pkg-config` is available.
- [ ] Windows manual static consumer smoke test
  `ruckig_c_windows_manual_static_consumer` passes where enabled.
- [ ] Windows DLL consumer smoke test `ruckig_c_windows_dll_consumer` passes
  where enabled.
- [ ] Shared build target `ruckig_c_exported_symbols` produces a review
  artifact.
- [ ] Shared build target `ruckig_c_compare_exported_symbols` produces a
  non-strict ABI diff artifact.
- [ ] GitHub Actions `Linux exported symbols` and `Windows exported symbols`
  jobs upload current export and baseline diff artifacts.
- [ ] `v0.2.3` annotated tag points at the final release evidence commit.
- [ ] GitHub Release `ruckig_c 0.2.3` is published.

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
Windows/Linux performance trend versus 0.2.2:
Consumer automation result:
Linux exported-symbol artifact:
Windows exported-symbol artifact:
Linux exported-symbol diff artifact:
Windows exported-symbol diff artifact:
Exported symbol review result:
Public header diff review result:
Enum/result-code review result:
GitHub Release URL:
```

## ABI Review Commands

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c diff v0.2.2 -- include/ruckig_c/ruckig.h
cmake --build build_release_check_shared --target ruckig_c_exported_symbols
cmake --build build_release_check_shared --target ruckig_c_compare_exported_symbols
```

The comparison target is warning/evidence only in `0.2.3`. It must not be used
to silently approve public ABI drift; review any added or removed symbols before
release.

The generated review artifacts are expected under the build tree, for example:

```text
build_release_check_shared/artifacts/abi/0.2.3/windows-exports.txt
build_release_check_shared/artifacts/abi/0.2.3/windows-export-diff.txt
build-shared/artifacts/abi/0.2.3/linux-exports.txt
build-shared/artifacts/abi/0.2.3/linux-export-diff.txt
```

## Preparation Evidence On Main

This is not final `v0.2.3` release evidence. It records that the maintenance
queue currently passes the core local gates after the initial `0.2.3`
preparation work:

```text
Static CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Shared CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Fixed oracle suite: Oracle comparisons passed: 70.
Random oracle seed 1: Random oracle comparisons passed: 100000 seed 1.
Random oracle seed 2: Random oracle comparisons passed: 100000 seed 2.
Random oracle seed 41: Random oracle comparisons passed: 100000 seed 41.
Per-DoF random seed 1: Random per-DoF oracle comparisons passed: 100000 seed 1.
Windows performance preparation ratio: 1.41946, threshold 1.5.
Windows exported-symbol baseline diff: 66 current symbols, 66 baseline symbols, 0 added, 0 removed.
```
