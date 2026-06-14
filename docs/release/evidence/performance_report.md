# Ruckig C Performance Report

This document records reproducible performance runs for the pure C rewrite
against the frozen C++ oracle under `original/ruckig-main`.

## 2026-06-14 0.16.0 Release Candidate Performance Evidence

The `0.16.0` release candidate records a fresh local performance gate after
promoting the public diagnostics API to stable release metadata. Diagnostics
are opt-in, legacy calculate/update behavior remains compatible, and the
release threshold remains unchanged.

Environment:

- OS: Windows
- Compiler: clang 21.1.8
- CMake preset: `windows-clang-ninja-performance`
- Seed: `1`
- Samples: `10000`

No-waypoint benchmark:

```text
out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold

c_average_ns: 678.09
c_p99_ns: 4900
c_worst_ns: 24500
oracle_average_ns: 513.16
oracle_p99_ns: 3800
oracle_worst_ns: 28600
average_ratio_c_over_oracle: 1.3214
release_threshold_average_ratio: 1.5
```

Waypoint alpha benchmark:

```text
out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints

waypoint_case_count: 10
waypoint_max_dofs: 8
waypoint_max_intermediate_positions: 3
waypoint_c_average_ns: 3.29597e+06
waypoint_c_p99_ns: 1.21724e+07
waypoint_c_worst_ns: 1.69419e+07
waypoint_oracle_ratio: unavailable
waypoint_benchmark_policy: alpha C-only local optimizer corpus
```

Release-candidate push CI Linux performance:

```text
run: 27500260082
url: https://github.com/DiamondY/ruckig_c/actions/runs/27500260082
artifact: linux-performance/linux-performance.txt

c_average_ns: 735.143
c_p99_ns: 5460
c_worst_ns: 29645
oracle_average_ns: 558.956
oracle_p99_ns: 4208
oracle_worst_ns: 28674
average_ratio_c_over_oracle: 1.31521
release_threshold_average_ratio: 1.5
```

Release-candidate push CI Linux waypoint alpha benchmark:

```text
artifact: linux-performance/linux-waypoint-performance.txt

waypoint_case_count: 10
waypoint_max_dofs: 8
waypoint_max_intermediate_positions: 3
waypoint_c_average_ns: 3.59332e+06
waypoint_c_p99_ns: 1.30727e+07
waypoint_c_worst_ns: 1.8106e+07
waypoint_oracle_ratio: unavailable
waypoint_benchmark_policy: alpha C-only local optimizer corpus
```

Manual release-random workflow Linux performance:

```text
run: 27500474307
url: https://github.com/DiamondY/ruckig_c/actions/runs/27500474307
artifact: linux-performance/linux-performance.txt

c_average_ns: 778.238
c_p99_ns: 5838
c_worst_ns: 39779
oracle_average_ns: 585.433
oracle_p99_ns: 4436
oracle_worst_ns: 19729
average_ratio_c_over_oracle: 1.32934
release_threshold_average_ratio: 1.5
```

Manual release-random workflow Linux waypoint alpha benchmark:

```text
artifact: linux-performance/linux-waypoint-performance.txt

waypoint_case_count: 10
waypoint_max_dofs: 8
waypoint_max_intermediate_positions: 3
waypoint_c_average_ns: 3.71911e+06
waypoint_c_p99_ns: 1.33658e+07
waypoint_c_worst_ns: 1.50279e+07
waypoint_oracle_ratio: unavailable
waypoint_benchmark_policy: alpha C-only local optimizer corpus
```

## 2026-06-14 0.16.0 Public Diagnostics Readiness Performance Evidence

The public diagnostics readiness slice records a fresh local performance gate
because alpha.3 and alpha.5 intentionally expanded the public ABI. The
diagnostics API is opt-in, the legacy calculate/update paths remain
behavior-compatible, and no performance-threshold policy changes are made.

Environment:

- OS: Windows
- Compiler: clang 21.1.8
- CMake preset: `windows-clang-ninja-performance`
- Seed: `1`
- Samples: `10000`

No-waypoint benchmark:

```text
out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold

c_average_ns: 672.37
c_p99_ns: 4900
c_worst_ns: 24100
oracle_average_ns: 519.88
oracle_p99_ns: 3800
oracle_worst_ns: 28300
average_ratio_c_over_oracle: 1.29332
release_threshold_average_ratio: 1.5
```

Waypoint alpha benchmark:

```text
out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints

waypoint_case_count: 10
waypoint_max_dofs: 8
waypoint_max_intermediate_positions: 3
waypoint_c_average_ns: 3.37152e+06
waypoint_c_p99_ns: 1.25174e+07
waypoint_c_worst_ns: 1.71957e+07
waypoint_oracle_ratio: unavailable
waypoint_benchmark_policy: alpha C-only local optimizer corpus
```

## 2026-06-14 Post-v0.15.0 Quality Closeout Performance Evidence

The post-`v0.15.0` quality closeout does not introduce a new performance
baseline. It references the existing performance evidence from the recent
quality series:

| Slice | Evidence |
| --- | --- |
| `post-v0.15.0-solver-branch-coverage` | Local quasi-release performance gates passed before `c934265`; ordinary push CI run `27460225445` concluded success. |
| `post-v0.15.0-solver-adjacent-branch-coverage` | Local performance gates passed before `8a0e82c`; ordinary push CI run `27469013933` concluded success. |
| `post-v0.15.0-review-followup-quality-hardening` | Local no-waypoint performance ratio `1.03447` was below the `1.5` threshold; waypoint C-only alpha corpus passed; ordinary push CI run `27472932035` concluded success. |
| `post-v0.15.0-portability-static-audit` | Ordinary push CI run `27475649359` concluded success, including the Linux Clang performance job. |

No package, release, tag, workflow, or performance-threshold policy change is
made by the quality closeout.

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

Current same-platform release baselines:

| Release | Windows clang ratio/evidence | Linux clang ratio/evidence | Notes |
| --- | ---: | ---: | --- |
| `0.2.1` | `0.328289` | `1.32335` | Final local Windows closeout and manual Linux release workflow evidence. |
| `0.2.2` | `1.39707` | Pass, artifact ids `7404574237` and `7404750529` | Final Windows tag-target rerun and manual Linux release workflow evidence; the Linux ratio was not excerpted into tracked docs. |
| `0.2.3` | `1.08893` | `1.26328` | Final Windows closeout and manual Linux release workflow evidence. |
| `0.2.4` | `1.4087` | `1.29012` | Final Windows closeout and manual Linux release workflow evidence. |
| `0.2.5` | `1.16244` | `1.30314` | Final planned pre-`0.3.0` stabilization baseline from local Windows closeout and manual Linux release workflow evidence. |
| `0.3.0` | Not a separate 0.3.0 gate | `1.2875` | Hardening release final push CI run `27028896945`, artifact `7442364071`; local Windows release gates covered build/test/ABI rather than a separate benchmark rerun. |
| `0.4.0` | `1.23622` | `1.17323` push CI; `1.29272` manual workflow | Original-surface parity release. Windows local alpha.4 pre-stable gate; Linux final push CI run `27038403450`, artifact `7446167572`; manual workflow run `27038538349`, artifact `7446219206`. |
| `0.4.1` | `1.18871` | `1.29559` push CI; `1.31319` manual workflow | Deep stabilization release. Windows local closeout gate; Linux final push CI run `27056498079`, artifact `7452526552`; manual workflow run `27058264617`. |
| `0.4.2` | `1.35448` | `1.26819` push CI; `1.30502` tag manual workflow | Original parity coverage/evidence closeout. Windows local closeout gate; Linux release-candidate push CI run `27063738903`, artifact `7454827710`; tag manual workflow run `27064919699`. |
| `0.5.0` | `1.27789` | Tag CI performance job success, artifact id `7461291915` | Stable tracking Fast-mode release. Windows local closeout gate passed; Linux tag CI run `27084478323` succeeded. Artifact download requires authenticated GitHub access, so the tracked report records job success and artifact id rather than raw ratio. |

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

## 2026-06-07 Windows 0.5.0 Release-Candidate Performance

- Source: Local `v0.5.0` release-candidate gate.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- C compiler: clang 21.1.8.
- C++ compiler: clang 21.1.8.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 651.76 ns | 510.03 ns |
| p99 | 4800 ns | 3800 ns |
| Worst | 23800 ns | 27500 ns |

Average C/oracle ratio: `1.27789`.

This Windows local release-candidate run is within the release threshold of
average calculation time no worse than `1.5x` the C++ oracle on the same
benchmark corpus.

Waypoint benchmark command:

```powershell
out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints
```

Waypoint output:

```text
waypoint_case_count: 10
waypoint_max_dofs: 8
waypoint_max_intermediate_positions: 3
waypoint_c_average_ns: 3.26093e+06
waypoint_c_p99_ns: 1.20148e+07
waypoint_c_worst_ns: 1.77536e+07
waypoint_oracle_ratio: unavailable
waypoint_benchmark_policy: alpha C-only local optimizer corpus
```

The waypoint benchmark remains same-platform local optimizer trend evidence
and is not mixed with the no-waypoint C++ oracle ratio.

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

## 2026-06-04 Windows 0.2.2 Post-Publication Sanity Run

- Source: Local rerun on the final `v0.2.2` tag target commit
  `15c896497fc5973fc19129c6fe59b2fd4da9533f`.
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
| Average | 700.52 ns | 501.42 ns |
| p99 | 5300 ns | 3700 ns |
| Worst | 18000 ns | 19800 ns |

Average C/oracle ratio: `1.39707`.

The same-platform `0.2.1` local Windows release baseline ratio was `0.328289`.
This final-commit rerun is within the release threshold of average calculation
time no worse than `1.5x` the C++ oracle on the same benchmark corpus.

## 2026-06-04 Linux 0.2.2 Manual Release Workflow Run

- Source: GitHub Actions workflow-dispatch run `26935519342`, job
  `Linux Clang performance`.
- Commit: `15c896497fc5973fc19129c6fe59b2fd4da9533f`.
- Artifact: `linux-performance`, artifact id `7404750529`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- Compiler: Linux Clang from the GitHub-hosted runner image.
- CMake build type: `Release`.
- Generator: `Ninja`.
- Seed: `1`.
- Samples: `10000`.

The benchmark job completed successfully. The benchmark executable enforces
`average_ratio_c_over_oracle <= 1.5`, so the artifact is accepted as final
Linux `0.2.2` release evidence. The push CI run `26935069765` also uploaded a
Linux performance artifact, id `7404574237`, for the same commit.

## 0.2.3 Benchmark Template

Use the same required corpus, `--samples 10000 --seed 1`, for `0.2.3`. Compare
Windows results only against the `0.2.2` Windows benchmark context and Linux
results only against the `0.2.2` Linux benchmark context; do not compare
absolute timings across platforms or runner classes.

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
0.2.2 same-platform baseline ratio:
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
0.2.2 same-platform baseline ratio:
Release threshold:
Result:
```

## 2026-06-06 Windows 0.4.1 Local Release-Candidate No-Waypoint Run

- Source: Local `0.4.1` release-candidate gate before release-candidate
  commit creation.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`.
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 651.78 ns | 548.31 ns |
| p99 | 4800 ns | 4000 ns |
| Worst | 18000 ns | 29500 ns |

Average C/oracle ratio: `1.18871`.

This Windows release-candidate run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same no-waypoint
benchmark corpus.

## 2026-06-06 Windows 0.4.1 Local Release-Candidate Waypoint Run

- Source: Local `0.4.1` release-candidate waypoint benchmark corpus.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints`.
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Maximum DoF count: `8`.
- Maximum intermediate waypoint count: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 3.27228e+06 ns |
| p99 | 1.21478e+07 ns |
| Worst | 1.70838e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This evidence is same-platform local optimizer trend data only.

## 2026-06-06 Linux 0.4.1 Final Push No-Waypoint Run

- Source: GitHub Actions push CI run `27056498079`, job `Linux Clang
  performance`, artifact `7452526552`.
- Commit: `95d733b0094f320d968ac057151e8a0a62e0353e`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux.
- Kernel:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 717.409 ns | 553.731 ns |
| p99 | 5330 ns | 4198 ns |
| Worst | 31699 ns | 21069 ns |

Average C/oracle ratio: `1.29559`.

## 2026-06-06 Linux 0.4.1 Final Push Waypoint Run

- Source: GitHub Actions push CI run `27056498079`, job `Linux Clang
  performance`, artifact `7452526552`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux.
- Kernel:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Maximum DoF count: `8`.
- Maximum intermediate waypoint count: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 3.5258e+06 ns |
| p99 | 1.28006e+07 ns |
| Worst | 2.01916e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This evidence is same-platform local optimizer trend data only.

## 2026-06-06 Linux 0.4.1 Manual Release No-Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27058264617`, job `Linux
  Clang performance`.
- Commit: `95d733b0094f320d968ac057151e8a0a62e0353e`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux.
- Kernel:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 735.815 ns | 560.325 ns |
| p99 | 5440 ns | 4198 ns |
| Worst | 29295 ns | 44332 ns |

Average C/oracle ratio: `1.31319`.

## 2026-06-06 Linux 0.4.1 Manual Release Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27058264617`, job `Linux
  Clang performance`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux.
- Kernel:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Maximum DoF count: `8`.
- Maximum intermediate waypoint count: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 3.53293e+06 ns |
| p99 | 1.28073e+07 ns |
| Worst | 1.91195e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This evidence is same-platform local optimizer trend data only.

## 2026-06-04 Windows 0.2.3 Maintenance Preparation Run

- Source: Local `0.2.3 - Unreleased` maintenance preparation after adding
  ABI baseline comparison and fixed oracle regressions.
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
| Average | 888.71 ns | 626.09 ns |
| p99 | 6600 ns | 4900 ns |
| Worst | 17700 ns | 29900 ns |

Average C/oracle ratio: `1.41946`.

The same-platform `0.2.2` Windows final-commit baseline ratio was `1.39707`.
This maintenance preparation run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus.

## 2026-06-04 Windows 0.2.3 Local Release Closeout Run

- Source: Local `0.2.3` release closeout gate after bumping the project version
  to `0.2.3`.
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
| Average | 720.33 ns | 661.5 ns |
| p99 | 5300 ns | 5200 ns |
| Worst | 24200 ns | 34300 ns |

Average C/oracle ratio: `1.08893`.

The same-platform `0.2.2` Windows final-commit baseline ratio was `1.39707`.
This local release closeout run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus. Linux `0.2.3` release evidence must be recorded from push CI or the
manual release workflow.

## 2026-06-04 Linux 0.2.3 Manual Release Workflow Run

- Source: GitHub Actions workflow-dispatch run `26956708717`, job
  `Linux Clang performance`.
- Commit: `833dde30417539dd7f09d04734c9fdbd38b8d32e`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: `Release`.
- Generator: `Ninja`.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 700.273 ns | 554.33 ns |
| p99 | 5290 ns | 4178 ns |
| Worst | 43451 ns | 20318 ns |

Average C/oracle ratio: `1.26328`.

The benchmark job completed successfully and is below the release threshold of
average calculation time no worse than `1.5x` the C++ oracle on the same
benchmark corpus.

## 0.2.4 Benchmark Template

Use the same required corpus, `--samples 10000 --seed 1`, for `0.2.4`. Compare
Windows results only against the `0.2.3` Windows benchmark context and Linux
results only against the `0.2.3` Linux benchmark context; do not compare
absolute timings across platforms or runner classes.

## 2026-06-04 Windows 0.2.4 Local Release Closeout Run

- Source: Local `0.2.4` release closeout gate after bumping the project version
  to `0.2.4`.
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
| Average | 984.85 ns | 699.12 ns |
| p99 | 6900 ns | 5200 ns |
| Worst | 28800 ns | 220100 ns |

Average C/oracle ratio: `1.4087`.

The same-platform `0.2.3` Windows release baseline ratio was `1.08893`. This
local release closeout run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus.

## 2026-06-04 Linux 0.2.4 Manual Release Workflow Run

- Source: GitHub Actions workflow-dispatch run `26962279739`, job
  `Linux Clang performance`.
- Commit: `3a65ce47449e4e9fe6708b57c9dc95d7151f2188`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- Kernel/build:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu SMP Wed May 6 22:37:49 UTC 2026 x86_64`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: `Release`.
- Generator: `Ninja`.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 717.002 ns | 555.765 ns |
| p99 | 5350 ns | 4158 ns |
| Worst | 39313 ns | 22522 ns |

Average C/oracle ratio: `1.29012`.

The same-platform `0.2.3` Linux release baseline ratio was `1.26328`. This
manual release workflow run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus.

## 0.2.5 Benchmark Template

Use the same required corpus, `--samples 10000 --seed 1`, for `0.2.5`. Compare
Windows results only against the `0.2.4` Windows benchmark context and Linux
results only against the `0.2.4` Linux benchmark context; do not compare
absolute timings across platforms or runner classes.

## 2026-06-04 Windows 0.2.5 Maintenance Preparation Run

- Source: Local `0.2.5 - Unreleased` maintenance preparation after adding ABI
  baseline comparison, documentation updates, and targeted fixed oracle cases.
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
| Average | 641.05 ns | 510.38 ns |
| p99 | 4700 ns | 3800 ns |
| Worst | 16800 ns | 20600 ns |

Average C/oracle ratio: `1.25602`.

The same-platform `0.2.4` Windows release baseline ratio was `1.4087`. This
maintenance preparation run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus.

## 2026-06-05 Windows 0.2.5 Local Release Closeout Run

- Source: Local `0.2.5` release closeout gate after bumping the project version
  to `0.2.5`.
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
| Average | 752.67 ns | 647.49 ns |
| p99 | 5500 ns | 5100 ns |
| Worst | 64300 ns | 20300 ns |

Average C/oracle ratio: `1.16244`.

The same-platform `0.2.4` Windows release baseline ratio was `1.4087`. This
local release closeout run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus. It is the Windows final pre-`0.3.0` performance baseline for `0.2.x`;
the Linux final baseline is recorded in the manual release workflow section
below.

## 2026-06-05 Linux 0.2.5 Manual Release Workflow Run

- Source: GitHub Actions workflow-dispatch run `26965856552`, job
  `Linux Clang performance`.
- Commit: `c45a6ece69921c26419efcaefe10eed87de03605`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- Kernel/build:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu SMP Wed May 6 22:37:49 UTC 2026 x86_64`.
- CPU identifier: `AMD EPYC 9V74 80-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: `Release`.
- Generator: `Ninja`.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 760.728 ns | 583.765 ns |
| p99 | 5719 ns | 4467 ns |
| Worst | 21011 ns | 21373 ns |

Average C/oracle ratio: `1.30314`.

The same-platform `0.2.4` Linux release baseline ratio was `1.29012`. This
manual release workflow run is within the release threshold of average
calculation time no worse than `1.5x` the C++ oracle on the same benchmark
corpus. It is the Linux final pre-`0.3.0` performance baseline for `0.2.x`.

## 2026-06-05 Linux 0.3.0 Final Push CI Run

- Source: GitHub Actions push CI run `27028896945`, job
  `Linux Clang performance`.
- Commit: `5c7bf60612e6910073fa64e4837a304d063a9d7d`.
- Artifact: `linux-performance`, artifact id `7442364071`, digest
  `sha256:520679364dbd65d0ddbe2b8ac561c898340488fbc0d45f85709595b5da1139f8`.
- Command: `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- Kernel/build:
  `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu SMP Wed May 6 22:37:49 UTC 2026 x86_64`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: `Release`.
- Generator: `Ninja`.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 716.329 ns | 556.371 ns |
| p99 | 5340 ns | 4188 ns |
| Worst | 36839 ns | 25928 ns |

Average C/oracle ratio: `1.2875`.

This final push CI run is within the release threshold of average calculation
time no worse than `1.5x` the C++ oracle on the same benchmark corpus. `0.3.0`
is a hardening release; the Windows release gates for this version covered
local build/test, shared build/test, and ABI/export validation rather than a
separate Windows performance rerun.

## 2026-06-06 Windows 0.4.0-Design Local Alpha No-Waypoint Run

- Source: Local `0.4.0-design` alpha after adding waypoint-aware C ABI,
  local waypoint optimizer tests, Python prototype expansion, Rust alpha
  wrapper, and waypoint alpha benchmark mode.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- C++ compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.

| Metric | C implementation | C++ oracle |
| --- | ---: | ---: |
| Average | 710.08 ns | 738.7 ns |
| p99 | 5000 ns | 5300 ns |
| Worst | 23400 ns | 54500 ns |

Average C/oracle ratio: `0.961256`.

Optional development reruns on the same binary also stayed below the `1.5`
threshold: seed `2` ratio `1.02636`; seed `41` ratio `1.20996`. No-waypoint
behavior remains on the frozen C++ oracle comparison path.

## 2026-06-06 Windows 0.4.0-Design Local Waypoint Alpha Run

- Source: Local `0.4.0-design` waypoint alpha benchmark corpus.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `5`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 1.02414e+06 ns |
| p99 | 5.176e+06 ns |
| Worst | 7.4146e+06 ns |

There is no C++ oracle ratio for this waypoint alpha corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This run is C-only trend evidence for the local optimizer after the
deterministic branch-queue search was enabled.

## 2026-06-06 Windows 0.4.0-Design Local Alpha.4 No-Waypoint Run

- Source: Local `0.4.0-design` alpha.4 pre-stable gate.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 637.57 ns | 515.74 ns |
| p99 | 4700 ns | 3800 ns |
| Worst | 15900 ns | 19600 ns |

Average C/oracle ratio: `1.23622`.

## 2026-06-06 Windows 0.4.0-Design Local Alpha.4 Waypoint Run

- Source: Local `0.4.0-design` alpha.4 waypoint benchmark corpus.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints`
- OS: Windows.
- CPU identifier: `Intel64 Family 6 Model 165 Stepping 5, GenuineIntel`.
- C compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `5`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 1.05448e+06 ns |
| p99 | 6.3575e+06 ns |
| Worst | 7.6835e+06 ns |

There is no C++ oracle ratio for this waypoint alpha corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer.

## 2026-06-06 Linux 0.4.0 Final Push No-Waypoint Run

- Source: GitHub Actions push CI run `27038403450`, job `Linux Clang
  performance`, artifact `7446167572`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --enforce-threshold`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 716.606 ns | 610.796 ns |
| p99 | 5350 ns | 4409 ns |
| Worst | 27141 ns | 40956 ns |

Average C/oracle ratio: `1.17323`.

## 2026-06-06 Linux 0.4.0 Final Push Waypoint Run

- Source: GitHub Actions push CI run `27038403450`, job `Linux Clang
  performance`, artifact `7446167572`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `5`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 1.11858e+06 ns |
| p99 | 5.1804e+06 ns |
| Worst | 8.57995e+06 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer.

## 2026-06-06 Linux 0.4.0 Manual Release No-Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27038538349`, job `Linux Clang
  performance`, artifact `7446219206`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --enforce-threshold`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 9V74 80-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 750.813 ns | 580.8 ns |
| p99 | 5659 ns | 4417 ns |
| Worst | 37195 ns | 25107 ns |

Average C/oracle ratio: `1.29272`.

## 2026-06-06 Linux 0.4.0 Manual Release Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27038538349`, job `Linux Clang
  performance`, artifact `7446219206`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 9V74 80-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `5`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 1.13993e+06 ns |
| p99 | 5.28296e+06 ns |
| Worst | 7.53232e+06 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer.

## 2026-06-06 Windows 0.4.2 Local Release Candidate No-Waypoint Run

- Source: Local `0.4.2` release-candidate gate.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --enforce-threshold`
- OS: Windows.
- CPU identifier: `Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz`.
- C compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 794.17 ns | 586.33 ns |
| p99 | 6100 ns | 4400 ns |
| Worst | 27600 ns | 44700 ns |

Average C/oracle ratio: `1.35448`.

## 2026-06-06 Windows 0.4.2 Local Release Candidate Waypoint Run

- Source: Local `0.4.2` release-candidate waypoint benchmark corpus.
- Command:
  `out\build\windows-clang-ninja-performance\ruckig_c_performance_benchmark.exe --samples 10000 --seed 1 --waypoints`
- OS: Windows.
- CPU identifier: `Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz`.
- C compiler: `clang 21.1.8`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Max DoF: `8`.
- Max intermediate positions: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 3.52466e+06 ns |
| p99 | 1.4749e+07 ns |
| Worst | 2.1875e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This run is C-only trend evidence for the local optimizer.

## 2026-06-06 Linux 0.4.2 Push CI No-Waypoint Run

- Source: GitHub Actions push CI run `27063738903`, job `Linux Clang
  performance`, artifact `7454827710`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --enforce-threshold`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 9V74 80-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 754.922 ns | 595.274 ns |
| p99 | 5658 ns | 4486 ns |
| Worst | 30075 ns | 49553 ns |

Average C/oracle ratio: `1.26819`.

## 2026-06-06 Linux 0.4.2 Push CI Waypoint Run

- Source: GitHub Actions push CI run `27063738903`, job `Linux Clang
  performance`, artifact `7454827710`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 9V74 80-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Max DoF: `8`.
- Max intermediate positions: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 3.63244e+06 ns |
| p99 | 1.31755e+07 ns |
| Worst | 1.8655e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This run is C-only trend evidence for the local optimizer.

## 2026-06-06 Linux 0.4.2 Pre-Tag Manual Workflow No-Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27064593851`, job `Linux Clang
  performance`.
- Commit: `2ec935a7cb2ed42eea9d437a5e62daf6c9a91102`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --enforce-threshold`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 604.085 ns | 456.656 ns |
| p99 | 4487 ns | 3505 ns |
| Worst | 25588 ns | 17045 ns |

Average C/oracle ratio: `1.32285`.

## 2026-06-06 Linux 0.4.2 Pre-Tag Manual Workflow Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27064593851`, job `Linux Clang
  performance`.
- Commit: `2ec935a7cb2ed42eea9d437a5e62daf6c9a91102`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux, GitHub-hosted Ubuntu runner.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Max DoF: `8`.
- Max intermediate positions: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 2.88561e+06 ns |
| p99 | 1.04851e+07 ns |
| Worst | 1.14663e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This run is C-only trend evidence for the local optimizer.

## 2026-06-06 Linux 0.4.2 Tag Manual Workflow No-Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27064919699`, job `Linux Clang
  performance`.
- Ref: `v0.4.2`.
- Commit: `002795293006c8205ade408706dadd70d0567f87`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --enforce-threshold`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- C++ compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Release threshold: average C/oracle ratio `<= 1.5`.

| Metric | C | C++ oracle |
| --- | ---: | ---: |
| Average | 726.891 ns | 556.996 ns |
| p99 | 5330 ns | 4228 ns |
| Worst | 44072 ns | 28403 ns |

Average C/oracle ratio: `1.30502`.

## 2026-06-06 Linux 0.4.2 Tag Manual Workflow Waypoint Run

- Source: GitHub Actions workflow-dispatch run `27064919699`, job `Linux Clang
  performance`.
- Ref: `v0.4.2`.
- Commit: `002795293006c8205ade408706dadd70d0567f87`.
- Command:
  `./build-perf/ruckig_c_performance_benchmark --samples 10000 --seed 1 --waypoints`.
- OS: Linux.
- Kernel: `Linux runnervm3jyl0 6.17.0-1015-azure #15~24.04.1-Ubuntu`.
- CPU identifier: `AMD EPYC 7763 64-Core Processor`.
- C compiler: `Ubuntu clang version 18.1.3 (1ubuntu1)`.
- CMake build type: Release.
- Generator: Ninja.
- Seed: `1`.
- Samples: `10000`.
- Waypoint case count: `10`.
- Max DoF: `8`.
- Max intermediate positions: `3`.

| Metric | C waypoint optimizer |
| --- | ---: |
| Average | 3.52565e+06 ns |
| p99 | 1.27822e+07 ns |
| Worst | 1.83234e+07 ns |

There is no C++ oracle ratio for this waypoint corpus because the frozen
Ruckig Community `0.17.3` baseline does not contain a local global waypoint
optimizer. This run is C-only trend evidence for the local optimizer on the
published tag.

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
0.2.4 same-platform baseline ratio:
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
0.2.4 same-platform baseline ratio:
Release threshold:
Result:
```
