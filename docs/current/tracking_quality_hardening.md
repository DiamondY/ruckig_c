# Tracking Quality Hardening

This document records the `0.9.0-alpha.2` tracking Optimized evaluator quality
hardening evidence on `0.9.0-design - Unreleased`. It builds on the
`0.9.0-alpha` fallback/diagnostics audit baseline and tunes the bounded local
evaluator without changing the public C ABI.

## Scope

- Public C API additions: `0`.
- Public symbol additions: `0`.
- Public diagnostics struct additions: `0`.
- Enum and result-code numeric changes: `0`.
- Tags, GitHub Releases, manual release-random workflows: none.
- Frozen original source changes: none.

The new evidence surface is test-runner only:

```powershell
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-quality-hardening
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit 100000 --seed 1
```

`--tracking-random` keeps its existing output and behavior. The alpha.2 audit
format replaces the alpha.1 audit text with threshold and attribution tables
for `--tracking-random-audit` only.

## Evaluator Changes

The tuned evaluator keeps the public optimized candidate budget default at `16`
and keeps prepared online/offline tracking paths allocation-free.

- Stable and Balanced still require strict tuned-evaluator-score improvement
  over Fast.
- Aggressive first uses the same strict tuned-evaluator-score improvement path.
- If Aggressive has no strict improvement, it may accept a near-tie candidate
  only when the raw candidate score is at most `1%` worse than Fast and the
  terminal position error is at most half of Fast terminal position error.
- Solver-failing, non-finite, or constraint-invalid candidates are never
  accepted.

`best_score` in diagnostics remains the selected evaluator score used by this
bounded local evaluator. Alpha.2 applies a private non-Fast family score ratio
inside that evaluator, so the score is evidence for this evaluator's selection
policy rather than a standalone physical-error metric.

## Private Attribution

Alpha.2 adds private fields to the opaque `ruckig_tracking_t` handle. They are
read only by the C test runner and are not exported:

- attempted candidates by family.
- valid candidates by family.
- strict-improved candidates by family.
- Aggressive near-tie accepted candidates by family.
- selected final family.

The tracked candidate families are `fast`, `instantaneous`, `horizon`,
`terminal_blend`, `derivative_damped`, and `lead_lag`.

## Hard Thresholds

Alpha.2 hard-gates each strategy against the alpha.1 audit baseline:

- required optimized count: `ceil(alpha1_optimized * 1.25)`.
- required average improvement: `alpha1_average_improvement * 1.10`.

The registered hard gates are 10k seed `1`, 100k seeds `1`, `2`, `41`, and 1M
seed `1`. The 10k gate is also run by the routine CTest
`ruckig_c_tracking_quality_hardening`.

`0.9.0-alpha.3` builds on this tuned baseline with
`docs/current/tracking_stability.md`, freezing representative selected-family,
near-tie, fallback, and budget-exhaustion behavior as regression evidence
without changing the evaluator policy again.

## Local Results

Current 10k seed `1` threshold evidence:

```text
stable: optimized 3298 required 335 average_improvement 0.00895310250693 required 0.007665743635 PASS
balanced: optimized 3320 required 318 average_improvement 0.0077184875496 required 0.006303605968 PASS
aggressive: optimized 3375 required 328 average_improvement 0.00973711453948 required 0.008090488758 PASS
```

Current 100k seed `1` threshold evidence:

```text
stable: optimized 33460 required 3285 average_improvement 0.0085343451629 required 0.007204027193 PASS
balanced: optimized 33165 required 3252 average_improvement 0.00878051754558 required 0.007474712102 PASS
aggressive: optimized 33310 required 3217 average_improvement 0.00946028786669 required 0.007933984795 PASS
```

Current 100k seed `2` threshold evidence:

```text
stable: optimized 33523 required 3310 average_improvement 0.00786218594236 required 0.006463353787 PASS
balanced: optimized 33152 required 3378 average_improvement 0.00812951058237 required 0.00675792095 PASS
aggressive: optimized 33259 required 3158 average_improvement 0.00874542449754 required 0.007070771532 PASS
```

Current 100k seed `41` threshold evidence:

```text
stable: optimized 33342 required 3298 average_improvement 0.00990384472304 required 0.008712181291 PASS
balanced: optimized 33328 required 3389 average_improvement 0.00961835804776 required 0.008397384611 PASS
aggressive: optimized 33302 required 3124 average_improvement 0.00875016454004 required 0.007632562487 PASS
```

Current 1M seed `1` threshold evidence:

```text
stable: optimized 333676 required 33289 average_improvement 0.0087752032204 required 0.007469606034 PASS
balanced: optimized 333123 required 32714 average_improvement 0.00871075032494 required 0.007397795867 PASS
aggressive: optimized 332552 required 31635 average_improvement 0.00932603685594 required 0.007832339636 PASS
```

## Deferred

- Formal global optimality proof.
- Pro/cloud numerical equivalence.
- Public diagnostics or ABI expansion.
- Tracking plots and visualization v2.
- Python wheel, Rust crate, and package-manager recipe publication.
- Soft interruption checkpoints.
- Upstream baseline upgrade.
