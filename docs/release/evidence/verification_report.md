# Ruckig C Verification Report

This report records verification runs for the C rewrite. Commands are run from
the repository root unless noted otherwise.

## 2026-06-03 Windows Clang Verification

Environment:

- OS: Windows
- Compiler: clang 21.1.8, target `x86_64-pc-windows-msvc`
- CMake: 4.1.0
- Ninja: Visual Studio bundled Ninja 1.11.0 at
  `C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe`
- CPU identifier available without elevated WMI access:
  `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`

### CMake/Ninja Static Build

Configure and build command:

```powershell
cmake -S . -B build_ninja -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=ON
cmake --build build_ninja --config Release
```

CTest command excluding the long release random test:

```powershell
ctest --test-dir build_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 13
```

This covers:

- C unit tests.
- Source-level allocation audit.
- C and C++ header compile/link tests.
- All five C examples.
- Fixed oracle comparisons.
- Random smoke oracle test.
- Development random oracle test with `100000` trajectories and seed `1`.
- Performance benchmark CTest threshold.

Release random CTest command:

```powershell
ctest --test-dir build_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random run executed `--random 1000000 --seed 1` and completed in
about 263 seconds.

### CMake/Ninja Shared Build

Configure and build command:

```powershell
cmake -S . -B build_ninja_shared -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build build_ninja_shared --config Release
```

CTest command excluding the long release random test:

```powershell
ctest --test-dir build_ninja_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 12
```

This verifies the shared-library export/import path for C unit tests, C/C++
header consumers, the source-level allocation audit, all five examples, fixed
oracle comparisons, random smoke, and the `100000` trajectory development
random oracle test.

### Optional Calculation-Duration Build

Configure, build, and test command:

```powershell
cmake -S . -B build_ninja_duration -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DRUCKIG_C_ENABLE_CALCULATION_DURATION=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build build_ninja_duration --config Release
ctest --test-dir build_ninja_duration --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 9
```

This verifies the optional `RUCKIG_C_ENABLE_CALCULATION_DURATION` path that
records `ruckig_update` calculation duration in microseconds for
`ruckig_output_get_calculation_duration`.

### Windows clang-cl C-Only Build

Configure, build, and test command:

```powershell
cmake -S . -B build_ninja_clangcl -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang-cl.exe" -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build build_ninja_clangcl --config Release
ctest --test-dir build_ninja_clangcl --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 9
```

This verifies the pure C library, C unit tests, C/C++ public header consumers,
allocation audit, and all C examples with the MSVC-like `clang-cl` driver.

### Direct Clang Reference Runs

### C Unit Tests

Command:

```powershell
clang -std=c99 -Wall -Wextra -Wpedantic -DRUCKIG_C_STATIC_DEFINE -Iinclude -Isrc src\ruckig_c\alloc.c src\ruckig_c\block.c src\ruckig_c\brake.c src\ruckig_c\input.c src\ruckig_c\output.c src\ruckig_c\position_first_step1.c src\ruckig_c\position_first_step2.c src\ruckig_c\position_second_step1.c src\ruckig_c\position_second_step2.c src\ruckig_c\position_third_step1.c src\ruckig_c\position_third_step2.c src\ruckig_c\profile.c src\ruckig_c\roots.c src\ruckig_c\ruckig.c src\ruckig_c\trajectory.c src\ruckig_c\utils.c src\ruckig_c\velocity_second_step1.c src\ruckig_c\velocity_second_step2.c src\ruckig_c\velocity_third_step1.c src\ruckig_c\velocity_third_step2.c test\c\test_api.c test\c\test_brake.c test\c\test_profile.c test\c\test_roots.c test\c\test_utils.c -o build\ruckig_c_tests_direct.exe
.\build\ruckig_c_tests_direct.exe
```

Result: passed.

The final pass after adding `RUCKIG_C_API` shared-library annotations used the
same commands with `RUCKIG_C_STATIC_DEFINE`; C unit tests, C header compile, C++
header compile, fixed oracle, and `--random 100000 --seed 1` all passed.

### Header Compile Tests

Commands:

```powershell
clang -std=c99 -Wall -Wextra -Wpedantic -DRUCKIG_C_STATIC_DEFINE -Iinclude test\c\header_compile.c -c -o build\header_compile_c.o
clang++ -std=c++11 -Wall -Wextra -DRUCKIG_C_STATIC_DEFINE -Iinclude test\c\header_compile.cpp -c -o build\header_compile_cpp.o
```

Result: passed.

### Static And Shared Libraries

Static-library direct build (source/object globs shown as shorthand for the
same source list used in the C unit command above):

```powershell
clang -std=c99 -O2 -DNDEBUG -DRUCKIG_C_STATIC_DEFINE -Wall -Wextra -Wpedantic -Iinclude -Isrc -c src\ruckig_c\*.c
llvm-ar rcs build\ruckig_c.lib build\lib_obj\*.o
clang -std=c99 -Wall -Wextra -Wpedantic -Iinclude examples\c\02_position_offline.c build\ruckig_c.lib -o build\example_static_link.exe
.\build\example_static_link.exe
```

Result: static library linked and ran successfully.

Shared-library direct build (source/object globs shown as shorthand for the
same source list used in the C unit command above):

```powershell
clang -std=c99 -O2 -DNDEBUG -DRUCKIG_C_BUILDING_LIBRARY -Wall -Wextra -Wpedantic -Iinclude -Isrc -c src\ruckig_c\*.c
clang -shared build\dll_obj\*.o "-Wl,/implib:build\ruckig_c_import.lib" -o build\ruckig_c.dll
clang -std=c99 -Wall -Wextra -Wpedantic -Iinclude examples\c\02_position_offline.c build\ruckig_c_import.lib -o build\example_shared_link.exe
.\build\example_shared_link.exe
```

Result: DLL and import library were generated, public symbols such as
`ruckig_create`, `ruckig_calculate`, and `ruckig_trajectory_at_time` were
exported, and the shared-link example ran successfully.

### Fixed Oracle Suite

Command:

```powershell
.\build\ruckig_c_oracle_tests_direct.exe
```

Result:

```text
Oracle comparisons passed: 38
```

### Development Random Oracle

Commands:

```powershell
.\build\ruckig_c_oracle_tests_direct.exe --random 100000 --seed 1
.\build\ruckig_c_oracle_tests_direct.exe --random 100000 --seed 2
.\build\ruckig_c_oracle_tests_direct.exe --random 100000 --seed 41
```

Results:

```text
Random oracle comparisons passed: 100000 seed 1
Random oracle comparisons passed: 100000 seed 2
Random oracle comparisons passed: 100000 seed 41
```

### Release Random Oracle

Command:

```powershell
.\build\ruckig_c_oracle_tests_direct.exe --random 1000000 --seed 1
```

Result:

```text
Oracle comparisons passed: 38
Random oracle comparisons passed: 1000000 seed 1
```

The CMake test manifest contains `ruckig_c_oracle_random_release`, which ran
the same `--random 1000000 --seed 1` command successfully in `build_ninja`.

The fixed oracle suite includes regressions for:

- third-order position Step2 candidate ordering,
- third-order position `NONE_UDDU` blocked-interval candidate collection,
- `Synchronization::None` avoiding blocked-duration synchronization changes.

### Examples

All C examples were compiled directly with clang and run:

- `examples/c/01_position.c`
- `examples/c/02_position_offline.c`
- `examples/c/05_velocity.c`
- `examples/c/06_stop.c`
- `examples/c/07_minimum_duration.c`

Result: passed.

### Allocation Checks

The C unit suite includes an internal allocation-counter check and a forbidden
allocation guard for representative `ruckig_calculate`, `ruckig_update`, and
`ruckig_trajectory_at_time` paths, plus root solver calls
(`ruckig_solve_cubic`, `ruckig_solve_quart_monic`, and
`ruckig_shrink_interval`).

Result: passed as part of `ruckig_c_tests_direct.exe`.

CTest also runs `ruckig_c_allocation_audit`, a source-level audit that fails if
any file under `src/ruckig_c` other than `alloc.c` directly calls
`malloc`, `calloc`, `realloc`, or `free`.

Result: passed in the static, shared, and sanitizer CMake/CTest runs.

Additional hardening: a platform-level symbol-wrapper audit is useful for CI on
platforms that support it. The current Windows release-candidate evidence
combines internal runtime allocation counters with source-level enforcement
against raw heap calls in the C rewrite.

### Sanitizers

CMake now exposes:

- `RUCKIG_C_ENABLE_ASAN`
- `RUCKIG_C_ENABLE_UBSAN`

Windows clang sanitizer CMake configure/build command:

```powershell
cmake -S . -B build_ninja_san -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DRUCKIG_C_ENABLE_ASAN=ON -DRUCKIG_C_ENABLE_UBSAN=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build build_ninja_san --config Debug
```

The sanitizer runtime DLL must be visible to the test process on this
workstation:

```powershell
$env:PATH='D:\Program Files\LLVM\lib\clang\21\lib\windows;' + $env:PATH
ctest --test-dir build_ninja_san --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 9
```

The sanitizer CTest run covers `ruckig_c_tests`, the source-level allocation
audit, the C/C++ header compile-link tests, and all five C examples. Without
the LLVM sanitizer runtime directory in `PATH`, the test executable exits with
`-1073741515`, which is a Windows runtime loading failure rather than a
sanitizer diagnostic.

Direct Windows clang sanitizer build command:

```powershell
clang -std=c99 "-fsanitize=address,undefined" -g -O1 -Wall -Wextra -Wpedantic -Iinclude -Isrc ... -o build\ruckig_c_tests_asan.exe
```

Build result: succeeded. Run result is equivalent to the CMake sanitizer test
above when the LLVM sanitizer runtime directory is present in `PATH`.

A Linux/Clang ASan+UBSan run remains useful for CI/release portability, but this
workstation has no configured WSL distribution available for Linux execution.

### Performance

See `docs/release/evidence/performance_report.md`.

## 2026-06-03 Release-Readiness Follow-Up

Environment:

- OS: Windows
- Compiler: clang 21.1.8, target `x86_64-pc-windows-msvc`
- Generator: Ninja via Visual Studio bundled Ninja

Static build with oracle and performance tests:

```powershell
cmake -S . -B build_release_check_ninja -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=ON
cmake --build build_release_check_ninja --config Release
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 14
```

This run includes the installed CMake consumer smoke test added for the
`0.1.0` release checklist.

Shared-library build with oracle tests:

```powershell
cmake -S . -B build_release_check_shared -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build build_release_check_shared --config Release
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 13
```

Manual release random oracle:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random run executed `--random 1000000 --seed 1` and completed in
about 262 seconds.

## 2026-06-03 Final Local Release Closeout

Environment:

- OS: Windows
- Compiler/build directories: existing Windows clang/Ninja release check
  directories `build_release_check_ninja` and `build_release_check_shared`.
- Pre-tag commit before documentation-only closeout:
  `86056d2d527396f8758faf841b2d3cad9133e2ad`.

Static release CTest rerun excluding the long release random test:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 14
```

Shared-library release CTest rerun excluding the long release random test:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 13
```

Manual release random oracle rerun:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random rerun executed `--random 1000000 --seed 1` and completed in
about 262 seconds.

## 2026-06-03 0.1.x Stability Follow-Up

This pass verifies the `0.1.1` stability queue additions: expanded fixed oracle
regressions, C API diagnostics coverage, minimal examples, and release-process
documentation.

Static release CTest excluding the long release random test:

```powershell
cmake --build build_release_check_ninja --config Release
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 16
```

This includes the two new C examples:

- `example_ruckig_c_00_minimal_offline`
- `example_ruckig_c_03_minimal_online`

The fixed oracle suite now reports:

```text
Oracle comparisons passed: 44
```

Shared-library release CTest excluding the long release random test:

```powershell
cmake --build build_release_check_shared --config Release
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 15
```

Additional deterministic random oracle runs:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Results:

```text
Random oracle comparisons passed: 100000 seed 2
Random oracle comparisons passed: 100000 seed 41
```

Manual release random oracle rerun:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random rerun executed `--random 1000000 --seed 1` and completed in
about 260 seconds.

## 2026-06-03 0.2.0 Per-DoF Override Implementation

This pass verifies the `0.2.0` per-DoF control-interface and synchronization
override implementation. It includes public C API setters/clearers, mixed
position/velocity control dispatch, mixed `Time`/`None` synchronization
dispatch, fixed C++ oracle cases, C API boundary tests, no-allocation coverage
with per-DoF settings enabled, and the new C example.

Static release CTest excluding the long release random test:

```powershell
cmake --build build_release_check_ninja --config Release
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 17
```

This includes the new C example:

- `example_ruckig_c_04_per_dof_override`

The fixed oracle suite now reports:

```text
Oracle comparisons passed: 48
```

Shared-library release CTest excluding the long release random test:

```powershell
cmake --build build_release_check_shared --config Release
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 16
```

Additional deterministic random oracle runs:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Results:

```text
Oracle comparisons passed: 48
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 48
Random oracle comparisons passed: 100000 seed 41
```

Manual release random oracle rerun:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random rerun executed `--random 1000000 --seed 1` and completed in
about 267 seconds.

GitHub Actions push CI for the same commit:

- Commit: `bc72bcbed4560694c32c4c6d42136081654b2961`
- Run id: `26883640106`
- Run URL: `https://github.com/DiamondY/ruckig_c/actions/runs/26883640106`
- Run status: `completed`
- Run conclusion: `success`

Successful jobs:

- `Linux Clang ASan UBSan`
- `Windows clang-cl C-only`
- `Linux Valgrind`
- `macOS Clang C-only`
- `Linux GCC C-only`
- `Linux Clang performance`
- `Windows clang oracle`
- `Linux Clang oracle`

The `Manual release random oracle` job was skipped in the push-triggered CI
run, as expected. The `0.2.0` release-random evidence above comes from the local
Windows clang/Ninja run of `--random 1000000 --seed 1`.

## 2026-06-03 0.2.0 Release Closeout Local Gate

This pass verifies the `0.2.0` release documentation closeout commit before the
final CI/tag gate.

- Commit: `4b04212eba8b793805275702c333ed41cf65de20`
- Branch state before local gate: clean, `main...ruckig_c/main [ahead 1]`

Static release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 17
```

Shared-library release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 16
```

Additional deterministic random oracle runs:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Results:

```text
Oracle comparisons passed: 48
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 48
Random oracle comparisons passed: 100000 seed 41
```

Manual release random oracle rerun:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random rerun executed `--random 1000000 --seed 1` and completed in
about 267 seconds.

Windows release performance:

```powershell
.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1
```

Result:

```text
average_ratio_c_over_oracle: 0.286698
release_threshold_average_ratio: 1.5
```

Final release evidence:

- Tag target commit: `7ae8d4c2d05c6ea02547d6387de207df59826650`.
- Annotated tag object: `c2da0df996a447fea46084a0a34133c6c6aa3931`.
- Push CI run id: `26887345035`, conclusion `success`.
- Manual release random workflow run id: `26887625326`, conclusion `success`.
- GitHub Release:
  `https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.0`.

The push CI and manual release random workflow both ran against the final tag
target commit. The push CI manual-release-random job was skipped as expected;
the workflow-dispatch run executed `Manual release random oracle` successfully.

## 2026-06-03 0.2.x Post-Release per-DoF Hardening

This pass verifies the first post-`v0.2.0` hardening changes. It does not move
or retag `v0.2.0`; the changes are intended for the following `0.2.x`
maintenance line.

Scope covered:

- Additional fixed per-DoF oracle cases for `Phase`, `TimeIfNecessary`,
  discrete `None`/`Time`, disabled DoFs, and mixed finite/infinite
  acceleration/jerk with mixed control interfaces.
- Controlled random per-DoF oracle mode:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 53
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Existing random oracle gate:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 53
Random oracle comparisons passed: 100000 seed 1
```

The per-DoF random mode preserves the existing `--random N --seed S` behavior.
It compares result codes, durations, independent durations, trajectory samples,
position extrema, and update-loop behavior. It intentionally leaves
`get_first_time_at_position` boundary queries to the fixed suite because those
queries can be sensitive to zero-duration segment and duplicate-position
boundary choices while the sampled trajectories remain equivalent.

Static release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 18
```

This includes the new `example_ruckig_c_08_per_dof_online` CTest entry.

Shared-library release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 17
```

## 0.2.1 Release Preparation

This section records initial `0.2.1` preparation evidence on `main`. It is not
final release evidence; all gates must be rerun from the final `0.2.1` release
candidate commit before tagging.

The `0.2.1` preparation queue adds a routine per-DoF oracle smoke CTest:

```text
ruckig_c_oracle_tests --random-per-dof 100 --seed 1
```

This smoke test is intended to catch routine per-DoF regressions in oracle CI.
It does not replace the manual/development
`ruckig_c_oracle_tests --random-per-dof 100000 --seed 1` gate or the
`--random 1000000 --seed 1` release random oracle.

Fixed suite and smoke checks:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100 --seed 1
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100 --seed 1
```

Result:

```text
Oracle comparisons passed: 59
Oracle comparisons passed: 59
Random oracle comparisons passed: 100 seed 1
Oracle comparisons passed: 59
Random per-DoF oracle comparisons passed: 100 seed 1
```

The fixed suite now includes additional `0.2.1` regression coverage for
large-magnitude positions, tiny nonzero limits, large discrete minimum
duration, mixed first/second/third-order per-DoF inputs, explicit
first-time-at-position boundaries, and disabled DoF per-DoF overrides under
discrete duration.

Static release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 19
```

Shared-library release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 18
```

Development random oracle seeds:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Result:

```text
Oracle comparisons passed: 59
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 59
Random oracle comparisons passed: 100000 seed 41
```

Per-DoF development random oracle:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 59
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Windows performance smoke:

```powershell
.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1
```

Result:

```text
average_ratio_c_over_oracle: 0.28883
release_threshold_average_ratio: 1.5
```

Release random oracle smoke from the same preparation worktree:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random oracle executed `--random 1000000 --seed 1` and completed in
about 340 seconds. Final `0.2.1` release evidence must still be rerun from the
tag candidate commit.

## 2026-06-04 0.2.1 Local Release Closeout

This pass verifies the local `0.2.1` release closeout after bumping the project
version to `0.2.1` and converting the changelog entry to a dated release entry.
Remote CI, manual workflow-dispatch, tag, and GitHub Release evidence must be
recorded after the release closeout commit is pushed.

Environment:

- OS: Windows.
- Compiler: clang 21.1.8, target `x86_64-pc-windows-msvc`.
- CMake build directories: `build_release_check_ninja` and
  `build_release_check_shared`.
- Previous commit before closeout edits:
  `134637212a09ae232e9cf4f4559471b82d1bc98d`.

Static release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 19
```

Shared-library release CTest excluding the long release random test:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 18
```

Fixed oracle suite:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe
```

Result:

```text
Oracle comparisons passed: 59
```

Development random oracle runs:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 1
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Results:

```text
Oracle comparisons passed: 59
Random oracle comparisons passed: 100000 seed 1
Oracle comparisons passed: 59
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 59
Random oracle comparisons passed: 100000 seed 41
```

Per-DoF development random oracle:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 59
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Release random oracle:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random oracle executed `--random 1000000 --seed 1` and completed in
340.14 seconds.

Windows manual static consumer smoke:

```powershell
clang -std=c99 -DRUCKIG_C_STATIC_DEFINE -I include -c examples\c\00_minimal_offline.c -o build_release_check_ninja\manual_static_consumer.obj
clang -nostartfiles -nostdlib -fuse-ld=lld-link build_release_check_ninja\manual_static_consumer.obj build_release_check_ninja\ruckig_c.lib -Xlinker /subsystem:console -o build_release_check_ninja\manual_static_consumer.exe -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -loldnames
.\build_release_check_ninja\manual_static_consumer.exe
```

Result:

```text
duration 1.000000 position 0.500000
```

Windows DLL consumer smoke:

```powershell
clang -std=c99 -I include examples\c\00_minimal_offline.c build_release_check_shared\ruckig_c.lib -o build_release_check_shared\dll_consumer.exe
$env:PATH=(Resolve-Path build_release_check_shared).Path + ';' + $env:PATH
.\build_release_check_shared\dll_consumer.exe
```

Result:

```text
duration 1.000000 position 0.500000
```

Windows DLL exported symbols were reviewed with:

```powershell
llvm-readobj --coff-exports build_release_check_shared\ruckig_c.dll
```

Result: 66 public `ruckig_*` exports were found, including lifecycle, input,
output, trajectory, validation, calculate, update, reset, and per-DoF APIs.
The public header diff from `v0.2.0` contains version macro changes and the
existing unsupported-scope comment cleanup only; no public functions, enum
values, or result-code values changed.

## 2026-06-04 0.2.1 Release Publication

The `v0.2.1` release was published after the local closeout gate, push CI, and
manual release-random workflow succeeded.

Final release evidence:

- Tag target commit: `dc09eb938484b407415e1d6b4f59a2242c18ba8b`.
- Annotated tag object: `64909d4b9192116713ca961c228c499ca4429931`.
- Push CI run id: `26898545004`, conclusion `success`.
- Push CI URL: `https://github.com/DiamondY/ruckig_c/actions/runs/26898545004`.
- Manual release random workflow run id: `26898859059`, conclusion `success`.
- Manual release random URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26898859059`.
- GitHub Release:
  `https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.1`.

Successful push CI jobs:

- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`

Successful manual release random workflow jobs:

- `Manual release random oracle`
- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`

The manual workflow Linux performance artifact id is `7390788705`. The push CI
Linux performance artifact id is `7390652575`. Both workflows ran against
commit `dc09eb938484b407415e1d6b4f59a2242c18ba8b`.

## 2026-06-04 0.2.2 Preparation Smoke

This pass verifies the initial `0.2.2 - Unreleased` maintenance work on `main`.
It is preparation evidence only; final `0.2.2` release evidence must be rerun
from the eventual release candidate commit.

Implemented maintenance coverage:

- Shared-build exported-symbol evidence target `ruckig_c_exported_symbols`.
- Windows manual static consumer CTest
  `ruckig_c_windows_manual_static_consumer`.
- Windows DLL consumer CTest `ruckig_c_windows_dll_consumer`.
- Fixed oracle suite expansion from 59 to 64 cases.
- New fixed oracle coverage for 4-6 DoF mixed scenarios, long high-frequency
  online update loops, very small `delta_time` with per-DoF mixed
  synchronization, segment-boundary query coverage, and multi-disabled
  mixed-order inputs.

Local static release-check CTest:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 20
```

Local shared release-check CTest:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 19
```

Fixed oracle suite:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe
```

Result:

```text
Oracle comparisons passed: 64
```

Development random oracle:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Result:

```text
Oracle comparisons passed: 64
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 64
Random oracle comparisons passed: 100000 seed 41
```

The seed `1` development random oracle also passed inside the static and shared
CTest runs through `ruckig_c_oracle_random_development`.

Per-DoF development random oracle:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 64
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Release random oracle:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random oracle executed `--random 1000000 --seed 1` and completed in
337.66 seconds. This is still preparation evidence; final `0.2.2` release
evidence must be rerun from the eventual release candidate commit.

Windows consumer automation:

```powershell
ctest --test-dir build_release_check_ninja -R "ruckig_c_windows_manual_static_consumer|ruckig_c_oracle_tests" --output-on-failure
ctest --test-dir build_release_check_shared -R "ruckig_c_windows_dll_consumer|ruckig_c_oracle_tests" --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 2
100% tests passed, 0 tests failed out of 2
```

Exported-symbol evidence:

```powershell
cmake --build build_release_check_shared --target ruckig_c_exported_symbols
```

Result:

```text
Wrote exported symbols to E:/Yww/DownLoad/source/ruckig_c/build_release_check_shared/artifacts/abi/0.2.2/windows-exports.txt
```

The Windows export snapshot contains 66 public `ruckig_*` exports, including
`ruckig_create`, `ruckig_calculate`, `ruckig_update`,
`ruckig_input_set_per_dof_control_interface`,
`ruckig_input_set_per_dof_synchronization`, and
`ruckig_trajectory_at_time`.

## 2026-06-04 0.2.2 Local Release Closeout

This pass verifies the local `0.2.2` release closeout after bumping the project
version to `0.2.2` and converting the changelog entry to a dated release entry.
Remote push CI, manual workflow-dispatch, tag, and GitHub Release evidence must
be recorded after the release closeout commit is pushed.

Local static release-check CTest:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 20
```

Local shared release-check CTest:

```powershell
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 20
```

Fixed oracle suite:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe
```

Result:

```text
Oracle comparisons passed: 64
```

Development random oracle:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
```

Result:

```text
Oracle comparisons passed: 64
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 64
Random oracle comparisons passed: 100000 seed 41
```

The seed `1` development random oracle passed inside both static and shared
CTest runs through `ruckig_c_oracle_random_development`.

Per-DoF development random oracle:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 64
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Release random oracle:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
```

The release random oracle executed `--random 1000000 --seed 1` and completed in
50.04 seconds.

Windows consumer automation:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
ruckig_c_windows_manual_static_consumer passed in the static CTest run.
ruckig_c_windows_dll_consumer passed in the shared CTest run.
```

Exported-symbol evidence:

```powershell
cmake --build build_release_check_shared --target ruckig_c_exported_symbols
```

Result:

```text
Wrote exported symbols to E:/Yww/DownLoad/source/ruckig_c/build_release_check_shared/artifacts/abi/0.2.2/windows-exports.txt
```

The Windows export snapshot contains 66 public `ruckig_*` exports, including
`ruckig_create`, `ruckig_calculate`, `ruckig_update`,
`ruckig_input_set_per_dof_control_interface`,
`ruckig_input_set_per_dof_synchronization`, and
`ruckig_trajectory_at_time`.

Public header diff from `v0.2.1` contains only version macro changes from
`0.2.1` to `0.2.2`; public functions, enum numeric values, and result-code
numeric values are unchanged. `original/ruckig-main` remains unchanged.

## 2026-06-04 0.2.2 Release Publication

The `v0.2.2` release was published after the local closeout gate, push CI, and
manual release-random workflow succeeded.

Final release evidence:

- Tag target commit: `15c896497fc5973fc19129c6fe59b2fd4da9533f`.
- Annotated tag object: `49b04e26776c6c787e97fb6223a03240031dd97a`.
- Push CI run id: `26935069765`, conclusion `success`.
- Push CI URL: `https://github.com/DiamondY/ruckig_c/actions/runs/26935069765`.
- Manual release random workflow run id: `26935519342`, conclusion `success`.
- Manual release random URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26935519342`.
- GitHub Release:
  `https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.2`.

Successful push CI jobs:

- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

Successful manual release random workflow jobs:

- `Manual release random oracle`
- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

The push CI artifacts are `7404574237` for Linux performance, `7404571215`
for Linux exported symbols, and `7404578412` for Windows exported symbols.
The manual workflow artifacts are `7404750529` for Linux performance,
`7404748851` for Linux exported symbols, and `7404752979` for Windows exported
symbols. Both workflows ran against commit
`15c896497fc5973fc19129c6fe59b2fd4da9533f`.

## 2026-06-04 0.2.3 Maintenance Preparation

This pass starts the `0.2.3 - Unreleased` maintenance queue after publishing
`v0.2.2`. It does not change the public C API and does not modify
`original/ruckig-main`.

Implemented maintenance coverage:

- Added tracked `v0.2.2` Linux and Windows exported-symbol baselines under
  `docs/abi/v0.2.2/`.
- Added `ruckig_c_compare_exported_symbols`, a warning/evidence-only shared
  build target that compares current exports against the `v0.2.2` baseline and
  writes a normalized diff artifact.
- Extended CI exported-symbol jobs to upload both current export snapshots and
  comparison diff summaries.
- Added `docs/release/checklists/0.2.3.md`.
- Added `docs/design/package_manager_feasibility.md` without adding package-manager
  recipes.
- Expanded Python bindings feasibility design to select `cffi` ABI mode as the
  default prototype path, still design-only.
- Expanded fixed oracle coverage for higher-DoF mixed synchronization,
  disabled DoFs, per-DoF overrides, discrete minimum-duration edges, tiny
  nonzero limits with large position magnitude, long online update loops, and
  repeated first-time-at-position boundary queries.

Local fixed oracle suite after the new cases:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe
```

Result:

```text
Oracle comparisons passed: 70
```

Static and shared release-check CTest:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 20
100% tests passed, 0 tests failed out of 20
```

Development random oracle runs:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 1
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 70
Random oracle comparisons passed: 100000 seed 1
Oracle comparisons passed: 70
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 70
Random oracle comparisons passed: 100000 seed 41
Oracle comparisons passed: 70
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Local Windows exported-symbol baseline comparison:

```powershell
cmake --build build_release_check_shared --target ruckig_c_compare_exported_symbols
```

Result:

```text
Exported symbols match the baseline
```

The generated comparison artifact reports 66 current symbols, 66 baseline
symbols, 0 added symbols, and 0 removed symbols. The comparison remains
warning/evidence only for `0.2.3`.

## 2026-06-04 0.2.3 Local Release Closeout

This pass verifies the local `0.2.3` release closeout after bumping the project
version to `0.2.3` and converting the changelog entry to a dated release entry.
Remote push CI, manual workflow-dispatch, tag, and GitHub Release evidence must
be recorded after the release closeout commit is pushed.

Version and scope:

- `CMakeLists.txt` project version: `0.2.3`.
- `include/ruckig_c/ruckig.h` version macros: `0.2.3`.
- Public header diff from `v0.2.2` contains only version macro changes.
- No public C API additions, removals, signature changes, enum numeric-value
  changes, or result-code numeric-value changes.
- `original/ruckig-main` remains frozen as the Ruckig Community `0.17.3`
  oracle baseline.
- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, and upstream baseline upgrades remain
  deferred.

Static and shared release-check CTest:

```powershell
ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release
ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release
```

Result:

```text
100% tests passed, 0 tests failed out of 20
100% tests passed, 0 tests failed out of 20
```

Fixed and development oracle gates:

```powershell
.\build_release_check_ninja\ruckig_c_oracle_tests.exe
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 1
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 2
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random 100000 --seed 41
.\build_release_check_ninja\ruckig_c_oracle_tests.exe --random-per-dof 100000 --seed 1
```

Result:

```text
Oracle comparisons passed: 70
Oracle comparisons passed: 70
Random oracle comparisons passed: 100000 seed 1
Oracle comparisons passed: 70
Random oracle comparisons passed: 100000 seed 2
Oracle comparisons passed: 70
Random oracle comparisons passed: 100000 seed 41
Oracle comparisons passed: 70
Random per-DoF oracle comparisons passed: 100000 seed 1
```

Release random oracle:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 1
ruckig_c_oracle_random_release passed in 51.24 seconds.
```

Local Windows exported-symbol baseline comparison:

```powershell
cmake --build build_release_check_shared --target ruckig_c_compare_exported_symbols
```

Result:

```text
Exported-symbol baseline comparison
current_symbol_count: 66
baseline_symbol_count: 66
added_symbol_count: 0
removed_symbol_count: 0
Exported symbols match the baseline
```

The generated artifacts are:

```text
build_release_check_shared/artifacts/abi/0.2.3/windows-exports.txt
build_release_check_shared/artifacts/abi/0.2.3/windows-export-diff.txt
```

Local consumer evidence:

- Installed CMake consumer passed in local release-check CTest.
- Windows manual static consumer passed in local release-check CTest.
- Windows DLL consumer passed in local shared release-check CTest.
- Linux pkg-config and Unix shared install-tree consumers must be recorded from
  CI or a Unix release gate.

## 2026-06-04 0.2.3 Release Publication

The `v0.2.3` release was published after the local closeout gate, push CI, and
manual release-random workflow completed successfully.

Release identity:

- Final release commit: `833dde30417539dd7f09d04734c9fdbd38b8d32e`.
- Annotated tag object: `b5c2979b97853cec56992ef13e73ac2e18653424`.
- Tag target commit: `833dde30417539dd7f09d04734c9fdbd38b8d32e`.
- Push CI run id: `26956280658`, conclusion `success`.
- Push CI URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26956280658`.
- Manual release random workflow run id: `26956708717`, conclusion `success`.
- Manual release random URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26956708717`.
- GitHub Release URL:
  `https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.3`.

Successful push CI jobs:

- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

Successful manual release random workflow jobs:

- `Manual release random oracle`
- `Windows clang-cl C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

The manual workflow ran against commit
`833dde30417539dd7f09d04734c9fdbd38b8d32e`. Its Linux performance job reported
`average_ratio_c_over_oracle: 1.26328`, below the `1.5` release threshold, and
the manual release random oracle job completed successfully in 59 seconds.

## 2026-06-04 0.2.4 Maintenance Start

This pass starts the `0.2.4 - Unreleased` maintenance queue after publishing
`v0.2.3`. It does not change the public C API and does not modify
`original/ruckig-main`.

Implemented maintenance setup:

- Added tracked `v0.2.3` Linux and Windows exported-symbol baselines under
  `docs/abi/v0.2.3/`.
- Updated the shared-build ABI comparison helper to compare current exports
  against the `v0.2.3` baseline and write `0.2.4` build-tree artifacts.
- Kept ABI comparison in warning/evidence mode for `0.2.4`; it is not a strict
  CI fail gate.
- Added `docs/release/checklists/0.2.4.md`.
- Extended Windows consumer smoke scripts to support both GNU-like `clang` and
  `clang-cl` frontend modes.
- Added Windows `clang-cl` shared C-only CI coverage so DLL/import-library
  consumer smoke also runs under the MSVC frontend variant.

Follow-up consumer-smoke verification:

- Push CI run id: `26958358278`, conclusion `success`.
- Push CI URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26958358278`.
- Commit: `79139d9a2581129f02c66ef0325921d709279b0d`.
- `Windows clang-cl C-only` passed with the manual static consumer smoke using
  the MSVC frontend dynamic CRT mode.
- `Windows clang-cl shared C-only` passed with the DLL/import-library consumer
  smoke.
- Linux and Windows exported-symbol jobs completed successfully; ABI comparison
  remains warning/evidence only for `0.2.4`.

## 2026-06-04 0.2.4 Local Release Closeout

This pass verifies the local `0.2.4` release closeout after bumping the project
version to `0.2.4` and converting the changelog entry to a dated release entry.
Remote push CI, manual release-random, tag, and GitHub Release evidence are
recorded after publication.

Version and scope:

- `CMakeLists.txt` project version: `0.2.4`.
- Public header version macros: `0.2.4`.
- No public C API additions were made.
- `original/ruckig-main` remains unchanged from `v0.2.3`.
- Intermediate waypoints, per-section constraints, cloud calculation, Python
  binding implementation, Rust bindings, and upstream baseline upgrades remain
  deferred.

Local release gates:

```text
Static CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Shared CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Fixed oracle suite: Oracle comparisons passed: 70.
Random oracle seed 1: Oracle comparisons passed: 70; Random oracle comparisons passed: 100000 seed 1.
Random oracle seed 2: Oracle comparisons passed: 70; Random oracle comparisons passed: 100000 seed 2.
Random oracle seed 41: Oracle comparisons passed: 70; Random oracle comparisons passed: 100000 seed 41.
Per-DoF random seed 1: Oracle comparisons passed: 70; Random per-DoF oracle comparisons passed: 100000 seed 1.
Release random oracle: 100% tests passed, 0 tests failed out of 1; passed in 48.88 seconds.
Windows performance ratio: 1.4087; threshold: 1.5.
```

Local Windows exported-symbol baseline comparison:

```text
Exported-symbol baseline comparison
current_symbol_count: 66
baseline_symbol_count: 66
added_symbol_count: 0
removed_symbol_count: 0
Exported symbols match the baseline
```

The generated artifacts are:

```text
build_release_check_shared/artifacts/abi/0.2.4/windows-exports.txt
build_release_check_shared/artifacts/abi/0.2.4/windows-export-diff.txt
```

Public header diff review:

```text
The diff from v0.2.3 contains only version macro changes from 0.2.3 to 0.2.4.
Public functions, enum numeric values, and result-code numeric values are unchanged.
```

Local consumer evidence:

- Installed CMake consumer passed in local release-check CTest.
- Windows manual static consumer passed in local release-check CTest.
- Windows DLL consumer passed in local shared release-check CTest.
- Linux pkg-config and Unix shared install-tree consumers must be recorded from
  push CI or a Unix release gate.

## 2026-06-04 0.2.4 Release Publication

The `v0.2.4` release was published after the local closeout gate, push CI, and
manual release-random workflow completed successfully.

Release identity:

- Final release commit: `3a65ce47449e4e9fe6708b57c9dc95d7151f2188`.
- Annotated tag object: `f71e18e95685ebccbe621e37aa115237d70f5bad`.
- Tag target commit: `3a65ce47449e4e9fe6708b57c9dc95d7151f2188`.
- Push CI run id: `26961897352`, conclusion `success`.
- Push CI URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26961897352`.
- Manual release random workflow run id: `26962279739`, conclusion `success`.
- Manual release random URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26962279739`.
- GitHub Release URL:
  `https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.4`.

Successful push CI jobs:

- `Windows clang-cl C-only`
- `Windows clang-cl shared C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

Successful manual release random workflow jobs:

- `Manual release random oracle`
- `Windows clang-cl C-only`
- `Windows clang-cl shared C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

The manual workflow ran against commit
`3a65ce47449e4e9fe6708b57c9dc95d7151f2188`. Its Linux performance job reported
`average_ratio_c_over_oracle: 1.29012`, below the `1.5` release threshold, and
the manual release random oracle job completed successfully.

## 2026-06-04 0.2.5 Maintenance Start

This pass starts the `0.2.5 - Unreleased` maintenance queue after publishing
`v0.2.4`. It does not change the public C API and does not modify
`original/ruckig-main`.

Implemented maintenance setup:

- Added tracked `v0.2.4` Linux and Windows exported-symbol baselines under
  `docs/abi/v0.2.4/`.
- Updated the shared-build ABI comparison helper to compare current exports
  against the `v0.2.4` baseline and write `0.2.5` build-tree artifacts.
- Kept ABI comparison in warning/evidence mode by default while documenting
  strict gate prerequisites and an exception-policy draft.
- Added `docs/release/checklists/0.2.5.md`.
- Expanded Windows standalone consumer matrix documentation for planned MSVC
  `cl` static/DLL smokes and MinGW feasibility status.
- Expanded package-manager feasibility notes without adding package-manager
  recipes.
- Expanded Python binding feasibility notes without adding binding code.
- Added targeted fixed oracle regression cases for the `0.2.5` queue; the fixed
  suite now reports 74 cases locally.

Local maintenance validation:

```text
Static CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Shared CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Fixed oracle suite: Oracle comparisons passed: 74.
Random oracle seed 1: Oracle comparisons passed: 74; Random oracle comparisons passed: 100000 seed 1.
Random oracle seed 2: Oracle comparisons passed: 74; Random oracle comparisons passed: 100000 seed 2.
Random oracle seed 41: Oracle comparisons passed: 74; Random oracle comparisons passed: 100000 seed 41.
Per-DoF random seed 1: Oracle comparisons passed: 74; Random per-DoF oracle comparisons passed: 100000 seed 1.
Windows performance maintenance ratio: 1.25602; threshold: 1.5.
Windows exported-symbol baseline comparison: 66 current symbols, 66 baseline symbols, 0 added, 0 removed.
```

## 2026-06-05 0.2.5 Local Release Closeout

This pass prepares `v0.2.5` as the final planned `0.2.x` stabilization release
before `0.3.0-design`. It does not add public C API, does not enable strict ABI
failure, does not add binding or package-manager implementation, and does not
modify `original/ruckig-main`.

Release candidate state:

- `CMakeLists.txt` project version: `0.2.5`.
- `include/ruckig_c/ruckig.h` version macros: `0.2.5`.
- `CHANGELOG.md` entry: `0.2.5 - 2026-06-05`.
- Public header diff against `v0.2.4`: version macro changes only.
- Fixed oracle suite count: `74`.
- Strict ABI gate: warning/evidence-only.
- MSVC `cl` standalone static/DLL consumer paths: documented, not yet
  CI-verified.
- MinGW static/DLL consumer paths: not yet verified.
- `0.2.6`: emergency patch reserve only.

Local release validation:

```text
Static CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Shared CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Fixed oracle suite: Oracle comparisons passed: 74.
Random oracle seed 1: Oracle comparisons passed: 74; Random oracle comparisons passed: 100000 seed 1.
Random oracle seed 2: Oracle comparisons passed: 74; Random oracle comparisons passed: 100000 seed 2.
Random oracle seed 41: Oracle comparisons passed: 74; Random oracle comparisons passed: 100000 seed 41.
Per-DoF random seed 1: Oracle comparisons passed: 74; Random per-DoF oracle comparisons passed: 100000 seed 1.
Release random oracle: 100% tests passed, 0 tests failed out of 1; ruckig_c_oracle_random_release passed in 48.99 seconds.
Windows performance release ratio: 1.16244; threshold: 1.5.
Windows manual static consumer: 100% tests passed, 0 tests failed out of 1.
Windows DLL consumer: 100% tests passed, 0 tests failed out of 1.
Windows exported-symbol baseline comparison: 66 current symbols, 66 baseline symbols, 0 added, 0 removed.
```

The new `docs/design/0.3.0_readiness.md` records the post-`v0.2.5` design
entry criteria and go/no-go decisions: Python `cffi` ABI-mode prototype design
can proceed after publication, Rust bindings remain deferred, vcpkg feasibility
is first among package-manager investigations, strict ABI diff failure remains
design-only, upstream baseline upgrade remains a separate project, and
waypoints/per-section/cloud remain behind separate public API design gates.

## 2026-06-05 0.2.5 Release Publication

The `v0.2.5` release was published after the local closeout gate, push CI, and
manual release-random workflow completed successfully.

Release identity:

- Final release commit: `c45a6ece69921c26419efcaefe10eed87de03605`.
- Annotated tag object: `7f11fb1e7ad3b513c3911c16a7699fc0bcd0b9bf`.
- Tag target commit: `c45a6ece69921c26419efcaefe10eed87de03605`.
- Push CI run id: `26965537200`, conclusion `success`.
- Push CI URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26965537200`.
- Manual release random workflow run id: `26965856552`, conclusion `success`.
- Manual release random URL:
  `https://github.com/DiamondY/ruckig_c/actions/runs/26965856552`.
- GitHub Release URL:
  `https://github.com/DiamondY/ruckig_c/releases/tag/v0.2.5`.

Successful push CI jobs:

- `Windows clang-cl C-only`
- `Windows clang-cl shared C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

Successful manual release random workflow jobs:

- `Manual release random oracle`
- `Windows clang-cl C-only`
- `Windows clang-cl shared C-only`
- `Windows clang oracle`
- `Linux GCC C-only`
- `Linux Clang oracle`
- `macOS Clang C-only`
- `Linux Clang ASan UBSan`
- `Linux Valgrind`
- `Linux Clang performance`
- `Linux exported symbols`
- `Windows exported symbols`

The manual workflow ran against commit
`c45a6ece69921c26419efcaefe10eed87de03605`. Its Linux performance job reported
`average_ratio_c_over_oracle: 1.30314`, below the `1.5` release threshold, and
the manual release random oracle job completed successfully.

Exported-symbol publication evidence:

```text
Windows exported-symbol comparison: 66 current symbols, 66 baseline symbols, 0 added, 0 removed.
Linux exported-symbol comparison: 127 current symbols, 66 baseline symbols, 61 added, 0 removed.
```

The Linux additions are implementation-internal `ruckig_*` symbols visible from
the Linux shared library while strict symbol visibility remains design-only.
They are not public header additions. The `v0.2.5` Linux ABI baseline saved
under `docs/abi/v0.2.5/linux-symbols.txt` records the actual release export set
so future comparison can detect drift from this state.

## 2026-06-05 0.3.0-design Transition Validation

This pass moves `main` to `0.3.0-design - Unreleased` after publishing
`v0.2.5`. It does not change the public C API, does not implement bindings,
does not add package-manager recipes, does not modify solver behavior, and
does not modify `original/ruckig-main`.

Post-release changes:

- Added `CHANGELOG.md` section `0.3.0-design - Unreleased`.
- Updated README and roadmap to identify `v0.2.5` as the latest release and
  final planned `0.2.x` stabilization baseline.
- Added `docs/abi/v0.2.5/` exported-symbol baselines.
- Moved ABI comparison baseline to `docs/abi/v0.2.5/`.
- Moved next-stage ABI artifacts to `artifacts/abi/0.3.0-design/`.

Local transition validation:

```text
Static CTest excluding release random: 100% tests passed, 0 tests failed out of 20.
Windows ABI comparison against docs/abi/v0.2.5: exported symbols match the baseline.
docs/abi/v0.2.5/linux-symbols.txt: 127 symbols.
docs/abi/v0.2.5/windows-symbols.txt: 66 symbols.
```

## 2026-06-05 0.3.0-design Local Hardening Pass

This pass verifies the next-stage local build entry, public ABI/export hygiene,
consumer smoke status, and Python `cffi` prototype smoke. It does not change
the public C API, does not add package-manager recipes, does not add
waypoints/per-section/cloud entry points, and does not promote strict ABI
comparison to a default CI failure gate.

Environment:

- OS: Windows.
- Compiler: clang 21.1.8, target `x86_64-pc-windows-msvc`.
- CMake: 4.1.0.
- Ninja: Visual Studio bundled Ninja 1.11.0 at
  `C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe`.
- Python: 3.14.3 in `out/python-prototype-venv`.
- `cffi`: 2.0.0.

Configured presets:

```powershell
cmake --list-presets
```

Result:

```text
Available configure presets include:
dev
windows-clang-ninja
windows-clang-ninja-shared
release
shared
oracle
```

Windows default local build entry:

```powershell
cmake --preset windows-clang-ninja
cmake --build --preset windows-clang-ninja
ctest --preset windows-clang-ninja
```

Result:

```text
100% tests passed, 0 tests failed out of 15.
```

The default Windows preset run covers the C unit tests, allocation audit,
C/C++ public header consumers, installed CMake consumer, Windows manual static
consumer, and all C examples.

Windows shared build and DLL/import-library consumer:

```powershell
cmake --preset windows-clang-ninja-shared
cmake --build --preset windows-clang-ninja-shared
ctest --preset windows-clang-ninja-shared
```

Result:

```text
100% tests passed, 0 tests failed out of 15.
```

The shared preset run covers the same routine tests plus the Windows
DLL/import-library consumer smoke.

Public symbol allowlist verification:

```powershell
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
```

Result:

```text
Public symbol allowlist matches include/ruckig_c/ruckig.h.
header_public_symbol_count: 66
expected_public_symbol_count: 66
missing_from_expected_count: 0
extra_in_expected_count: 0
```

Public exported-symbol comparison:

```powershell
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
```

Result:

```text
Public exported symbols match the approved allowlist.
strict_public_abi: OFF
current_symbol_count: 66
baseline_symbol_count: 66
approved_public_symbol_count: 66
current_public_symbol_count: 66
baseline_public_symbol_count: 66
missing_public_symbol_count: 0
added_public_since_baseline_count: 0
removed_public_since_baseline_count: 0
unapproved_exported_symbol_count: 0
```

The generated artifacts are under
`out/build/windows-clang-ninja-shared/artifacts/abi/0.3.0-design/`. The public
comparison remains evidence/trial mode; strict local failure is still opt-in.
`docs/abi/public-symbol-exceptions.txt` remains empty.

Python `cffi` prototype smoke:

```powershell
$env:RUCKIG_C_SHARED_LIBRARY = (Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\out\python-prototype-venv\Scripts\python.exe bindings\python_prototype\test_prototype.py
```

Result:

```text
Ran 4 tests in 0.004s
OK
```

The prototype remains experimental, not installed, not published, and outside
routine CI until shared-library discovery and packaging strategy are stable.

Windows consumer matrix discovery before installing the local MinGW toolchain:

```powershell
Get-Command clang.exe,clang++.exe,clang-cl.exe,cl,gcc,mingw32-make,x86_64-w64-mingw32-gcc -ErrorAction SilentlyContinue
```

Result:

```text
Found:
clang.exe
clang++.exe
clang-cl.exe

Not found in the current shell:
cl
gcc
mingw32-make
x86_64-w64-mingw32-gcc
```

MSVC `cl` standalone static/DLL smokes therefore remain opt-in local gates and
were not promoted to routine CI in this pass.

MSVC `cl` opt-in standalone static consumer:

```powershell
cmd.exe /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake -S . -B out\build\msvc-cl-static-smoke-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DRUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF"
cmd.exe /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake --build out\build\msvc-cl-static-smoke-ninja --config Release"
cmd.exe /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && ctest --test-dir out\build\msvc-cl-static-smoke-ninja --build-config Release -R ruckig_c_msvc_cl_static_consumer --output-on-failure"
```

Result:

```text
100% tests passed, 0 tests failed out of 1.
```

MSVC `cl` opt-in standalone DLL/import-library consumer:

```powershell
cmd.exe /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake -S . -B out\build\msvc-cl-dll-smoke-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DBUILD_SHARED_LIBS=ON -DRUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF"
cmd.exe /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && cmake --build out\build\msvc-cl-dll-smoke-ninja --config Release"
cmd.exe /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"" >nul && ctest --test-dir out\build\msvc-cl-dll-smoke-ninja --build-config Release -R ruckig_c_msvc_cl_dll_consumer --output-on-failure"
```

Result:

```text
100% tests passed, 0 tests failed out of 1.
```

MSVC `cl` remains an opt-in local gate rather than routine CI because the
routine Windows matrix already covers `clang-cl` static and shared consumers.

MinGW local toolchain:

```text
gcc.exe (x86_64-posix-seh-rev0, Built by MinGW-Builds project) 15.2.0
```

MinGW static consumer:

```powershell
$env:PATH = "C:\ProgramData\mingw64\mingw64\bin;" + $env:PATH
cmake -S . -B out\build\mingw-static-consumer -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/ProgramData/mingw64/mingw64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/ProgramData/mingw64/mingw64/bin/g++.exe" -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build out\build\mingw-static-consumer
ctest --test-dir out\build\mingw-static-consumer --output-on-failure -R ruckig_c_windows_manual_static_consumer
```

Result:

```text
100% tests passed, 0 tests failed out of 1.
```

MinGW DLL/import-library consumer:

```powershell
$env:PATH = "C:\ProgramData\mingw64\mingw64\bin;" + $env:PATH
cmake -S . -B out\build\mingw-dll-consumer -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/ProgramData/mingw64/mingw64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/ProgramData/mingw64/mingw64/bin/g++.exe" -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build out\build\mingw-dll-consumer
ctest --test-dir out\build\mingw-dll-consumer --output-on-failure -R ruckig_c_windows_dll_consumer
```

Result:

```text
100% tests passed, 0 tests failed out of 1.
```

This pass also adds a dedicated MSYS2 MinGW64 routine CI gate for the MinGW
static and DLL/import-library consumer smokes.

## 2026-06-05 0.3.0-design Final Checklist Audit

This audit verifies the implemented next-stage checklist after adding MinGW
consumer support and the Windows-specific local presets. It does not add public
C API symbols and does not change `docs/abi/public-symbols.txt` or
`docs/abi/public-symbol-exceptions.txt`.

Local preset and routine Windows clang validation:

```powershell
cmake --list-presets
cmake --preset windows-clang-ninja
cmake --build --preset windows-clang-ninja
ctest --preset windows-clang-ninja
```

Result:

```text
Available configure presets include windows-clang-ninja and
windows-clang-ninja-shared.
100% tests passed, 0 tests failed out of 15.
```

Shared build, DLL consumer, and ABI/export hygiene:

```powershell
cmake --preset windows-clang-ninja-shared
cmake --build --preset windows-clang-ninja-shared
ctest --preset windows-clang-ninja-shared
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_exported_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
```

Result:

```text
Shared CTest: 100% tests passed, 0 tests failed out of 15.
Public symbol allowlist verification: status clean, 66 header symbols, 66 expected symbols.
Windows exported-symbol baseline comparison: 66 current symbols, 66 baseline symbols, 0 added, 0 removed.
Public exported-symbol comparison: status clean, strict_public_abi OFF, 66 approved public symbols, 0 missing, 0 added, 0 removed, 0 unapproved exported symbols.
```

Python prototype smoke:

```powershell
$env:RUCKIG_C_SHARED_LIBRARY = (Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
.\out\python-prototype-venv\Scripts\python.exe bindings\python_prototype\test_prototype.py
```

Result:

```text
Ran 4 tests in 0.007s
OK
```

Windows clang-cl consumer smokes:

```powershell
cmake -S . -B out\build\clangcl-static-consumer -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang-cl.exe" -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build out\build\clangcl-static-consumer
ctest --test-dir out\build\clangcl-static-consumer --output-on-failure -R ruckig_c_windows_manual_static_consumer

cmake -S . -B out\build\clangcl-dll-consumer -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang-cl.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang-cl.exe" -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build out\build\clangcl-dll-consumer
ctest --test-dir out\build\clangcl-dll-consumer --output-on-failure -R ruckig_c_windows_dll_consumer
```

Result:

```text
clang-cl static consumer: 100% tests passed, 0 tests failed out of 1.
clang-cl DLL/import-library consumer: 100% tests passed, 0 tests failed out of 1.
```

MSVC `cl` opt-in consumer smokes:

```text
MSVC cl static consumer: 100% tests passed, 0 tests failed out of 1.
MSVC cl DLL/import-library consumer: 100% tests passed, 0 tests failed out of 1.
```

MSVC `cl` remains opt-in and local, not routine CI.

MinGW consumer smokes:

```text
MinGW GCC: 15.2.0.
MinGW static consumer: 100% tests passed, 0 tests failed out of 1.
MinGW DLL/import-library consumer: 100% tests passed, 0 tests failed out of 1.
MinGW static full CTest: 100% tests passed, 0 tests failed out of 15.
MinGW DLL full CTest: 100% tests passed, 0 tests failed out of 15.
```

The Windows MinGW static and DLL consumer checks are also wired into a
dedicated MSYS2 MinGW64 routine CI job.

Scope-freeze audit:

```text
No public header diff.
No docs/abi/public-symbols.txt diff.
docs/abi/public-symbol-exceptions.txt remains empty.
No package-manager recipe or new package-manager prototype was added.
No waypoint, per-section constraint, or cloud public API entry point was added.
Python prototype remains experimental, not installed, not published, and outside routine CI.
Strict public ABI failure remains opt-in/warning-evidence mode.
```

Linux ELF public-only export hygiene:

```powershell
$env:ZIG_GLOBAL_CACHE_DIR = 'E:\Yww\DownLoad\source\ruckig_c\out\zig-global-cache'
$env:ZIG_LOCAL_CACHE_DIR = 'E:\Yww\DownLoad\source\ruckig_c\out\zig-local-cache'
cmake -S . -B out\build\linux-elf-symbol-probe -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/ProgramData/chocolatey/bin/zig.exe" -DCMAKE_C_COMPILER_ARG1=cc -DCMAKE_C_FLAGS="-target x86_64-linux-gnu" -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_TESTS=OFF -DBUILD_RUCKIG_C_EXAMPLES=OFF -DBUILD_RUCKIG_C_ORACLE_TESTS=OFF -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build out\build\linux-elf-symbol-probe --target ruckig_c_verify_public_symbols ruckig_c_exported_symbols ruckig_c_compare_public_exported_symbols
```

Result:

```text
Linux ELF shared artifact: out/build/linux-elf-symbol-probe/libruckig_c.so.
Export inspector: llvm-nm -D --defined-only through the CMake exported-symbol target.
Public symbol allowlist verification: status clean, 66 header symbols, 66 expected symbols.
Public exported-symbol comparison: status clean, strict_public_abi OFF, 66 approved public symbols, 0 missing, 0 added, 0 removed, 0 unapproved exported symbols.
Current Linux ELF exported-symbol count: 66.
Historical v0.2.5 Linux baseline count: 127, including 61 implementation-internal symbols that are now hidden by the public-symbol version script.
```

This is export-hygiene evidence for a Linux ELF shared library generated from
the current worktree on a Windows host with Zig `cc -target x86_64-linux-gnu`.
It is not a Linux host CTest run. The dedicated Linux exported-symbol GitHub
Actions job remains the routine native Linux evidence path, but the local
GitHub CLI token was invalid during this audit and no WSL distribution was
available for a native local Linux run.

Local cleanup control:

```powershell
.\scripts\clean-local.ps1
.\scripts\clean-local.ps1 -Apply
```

Result:

```text
Dry run preview listed bindings/python_prototype/__pycache__/, build_vcpkg_tool/, and out/.
Apply removed bindings/python_prototype/__pycache__/, build_vcpkg_tool/, and out/.
No ignored local artifacts remained in git status after cleanup.
```

## 2026-06-05 0.3.0-design Closeout Follow-up

This pass implements the next closeout checklist items after commit
`15d85c7 Harden 0.3.0 design build and ABI workflow`. It keeps the public C API
frozen, keeps strict public ABI failure in trial/evidence mode, adds macOS
shared/export artifact bootstrap coverage, and expands the Python `cffi`
prototype tests without installing or publishing a binding package.

Repository state before edits:

```text
HEAD: 15d85c7 Harden 0.3.0 design build and ABI workflow
git status --short --ignored: clean
```

GitHub Actions evidence collection:

```powershell
gh auth status
```

Result:

```text
Failed to log in to github.com account DiamondY.
The token in default is invalid.
```

Remote CI run inspection and artifact download were therefore blocked in this
local pass. Re-run the GitHub Actions evidence step after refreshing the GitHub
CLI token, then record the run id, URL, job results, Linux performance artifact,
and exported-symbol artifacts here.

After pushing the closeout follow-up commit, public GitHub Actions metadata and
`gh run download` provided the remote evidence below.

Remote GitHub Actions push CI:

```text
Run id: 27014576431
Run URL: https://github.com/DiamondY/ruckig_c/actions/runs/27014576431
Commit: ec9e3b2b566e33bfddf91974a8263b8d0c85e0b1
Event: push
Conclusion: success
Started: 2026-06-05T12:22:30Z
```

Successful push CI jobs:

```text
Windows MinGW DLL consumer
Linux Clang ASan UBSan
macOS exported symbols
Windows MinGW static consumer
Linux Clang performance
Linux Valgrind
Linux Clang oracle
Windows clang-cl C-only
Windows exported symbols
Windows clang oracle
Windows clang-cl shared C-only
Linux exported symbols
macOS Clang C-only
Linux GCC C-only
```

The `Manual release random oracle` job was skipped as expected for a
push-triggered workflow.

Uploaded artifacts:

```text
Windows exported symbols: artifact id 7436395702, size 2983 bytes.
Linux performance: artifact id 7436387971, size 527 bytes.
Linux exported symbols: artifact id 7436384845, size 3386 bytes.
macOS exported symbols: artifact id 7436381416, size 1843 bytes.
```

Downloaded artifact inspection:

```text
Windows public exported-symbol comparison: status clean, strict_public_abi ON,
66 current symbols, 66 baseline symbols, 66 approved public symbols,
0 missing public symbols, 0 added public symbols, 0 removed public symbols,
0 unapproved exported symbols.

Windows exported-symbol baseline comparison: 66 current symbols,
66 baseline symbols, 0 added, 0 removed.

Linux public exported-symbol comparison: status clean, strict_public_abi ON,
66 current symbols, 127 baseline symbols, 66 approved public symbols,
0 missing public symbols, 0 added public symbols, 0 removed public symbols,
0 unapproved exported symbols, 61 historical baseline internal symbols.

Linux exported-symbol baseline comparison: 66 current symbols,
127 baseline symbols, 0 added, 61 historical internal symbols removed.

macOS public symbol allowlist verification: status clean,
66 header symbols, 66 expected symbols, 0 missing, 0 extra.
macOS Mach-O exported-symbol snapshot contains 66 symbols.

Linux performance: samples 10000, seed 1, clang 18.1.3,
c_average_ns 744.378, oracle_average_ns 586.91,
average_ratio_c_over_oracle 1.2683, release threshold 1.5.

Downloaded artifacts were inspected under `out/gh-artifacts/` and then removed
by `.\scripts\clean-local.ps1 -Apply` as ignored local artifacts.
```

Local Windows default preset validation:

```powershell
cmake --list-presets
cmake --preset windows-clang-ninja
cmake --build --preset windows-clang-ninja
ctest --preset windows-clang-ninja
```

Result:

```text
Available configure presets include windows-clang-ninja and windows-clang-ninja-shared.
100% tests passed, 0 tests failed out of 15.
```

Local Windows shared preset and ABI validation:

```powershell
cmake --preset windows-clang-ninja-shared
cmake --build --preset windows-clang-ninja-shared
ctest --preset windows-clang-ninja-shared
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_exported_symbols
```

Result:

```text
Shared CTest: 100% tests passed, 0 tests failed out of 15.
Public symbol allowlist verification: matches include/ruckig_c/ruckig.h.
Public exported-symbol comparison: clean, strict_public_abi OFF.
Windows exported-symbol baseline comparison: exported symbols match the baseline.
```

Python `cffi` prototype smoke:

```powershell
python -m venv out\python-prototype-venv
out\python-prototype-venv\Scripts\python.exe -m pip install cffi
$env:RUCKIG_C_SHARED_LIBRARY = (Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
out\python-prototype-venv\Scripts\python.exe bindings\python_prototype\test_prototype.py
```

Result:

```text
Ran 8 tests in 0.009s
OK
```

The expanded prototype coverage now includes create/destroy, double close,
method-after-close lifecycle errors, offline calculation, online update with
`output_pass_to_input`, list/tuple copy-in and list copy-out, length mismatch
checks before C array writes, and typed Python exceptions that retain the
original result code and operation name.

CI and documentation changes made in this pass:

```text
Added a macOS exported-symbol CI matrix entry for shared-build Mach-O export
artifact bootstrap. The job verifies the public symbol allowlist and uploads
build-shared/artifacts/abi/0.3.0-design/* without running a historical
exported-symbol diff because docs/abi/v0.2.5/macos-symbols.txt does not exist.

Renamed the Linux/Windows public exported-symbol comparison CI step to make the
strict-script, non-blocking trial semantics explicit.

Added docs/design/0.3.0_closeout_checklist.md and linked it from docs/index.md.
```

Scope-freeze audit:

```text
No public C header change.
No docs/abi/public-symbols.txt change.
docs/abi/public-symbol-exceptions.txt remains empty.
No package-manager recipe or new package-manager prototype was added.
No waypoint, per-section constraint, or cloud public API entry point was added.
Python prototype remains experimental, not installed, not published, and outside routine CI.
Rust bindings and upstream baseline upgrades remain deferred independent projects.
```

Local cleanup control:

```powershell
.\scripts\clean-local.ps1
.\scripts\clean-local.ps1 -Apply
```

Result:

```text
Dry run preview listed bindings/python_prototype/__pycache__/ and out/.
Apply removed bindings/python_prototype/__pycache__/ and out/.
```

## 2026-06-05 0.3.0 Hardening Release Preparation

This pass starts the accepted `0.3.0` hardening release after the completed
`0.3.0-design` closeout evidence. It promotes the version metadata and release
documentation to `0.3.0` without adding public C API, changing solver scope,
publishing bindings, adding package-manager recipes, or updating the frozen
upstream oracle baseline.

Repository state before edits:

```text
HEAD: cf956eb Record 0.3.0 design closeout CI evidence
git status --short --ignored: clean
```

Release-preparation commit:

```text
Commit: 1353ee665d5d1a9b0bd5c2eafd078b8ded214450
Subject: Prepare 0.3.0 hardening release
```

Intermediate release evidence commit:

```text
Commit: 221e3ab09819cd3f0c39e1033386e526ff1e1a8e
Subject: Record 0.3.0 release preparation evidence
```

Final release closeout commit and tag target:

```text
Commit: 5c7bf60612e6910073fa64e4837a304d063a9d7d
Subject: Complete 0.3.0 release evidence
```

Scope changes:

```text
CMakeLists.txt project version: 0.3.0.
include/ruckig_c/ruckig.h version macros: 0.3.0.
CHANGELOG.md entry: 0.3.0 - 2026-06-05.
Added docs/design/0.3.0_release_decision.md.
Added docs/release/checklists/0.3.0.md.
Moved next-stage ABI artifacts from artifacts/abi/0.3.0-design/ to artifacts/abi/0.3.0/.
```

Public API and scope-freeze audit:

```text
Public header diff against v0.2.5 contains only version macro changes.
No public C function additions, removals, or signature changes.
No enum numeric-value changes.
No result-code numeric-value changes.
docs/abi/public-symbols.txt unchanged.
docs/abi/public-symbol-exceptions.txt remains empty.
No package-manager recipe or new package-manager prototype was added.
No waypoint, per-section constraint, or cloud public API entry point was added.
Python prototype remains feasibility evidence only: not installed, not published, not packaged, and outside routine CI.
Rust bindings and upstream baseline upgrades remain deferred independent projects.
MSVC cl standalone consumer smokes remain optional local gates, not routine CI.
```

Local Windows default preset validation:

```powershell
cmake --preset windows-clang-ninja
cmake --build --preset windows-clang-ninja
ctest --preset windows-clang-ninja
```

Result:

```text
100% tests passed, 0 tests failed out of 15.
```

Local Windows shared preset and ABI validation:

```powershell
cmake --preset windows-clang-ninja-shared
cmake --build --preset windows-clang-ninja-shared
ctest --preset windows-clang-ninja-shared
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols
cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_exported_symbols
```

Result:

```text
Shared CTest: 100% tests passed, 0 tests failed out of 15.
Public symbol allowlist verification: clean, 66 header symbols, 66 expected symbols.
Windows public exported-symbol comparison: clean, strict_public_abi OFF,
66 current symbols, 66 baseline symbols, 66 approved public symbols,
0 missing public symbols, 0 added public symbols, 0 removed public symbols,
0 unapproved exported symbols.
Windows exported-symbol baseline comparison: 66 current symbols,
66 baseline symbols, 0 added, 0 removed.
```

Oracle validation:

```powershell
cmake -S . -B out\build\windows-clang-ninja-oracle -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DBUILD_RUCKIG_C=ON -DBUILD_RUCKIG_C_TESTS=ON -DBUILD_RUCKIG_C_EXAMPLES=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=OFF
cmake --build out\build\windows-clang-ninja-oracle
ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -E ruckig_c_oracle_random_release
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe
out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --random 100000 --seed 1
```

Result:

```text
Oracle CTest excluding release random: 100% tests passed, 0 tests failed out of 19.
Fixed oracle suite: Oracle comparisons passed: 76.
Development random oracle seed 1: Oracle comparisons passed: 76; Random oracle comparisons passed: 100000 seed 1.
```

Python `cffi` prototype smoke:

```powershell
python -m venv out\python-prototype-venv
out\python-prototype-venv\Scripts\python.exe -m pip install cffi
$env:RUCKIG_C_SHARED_LIBRARY = (Resolve-Path out\build\windows-clang-ninja-shared\ruckig_c.dll).Path
out\python-prototype-venv\Scripts\python.exe bindings\python_prototype\test_prototype.py
```

Result:

```text
Ran 8 tests in 0.005s
OK
```

Local cleanup control:

```powershell
.\scripts\clean-local.ps1
.\scripts\clean-local.ps1 -Apply
```

Result:

```text
Dry run preview listed bindings/python_prototype/__pycache__/ and out/.
Apply removed bindings/python_prototype/__pycache__/ and out/.
```

Remote GitHub Actions push CI for the final release closeout commit:

```text
Run id: 27028896945
Run URL: https://github.com/DiamondY/ruckig_c/actions/runs/27028896945
Commit: 5c7bf60612e6910073fa64e4837a304d063a9d7d
Event: push
Conclusion: success
Started: 2026-06-05T17:07:15Z
Completed: 2026-06-05T17:08:50Z
```

Successful push CI jobs:

```text
Windows clang-cl C-only
Windows clang-cl shared C-only
Windows clang oracle
Linux GCC C-only
Linux Clang oracle
macOS Clang C-only
Linux Clang ASan UBSan
Linux Valgrind
Linux Clang performance
Windows MinGW static consumer
Windows MinGW DLL consumer
Linux exported symbols
Windows exported symbols
macOS exported symbols
```

The `Manual release random oracle` job was skipped as expected for a
push-triggered workflow.

Manual release-random workflow dispatch for the published tag:

```text
Run id: 27029369635
Run URL: https://github.com/DiamondY/ruckig_c/actions/runs/27029369635
Ref: v0.3.0
Commit: 5c7bf60612e6910073fa64e4837a304d063a9d7d
Event: workflow_dispatch
Conclusion: success
Started: 2026-06-05T17:17:00Z
Completed: 2026-06-05T17:18:51Z
Manual release random oracle job conclusion: success
```

Successful workflow-dispatch jobs:

```text
Windows clang-cl C-only
Windows clang-cl shared C-only
Windows clang oracle
Linux GCC C-only
Linux Clang oracle
macOS Clang C-only
Linux Clang ASan UBSan
Linux Valgrind
Linux Clang performance
Windows MinGW static consumer
Windows MinGW DLL consumer
Linux exported symbols
Windows exported symbols
macOS exported symbols
Manual release random oracle
```

Uploaded artifact metadata for push CI run `27028896945`:

```text
Linux exported symbols: artifact id 7442363754, size 3372 bytes,
digest sha256:03606042215fa4af3b7a42dcbccccf869b5ad4566ba659c7c3b0152ee727f3ce.

Windows exported symbols: artifact id 7442367811, size 2969 bytes,
digest sha256:381bc6f1f6bc3b90e8c7a38d06b76db6bd68de38eb351f08f5f91611638c79a0.

macOS exported symbols: artifact id 7442357942, size 1838 bytes,
digest sha256:657270d732ae36ae3e09be6b28b4ba9004146f9125042d95297ac9785cbdd229.

linux-performance: artifact id 7442364071, size 527 bytes,
digest sha256:520679364dbd65d0ddbe2b8ac561c898340488fbc0d45f85709595b5da1139f8.
```

Downloaded artifact inspection:

```text
The artifacts were downloaded through the GitHub API and inspected from
out\gh-artifacts\27028896945 before local cleanup.

Linux public symbol allowlist verification: status clean, 66 header public
symbols, 66 expected public symbols, 0 missing, 0 extra.

Linux public exported-symbol comparison: status clean, strict_public_abi ON,
66 current symbols, 127 historical baseline symbols, 66 approved public
symbols, 0 missing public symbols, 0 added public symbols, 0 removed public
symbols, 0 unapproved exported symbols, 61 historical baseline internal
symbols intentionally hidden from the current public-only shared library.

Windows public symbol allowlist verification: status clean, 66 header public
symbols, 66 expected public symbols, 0 missing, 0 extra.

Windows public exported-symbol comparison: status clean, strict_public_abi ON,
66 current symbols, 66 historical baseline symbols, 66 approved public
symbols, 0 missing public symbols, 0 added public symbols, 0 removed public
symbols, 0 unapproved exported symbols.

macOS public symbol allowlist verification: status clean, 66 header public
symbols, 66 expected public symbols, 0 missing, 0 extra.

macOS Mach-O exported-symbol snapshot contains 66 symbols. No v0.2.5 macOS
historical baseline exists, so 0.3.0 records this as bootstrap evidence rather
than a historical exported-symbol diff.

Linux performance: samples 10000, seed 1, clang 18.1.3, Linux,
c_average_ns 716.329, oracle_average_ns 556.371,
average_ratio_c_over_oracle 1.2875, release threshold 1.5.
```

The Linux/Windows public exported-symbol comparison artifacts were generated
with `strict_public_abi: ON` so drift evidence is strict and reviewable. The
project policy for `0.3.0` remains trial/evidence mode, not a routine required
CI hard gate for all future changes.

Release publication:

```text
Annotated tag: v0.3.0
Annotated tag object: 42ba7ee0deb79e6ad9bb9416ef4ec9a829359df9
Tag target commit: 5c7bf60612e6910073fa64e4837a304d063a9d7d
GitHub Release: https://github.com/DiamondY/ruckig_c/releases/tag/v0.3.0
Published at: 2026-06-05T17:13:29Z
```

GitHub CLI authentication remained invalid during this pass, so release
creation, workflow dispatch, and final artifact download used the existing Git
Credential Manager token through the GitHub API without printing the token.

## 2026-06-06 0.4.0-Design Alpha Push CI

Remote GitHub Actions push CI for the `0.4.0-design` alpha commit:

```text
Run id: 27035876734
Run URL: https://github.com/DiamondY/ruckig_c/actions/runs/27035876734
Commit: d3a010ec89af0385fc0497a3d1fe5ff824fcef1d
Event: push
Conclusion: success
```

Successful push CI jobs:

```text
Windows clang-cl C-only
Windows clang-cl shared C-only
Windows clang oracle
Linux GCC C-only
Linux Clang oracle
macOS Clang C-only
Linux Clang ASan UBSan
Linux Valgrind
Linux Clang performance
Windows MinGW static consumer
Windows MinGW DLL consumer
Linux exported symbols
Windows exported symbols
macOS exported symbols
Python prototype smoke (Windows)
Python prototype smoke (Linux)
Python prototype smoke (macOS)
Rust alpha wrapper smoke
```

The `Manual release random oracle` job was skipped as expected for a
push-triggered workflow.

Uploaded artifact metadata for push CI run `27035876734`:

```text
linux-performance: artifact id 7445188452, size 1057 bytes.
Linux exported symbols: artifact id 7445183264, size 4544 bytes.
Windows exported symbols: artifact id 7445184317, size 4377 bytes.
macOS exported symbols: artifact id 7445180547, size 2459 bytes.
```

This CI run verifies the updated `0.4.0-design` ABI artifact path, Linux
waypoint alpha performance output, cross-platform Python prototype smoke, and
Rust alpha wrapper smoke. Stable `v0.4.0` is still not released from this
evidence; `docs/release/checklists/0.4.0-alpha.md` records the alpha decision
that another alpha cycle is required before making a stable waypoint optimizer
readiness claim.

## 2026-06-06 0.4.0-Design Local Alpha.2 Waypoint Evidence

The second local alpha pass strengthened waypoint optimizer evidence without
changing the public C header, existing ABI symbols, or the frozen
`original/ruckig-main` baseline.

Code coverage added in this pass:

```text
Fixed waypoint corpus:
- 4 DoF, two waypoints, per-section minimum duration, and per-section
  position bounds.
- 6 DoF, one waypoint, disabled constant DoFs, and waypoint/final-state checks.
- 1 DoF, two waypoints, nonzero current and target velocities for quality
  regression coverage.

Section-level oracle corpus:
- 3 DoF waypoint case with per-section velocity, acceleration, jerk, and
  minimum-duration constraints.
- 4 DoF waypoint case with three intermediate waypoints.

CTest structure:
- ruckig_c_waypoint_optimizer, ruckig_c_per_section_constraints, and
  ruckig_c_waypoint_quality now run distinct focused subsets.
```

Local verification:

```text
ctest --test-dir out\build\windows-clang-ninja -R "ruckig_c_waypoint_optimizer|ruckig_c_per_section_constraints|ruckig_c_waypoint_quality" --output-on-failure:
pass, 3/3.

ctest --test-dir out\build\windows-clang-ninja --output-on-failure:
pass, 24/24.

ctest --test-dir out\build\windows-clang-ninja-oracle --output-on-failure -E ruckig_c_oracle_random_release:
pass, 29/29.

out\build\windows-clang-ninja-oracle\ruckig_c_oracle_tests.exe --waypoint-section-oracle:
pass; Waypoint section oracle comparisons passed: 4.
```

This alpha.2 evidence still does not publish stable `v0.4.0`. The stable
release checklist remains open for a later release decision, cross-platform CI
evidence from the final release candidate, and release closeout.
