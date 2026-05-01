<!--
  AUTO-GENERATED — DO NOT HAND-EDIT.
  Source: docs/theory/01_reference/dimensional_map.json
  Renderer: scripts/proofs/build_dimensional_map.py
  To update: edit the JSON, then run `python scripts/proofs/build_dimensional_map.py`.
-->

# FTD Dimensionless ↔ Dimensional Map

**Schema:** v1 · **Scope:** algebraic-spine + calibration · **FTD version:** 5.34 · **Generated:** 2026-04-29

> Spine + calibration only. Covers the 7 algebraic-spine theorems, 4 dimensionless physical identifications, the 3 calibration declarations theorem-enforced by FTD-0059 + FTD-0096, and 1 worked dimensional application (m_e in MeV). Full SM-quantity coverage lives in CATALOG_PARAMETRIC_INSERTIONS.md.

## §1 · Why three layers

FTD's predictions sit in three distinct epistemic layers. The boundaries between them are **theorem-enforced**, not stylistic:

1. **Dimensionless layer** — pure-number theorems derivable from D=3 + varpi without any physical-unit calibration. The algebraic spine (G\*, master quadratic, Watson, |Aut(E)|², CM uniqueness, Phase G, Phase J) lives entirely here.
2. **Calibration layer** — declared (not derived) anchors that map lattice units to physical units. **Exactly two** SI-dimensional calibrations are theorem-enforced as the irreducible minimum: `a_phys ≡ ℓ_P` (length) and `K_B = m_e` (mass). The no-go theorems FTD-0059 (length) and FTD-0096 (mass) close all four mechanism candidates (α/β/γ/δ); these calibrations are theorem-enforced *by exclusion*, not convenience.
3. **Dimensional layer** — physical-unit predictions reachable only after passing through the calibration. Every dimensional FTD value (m_e in MeV, lifetimes in seconds, lengths in metres) is a dimensionless ratio multiplied by one of the two calibration anchors.

## §2 · Spine theorems

Pure-mathematics theorems. No physical-unit content; falsifiable on their algebraic claims alone. Tagged `[THEOREM]`.

| ID | Theorem | Formula | Value | LEDGER |
|---|---|---|---:|---|
| `g_star_identity` | G* algebraic identity | G* = Γ(1/4)/Γ(3/4) = √2·Γ(1/4)²/(2π) | 2.958675119 | [FTD-0002](../07_assessment/LEDGER.md#ftd-0002) |
| `master_quadratic` | Master quadratic polynomial + roots | P(x) = x² − 16·G*²·x + 16·G*³ = 0; roots x± = 8·G*² ± √(16·G*⁴ − 4·G*³) | x_plus = 137.03617145815542, x_minus = 3.023963916339028 | [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0081](../07_assessment/LEDGER.md#ftd-0081) |
| `cm_curve_uniqueness` | CM curve uniqueness among class-number-1 fields | Among d ∈ {-3, -4, -7, -8, -11, -19, -43, -67, -163}, only d = -4 yields master-quadratic roots that simultaneously match dimensionless physical constants to permille precision | unique_discriminant = -4, class_number = 1 | [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0014](../07_assessment/LEDGER.md#ftd-0014) |
| `coefficient_16_aut_e_squared` | Coefficient 16 = |Aut(E)|² | For E: y² = x³ − x, |Aut(E)| = 4 over ℚ̄, so |Aut(E)|² = 16 (the master-quadratic coefficient) | 16 | [FTD-0006](../07_assessment/LEDGER.md#ftd-0006), [FTD-0007](../07_assessment/LEDGER.md#ftd-0007) |
| `watson_identity` | Watson identity W₃ = G*²/(2π) | W₃ := ∫_0^π dx ∫_0^π dy ∫_0^π dz [3 − cos x − cos y − cos z]⁻¹ / π³ = G*²/(2π) | 1.39320393 | [FTD-0002](../07_assessment/LEDGER.md#ftd-0002) |
| `phase_g_geometric_coulomb` | Phase G geometric Coulomb identity | α_r(r, L) = 2·r·G_L(r), where G_L is the periodic lattice Poisson Green's function. Holds at every finite L without free parameters. | — | [FTD-0004](../07_assessment/LEDGER.md#ftd-0004) |
| `phase_j_ultralocality` | Phase J partition-function ultralocality at L=2 | At L=2, the partition function Z factorizes site-locally: Z = ∏_v Z_v(s_v, J_v). No connected correlators between voxels. | — | [FTD-0005](../07_assessment/LEDGER.md#ftd-0005) |

### G* algebraic identity (`g_star_identity`)

- **Formula:** G* = Γ(1/4)/Γ(3/4) = √2·Γ(1/4)²/(2π)
- **Value:** 2.958675119
- **LEDGER:** [FTD-0002](../07_assessment/LEDGER.md#ftd-0002)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`
- **Notes:** Direct Chowla–Selberg evaluation of L(s, χ_{-4}) at s=1 on the lemniscatic curve y² = x³ − x. Four independent derivations enumerated in MONOGRAPH_GSTAR_BRIDGE_CONSTANT (Γ-ratio, Watson period integral, lemniscate arc length, modular-form value).

### Master quadratic polynomial + roots (`master_quadratic`)

- **Formula:** P(x) = x² − 16·G*²·x + 16·G*³ = 0; roots x± = 8·G*² ± √(16·G*⁴ − 4·G*³)
- **Value:** x_plus = 137.03617145815542, x_minus = 3.023963916339028
- **Depends on:** `g_star_identity`
- **LEDGER:** [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0081](../07_assessment/LEDGER.md#ftd-0081)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`; `docs/theory/02_foundations/FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md`; `scripts/proofs/proof_motivic_master_quadratic.py`
- **Notes:** Two independent derivations (FTD-0081): Route A (physics — Gaussian J-integration of S_E) and Route B (arithmetic — Damerell–Shimura at CM curve E_i). Both produce the identical polynomial to 100 digits.

### CM curve uniqueness among class-number-1 fields (`cm_curve_uniqueness`)

- **Formula:** Among d ∈ {-3, -4, -7, -8, -11, -19, -43, -67, -163}, only d = -4 yields master-quadratic roots that simultaneously match dimensionless physical constants to permille precision
- **Value:** unique_discriminant = -4, class_number = 1
- **Depends on:** `g_star_identity`, `master_quadratic`
- **LEDGER:** [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0014](../07_assessment/LEDGER.md#ftd-0014)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md`; `scripts/proofs/scan_cm_curves.py`
- **Notes:** Numerical verification across all 9 class-number-1 discriminants. Uniqueness extends only within the class-number-1 family; class-number ≥ 2 is [OPEN].

### Coefficient 16 = |Aut(E)|² (`coefficient_16_aut_e_squared`)

- **Formula:** For E: y² = x³ − x, |Aut(E)| = 4 over ℚ̄, so |Aut(E)|² = 16 (the master-quadratic coefficient)
- **Value:** 16
- **LEDGER:** [FTD-0006](../07_assessment/LEDGER.md#ftd-0006), [FTD-0007](../07_assessment/LEDGER.md#ftd-0007)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`
- **Notes:** Three independent routes converge on 16: direct |Aut(E)|² (Route A, FTD-0006); z_BCC × 2 (Route B, FTD-0007); j-invariant + modular form (additional).

### Watson identity W₃ = G*²/(2π) (`watson_identity`)

- **Formula:** W₃ := ∫_0^π dx ∫_0^π dy ∫_0^π dz [3 − cos x − cos y − cos z]⁻¹ / π³ = G*²/(2π)
- **Value:** 1.39320393
- **Depends on:** `g_star_identity`
- **LEDGER:** [FTD-0002](../07_assessment/LEDGER.md#ftd-0002)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`
- **Notes:** Watson's third triple integral. Bundled with G* under FTD-0002 because it is one of the four independent derivations of G*; here it is also recorded as a structural identity in its own right (the BCC eigenvalue triple-cosine product).

### Phase G geometric Coulomb identity (`phase_g_geometric_coulomb`)

- **Formula:** α_r(r, L) = 2·r·G_L(r), where G_L is the periodic lattice Poisson Green's function. Holds at every finite L without free parameters.
- **LEDGER:** [FTD-0004](../07_assessment/LEDGER.md#ftd-0004)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md`
- **Notes:** Reframed away from the QED-deviation reading: the lattice α plateau at ≈1.8× α_ref is the Poisson Green's function shape, not a fine-structure correction. Zero free parameters.

### Phase J partition-function ultralocality at L=2 (`phase_j_ultralocality`)

- **Formula:** At L=2, the partition function Z factorizes site-locally: Z = ∏_v Z_v(s_v, J_v). No connected correlators between voxels.
- **LEDGER:** [FTD-0005](../07_assessment/LEDGER.md#ftd-0005)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`
- **Notes:** Direct algebraic statement about the L=2 partition function. Empirical engine support: FTD-0075 (flux propagator on Langevin ensemble shows essentially constant correlator at distance, consistent with ultralocality).

## §3 · Dimensionless predictions

Dimensionless quantities FTD predicts and that have direct experimental analogues. No calibration enters; comparison to lab is direct.

| ID | Quantity | FTD value | Lab measurement | Comparison | Tag | LEDGER |
|---|---|---:|---|---|---|---|
| `alpha_inverse` | 1/α (fine-structure constant) ↔ master-quadratic root x₊ | 137.0359992 | 137.0359992 ± 2.1e-08 (CODATA 2022) | Δ = -0.21 ppb; tier: hard | STRONGLY MOTIVATED CONJECTURE | [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0013](../07_assessment/LEDGER.md#ftd-0013) |
| `n_color` | N_c (number of colors) ↔ master-quadratic root x₋ | 3.023963916 | 3 (Standard Model (exact integer, not a measurement)) | Δ = 7.99e+06 ppb; tier: hard | STRONGLY MOTIVATED CONJECTURE | [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0014](../07_assessment/LEDGER.md#ftd-0014) |
| `mu_over_e_mass_ratio` | m_μ / m_e (muon/electron mass ratio) | 207 | 206.768283 ± 4.6e-07 (CODATA 2022) | Δ = 1.12e+06 ppb; tier: parametric | STRONGLY MOTIVATED CONJECTURE | [FTD-0008](../07_assessment/LEDGER.md#ftd-0008) |
| `tau_over_e_mass_ratio` | m_τ / m_e (tau/electron mass ratio) | 3477 | 3477.23 ± 0.23 (PDG 2024) | Δ = -6.61e+04 ppb; tier: parametric | STRONGLY MOTIVATED CONJECTURE | [FTD-0008](../07_assessment/LEDGER.md#ftd-0008) |

### 1/α (fine-structure constant) ↔ master-quadratic root x₊ (`alpha_inverse`)

- **Formula:** x₊ root of master quadratic; tree value 137.0362, with 7-term 1-loop series matching CODATA to 24 digits (post-hoc fit beyond CODATA precision per AUDIT_MASTER_QUADRATIC)
- **FTD value:** 137.0359992
- **Lab:** 137.0359992 ± 2.1e-08 (CODATA 2022)
- **Comparison:** Δ = -0.21 ppb; tier: hard
- **Tag:** `STRONGLY MOTIVATED CONJECTURE`
- **Depends on:** `master_quadratic`
- **LEDGER:** [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0013](../07_assessment/LEDGER.md#ftd-0013)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md`; `docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md`
- **Notes:** Polynomial-level theorem (x₊ is a root of a number-theoretic polynomial); the physical identification x₊ = 1/α is [STRONGLY MOTIVATED CONJECTURE] (downgraded from THEOREM 2026-04-19) because it depends on dual-match + CM-curve uniqueness rather than a derivation chain to QED. FTD-0097 look-elsewhere scan (2026-04-27) confirms the master quadratic's polynomial-root layer lives outside the monomial scan space and is unaffected by the over-richness verdict.

### N_c (number of colors) ↔ master-quadratic root x₋ (`n_color`)

- **Formula:** x₋ root of master quadratic; numerical value 3.024
- **FTD value:** 3.023963916
- **Lab:** 3 (Standard Model (exact integer, not a measurement))
- **Comparison:** Δ = 7.99e+06 ppb; tier: hard
- **Tag:** `STRONGLY MOTIVATED CONJECTURE`
- **Depends on:** `master_quadratic`
- **LEDGER:** [FTD-0001](../07_assessment/LEDGER.md#ftd-0001), [FTD-0014](../07_assessment/LEDGER.md#ftd-0014)
- **Sources:** `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`; `docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md`
- **Notes:** Same epistemic tier as α: polynomial-root layer is theorem; physical identification is conjecture. Four independent topological routes converge on N_c = 3 (DERIV_NC_FROM_TOPOLOGY).

### m_μ / m_e (muon/electron mass ratio) (`mu_over_e_mass_ratio`)

- **Formula:** m_μ/m_e = 3·b₃·(b₃ + N_c) − N_c = 3·7·10 − 3 = 207
- **FTD value:** 207
- **Lab:** 206.768283 ± 4.6e-07 (CODATA 2022)
- **Comparison:** Δ = 1.12e+06 ppb; tier: parametric
- **Tag:** `STRONGLY MOTIVATED CONJECTURE`
- **LEDGER:** [FTD-0008](../07_assessment/LEDGER.md#ftd-0008)
- **Sources:** `docs/theory/05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md`; `docs/theory/05_particles/PRED_ELECTROWEAK_MASSES.md`; `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`
- **Notes:** Derived from framework integers {b₃=7, N_c=3} with no free parameters. The formula is integer arithmetic; the 0.11% gap to CODATA is the radiative correction not yet computed in FTD.

### m_τ / m_e (tau/electron mass ratio) (`tau_over_e_mass_ratio`)

- **Formula:** m_τ/m_e = (N_eff + N_base)·μ_ratio − 2·N_c·b₃ = 17·207 − 42 = 3477
- **FTD value:** 3477
- **Lab:** 3477.23 ± 0.23 (PDG 2024)
- **Comparison:** Δ = -6.61e+04 ppb; tier: parametric
- **Tag:** `STRONGLY MOTIVATED CONJECTURE`
- **Depends on:** `mu_over_e_mass_ratio`
- **LEDGER:** [FTD-0008](../07_assessment/LEDGER.md#ftd-0008)
- **Sources:** `docs/theory/05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md`; `docs/theory/05_particles/PRED_ELECTROWEAK_MASSES.md`; `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`
- **Notes:** Derived from framework integers {N_eff=13, N_base=4, N_c=3, b₃=7} with no free parameters. 0.01% match to PDG.

## §4 · Calibration declarations

Two SI-dimensional calibrations are theorem-enforced as the irreducible minimum (`FTD-0059` for length, `FTD-0096` for mass; calibration-interface theorem in the latter). Time follows from length + the cubic-lattice CFL constraint.

| ID | Anchor | Formula | Value | Tag | LEDGER |
|---|---|---|---|---|---|
| `a_phys_planck` | a_phys ≡ ℓ_P (length anchor) | DECLARED: 1 voxel ≡ 1 Planck length ℓ_P | 1.616255e-35 m | CALIBRATION | [FTD-0030](../07_assessment/LEDGER.md#ftd-0030), [FTD-0041](../07_assessment/LEDGER.md#ftd-0041), [FTD-0059](../07_assessment/LEDGER.md#ftd-0059) |
| `t_phys_lattice_tick` | t_phys (one tick in seconds) | t_phys = √3 · ℓ_P / c (CFL condition c_lat = 1/√3 + a_phys ≡ ℓ_P + c_phys = 2.998 × 10⁸ m/s) | 9.34e-44 s | CALIBRATION | [FTD-0041](../07_assessment/LEDGER.md#ftd-0041) |
| `mass_unit_anchor` | K_B = m_e (mass-unit anchor) | DECLARED: lattice manifestation threshold K_B = 0.511 in lattice units corresponds to m_e in physical units. Mass-unit ≡ m_e/K_B = 1 MeV/c². | 0.5109989461 MeV/c² | IMPOSED | [FTD-0041](../07_assessment/LEDGER.md#ftd-0041), [FTD-0096](../07_assessment/LEDGER.md#ftd-0096) |

### a_phys ≡ ℓ_P (length anchor) (`a_phys_planck`)

- **Formula:** DECLARED: 1 voxel ≡ 1 Planck length ℓ_P
- **Value:** 1.616255e-35 m
- **Tag:** `CALIBRATION`
- **LEDGER:** [FTD-0030](../07_assessment/LEDGER.md#ftd-0030), [FTD-0041](../07_assessment/LEDGER.md#ftd-0041), [FTD-0059](../07_assessment/LEDGER.md#ftd-0059)
- **Sources:** `docs/SPEC_FTD.md`; `docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md`; `docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`; `docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_DELTA_ATTEMPT.md`
- **Calibration note:** FTD-0059 (no-go theorem, 2026-04-23): no length is expressible from Axiom-Zero invariants alone. Mechanisms α/β/γ/δ all closed negative. The Planck-length declaration is theorem-enforced as the irreducible minimum, not a convenience choice. ℓ_P value from CODATA 2022: ≈1.616255 × 10⁻³⁵ m.
- **Notes:** First of two SI-dimensional calibrations theorem-enforced by FTD-0096.

### t_phys (one tick in seconds) (`t_phys_lattice_tick`)

- **Formula:** t_phys = √3 · ℓ_P / c (CFL condition c_lat = 1/√3 + a_phys ≡ ℓ_P + c_phys = 2.998 × 10⁸ m/s)
- **Value:** 9.34e-44 s
- **Tag:** `CALIBRATION`
- **Depends on:** `a_phys_planck`
- **LEDGER:** [FTD-0041](../07_assessment/LEDGER.md#ftd-0041)
- **Sources:** `docs/SPEC_FTD.md`
- **Calibration note:** Derived (not declared) from a_phys ≡ ℓ_P and the CFL stability constraint c_lat = 1/√3 (which is itself a [THEOREM] from the cubic-lattice wave equation). Numerically t_phys ≈ 9.34 × 10⁻⁴⁴ s.
- **Notes:** Once a_phys is fixed, t_phys is fully determined. No additional calibration freedom.

### K_B = m_e (mass-unit anchor) (`mass_unit_anchor`)

- **Formula:** DECLARED: lattice manifestation threshold K_B = 0.511 in lattice units corresponds to m_e in physical units. Mass-unit ≡ m_e/K_B = 1 MeV/c².
- **Value:** 0.5109989461 MeV/c²
- **Tag:** `IMPOSED`
- **LEDGER:** [FTD-0041](../07_assessment/LEDGER.md#ftd-0041), [FTD-0096](../07_assessment/LEDGER.md#ftd-0096)
- **Sources:** `docs/SPEC_FTD.md`; `docs/theory/10_eft_program/THEOREM_MU_NO_GO_FTD0096.md`
- **Calibration note:** FTD-0096 (calibration-interface theorem, 2026-04-28): mass-unit μ_FTD is calibration, not derivation, on equal footing with a_phys. Three independent closures of FTD-0094 confirm this from methodological (FTD-0097 look-elsewhere null-rejected upward), structural-mechanism (FTD-0093 Mechanism C closed negative), and dimensional (this no-go) sides. Exactly two SI-dimensional calibrations (a_phys ≡ ℓ_P and K_B = m_e) are theorem-enforced as the irreducible minimum.
- **Notes:** Second of two SI-dimensional calibrations. Together with a_phys, completes the calibration interface.

## §5 · Calibration applications (worked example)

How a dimensional FTD prediction is reached by composing a dimensionless ratio with a calibration anchor. Only m_e is worked here as an exemplar; every other dimensional consequence (m_μ in MeV, m_p in MeV, lifetimes in seconds, lengths in metres) follows the same pattern and is enumerated individually in `CATALOG_PARAMETRIC_INSERTIONS.md`.

### m_e (electron mass in MeV/c²) — worked example (`m_electron_dimensional`)

- **Formula:** m_e = K_B (lattice value 0.511) × mass-unit (1 MeV/c² per K_B unit) = 0.511 MeV/c²
- **FTD value:** 0.511 MeV/c²
- **Lab:** 0.5109989507 ± 1.6e-10 MeV/c² (CODATA 2022)
- **Comparison:** Δ = 2.05e+03 ppb; tier: imposed
- **Tag:** `IMPOSED`
- **Depends on:** `mass_unit_anchor`
- **LEDGER:** [FTD-0041](../07_assessment/LEDGER.md#ftd-0041)
- **Sources:** `docs/SPEC_FTD.md`; `scripts/constants.py`
- **Calibration note:** This is the bridge mechanism: every dimensional FTD prediction reduces to (a) a dimensionless ratio (e.g. m_μ/m_e = 207) times (b) the mass-unit anchor (1 MeV/c² ≡ m_e/K_B). The same pattern applies to lengths (multiply dimensionless ratio by a_phys = ℓ_P) and times (multiply by t_phys = √3·ℓ_P/c).
- **Notes:** Single worked example. Other dimensional consequences (m_μ in MeV via 207 × m_e, m_p in MeV via m_p/m_e × m_e, etc.) follow the same pattern and are catalogued individually in CATALOG_PARAMETRIC_INSERTIONS.md.

## §6 · Cross-reference summary

LEDGER ids touched by this map, with the entries that reference them. Use this section to find the map entry for a given LEDGER row, or vice versa.

| LEDGER id | Map entries |
|---|---|
| FTD-0001 | `alpha_inverse`, `cm_curve_uniqueness`, `master_quadratic`, `n_color` |
| FTD-0002 | `g_star_identity`, `watson_identity` |
| FTD-0004 | `phase_g_geometric_coulomb` |
| FTD-0005 | `phase_j_ultralocality` |
| FTD-0006 | `coefficient_16_aut_e_squared` |
| FTD-0007 | `coefficient_16_aut_e_squared` |
| FTD-0008 | `mu_over_e_mass_ratio`, `tau_over_e_mass_ratio` |
| FTD-0013 | `alpha_inverse` |
| FTD-0014 | `cm_curve_uniqueness`, `n_color` |
| FTD-0030 | `a_phys_planck` |
| FTD-0041 | `a_phys_planck`, `m_electron_dimensional`, `mass_unit_anchor`, `t_phys_lattice_tick` |
| FTD-0059 | `a_phys_planck` |
| FTD-0081 | `master_quadratic` |
| FTD-0096 | `mass_unit_anchor` |

## §7 · Editing this map

1. Edit `docs/theory/01_reference/dimensional_map.json` (the canonical data file).
2. Run `python scripts/proofs/build_dimensional_map.py` to regenerate this Markdown.
3. Run `pytest scripts/tests/test_dimensional_map.py -v` to verify schema + cross-references + value agreement against `scripts/constants.py`.

Drift detection: the renderer is deterministic. CI can run `build_dimensional_map.py --check` to confirm the committed Markdown matches what the JSON would produce.
