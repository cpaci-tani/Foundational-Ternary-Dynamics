# LEDGER row drafts: engine wave-sector isotropy and genesis symmetry (2026-09-14)

Date: 2026-09-14. Draft only — **not booked into `LEDGER.md`**; booking is the
owner's act. Nothing here changes canonical Φ-v2, v3, or any constant. All
rows concern the **engine's v1 wave/genesis law** (`C_SPEED = 1/√3`, 18-point
stencil), not the v3 common-action Φ.

Sources: `scripts/proofs/proof_stencil18_symbol_isotropy.py`,
`engine/tests/test_stencil18_dispersion.cpp`, `engine/tests/test_genesis_symmetry.cpp`,
`engine/tests/test_gauss_projection_symmetry.cpp`, `engine/tests/test_scenario_behavior.cpp`
(odd-L replay block). Measurement record: memory note
`project_2026_09_14_engine_stencil_isotropy_and_streamline_artifact.md`.

**Next-free id.** `python scripts/audit/check_registry.py` (2026-09-14) reports
`NEXT FREE FTD ID: FTD-1042`; re-run immediately before booking.

## FTD-1042 — The engine's wave operator is the 18-point stencil whose symbol has no fourth-order cubic anisotropy

| Field | Value |
|---|---|
| Tag | `[THEOREM]` (stencil algebra) + `[MEASURED]` (engine realises it) |
| Claim | `Lap = −4δ + (1/3)Σ_face + (1/6)Σ_edge`; `L̂(q) = −|q|² + |q|⁴/12 + O(q⁶)`; the coefficient of `H4 = Σq_i⁴ − (3/5)|q|⁴` at fourth order is exactly 0; the q⁶ coefficient on unit directions is −1/360 (axial), −1/240 (face), −11/3240 (body). Plane-wave frequencies for axial, face- and body-diagonal m=1 modes and axial m=2 at L=33 match `sin²(ω/2) = (C²/4)(−L̂)` to 7 figures; envelope flat to 5×10⁻⁵ over 400 ticks. |
| Evidence | proof script (7 exact checks); CTest `stencil18_dispersion`; live browser measurement 2026-09-14 (ratios 1.0000000 / 0.9999996 / 0.9999997 / 0.9999999). |
| Boundary | Engine law only. Not the v3 Φ (whose transverse coefficient is 1/6 with a scalar pair at 1/√27). This is the engine's answer to the manuscript's falsification-contract-5 question (H4 coefficient) for its own wave sector. Moves no v3 tag. |
| Depends on | FTD-0407 (`C_SPEED` `[SELECTION]`). |

## FTD-1043 — A symmetric seed does not stay O_h-symmetric under the shipped genesis stack; two mechanisms isolated

| Field | Value |
|---|---|
| Tag | `[MEASURED]`; `[CLOSED NEGATIVE]` for the reading "the six-axis seed recovers O_h symmetry" |
| Claim | Six-axis A=20, T=0 seed at L=33, all 48 signed permutations checked per tick: (i) wave-only update: flux asymmetry ≤ 10⁻¹⁵ for 120 ticks (exactly covariant); (ii) Gauss projection alone (6 SOR sweeps): 1.0% at tick 1 → 30% by tick 40; (iii) genesis alone: manifested set breaks at tick 1 (11 sites, 5 outside full orbits); (iv) shipped: broken at tick 1. Mechanisms: manifestation is a per-voxel Bernoulli draw `voxel_uniform(seed,i,tick) < 1 − exp(−excess/K_MANIFEST)` keyed by voxel index (T=0 disables Langevin noise, not this draw; `manifest_at` writes only its own voxel); the CPU Gauss solve is an 8-colour SOR run 6 sweeps per tick by default. |
| Consequence | Covariance of the stochastic rule holds for the ensemble, not per history. `genesis_deterministic` (default OFF; replaces the manifestation, evaporation and spin tie-break draws by their deterministic limits) makes the covariant sub-law testable: with it on and Gauss off the manifested set is exactly O_h-invariant for 120 ticks (CTest `genesis_symmetry`). SOR sweep count is now browser-settable; asymmetry vs sweeps is pinned (CTest `gauss_projection_symmetry`). |
| Boundary | Engine implementation facts; no v3 content. |

## FTD-1044 — Seed-direction particle counts at A=10 are within scatter; cluster geometry is lattice-locked

| Field | Value |
|---|---|
| Tag | `[MEASURED]`; supersedes any reading of `test_scenario_behavior.cpp`'s even-L=24 counts as a direction signal |
| Claim | Same law, T=0.005, Langevin seed 1, periodic, tick 120 (axial / body-diag / six-axis): L=25 → 1/3/6, L=33 → 4/4/6, L=41 → 2/3/7; generic single-site directions at L=33: face-diag 3, (1,2,3) 4, (2,1,0) 5, (3,2,1) 5. Every run's particles sit on the centre's SC face neighbours (plus one FCC edge site for two seeds); a body-diagonal seed never produced a body-diagonal particle. At A=20, T=0: axial cluster 5×3×3 with antlers at (±2,0,0), body-diagonal 3×3×3 with none. |
| Agreement | FTD-0110 T8 (2026-04-27, L=32 GPU): body-diagonal cluster size within ~5% of axial. |
| Amendment to FTD-0110 / FTD-0276 provenance | Since FTD-0276 (2026-06-12) `kinetic_drain = 0.5` is honoured on both CPU and GPU. The 2026-04-27 rows and the scenario comments describing "CPU drains, CUDA does not" predate that change; the ¼·A² cluster-law calibration (≈25-voxel clusters at A=10) was measured before drain unification. Today's engine gives ≈4 at A=10 and 22–26 at A=20 on the CPU/WASM path. |
| Boundary | Engine measurements; no v3 content. |
