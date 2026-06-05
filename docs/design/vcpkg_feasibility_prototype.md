# vcpkg Feasibility Prototype Plan

This is a `0.3.0-design` prototype plan only. It does not add a vcpkg recipe to
the release scope and does not promise package-manager support.

## Preconditions

- Public exports are aligned with `docs/abi/public-symbols.txt`.
- Shared/static consumer behavior is stable on Windows.
- `original/ruckig-main` remains a test oracle only and is not packaged as a
  runtime dependency.

## Prototype Layout To Test Externally

Use an external vcpkg overlay port or an experimental branch outside the release
scope. The prototype should install:

- `include/ruckig_c/ruckig.h`.
- the static or shared `ruckig_c` library;
- `ruckig_cConfig.cmake` and `ruckig_cConfigVersion.cmake`;
- `ruckig_c.pc` where pkg-config metadata is expected.

Do not install oracle tests, performance benchmarks, examples, or
`original/ruckig-main` as runtime package contents.

This repository includes an experimental overlay prototype under
`prototypes/vcpkg/`. It is intentionally not a supported release recipe. Use it
to validate package shape and consumer behavior before deciding whether a real
vcpkg packaging project should be opened.

## Required Matrix

- Windows static.
- Windows shared/DLL.
- Linux static.
- Linux shared.
- macOS static.
- macOS shared.

Each matrix entry must build a consumer that uses:

```cmake
find_package(ruckig_c CONFIG REQUIRED)
target_link_libraries(app PRIVATE ruckig_c::ruckig_c)
```

Windows static consumers must receive `RUCKIG_C_STATIC_DEFINE` through target
metadata. Windows DLL consumers must not define `RUCKIG_C_STATIC_DEFINE`.

## Prototype Checks

- Static and shared package options map directly to `BUILD_SHARED_LIBS`.
- Test and oracle options remain disabled by default for package consumers.
- The package does not require a C++ runtime for normal C consumers.
- The installed CMake target name remains `ruckig_c::ruckig_c`.
- The installed pkg-config file links `-lm` privately on Unix-like static
  builds where needed.
- Public symbol artifacts from packaged shared builds match
  `docs/abi/public-symbols.txt`.

## Decision Gate

The vcpkg prototype can be promoted to a real packaging project only after:

- Linux and Windows public exported-symbol comparisons are clean.
- MSVC `cl` static/DLL consumer smoke has repeatable evidence.
- MinGW has an explicit pass/fail feasibility result.
- The prototype proves that no frozen C++ oracle source is packaged as runtime
  dependency.

## Local Prototype Evidence

Current workspace result:

```text
Tooling:
- vcpkg cloned and bootstrapped under ignored build_vcpkg_tool/
- CMake 4.3.2, Ninja 1.13.2, 7zip, 7zr, and PowerShell Core were cached under
  build_vcpkg_tool/downloads/ because vcpkg's direct downloads hit proxy/SSL
  errors.

x64-windows shared/default package:
- Command: vcpkg install ruckig-c --overlay-ports=prototypes/vcpkg/overlay-ports --triplet x64-windows
- Result: All requested installations completed successfully.
- Consumer configure: passed.
- Consumer macro check: DLL target did not propagate RUCKIG_C_STATIC_DEFINE.
- Consumer build/run: passed with PATH pointing at installed/x64-windows/bin.

x64-windows-static package:
- Command: vcpkg install ruckig-c --overlay-ports=prototypes/vcpkg/overlay-ports --triplet x64-windows-static
- Result: All requested installations completed successfully.
- Consumer configure: passed.
- Consumer macro check: static target propagated RUCKIG_C_STATIC_DEFINE.
- Consumer build/run: passed.
```

Linux and macOS vcpkg matrix entries remain unverified until run on those
platforms. The prototype must still not be promoted to a supported recipe until
that matrix is complete.
