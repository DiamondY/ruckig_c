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

See `docs/performance_report.md`.

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
