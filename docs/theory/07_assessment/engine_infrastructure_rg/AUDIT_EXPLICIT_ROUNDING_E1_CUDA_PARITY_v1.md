# FTD-0752 — Explicit-rounding E1 CUDA parity audit v1

**Status:** `[CONSTRUCTIVE NUMERICAL FACT — BOUNDED BACKEND GATE CLOSED]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_EXPLICIT_ROUNDING_E1_CUDA_PARITY_v1.md`  
**Certificate:** `scripts/proofs/proof_explicit_rounding_e1_cuda_parity.py`

## Verdict

The research-only explicit-rounding CUDA library closes the bounded E1 dynamic
parity gate.

Across `L={33,65}`, face/edge/body directions, and ticks `1..8`, all 336
dynamic stage rows are bit-identical between CPU and CUDA:

- initial electric and magnetic fields;
- magnetic and electric source-free preparation;
- implicit matter-root metadata, constituent endpoints, and sparse current;
- ordered current deposition;
- accepted state transfer.

Every arm returns `EXACT_DYNAMIC_PARITY_DIAGNOSTIC_BOUNDED`. The maximum
read-only regional diagnostic difference is
`1.3877787807814457e-17`, below the locked `2e-15` gate by a factor of 144.
The independent certificate passes `101/101` checks.

## Repair identity

The established `ftd_cuda` target and CUDA source are unchanged. A separate
research library compiles only `cuda_matched_field_pipeline.cu` with
`--fmad=false`, matching the host target's `-ffp-contract=off` evaluation.

Frozen SM120 PTX contains separate `mul.rn.f64` then `add.rn.f64` in electric
preparation and separate `mul.rn.f64` then `sub.rn.f64` in magnetic
preparation. Neither prepare entry contains `fma.rn.f64`. This changes the
binary64 evaluation path but not the real-valued stencil.

## Scope

This result establishes exact eight-tick dynamic equivalence only for the six
registered selected-E1 arms. It does not retroactively pass FTD-0747--0750,
does not establish long-horizon exact parity without a fresh campaign, and
does not promote the selected compact interaction or `(s,C,F)` representation
to ontology.

The short backend arithmetic item is closed. M2/M3 campaign design may now use
the explicit-rounding research library. The next decisive question is
physical: whether the frozen E1 candidate defines an uncontained metastable
object basin and autonomous matter motion. Charge naming, Lorentz recovery,
pole extraction, and new constant formulae remain deferred.

## Reproducibility

- protocol SHA-256:
  `A12929B5C50CFD5586345BF78C5E943B21C430EDA32ECBFB5B9DE98DD23E791E`;
- six CSV and six JSON records, PTX, and hash manifest under
  `engine/results/ftd_0752/`;
- independent record/hash/PTX certificate: `101/101`;
- production tick, defaults, legacy force branches, and established CUDA
  library unchanged.
