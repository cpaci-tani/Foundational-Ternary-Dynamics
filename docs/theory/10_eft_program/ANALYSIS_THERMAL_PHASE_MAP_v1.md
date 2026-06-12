# ANALYSIS — Thermal phase map of the FTD lattice, run of record (FTD-0275)

**Status:** `[MEASURED]` (pre-registered run of record). **Date:** 2026-06-12.
**Pre-registration:** [`PREREG_THERMAL_PHASE_MAP_v1.md`](preregistrations/PREREG_THERMAL_PHASE_MAP_v1.md),
git tag `preregister-thermal-phase-map-v1`, lock commit `74caabec`.
**Artifacts (SHA256-locked):** `engine/tests/campaign_thermal_ignition.cpp`
(`602ad532…`), `scripts/exploration/analyze_thermal_phase_map.py` (`6e238ff6…`).
**Runs of record (local, gitignored):** `engine/results/thermal_ignition/thermal_ignition_{q1_tup_scan,q2_ablation_ng,q2_control_gen,q3_spark}.csv`.
**Supersedes:** the FTD-0274 scout `ANALYSIS_THERMAL_IGNITION_v1.md` (`[MEASURED — scout]`)
on all three questions below.

---

## 0 · Verdicts

| Q | Question | Frozen verdict |
|---|---|---|
| **Q1** | T_up(L) scaling | **RISES-CONFIRMED** |
| **Q2** | is genesis the high-T safety valve? | **SAFETY-VALVE-FALSIFIED** |
| **Q3** | local spark in a near-critical bath? | **DETONATES** |

`FTD-0275 SUMMARY: Q1=RISES-CONFIRMED  Q2=SAFETY-VALVE-FALSIFIED  Q3=DETONATES`

**Nothing is promoted.** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`,
FTD-0110/0261/0269/0272/0273/0274 are unchanged. All three results land at
`[MEASURED]`; one of them (Q2) is an honest **correction** of the FTD-0274
scout's mechanism conjecture.

---

## 1 · Q1 — T_up(L) rises (the floor climbs with box size)

T_up(L, seed) = the first ramp temperature at which the manifestation fraction
m = N/L³ exceeds 0.5 (kinetic spinodal at the frozen protocol: dT = 0.002,
settle = 400, cumulative lattice). Per-seed first-crossings, 3 seeds per L:

| L | T_up (mean ± seed-scatter) | T_up / c² |
|---|---|---|
| 16 | 0.0133 ± 0.0025 | 0.040 |
| 24 | 0.0253 ± 0.0074 | 0.076 |
| 32 | 0.0393 ± 0.0123 | 0.118 |
| 48 | 0.0587 ± 0.0066 | 0.176 |
| 64 | (0.064, single seed — not in the fit; see §4) | 0.192 |

- **Monotone, span ≫ scatter:** mean T_up strictly increases across all four
  fitted L; span (L16→L48) = 0.0453, pooled cross-seed σ = 0.0080 — span is 5.7×
  the scatter. Verdict **RISES-CONFIRMED**.
- **Scaling form (descriptive only — NOT a derived law):** of the three frozen
  candidate forms, the **power law `T_up ≈ 0.000324 · L^{1.36}`** fits best
  (SSE_log = 0.016, vs 0.042 log-linear, 0.174 saturating). The floor rises
  **super-linearly** with box size — a small box ignites at a much lower
  temperature than a large one.
- **Mechanism reading (consistent with FTD-0272/0274, not a new claim):** finite-size
  nucleation. A small box is ~one correlation volume, so a thermal fluctuation
  tips the whole lattice over with little superheat; a larger box must pay a
  bigger nucleation barrier, so its (protocol-relative) spinodal sits higher.
  The scout's "tighter lattices ignite at lower temperature" is **quantitatively
  confirmed** and sharpened from 3 coarse-dT boxes to 4 fine-dT boxes with seed
  statistics.
- **Protocol caveat (frozen):** T_up here is a *kinetic spinodal at the stated
  ramp*, not the thermodynamic binodal. The finer dT (0.002 vs the scout's 0.01)
  gives more dwell time below any T, so these values sit below the scout's
  (L=24 here 0.025 vs scout ≈ 0.05). This is expected and was pre-declared; the
  *L-trend* is the claim, not the absolute number.

## 2 · Q2 — the safety valve is the thermostat, not genesis

The FTD-0274 scout found "no temperature ceiling found up to T_set = 6" and
conjectured `[CONJECTURE]` that the **manifestation rule is the operative safety
valve** absorbing arbitrary heat. The ablation discriminates this directly.

| arm | reached T | first UNSTABLE |
|---|---|---|
| ablation (genesis **OFF**) | 12.0 | never |
| control (genesis **ON**) | 12.0 | never |

- Both arms ramp to **T = 12** (T_kin ≈ 36 ≈ 108× c² — 2× beyond the scout's
  reach) with `total_energy` finite throughout. The genesis-disabled lattice is
  **just as stable** as the genesis-on one.
- **Verdict SAFETY-VALVE-FALSIFIED.** High-T stability does **not** require the
  manifestation rule. The Langevin Ornstein–Uhlenbeck update is mean-reverting
  (`v ← v(1−γ) + √(2γT)·ξ`): it is unconditionally stable on its own, and that —
  not genesis — is what keeps the lattice finite at arbitrary T. The scout's
  safety-valve `[CONJECTURE]` is **withdrawn as the mechanism**.
- **What survives:** the scout's *observation* that no ceiling was found is
  untouched — there is still no thermal runaway up to T = 12. Only the *mechanism
  attribution* ("genesis is the valve") is falsified. The reason there is no
  ceiling is more mundane than the scout proposed: a mean-reverting thermostat
  cannot diverge. (A genuinely undamped microcanonical drive could still probe a
  CFL ceiling; that is a different, un-run experiment.)
- Control note: the genesis-on arm condenses at its T_up (≈ 0.25 at this coarse
  dT = 0.25) and then runs as a hot condensate to T = 12 without destabilizing,
  consistent with the FTD-0272 self-sustaining condensate.

## 3 · Q3 — a large spark in a near-critical bath detonates

Per (f, A) cell at L = 24: equilibrate a fresh lattice in a Langevin bath at
T = f·T_up(24) (the run re-measured T_up(24) = 0.032 at fine dT inside the run),
inject A·K_GENESIS at the center, settle 600 ticks, classify DETONATION
(N > 0.25·L³) vs BOUNDED. A = 0 rows are the bath-only control. 3 seeds/cell.

| f (T_bath) | A=0 (control) | A=10 | A=30 | A=60 |
|---|---|---|---|---|
| 0.80 (0.0256) | 1/3 | 1/3 | 1/3 | 1/3 |
| 0.90 (0.0288) | 1/3 | 1/3 | 1/3 | **3/3** |
| 0.95 (0.0304) | 0/3 | 0/3 | 0/3 | 1/3 |
| 0.99 (0.0317) | 1/3 | 1/3 | 1/3 | 1/3 |

- **Verdict DETONATES**, fired by the **f = 0.90, A = 60 cell (3/3 detonation)**
  against a clean same-f control (0/3 pre-condensed at equilibration, majority
  BOUNDED at settle). A *large* local spark in a near-critical thermal bath tips
  the entire lattice into global condensation — the "spark in a flammable
  atmosphere" case the FTD-0274 scout left open. This is genuinely new physics
  relative to the scout: the *same* A = 60 injection in the COLD (langevin-off)
  vacuum grows only a bounded droplet (FTD-0274 §3); near criticality it
  detonates.
- **Honest effect-size caveat (load-bearing):** the signal is at the edge of the
  noise at 3 seeds. (i) There is a **non-trivial spontaneous background** — the
  metastable bath decays on its own at ≈ 1/3 over the 600-tick settle at most
  fractions, independent of A. (ii) The detonation rate is **non-monotone in f**
  (f = 0.90 A=60 → 3/3 but f = 0.95 A=60 → 1/3), because the bath crossover is
  itself stochastic and 3 seeds under-resolve it. The frozen rule fires on the
  one clean-control majority cell; the *binomial* significance of 3/3 against a
  1/3 background is p ≈ 0.037 — real but modest. A higher-seed (≥ 20) follow-up
  is the natural confirmation, recorded as queued, not run.
- **Reading:** detonation is amplitude × near-criticality, not injection alone.
  Below ~A = 60 the spark never beats the background at any f; the threshold
  amplitude is large and the window in f is narrow. Consistent with nucleation:
  the spark must exceed the critical-nucleus size, which shrinks as the bath
  approaches T_up.

## 4 · Instrument note (does not affect verdicts)

The run-of-record chain (Q2a → Q2b → Q3 → Q1) was **stopped by the operator**
during Q1's L = 64 leg (the slow CPU box: ≈ 25–35 s per temperature step). Two
consequences, both contained:

1. **CSV tail loss (Q1/L=64 only).** The v1 instrument flushed `stdout` per row
   but not the CSV FILE* buffer, so the un-flushed full-block tail was lost on
   kill: the L = 64 s = 0 crossing (console-recorded T_up = 0.064, consistent
   with the L^1.36 trend) and the s = 1/s = 2 ramps never reached disk. L = 64 is
   therefore **CENSORED in the frozen analyzer** and excluded from the §1 fit.
   Q1's verdict rests on the four complete boxes (L = 16–48), which is decisive
   (span 5.7× σ). Q2a/Q2b/Q3 binaries exited normally (`fclose` flushed), so
   their CSVs are complete and unaffected.
2. **Post-lock I/O fix.** `setvbuf(f, …, _IOLBF, …)` (line-buffered) was added
   *after* the lock so future runs survive interruption. This changes no physics
   and no verdict logic; the §3 pre-registration SHA256 binds the v1 source *as
   run*, and the fix is recorded here and in the source comment.

## 5 · Epistemic accounting

`[MEASURED]` run of record; **zero promotions**. Net new content over the scout:
(Q1) T_up(L) is a monotone super-linear rise ≈ L^1.36 with seed statistics — the
"tighter ignites lower" reading is now quantitative; (Q2) the high-T "no ceiling"
is the **thermostat's mean-reversion**, not a genesis safety valve — the scout's
mechanism `[CONJECTURE]` is **falsified** while its no-runaway *observation*
stands; (Q3) a large local spark in a near-critical bath **does** detonate
(modest effect size, ≥ 20-seed confirmation queued), closing the scout's open
"flammable atmosphere" follow-up. FTD-0013, MC-T4.3, FTD-0110/0261/0269/0272/0273
unchanged. Next free LEDGER id after this row: **FTD-0276**.
