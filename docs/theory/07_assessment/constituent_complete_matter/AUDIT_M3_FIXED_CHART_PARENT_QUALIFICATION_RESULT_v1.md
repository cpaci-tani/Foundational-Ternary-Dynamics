# FTD-0757 — M3 fixed-chart parent qualification result v1

**Status:** `[CONSTRUCTIVE NUMERICAL FACT — PARENT QUALIFIED; M3 STILL OPEN]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_M3_FIXED_CHART_PARENT_QUALIFICATION_v1.md`  
**Certificate:** `scripts/proofs/proof_m3_fixed_chart_parent_qualification.py`

## Verdict

All six locked arms ran exactly once and the independent certificate returns

```text
FTD-0757 artifact: 6833/6833 checks
verdict=M3_FIXED_CHART_PARENT_QUALIFIED
rows=966
exact_scalar_comparisons=3846
maximum_numeric_difference=0
```

Every finite-support preparation passes, every regional observation and
common-action transaction passes through tick 160, and every final state
satisfies the support-independent core predicate at both volumes.

| ray | first fractional midpoint tick | final graph margin | final energy margin |
|---|---:|---:|---:|
| face | 57 | `0.7343855134021913` | `0.0013194545545783148` |
| edge | 30 | `0.6935289448583533` | `0.0010732076736830612` |
| body | 122 | `0.40249144328636466` | `0.0007660270835095281` |

The tick and margin entries are identical at `L=321` and `L=385`. Every
accepted `L=321` scalar string matches the FTD-0753 source of record exactly.

## What was repaired

Only the observer chart changed relative to the failed FTD-0755 wrapper. The
continuous constituent midpoint

\[
m_t=(x_+(t)+x_-(t))/2
\]

is recorded but never rounded or fed into the state. The deterministic
regional observer uses the fixed integer preparation chart `C_L`. The action,
implicit matter solve, current deposition, field update, predicate,
normalization, volumes, tolerances, and initial states are unchanged.

The first ticks at which `m_t` fails the observer API's exact integer-center
condition are face 57, edge 30, and body 122 at both volumes. These are exactly
the FTD-0756 stage-four abort ticks. Continuing through tick 160 with `C_L`
therefore confirms that the earlier failure was caused by the moving-center
readout domain, not by preparation, field evolution, current deposition,
common-action closure, or relational-core decay.

## Ontological consequence

The result forces a useful separation among three structures:

1. **relational object coordinate:** the continuous midpoint derived from the
   constituent state;
2. **primitive dynamics:** the center-free local matter/current/field
   transaction;
3. **regional measurement chart:** a selected lattice-centered energy
   observer.

The fixed chart is not a material center, boundary, preferred object rest
frame, or new primitive. Conversely, the derived continuous midpoint is not
automatically an admissible center for a cell-indexed regional sum. A
fractional co-moving observer would require a separately derived
translation-covariant interpolation and cannot be assumed from the existence
of `m_t`.

## Exact scope

FTD-0757 qualifies only the parent construction required by a fresh held-out
validation. It does not retroactively execute FTD-0755's hostile candidates or
causal fibres, establish an open family, prove autonomous translation, or
promote matter to a particle. FTD-0755 remains consumed and inconclusive.

A successor may now freeze new held-out candidate/fibre arms using the fixed
integer chart. It must retain the FTD-0755 support-independent predicate and
may not use the continuous midpoint, dressing energy, regional shell, or
environmental field as a membership threshold.

## Reproducibility

- protocol SHA-256:
  `E867A86868E00673EDAA716F1D7CB021A2E9BFB6F798BDC8C552385C4EE6DB50`;
- twelve CSV/JSON artifacts under `engine/results/ftd_0757/`;
- six arms, 966 total rows, no reruns;
- all metadata records `dynamics_changed = false`.

Production defaults, established CUDA libraries, scenarios, and ontology were
unchanged.
