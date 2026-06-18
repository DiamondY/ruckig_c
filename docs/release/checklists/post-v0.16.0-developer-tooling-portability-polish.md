# Post-v0.16.0 Developer Tooling Portability Polish Checklist

This checklist records the local developer tooling portability slice. It fixes
portable preset guidance and clang-tidy configuration usability without running
formatter rewrites, adding a default CI gate, changing public C ABI, or changing
release state.

## Scope

- [x] Updated the `portable-ninja` preset description to refer to shell compiler
  discovery, `CC`/`CXX`, or explicit CMake compiler cache variables.
- [x] Updated packaging guidance so `CMAKE_C_COMPILER` and
  `CMAKE_CXX_COMPILER` are documented as cache variables, not normal
  environment variables.
- [x] Updated `.clang-tidy` `HeaderFilterRegex` to match absolute paths and
  Windows or POSIX path separators.
- [x] Removed an unsupported `.clang-tidy` key found by the local tool.
- [x] Kept clang-tidy and formatting as local/manual evidence only.

## Boundary

- [x] No formatter rewrite.
- [x] No workflow change.
- [x] No public header declaration, ABI allowlist, version metadata, tag, or
  release change.
- [x] No upstream baseline or visualization asset change.

## Verification

| Command | Result |
| --- | --- |
| `cmake --list-presets` | Passed; `portable-ninja` is listed |
| `clang-tidy test/c/linked_library_smoke.c --quiet -- -std=c99 -Iinclude` | Passed; configuration parsed and the command exited 0 with warning summary only |
| `cmake --build --preset windows-clang-ninja` | Passed; Ninja reported no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed 3/3 |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Empty |
| `git diff --check` | Passed; CRLF normalization warnings only |
