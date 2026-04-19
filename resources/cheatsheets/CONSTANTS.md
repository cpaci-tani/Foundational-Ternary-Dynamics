# FTD Constants Cheatsheet

Every value traces to `scripts/constants.py`. Errors are against CODATA 2022 / PDG 2024.

## Framework integers (the four pillars)

| Symbol | Value | Origin |
|---|---|---|
| `N_c` | **3** | Master quadratic smaller root `x₋` (number of colors) |
| `N_base` | **4** | Second FLT-forbidden exponent — dimensionality of base |
| `b_3` | **7** | QCD β-coefficient `(11 N_c − 2 N_f)/3` at `N_f = 6` |
| `N_eff` | **13** | Fibonacci `F_7` — effective degrees of freedom |

Derived: `D_constraint = N_c · N_base² − 1 = 47`, `N_gen = ⌊x₋⌋ = 3` generations.

## Mathematical primitives

| Symbol | Expression | Value |
|---|---|---|
| `Γ(1/4)` | gamma-quarter | 3.62561 |
| `Γ(1/2)` | gamma-half = `√π` | 1.77245 |
| `Γ(3/4)` | gamma-three-quarter | 1.22542 |
| `G*` | `Γ(1/4) / Γ(3/4)` = `Γ(1/4)² / (√2 · Γ(1/2)²)` | **2.9586751** |
| `ϖ` | classical lemniscate constant = `Γ(1/4)² / (2√2 · Γ(1/2))` | 2.6220575 |
| `r_γ` | `Γ(1/4)² / Γ(1/2)²` — the single dimensionless framework ratio | 4.184 |
| `PF` | packing fraction `= Γ(1/2)² / 4 = π/4` | 0.7854 |
| `ε` | CFT anomaly `= e^π − π − 20` | ≈ 0.0047 |

## Master quadratic

`x² − 16 G*² x + 16 G*³ = 0`

| Root | Value | Meaning |
|---|---|---|
| `x₊` (tree) | 137.030 | 1/α at tree level |
| `x₊` (4-term) | 137.0359989 | after `c₁…c₄` corrections (see FORMULAS.md) |
| `x₊` (1-loop lattice) | 137.036000… | after tadpole integral closes 99.2 % of gap |
| `x₋` | 3.024 | ≈ `N_c` |
| `x₊ − x₋` | 134.012 | φ³ EFT mass² |

## Couplings

| Symbol | Formula | Derived | Experimental | Tag |
|---|---|---|---|---|
| `α⁻¹` (engine, precision) | `X_PLUS_PRECISION` (4-term corrected) | **137.035999177** | 137.035999177 | [THEOREM] |
| `α⁻¹` (tree, reference only) | `x₊` (master-quadratic root) | 137.0361714582 | — | [THEOREM] |
| `α_s(M_Z)` | `b_3 / (b_3 + 4 N_eff)` = 7/59 | 0.1186 | 0.1179 ± 0.0009 | [THEOREM] |
| `sin²θ_W` | `N_c / N_eff` = 3/13 | 0.23077 | 0.23122 ± 0.00003 | [THEOREM] |
| `α_G` | `2π (N_base²/N_c)² (N_eff + N_c/b_3)² α²⁰` | ≈ 10⁻⁴⁰ | ≈ 10⁻⁴⁰ | [THEOREM] |

Since 2026-04-17 the engine's `ALPHA` is the precision value (1.26 ppm tighter than tree-level). `ALPHA_TREE = 1/X_PLUS` is exposed for reference comparisons only.

## Engine constants (all live in `ontic.h` via `constants.py`)

| Symbol | Value | Use |
|---|---|---|
| `K_B` | 0.511 MeV | manifestation threshold = electron mass |
| `K_GENESIS` | 1.533 MeV | `K_B · N_c` — fill all color channels |
| `C_SPEED` | `1/√3` ≈ 0.577 | CFL speed limit on cubic lattice |
| `C_WAVE` | `1/√3` | wave propagation speed (= speed limit) |
| `DAMPING` | `α` ≈ 7.30 × 10⁻³ | dissipation rate |
| `G_N` | `1 / (b_3 + N_c)²` = 0.01 | gravitational coupling on lattice |
| `LATTICE_SPACING` | `2/D` = 2/3 | [SELECTION] boundary-to-bulk ratio |

## Mass sector

| Particle | FTD formula | Derived | Experimental | Error |
|---|---|---|---|---|
| electron `m_e` | `m_P · √(2π) · (N_base²/N_c) · α¹¹` | 0.510 MeV | 0.51099895 | 0.27 % |
| muon `m_μ/m_e` | `3 b_3 (b_3 + N_c) − N_c` | 207 | 206.768 | 0.11 % |
| tau `m_τ/m_e` | `(N_eff + N_base) μ − 2 N_c b_3` | 3477 | 3477.23 | < 0.01 % |
| Higgs `m_H` | `(N_eff/α²) · m_e` | 124.8 GeV | 125.25 | 0.36 % |
| proton `m_p/m_e` | `N_eff/α + N_base · N_eff + N_c` | 1836.47 | 1836.153 | 174 ppm |

## Master coefficient family (loop corrections)

| Coefficient | Value | Expression |
|---|---|---|
| `c₁` | 9/47 = 0.1915 | `N_c² / D_constraint` |
| `c₂` | 5/64 = 0.0781 | `(N_eff − 2 N_base) / N_base³` |
| `c₃` | 4/141 = 0.0284 | `N_base / (N_c · D_constraint)` |
| `c₄` | 141/11 = 12.818 | `(N_c · D_constraint) / (b_3 + N_base)` |

## Consciousness sector (Scale 11)

| Symbol | Value | Meaning |
|---|---|---|
| `y_real` | `G*² / 4` = 2.188 | real part of consciousness roots |
| `\|y\|²` | `G*³ / 2` = 12.96 | squared magnitude |
| `cos²θ_C` | `G*/8` = 0.370 | observable fraction |
| `sin²θ_C` | 1 − `G*/8` = 0.630 | subjective fraction |
| `C_mandelbrot` | `1/G*` = 0.338 | sLoop fixed point |

## Cross-references

- Full derivation chain: `docs/SPEC_FTD.md`
- Motivic proof of the master quadratic: `scripts/proofs/proof_motivic_master_quadratic.py`
- Complete SM computation: `scripts/proofs/proof_complete_sm.py`
- Framework-integer provenance: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`
