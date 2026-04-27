# PROTOCOL — Topological Observable Mapping (engine-native exploration)

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-27
**LEDGER row:** FTD-0104 (assigned ahead of measurement)
**Companion:** [`PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md`](PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md), [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md)
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (Campaign D of the engine-as-instrument portfolio)

This protocol is **pre-registered before measurement** per CLAUDE.md epistemic discipline rules.

---

## 1 · Why this protocol exists

Per the user's 2026-04-26 reorientation, the engine should be exercised as a primary instrument rather than a fitter to SM expectations. Four topology-seeded scenarios already exist in `engine/src/scenarios/s0_seed.cpp` (see lines 372–418):

- `s0-seed-wilson-loop` — closed planar Wilson loop of radius R
- `s0-seed-flux-tube` — confining string between ±1 charges at separation L/4
- `s0-seed-monopole` — radial 1/r² Dirac-monopole field
- `s0-seed-instanton` — localized self-dual blob with size scale 3.0

These have been used opportunistically (e.g. `engine/tests/benchmark_wilson_loops.cpp`) but never measured under a unified protocol that asks: **what topological structures does the engine actually support natively, and what is their lifetime / stability / interaction structure under Langevin?**

This protocol pre-registers a single sweep that exercises all four scenarios under matched lattice + Langevin parameters and emits a common observable schema. The output is descriptive (a "topology atlas") rather than fit-driven — engine-native phenomenology, no SM-comparison required.

It addresses (partially or in full):

- The user's reorientation toward engine-as-instrument exploration
- STATUS_EFT_CHECKLIST.md §7 (Wilson loops were partial-only — this elevates them to a structured measurement under a common schema)
- Existing topology-seeded scenarios were under-measured

---

## 2 · Pre-registered sub-experiments

Four sub-experiments, each on its own scenario, with a shared measurement schema. All run at L=32, Langevin T=0.005, γ=0.02, gauss-projection ON, 5 seeds per parameter setting.

### Sub-experiment D1 — Wilson loop area-law structure
- Scenario: `s0-seed-wilson-loop`, with custom radii R ∈ {4, 6, 8, 12}
- Burn 200 ticks; sample every 50 ticks for 2000 ticks
- Per snapshot: total Wilson-loop trace W(R) = ⟨exp(i ∮ A·dl)⟩ approximated by the path-integrated J·dl on the seeded loop
- Per run: extract σ_eff from log W(R) ≈ −σ·area + const

### Sub-experiment D2 — Flux-tube tension and breaking
- Scenario: `s0-seed-flux-tube`, with separations r ∈ {6, 8, 10, 12}
- Same burn / sample schedule
- Per snapshot: tube line-integral energy E_tube(t), tube transverse profile FWHM, tube length (last voxel above 0.5·K_B threshold from each end)
- Per run: tube tension σ_tube = E_tube/length; tube-breaking event flagged when length drops below 50% of seeded value for ≥100 ticks

### Sub-experiment D3 — Monopole / anti-monopole confinement
- Scenario: `s0-seed-monopole` (single) and a 2-monopole variant (anti-monopole at r = L/4 from monopole — implemented as one-time scenario hook below)
- Same burn / sample schedule
- Per snapshot: monopole position (centroid of |J| within 4 voxels of seeded location), monopole "core" voxel count
- Per run: monopole drift trajectory, decay time (when core voxel count drops below 50% of initial), inter-monopole force estimate (from displacement statistics in 2-monopole case)

### Sub-experiment D4 — Vacuum instanton background
- Scenario: empty lattice + Langevin only (NO topology seed) plus `s0-seed-instanton` arm
- Same burn / sample schedule
- Per snapshot: lattice-summed Pontryagin-density estimator Q_top = (1/32π²) Σ ε^{μνρσ} F_μν F_ρσ ≈ Σ J·(∇×J) on the lattice
- Per run: |Q_top| time-series, mean and stderr; histogram of per-snapshot Q_top values

The 2-monopole variant in D3 is the only sub-experiment that needs a new scenario hook; D1/D2/D4 use existing scenarios with parameter sweeps via campaign-side CLI flags.

---

## 3 · Pre-registered observables (shared schema)

Per snapshot CSV columns (same across D1–D4 where applicable):

| Column | Description |
|---|---|
| `tick` | Simulation tick |
| `total_density` | Sum |J|² over lattice |
| `total_charge` | Σ s_i |
| `manifested_voxels` | Σ [s_i ≠ 0] |
| `Q_top` | Pontryagin estimator (D4 + others) |
| `W_R` | Wilson trace at scenario radius (D1) |
| `E_tube` | Line-integrated tube energy (D2) |
| `tube_length` | Active tube length (D2) |
| `monopole_core_voxels` | Centered |J|² above threshold (D3) |
| `monopole_centroid_x/y/z` | Drift position (D3) |

Per run JSON `meta.json`: scenario, parameter settings, all 5 seeds, derived per-run values (σ_eff, σ_tube, decay_time, mean_Qtop), elapsed time.

Volumetric snapshots emitted at 3 characteristic moments per run (post-burn, mid-run, end-of-run) via `csv::export_flux_field` for offline visual confirmation.

---

## 4 · Pre-registered outcome interpretations

Each sub-experiment has its own pre-registered outcome grid:

### D1 (Wilson loop)
- **W1** Clean area-law: log W(R) ≈ −σ·R² + c, σ stable across seeds (relative stderr < 25%) → confirms confinement on Langevin lattice; report σ
- **W2** Sub-area-law (perimeter): log W(R) ≈ −σ_p·R + c → deconfined or Coulomb-phase signature
- **W3** No clean fit: residual > 30% relative → Wilson loop signal lost in Langevin noise at this lattice/temperature; report stable upper bound

### D2 (Flux tube)
- **T1** Tension matches D1's σ within 30%: confinement is consistent across observables
- **T2** Tension differs from D1's σ by > 30%: scenarios produce different effective confinement; report both
- **T3** Tube breaks within burn: confinement is unstable at this Langevin coupling; report breaking time distribution

### D3 (Monopole)
- **M1** Single monopole stable for full run (< 20% core decay): monopoles are robust localized topology
- **M2** Single monopole decays > 50% within run: monopoles are unstable on Langevin lattice
- **M3** 2-monopole pair attracts (separation drops): confining force between topological charges measurable
- **M4** 2-monopole pair drifts independently: no measurable inter-monopole force at L=32

### D4 (Vacuum instanton)
- **I1** Q_top fluctuations stable, mean ≈ 0, stderr finite: vacuum supports topological-charge fluctuations
- **I2** Q_top mean shifts away from 0: lattice has charge bias (concerning for parity claims)
- **I3** Q_top noise dominated by integration error: estimator unreliable at L=32; defer to L=64

ALL outcomes are publishable. Negative results (W3, T3, M2, I3) are informative and qualifying per the user's reorientation (qualitate negative results).

---

## 5 · Falsifier and acceptance gate

This is an **exploratory** protocol — no single falsifier, no SM-target. Acceptance criteria for the campaign as a whole:

1. All four sub-experiments produce non-degenerate output (no all-zero, no machine-precision noise dominating)
2. Per-sub-experiment outcome (W1–W3, T1–T3, M1–M4, I1–I3) is uniquely identifiable from data + bootstrap stderr
3. Cross-sub-experiment consistency check: σ_eff (D1) and σ_tube (D2) reported with their relative agreement
4. Volumetric snapshots successfully emit at 3 moments per run

Failure of any item triggers a re-run with adjusted parameters, not a re-write of the outcome grid.

---

## 6 · Implementation outline

- New: `engine/tests/campaign_topological_observables_2026-04-27.cpp` (~400 LOC)
  - 4 scenario harnesses sharing a per-snapshot CSV writer
  - Custom-radius Wilson-loop hook (overrides scenario's auto-radius) for D1 sweep
  - 2-monopole arm in D3 implemented as twin scenario invocation with offset hooks
  - Pontryagin estimator using 4-point cross-product sum (shared with `correlations.h::compute_topological_charge` if present, or local helper)
- Output: `engine/results/topological_observables_2026-04-27/{D1_wilson,D2_flux_tube,D3_monopole,D4_vacuum_instanton}/`
- Each subdirectory: `meta.json`, `per_snapshot.csv`, `volumetric/snapshot_{tick}.csv`
- Post-hoc: `docs/theory/10_eft_program/ANALYSIS_TOPOLOGICAL_OBSERVABLES.md`

GPU build: must run via WSL2 RTX 5090 (`engine/build_wsl/`) per CLAUDE.md GPU mandate. Estimated wall: ~2 hours (4 sub-experiments × ~5 parameter settings × 5 seeds × L=32 × 2200 ticks each).

---

## 7 · What this protocol does NOT claim

- No identification of any seeded structure with an SM particle (no "instanton ≈ tunneling rate" claim, no "monopole = magnetic monopole" claim, no Wilson loop = QCD confinement claim).
- No fit to SM observables.
- No update of `g_c`, `α`, or any parametric inserts.
- No claim of "FTD recovers continuum confinement" — D1's σ_eff is reported as a lattice quantity in lattice units, not Λ_QCD.

The output is engine-native phenomenology. Interpretation is deferred.

---

## 8 · Single-line summary

**Pre-registers a four-sub-experiment topology atlas (Wilson loop, flux tube, monopole, vacuum instanton) on the L=32 Langevin lattice, with shared per-snapshot CSV schema, per-sub-experiment outcome interpretation grid (W1–W3, T1–T3, M1–M4, I1–I3), no SM-comparison anti-target, and explicit acceptance for negative or null findings. Implements directly via existing s0-seed scenarios + ~400 LOC campaign harness; runs on WSL2 RTX 5090 GPU per CLAUDE.md mandate.**
