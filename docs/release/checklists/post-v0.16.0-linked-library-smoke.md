# Post-v0.16.0 Linked Library Smoke Checklist

This checklist records the linked-library smoke slice. It adds a tiny test
executable that includes only the public header and links the `ruckig_c` target
instead of compiling implementation sources directly. The existing white-box
`ruckig_c_tests` target remains unchanged.

## Scope

- [x] Added `test/c/linked_library_smoke.c`.
- [x] Registered `ruckig_c_linked_library_smoke` as an executable linked with
  `ruckig_c`.
- [x] Added CTest `ruckig_c_linked_library_smoke`.
- [x] Covered create/input/output/trajectory lifecycle, calculate with public
  diagnostics, online update, and `ruckig_output_pass_to_input`.
- [x] Kept the test on the public C header only.
- [x] Kept existing `ruckig_c_tests` white-box structure unchanged.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_linked_library_smoke\|ruckig_c_tests\|ruckig_c_cmake_consumer"` | Passed; 3/3 tests |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_linked_library_smoke\|ruckig_c_windows_dll_consumer\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed; 4/4 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
