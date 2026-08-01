# FTD-0766 aged wake/entrainment discriminator — pre-execution audit v1

**Status:** `[LOCKED PROTOCOL + CUDA OBSERVER QUALIFIED + REGISTERED EXECUTION IN PROGRESS]`  
**Date:** 2026-07-31  
**Scope:** observer-only face-ray discovery; no production or ontology change

**Result successor:**
[`AUDIT_AGED_WAKE_ENTRAINMENT_RESULT_v1.md`](AUDIT_AGED_WAKE_ENTRAINMENT_RESULT_v1.md)
records the completed invalid execution and 404/404 certificate. This file
remains the pre-execution provenance record.

## 1. Locked question

FTD-0765 proved that the FTD-0764 longitudinal moment is a core--residual
centroid lag. FTD-0766 therefore does not reuse that bit as wake evidence. It
asks whether signed motion creates a residual-energy excess behind the moving
core that survives direction alignment, amplitude ordering, and preparation
aging.

The locked protocol is
[`PREREG_AGED_WAKE_ENTRAINMENT_DISCRIMINATOR_v1.md`](../10_eft_program/preregistrations/PREREG_AGED_WAKE_ENTRAINMENT_DISCRIMINATOR_v1.md),
whose SHA256 is
`B8FF05668DF306D05B6D3F7F4715C38B6C3A78C9205E9C747146C2F3A95AFA7F`.

## 2. Registered matrix

- volume `L=321`;
- face direction `(0,0,1)`;
- preparation ages `{0,64,128}` after tick-160 formation;
- signed boosts `q={0,+/-0.0075,+/-0.015,+/-0.030}`;
- 64 ticks per branch;
- checkpoints `{0,16,32,48,64}`;
- no field boost, regeneration, recentering, or post-hoc correction.

At each checkpoint the actual-minus-selected-bound residual energy is divided
in the velocity-aligned transported chart into

```text
trailing: xi dot d_q < -1/2,
neutral: |xi dot d_q| <= 1/2,
leading: xi dot d_q > +1/2.
```

The primary scalar is `(T-H)/(T+H)`, recorded separately in the radius-8 near
window, the `8<r_infinity<=48` outer annulus, and their union. Opposite boosts
are aligned to their own directions before averaging. Thus a static lattice
asymmetry changes sign instead of masquerading as a wake.

## 3. Qualification result

The focused WSL2 RTX 5090 CTest
`cuda_transported_chart_morphology` passes in 3.09 seconds. The matrix covers
`L={17,33}`, both polarities, and face/edge/body directions. It verifies:

- CPU/CUDA parity of every near/outer trailing, neutral, and leading energy;
- direct scalar recomputation;
- exact near/outer partition and actual/bound/residual/interference identity;
- integer translation, polarity conjugation, and proper cubic covariance;
- direction reversal exchanging trailing and leading;
- an independently constructed symmetric fixture with zero asymmetry;
- independently constructed trailing and leading fixtures with opposite signs;
- zero complete-field downloads.

The qualification changes only an observer-side reduction and its test. The
production tick, common action, state, field variables, defaults, toggles,
scenarios, and `RenderBridge` are unchanged.

## 4. Executable identity

| Artifact | SHA256 |
|---|---|
| `engine/include/ftd/eft/transported_chart_morphology.h` | `F7200B42DD953DE529532CBBCC241D82EAF181A286CCBBB7BB580ACBB3B07C5D` |
| `engine/src/eft/transported_chart_morphology.cpp` | `A5A53566D2C9CD46AE098D4343F4EB9FA6FEB6AAA45801909D29F3284DD8C5E6` |
| `engine/cuda/cuda_transported_chart_morphology.cu` | `79DF992549FC72DDE42B4F595D22D8765B3BFE60C1509940446B1119D642622E` |
| `engine/tests/test_cuda_transported_chart_morphology.cpp` | `A0583E7F66FF5C423A304CFFDA5F8716B35A65A3B804E76C6F4B576F183ECE8B` |
| `engine/tests/campaign_aged_wake_entrainment_cuda.cpp` | `DD5AE3824E01AC9D168240C5709DBD618AE7084BE3688DF370BFFA0B339AC4CB` |
| WSL2 executable `engine/build_wsl/campaign_aged_wake_entrainment_cuda` | `31795498B066A6359397CF822298791BC5434F49B32387624361051FC6485F9E` |

## 5. Result provenance and status

The registered executable was launched only after the qualification passed.
At this audit checkpoint `engine/results/ftd_0766` is absent because the runner
writes the versioned JSON only after all 21 branches finish. Console progress
is diagnostic and is not a claim of record.

No wake, entrainment, mobility-basin, or matter verdict is assigned here. A
result audit must hash the completed artifact, independently reconstruct every
locked label, and retain any failed amplitude, sign, age, or mirror gate.
