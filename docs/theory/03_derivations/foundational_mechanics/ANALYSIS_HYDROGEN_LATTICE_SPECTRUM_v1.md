# ANALYSIS — Hydrogen-like spectrum on the FTD lattice: HYDROGEN-CONFIRMED (FTD-0278 Leg 1)

**Status:** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]` (pre-registered run of record).
**Date:** 2026-06-12.
**Pre-registration:** [`PREREG_HYDROGEN_LATTICE_SPECTRUM_v1.md`](../../10_eft_program/preregistrations/PREREG_HYDROGEN_LATTICE_SPECTRUM_v1.md),
tag `preregister-hydrogen-lattice-spectrum-v1`, lock commit `6be49fe9`; artifact SHA
`8e953fac…`. **Verdict per frozen logic: `HYDROGEN-CONFIRMED`** (G-1 ∧ F-A ∧ F-B ∧ F-C ∧ F-E all PASS).
**Run of record:** `scripts/exploration/results/hydrogen_spectrum_2026-06-12.{csv,log}` (local).
**Companions:** Leg 0 [`DERIV_GUIDANCE_ABSENCE_NOGO.md`](DERIV_GUIDANCE_ABSENCE_NOGO.md)
(commit `fe05c473`); Leg 2 (engine time-series demonstration) **QUEUED**, not run.

---

## 0 · The result, stated at its honest ceiling

**GIVEN** (i) the de Broglie clock (one scalar ω₀ ∝ M_REST, FTD-0271 `[IMPOSED]`; its
covariant *rate* is FTD-native per FTD-0252/0271-A5) and (ii) a scalar-potential
coupling `ω_eff²(r) = ω₀² + 2ω₀V(r)` — the same structural move the engine already
makes for gravity, applied to the Gauss potential — **the engine's exact lattice
machinery produces a hydrogen-like bound-state spectrum**: a Rydberg-approaching level
ladder, the correct O_h-split multiplet structure, in the engine's own Coulomb
potential, which is itself verified Coulombic against ideal 1/r to ~2–5%.

This flips the framework's worst sector statement from "atomic dynamics ~0% derived"
(FTD-0270, which **stands unconditionally**) to "atomic bound-state structure is
derivable GIVEN one motivated scalar + one motivated coupling." KG-in-a-Coulomb-well
is textbook 1926 physics — the FTD content is that **every other ingredient is the
engine's own theorem-grade machinery** (the 18-pt operator, exact to 1.33×10⁻¹⁵; the
mean-free periodic Green's function = the engine's Gauss solution, OT-1.4 `[THEOREM]`).
**This is never to be cited as "FTD derives QM/Schrödinger/hydrogen."** FC-1 stands.

## 1 · Construction

Spectroscopy operator `A = −c²L₁₈ + 2ω₀V`, `V = +q·φ_G` (mean-free engine convention);
carrier `ω_n = √(ω₀² + a_n)`; envelope `E_n = ω_n − ω₀` (the Schrödinger sector).
All falsifier observables are energy **gaps** (offset-invariant); the n=2 gap is
`gap12 = mean(E₂..E₅) − E₁`. Reference: periodized continuum `−q/(4πr)` at the same L,
identical core regularization and mean-free convention — torus truncation cancels in
the lattice/reference ratio. Record grid (frozen): ω₀ = 1.5, q = {1.1170, 0.9308,
0.6981} (Schrödinger-limit a₀ = {2.5, 3, 4}), L = {48, 64}, 10 lowest states per cell.

## 2 · Run-of-record results (all frozen gates PASS)

| q (a₀) | L | gap12 | lat/ref ratio (F-A) | Rydberg ratio gap12/(¾R) | T1u spread | A1g–T1u split |
|---|---|---|---|---|---|---|
| 1.1170 (2.5) | 48 | +0.02619 | 1.0207 | 1.964 | 0.000 | 0.066 |
| 1.1170 (2.5) | 64 | +0.02633 | 1.0157 | 1.975 | 0.000 | 0.060 |
| 0.9308 (3.0) | 48 | +0.01326 | 0.9888 | 1.432 | 0.000 | 0.086 |
| 0.9308 (3.0) | 64 | +0.01340 | 0.9956 | 1.447 | 0.000 | 0.048 |
| 0.6981 (4.0) | 48 | +0.00591 | 0.9522 | 1.135 | 0.000 | 0.196 |
| 0.6981 (4.0) | 64 | +0.00592 | 0.9697 | 1.138 | 0.000 | 0.090 |

- **G-1** operator correctness: |eig − M(k)| = 1.33×10⁻¹⁵ — PASS.
- **F-A** (engine potential Coulombic): all six ratios ∈ [0.952, 1.021] ⊂ 1 ± 0.05 —
  PASS. The engine's own Gauss potential binds like ideal 1/r at the same lattice.
- **F-B** (Rydberg approach — **the blind leg**; a₀ = 4 cells never run pre-lock):
  ratio strictly decreasing 1.975 → 1.447 → **1.138** across a₀ = {2.5, 3, 4} at
  L = 64, endpoint inside the frozen (1.0, 1.40) band — PASS. The level spacing
  converges to the continuum Rydberg from above as the Bohr radius grows off the
  lattice scale, exactly the discretization-correction signature.
- **F-C** (O_h multiplet structure): the n=2 quadruple appears in every cell as an
  A1g singlet + an **exactly degenerate T1u triple** (spread 0.000 of gap12; splitting
  ≤ 0.20 of gap12) — the hydrogen n=2 multiplet, lattice-split precisely as O_h
  representation theory requires — PASS.
- **F-E** (causal control): massless Dirichlet ground mode s = 0.944 ∈ [0.8, 1.2],
  reproducing FTD-0270 — the harness resolves both dispersions — PASS.

**Verdict (mechanical): HYDROGEN-CONFIRMED.**

## 3 · What is imposed vs derived (the register, settled)

| Ingredient | Status |
|---|---|
| 18-pt Laplacian, c² = 1/3 | engine axiom-level (theorem-grade symbol) |
| Lattice Coulomb potential φ_G | **`[THEOREM]`** (OT-1.4 / Phase-G — the engine's own Gauss solution) |
| Clock scalar ω₀ | **`[IMPOSED]`** (FTD-0271; covariant rate FTD-native; no ℏ in the substrate) |
| Scalar-potential coupling 2ω₀V | **`[IMPOSED — motivated]`** (the engine's gravity-clock move applied to φ; the flux wave does not natively feel φ — measured engine fact, and Leg 0's no-go is the same structural statement for the particle sector) |
| q_eff (well depth / a₀ scale) | **`[IMPOSED]`** lattice-scale choice (α = 1/137 puts a₀ off-lattice); cross-checked by the F-B a₀-series |
| Rydberg ladder, O_h multiplets, Coulombic binding | **derived given the above** (this run of record) |

## 4 · Limits and queued work

- **No absolute calibration is claimed** (no R-to-eV map; the q_eff are lattice-scale
  surrogates, not α).
- The continuum q² scaling is not testable on-lattice without folding in
  discretization running (measured in development, disclosed in the pre-reg §7.5);
  the same-L reference ratio carries that content.
- **Leg 2 (QUEUED):** the engine-native demonstration — `db_clock_coulomb` toggle
  (clock at all sites with ω_eff²(r) sourced from the engine's live Gauss φ around a
  locked central charge) + time-series spectroscopy campaign; falsifier F-5: engine
  FFT peaks match this leg's ω_n = √(ω₀² + a_n) at the same (L, q). Golden gate must
  stay `0x56fa28acb5b9fe88` (default-off toggle).
- A v2 with larger L (96–128, shift-invert) could push the Rydberg ratio toward 1 and
  bind n=3; queued, not required for the verdict.

## 5 · Epistemic accounting

**Nothing is promoted.** FTD-0270's unconditional boundary ("atomic spectra NOT
substrate-derivable" for unmodified FTD) stands; FC-1 stands; FTD-0013 `[SMC]`,
MC-T4.3 unchanged. This row lands as a new `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]`
result — the conditional twin of FTD-0271, extended from free-particle kinematics
(Schrödinger envelope, de Broglie λ) to **bound-state structure** (the Rydberg ladder
and its multiplets) with the binding potential supplied by the engine's own theorem.
Next free LEDGER id after FTD-0278: **FTD-0279**.
