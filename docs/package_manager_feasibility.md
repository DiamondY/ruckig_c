# Package Manager Feasibility

This is a feasibility note for future packaging work. It does not implement or
promise package-manager recipes for `0.2.x`.

## Current Verified Consumption Paths

- Installed CMake package with `find_package(ruckig_c CONFIG REQUIRED)`.
- Unix `pkg-config` consumer.
- Unix shared install-tree consumer through CMake and `pkg-config`.
- Windows manual static consumer with `RUCKIG_C_STATIC_DEFINE`.
- Windows DLL/import-library consumer without `RUCKIG_C_STATIC_DEFINE`.

These paths should remain the priority for `0.2.x` patch releases.

## Candidate Package Managers

### vcpkg

vcpkg is likely the first package-manager candidate because it directly tests
Windows static and DLL consumption. Preconditions:

- Stable public C ABI through at least one additional `0.2.x` patch release.
- Repeatable exported-symbol evidence on Windows and Linux.
- Documented static and shared linkage behavior.
- A portfile that does not modify `original/ruckig-main`.

### Conan

Conan is useful for mixed compiler and profile coverage. Preconditions:

- Clear package options for static/shared builds.
- CMake package metadata verified from the install tree.
- A test package that exercises `ruckig_c::ruckig_c`.

### Homebrew

Homebrew is useful after Unix install-tree and shared library behavior is
stable. Preconditions:

- macOS C-only CI remains green.
- Shared-library install names and CMake package paths are reviewed.
- Formula test uses the C API directly.

### FetchContent

FetchContent can be evaluated for source consumers that build `ruckig_c`
directly. Preconditions:

- Subproject option behavior is documented.
- Test and oracle options remain opt-in for consumers.
- No dependency on the frozen C++ oracle at runtime.

### Vendored Subdirectory

Vendored subdirectory use is viable if consumers can add this repository with
`add_subdirectory` and link `ruckig_c::ruckig_c`. Preconditions:

- Options are namespaced and do not force examples, tests, oracle builds, or
  performance benchmarks into parent projects.
- Static/shared export macros behave correctly in parent builds.

## Recommended Order

1. Keep validating existing CMake, `pkg-config`, static, DLL, and shared
   install-tree consumers through `0.2.x`.
2. Prototype vcpkg packaging after ABI baseline comparison has been used in a
   patch release.
3. Evaluate Conan after vcpkg because it benefits from the same install-tree
   and option model.
4. Evaluate Homebrew after macOS shared-install details are reviewed.
5. Document FetchContent and vendored subdirectory patterns only after package
   options have stayed stable through another patch cycle.

No package-manager recipe should be added as part of `0.2.3` release closeout.
