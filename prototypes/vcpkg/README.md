# vcpkg Overlay Prototype

This is an experimental `0.3.0-design` workspace for vcpkg feasibility. It is
not a supported package-manager recipe and is not part of release packaging.

## Run From A vcpkg Checkout

From the repository root:

```powershell
vcpkg install ruckig-c --overlay-ports=prototypes/vcpkg/overlay-ports --triplet x64-windows
```

Then configure the test package with the installed toolchain:

```powershell
cmake -S prototypes/vcpkg/test-package -B build_vcpkg_test_package -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
cmake --build build_vcpkg_test_package
.\build_vcpkg_test_package\vcpkg-ruckig-c-consumer.exe
```

Use `x64-windows-static` to verify that the CMake target propagates
`RUCKIG_C_STATIC_DEFINE` for static consumers. Use a shared triplet to verify
that DLL consumers do not define the static macro.

## Current Status

- Prototype only.
- Overlay port uses the current repository checkout as its source.
- No frozen C++ oracle sources are installed as runtime package contents.
- No vcpkg recipe is promoted as supported release scope.
- Locally verified on Windows:
  - `x64-windows` package install, CMake consumer configure/build/run.
  - `x64-windows-static` package install, CMake consumer configure/build/run.
  - DLL consumer target does not propagate `RUCKIG_C_STATIC_DEFINE`.
  - Static consumer target propagates `RUCKIG_C_STATIC_DEFINE`.
- Linux and macOS vcpkg matrix entries are not yet verified.
