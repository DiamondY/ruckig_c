# Post-v0.16.0 Allocation Audit Threading Policy Checklist

This checklist records the allocation-audit threading policy slice. It
documents the allocation counters as test/audit instrumentation and adds a
deterministic counter sequencing test without adding atomics, mutexes, public
ABI, workflow changes, or release-state changes.

## Scope

- [x] Documented allocation counters as local test/audit instrumentation.
- [x] Documented that `ruckig_allocation_count`,
  `ruckig_free_count`, `ruckig_allocation_forbidden_count`,
  `ruckig_allocation_counters_reset`, and
  `ruckig_allocation_forbidden_set` do not provide thread-safe aggregate
  statistics.
- [x] Kept core handle threading guidance separate from audit counters:
  independent handles may be used independently, and callers must externally
  synchronize shared handles.
- [x] Added deterministic allocation counter reset/forbidden sequencing
  coverage.
- [x] Did not add multi-thread race tests, C11 atomics, locks, or real-time
  path synchronization.

## Verification

| Gate | Result |
| --- | --- |
| `cmake --build --preset windows-clang-ninja` | Passed |
| `ctest --test-dir out\build\windows-clang-ninja --output-on-failure -R "ruckig_c_allocation_audit\|ruckig_c_tests\|ruckig_c_header_c\|ruckig_c_header_cpp"` | Passed; 4/4 tests |
| Public header / ABI / workflow boundary diff | Empty |
| `original/ruckig-main` and visualization asset boundary diff | Empty |
| `git diff --check` | Passed with CRLF normalization warnings only |
