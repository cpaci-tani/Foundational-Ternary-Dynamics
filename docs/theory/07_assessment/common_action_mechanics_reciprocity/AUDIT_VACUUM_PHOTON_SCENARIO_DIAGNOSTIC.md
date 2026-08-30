# FTD-0434 — Exact Vacuum-Photon Scenario Diagnostic

**Status:** `[CLOSED NEGATIVE — HISTORICAL INITIALIZER; REPAIRED IN CURRENT SOURCE]`  
**Date:** 2026-07-23  
**Scope:** canonical `s0-vacuum-photon`, production CPU tick, `L=33`

**2026-07-25 scope update:** This audit remains the run-of-record defect report
for the removed `J_z=g,W_x=g` initializer. The current scenario now uses the
shared transverse one-way plane-packet construction. FTD-0475 measures that
replacement at `L=49,65`: both arms pass its co-moving-bound morphology clause
and neither passes its detached-wake clause. Photon identity and quantization
remain open.

## Verdict

The canonical initializer does not produce a demonstrated propagating photon
packet. It seeds two orthogonal componentwise wave degrees of freedom. Under
the isolated production wave operator the structure remains centered and
spreads. Under the browser-equivalent profile it is strongly rewritten and
drifts only 2.64 sites by tick 20, far below the locked translating-packet
gate and far below the expected `20*C_WAVE = 11.55` sites.

This closes the scenario's current claim, not native wave propagation in
general and not the possibility of a corrected photon construction.

## Defect in the initial condition

At `engine/src/scenarios/vacuum.cpp:115-116`, the scenario seeds

\[
 J=(0,0,g),\qquad W=(g,0,0).
\]

The production wave update is componentwise: `phase_read` computes a
Laplacian of each component and `phase_write` applies

\[
 W_i\leftarrow W_i+\Delta J_i,\qquad J_i\leftarrow J_i+W_i.
\]

Therefore `W_x` is the time derivative of `J_x`; it is not a declaration that
the `J_z` profile should move in the `+x` direction. A right-moving `J_z`
profile instead requires, to leading centered-difference accuracy,

\[
 W_z=-C_{\rm WAVE}D_xJ_z.
\]

The measured normalized residual from that relation is exactly `1.0`, and the
wave-velocity fractions are `(x,y,z)=(1,0,0)`.

The initializer also has nonzero divergence because a localized
`J_z=g(x,y,z)` has `partial_z J_z != 0`. Gauss projection consequently changes
the displayed vector geometry, but it does not satisfy the pre-registered
100-fold `PROJECTION-DOMINATED` discriminator: normalized divergence falls by
only a factor `4.04` on tick 1 while the flux norm changes by `30.6%`.

## Run-of-record measurements

| Arm | tick-20 displacement | width ratio | best shift | overlap | locked result |
|---|---:|---:|---:|---:|---|
| browser-equivalent dashboard | 2.63775 | 3.57577 | 2 | 0.702512 | not translating |
| production wave only | 1.78e-15 | 3.19477 | 0 | 0.623226 | nontranslating/splitting |

In the wave-only arm, the first tick creates `sum J_x^2 = 157.0245` directly
from the seeded `W_x`, while the `J_z` profile begins changing independently.
By tick 20 the original-center profile has broadened by a factor `3.19` with
zero best-fit shift. The browser-equivalent arm also fails translation: its
mean apparent speed through tick 20 is `0.132` sites/tick, versus
`C_WAVE = 0.577`.

## Correct statement

`s0-vacuum-photon` currently visualizes the evolution of a localized,
non-transverse flux/wave-velocity seed. Its streamline morphology is not
evidence that a photon propagates in the engine. The current result does not
establish photon identity, photon quantization, helicity, Maxwell gauge
structure, or a common Lorentz cone.

## Reproducibility

- Pre-registration:
  `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_VACUUM_PHOTON_SCENARIO_DIAGNOSTIC_v2.md`
- Source lock:
  `scripts/proofs/vacuum_photon_scenario_diagnostic_lock.json`
- Run manifest: `engine/results/ftd_0434/manifest.json`
- Run record: `engine/results/ftd_0434/windows_msvc_cpu_L33.csv`
- Result verifier:
  `scripts/proofs/proof_vacuum_photon_scenario_diagnostic_results.py`

The revision-1 dashboard arm used native defaults rather than browser defaults
and is excluded. Its file is retained as provenance at
`engine/results/ftd_0434/invalid_v1_native_defaults_L33.csv`.

An additional loose-root output was consolidated on 2026-08-30 at
`engine/results/ftd_0434/unregistered_legacy_root_output.csv`. It has no run
manifest and its dashboard arm records `execution_valid=0`; it is retained as
unregistered provenance and does not enter the revision-2 verdict.
