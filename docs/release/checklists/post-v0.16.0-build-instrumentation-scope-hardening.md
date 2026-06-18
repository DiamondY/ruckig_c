# Post-v0.16.0 Build Instrumentation Scope Hardening Checklist

This checklist records the build instrumentation scope hardening slice. It
keeps sanitizer and coverage instrumentation opt-in for the local targets that
request it without propagating CMake target link options to downstream
consumers.

## Scope

- [x] Changed sanitizer link options from `PUBLIC` to `PRIVATE` for Clang/GNU
  and MSVC targets.
- [x] Changed coverage link options from `PUBLIC` to `PRIVATE`.
- [x] Kept sanitizer and coverage compile options private.
- [x] Kept pkg-config `Libs.private` sanitizer handling for opt-in
  instrumented pkg-config builds.
- [x] Did not change public ABI, public headers, workflows, release metadata,
  package recipes, upstream baseline, or visualization assets.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; reconfigured, no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_cmake_consumer\|ruckig_c_public_diagnostics"` | Passed; 4/4 tests |
| `cmake --build --preset windows-clang-ninja-shared` | Passed; reconfigured, no work to do |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_shared_install_consumer\|ruckig_c_public_diagnostics"` | Passed; 3/3 matched tests on Windows; `ruckig_c_shared_install_consumer` is not registered in this Windows preset |
| Windows shared consumer follow-up | Passed; `ruckig_c_cmake_consumer` and `ruckig_c_windows_dll_consumer` 2/2 |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
