# 0.3.0 Design Priority Evaluation

This document records the first `0.3.0-design` priority decision after the
`v0.2.1` patch release. It is design-only and does not approve implementation,
public API changes, solver changes, or upstream baseline changes.

## Non-Goals

During `0.2.x` maintenance and `0.3.0-design`, this document does not approve:

- Binding implementation work.
- Public C API additions or removals.
- Solver dispatch or numerical behavior changes.
- Upstream baseline upgrades.
- Waypoints, per-section constraints, or cloud calculation implementation.

## Decision

The next feature-planning priority is package and ABI maturity before bindings.
Python or Rust bindings should not start until `ruckig_c` has completed at least
one `0.2.x` patch cycle with release evidence for:

- Stable public header review.
- Exported-symbol review.
- Installed CMake consumer verification.
- pkg-config consumer verification.
- Windows static and DLL consumer verification.
- Windows and Linux performance records.

After those conditions are met, Python bindings should be evaluated before Rust
bindings because they are the faster path for broad trajectory-generation
experimentation and API ergonomics feedback. Rust bindings remain a follow-up
candidate after the C ABI evidence has proved stable enough for another typed
FFI layer.

The Python feasibility document selects `cffi` ABI mode as the default
prototype route. That decision is still design-only: it does not add binding
code, change the public C header, change solver behavior, or alter packaging
outputs during `0.2.x`.

## Upstream Baseline Upgrade

An upstream Ruckig baseline upgrade is lower priority than bindings design and
must remain a separate project. It must include:

- Upstream diff review.
- Source inventory update.
- Public and deferred-scope review.
- Numerical tolerance review.
- Fixed oracle corpus update.
- Full deterministic random stress.
- Windows and Linux performance baseline rebuild.

The upgrade must not be mixed with patch releases, bug fixes, per-DoF
hardening, or bindings work.

## Waypoints, Per-Section Constraints, And Cloud

Intermediate waypoints, per-section constraints, and cloud calculation remain
deferred. They require a separate design document before any public API work.
That design must define:

- The Ruckig Community cloud/pro behavior boundary.
- The C API representation.
- Which partial combinations are unsupported.
- How unsupported or partial behavior returns explicit errors.

No public API should be added for these features during `0.2.x`.

## Current Ordering

1. Finish `0.2.x` package, consumer, ABI, performance, and regression evidence.
2. Evaluate Python bindings design.
3. Re-evaluate Rust bindings after Python binding feasibility is clear.
4. Consider upstream baseline upgrade as an independent project.
5. Keep waypoints, per-section constraints, and cloud behind separate design
   gates.
