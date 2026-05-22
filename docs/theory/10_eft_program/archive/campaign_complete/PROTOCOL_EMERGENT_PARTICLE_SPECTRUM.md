# PROTOCOL — Emergent Particle Spectrum from Generic Initial Conditions

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-27
**LEDGER row:** FTD-0102 (assigned ahead of measurement)
**Companion:** [`docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md`](PROTOCOL_OPERATOR_MIXING_MATRIX.md), [`docs/theory/10_eft_program/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md)
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (Campaign B of the engine-as-instrument portfolio)

This protocol is **pre-registered before measurement** per CLAUDE.md epistemic discipline rules.

---

## 1 · Why this protocol exists

The user's 2026-04-26 reorientation (recorded in the campaign portfolio plan): the FTD program has spent substantial effort on SM-quantity recovery (mixing angles, Yukawa structure, mass-formula exponent fitting). After 96 years of physics scaffolding being layered on potentially-wrong-turn foundations, the program should shift toward **using the lattice engine as a primary instrument**: run from generic initial conditions and measure what emerges, rather than refining post-hoc fits to phenomena the SM already curve-fitted.

The flagship campaign of that reorientation: **measure the emergent particle spectrum from generic initial conditions**. The SM cannot do this — it has its particle content as input. FTD's structural advantage is that it SHOULD produce a spectrum from generic initial conditions (the lattice has its own dynamics; particles are emergent bound states, not assumed). Whether it does, what the spectrum looks like, how it depends on initial-condition class — these are engine-native questions with no SM-comparison required.

This protocol pre-registers the design of that measurement. The deliverable is an empirical histogram of stable-bound-state energies measured across multiple initial-condition (IC) classes at L=32 on the Langevin+genesis lattice.

It addresses (partially or in full):
- The user's reorientation toward engine-as-instrument
- STATUS_EFT_CHECKLIST.md §6 line 86 ("Build systematic nonlinear b=2 flow campaigns from engine histories") — provides spectrum-side companion to the operator-mixing matrix campaign
- New ground: there is no prior FTD measurement of "what particles does the engine produce."

---

## 2 · Pre-registered initial-condition classes

Five IC classes, each run at multiple parameter settings:

| Class | Description | Parameters | Rationale |
|---|---|---|---|
| **IC-1: High-energy point injection** | Single voxel at lattice center receives flux of `inj_mult × K_GENESIS`; let cascade | `inj_mult ∈ {3, 5, 10, 50, 100}` | Probes high-density regime; pair production cascade |
| **IC-2: Random thermal initialization** | Full lattice random Langevin perturbation; long burn-in | Langevin `T ∈ {0.005, 0.05, 0.5}`, γ=0.02 | Probes thermal vacuum; spontaneous structure formation |
| **IC-3: Two-injection collision** | Inject two opposed-momentum flux blobs at distance ±L/4 from center | Center-mass energy `E_cm ∈ {2, 5, 10} × K_GENESIS` | Probes collision dynamics; pair / multi-particle production |
| **IC-4: Pair-creation seed** | Minimal energy state plus single perturbation; observe spontaneous pair creation | Perturbation amplitude `δ × K_GENESIS`, `δ ∈ {0.1, 0.5, 1.0}` | Probes vacuum decay rate; tests whether spontaneous pair creation is structural or rare |
| **IC-5: Pre-thermalized cosmic-baryogenesis-style** | Random ±1 state field; gauss-projected; evolved | Initial state-density ρ₀ ∈ {0.01, 0.1, 0.5} | Probes high-state-density regime; cosmic baryogenesis analog |

Each class × parameter setting × 5 seeds = independent ensemble. Total: ~75 ensembles.

Lattice: **L = 32** (consistent with FTD-0099 multilatitude data). Burn-in 200 ticks; sample stride 50 ticks; total run 5000 ticks.

---

## 3 · Pre-registered observables

Per snapshot (every 50 ticks):

1. **Particle census** (from `Diagnostics`): `manifested_count`, `positive_count`, `negative_count`, `spin_up_count`, `spin_down_count`, `color_count[4]`.
2. **Total energy + decomposition** (from `EnergyAudit`): `total_energy`, `field_energy`, `wave_energy`, `particle_ke`, `coulomb_pe`, `E_field_energy`, `B_field_energy`.
3. **Per-cluster bound-state tracking** (greenfield code in this campaign):
   - Cluster manifested voxels by spatial connectivity (≤2-voxel separation in Moore neighborhood)
   - Persistent ID across snapshots (cluster matched by spatial overlap with previous snapshot's clusters)
   - Per-cluster: voxel count, total `density()` mass, centroid position, velocity (centroid Δ across snapshots), age (ticks alive)
4. **Volumetric snapshots** (sparse): emit `csv::export_flux_field` at characteristic moments (post-burn-in, mid-cascade, end-of-run).
5. **Mass spectrum histogram**: per-class, histogram of stable-cluster total-energy values (cluster considered "stable" if alive ≥ 100 ticks).

---

## 4 · Pre-registered outcome interpretation grid

The following outcomes are pre-registered. Whichever matches will be the reported finding.

| Outcome | Description | Interpretation |
|---|---|---|
| **A — Discrete, IC-invariant spectrum** | Per-class histograms have distinct peaks; same peaks appear across IC classes within bootstrap stderr | FTD has a **structural mass spectrum** (positive, novel finding). Peak positions are FTD's emergent particle catalog. |
| **B — Continuous mass distribution** | Histograms have no peaks; mass distribution is broad and structureless | FTD produces excitations not particles at L=32; no discrete spectrum at this scale. Possible: lattice IS too small for particle-sized confinement; possible: spectrum is genuinely continuous. |
| **C — IC-dependent peaks only** | Histograms have peaks but peaks shift with IC class | Spectrum is initial-condition-conditioned; not a true particle spectrum. Suggests lattice produces local-equilibrium structures that depend on input rather than universal output. |
| **D — No stable clusters formed** | All clusters live <100 ticks; no stable bound states | Either L=32 is too small for confinement, OR spontaneous decay is fundamental at this Langevin coupling. Recommend L=64 follow-up + lower-T runs. |

ALL outcomes are publishable. There's no "failure mode."

**Pre-registered acceptance threshold for each outcome**:
- Outcome A requires ≥3 IC classes show peaks at the same energies within ±10% relative.
- Outcome B requires <30% of IC classes produce any peak whose width is < 30% of the histogram support.
- Outcome C requires ≥3 IC classes produce peaks, but peak positions vary >30% across classes.
- Outcome D requires ≥80% of clusters die within 100 ticks across all IC classes.

If multiple outcomes are partially supported, report the ranking with stderr.

---

## 5 · What we DO NOT measure (anti-targets per user reorientation)

- Comparison to SM particle masses (mₑ, m_p, mₐ, etc.)
- Identification of clusters as "electrons," "quarks," "hadrons," etc.
- Mixing-angle recovery (sin²θ_W, PMNS, CKM)
- Yukawa-structure recovery
- Mass-ratio recovery (m_p/m_e, m_τ/m_μ, etc.)

The output is "a histogram of bound-state energies emitted by the engine." The interpretation is "what does this lattice produce." Not "does this match the SM." Not "is FTD right."

---

## 6 · Output artifacts

Result directory: `engine/results/emergent_spectrum_2026-04-27/`

```
{ic-class-tag}_{param-tag}/
├── meta.json                       # campaign metadata + headline statistics
├── per_snapshot_census.csv         # one row per snapshot: tick, manifested, +1, -1, spin counts, color counts, total_energy
├── cluster_history.csv             # one row per (cluster_id, snapshot): cluster_id, tick, voxel_count, total_density, centroid_x, centroid_y, centroid_z, velocity_x/y/z, age, stable_flag
├── stable_clusters_terminal.csv    # one row per stable cluster at end-of-run: cluster_id, terminal_voxel_count, terminal_density, terminal_age, total_energy
├── volumetric/
│   ├── snapshot_t0500.csv          # via csv::export_flux_field
│   └── snapshot_t5000.csv
└── mass_histogram.csv              # binned histogram of stable-cluster total-energy
```

Plus a top-level `engine/results/emergent_spectrum_2026-04-27/PORTFOLIO_SUMMARY.md` cross-class report.

---

## 7 · Acceptance gates

| Gate | Threshold | If passed | If failed |
|---|---|---|---|
| Q conservation per IC ensemble | `|Q_final − Q_initial| / N_voxels < 1e-3` | continue | abort with diagnostic |
| Gauss residual gate | `max\|D Φ − ρ\| < 1.0` per snapshot (loose tolerance) | continue snapshot | drop snapshot from histogram |
| At least 3 IC ensembles complete | minimum 3/5 classes produce stable-cluster data | report findings as "partial coverage" | abort with diagnostic |
| Histogram non-degeneracy | histogram has ≥10 distinct bins populated | proceed to peak analysis | report Outcome B (continuous) |
| Pattern-match coherence | At least one IC class produces clusters with consistent age distribution | normal analysis | report Outcome D (no stable clusters) |

---

## 8 · Cross-references

- Plan: `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` Campaign B
- Companion infrastructure: `engine/include/ftd/csv_export.h::export_flux_field`, `engine/include/ftd/render_bridge.h` (`Diagnostics`, `EnergyAudit`)
- Existing scenarios that the IC classes draw from: `engine/src/scenarios/{flux,light,quantum,s0_seed}.cpp`
- LEDGER row: FTD-0102 (assigned)

---

## 9 · Open questions this campaign does NOT resolve

- Whether the spectrum scales with L (this is L=32 only; multilatitude follow-up is a separate campaign)
- Whether the spectrum has internal symmetries (gauge, color) — would require operator-basis follow-up
- Whether clusters are "particles" in the SM sense — explicitly out of scope
- The exact form of the emergent spectrum's generating action — requires S_eff fitting (separate)

---

**End of pre-registration.** No measurement code lands until this protocol is reviewable.
