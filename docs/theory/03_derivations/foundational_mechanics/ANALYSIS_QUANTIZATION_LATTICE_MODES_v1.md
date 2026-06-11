# Analysis — FTD-0270: The Lattice Quantizes, But With the Wrong Dispersion (BOUNDARY)

**Tag:** `[MEASURED — BOUNDARY]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0270
**Pre-registration:** `PREREG_QUANTIZATION_LATTICE_MODES_v1.md`, tag `preregister-quantization-lattice-modes-v1`, lock commit `e1c7377f`
**Artifact:** `scripts/exploration/derive_quantization_lattice_modes_2026-06-11.py` (SHA `fe0a8ec1`); run of record `scripts/exploration/results/quantization_2026-06-11.csv`

---

## 0 · Verdict

The FTD lattice **does** quantize — a bound region has discrete standing-wave eigenmodes (no ℏ needed for discreteness, purely linear algebra). But the modes carry **linear/cavity dispersion** (`ω ∝ |k|`), which is **structurally the wrong dispersion** for atomic spectra: the hydrogen Rydberg `1/n²` requires the Schrödinger **quadratic** dispersion (`E ∝ k²`). And a moving cluster sources **no de Broglie wave**. So **discrete atomic energy levels are NOT substrate-derivable in FTD, and the boundary is the dispersion law itself.** This converts the vague "atomic spectra ABSENT" (`AUDIT_ATOMIC_DYNAMICS_STATUS.md`) into a sharp, measured, falsifiable structural statement.

## 1 · What was measured (run of record)

Both pre-registered gates passed: operator correctness `|eig(L18) − M(k)| = 1.33e-15` (machine precision); discriminator validity (the Schrödinger-analog diagnostic resolves quadratic dispersion, `s_schrod = 1.89 ∈ [1.6,2.4]`).

**C1 — finite-size scaling of the free Dirichlet box ground mode** (Ls = {12,16,20,24,32}):

| L | 12 | 16 | 20 | 24 | 32 |
|---|---|---|---|---|---|
| ω₁ (FTD 2nd-order wave) | 0.2399 | 0.1840 | 0.1492 | 0.1254 | 0.0951 |
| E₁ (Schrödinger-analog, same eigenvectors) | 0.0576 | 0.0339 | 0.0223 | 0.0157 | 0.0090 |

- **FTD wave: `ω₁ ∝ L^−0.944 ± 0.005`** → `s ≈ 1` → **`WAVE-CAVITY-BOUNDARY`** (linear dispersion `ω ∝ |k|`).
- **Schrödinger diagnostic: `E₁ ∝ L^−1.887 ± 0.010`** → `s ≈ 2` (quadratic). Same operator, same eigenvectors — the **only** difference is the physical readout: `ω = c√(−M)` (FTD, 2nd-order in time) vs `E = −c²M` (Schrödinger, 1st-order). The √ is the entire story.

**C2 — de Broglie test:** the flux wake of a cluster moving at velocity v has no characteristic wavelength that scales with v (`λ` flat, `r = 0.00`) → **`DE-BROGLIE-FAILED`**, exactly as a non-dispersive medium requires. (C2 is corroborating; the wake reduces to the box fundamental, i.e. no de Broglie scale exists — the same fact C1 measures, seen kinetically.)

**Combined verdict: `WAVE-CAVITY-BOUNDARY` + `DE-BROGLIE-FAILED` = QUANTIZATION EXISTS, WRONG DISPERSION.**

## 2 · The honest reading

- **What is genuinely new and positive:** FTD *does* produce discrete bound-state levels from the substrate (standing-wave quantization) — something the corpus previously listed as ABSENT. Quantization does not require ℏ; it is a boundary-condition fact.
- **Why it is a boundary, not a win:** the level *pattern* is set by the dispersion, and FTD's dispersion is **linear** (`s = 0.94`, cavity/EM-like), not the **quadratic** (`s = 2`) the hydrogen Rydberg needs. These modes are field/cavity modes, *not* atomic orbitals (FTD's electron is a manifested cluster, not a wavefunction). The dispersion mismatch is the precise, measured reason atomic spectra are out of reach — sharper than "ℏ is missing."
- **Where ℏ sits (committed regardless):** every quantity is dimensionless (`ω` in `c/a`, `λ` in `a`). Physical eV levels need `E = ℏω`; the substrate fixes the *shape* (the `s` exponent), never the *scale*. The gap to physical energies is exactly one action quantum, and the shape is already the wrong shape.

## 3 · What would change the verdict

Only a demonstration that some FTD field's *effective* dynamics is **1st-order in time with quadratic dispersion** (`ω ∝ k²`) — i.e. a substrate-native Schrödinger sector — would move `s` from 1 toward 2. The genesis/coupling pipeline at the linear level does not provide it (this analysis used the exact engine wave operator). That is the concrete open target; absent it, FTD-0270 stands as a derivation-blocking boundary.

## 4 · No promotions

FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, FC-1 (declines QM recovery) — all unchanged. This maps a boundary; it derives neither ℏ, the Born rule, nor atomic spectra. The linear-dispersion fact is the deliverable.
