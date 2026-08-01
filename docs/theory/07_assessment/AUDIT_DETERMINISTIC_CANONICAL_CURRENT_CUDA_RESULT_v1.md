# FTD-0749 deterministic canonical-current CUDA result audit v1

**Status:** `[MIXED — TRAJECTORY REPLAY EXACT; STRICT D0 AND FACE H1 FAIL]`  
**Overall conjunction:** `[CLOSED NEGATIVE FOR THE FROZEN PROTOCOL]`  
**Date:** 2026-07-29  
**Protocol SHA-256:**
`6C0BE1E8109DBD17451FF3A21F426A75583120810EB8C0C9B9077056AE86BB83`

## Execution and certificate

The frozen WSL2 executable was invoked exactly once, serially, for `face_a`,
`face_b`, `edge_a`, `edge_b`, `body_a`, and `body_b`. Every process reached
tick 312 and wrote a 313-row main CSV, 313-row support CSV, and two JSON
summaries. Internal runtimes were 462.23, 468.82, 459.72, 449.29, 476.34, and
454.40 seconds. No rebuild, retuning, tolerance change, or early stopping
occurred between processes.

The independent serialized-record certificate passes `121/121`. A successful
certificate exit verifies frozen provenance and faithfully reconstructs the
failed registered gates; it does not relabel them as passes.

## Physics-prefix verdicts

| ray | replicates | corrected discrete prefix | maximum scalar prefix difference | H0, A0, H2--H5 | verdict |
|---|---|---:|---:|---:|---|
| face | a, b | exact | `1.386979420204e-10`, tick 91 `separation` | all pass | prefix drift |
| edge | a, b | exact | `1.667999072197e-12`, tick 148 `separation` | all pass | constructive |
| body | a, b | exact | `2.789546371673e-12`, tick 150 `separation` | all pass | constructive |

The frozen CPU-prefix tolerance is `1e-10`. Both face replicates exceed it by
a factor of 1.387. Edge and body improve by roughly two orders of magnitude
relative to the tolerance. Radius-48 arrival remains tick 297 and persistent
core onset remains tick 80/96/115 in every replicate. The six-record physics
conjunction is closed negative on face H1.

## D0 replay split

The strict registered D0 gate fails in all three pairs, but the failure is
entirely in read-only diagnostics:

- support CSVs are byte-identical within every pair;
- support JSON physical/gate values are identical after removing `arm`;
- every trajectory/discrete cell is exactly identical, including
  `separation`, `pair_energy`, `field_energy`, validity, graph status, source
  radius, and canonical support;
- only `regional_residual`, `outside_source_residual`, and the five regional
  energy/transport fields at each of six radii differ;
- the maximum pairwise difference is `2.775557561563e-17`.

The main JSON differences are likewise restricted to summaries derived from
those regional observers. The protocol required every main cell and every JSON
physical/gate value to match, so exact trajectory identity cannot override the
strict D0 failure.

## What the CUDA correction actually fixed

FTD-0749 removes the collision-prone floating atomic from current deposition.
The complete raw current is aggregated to one unique periodic oriented-face
coefficient and each face has one non-atomic writer. Independent 312-step
processes then reproduce all dynamical state and matter/field trajectory
observables exactly. This is constructive qualification of deterministic
unique-face dynamics at the tested scope.

The remaining D0 mismatch comes from `regional_profile_kernel`, which still
uses floating `atomicAdd` both inside each block histogram and when combining
block histograms. Those reductions are observers and do not feed the state,
explaining the exact trajectory alongside last-bit regional differences. A
deterministic selected-radius block reduction is the correct observer repair;
tolerance relaxation is unnecessary and inadmissible.

The face H1 failure has a different cause. FTD-0749 applies one long-double
aggregated coefficient per face, whereas the CPU baseline applies raw double
increments sequentially. These are mathematically equivalent but not
bit-identical floating-point maps. The admissible CPU-parity successor groups
raw entries by face while retaining their original per-face double-addition
order, then assigns one device writer to execute that ordered list. Redefining
the CPU baseline or increasing H1 tolerance after this result is not a repair.

## Claim boundary

The strict FTD-0749 conjunction is negative. The narrower dynamical conclusion
is constructive: unique-face deposition eliminates run-to-run trajectory
nondeterminism for all three registered directions. This does not establish a
physical particle, asymptotic stability, inverse recovery, Lorentz recovery,
unitarity, production adoption, or electromagnetic ontology. Production
defaults and the legacy CUDA deposition API remain unchanged.

## Frozen artifact manifest

All 24 result SHA-256 values are embedded and independently verified in
`scripts/proofs/proof_deterministic_canonical_current_cuda.py`. The pre-run
executable SHA-256
`13ECEF1C337BF1E50DD783D936F9263AB5F77DAD0697120B2CB7FB2C2280053D`
is retained by the immutable pre-execution audit. Identical support CSV pairs
intentionally share hashes; main CSV/JSON records retain replicate-specific
hashes because strict D0 failed.

