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
