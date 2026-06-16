# ANALYSIS (scout) — Min/max temperature + ignition map of the FTD lattice (FTD-0274)

> **SUPERSEDED (2026-06-12) by the FTD-0275 run of record**
> ([`ANALYSIS_THERMAL_PHASE_MAP_v1.md`](ANALYSIS_THERMAL_PHASE_MAP_v1.md),
> pre-registered, tag `preregister-thermal-phase-map-v1`). Three scout readings
> are revised: **(Q1)** "T_up rises with L" is **confirmed and quantified**
> (≈ L^1.36 across L = 16–48 with seed statistics); **(Q2)** the "no ceiling" /
> safety-valve `[CONJECTURE]` of §2 is **FALSIFIED as a mechanism** — a
> genesis-disabled ablation is equally stable to T = 12, so the Langevin OU
> thermostat's mean-reversion (not the manifestation rule) is what bounds the
> energy (the *no-runaway observation* still stands); **(Q3)** the scout's
> "local injection never detonates" (§3, tested only in cold vacuum) is
> **superseded** — a large spark (A = 60) in a *near-critical* bath (f = 0.90)
> detonates. Read the FTD-0275 analysis for the canonical statements; this scout
> is retained for provenance.

**Status:** `[MEASURED — scout; SUPERSEDED by FTD-0275]` (bracketing run, not a
pre-registered run of record). **Date:** 2026-06-11. **Artifacts:** `engine/tests/campaign_thermal_ignition.cpp`,
results `engine/results/thermal_ignition/*.csv` (local). Follows the FTD-0273 mass/energy
work and the thermodynamic reading of FTD-0272's first-order genesis transition.

## 0. Question

Establish what "temperature" means on the discrete lattice and whether it has a minimum
and a maximum. **Temperature `[DEFINITION]`** = the equipartition kinetic temperature of
the wave field: `⟨½|wave_vel|²⟩ = (3/2)·T` per voxel (k_B≡1), set by the Langevin bath
`langevin_T` (validated by `test_langevin_equipartition.cpp`). Below the condensation point
the measured `T_kin` tracks `T_set` (equipartition holds).

## 1. Minimum — condensation point T_up (sharp, L-dependent)

Heating the void by ramping `langevin_T` (no injection), the manifestation fraction
`m = N/L³` jumps **first-order** at a condensation point `T_up`:

| L | m at T=0.04 | m at T=0.06 | m at T=0.07 | T_up |
|---|---|---|---|---|
| 24 | 0.035 | 0.79 | — | ≈0.055 |
| 32 | 0.000 | 0.002 | 0.014 | > 0.07 |
| 48 | 0.000 | 0.002 | 0.002 | > 0.07 |

- **T_up ≈ 0.05–0.06 (lattice units) at L=24** — matches FTD-0272. Below it the void is a
  *metastable thermal vacuum* (flux fluctuates, never crosses K_GENESIS); at it the lattice
  condenses. This is the meaningful **minimum-temperature landmark** (absolute zero T=0 is
  just the static limit — void or frozen condensate).
- **T_up RISES with L** (L=24 ignites by 0.06; L=32, L=48 still vacuum at 0.07). Finite-size
  nucleation: a small box is ~one correlation volume, so fluctuations tip the whole thing
  over with less superheat; a big box must pay a larger nucleation barrier. **Tighter
  (smaller) lattices ignite at LOWER temperature** — and the hysteresis loop widens with L
  (consistent with FTD-0272). `[MEASURED]`.
- **Self-sustaining condensate**: cooling from the condensed phase stays pinned at m=1 down
  to T=0 (`T_down`→0) — the maximal hysteresis of FTD-0272 (the gauss/coupling self-field
  latches every voxel above K_GENESIS; no damping to relax it). `[MEASURED]`.

## 2. Maximum — none found in this scout (the manifestation safety valve)

Prior `[HYPOTHESIS]`: discreteness imposes a CFL/causality ceiling `T_max ~ c²` (where
thermal velocity `√(3T)` reaches the max signal speed `c=1/√3`). **FALSIFIED by
measurement.** Heating to `T_set = 6` drives `T_kin = 9.2` — that is **27× c²** — with the
lattice **still stable** (m=1 condensed, no CFL blow-up, `T_kin ∝ T_set` linearly, slope
≈1.53, no saturation):

| T_set | 0.4 | 0.8 | 2.0 | 4.0 | 6.0 |
|---|---|---|---|---|---|
| T_kin | 0.53 | 1.21 | 3.07 | 6.14 | 9.19 |
| stable | ✓ | ✓ | ✓ | ✓ | ✓ |

**Why no ceiling `[CONJECTURE]`:** (i) the Langevin OU process is mean-reverting
(unconditionally stable), and (ii) the **manifestation rule acts as a safety valve** —
when `|J|` exceeds K_GENESIS the voxel condenses, capping the flux and absorbing the
energy into the bounded state field instead of a runaway wave (this mechanism has not been
ablated; the safety-valve interpretation is inferred from the finite-T scan, not confirmed
by a controlled genesis-disabled comparison). Discreteness *alone* would give a CFL
ceiling; **FTD's genesis nonlinearity appears to remove it** `[CONJECTURE]`. The discrete
lattice has a sharp temperature *floor* (condensation) but **no ceiling was found up to
T_set=6 (T_kin ≈ 27× c²) in this scout**. The "explosion" is the *condensation itself*
(first-order, global, autocatalytic), not a high-T instability. `[MEASURED — scout]`.

## 3. Ignition — local injection never detonates; the explosion is global/thermal

Deterministic (langevin OFF) injection-amplitude sweep at L=24: every amplitude A∈{2..60}
settles to a **BOUNDED** cluster — none floods (`A* = none`). A *local* energy dump, however
large, grows a bounded droplet (a "flame"), not a detonation. The autocatalytic
runaway (whole-lattice condensation) is a **global thermal** phenomenon (crossing T_up),
not something a local spark triggers in the cold (langevin-off) vacuum. `[MEASURED]`.
(Open follow-up: a local spark in a *near-critical* bath — injection with `langevin_T`
just below T_up — may trigger global detonation; the "spark in a flammable atmosphere" case.)

## 4. The thermodynamic axis of the FTD lattice

| T (lattice units) | phase | meaning |
|---|---|---|
| 0 | static | absolute zero — void or frozen condensate |
| 0 → T_up | thermal vacuum | flux fluctuates, never manifests (metastable) |
| **T_up ≈ 0.05** (rises with L) | **condensation** | first-order void→matter; the "explosion" |
| T_up → ∞ | hot condensate | m=1; T_kin ∝ T_set, **no ceiling found** (up to T_set=6, T_kin ≈ 27× c²; mechanism `[CONJECTURE]`) |

## 5. Epistemic accounting

`[MEASURED]` scout. **Nothing promoted.** The new content: the temperature *definition*
(equipartition kinetic T), the L-dependence of T_up (tighter ignites lower), and the
**falsification of the `T_max ~ c²` hypothesis** — the manifestation nonlinearity gives the
lattice a floor but no ceiling. FTD-0013 `[SMC]`, MC-T4.3, FTD-0110/0269/0272 unchanged. A
pre-registered run of record (finer L-scan of T_up(L); the near-critical spark test) is the
natural next step. Web demo: Scale-0 `s0-seed-thermal-ignition` + a docked **Thermo side panel**
(`thermo-panel.js`): temperature slider across T_up, live telemetries (T_kin, m, phase, energy
ledger), and a flux |J| heat-map slice. Next free LEDGER id: **FTD-0274**.
