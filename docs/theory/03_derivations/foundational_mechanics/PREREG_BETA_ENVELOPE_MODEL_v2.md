# PRE-REGISTRATION — Refined Mechanism Beta Envelope Model, v2

**Tag:** `[PRE-REGISTRATION]` (locked before the verdict). **Supersedes the linear Variant A/B formulations of v1.**
**Date:** 2026-06-10
**LEDGER:** FTD-0263 (continued).
**Builds on:** [`ANALYSIS_BETA_ENVELOPE_MODEL_v1.md`](ANALYSIS_BETA_ENVELOPE_MODEL_v1.md) + its partial resolution.

---

## §1 · Why v2

The v1 envelope analysis (`BETA-PARTIAL`) demonstrated that while the linear wave envelope locates the sub-knee onset boundary, the staircase shape and quantitative thresholds are not purely geometric. Specifically:
- Naive linear wave propagation (Variant A) predicts nearest-neighbor ($r=1$) manifestation at $A \approx 5.62$.
- The actual C++ engine does not manifest any nearest-neighbor voxels until $A \ge 8.5$, indicating a significant threshold shift.

v2 refines the model by incorporating the center voxel's manifestation back-reaction at $t=1$. This back-reaction is composed of:
1. **Kinetic Energy Drain**: Wave velocity is scaled by $(1 - K_{\text{genesis\_kinetic\_drain}}) = 0.5$.
2. **Latent Heat (Flux Drain)**: Voxel flux is scaled by $\max(0, 1 - K_{\text{genesis}}/|J|)$.
3. **Gauss Projection**: Divergence projection ($J \to J - \nabla \phi$) is solved using Red-Black SOR sweeps, subtracting the longitudinal static Coulomb field from the neighboring ($state == 0$) voxels.

---

## §2 · The Question (LOCKED)

Does the inclusion of the center voxel's manifestation back-reactions (kinetic drain + flux drain + Gauss projection) explain the shift in the nearest-neighbor manifestation threshold from the naive $A \approx 5.62$ up to the measured $A \approx 8.5$?

---

## §3 · Method (LOCKED)

- We simulate the discrete wave equations on a 3D grid ($L=16$) under three comparative cases:
  - **Case 1 (Naive)**: No center voxel back-reaction (pure linear propagation).
  - **Case 2 (Drains Only)**: Center voxel kinetic and flux drains applied at $t=1$. No Gauss projection.
  - **Case 3 (Full Back-Reaction)**: Case 2 + discrete Gauss projection solver (6 SOR iterations matching the engine).
- At each amplitude $A \in [4.0, 10.0]$, we run $30$ seeds with Langevin thermal noise ($\gamma = 0.02$, $T_L = 0.005$) and measure:
  - The peak flux magnitude $J_{\text{peak}}$ at $r=1$ over $T=10$ ticks.
  - The thermal crossing probability $P_{\text{manifest}}$ of the $K_{\text{genesis}} = 1.533$ threshold.

---

## §4 · Benchmark + decision (LOCKED)

We evaluate the model using three frozen thresholds:
- **C1 — Naive Threshold**: Case 1 crosses the $50\%$ probability mark near the naive prediction $A \approx 5.62$.
- **C2 — Back-Reaction Suppression**: Case 3 significantly suppresses the neighbor flux relative to Case 1 at all amplitudes (at least a $15\%$ reduction in peak neighbor flux at low amplitudes).
- **C3 — Shift to Measured Onset**: Case 3 crosses the $1\%$ onset probability threshold at $A \ge 8.0$, showing a transition from $0\%$ to $>15\%$ manifestation probability between $A=8.0$ and $A=9.0$, matching the observed $A \approx 8.5$ onset on the current stack.

---

## §5 · Pre-blessed Outcomes (LOCKED)

- **BETA_v2_CONFIRMED** — All three criteria (C1, C2, C3) pass.
  → `[MEASURED — Mechanism Beta v2 resolves the sub-knee onset threshold shift]`; the analysis will be documented in `ANALYSIS_BETA_ENVELOPE_MODEL_v2.md`.
- **BETA_v2_FAIL** — One or more criteria fail.
  → The threshold shift is not explained by the center back-reaction; further exploration is required.

---

## §6 · Falsifiers + Banned Moves (LOCKED)

- **F-a** No tuning of engine constants ($K_{\text{genesis\_kinetic\_drain}} = 0.5$, $K_{\text{genesis}} = 1.533$, $\alpha = 1/18$).
- **F-b** Langevin parameters must be held at their canonical values ($T_L = 0.005$, $\gamma = 0.02$).
- Banned: No post-hoc data selection; no adjusting of the Poisson solver iterations to fit the curve.
