# Post-v0.16.0 Public Header Doxygen Checklist

This checklist records the public header documentation slice. It adds Doxygen
comments to the public C header without changing declarations, enum values,
struct layout, public symbols, version metadata, or release state.

## Scope

- [x] Added file-level public API conventions for ownership, opaque handles,
  destroy `NULL` safety, and accessor lifetime.
- [x] Documented public enums, diagnostics structs, tracking diagnostics, and
  opaque handle typedefs.
- [x] Documented lifecycle, validation, calculate/update, diagnostics,
  input/output/trajectory, waypoint/per-section, tracking, and tracking
  sequence continuation declarations.
- [x] Documented `ruckig_output_pass_to_input` DoF mismatch no-op behavior.
- [x] Kept public declarations and ABI unchanged.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics\|ruckig_c_cmake_consumer"` | Passed; 4/4 tests |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_verify_public_symbols` | Passed; generated public symbol allowlist matches the public header |
| `cmake --build out\build\windows-clang-ninja-shared --target ruckig_c_compare_public_exported_symbols` | Passed; public exported symbols match the approved allowlist |
| ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
