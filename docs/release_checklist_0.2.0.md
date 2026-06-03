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

- [ ] `git status --short --branch` is clean before the final local gate.
- [ ] Static CMake release tests pass on Windows clang/Ninja.
- [ ] Shared-library release tests pass on Windows clang/Ninja.
- [ ] Fixed C++ oracle suite passes.
- [ ] Development random oracle passes with `--random 100000 --seed 2`.
- [ ] Development random oracle passes with `--random 100000 --seed 41`.
- [ ] Release random oracle passes with `--random 1000000 --seed 1`.
- [ ] Windows release performance benchmark average ratio is `<= 1.5`.
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

### Local Windows Static Release

Command:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
Pending final release gate.
```

### Local Windows Shared Release

Command:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
Pending final release gate.
```

### Development Random Oracle

Commands:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Result:

```text
Pending final release gate.
```

### Release Random Oracle

Command:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
Pending final release gate.
```

### Windows Performance

Command:

```powershell
.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1
```

Result:

```text
Pending final release gate.
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
