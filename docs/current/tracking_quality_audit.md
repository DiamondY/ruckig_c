# Tracking Quality Audit

This document records the `0.9.0-alpha` tracking quality baseline evidence on
`0.9.0-design - Unreleased`. The goal was to make Optimized tracking fallback
and diagnostics behavior auditable before later evaluator tuning. The tuning
follow-up is recorded separately in
`docs/current/tracking_quality_hardening.md`.

## Status

`0.9.0-alpha` adds a test-runner-only audit selector:

```powershell
out\build\windows-clang-ninja\ruckig_c_tests.exe --tracking-random-audit 100000 --seed 1
```

The selector keeps the existing `--tracking-random` output unchanged. It prints
fixed text tables for overall behavior and for strategy, DoF, signal,
lookahead, reactiveness, disabled-DoF, and constraint-profile buckets. It also
prints deterministic representative fallback cases with public diagnostics
fields and candidate-family counters.

The audit is local evidence and routine light coverage. CTest runs
`--tracking-random-audit 10000 --seed 1`; 100k and 1M runs remain local/manual
evidence.

## Corpus

- DoF: `1`, `2`, `4`, `8`.
- Signals: `ramp`, `constant_acceleration`, `sinus`, `half_sinus`.
- Lookahead cycles: `1`, `2`, `5`, `10`.
- Reactiveness: `0`, `0.25`, `0.5`, `1`.
- Strategies: Stable, Balanced, Aggressive.
- Disabled DoF: enabled-only and one deterministic disabled DoF where valid.
- Constraints: default and `tight_valid`.

The audit harness uses high bits from the deterministic LCG for bucket choices.
This avoids the low-bit `% 4` degeneracy that would otherwise collapse the
audit corpus into a narrow subset. The existing `--tracking-random` selector is
not changed.

## Baseline Summary

Routine audit:

```text
tracking random audit: samples 10000 seed 1
overall: optimized 784 fallback 9216 candidates 141520 valid 141520 rejected 0 budget_exhausted 40298 average_improvement 0.00668805147
by_strategy stable: optimized 268 fallback 3033 average_improvement 0.00696885785
by_strategy balanced: optimized 254 fallback 3067 average_improvement 0.00573055088
by_strategy aggressive: optimized 262 fallback 3116 average_improvement 0.00735498978
```

100k seed evidence:

```text
seed 1: optimized 7802 fallback 92198 candidates 1414232 valid 1414232 rejected 0 budget_exhausted 402390 average_improvement 0.00685194194
seed 2: optimized 7876 fallback 92124 candidates 1413534 valid 1413534 rejected 0 budget_exhausted 402924 average_improvement 0.00614838341
seed 41: optimized 7848 fallback 92152 candidates 1413402 valid 1413402 rejected 0 budget_exhausted 403226 average_improvement 0.00749778288
```

Manual 1M evidence:

```text
seed 1: optimized 78110 fallback 921890 candidates 14138968 valid 14138968 rejected 0 budget_exhausted 4025250 average_improvement 0.00687851205
```

The important baseline interpretation is that the random audit has no rejected
candidate evaluations in these runs. The high fallback count therefore mainly
means the bounded Optimized candidate evaluator frequently does not beat the
Fast baseline score under the current candidate families and scoring, not that
the target solver is rejecting candidate trajectories.

## Representative Cases

The fixed C corpus uses deterministic representative fallback cases selected
from `--tracking-random-audit 100000 --seed 1`:

```text
sample 6: stable, 2 DoF, constant_acceleration, lookahead 1, reactiveness 0.25, disabled DoF 0, tight_valid, fallback, candidates 10
sample 12: balanced, 2 DoF, constant_acceleration, lookahead 5, reactiveness 0, disabled DoF 1, tight_valid, fallback, candidates 16, budget_exhausted 2
sample 22: aggressive, 8 DoF, ramp, lookahead 5, reactiveness 0.25, disabled DoF 7, tight_valid, fallback, candidates 16, budget_exhausted 6
```

The fixed tests assert successful calculation, finite constrained output,
diagnostics consistency, family-counter consistency, legacy getter consistency,
zero reserved fields, and internally consistent fallback/optimized/error step
counts. They intentionally do not require a quality improvement over Fast.

## Deferred

- Optimized evaluator tuning, scoring changes, candidate-family attribution,
  and acceptance-policy changes are handled by `0.9.0-alpha.2` in
  `docs/current/tracking_quality_hardening.md`.
- No public C API or ABI change is part of this baseline.
- No Pro/cloud equivalence, package publication, visualization v2, or upstream
  baseline upgrade is part of this evidence.
