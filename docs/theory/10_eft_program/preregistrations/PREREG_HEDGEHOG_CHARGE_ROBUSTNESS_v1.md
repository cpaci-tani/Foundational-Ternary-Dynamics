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

## AMENDMENT v1.1 (2026-07-20, before any re-run) — V2 target correction

**Run 1 result:** V2 (`e_half` reproducibility) FAILED for all three dynamical arms — measured values (1.368676308503 / 5.828246462835 / 0.540720277788) did not match the `known_e_half` constants baked into the instrument (1.709171333089 / 7.221033315847 / 0.781682031101). Q for Arms A/C/E came back ≈0.000000 with large RMS angular deviation (134–153°); the S_synthetic arm (Q=1.000000, rms_dev=0.000°, V3 converged) was unaffected.

**Root cause, confirmed by re-running the original, already-committed `campaign_manifestation_seed_diversity` binary against the current build:** its own V4 self-check still passes (`seedA_matches_known=1`, e_half=1.709171333089 exactly) — the underlying dynamics have not drifted. But that campaign's SEED rows carry *two* e_half fields: `e_half` (post-relaxation, the headline number this document's V2 gate wrongly used as the target) and `e_half_prerelax` (measured at freeze, before relaxation). This instrument measures Q at freeze time — before relaxation — so the correct V2 target was always `e_half_prerelax`, not `e_half`. Cross-checked directly: the re-run's `e_half_prerelax` values (1.368676308503 / 5.828246462835 / 0.540720277788) are bit-identical to Run 1's "failing" measurements. **The measurement was correct throughout; the V2 reference constant was copied from the wrong column.**

**Fix:** `known_e_half` in the three `SeedSpec` rows corrected to the `e_half_prerelax` values. No other change — §1–§3 (design, gates, frozen reading bands) stand exactly as registered. This is a reference-constant correction to a reproducibility check, not a change to the falsifiable Q-robustness prediction, which was never touched by Run 1's data.

Re-run authorized under v1.1. Run 1's Q/rms_dev numbers for Arms A/C/E remain scientifically valid data (the field state they were computed from was never in question) but are formally VOID under the v1-as-registered gate; the v1.1 re-run is canonical.

---

## OUTCOME (2026-07-20, v1.1 re-run) — ROBUST, but robustly ZERO, not ±1

All gates pass. Data (identical to the Run-1 measurements, now correctly gated):

| Arm | e_half (freeze) | fire_tick | Q | RMS angular deviation |
|---|---|---|---|---|
| A_baseline | 1.368676308503 | 2 | −0.0000000000 | 143.14° |
| C_hot | 5.828246462835 | 2 | +0.0000000000 | 134.72° |
| E_cold | 0.540720277788 | 2 | −0.0000000000 | 152.95° |
| S_synthetic (reference) | 0.478915969616 | — | +1.0000000000 | 0.00° |

**Primary verdict, per the frozen §3 bands as literally written:** spread among Q_A/Q_C/Q_E is ≈1e-10 (< 0.10) and each is within 0.05 of an integer (namely 0) → **ROBUST**. The hedgehog charge is pinned across the same three birth circumstances that produced a 9.2× energy spread — exactly the property energy lacked in the closed constraint-energy arc.

**The reported (non-gating) comparison is the important part:** Q_S = +1 (the synthetic reference, matching the pure-radial injection's own construction) does **not** match Q_A = Q_C = Q_E = 0. The topological charge present at injection (Q=1, by construction of the radial pulse) is not preserved to freeze time under real dynamics — it is driven to *exactly* zero, reproducibly, within 2 ticks, independent of amplitude across a 2.3× range (2.15 to 5.00). The RMS angular deviation (135–153°) confirms this is not "nothing happened": the field's direction on the measurement shell has been substantially reoriented by freeze time, consistent with a genuine topology change (Q: 1 → 0), not measurement noise around an unchanged configuration.

**Reading:** §1's structural argument holds — Q is robust where energy is not, confirmed directly rather than assumed. But the specific hoped-for picture (a nonzero, conserved charge that could anchor a Bogomolny-type mass floor, the way a soliton's winding number does) does not survive: real manifestation dynamics collapses the octahedral-shell charge to the *trivial* topological sector for all three tested seeds. A trivial sector carries no floor above zero in the standard construction, so this specific charge cannot be the rest-mass invariant as originally hoped — that specific route is closed. What is newly established, and was not known before this test: manifestation (or the two ticks of dynamics preceding it) is topology-erasing on this shell, consistently and amplitude-independently. Whether the charge is genuinely destroyed or has migrated to a larger enclosing shell (the k=2 cuboctahedron layer, or further out) is open and would need its own freshly-designed, freshly-pre-registered test — not pursued here.

**Consequence for `DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md`:** §4's question is answered (ROBUST) but the answer does not support the document's §1 hoped-for conclusion (a nonzero mass-anchoring charge). Status updated accordingly.

---

*Registered 2026-07-20, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion: `DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md`, `preregister-manifestation-seed-diversity-v1` (source of the three reused seeds).*
