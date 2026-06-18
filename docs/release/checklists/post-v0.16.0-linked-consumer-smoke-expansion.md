# Post-v0.16.0 Linked Consumer Smoke Expansion Checklist

This checklist records the linked-library consumer smoke expansion. It extends
the public-header-only executable that links the `ruckig_c` target, without
restructuring the white-box `ruckig_c_tests` binary or changing public ABI.

## Scope

- [x] Kept `test/c/linked_library_smoke.c` public-header-only.
- [x] Kept `ruckig_c_linked_library_smoke` linked against the `ruckig_c` target.
- [x] Covered a no-waypoint calculate/update/public diagnostics workflow.
- [x] Added a minimum waypoint calculate workflow.
- [x] Added a minimum Fast tracking update plus public diagnostics getter
  workflow.
- [x] Avoided private diagnostics, candidate order, score internals, queue
  internals, solver internals, and test-binary restructuring.

## Boundary

- [x] No public header declaration change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or release change.
- [x] No upstream baseline or visualization asset change.

## Verification

| Command | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; rebuilt `ruckig_c_linked_library_smoke` |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_linked_library_smoke"` | Passed 1/1 |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_linked_library_smoke\|ruckig_c_tests\|ruckig_c_cmake_consumer\|ruckig_c_public_diagnostics"` | Passed 5/5 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed; rebuilt `ruckig_c_linked_library_smoke` |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_linked_library_smoke\|ruckig_c_shared_install_consumer\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed 3/3 matching tests; `ruckig_c_shared_install_consumer` is not configured in this Windows shared preset |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Empty |
| `git diff --check` | Passed; CRLF normalization warnings only |
