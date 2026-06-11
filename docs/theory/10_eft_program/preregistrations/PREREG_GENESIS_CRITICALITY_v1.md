# PREREG — Order of the FTD Genesis Transition (RG-spectrum probe)

**Status:** `[PRE-REGISTRATION — design locked before the run of record]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0272 (reserved)
**Git tag:** `preregister-genesis-criticality-v1` (applied at the lock commit)

## 0 · Purpose & the narrow target

We are asking whether the FTD lattice **derives a mass/excitation spectrum via
renormalization**. The free (flux/wave) sector is Gaussian — its RG flow is the
dispersion flowing to the Lorentz-invariant continuum (anisotropy dying as k⁴,
PL-5, already measured), with the mass scale *imposed* (the FTD-0271 clock). The
**only** place a non-trivial, RG-*derived* spectrum can come from is the
**genesis nonlinearity**. So the narrow, decisive question is:

> **Is the genesis/manifestation transition a 2nd-order CRITICAL point** (a
> scaling fixed point ⇒ genesis is a *relevant* operator ⇒ the cluster-mass
> ladder has genuine RG content) **or a 1st-order/trivial transition** (no
> diverging correlation length, no fixed point ⇒ no RG-derived spectrum; the
> `N∝A²` cluster law is energy-budget / pattern formation)?

Genesis is an **absorbing-state transition**: void (`s=0`) = quiescent phase,
manifestation (`s=±1`) = activity. We drive it **purely by Langevin temperature
T** (no injection — the FTD-0107 ic2 regime) and measure the order parameter
`m = N_manifested / L³` and its full fluctuation distribution under finite-size
scaling (FSS). The **order of the transition** is the verdict.

**Honesty ceiling.** This does not derive any spectrum by itself. A CRITICAL
verdict *opens* the door (genesis is RG-relevant; the cluster spectrum *could*
be a scaling phenomenon) and mandates a follow-up exponent extraction. A
FIRST-ORDER verdict *closes* it: the cluster ladder is not RG-flow-derived. Both
are real boundary results. ℏ-scale and the atomic-spectrum gaps (FTD-0270) are
untouched either way.

## 1 · Frozen artifacts

| Role | Path | SHA256 (16) |
|---|---|---|
| Engine campaign | `engine/tests/campaign_genesis_criticality.cpp` | `13b006c116fc6030` |
| FSS analyzer | `scripts/exploration/analyze_genesis_criticality.py` | `18cf7e622471d652` |

## 2 · Run of record (frozen)

Canonical stack, **CPU** (Langevin = the CPU OU thermostat), **no injection**:
`wave_propagation + gauss_projection + genesis + coupling + langevin`, `gamma=0.02`.

- **Lattice sizes:** `L ∈ {16, 24, 32}` (≥3 for the χ_max scaling fit).
- **Temperature window (from the pre-lock scout, T_c≈0.12):**
  `T ∈ {0.07, 0.08, 0.09, 0.10, 0.11, 0.12, 0.13, 0.14, 0.16, 0.18}`.
- **Seeds:** 12 per (L,T). **Equilibration:** 3000 ticks. **Sampling:** 2000 ticks (per-tick `m` logged).

Command per L:
`campaign_genesis_criticality --L=<L> --Ts=0.07,0.08,0.09,0.10,0.11,0.12,0.13,0.14,0.16,0.18 --seeds=12 --equil=3000 --sample=2000 --cpu --tag=ror --output-dir=engine/results/genesis_criticality_ror`

A reduced pre-lock SCOUT (`L=24`, 8 coarse T, 2 seeds, equil=400) was run only
to bracket the window; it is NOT the run of record. (Scout result: m≈0 for
T≤0.05, χ-peak at T=0.12, U4_min=−0.54 — a first-order hint, to be confirmed.)

## 3 · Three theory-fixed discriminators (frozen thresholds)

- **D1 — P(m) MODALITY** at the susceptibility-peak `T_c(L)` for the largest L:
  **BIMODAL** (two separated peaks, inter-peak valley `< 0.6·` smaller peak,
  peak separation `> 0.08`) → FIRST-ORDER; **UNIMODAL** → CRITICAL.
- **D2 — BINDER cumulant** `U4 = 1 − ⟨m⁴⟩/(3⟨m²⟩²)`: a deep minimum
  `U4_min(L_max) < 0.40` that **deepens with L** → FIRST-ORDER (phase
  coexistence); `U4_min(L_max) ≥ 0.50` with curves crossing → CRITICAL.
- **D3 — SUSCEPTIBILITY scaling** `χ_max(L) ~ L^a` (`χ = N·Var(m)`): `a ≥ 2.6`
  (volume-like, ~D=3) → FIRST-ORDER; `a ≤ 2.2` (anomalous, γ/ν<D) → CRITICAL.

## 4 · Verdict map (frozen)

Majority of {D1, D2, D3}:
- **GENESIS-FIRST-ORDER** — ≥2 say FIRST-ORDER → `[MEASURED — BOUNDARY]`: genesis
  is RG-irrelevant as a spectrum generator; the lattice does **not** derive a
  mass spectrum via RG in the genesis sector.
- **GENESIS-CRITICAL** — ≥2 say CRITICAL → `[MEASURED]`: genesis is a relevant
  operator with a scaling fixed point; mandates a follow-up exponent extraction
  (ν, β, γ; universality-class test, e.g. directed percolation).
- **INCONCLUSIVE** — otherwise → refine statistics/window and re-run; no claim.

## 5 · Priors (disclosed)

FIRST-ORDER ~55% (the EWSB condensate was "sharp first-order"; scout U4_min=−0.54
and seed bistability 0.20↔0.77 hint coexistence); CRITICAL ~35% (absorbing-state
transitions are *usually* continuous — the genuine hope); INCONCLUSIVE ~10%.
Even a CRITICAL verdict yields the *cluster* ladder, not atomic levels, and still
inherits the ℏ-scale gap.

## 6 · Scope & non-promotion

Pre-registers the **order-of-transition** measurement only. Exponent extraction /
universality class is a downstream phase contingent on a CRITICAL verdict.
Nothing promoted: FTD-0013 `[SMC]`, MC-T4.3, FTD-0050 (`[CLOSED-NEGATIVE]`,
not retread — this drives by T, not by blocking the BCC-orthogonal stencil),
FTD-0270/0271 all unchanged.
