# Upstream Delta Audit

This audit records the post-`v0.16.0` review of the frozen upstream Ruckig
source tree under `original/ruckig-main` against the current public upstream
repository. It does not update the baseline, modify `original/ruckig-main`,
change oracle tolerances, or change the public C ABI.

## Local Baseline

- Local frozen source tree: `original/ruckig-main`.
- Local upstream project metadata: `project(ruckig VERSION 0.17.3 LANGUAGES CXX)`.
- The directory is tracked as part of this repository, not as an independent
  Git checkout. Ordinary maintenance must continue to treat it as frozen input.

## Remote Upstream Snapshot

Commands used:

```powershell
git ls-remote --tags https://github.com/pantor/ruckig.git
git ls-remote https://github.com/pantor/ruckig.git HEAD
```

Observed remote facts:

- Latest listed upstream tag remains `refs/tags/v0.17.3`
  (`cb99a04ce488f83701aaee6efd9c9f0d36a3d43b`).
- Remote upstream HEAD is `f48cf5fe8c48083b88b7ceda9a069dd5565d0d38`.
- Temporary audit clone described HEAD as `v0.17.3-7-gf48cf5fe8c48`.

The upstream repository has post-tag commits, but no newer public tag than
`v0.17.3` was observed during this audit.

## Post-Tag Delta Categories

The temporary upstream clone reported these commits after `v0.17.3`:

- `753d689 initialize max/min position limit to +/- infinity`
- `0da6ab7 align get_position_extrema method to pass reference of bounds`
- `765c593 remove unneeded position_extrema member in trajectory class`
- `4583f54 rust wrapper add min/max position limits`
- `53f42b3 improve error handling with cloud client`
- `f836852 add link to docs to readme`
- `f48cf5f Add ruckig-scl to Used By (#259)`

File-level changes from upstream `v0.17.3` to upstream HEAD were limited to:

- README and example/plotting documentation.
- Cloud client error handling.
- C++ input/trajectory API shape around position limits and position extrema.
- Upstream Python/Rust wrappers.
- Upstream tests around the previous position-extrema return shape.

No dedicated solver source file under `src/ruckig/*step*.cpp` or
`src/ruckig/brake.cpp` changed in this post-tag delta.

## Local Tree Comparison

Ignoring CRLF differences, the local `original/ruckig-main` code files checked
for the material post-tag changes matched the temporary upstream HEAD for:

- `include/ruckig/input_parameter.hpp`
- `include/ruckig/trajectory.hpp`
- `src/ruckig/cloud_client.cpp`
- `src/wrapper/rust.rs`

`original/ruckig-main/README.md` differs from upstream HEAD content, but that
is documentation-only for this audit and does not affect the oracle source
surface used by `ruckig_c`.

## Risk Assessment

Material upgrade risk is low for solver numerics because no post-tag analytical
solver source delta was observed. The higher risk is source-baseline accounting:
the local frozen tree still reports upstream version `0.17.3`, while selected
code files already match upstream post-`v0.17.3` HEAD behavior.

Potential future upgrade work should therefore start with baseline provenance
and source inventory, not with direct code replacement.

## Conclusion

Do not update the upstream baseline in ordinary post-`v0.16.0` maintenance.

If the project wants to regularize baseline provenance, open a separate
`0.18.0-upstream-baseline-upgrade-readiness` slice. That slice should decide
whether to keep the current frozen source tree as-is, re-anchor it to an
upstream tag when one exists, or explicitly document the current post-tag source
snapshot as the oracle baseline.

## Follow-up Readiness

`docs/design/0.18.0_upstream_baseline_provenance_readiness.md` records the
readiness-only follow-up. It keeps the current tree frozen as the `0.17.3-line`
baseline, documents the provenance risk, and does not formally re-label the
baseline as a post-tag snapshot.
