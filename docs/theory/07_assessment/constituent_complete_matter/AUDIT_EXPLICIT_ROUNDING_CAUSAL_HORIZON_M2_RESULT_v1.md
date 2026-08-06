# FTD-0753 — Explicit-rounding causal-horizon M2 result audit v1

**Status:** `[CONSTRUCTIVE NUMERICAL FACT — LARGE CAUSAL-HORIZON WITNESS; FULL M2 PARTIAL]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_EXPLICIT_ROUNDING_CAUSAL_HORIZON_M2_v1.md`  
**Certificate:** `scripts/proofs/proof_explicit_rounding_causal_horizon_m2.py`

## Verdict

The fresh face/edge/body conjunction is constructive. On each ray, the
selected reciprocal `(s,C,F)` candidate forms a persistent negative-energy
core, retains a stable late near field, and sends field energy past radius 48
before the registered exterior-contact tick. That radius-48 component remains
outward through the locked terminal window.

The independent certificate recomputes every gate from 939 trajectory rows
and 939 canonical-support rows, verifies 12 artifact hashes plus the frozen
protocol/runner/executable/library identities, and passes `134/134` checks.

## Registered results

| arm | core onset | radius-48 first passage | late radius-8 min/max | ticks 301--312 radius-48 minimum | minimum outward increment |
|---|---:|---:|---:|---:|---:|
| face | 80 | 297 | `2.033e-3 / 2.335e-3` | `2.649e-8` | `2.276e-9` |
| edge | 96 | 297 | `1.889e-3 / 2.152e-3` | `2.665e-8` | `2.289e-9` |
| body | 115 | 297 | `1.794e-3 / 2.144e-3` | `2.669e-8` | `2.293e-9` |

All arms pass H0, A0, and H2--H5. Across the full conjunction:

- maximum common-action residual: `5.378e-14`;
- maximum total-energy residual: `6.579e-15`;
- maximum recoil defect: `2.053e-14`;
- maximum pair-plus-field balance defect: `3.098e-14`;
- maximum regional-ledger residual: `2.776e-17`;
- maximum source exchange outside compact support: `6.076e-19`;
- discarded canonical current: exactly zero;
- maximum current first-moment residual: `2.202e-18`;
- maximum canonical net support: 54 faces, within source radius 2.

## What changed in the evidence

FTD-0745's `L=193`, tick-184 continuation failed because its locked radius-32
and radius-48 disturbances had not yet arrived. FTD-0753 uses the previously
registered causal extrapolation rather than relaxing that deadline: on the
larger `L=321`, tick-312 horizon, radius 48 is reached at tick 297 on every
ray, leaving 12 registered post-arrival ticks before contact at tick 313.

FTD-0753 is a fresh run on the FTD-0752 explicit-rounding research backend. It
does not retroactively alter the failed CPU-prefix verdicts of FTD-0747--0750.
Their diagnostic remains serialized; it is not part of this physical
conjunction.

## Exact scope boundary

This result establishes one finite, large-volume, three-ray causal-horizon
environmental-persistence witness. It is positive evidence for M2 and removes
the specific horizon-mismatch defect that closed FTD-0745.

Full M2 remains partial because the existing radius observer is not a unique
state-only decomposition of bound dressing, outgoing radiation, and
background. M3 remains open because no frozen state-only family predicate has
yet survived a held-out nonzero perturbation neighborhood over the causal
volume/horizon ladder. This result is not an invariant basin, autonomous
moving particle, charge, pole, unitarity, or Lorentz result.

## Reproducibility

- protocol SHA-256:
  `66D64B1A09AAB3243C5BA06991B9979C10C03EA8B8B4A01BA3803260BF3822A4`;
- runner SHA-256:
  `B8AC5DED34953F8F59D9036EED9F72266DAF218842DA21CDA226666357986562`;
- WSL2 executable SHA-256:
  `878D752B4C4422A865B5C08EC1DC55C50610ECB2F743AFA6793A29303606F4D6`;
- 12 run artifacts plus `manifest.json` under `engine/results/ftd_0753/`;
- independent certificate: `134/134`.

Production defaults, the established CUDA library, scenarios, and ontology
are unchanged.
