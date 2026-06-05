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

- Stable public C ABI through the `v0.2.5` pre-`0.3.0` baseline.
- Clean public exported-symbol evidence on Windows and Linux against
  `docs/abi/public-symbols.txt`.
- Documented static and shared linkage behavior.
- A portfile that does not modify `original/ruckig-main`.

Prototype scope for a future branch:

- Record the required install layout for headers, CMake config files, and
  optional pkg-config metadata.
- Validate both static and shared builds on Windows, Linux, and macOS.
- Confirm that Windows static consumers receive `RUCKIG_C_STATIC_DEFINE`
  through target metadata and that DLL consumers do not define it.
- Confirm the portfile does not build or package the frozen C++ oracle as a
  runtime dependency.

See `docs/design/vcpkg_feasibility_prototype.md` and `prototypes/vcpkg/` for
the executable overlay prototype. That prototype is still outside release scope
and must not be treated as a supported recipe. Current local evidence verifies
Windows `x64-windows` and `x64-windows-static`; Linux and macOS remain
unverified.

### Conan

Conan is useful for mixed compiler and profile coverage. Preconditions:

- Clear package options for static/shared builds.
- CMake package metadata verified from the install tree.
- A test package that exercises `ruckig_c::ruckig_c`.

Conan is second priority. It should reuse the install-tree and option decisions
proven by the vcpkg prototype rather than introducing a separate package shape.
The first Conan note should define package metadata, static/shared options, and
test-package coverage; it should not promise a recipe in a `0.2.x` patch
release.

### Homebrew

Homebrew is useful after Unix install-tree and shared library behavior is
stable. Preconditions:

- macOS C-only CI remains green.
- Shared-library install names and CMake package paths are reviewed.
- Formula test uses the C API directly.

Homebrew is third priority. It should wait until Linux and macOS shared
install-tree evidence is stable and the package metadata used by vcpkg/Conan is
not still changing.

### FetchContent

FetchContent can be evaluated for source consumers that build `ruckig_c`
directly. Preconditions:

- Subproject option behavior is documented.
- Test and oracle options remain opt-in for consumers.
- No dependency on the frozen C++ oracle at runtime.

FetchContent is documentation-only for now. A future note can show the
recommended `FetchContent_MakeAvailable` or `add_subdirectory` pattern, but no
recipe or release promise should be attached to `0.2.x`.

### Vendored Subdirectory

Vendored subdirectory use is viable if consumers can add this repository with
`add_subdirectory` and link `ruckig_c::ruckig_c`. Preconditions:

- Options are namespaced and do not force examples, tests, oracle builds, or
  performance benchmarks into parent projects.
- Static/shared export macros behave correctly in parent builds.

## Recommended Order

1. Keep validating existing CMake, `pkg-config`, static, DLL, and shared
   install-tree consumers from the `v0.2.5` baseline.
2. Clean the shared-library public export surface and compare it against
   `docs/abi/public-symbols.txt`.
3. Prototype vcpkg packaging after ABI/export hygiene and MSVC `cl` consumer
   smoke have repeatable evidence.
4. Evaluate Conan after vcpkg because it benefits from the same install-tree
   and option model.
5. Evaluate Homebrew after macOS shared-install details are reviewed.
6. Document FetchContent and vendored subdirectory patterns only after package
   options have stayed stable through another patch cycle.

No package-manager recipe should be added as part of `0.2.3` release closeout.
No package-manager recipe should be added as part of `0.2.5` maintenance work
either; this queue is limited to feasibility notes and future project scoping.
