# Package Manager Feasibility

This is a frozen feasibility note for possible future packaging work. It does
not implement or promise package-manager recipes for `0.2.x` or the active
`0.3.0-design` roadmap. Package-manager work is downgraded to a long-term
optional project and may remain frozen indefinitely.

## Current Verified Consumption Paths

- Installed CMake package with `find_package(ruckig_c CONFIG REQUIRED)`.
- Unix `pkg-config` consumer.
- Unix shared install-tree consumer through CMake and `pkg-config`.
- Windows manual static consumer with `RUCKIG_C_STATIC_DEFINE`.
- Windows DLL/import-library consumer without `RUCKIG_C_STATIC_DEFINE`.

These paths remain the active maintenance priority. They are the supported
downstream integration surface unless a separate package-manager project is
accepted.

## Frozen Candidate Notes

The following notes preserve earlier feasibility reasoning. They are not an
active implementation queue.

### vcpkg

vcpkg was the first package-manager candidate because it directly tests Windows
static and DLL consumption. It is now long-term optional. Preconditions for
unfreezing it as a separate project would include:

- Stable public C ABI through the `v0.2.5` pre-`0.3.0` baseline.
- Clean public exported-symbol evidence on Windows and Linux against
  `docs/abi/public-symbols.txt`.
- Documented static and shared linkage behavior.
- A portfile that does not modify `original/ruckig-main`.

Prototype scope if a future packaging project is accepted:

- Record the required install layout for headers, CMake config files, and
  optional pkg-config metadata.
- Validate both static and shared builds on Windows, Linux, and macOS.
- Confirm that Windows static consumers receive `RUCKIG_C_STATIC_DEFINE`
  through target metadata and that DLL consumers do not define it.
- Confirm the portfile does not build or package the frozen C++ oracle as a
  runtime dependency.

See `docs/design/vcpkg_feasibility_prototype.md` and `prototypes/vcpkg/` for
the executable overlay prototype. That prototype is still outside release scope
and must not be treated as a supported recipe or active roadmap item. Current
local evidence verifies Windows `x64-windows` and `x64-windows-static`; Linux
and macOS remain unverified.

### Conan

Conan would be useful for mixed compiler and profile coverage. Preconditions:

- Clear package options for static/shared builds.
- CMake package metadata verified from the install tree.
- A test package that exercises `ruckig_c::ruckig_c`.

Conan is not an active priority. If revisited, it should reuse the install-tree
and option decisions proven by the existing consumer paths rather than
introducing a separate package shape. The first Conan note should define
package metadata, static/shared options, and test-package coverage; it should
not imply an active release commitment.

### Homebrew

Homebrew would be useful after Unix install-tree and shared library behavior is
stable. Preconditions:

- macOS C-only CI remains green.
- Shared-library install names and CMake package paths are reviewed.
- Formula test uses the C API directly.

Homebrew is not an active priority. It should wait until Linux and macOS
shared install-tree evidence is stable and a separate packaging project has
accepted the maintenance cost.

### FetchContent

FetchContent could be evaluated for source consumers that build `ruckig_c`
directly. Preconditions:

- Subproject option behavior is documented.
- Test and oracle options remain opt-in for consumers.
- No dependency on the frozen C++ oracle at runtime.

FetchContent is documentation-only and long-term optional. A future note could
show the recommended `FetchContent_MakeAvailable` or `add_subdirectory`
pattern, but no recipe or release promise is attached to active `0.3.0-design`
work.

### Vendored Subdirectory

Vendored subdirectory use could be viable if consumers can add this repository
with `add_subdirectory` and link `ruckig_c::ruckig_c`. Preconditions:

- Options are namespaced and do not force examples, tests, oracle builds, or
  performance benchmarks into parent projects.
- Static/shared export macros behave correctly in parent builds.

## Active Scope And Freeze Policy

1. Keep validating existing CMake, `pkg-config`, static, DLL, and shared
   install-tree consumers from the `v0.2.5` baseline.
2. Clean the shared-library public export surface and compare it against
   `docs/abi/public-symbols.txt`.
3. Keep package-manager recipes and new package-manager prototypes frozen
   unless a separate packaging project is explicitly accepted.
4. If unfrozen later, reopen the cross-platform static/shared matrix, runtime
   dependency checks, package metadata shape, and CI maintenance budget before
   adding any supported recipe.

No package-manager recipe should be added as part of `0.2.3` release closeout.
No package-manager recipe should be added as part of `0.2.5` maintenance work
either. No package-manager recipe or new prototype is part of active
`0.3.0-design`; this document is retained as frozen feasibility evidence and
future project scoping only.
