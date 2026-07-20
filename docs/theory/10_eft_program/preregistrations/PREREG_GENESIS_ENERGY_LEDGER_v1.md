# PREREG — The Genesis Energy Ledger: does a REAL charge lock W_SC(L)?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-genesis-energy-ledger-v1` at the registration commit)
**Question owner:** the falsifier specified in §3 of `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` (2026-07-19): whether M_REST = W_SC — rest mass as the energy the Gauss constraint permanently forces a manifested voxel to hold — extends from a *synthetic* unit charge (FTD-0388's measured object) to a *genesis-born* one.
**Engine state at lock:** post Term-2 coupling-sign amendment and post proper-time-hazard amendment (both 2026-07-18/19).

---

## 1 · Disclosure of pre-lock instrument validation (read before the design)

This instrument required two rounds of fixing before it produced a trustworthy number, and — because FTD is fully deterministic — those pilot runs are not erasable "peeking": the same code with the same parameters will reproduce the same output every time, so hiding the pilot numbers from this document would be theater, not rigor. They are disclosed here as design rationale, exactly as the deflection and two-clock campaigns disclosed prior-cycle numbers when locking their next revision.

**Bug 1 — stale `delta_j_` runaway (found, fixed).** `phase_write`'s leapfrog (`wave_vel += delta_j_; flux += wave_vel`) is **not gated by `wave_propagation`** — only `phase_read` (which *computes* `delta_j_`) is. Toggling `wave_propagation` off after Phase A's real dynamics and then iterating projector-only left `delta_j_` at its last-computed, nonzero value; every subsequent tick re-added it, compounding into a runaway (observed: `e_half` 6 → 1.3×10¹⁴ within ~90 applications). Fixed by relaxing on a **fresh** `RenderBridge` — copy state + flux across, leave `wave_vel` at its zero-initialized default, never enable `wave_propagation` on the fresh instance — the exact pattern `test_gauss_law_fidelity.cpp`'s own freeze arm already documents ("copy the flux onto a fresh bridge instead").

**Bug 2 — seed geometry (found, fixed).** The first radial-pulse parameterization left 2 sites above `K_GENESIS`, not the required 1. Retuned (amplitude 3.0, σ = 0.45) so only the nearest site to the generic virtual center clears threshold (next-nearest sits at ≤ 0.89 vs. threshold 1.516 — safe by a wide margin). Confirmed empirically: `sites_above_threshold_preseed = 1` in both arms after the fix.

**The pilot's physics result, disclosed:** with both bugs fixed, `e_half`(G-early) = **1.709171333089**, `e_half`(G-late, +5 ticks) = **1.281809176362** — both far from the synthetic prediction W_SC(17) = 0.478917129, and both stable/reproducible (no divergence, clean residual convergence in ~1200 applications, matching Arm S's own convergence count). The raw seed pulse itself carries `e_half` = 2.892 before any dynamics run; Phase A's combined wave+coupling+gauss dynamics pull this down to 1.369 by the freeze tick, and Phase B's projector-only relaxation then pushes it *up* to 1.709 (building the field toward the newly-nonzero-state constraint). G-late's lower value than G-early — despite *more* pre-freeze dynamics time — shows the excess **shrinks** under additional damped relaxation rather than growing; it is not a runaway, but it is also not shrinking toward W_SC within any window this design tests.

This document locks the instrument exactly as validated above. The run following this lock is a **byte-for-byte reproducibility check** of the disclosed numbers (full determinism — no code changes pending), not a blind trial; its purpose is to confirm the locked binary reproduces the pilot exactly (itself a real check — it would fail if any of the above were fragile to build/environment specifics) before the outcome is booked.

## 2 · The confound this design is built around

`gauss_project_cpu`'s correction is `voxels[i].flux -= grad_phi` — a pure gradient, hence **curl-free by construction** (confirmed by reading `poisson_solvers.cpp` before writing this instrument). Repeated projection can only ever correct the *longitudinal* (divergence) part of J; any *transverse* (divergence-free) content present when projection begins is invariant under it, forever. A synthetic charge starts at J=0 (zero transverse content, trivially) and so provably stays there — this is why Arm S reproduces W_SC(L) to sub-ppm. A genesis-born charge's field is whatever real dynamics left behind, and — as §1's disclosed numbers show — that is *not* transverse-free, even from a curl-free radial seed.

## 3 · Design

`engine/tests/campaign_genesis_energy_ledger.cpp`, locked at the registration commit.

**Energy observable** (reused verbatim from `scripts/proofs/prereg_selfenergy_pinning_predictions.py`'s canonical "tracker convention"): `E_half = (1/2) · Σ_all_voxels |J|²`.

**L = 17** (matches the FTD-0388 pinning triple; frozen closed-form predictions: P1/W_SC family 0.478917129, P2/matched-18pt alternative 0.151842301 — a strong discriminator between the two candidate operator families, per the original pinning script).

**Arm S (sanity/validation):** synthetic +1 charge at lattice center, J=0 elsewhere, relaxed on a fresh bridge to the residual-1e-8 fixed point (cap 5000 applications). Reproduces the FTD-0388 GF-A protocol exactly. **Validity gate:** must match 0.478917129 within the established ≤0.00084% tolerance, else VOID (harness bug).

**Arm G-early:** a curl-free radial flux pulse (peak amplitude 3.0, σ=0.45, cutoff 4, centered at a generic non-lattice-symmetric offset (mid+0.31, mid+0.17, mid+0.07) so exactly one site is nearest and gets the peak) is seeded; realistic dynamics (`wave_propagation, coupling, gauss_projection, genesis, damping, selective_damping` ON — no `movement`, no forces/gravity/poisson_coulomb/lorentz_force, no `weak_transmutation`) run tick-by-tick until the seeded site manifests; frozen at that exact tick; relaxed on a fresh bridge to the same fixed point.

**Arm G-late:** identical, but Phase A continues 5 further ticks after manifestation before freezing — the debris-evolution diagnostic.

**Validity gates:** (V1) exactly 1 site above `K_GENESIS` pre-seed; (V2) manifestation occurs within 200 ticks and exactly 1 voxel is manifested at freeze; (V3) both G-arms' relaxation converges (residual gate met, not cap-exhausted) and never exceeds `e_half` = 10¹² (the Bug-1 divergence signature) before doing so.

## 4 · Pre-committed discriminator (stated before the numbers are re-confirmed post-lock)

The question that matters is not "does it match 0.4789 to sub-ppm" — §1 already shows, honestly, that it does not, by a factor of ~2.7–3.6×. The question this locks a verdict on is:

| Condition | Reading |
|---|---|
| G-early and G-late reproduce §1's disclosed values (determinism check) AND both converge cleanly (V3) | Instrument confirmed working; proceed to interpretation below |
| The G-arm excess over W_SC(17) is **stable and reproducible**, not numerical noise | Supports: genesis dynamics genuinely leave transverse (non-minimal) field content, not an artifact |
| G-late < G-early (excess shrinks with more damped relaxation time) | Supports: a slow-relaxing residual, not a runaway or a stable alternative fixed point |

## 5 · Outcome (booked, not deferred — the numbers are already known and disclosed in §1)

**REFUTED AS STATED.** The naive form of the identification proposed in `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` — "the constraint locks exactly W_SC(L), regardless of how the charge was created" — does **not** hold for a genesis-born charge under this engine's current birth mechanics. `E_half`(G-early) = 1.709 (3.57× W_SC(17)); `E_half`(G-late) = 1.282 (2.68× W_SC(17)) — both stable, reproducible, cleanly converged, and *not* runaway artifacts (Bug 1's signature — divergence past 10¹² within ~100 applications — is qualitatively distinct from this smooth, bounded, decreasing-with-relaxation-time result).

**What is and is not refuted.** FTD-0388's synthetic-charge result is untouched: W_SC *is* the Gauss constraint's minimal demand for an idealized point charge, measured cleanly to ≤0.00084%. What fails is the *bridge* — the assumption that a real manifestation event trivially inherits that minimal-norm configuration. It does not: genesis leaves the charge non-minimal, and the excess relaxes slowly (shrinking under 5 extra ticks of damped dynamics, not vanishing) rather than either being negligible or diverging.

**Most likely mechanism (flagged, not measured — a real open question, not a claim):** the kinetic-drain operation (`wave_vel *= (1 − K_GENESIS_KINETIC_DRAIN)`, applied only at the manifesting site) is a spatially localized, single-point multiplicative operation — generically NOT curl-free, unlike the radial seed or the coupling source term (both gradients by construction). This campaign does not isolate or measure curl content directly and does not claim to have identified the mechanism; it establishes only that *some* transverse-content-introducing process is active during genesis, consistent with, but not proof of, the kinetic drain being that process.

**Consequence for the rest-mass derivation:** §5 of `DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md` (identification C2) does not close on this measurement. The document's claim ledger is updated accordingly (see that file). This is a genuine boundary result, not a dead end: the constraint-locked-energy *idea* survives (W_SC remains the geometrically forced minimal value); its *literal, unconditional* application to real particles does not, and the gap is now named and measured rather than assumed away.

---

*Registered 2026-07-19, after mandatory pre-lock instrument validation (§1, disclosed in full per FTD's determinism — no blind trial was possible or pretended). Author: session 8294fddb, following LOCK-STD v1.*
