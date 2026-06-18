# Post-v0.16.0 CMake Package Version Policy Checklist

This checklist records the CMake package version policy tightening slice. It is
a packaging metadata and consumer-smoke change only: no public C ABI, workflow,
version metadata, release tag, upstream baseline, or visualization asset is
changed.

## Scope

- [x] Changed installed CMake package version compatibility from
  `SameMajorVersion` to `SameMinorVersion`.
- [x] Kept the existing unversioned installed CMake consumer smoke.
- [x] Added a versioned installed CMake consumer smoke for
  `find_package(ruckig_c 0.16 CONFIG REQUIRED)`.
- [x] Documented that `0.16.x` patch releases remain compatible for CMake
  package matching, while future `0.17` or `0.18` packages are not accepted for
  `0.16` requests by default.

## Boundary

- [x] No public header declaration change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or release change.
- [x] No wrapper/package-manager publication change.
- [x] No upstream baseline or visualization asset change.

## Verification

| Command | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; CMake regenerated and Ninja reported no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_cmake_consumer\|ruckig_c_cmake_consumer_versioned\|ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed 5/5 |
| `cmake --build --preset windows-clang-ninja-shared` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja-shared --output-on-failure -R "ruckig_c_shared_install_consumer\|ruckig_c_cmake_consumer_versioned\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed 3/3 matching tests; `ruckig_c_shared_install_consumer` is not configured in this Windows shared preset |
| `git diff -- include/ruckig_c/ruckig.h docs/abi/public-symbols.txt docs/abi/public-symbol-exceptions.txt docs/abi/exceptions.md .github/workflows/ci.yml` | Empty |
| `git diff -- original/ruckig-main docs/assets/visualization` | Empty |
| `git diff --check` | Passed; CRLF normalization warnings only |
