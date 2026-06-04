# Packaging and Consumer Notes

This document records supported consumer paths for `ruckig_c 0.2.x`. It is a
maintenance checklist, not a commitment to package-manager recipes.

## Installed CMake Package

Install the library and consume the exported CMake target:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix /path/to/prefix
```

```cmake
find_package(ruckig_c CONFIG REQUIRED)
add_executable(app main.c)
target_link_libraries(app PRIVATE ruckig_c::ruckig_c)
```

The exported target is `ruckig_c::ruckig_c`. Static CMake consumers receive
`RUCKIG_C_STATIC_DEFINE` from the target and should not add it manually.

## pkg-config

On Unix-like systems with `pkg-config`, use the installed `.pc` file:

```sh
cc main.c $(pkg-config --cflags --libs ruckig_c) -o app
```

For static Unix consumers, use:

```sh
cc main.c $(pkg-config --cflags --static --libs ruckig_c) -o app
```

The existing `ruckig_c_pkg_config_consumer` CTest verifies the installed
pkg-config path on Unix systems that provide `pkg-config`.

## Consumer Matrix Summary

| Toolchain/path | Status |
| --- | --- |
| clang static | verified |
| clang DLL | verified |
| clang-cl static | verified in CI |
| clang-cl DLL | verified in CI |
| MSVC cl static | documented, not yet CI-verified |
| MSVC cl DLL | documented, not yet CI-verified |
| MinGW static | not yet verified |
| MinGW DLL | not yet verified |
| CMake installed package | verified |
| pkg-config | verified on Unix CI |
| shared install-tree | verified on Unix CI |

## Windows Manual Static Link

When manually linking a static Windows build without CMake, define
`RUCKIG_C_STATIC_DEFINE` before including the public header:

```sh
clang -std=c99 -DRUCKIG_C_STATIC_DEFINE -I path\to\include main.c path\to\ruckig_c.lib
```

CMake target consumers should not define this manually because the exported
static target propagates it. DLL consumers must not define
`RUCKIG_C_STATIC_DEFINE`.

Manual smoke command template from a release-check build:

```powershell
clang -std=c99 -DRUCKIG_C_STATIC_DEFINE -I include -c examples\c\00_minimal_offline.c -o build_release_check_ninja\manual_static_consumer.obj
clang -nostartfiles -nostdlib -fuse-ld=lld-link build_release_check_ninja\manual_static_consumer.obj build_release_check_ninja\ruckig_c.lib -Xlinker /subsystem:console -o build_release_check_ninja\manual_static_consumer.exe -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -loldnames
.\build_release_check_ninja\manual_static_consumer.exe
```

The second command mirrors the Windows clang/Ninja release-check link mode.
Toolchains that use a different C runtime may need equivalent system-runtime
libraries for math functions such as `cbrt`.

Where the Windows clang release-check toolchain supports the same driver and
link mode, CTest enables a repeatable smoke test:

```powershell
ctest --test-dir build_release_check_ninja -R ruckig_c_windows_manual_static_consumer --output-on-failure
```

The automated smoke uses `examples/c/00_minimal_offline.c`, defines
`RUCKIG_C_STATIC_DEFINE`, links the static `ruckig_c` archive, and runs the
resulting executable.

Current Windows toolchain status:

- `clang` with the MSVC linker environment: verified by local release-check
  CTest.
- `clang-cl`: verified for C-only CMake target consumption and standalone
  manual static-link smoke in CI.
- MSVC `cl`: planned as a standalone manual static-link smoke; not yet part of
  routine CI.
- MinGW: not yet verified.

Planned MSVC `cl` standalone static smoke:

```powershell
cl /nologo /std:c11 /DRUCKIG_C_STATIC_DEFINE /I include examples\c\00_minimal_offline.c build_release_check_ninja\ruckig_c.lib /Fe:build_release_check_ninja\msvc_static_consumer.exe
.\build_release_check_ninja\msvc_static_consumer.exe
```

This smoke must run from a Developer Command Prompt or an equivalent
environment where `cl`, the Windows SDK, and the C runtime libraries are
available. It is a manual or CMake-scripted gate until CI evidence proves the
toolchain is stable enough for routine execution.

## DLL Consumers

DLL consumers include the same public header and link against the import
library. Do not define `RUCKIG_C_STATIC_DEFINE`; the header will import public
symbols on Windows. The DLL must be discoverable at run time, for example by
placing it next to the executable or adding its directory to `PATH`.

The shared-library release gate covers the DLL/import-library build path and
runs header consumers plus examples against the shared library.

Manual DLL consumer smoke command template from a shared release-check build:

```powershell
clang -std=c99 -I include examples\c\00_minimal_offline.c build_release_check_shared\ruckig_c.lib -o build_release_check_shared\dll_consumer.exe
$env:PATH = (Resolve-Path build_release_check_shared).Path + ";" + $env:PATH
.\build_release_check_shared\dll_consumer.exe
```

The exact import-library filename may vary by generator and toolchain; use the
import library produced next to `ruckig_c.dll`.

Where the Windows clang shared release-check toolchain supports the same driver
mode, CTest enables a repeatable DLL smoke test:

```powershell
ctest --test-dir build_release_check_shared -R ruckig_c_windows_dll_consumer --output-on-failure
```

The automated smoke compiles `examples/c/00_minimal_offline.c` without
`RUCKIG_C_STATIC_DEFINE`, links the import library, adds the DLL directory to
the process `PATH`, and runs the executable.

Current Windows DLL consumer status:

- `clang` with the MSVC linker environment: verified by local shared
  release-check CTest.
- `clang-cl`: verified through a shared C-only CI job that builds the DLL,
  links the import library, updates the process `PATH`, and runs the consumer.
- MSVC `cl`: planned as a standalone DLL/import-library smoke; not yet part of
  routine CI.
- MinGW: not yet verified.

Planned MSVC `cl` standalone DLL smoke:

```powershell
cl /nologo /std:c11 /I include examples\c\00_minimal_offline.c build_release_check_shared\ruckig_c.lib /Fe:build_release_check_shared\msvc_dll_consumer.exe
$env:PATH = (Resolve-Path build_release_check_shared).Path + ";" + $env:PATH
.\build_release_check_shared\msvc_dll_consumer.exe
```

Do not define `RUCKIG_C_STATIC_DEFINE` for this DLL consumer path.

MinGW feasibility remains separate from MSVC-style consumers. It should be
validated as two independent smokes, one static and one DLL/import-library,
before documentation or CI implies support.

## Shared Install-Tree Verification

For shared install-tree checks:

```sh
cmake -S . -B build-shared -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-shared
cmake --install build-shared --prefix /tmp/ruckig-c-prefix
```

Then verify both installed CMake and pkg-config consumers against the install
prefix. On Windows, ensure the DLL directory is present in the consumer process
environment.

On Unix systems with `pkg-config`, the `ruckig_c_shared_install_consumer` CTest
configures a temporary `BUILD_SHARED_LIBS=ON` build, installs it, then verifies
both installed CMake and pkg-config consumers against that shared install tree.

## Future Packaging

The likely package-manager integration paths are vcpkg, Conan, Homebrew,
CMake FetchContent, and vendored subdirectory use. These should be evaluated
after `0.2.x` has completed at least one patch cycle with stable public ABI
evidence.

See `docs/design/package_manager_feasibility.md` for the recommended evaluation order.
No vcpkg, Conan, Homebrew, FetchContent, or vendored-subdirectory recipe is part
of the current `0.2.x` release scope.
