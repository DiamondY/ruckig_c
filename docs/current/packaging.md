# Packaging and Consumer Notes

This document records supported consumer paths for `ruckig_c`. It is a
maintenance checklist, not a commitment to package-manager recipes.

## CMake Presets

The repository keeps two kinds of local presets:

- `windows-clang-ninja*` presets are maintainer-verified Windows local presets.
  They intentionally point at the currently verified LLVM and Visual Studio
  Ninja paths on the maintainer machine.
- `dev`, `portable-ninja`, `release`, `shared`, and `oracle` avoid hardcoded
  compiler paths. Use these when the current shell or environment provides the
  compiler and generator.

For a portable Ninja build:

```sh
cmake --preset portable-ninja
cmake --build --preset portable-ninja
ctest --preset portable-ninja
```

If Ninja or the compiler is not discoverable, set `CMAKE_C_COMPILER` and
`CMAKE_CXX_COMPILER` in the environment or use normal `cmake -S/-B` arguments.

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
The installed CMake package version file uses `SameMinorVersion`
compatibility: `0.16.x` patch releases are compatible for `find_package`
version matching, but future `0.17` or `0.18` packages are not accepted as a
match for a `0.16` request by default. The `ruckig_c_cmake_consumer_versioned`
CTest covers `find_package(ruckig_c 0.16 CONFIG REQUIRED)`.

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
| MSVC cl static | optional CTest gate, locally verified; not routine CI |
| MSVC cl DLL | optional CTest gate, locally verified; not routine CI |
| MinGW static | locally verified; routine CI gate added |
| MinGW DLL | locally verified; routine CI gate added |
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
- MSVC `cl`: locally verified with the opt-in standalone static-link CTest;
  not part of routine CI. The repeatable gate is available when configured
  with `-DRUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE=ON`.
- MinGW: locally verified with GCC 15.2.0 through the Windows manual static
  consumer CTest. A dedicated MSYS2 MinGW64 routine CI gate now covers this
  path.

MSVC `cl` standalone static smoke:

```powershell
cl /nologo /std:c11 /DRUCKIG_C_STATIC_DEFINE /I include /c examples\c\00_minimal_offline.c /Fo:build_release_check_ninja\msvc_static_consumer.obj
link /nologo build_release_check_ninja\msvc_static_consumer.obj build_release_check_ninja\ruckig_c.lib /OUT:build_release_check_ninja\msvc_static_consumer.exe
.\build_release_check_ninja\msvc_static_consumer.exe
```

This smoke must run from a Developer Command Prompt or an equivalent
environment where `cl`, the Windows SDK, and the C runtime libraries are
available. The CMake-scripted gate is:

```powershell
cmake -S . -B build_msvc_static_smoke -G "Visual Studio 17 2022" -A x64 -DRUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE=ON
cmake --build build_msvc_static_smoke --config Release
ctest --test-dir build_msvc_static_smoke -C Release -R ruckig_c_msvc_cl_static_consumer --output-on-failure
```

It remains opt-in until CI evidence proves the toolchain is stable enough for
routine execution.

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
- MSVC `cl`: locally verified with the opt-in standalone DLL/import-library
  CTest; not part of routine CI. The repeatable gate is available when
  configured with `-DRUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE=ON`.
- MinGW: locally verified with GCC 15.2.0 through the Windows
  DLL/import-library consumer CTest. A dedicated MSYS2 MinGW64 routine CI gate
  now covers this path.

MSVC `cl` standalone DLL smoke:

```powershell
cl /nologo /std:c11 /I include /c examples\c\00_minimal_offline.c /Fo:build_release_check_shared\msvc_dll_consumer.obj
link /nologo build_release_check_shared\msvc_dll_consumer.obj build_release_check_shared\ruckig_c.lib /OUT:build_release_check_shared\msvc_dll_consumer.exe
$env:PATH = (Resolve-Path build_release_check_shared).Path + ";" + $env:PATH
.\build_release_check_shared\msvc_dll_consumer.exe
```

Do not define `RUCKIG_C_STATIC_DEFINE` for this DLL consumer path.

The CMake-scripted gate is:

```powershell
cmake -S . -B build_msvc_dll_smoke -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=ON -DRUCKIG_C_ENABLE_MSVC_CL_CONSUMER_SMOKE=ON
cmake --build build_msvc_dll_smoke --config Release
ctest --test-dir build_msvc_dll_smoke -C Release -R ruckig_c_msvc_cl_dll_consumer --output-on-failure
```

MinGW feasibility remains separate from MSVC-style consumers. The current
repeatable checks are:

```powershell
$env:PATH = "C:\ProgramData\mingw64\mingw64\bin;" + $env:PATH
cmake -S . -B out\build\mingw-static-consumer -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/ProgramData/mingw64/mingw64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/ProgramData/mingw64/mingw64/bin/g++.exe"
cmake --build out\build\mingw-static-consumer
ctest --test-dir out\build\mingw-static-consumer --output-on-failure -R ruckig_c_windows_manual_static_consumer
```

```powershell
$env:PATH = "C:\ProgramData\mingw64\mingw64\bin;" + $env:PATH
cmake -S . -B out\build\mingw-dll-consumer -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="C:/ProgramData/mingw64/mingw64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/ProgramData/mingw64/mingw64/bin/g++.exe" -DBUILD_SHARED_LIBS=ON
cmake --build out\build\mingw-dll-consumer
ctest --test-dir out\build\mingw-dll-consumer --output-on-failure -R ruckig_c_windows_dll_consumer
```

The routine CI gate uses MSYS2 MinGW64 and runs the same CTest consumer names
for static and DLL/import-library builds.

MSVC `cl` standalone static and DLL/import-library smokes remain opt-in local
gates. They are not routine CI jobs because the current routine Windows matrix
already covers `clang-cl` static and shared consumers, and `cl` availability is
environment-dependent. Reconsider routine CI only after repeated opt-in `cl`
evidence shows stable Visual Studio C++ tool availability and acceptable
maintenance cost.

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

The post-`v0.16.0` consumer refresh keeps these install paths unchanged while
upgrading the CMake and pkg-config smoke sources to compile and run minimal
public diagnostics usage: an invalid limit reports a stable diagnostics code,
and the restored valid path succeeds through
`ruckig_calculate_with_diagnostics`.

## Frozen Packaging Scope

The active maintenance scope keeps the existing installed CMake package,
pkg-config, Windows static/DLL, and shared install-tree consumption paths.
Those paths are the supported downstream integration surface for now.

vcpkg, Conan, Homebrew, CMake FetchContent, and vendored subdirectory recipes
are frozen outside the active roadmap. The existing feasibility notes and
experimental vcpkg overlay may remain as frozen reference material, but no new
package-manager recipe, prototype, or release commitment is part of the active
roadmap unless a separate packaging demand decision accepts it.

See `docs/design/package_manager_feasibility.md` for the frozen feasibility
record and the conditions that would need to be revisited before any future
package-manager project is opened.
