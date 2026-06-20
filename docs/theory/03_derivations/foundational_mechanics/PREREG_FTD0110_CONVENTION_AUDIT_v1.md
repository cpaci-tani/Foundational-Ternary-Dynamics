# PREREG — FTD-0110 Convention Audit: is the N(A) calibration gauge or physical?

**Status:** `[PRE-REGISTRATION — design locked before any run-of-record]`
**Date:** 2026-06-19
**LEDGER row:** FTD-0110 (nonlinear bridge) / new id at adjudication (provisional **FTD-0307**; confirm against corpus max-id before allocating — the LEDGER main table trails the true max).
**Git tag:** `preregister-ftd0110-convention-audit-v1` (applied at the lock commit)
**Branch:** `engine/ftd0110-convention-audit` (based on `fix/cuda13-atomicadd-poisson`; CPU golden `0xb604d81a3d79366e` green).

## 0 · Purpose

Attack the **never-attempted exit (ii)** of the FTD-0110 nonlinear bridge
(`ANALYSIS_FTD0110_NA_LAW.md` §3, `DERIV_KINETIC_DRAIN_FROM_QUADRATURE.md` §4):
*is the engine-tuning calibration of the N(A) cluster-mass law pure CONVENTION —
only the dimensionless shape is physical and the rest is an affine rescaling of
(A, N) — or PHYSICAL (the knob changes the dimensionless shape)?*

FTD-0269 found the calibration "load-bearing" from a **knee shift** under the drain.
This audit corrects the discriminator: a broken power law's **exponents are invariant
under any affine rescaling** `A→A/λ, N→N/μ` (only the knee location and the prefactor
move), so a knee shift does **not** distinguish convention from physics — **exponent
movement** does. The audit measures the per-drain exponents that FTD-0276 Leg A never
fit (it fit only the sub-knee prefactor `k_eff`).

**γ is taken as PHYSICAL — `[established]`, not re-measured** (owner decision 2026-06-19).
A quick-check (2026-06-19) on the existing FTD-0276 Leg B map showed the exponents move
monotonically with γ (`p_lo` by 65%, `p_hi` by 39% across γ∈{0…0.1}, far beyond the
~5% reseeding-noise floor); exponent-invariance ⇒ no rescaling absorbs γ ⇒ γ is a
genuine shape-setting rate. This pre-reg therefore registers **only the drain arm**.

## 1 · Frozen artifacts (SHA256)

| Role | Path | SHA256 |
|---|---|---|
| Engine instrument | `engine/tests/campaign_drain_scan.cpp` | `acd03bbd72a428b1d0ef2ff7f934881057e7db9467fa02742dfb9a003f1d92fd` |
| Adjudicator | `scripts/exploration/analyze_drain_convention.py` | `c9bbe1a6fcb8cbfe9d608262561adde73a17c75728aa24d3a98b47a2dbb8f3ec` |

## 2 · Measurement platform (frozen)

- **Stack:** canonical ic1 — `wave_propagation + gauss_projection + genesis + coupling
  + langevin(γ=0.02, T=0.005)`; `kinetic_drain` set per-drain at runtime (FTD-0276 knob);
  L=32; x-axial point injection `A·K_GENESIS` at the center voxel; CPU (`--cpu`,
  SOR=150) — genesis counters are CPU-only.
- **Sweep:** `--drains=0.125,0.25,0.375,0.5,0.625,0.75` (the FTD-0276 Leg A grid) ×
  `--As=10,12,14,16,20,25,30,40,50,70,90` (**extended past Leg A's 40** so the
  super-knee exponent `p_hi` is fit, not extrapolated) × `--seeds=8` × `--settle=300`.
- **Engine:** `engine/ftd0110-convention-audit` HEAD; the drain campaign is
  golden-neutral (read-only; the default drain 0.5 is bit-identical to the constexpr
  path). Determinism: the genesis/flux FIELD is bit-reproducible at fixed seed
  (FTD-0269 §2); seed averaging (8) supplies the bootstrap noise estimate.

## 3 · What is measured (the discriminator)

For each drain, the adjudicator fits a segmented log-log broken power law to N̄(A),
returning `(knee, p_lo, p_hi)` with a seed-bootstrap CI on each exponent. It then
computes:
- **exponent spread** across drains: `spread(p) = (max_d p − min_d p) / mean_d p`, for
  `p_lo` and `p_hi` separately;
- **collapse residual**: rescale each drain curve `A→A/knee_d`, `N→N/N(knee_d)` and
  measure the cross-drain scatter (median coefficient of variation) on the shared
  log-A′ grid — a single per-drain affine rescaling collapsing all curves ⇒ convention.

## 4 · Gates and outcome map (mechanical — `analyze_drain_convention.py`)

STRICT band (owner decision 2026-06-19 — "exponents constant within reseeding noise"):

- **CONVENTION (drain)** — ALL must hold: `spread(p_lo) < 0.10` **and**
  `spread(p_hi) < 0.10` **and** collapse median-CV `< 0.05`. Reading: the drain is an
  affine (A,N) rescaling; only the dimensionless N(A) shape is substrate-physical.
- **PHYSICAL (drain)**: `spread(p_lo) ≥ 0.10` **or** `spread(p_hi) ≥ 0.10` **or**
  collapse median-CV `≥ 0.05`. Reading: the drain changes the dimensionless shape;
  exit (ii) fails for the drain too.
- **FIT-FAIL guard**: if any per-drain broken-power fit is ill-conditioned (a segment
  with <2 points or a non-finite slope), no verdict is emitted; the only permitted
  remedy is widening the A-grid (a documented platform fix), not a criteria edit.

**Combined landing (with γ = PHYSICAL [established]):**
- CONVENTION (drain) + γ PHYSICAL ⇒ **the SPLIT boundary** (prior-favored): *the N(A)
  prefactor is a normalization convention, γ is FTD's one substrate-physical finite-time
  rate, and only the dimensionless shape is physical.*
- PHYSICAL (drain) + γ PHYSICAL ⇒ both knobs are physical shape-setters; the calibration
  is irreducibly engine-emergent (the FTD-0269 BOUNDARY hardened, no convention escape).

Either landing is a Number-One-Goal result (a mapped boundary, both directions stated
in advance).

## 5 · Frozen data rules

- F-1: the first valid run (no FIT-FAIL) is the run of record; no seed/window re-rolls.
- F-2: no edit to §4 gates or the §1 artifacts after the tag; demotion of any tag is
  free, the CONVENTION reading requires the §4 bands.
- F-3: the A-grid may be widened ONCE if FIT-FAIL fires, with the change committed and
  noted; the gates are unchanged.

## 6 · Stated priors and scope

Priors: **CONVENTION (drain) 55% · PHYSICAL (drain) 40% · FIT-FAIL/other 5%.** The
sub-knee `k_eff ∝ drain^−0.92` (Leg A) is roughly prefactor-like, weakly favouring
convention, but the imperfect collapse (CV 21% on the sub-knee prefactor alone) and the
FTD-0269 knee shift leave it genuinely open.

Under every outcome: **no promotion** of FTD-0013 `[SMC]`, MC-T4.3
`[FOUNDATIONAL OBSTRUCTION]`, or the SM cluster-mass identification `[SMC]`; the linear
k=¼ theorem (O_h representation theory) is untouched mathematics. This audits the
**convention status of the drain calibration**, not the SM mass identification.
