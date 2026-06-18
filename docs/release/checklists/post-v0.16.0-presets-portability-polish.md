# Post-v0.16.0 Presets Portability Polish Checklist

This checklist records the CMake preset portability slice. It adds a portable
Ninja preset and documentation while keeping the maintainer-verified Windows
LLVM/Ninja presets unchanged.

## Scope

- [x] Kept existing `windows-clang-ninja*` presets and their verified local
  absolute toolchain paths.
- [x] Added `portable-ninja` configure/build/test presets with no hardcoded
  compiler path.
- [x] Documented the difference between maintainer-local Windows presets and
  portable developer presets.
- [x] Did not modify `.github/workflows/ci.yml`.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --preset dev` | Failed in the current shell because no default C compiler was discoverable; this matches the documented `dev` prerequisite |
| `cmake --list-presets` | Passed; `portable-ninja` is listed |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp\|ruckig_c_public_diagnostics"` | Passed; 3/3 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
