# FTD-0756 — M3 parent-replay forensics result v1

**Status:** `[NUMERICAL FACT — VALIDATION-WRAPPER FAILURE LOCALIZED; NO MATTER VERDICT]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_M3_PARENT_REPLAY_FORENSICS_v1.md`  
**Certificate:** `scripts/proofs/proof_m3_parent_replay_forensics.py`

## Verdict

The locked six-arm replay returns

```text
FTD-0756 artifact: 1331/1331 checks
verdict=M3_PARENT_WRAPPER_TRANSACTION_FAILURE
rows=424
exact_scalar_comparisons=1654
maximum_numeric_difference=0
```

Finite-support preparation passes on every ray at both `L=321` and `L=385`.
Every successfully completed `L=321` row is string-identical to the published
FTD-0753 scalar record. The wrapper then stops at failure stage 4 on the same
tick at both volumes:

| ray | first failed tick | stage | retained pre-failure core member |
|---|---:|---:|---:|
| face | 57 | 4 | no |
| edge | 30 | 4 | no |
| body | 122 | 4 | yes |

Stage 4 is the wrapper's call to `observe_deterministic`; source-free field
preparation, the implicit constituent solve, and ordered sparse-current
deposition have already succeeded. No failed stage-4 transaction is committed
to state. The body row therefore demonstrates directly that the observer can
abort while the retained relational state still satisfies the frozen core
predicate.

## Defect localization

The FTD-0755/0756 wrapper supplies the constituents' continuously valued
midpoint as the regional-observer center. The deterministic observer interface
rejects any center component that is not exactly equal to its nearest integer:

```cpp
center.x != std::round(center.x)
|| center.y != std::round(center.y)
|| center.z != std::round(center.z)
```

The established FTD-0753 replay instead uses the fixed integer preparation
center for all regional measurements. Thus the validation wrapper coupled a
continuous dynamical midpoint to an integer-center readout API. This extra
domain condition is not a matter equation, common-action gate, or physical
boundary. The volume-independent, ray-dependent abort ticks and the exact
pre-abort scalar replay are the observed signature of that mismatch.

FTD-0756 localizes but does not repair the defect. A successor must freeze a
fixed-center replay (or a separately proved fractional-center observer),
reproduce FTD-0753 through tick 160, and only then launch a fresh held-out
validation identifier. FTD-0755 remains consumed and inconclusive.

## Artifacts and scope

- protocol SHA-256:
  `773BDB791B06A0250C980945A1B52EF9F2A6F119EF8905E9AC57DC83A6FB5CFC`;
- six CSV/JSON pairs under `engine/results/ftd_0756/`;
- all records state `dynamics_changed = false`;
- no production, CUDA-library, scenario, predicate, ontology, or physical
  parameter changed.

M3, an invariant matter family, particle identity, asymptotic persistence,
autonomous mobility, charge, mass, spin, statistics, unitarity, and Lorentz
recovery remain open.
