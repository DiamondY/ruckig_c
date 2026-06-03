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

## Windows Manual Static Link

When manually linking a static Windows build without CMake, define
`RUCKIG_C_STATIC_DEFINE` before including the public header:

```sh
clang -std=c99 -DRUCKIG_C_STATIC_DEFINE -I path\to\include main.c path\to\ruckig_c.lib
```

CMake target consumers should not define this manually because the exported
static target propagates it. DLL consumers must not define
`RUCKIG_C_STATIC_DEFINE`.

## DLL Consumers

DLL consumers include the same public header and link against the import
library. Do not define `RUCKIG_C_STATIC_DEFINE`; the header will import public
symbols on Windows. The DLL must be discoverable at run time, for example by
placing it next to the executable or adding its directory to `PATH`.

The shared-library release gate covers the DLL/import-library build path and
runs header consumers plus examples against the shared library.

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
