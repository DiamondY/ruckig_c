# Ruckig C 0.2.0 Release Checklist

Run these checks from a clean worktree before tagging `v0.2.0`.

## Scope

- [x] Version is `0.2.0` in `CMakeLists.txt`.
- [x] Version macros in `include/ruckig_c/ruckig.h` are `0.2.0`.
- [x] `CHANGELOG.md` has a `0.2.0` release entry.
- [x] `0.2.0` adds per-DoF control-interface overrides.
- [x] `0.2.0` adds per-DoF synchronization overrides.
- [x] Public setters reject `NULL`, count mismatches, and invalid enum values
  with `RUCKIG_ERROR_INVALID_INPUT`.
- [x] Clear functions restore the matching global setter behavior.
- [x] Per-DoF storage is allocated during create; calculate, update, and
  trajectory sampling keep the no-allocation release contract.
- [x] Intermediate waypoints remain unsupported.
- [x] Per-section constraints remain unsupported.
- [x] Cloud calculation remains unsupported.
- [x] Python and Rust bindings remain deferred.
- [x] `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  oracle baseline.

## Required Gates

- [x] `git status --short --branch` is clean before the final local gate.
- [x] Static CMake release tests pass on Windows clang/Ninja.
- [x] Shared-library release tests pass on Windows clang/Ninja.
- [x] Fixed C++ oracle suite passes.
- [x] Development random oracle passes with `--random 100000 --seed 2`.
- [x] Development random oracle passes with `--random 100000 --seed 41`.
- [x] Release random oracle passes with `--random 1000000 --seed 1`.
- [x] Windows release performance benchmark average ratio is `<= 1.5`.
- [ ] GitHub Actions push CI passes for the release evidence commit.
- [ ] GitHub Actions manual release random oracle passes with
  `release_random=true`.
- [ ] Linux Clang ASan+UBSan passes.
- [ ] Linux Valgrind passes.
- [ ] Linux Clang performance benchmark passes and is recorded.
- [ ] Installed CMake consumer smoke test passes.
- [ ] Linux pkg-config consumer smoke test passes.
- [ ] `v0.2.0` annotated tag points at the release evidence commit.
- [ ] GitHub Release `ruckig_c 0.2.0` is published with notes from
  `docs/release_notes_0.2.0.md`.

## Release Evidence

Final release commit:

```text
Resolve from: git rev-parse v0.2.0^{commit}
```

The tag commit cannot be self-recorded inside the same commit that carries this
checklist. Resolve it from the annotated tag after tagging and verify that the
tag target is the evidence commit.

Local release-gate evidence below was recorded on documentation release-closeout
commit `4b04212eba8b793805275702c333ed41cf65de20`. The only later expected
changes before tagging are evidence-document updates and CI metadata; push CI
and the manual release-random workflow must still pass on the final tag target.

### Local Windows Static Release

Command:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 17
```

### Local Windows Shared Release

Command:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 16
```

### Development Random Oracle

Commands:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Result:

```text
Oracle comparisons passed: 48
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 48
Random oracle comparisons passed: 100000 seed 41
```

### Release Random Oracle

Command:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random oracle executed `--random 1000000 --seed 1` and completed in
about 267 seconds.

### Windows Performance

Command:

```powershell
.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1
```

Result:

```text
Ruckig C performance benchmark
samples: 10000
seed: 1
compiler: clang 21.1.8
os: Windows
c_average_ns: 1345.11
c_p99_ns: 9100
c_worst_ns: 20200
oracle_average_ns: 4691.73
oracle_p99_ns: 20000
oracle_worst_ns: 46500
average_ratio_c_over_oracle: 0.286698
release_threshold_average_ratio: 1.5
```

### GitHub Actions

- Push CI run id: pending.
- Push CI run URL: pending.
- Push CI conclusion: pending.
- Manual release random run id: pending.
- Manual release random run URL: pending.
- Manual release random conclusion: pending.

Expected successful jobs:

- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Manual release random oracle`

### Linux Performance

Copy the final `linux-performance.txt` artifact from the release push CI into
`docs/performance_report.md` before tagging.

## Release Commands

```powershell
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c status --short --branch
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c tag -a v0.2.0 -m "Release ruckig_c 0.2.0"
git -c safe.directory=E:/Yww/DownLoad/source/ruckig_c push ruckig_c v0.2.0
```

Use `docs/release_notes_0.2.0.md` as the GitHub Release notes source.
