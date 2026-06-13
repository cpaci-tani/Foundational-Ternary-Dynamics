# ANALYSIS — Helium on the FTD lattice: HELIUM-CONFIRMED (FTD-0279)

**Status:** `[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT]` (pre-registered run of record).
**Date:** 2026-06-12.
**Pre-registration:** [`PREREG_HELIUM_LATTICE_SCF_v1.md`](../../10_eft_program/preregistrations/PREREG_HELIUM_LATTICE_SCF_v1.md),
tag `preregister-helium-lattice-scf-v1`, lock commit `310ad4ee`; artifact SHA `ecfa2cd0…`.
**Verdict per frozen logic: `HELIUM-CONFIRMED`** (F-He-A ∧ B ∧ C ∧ D all PASS, including
the blind L=64 trend leg).
**Run of record:** `scripts/exploration/results/helium_scf_2026-06-12.{csv,log}` (local).
**Companions:** FTD-0278 (HYDROGEN-CONFIRMED — the sector this extends);
FTD-0270 (the helium challenge that this finally answers, at its honest ceiling).

---

## 0 · The result, stated at its honest ceiling

The helium challenge — "FTD cannot compute helium" — produced FTD-0270's verdict that
atomic dynamics is ~0% substrate-derived. That unconditional verdict **stands**. What
this run establishes is the conditional answer: **GIVEN three motivated imports** —
(I1) the rest-mass clock, (I2) the scalar-potential coupling, and (I3, new) the
**mode-occupancy import** — **the engine's exact lattice machinery produces a mean-field
helium atom with the correct screening and ionization physics**, where the nuclear
attraction AND the electron–electron repulsion are both supplied by the engine's own
Gauss-law Green's function (OT-1.4 `[THEOREM]`, used twice).

**The new import (I3), honestly priced:** a classical field `J(x)` carries no
two-particle configuration space (L⁶). "Two electrons" = two unit-norm occupations of
the field's bound modes, each sourcing Gauss and feeling the other's Hartree
potential — the minimal slice of second quantization a 2-electron atom needs. Ground
state 1s²: spatially symmetric, spin-singlet bookkeeping imported. **No exchange, no
correlation.** Hartree-for-helium is textbook 1928; never cite this as "FTD derives
helium" unconditionally.

## 1 · Run-of-record results (all frozen gates PASS)

| q (a₀ᴴ) | L | E_He (ENG) | σ = E_He/E_nonint (REF; Δ) | I_He/I_He⁺ (REF; Δ) | cross-check |
|---|---|---|---|---|---|
| 0.4654 (6) | 48 | −0.016723 | 0.6767 (0.6911; 0.0144) | 0.3534 (0.3822; 0.0289) | 1.0e-14 |
| 0.4654 (6) | 64 | −0.018342 | 0.6824 (0.6972; 0.0148) | 0.3648 (0.3944; 0.0296) | 7.9e-15 |
| 0.3490 (8) | 48 | −0.006331 | 0.6867 (0.6889; 0.0022) | 0.3735 (0.3779; 0.0044) | 3.2e-14 |
| 0.3490 (8) | 64 | −0.007491 | **0.6929** (0.6942; 0.0013) | 0.3858 (0.3885; 0.0026) | 1.3e-14 |

Continuum (Hartree-Fock-level) targets: σ → **0.7154** (the Z_eff = Z − 5/16 = 27/16
screening physics), I_He/I_He⁺ → **0.4519**.

- **F-He-A** (the engine's potential does helium like 1/r — dimensionless): all
  |Δσ| ≤ 0.0148 (band 0.03), |Δion| ≤ 0.0296 (band 0.05) — PASS. The engine's Green's
  function reproduces the two-electron screening physics as faithfully as the ideal
  periodized Coulomb at the same lattice.
- **F-He-B** (screening; **the blind L=64 trend leg**): every σ in (0.60, 0.80), and
  at L = 64 the a₀ = 8 value (0.6929) sits closer to the continuum 0.7154 than the
  a₀ = 6 value (0.6824) — PASS. The whole table converges monotonically toward 0.7154
  with every step in a₀ or L.
- **F-He-C** (ionization ordering): all ratios in (0.30, 0.60), marching 0.353 → 0.386
  toward the continuum 0.452 — the second electron is bound, and roughly half as
  tightly, exactly the helium signature — PASS.
- **F-He-D** (independent-eigenpath He⁺ cross-check vs the locked FTD-0278 module):
  agreement to ≤ 3.2×10⁻¹⁴ in every cell — PASS.

## 2 · What is FTD-exact vs imported (the cumulative register)

| Ingredient | Status |
|---|---|
| 18-pt Laplacian operator | engine-exact (machine-precision symbol) |
| Nuclear well V_nuc = 2q·φ_G | **`[THEOREM]`** (OT-1.4, the engine's Gauss solution) |
| e–e Hartree repulsion −q·(φ_G ⊛ ρ) | **`[THEOREM]`** machinery (the same Green's function, FFT-convolved against the engine symbol) |
| Clock scalar ω₀ | `[IMPOSED]` (I1, FTD-0271) |
| Scalar-potential coupling | `[IMPOSED — motivated]` (I2, FTD-0278) |
| **Mode-occupancy (two electrons)** | **`[IMPOSED — motivated]` (I3, NEW)** — the minimal second-quantization slice; declared, not derived |
| Screening σ, ionization structure, mean-field E_He | **derived given the register** (this run) |

## 3 · Methodology findings (load-bearing for future spectroscopy)

1. **Absolute energies are core-convention artifacts** at lattice-scale orbitals:
   three reference conventions (minimum-image, core-matched, spectral) gave three
   absolute E_He, while the **dimensionless observables agreed across all three to
   ~2%** — core sensitivity cancels in the ratios. All falsifiers were therefore
   frozen on dimensionless observables; absolute ENG/REF (1.15–1.41 across the grid)
   is reported descriptively only.
2. The **spectral periodized Coulomb** (IFFT of −1/|k|²: same torus, full image sum,
   only the symbol differs from M(k)) is the right reference class for shallow-binding
   problems; the minimum-image construction under-counts torus images (measured in
   development, disclosed in the lock).

## 4 · The boundary beyond (declared before the run; now the standing edge)

**Correlation energy** — the 1.4% of E_He beyond mean-field (continuum −0.042 Hartree)
— lives in genuine two-particle configuration space: entanglement the classical
substrate does not carry. That is **FC-1 territory by construction**, and it is the
ceiling of the mode-occupancy import. The conditional sector now ends exactly where
the framework's own commitment (declining the QM measurement-map import) says it
should: **mean-field atoms in; correlation out.** The ortho/para (1s2s singlet–triplet)
splitting — which requires the exchange integral, a deeper statistics import — is
queued as a possible follow-up, not attempted.

## 5 · Epistemic accounting

**Nothing is promoted.** FTD-0270's unconditional boundary stands; FC-1 stands (and is
*illustrated* by §4); FTD-0013 `[SMC]`, MC-T4.3 unchanged; FTD-0278's register and
honesty rails carry over with I3 added. The conditional atomic sector now contains:
hydrogen (Rydberg ladder + O_h multiplets, FTD-0278) and mean-field helium (screening
+ ionization structure, this row) — each priced against its explicit import register.
Next free LEDGER id: **FTD-0280**.
