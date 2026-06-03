# Ruckig C Performance Report

This document records reproducible performance runs for the pure C rewrite
against the frozen C++ oracle under `original/ruckig-main`.

The benchmark measures `ruckig_calculate` after all C handles, inputs, and
trajectories have been created. The C++ oracle measurement uses the equivalent
`Ruckig::calculate` call on the same generated corpus. Lifecycle allocation cost
is intentionally excluded.

## Benchmark Command

Direct compiler path used on this workstation:

```powershell
clang -std=c99 -O2 -DNDEBUG -DRUCKIG_C_STATIC_DEFINE -Wall -Wextra -Wpedantic -I..\include -I..\src -c ..\src\ruckig_c\alloc.c ..\src\ruckig_c\block.c ..\src\ruckig_c\brake.c ..\src\ruckig_c\input.c ..\src\ruckig_c\output.c ..\src\ruckig_c\position_first_step1.c ..\src\ruckig_c\position_first_step2.c ..\src\ruckig_c\position_second_step1.c ..\src\ruckig_c\position_second_step2.c ..\src\ruckig_c\position_third_step1.c ..\src\ruckig_c\position_third_step2.c ..\src\ruckig_c\profile.c ..\src\ruckig_c\roots.c ..\src\ruckig_c\ruckig.c ..\src\ruckig_c\trajectory.c ..\src\ruckig_c\utils.c ..\src\ruckig_c\velocity_second_step1.c ..\src\ruckig_c\velocity_second_step2.c ..\src\ruckig_c\velocity_third_step1.c ..\src\ruckig_c\velocity_third_step2.c
clang++ -std=c++20 -O2 -DNDEBUG -DRUCKIG_C_STATIC_DEFINE -D_USE_MATH_DEFINES -I..\include -I..\original\ruckig-main\include alloc.o block.o brake.o input.o output.o position_first_step1.o position_first_step2.o position_second_step1.o position_second_step2.o position_third_step1.o position_third_step2.o profile.o roots.o ruckig.o trajectory.o utils.o velocity_second_step1.o velocity_second_step2.o velocity_third_step1.o velocity_third_step2.o ..\test\cpp\performance_benchmark.cpp ..\original\ruckig-main\src\ruckig\brake.cpp ..\original\ruckig-main\src\ruckig\position_first_step1.cpp ..\original\ruckig-main\src\ruckig\position_first_step2.cpp ..\original\ruckig-main\src\ruckig\position_second_step1.cpp ..\original\ruckig-main\src\ruckig\position_second_step2.cpp ..\original\ruckig-main\src\ruckig\position_third_step1.cpp ..\original\ruckig-main\src\ruckig\position_third_step2.cpp ..\original\ruckig-main\src\ruckig\velocity_second_step1.cpp ..\original\ruckig-main\src\ruckig\velocity_second_step2.cpp ..\original\ruckig-main\src\ruckig\velocity_third_step1.cpp ..\original\ruckig-main\src\ruckig\velocity_third_step2.cpp -o ruckig_c_performance_benchmark_direct.exe
.\ruckig_c_performance_benchmark_direct.exe --samples 10000 --seed 1
```

CMake/Ninja target used on this workstation:

```powershell
cmake -S . -B build_ninja -G Ninja -DCMAKE_MAKE_PROGRAM="C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe" -DCMAKE_C_COMPILER="D:/Program Files/LLVM/bin/clang.exe" -DCMAKE_CXX_COMPILER="D:/Program Files/LLVM/bin/clang++.exe" -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=ON
cmake --build build_ninja --config Release
ctest --test-dir build_ninja -R ruckig_c_performance_benchmark --output-on-failure
```

## 2026-06-03 Windows Direct Clang Run

- OS: Windows
- Compiler: clang 21.1.8
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`
- Flags:
  - C: `-std=c99 -O2 -DNDEBUG -DRUCKIG_C_STATIC_DEFINE -Wall -Wextra -Wpedantic`
  - C++: `-std=c++20 -O2 -DNDEBUG -DRUCKIG_C_STATIC_DEFINE -D_USE_MATH_DEFINES`
- Seed: `1`
- Samples: `10000`
- Corpus: supported representative random generator from the oracle harness,
  covering 1-3 DoF position and velocity cases, continuous/discrete duration,
  synchronization modes including Phase/None, directional limits, disabled
  DoFs, and general third-order position states.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 599.92 ns | 495.05 ns |
| p99 | 4600 ns | 3700 ns |
| Worst | 31500 ns | 24000 ns |

Average C/oracle ratio: `1.21184`.

Release threshold from the PRD: average calculation time no worse than `1.5x`
the C++ oracle on the same benchmark corpus. This run is within that threshold.

## 2026-06-03 Windows CMake/Ninja CTest Run

- OS: Windows
- CMake: 4.1.0
- Generator: Ninja 1.11.0 from Visual Studio 2022 Community
- Compiler: clang 21.1.8, target `x86_64-pc-windows-msvc`
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`
- Seed: `1`
- Samples: `10000`

Command:

```powershell
.\build_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1
```

Output:

```text
Ruckig C performance benchmark
samples: 10000
seed: 1
compiler: clang 21.1.8
os: Windows
c_average_ns: 1285.41
c_p99_ns: 9000
c_worst_ns: 20200
oracle_average_ns: 4574.84
oracle_p99_ns: 19400
oracle_worst_ns: 73100
average_ratio_c_over_oracle: 0.280974
release_threshold_average_ratio: 1.5
```

This CMake/Ninja run is also within the PRD threshold. The absolute timings
differ from the direct-clang run, so direct and CMake measurements should be
compared within their own build contexts rather than mixed.

## Notes

- Both recorded Windows runs satisfy the PRD average-ratio threshold.
- WMI CPU model lookup is denied in this workspace, so the report records the
  non-elevated `PROCESSOR_IDENTIFIER` value instead of a marketing CPU name.
- A Linux benchmark is required before tagging a portable public `0.1.0`
  release. Record at least one Linux GCC or Clang run with the same
  `--samples 10000 --seed 1` command, compiler flags, CPU identifier, average,
  p99, worst case, and C/oracle average ratio.
