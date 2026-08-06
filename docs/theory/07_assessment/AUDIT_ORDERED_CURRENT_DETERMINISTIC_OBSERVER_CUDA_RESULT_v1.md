# FTD-0750 ordered-current deterministic-observer CUDA result audit v1

**Status:** `[MIXED — STRICT REPLAY CONSTRUCTIVE; THREE-RAY CPU PREFIX NEGATIVE]`  
**Overall conjunction:** `[CLOSED NEGATIVE FOR THE FROZEN PROTOCOL]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`C3A3E787A201F1E429E1ED8D8D81B9F06B508A413B41A6B5E2584ED1BFD13385`

## Execution and certificate

The frozen WSL2 executable was invoked exactly once, serially, for `face_a`,
`face_b`, `edge_a`, `edge_b`, `body_a`, and `body_b`. Every process reached
tick 312 and wrote a 313-row main CSV, 313-row support CSV, and two JSON
summaries. Internal runtimes were 451.61, 480.69, 498.37, 431.72, 422.42,
and 467.92 seconds. No rebuild, retuning, tolerance change, result inspection,
or early stopping occurred between processes.

The independent serialized-record certificate passes `117/117`. A successful
certificate verifies frozen provenance and reconstructs the mixed result; it
does not relabel the failed conjunction as constructive.

## Strict replay result

D0 passes exactly for face, edge, and body:

- paired support CSVs are byte-identical;
- every main CSV cell except the registered `arm` token is identical;
- every support and main JSON physical/gate value except `arm` is identical;
- independently reconstructed verdicts match within each pair.

The FTD-0749 `2.78e-17` observer-only replay defect is eliminated without a
tolerance change. The selected-radius fixed-tree reduction therefore closes
the registered CUDA record-determinism defect constructively at this scope.

## CPU-prefix and physics gates

| ray | replicates | discrete prefix | maximum scalar prefix difference | H0, A0, H2--H5 | verdict |
|---|---|---:|---:|---:|---|
| face | a, b | exact | `6.568257049366e-11`, tick 91 separation | all pass | constructive |
| edge | a, b | exact | `2.794713349630e-10`, tick 88 separation | all pass | prefix drift |
| body | a, b | exact | `1.207889344101e-10`, tick 113 separation | all pass | prefix drift |

The frozen CPU-prefix tolerance is `1e-10`. Persistent-core onset remains tick
80/96/115 and radius-48 arrival remains tick 297 in every replicate. Every
registered execution, aggregation, core, near-field, arrival, and persistence
gate passes. The six-record conjunction is nevertheless closed negative on
edge/body H1.

## What the arithmetic repairs establish

The ordered-current unit proves exact one-step CPU/device equality for raw
same-face updates, including an adversarial cancellation whose result depends
on entry order. The full face trajectory now passes CPU H1, and all three rays
replay exactly between independent CUDA processes. Those are constructive
backend results.

They do not establish uniform CPU/CUDA prefix equivalence. The three disclosed
deposition maps produce complementary ray failures:

| campaign path | face H1 | edge H1 | body H1 |
|---|---:|---:|---:|
| FTD-0748 raw atomic | `6.66e-11` pass | `2.18e-10` fail | `1.21e-10` fail |
| FTD-0749 aggregate once | `1.39e-10` fail | `1.67e-12` pass | `2.79e-12` pass |
| FTD-0750 ordered per face | `6.57e-11` pass | `2.79e-10` fail | `1.21e-10` fail |

Because FTD-0750's maxima occur in the dynamical `separation` column, the
remaining failure is not a regional-observer artifact. Exact deposition parity
is insufficient to force long-horizon CPU parity. Arithmetic ordering or
contraction in field preparation, field diagnostics feeding the implicit
root, or the host/device trajectory boundary remains exposed. No further full
horizon run is justified until one-step stage-by-stage CPU/CUDA array and root
parity localizes that residual.

## Claim boundary

Strict deterministic CUDA replay is constructive for the selected campaign.
The broader D0-plus-H0--H5 conjunction is negative because edge and body miss
the preregistered CPU-prefix tolerance. This is a backend reproducibility
result, not evidence for or against the selected matter dynamics. It does not
establish a physical particle, asymptotic stability, inverse recovery, Lorentz
recovery, unitarity, production adoption, or electromagnetic ontology.
Production defaults and all legacy CUDA APIs remain unchanged.

## Frozen artifact manifest

All 24 result SHA-256 values are embedded and independently verified in
`scripts/proofs/proof_ordered_current_deterministic_observer_cuda.py`. The
pre-run executable SHA-256
`F5E423093D8AB69BCBAAB936F25EABA4EB38E3E5223DE9E4EE3A01A2347EDAD2`
is retained by the immutable pre-execution audit.
