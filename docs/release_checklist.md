# Ruckig C 0.1.0 Release Checklist

Run these checks from a clean worktree before tagging `0.1.0`.

## Required Gates

- [x] `git status --short` is clean after committing the release evidence.
- [x] Static CMake build passes on Windows clang/Ninja.
- [x] Shared-library CMake build passes on Windows clang/Ninja.
- [x] C and C++ public header consumer tests pass on Windows clang/Ninja.
- [x] All C examples build and run on Windows clang/Ninja.
- [x] Source-level allocation audit passes on Windows clang/Ninja.
- [x] Fixed C++ oracle suite passes on Windows clang/Ninja.
- [x] Random development oracle passes with `--random 100000 --seed 1` on Windows clang/Ninja.
- [x] Manual release oracle passes with `--random 1000000 --seed 1` on Windows clang/Ninja.
- [x] Linux Clang ASan+UBSan CTest passes.
- [x] Linux Valgrind/memcheck passes for `ruckig_c_tests` and one example.
- [x] Linux performance benchmark result is recorded in `docs/performance_report.md`.
- [x] Installed CMake consumer smoke test passes on Windows clang/Ninja.
- [x] Installed pkg-config consumer smoke test passes on Linux.
- [x] `LICENSE` and `CHANGELOG.md` are present and accurate.

## Current Windows Evidence

Recorded on 2026-06-03 with LLVM clang 21.1.8 and Visual Studio bundled Ninja.
The final local release pass in this workspace was run at pre-tag commit
`86056d2d527396f8758faf841b2d3cad9133e2ad` before the documentation-only
release closeout commit:

- Static build directory: `build_release_check_ninja`.
- Static command: `ctest --test-dir build_release_check_ninja --output-on-failure -E ruckig_c_oracle_random_release`.
- Static result: `100% tests passed, 0 tests failed out of 14`.
- Shared build directory: `build_release_check_shared`.
- Shared command: `ctest --test-dir build_release_check_shared --output-on-failure -E ruckig_c_oracle_random_release`.
- Shared result: `100% tests passed, 0 tests failed out of 13`.
- Release random command: `ctest --test-dir build_release_check_ninja -R ruckig_c_oracle_random_release --output-on-failure`.
- Release random result: `100% tests passed, 0 tests failed out of 1`.
- Final local rerun result: static `14/14`, shared `13/13`, and release random
  `1/1` all passed on 2026-06-03.

## Current GitHub Actions Evidence

Recorded on 2026-06-03 from CI run `26861662259` at commit `5d5743c`.
This is the latest recorded cross-platform release evidence available in this
workspace; repeat the workflow after any post-evidence source change before
publishing the final tag:

- CI run result: completed successfully.
- Linux Clang ASan UBSan job: passed.
- Linux Valgrind job: passed, including CTest, `ruckig_c_tests` under Valgrind,
  and `example-ruckig-c-02-position-offline` under Valgrind.
- Linux Clang performance job: passed and uploaded `linux-performance`.
- Linux performance result is recorded in `docs/performance_report.md`.
- Linux pkg-config consumer smoke test passes as part of the Unix CTest suite.

## Recommended Commands

Static build:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_RUCKIG_C_ORACLE_TESTS=ON -DBUILD_RUCKIG_C_PERFORMANCE_TESTS=ON
cmake --build build-release
ctest --test-dir build-release --output-on-failure -E ruckig_c_oracle_random_release
```

Manual release oracle:

```sh
ctest --test-dir build-release -R ruckig_c_oracle_random_release --output-on-failure
```

Shared build:

```sh
cmake -S . -B build-shared -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DBUILD_RUCKIG_C_ORACLE_TESTS=ON
cmake --build build-shared
ctest --test-dir build-shared --output-on-failure -E ruckig_c_oracle_random_release
```

Linux sanitizer build:

```sh
cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DRUCKIG_C_ENABLE_ASAN=ON -DRUCKIG_C_ENABLE_UBSAN=ON
cmake --build build-san
ctest --test-dir build-san --output-on-failure
```

Linux Valgrind spot checks:

```sh
valgrind --error-exitcode=1 --leak-check=full ./build-release/ruckig_c_tests
valgrind --error-exitcode=1 --leak-check=full ./build-release/example-ruckig-c-02-position-offline
```

Performance benchmark:

```sh
./build-release/ruckig_c_performance_benchmark --samples 10000 --seed 1
```

GitHub Actions also runs a `Linux Clang performance` job that uploads
`linux-performance.txt`. Copy that output into `docs/performance_report.md`
before tagging.

## Release Notes

Use `docs/release_notes_0.1.0.md` as the base text for the GitHub Release.
Before publishing, add the final tagged commit hash and final CI run id to the
release page. The exact final commit cannot be self-recorded inside the same
commit that carries this checklist; resolve it from `v0.1.0^{commit}` after
tagging.

## Scope Lock

Do not add waypoints, per-section constraints, cloud behavior, language
bindings, per-DoF control/synchronization overrides, broad epsilon rewrites, or
large algorithm-file splits before the `0.1.0` tag.
