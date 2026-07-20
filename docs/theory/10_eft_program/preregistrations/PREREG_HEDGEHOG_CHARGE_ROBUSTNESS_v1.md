# PREREG — Is the hedgehog topological charge robust where energy was not?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-hedgehog-charge-robustness-v1` at the registration commit)
**Parent:** `DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md` §4. Formula validated pre-lock: `scripts/exploration/validate_hedgehog_charge.py`, 6/6 hand-constructed test cases with known degree passed (rotation-invariant, magnitude-invariant, correct sign, correct null).

## 1 · Design

Reuse the three seeds from `PREREG_MANIFESTATION_SEED_DIVERSITY_v1.md` that passed every validity gate cleanly (A_baseline, C_hot, E_cold — identical `SeedSpec` parameters, identical natural-dynamics protocol: `wave_propagation, coupling, gauss_projection, genesis, damping, selective_damping` ON, run until exactly one voxel manifests). At the instant of freeze (first manifestation, before any relaxation), compute the Berg–Lüscher hedgehog charge Q on the octahedral shell (the 6 face-neighbors of the manifested voxel) using the validated formula.

A fourth arm reproduces the FTD-0388 synthetic-charge setup: a single `state=+1` voxel with all other dynamics OFF, Gauss-projected only to its fixed point, Q measured on the same octahedral shell around that voxel — the idealized reference.

Each arm also reports the RMS angular deviation of the freeze-time direction field from a pure-radial reference on the same shell, as a diagnostic (not a pass/fail gate): this confirms whether the field's *direction* actually moved away from its t=0 radial injection by freeze time, so a "Q unchanged" result can be read as non-trivial robustness rather than "nothing happened."

## 2 · Validity gates (per arm — a failure VOIDs that arm, not the whole campaign)

- **V1:** `|J| > 1e-6` at all 6 face-neighbor sites at measurement time (else Q is undefined there).
- **V2 (Arms A/C/E only):** `e_half` at freeze reproduces the known seed-diversity value to 1e-9 — confirms this is a faithful re-run of the identical protocol, not a silently-diverged instrument.
- **V3 (Arm S only):** relaxation converges (residual gate met, not cap-exhausted) — identical protocol to every prior self-energy/relaxation arm this session.

## 3 · Frozen reading (stated before running)

Let Q_A, Q_C, Q_E be the three dynamical arms' charges and Q_S the synthetic reference, restricted to arms that pass V1.

| Band | Reading |
|---|---|
| `max(Q_A,Q_C,Q_E) − min(Q_A,Q_C,Q_E) < 0.10` **and** each within `0.05` of the nearest integer | **ROBUST.** The hedgehog charge is pinned across birth circumstances that produced a 9.2× energy spread. Genuine support for §1 of the parent document's hypothesis — proceed to formulating a Bogomolny-type energy/Q relation as the next target. |
| Spread `≥ 0.10`, or any arm more than `0.05` from an integer | **NOT ROBUST.** The topological-invariant route closes negative at its first test with this specific construction (octahedral shell, freeze-time measurement). Do not re-attempt with the same surface/timing without a new, independently-motivated reason to expect a different result. |
| Between | Report the actual numbers and the RMS-angular-deviation diagnostic for each arm; do not force a verdict either way. |

Separately, **report** (does not gate the primary verdict): whether Q_S matches the dynamical arms' common value (if ROBUST) — same topological sector for idealized and genesis-born charges is the expected, unsurprising case; a mismatch would itself be a notable, separate finding about the difference between idealized and dynamical manifestation.

`fire_tick` and the RMS-angular-deviation diagnostic are recorded for every arm regardless of verdict.

---

*Registered 2026-07-20, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion: `DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md`, `preregister-manifestation-seed-diversity-v1` (source of the three reused seeds).*
