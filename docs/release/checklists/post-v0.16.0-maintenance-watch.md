# Post-v0.16.0 Maintenance Watch Checklist

This checklist records the docs-only maintenance watch slice. It adds no code,
does not change public ABI, and does not start package, wrapper, upstream, or
patch-release work.

## Scope

- [x] Added `docs/current/maintenance_watch.md`.
- [x] Recorded event-driven triggers for upstream baseline upgrade readiness,
  oracle-backed solver regression, generated fixture auto-write, `0.16.1`,
  package-manager recipes, and wrapper stable publication.
- [x] Recorded explicit non-triggers for coverage-percentage slices, docs-polish
  patch releases, visualization churn, and Cloud/Pro claims over unavailable
  source.
- [x] Kept `v0.16.0` stable release state unchanged.

## Boundaries

- [x] No public C header change.
- [x] No public symbol allowlist or exception change.
- [x] No workflow, version metadata, tag, or GitHub Release change.
- [x] No update to `original/ruckig-main`.
- [x] No visualization asset change.
- [x] No default heavy random, coverage, performance, or static analyzer CI
  gate change.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed; no work to do |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed; 2/2 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
