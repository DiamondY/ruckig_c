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

## Patch Release Procedure

Every `0.1.x` and `0.2.x` patch release must record a Windows clang release
benchmark and a Linux clang release benchmark against the frozen C++ oracle.
Use the existing benchmark executable and keep the release threshold at average
C/oracle ratio `<= 1.5`.

## 0.2.x Performance Trend Procedure

Use the same benchmark corpus for each `0.2.x` patch release unless a release
explicitly documents a corpus change:

- Required release corpus: `--samples 10000 --seed 1`.
- Optional development comparison corpora: `--samples 10000 --seed 2` and
  `--samples 10000 --seed 41`.
- Required platforms: Windows clang release and Linux clang release.
- Required threshold: average C/oracle ratio `<= 1.5`.

Store raw local or CI outputs outside version-controlled source unless the
release process intentionally promotes a short excerpt into this report. Use a
stable artifact convention such as:

```text
artifacts/performance/0.2.1/windows-clang-release.txt
artifacts/performance/0.2.1/linux-clang-release.txt
```

Only the summarized release evidence should be committed to this report.

Record this template for each run:

```text
- Source:
- Command:
- OS:
- Kernel/build:
- CPU identifier:
- C compiler:
- C++ compiler:
- CMake build type:
- Generator:
- Seed:
- Samples:
- C average ns:
- C p99 ns:
- C worst ns:
- Oracle average ns:
- Oracle p99 ns:
- Oracle worst ns:
- Average C/oracle ratio:
- Release threshold:
- Result:
```

The GitHub Actions `Linux Clang performance` job uploads
`linux-performance.txt` and emits a `Linux performance` check-run annotation.
Record the final release-gate output or annotation in this report. On Windows,
run:

```powershell
.\build-release\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1
```

or the equivalent executable path for the release-check build directory.

## 2026-06-03 Linux 0.2.0 Release Push CI Run

- Source: GitHub Actions CI run `26887345035`, job `Linux Clang performance`
  (`79303225672`).
- Commit: `7ae8d4c2d05c6ea02547d6387de207df59826650`.
- Artifact: `linux-performance`, artifact id `7385683968`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`
- OS: Linux, GitHub-hosted Ubuntu runner.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu SMP Wed May 6 22:37:49 UTC 2026 x86_64`
- CPU identifier: `AMD EPYC 7763 64-Core Processor`
- Compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`
- CMake build type: `Release`
- Generator: `Ninja`
- Seed: `1`
- Samples: `10000`

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 703.93 ns | 551.239 ns |
| p99 | 5330 ns | 4168 ns |
| Worst | 21060 ns | 15900 ns |

Average C/oracle ratio: `1.277`.

This Linux CI run is within the release threshold of average calculation time
no worse than `1.5x` the C++ oracle on the same benchmark corpus.

## 2026-06-03 Windows 0.2.0 Release Closeout Run

- Source: Local release closeout gate at commit
  `4b04212eba8b793805275702c333ed41cf65de20`.
- Command:
  `.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: `Release`.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 1345.11 ns | 4691.73 ns |
| p99 | 9100 ns | 20000 ns |
| Worst | 20200 ns | 46500 ns |

Average C/oracle ratio: `0.286698`.

This Windows release closeout run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus.

## 2026-06-03 Windows 0.2.1 Preparation Smoke

Superseded by final 0.2.1 release evidence below.

- Source: Local `0.2.1` preparation smoke on `main`; not final release
  evidence.
- Command:
  `.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: `Release`.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 1348.88 ns | 4670.15 ns |
| p99 | 9100 ns | 19600 ns |
| Worst | 20500 ns | 46300 ns |

Average C/oracle ratio: `0.28883`.

This preparation smoke is within the release threshold of average calculation
time no worse than `1.5x` the C++ oracle on the same benchmark corpus. Final
`0.2.1` release evidence must be rerun from the tag candidate commit.

## 2026-06-04 Windows 0.2.1 Local Release Closeout Run

- Source: Local `0.2.1` release closeout gate; final commit hash to be recorded
  after commit creation.
- Command:
  `.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: Release-check Ninja build directory.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 1506.44 ns | 4588.76 ns |
| p99 | 10000 ns | 19500 ns |
| Worst | 25800 ns | 53400 ns |

Average C/oracle ratio: `0.328289`.

This local Windows release closeout run is within the release threshold of
average calculation time no worse than `1.5x` the C++ oracle on the same
benchmark corpus.

## 2026-06-04 Linux 0.2.1 Manual Release Workflow Run

- Source: GitHub Actions workflow-dispatch run `26898859059`, job
  `Linux Clang performance`.
- Commit: `dc09eb938484b407415e1d6b4f59a2242c18ba8b`.
- Artifact: `linux-performance`, artifact id `7390788705`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`
- OS: Linux, GitHub-hosted Ubuntu runner.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu SMP Wed May 6 22:37:49 UTC 2026 x86_64`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- Compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: `Release`.
- Generator: `Ninja`.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 734.393 ns | 554.951 ns |
| p99 | 5400 ns | 4178 ns |
| Worst | 38813 ns | 26409 ns |

Average C/oracle ratio: `1.32335`.

This Linux manual release workflow run is within the release threshold of
average calculation time no worse than `1.5x` the C++ oracle on the same
benchmark corpus.

## 0.2.2 Benchmark Template

Use this template for the next patch-release evidence. Compare Windows results
only against the same Windows benchmark context and Linux results only against
the same Linux benchmark context; do not compare absolute timings across
different platforms or runner classes.

## 2026-06-04 Windows 0.2.2 Preparation Smoke

- Source: Local `0.2.2` preparation smoke on `main`; not final release
  evidence.
- Command:
  `.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: Release-check Ninja build directory.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 1327.32 ns | 4443.77 ns |
| p99 | 9000 ns | 18900 ns |
| Worst | 50500 ns | 149100 ns |

Average C/oracle ratio: `0.298692`.

The same-platform `0.2.1` local Windows release baseline ratio was `0.328289`.
This preparation smoke is within the release threshold of average calculation
time no worse than `1.5x` the C++ oracle on the same benchmark corpus. Final
`0.2.2` release evidence must be rerun from the release candidate commit.

## 2026-06-04 Windows 0.2.2 Local Release Closeout Run

- Source: Local `0.2.2` release closeout gate after bumping the project version
  to `0.2.2`.
- Command:
  `.\build_release_check_ninja\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: Release-check Ninja build directory.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 696.12 ns | 643.18 ns |
| p99 | 5100 ns | 5100 ns |
| Worst | 72500 ns | 25300 ns |

Average C/oracle ratio: `1.08231`.

The same-platform `0.2.1` local Windows release baseline ratio was `0.328289`.
This release closeout run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus. Linux `0.2.2` release evidence must be recorded from the push CI or
manual workflow artifact for the release commit.

### Windows clang release

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

### Linux clang release

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
