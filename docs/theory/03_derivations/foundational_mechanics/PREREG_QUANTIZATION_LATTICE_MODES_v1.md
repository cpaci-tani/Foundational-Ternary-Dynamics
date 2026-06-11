# PREREG — FTD-0270: Lattice Quantization & the Atomic-Dispersion Boundary

**Status:** `[PRE-REGISTRATION — design locked before the run of record]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0270 (reserved)
**Git tag:** `preregister-quantization-lattice-modes-v1` (applied at the lock commit)

## 0 · Purpose

Attempt to derive discrete energy-level **quantization** from the FTD lattice substrate **without importing ℏ**, and locate the exact boundary where the substrate stops. This is the one genuinely-attackable piece of "atomic dynamics" (status map: `AUDIT_ATOMIC_DYNAMICS_STATUS.md`). It is **not** an attempt to beat QM on atoms — the pre-registered prior is that it lands a sharp **BOUNDARY**, and the boundary is the deliverable.

## 1 · Frozen artifact

| Role | Path | SHA256 |
|---|---|---|
| Analysis | `scripts/exploration/derive_quantization_lattice_modes_2026-06-11.py` | `fe0a8ec1a39f0f2983ab1b8dea9f99e3d06298766942105030d227c08ea2f796` |

Run of record: `python scripts/exploration/derive_quantization_lattice_modes_2026-06-11.py --box-Ls 12,16,20,24,32 --out scripts/exploration/results/quantization_2026-06-11.csv`. First valid run is the run of record. (A reduced 2-point smoke at `--box-Ls 8,10` was run pre-lock for code validation only; the criteria below are theory-derived, not data-derived.)

## 2 · The physics and why the criteria are pre-determined by theory

FTD's flux obeys a **classical wave equation, 2nd-order in time** (leapfrog, `phase_write.cpp`): `∂²J/∂t² = c²·Lap18(J)`, exact 18-pt symbol `M(k) = (2/3)Σcos kᵢ + (2/3)Σcos kᵢcos kⱼ − 4`. A bound lattice region has discrete standing-wave eigenmodes (no ℏ needed for discreteness). The discriminator is the **dispersion**, which is analytically known:

- **FTD 2nd-order wave:** physical excitation `ω = c·√(−M) ∝ |k|` (LINEAR) ⇒ box ground mode `ω₁ ∝ 1/L`, finite-size exponent **s = 1**.
- **Schrödinger (atoms):** `E = −c²M ∝ k²` (QUADRATIC) ⇒ `E₁ ∝ 1/L²`, **s = 2**, and Rydberg `1/n²`.

So `s` is a clean, theory-fixed discriminator. Candidate 2 (de Broglie) is the same fact kinetically: a non-dispersive medium has no `λ ∝ 1/v` wavelength.

## 3 · Gates (mechanical, must pass or run INVALID)

- **G-1 operator correctness:** periodic-BC eigenvalues match the closed-form symbol `M(k)` to `< 1e-10`.
- **G-2 discriminator validity:** the Schrödinger-analog diagnostic (same eigenvectors, energy readout `E=−c²M`) gives finite-size exponent `s_schrod ∈ [1.6, 2.4]` — proving the test can resolve quadratic dispersion (else the test is broken).

## 4 · Frozen outcome map

Fit `s` (FTD wave) and `s_schrod` over `Ls={12,16,20,24,32}`; fit de Broglie `r` in `λ ∝ v^−r`.

- **C1 = `WAVE-CAVITY-BOUNDARY`** if `s ∈ [0.8, 1.2]`. **= `RYDBERG-SHAPE`** if `s ∈ [1.8, 2.2]` (UNEXPECTED — forces a re-audit before any claim). Else `AMBIGUOUS`.
- **C2 = `DE-BROGLIE-CONFIRMED`** if `r ∈ [0.7, 1.3]` (and a clean >3σ wake peak). **= `DE-BROGLIE-FAILED`** if `r < 0.3` or no clean wake (NULL). Else `AMBIGUOUS`.

**Combined verdict:**
| C1 | C2 | Verdict |
|---|---|---|
| WAVE-CAVITY-BOUNDARY | DE-BROGLIE-FAILED | **QUANTIZATION EXISTS, WRONG DISPERSION** — lattice quantizes (discrete modes) but with linear cavity dispersion, not Schrödinger quadratic; no de Broglie wave; atomic spectra NOT substrate-derivable; the boundary is the dispersion law. `[MEASURED — BOUNDARY]` |
| RYDBERG-SHAPE | (any) | Re-audit; do not claim. |
| AMBIGUOUS | AMBIGUOUS | Inconclusive; report exponents. |

## 5 · Committed regardless of outcome (the ℏ boundary)

Every quantity is **dimensionless** (`ω_n` in units `c/a`, `λ` in units `a`). Physical eV levels need `E = ℏω` (C1) or ℏ in `λ = h/p` (C2). The substrate fixes the *shape* (n-dependence), never the *scale*: the gap is exactly one action quantum. This is stated whatever the verdict.

## 6 · Priors and scope

Prior (disclosed): **WAVE-CAVITY-BOUNDARY + DE-BROGLIE-FAILED ~85%** (analytically predicted by the 2nd-order/linear dispersion), RYDBERG <5% (would force re-audit), ambiguous ~10%. Under every outcome: nothing promoted; FTD-0013, MC-T4.3, FC-1 unchanged. This maps a boundary; it does not derive ℏ, the Born rule, or atomic spectra. Candidate 1 modes are **field/cavity modes, not atomic levels** (FTD's electron is a manifested cluster, not a wavefunction); the deliverable is the dispersion law, not an atomic-level claim.
