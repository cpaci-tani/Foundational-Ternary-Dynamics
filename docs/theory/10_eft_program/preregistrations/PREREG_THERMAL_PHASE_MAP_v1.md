# PRE-REGISTRATION — Thermal phase map of the FTD lattice, run of record (FTD-0275)

**Status:** `[PRE-REGISTRATION]` — design lock; run of record follows the hash-lock.
**Date:** 2026-06-12
**LEDGER id (reserved):** FTD-0275
**Git tag (to be applied at lock):** `preregister-thermal-phase-map-v1`
**Promotes:** the FTD-0274 scout (`ANALYSIS_THERMAL_IGNITION_v1.md`, `[MEASURED — scout]`) to a pre-registered run of record.
**Prior context:** FTD-0272 (genesis transition first-order), FTD-0274 (floor T_up ≈ 0.05 at L=24; no ceiling found to T_set=6; safety-valve mechanism `[CONJECTURE]`, not ablated).

---

## §1 · Purpose and narrow targets

Three frozen questions, each with its own verdict map. No other claims are in scope.

- **Q1 — T_up(L) scaling.** The scout observed T_up rising with L across 3 box sizes at coarse dT. Quantify T_up(L) across 5 box sizes at fine dT with seed scatter; test the scout's "tighter lattices ignite at lower temperature" reading and characterize the scaling form descriptively.
- **Q2 — safety-valve ablation.** The scout's "no temperature ceiling" explanation conjectured that the **manifestation rule is the operative safety valve** absorbing arbitrary heat. Discriminate it: run the heating ramp with genesis DISABLED. If high-T stability requires genesis, the ablated lattice should destabilize; if the Langevin OU mean-reversion alone suffices, the conjecture's mechanism attribution is falsified.
- **Q3 — near-critical spark.** The scout found a local injection never detonates in the COLD (langevin-off) vacuum. Test the open follow-up: a local spark in a *near-critical thermal bath* (T just below T_up) — the "spark in a flammable atmosphere" case.

## §2 · Frozen definitions

- **Temperature** `[DEFINITION]` (unchanged from FTD-0274): equipartition kinetic temperature of the wave field, `⟨½|wave_vel|²⟩ = (3/2)·T` per voxel, k_B ≡ 1, set by `langevin_T` (γ = 0.02).
- **T_up(L, seed)** `[DEFINITION — protocol-relative]`: the first ramp temperature at which the manifestation fraction m = N/L³ exceeds 0.5, under the frozen ramp protocol (cumulative lattice, `settle` ticks per step, step `dT`). This is a **kinetic spinodal at the stated observation protocol**, not the thermodynamic binodal; values are comparable only within the frozen protocol. Finer dT gives the lattice more total dwell time below any T than the scout's coarser ramp, so Q1 values may sit below the scout's — this is expected and is not a discrepancy.
- **Stability**: `total_energy` finite and < 10⁸ (probe criterion frozen in the instrument).
- **DETONATION** (Q3): settled manifested count N > `flood_frac`·L³ with flood_frac = 0.25.

## §3 · Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `engine/tests/campaign_thermal_ignition.cpp` | `602ad532cd7084976f688de744f3aaad052cf3c6fb9d171dfbb039b3ded20e58` |
| `scripts/exploration/analyze_thermal_phase_map.py` | `6e238ff65d55adb462c8a584406927950fa3f04a46ee887719fb4a1ad0808095` |

The analyzer encodes the §5 verdict logic; it was written and frozen before any run of record. The campaign instrument is golden-neutral (read-only campaign; golden gate `0x56fa28acb5b9fe88` verified green on the build used).

## §4 · Runs of record (frozen invocations)

All runs CPU (`force_cpu`), SOR 150, seed base `0x73E12000`, executed only AFTER the hash-lock tag.

```
# Q1 — T_up(L) scan (heat-only; per-seed fresh lattice)
campaign_thermal_ignition --mode=thermal --heat-only --Ls=16,24,32,48,64 \
    --Tmax=0.30 --dT=0.002 --settle=400 --seeds=3 --tag=q1_tup_scan

# Q2a — ablation arm (genesis OFF)
campaign_thermal_ignition --mode=thermal --no-genesis --Ls=24 \
    --Tmax=12 --dT=0.25 --settle=400 --seeds=2 --tag=q2_ablation_ng

# Q2b — control arm (genesis ON, same ramp)
campaign_thermal_ignition --mode=thermal --Ls=24 \
    --Tmax=12 --dT=0.25 --settle=400 --seeds=2 --tag=q2_control_gen

# Q3 — near-critical spark grid
campaign_thermal_ignition --mode=spark --Ls=24 --Tmax=0.12 --dT=0.002 \
    --settle=600 --equil=400 --seeds=3 \
    --spark-fracs=0.8,0.9,0.95,0.99 --spark-As=0,10,30,60 --tag=q3_spark

# Verdict
python scripts/exploration/analyze_thermal_phase_map.py
```

Note (Q2b): the genesis-on control will condense at ~T_up early in the ramp and then continue as a hot condensate — the control's question is only whether it remains STABLE to Tmax = 12 (T_kin ≈ 36 ≈ 108× c², 2× beyond the scout's reach).

## §5 · Frozen verdict logic (three-outcome per question)

**Q1** (computed on per-(L, seed) first-crossings):
- **RISES-CONFIRMED** — mean T_up strictly non-decreasing across uncensored L AND span T_up(L_max) − T_up(L_min) > pooled cross-seed σ.
- **NON-MONOTONE** — any decrease beyond pooled σ (falsifies the scout reading).
- **FLAT** — differences within scatter.
- **CENSORED** — ≥ 2 of the 5 L values right-censored at Tmax = 0.30 (protocol insufficient; no scaling claim).
- Candidate-form fits (power / log / saturating, SSE on log T) are **descriptive only** — reported, never promoted to a derived scaling law.

**Q2:**
- **SAFETY-VALVE-CONFIRMED** — ablation arm goes UNSTABLE at some T ≤ 12 while the genesis-on control stays stable to 12. (The FTD-0274 safety-valve `[CONJECTURE]` becomes `[MEASURED]` as the operative stabilization mechanism at this protocol.)
- **SAFETY-VALVE-FALSIFIED** — ablation arm stable through Tmax = 12. (Stability is thermostat-native OU mean-reversion; the conjecture's mechanism attribution dies; the "no ceiling found" *observation* of FTD-0274 is unaffected.)
- **OTHER** — any other pattern (e.g. both arms unstable), reported as-is.

**Q3** (per (f, A) cell, majority over 3 seeds; A = 0 rows are the bath-only control arm):
- **DETONATES** — some (f, A>0) cell has majority DETONATION while the same-f A=0 control is majority PRE_VACUUM at equilibration and BOUNDED at settle.
- **BOUNDED-ALWAYS** — no A>0 cell reaches majority DETONATION, all controls clean.
- **INVALID-CONTROL** — any f whose A=0 control itself condenses within the window (bath supercritical at this protocol; that f is unusable; verdict INVALID-CONTROL only if no clean-control detonation exists elsewhere).

## §6 · Pre-declared outcomes

- **OUTCOME A (map established):** Q1 ∈ {RISES-CONFIRMED, NON-MONOTONE, FLAT} + Q2 ∈ {CONFIRMED, FALSIFIED} + Q3 ∈ {DETONATES, BOUNDED-ALWAYS}. All verdict combinations are informative; none promotes anything beyond `[MEASURED]`.
- **OUTCOME B (partial):** exactly one question lands CENSORED / OTHER / INVALID-CONTROL — the other two stand; the failed leg is reported honestly and re-registered (v2) only with changed protocol.
- **OUTCOME C (indeterminate):** ≥ 2 questions fail their protocol — the run of record is recorded as protocol-insufficient; no verdict claimed.

**Prior-favoured outcomes (declared):** Q1 RISES-CONFIRMED (scout evidence); Q2 genuinely uncertain (this is the discriminating test — both verdicts plausible); Q3 genuinely uncertain (the scout's bounded-droplet result argues BOUNDED-ALWAYS; nucleation theory argues DETONATES near the spinodal).

## §7 · Pre-declared exclusions (banned moves)

1. No post-hoc re-thresholding: m > 0.5, flood_frac = 0.25, stability < 10⁸ are frozen.
2. No re-running with adjusted settle/dT/Tmax to move a verdict; protocol changes require a v2 pre-registration.
3. No near-miss reinterpretation (e.g. "m reached 0.4, almost condensed").
4. T_up is protocol-relative (kinetic spinodal); no claim about the true thermodynamic transition temperature, and no extrapolated "T_up(∞)" claim beyond the descriptive fits.
5. Q2 CONFIRMED does **not** promote the FTD-0274 "no ceiling" claim beyond its scout scope (T_set ≤ 6 there; ≤ 12 here); it only settles the *mechanism attribution*.
6. No numerical coincidence-hunting on T_up values against framework constants.
7. Zero promotions: FTD-0013 `[SMC]`, MC-T4.3, FTD-0110/0269/0272/0273/0274 statuses unchanged regardless of outcome.

## §8 · Method specification

- Toggles: `wave_propagation + gauss_projection + genesis` (genesis OFF in Q2a only) + `langevin` (γ = 0.02), `dual_substrate = false`, all else disabled. CPU forced; SOR iterations 150.
- Q1 per-seed FRESH lattice (seed = base + s·2654435761); heat-only ramp stops at first crossing.
- Q3 stage 1 re-measures T_up(24) with the same ramp protocol (genesis ON, dT = 0.002, settle = 600) to set the bath fractions self-consistently inside the run; stage 2 equilibrates a FRESH lattice per (f, A, seed) for 400 ticks before injection at the center voxel with `A·K_GENESIS` along x̂.
- Outputs: `engine/results/thermal_ignition/thermal_ignition_{q1_tup_scan,q2_ablation_ng,q2_control_gen,q3_spark}.csv` (run-of-record CSVs are local/gitignored; the analysis doc records the verdict tables).

## §9 · Hash-lock declaration

This document, the campaign instrument, and the analyzer are committed together; the commit is tagged `preregister-thermal-phase-map-v1` BEFORE any §4 run executes. The §3 SHA256 hashes bind the instrument and analyzer versions. Any post-lock edit to §§2–8 or to either artifact invalidates the lock and requires a v2.
