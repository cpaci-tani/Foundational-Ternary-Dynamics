# FTD Parametric Insertions Catalog

**Purpose:** Canonical enumeration of every quantity where FTD *supplies numbers to a standard-physics formula* rather than *deriving the formula from lattice dynamics*. Honest to count; hazardous to mislabel as a derivation.

**Version:** 1.0
**Date:** 2026-04-19
**Status:** [REFERENCE] — foundational audit for the EFT Recovery Program (see `docs/theory/10_eft_program/`)

> **Epistemic framing.** This catalog exists because the tag `[DERIVED]` is often overapplied. A *derivation* takes FTD axioms and produces both the functional form and the numerical value. A *parametric insertion* borrows the functional form from external physics (QED, ChPT, Fermi theory, Regge phenomenology, seesaw ansatz, etc.) and inserts FTD-supplied integers or couplings into it. Insertions are not wrong — they are how any EFT cross-checks with the existing Standard Model — but they must be tagged honestly. A reviewer should be able to read this catalog and tell in five minutes which claims are independent evidence for FTD and which are cross-checks against known physics.

> **See also:** for the spine-layer + calibration interface (the 7 algebraic-spine theorems, 4 dimensionless physical predictions, 3 calibration declarations, and worked dimensional applications), see `docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md` (rendered) and `docs/theory/01_reference/dimensional_map.json` (canonical data). The dimensional map exposes the *bridge mechanism* (how dimensionless quantities cross over to physical units via the two theorem-enforced anchors `a_phys ≡ ℓ_P` and `K_B = m_e`); this catalog enumerates the ~129 parametric insertions that consume that bridge.

---

## 1 · Epistemic Tags Used in This Catalog

| Tag | Meaning | Counts as evidence for FTD? |
|-----|---------|---|
| **[DERIVED]** | Formula + numerical value both produced from FTD axioms alone | **Yes** |
| **[PARAMETRIC]** | Formula adopted from external physics; FTD supplies one or more parameters | No — cross-check only |
| **[IMPOSED]** | Value fit to experiment; FTD does not predict it | No |
| **[SELECTION]** | Formula argued from consistency; uniqueness not established | Weak evidence; critique required |
| **[THEOREM]** | Proven from axioms with no empirical fit | **Yes** |
| **[NULL-PREDICTION]** | Dynamics *forbid* a quantity (e.g., monopoles). Valuable when the absence is testable | **Yes** |

---

## 2 · Summary Statistics

| Category | Count | [DERIVED]/[THEOREM] | [PARAMETRIC] | [IMPOSED]/[SELECTION] |
|---|---|---|---|---|
| Foundational constants (α, N_c, G*) | 7 | 7 | 0 | 0 |
| Lepton-mass ratios | 3 | 3 | 0 | 0 |
| Quark masses (6 quarks) | 6 | 0 | 6 | 0 |
| Meson masses | ~42 | 0 | ~42 | 0 |
| Baryon masses | ~48 | 0 | ~48 | 0 |
| Mixing angles (PMNS/CKM) | 7 | 3 | 4 | 0 |
| Running couplings | 2 | 0 | 2 | 0 |
| Decay rates & widths | ~22 | 0 | ~22 | 0 |
| Precision QED (g−2, Lamb) | 3 | 0 | 3 | 0 |
| Neutrino absolute masses | 3 | 0 | 0 | 3 |
| Higgs sector | 5 | 2 | 0 | 3 |
| QCD sector (Λ_QCD, σ) | 3 | 1 | 2 | 0 |
| Cosmological parameters | 4 | 0 | 0 | 4 |
| Cross-sections | 3 | 3 | 0 | 0 |
| Structural null-predictions | 4 | 4 | 0 | 0 |
| **TOTAL** | **~162** | **~23** | **~129** | **~10** |

**Headline finding.** Of the ~162 quantities FTD reports for the Standard Model, **~23 are genuine derivations**, **~129 are parametric insertions** (standard formulas with FTD-supplied inputs), and **~10 are imposed or selected** (no formal derivation). This is consistent with the "~50 parametric insertions" claim in `AUDIT_EPISTEMIC_AUDIT.md` when mass/decay/CKM sub-tables are not expanded, and with ~150 when they are.

**What this means for the EFT program.** Phases 1–4 of `SPEC_EFT_RECOVERY_PROGRAM.md` aim to move 5–15 items from the right two columns to the left column by measuring (not fitting) them on the lattice. See §11 for the upgrade-candidates shortlist.

---

## 3 · Foundational Constants — [DERIVED] / [THEOREM]

These are the load-bearing claims. Every catalog entry below that is [PARAMETRIC] consumes one or more of these.

| Quantity | Value | Formula from axioms | Tag | Source |
|---|---|---|---|---|
| G* (lemniscatic constant) | 2.95868… | G* = √2·Γ(1/4)²/(2π), Fourier self-duality of θ₃² | [THEOREM] | `DERIV_GSTAR_FROM_THETA.md` |
| α (fine structure) | 1/137.036 | Master quadratic x_+ root; tree 1.26 ppm | **[STRONGLY MOTIVATED CONJECTURE]** (FTD-0013; downgraded 2026-04-19 per `AUDIT_MASTER_QUADRATIC.md`) | Polynomial layer is [THEOREM]; physical identification x_+ = 1/α conditional on curve + degree + root selections; 7-term expansion is a post-hoc fit to CODATA digits beyond experimental precision (NOT a "<0.001 ppt derivation"). *(Historical: Dual-prediction (x_- ≈ N_c) was the strongest evidence — rigidity scan in `audit_master_quadratic_rigidity.py` showed no other polynomial in 60k scan matched both roots.) **2026-05-22 update:** the `x_-  N_c` identification is **RETIRED** per v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`). The new canonical structural-uniqueness evidence is FTD-0189's adversarial polynomial-template scan (0 non-G\* dual-matchers across 2.65 M polynomials over an 18-constant FTD-undesigned basket; rank 1 by ~130×).* |
| N_c (colors) | 3 | Independently sourced via 4 topological routes converging on N_c = 3 (Moore Layer Theorem; `DERIV_NC_FROM_TOPOLOGY.md`). *(The historical `x_-  N_c` master-quadratic-root reading — `x_- = 3.024`, 0.80% of 3 — is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`.)* | [THEOREM] for topology routes | `DERIV_NC_FROM_TOPOLOGY.md` |
| {N_base, N_eff, b_3} | {4, 13, 7} | Moore-neighborhood integer invariants | [THEOREM] | `AUDIT_SELF_CONSISTENCY.md` |
| G_C (state-flux coupling) | √α | Lattice-QED bare coupling; g_c = √α at Thomson scale | [THEOREM] | `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` |
| sin²θ_W | 3/13 | N_c / N_eff from SU(2)×U(1) Moore-layer decomposition | **[PARAMETRIC]** (downgraded 2026-04-19 per `AUDIT_RATIONAL_FIT_CLAIMS.md`) | 3.53% error vs experimental 0.2229; experimental precision is 20 ppm (1700× tighter than FTD claim). Competitor 2/9 = 0.2222 fits better (0.31% vs 3.5%) with no Moore-neighborhood meaning. `proof_complete_sm.py` §electroweak |
| C_SPEED | 1/√D = 1/√3 | CFL stability on cubic lattice | [THEOREM] | `SPEC_FTD.md` §dynamics |

---

## 4 · Lepton Masses — partial [DERIVED] / [PARAMETRIC] mix

| Quantity | Value | Formula | FTD inputs | Tag | Source |
|---|---|---|---|---|---|
| m_e | 0.511 MeV | m_e = m_P · √(2π) · (16/3) · α¹¹ | α, m_P | **[STRONGLY MOTIVATED CONJECTURE]** (downgraded 2026-04-19 per `AUDIT_RATIONAL_FIT_CLAIMS.md`) | 0.19% error; among 6489 (p/q, n) combinations with p,q ≤ 50, n ∈ [8, 14], FTD's (16/3, n=11) is the tightest within 1% but only 1 of 2 within that band. Prefactor 16/3 and exponent 11 motivated but not dynamically derived. `proof_electron_mass.py` |
| m_μ / m_e | 206.77 | N_c² · N_eff · b_3 + offsets | {N_c, N_eff, b_3} | [DERIVED] | `proof_mass_ratios.py` |
| m_τ / m_e | 3477 | Triangular number formula | {N_c, N_eff} | [DERIVED] | `proof_mass_ratios.py` |
| m_p / m_e | 1836.47 | N_eff/α + N_base·N_eff + N_c | {N_c, N_base, N_eff, α} | **[STRONGLY MOTIVATED CONJECTURE]** (downgraded 2026-04-19 per `AUDIT_RATIONAL_FIT_CLAIMS.md`) | 173 ppm error; 5.8× experimental precision (30 ppm). Uses three Moore integers + α as inputs so harder to dismiss as rational fit, but "derivation" status is overstated — a 1-loop refinement is warranted. `proof_proton_electron_ratio.py` |

These are genuine: each produces both the formula *and* the numerical value from lattice structure.

---

## 5 · Quark Masses — fully [PARAMETRIC]

All six quark masses are integer-combination *fits* using the framework integers {N_c, N_base, N_eff, b_3, α}. The formulas are *chosen to match experiment*, not derived. They belong in this catalog because the reverse-engineering of integer combinations from experimental values is exactly the fishing pattern the project's epistemic-discipline rules prohibit. Until each formula has an independent *structural* justification (e.g., from a Moore-layer decomposition the way lepton masses do), they stay [PARAMETRIC].

| Quantity | Formula | FTD inputs | Source |
|---|---|---|---|
| m_u/m_e | N_base + sin²θ_W = 4 + 3/13 | {N_base, sin²θ_W} | `proof_complete_sm.py:221` |
| m_d/m_e | 2N_base + 1 + α·N_eff | {N_base, α, N_eff} | `proof_complete_sm.py:222` |
| m_s/m_e | N_eff(N_eff+1) + 1 = 183 | {N_eff} | `proof_complete_sm.py:223` |
| m_c/m_e | ≈ 2485 (multi-integer fit) | {3, 4, 7, 13} | `proof_complete_sm.py:224` |
| m_b/m_e | T(127) + 42 = 8170 | Triangular numbers | `proof_complete_sm.py:225` |
| m_t/m_b | N_eff·N_c + 2 = 41 | {N_c, N_eff} | `proof_complete_sm.py:226` |

**Reviewer flag.** These formulas have no independent structural derivation on the lattice as of 2026-04-19. They are valuable as consistency checks (they *do* hit experimental values to within a few percent), but reporting them as "FTD predicts the quark masses" overstates the evidence. Correct reporting: "given the framework integers, integer-combination fits reproduce the six quark masses."

---

## 6 · Hadron Spectroscopy — ~90 items, all [PARAMETRIC]

### 6.1 Mesons (~42 particles)
Formula: m_P² = (m_q + m_q̄) · Λ_QCD³ / f_P² (imported chiral perturbation theory at leading order).
FTD inputs: quark masses from §5, Λ_QCD from §9, decay constants f_P fit per meson family.
Status: [PARAMETRIC]. See `proof_complete_sm.py` mesons block + `AUDIT_EPISTEMIC_AUDIT.md` §II.3.

### 6.2 Baryons (~48 particles)
Formula: m_baryon = Σ m_q + hyperfine + binding; Regge trajectories M² = M₀² + n·a for excited states.
FTD inputs: quark masses, ΔM ≈ Λ_QCD·√N_c.
Status: [PARAMETRIC]. See `AUDIT_EPISTEMIC_AUDIT.md` §II.4.

**Reviewer flag.** The ~90 hadron masses are the largest single category of parametric insertions. None carries independent predictive weight; collectively they constitute a consistency cross-check against the quark-model + Regge-phenomenology parametrization that already fits the spectrum in standard QCD.

---

## 7 · Mixing Angles — mixed [DERIVED] / [PARAMETRIC]

### 7.1 PMNS (neutrino mixing) — [DERIVED]
| Angle | FTD formula | Value | Tag | Source |
|---|---|---|---|---|
| sin²θ₁₂ | N_c / (N_c + b_3) = 3/10 | 0.300 (exp 0.307) | **[STRUCTURALLY MOTIVATED PARAMETRIC]** (downgraded 2026-04-19) | 2.28% error; 4 rational competitors within same tolerance (4/13, 7/23, etc.). `proof_complete_sm.py:229` |
| sin²θ₂₃ | (N_eff + N_c)/(2N_eff + N_c) = 16/29 | 0.552 (exp 0.546) | **[STRUCTURALLY MOTIVATED PARAMETRIC]** (downgraded 2026-04-19) | 1.05% error; 3 competitors within same tolerance (6/11, 11/20, 13/24) including closer fits. `proof_complete_sm.py:230` |
| sin²θ₁₃ | 1/(N_base·N_eff) = 1/52 | 0.0192 (exp 0.0220) | **[PARAMETRIC]** (downgraded 2026-04-19) | **12.6% error, 37× experimental precision**; experimental 0.0220 is closer to 1/45 or 1/46 (neither structurally motivated). Essentially a mis-prediction. `proof_complete_sm.py:231` |
| Δm²₃₁/Δm²₂₁ | (b_3+N_c)²/N_c = 100/3 | 33.3 (exp 32.8) | **[STRUCTURALLY MOTIVATED PARAMETRIC]** (downgraded 2026-04-19) | 1.63% error; simpler 33/1 also fits within 1%. `proof_complete_sm.py:232` |

### 7.2 CKM (quark mixing) — [PARAMETRIC]
The CKM matrix is parametrized via standard Wolfenstein (λ, A, ρ, η) form. FTD supplies λ = sin θ_C ≈ 1/√N_eff + corrections, A, δ = arctan(7/3), but the *parametrization* is imported. Four Wolfenstein elements are [PARAMETRIC]. See `proof_complete_sm.py:270-277`.

---

## 8 · Running Couplings — [PARAMETRIC] (awaiting Phase 2 measurement)

| Quantity | Formula | FTD inputs | Tag | Note |
|---|---|---|---|---|
| α_s(M_Z) | 7/59 | {b_3 = 7} | **[PARAMETRIC]** (downgraded 2026-04-19 per `AUDIT_RATIONAL_FIT_CLAIMS.md`) | 0.63% error, but competitor 2/17 = 0.1176 fits BETTER (0.29% vs 0.63%) with no Moore interpretation. The "59" denominator is not structural. `proof_complete_sm.py:262` |
| α_s(Q²) running | QCD one-loop form α_s(M_Z)/[1 + (b₀α_s/2π)·ln(Q²/M_Z²)] | b₀ = b_3 = 7 | **[PARAMETRIC]** | `src/ontic_running_coupling.cpp` |
| α(Q²) running | Imported QED form | α | **[PARAMETRIC]** | Not currently measured as pure continuum form |
| α_EM running under blocking (2026-04-19) | Measured on L ∈ {32, 64, 128}, three extraction methods | asymptotic α_r on lattice | **[MEASURED]** | `DERIV_BETA_FUNCTION_MEASURED.md`; β_measured / β_QED ≈ −160 (first measurement); refined to ≈ −80 after T3 L=128 slope method |
| Yukawa screening length λ ∝ L (2026-04-19 T3) | λ(L=32)=2.88, λ(L=64)=10.57, λ(L=128)=25.61 → λ ≈ L/5 | finite-size from periodic images | **[MEASURED]** | `DERIV_GAP_CLOSURE.md` T3; proves Phase-2 "Yukawa screening" is a periodic-image artefact, not a physical mass |
| Dynamical EWSB threshold (2026-04-19 T4) | Branch A observed at initial_amp=0.80 on L=16 (⟨\|J\|⟩ triples, 62 charges manifest) | seed amplitude | **[MEASURED]** | `DERIV_GAP_CLOSURE.md` T4; amp-threshold in (0.50, 0.80); first dynamical manifestation in EFT programme |
| Operator divJ² scaling dim (2026-04-19 T5) | Δ(pulse)=0.46, Δ(flux-baryon)=**1.69** | scenario dependence | **[MEASURED]** | `DERIV_GAP_CLOSURE.md` T5; 3.7× shift confirms pulse-envelope artefact of Phase 3 |
| Ward floor (matched stencil, 2026-04-19 Day 2) | max \|∇·J − ρ\| ≤ 1e-8 on deep vacuum via CG Poisson on Yee-staggered differences | lattice + CG tol | **[MEASURED]** | `DERIV_DAY2_CAMPAIGN.md` §1; million-fold improvement over engine SOR |
| EWSB condensation threshold (2026-04-19 Day 2) | First-order phase transition amp ∈ (0.6, 0.7) on L=32, 5000 ticks | amplitude parameter | **[MEASURED]** | `DERIV_DAY2_CAMPAIGN.md` §2; below threshold vacuum decays quietly, above saturates all 32768 voxels |
| Condensate mass gap (2026-04-19 Day 2) | m_flux=0.181, m_charge=0.186 at amp=0.80 (R² ≥ 0.96); ratio 0.97 | amp=0.80 | **[MEASURED]** | `DERIV_DAY2_CAMPAIGN.md` §3; two independent channels agree within 3% |
| Rutherford α cross-check (2026-04-19 Day 2) | α=0.035 at b=3 matches V(r) asymptotic=0.035 exactly; α_mean=0.042±0.005 over b∈[3,8] | v_0=0.3, L=32 | **[MEASURED]** | `DERIV_DAY2_CAMPAIGN.md` §4; independent dynamical method confirms the 5× α_ref gap is genuine engine physics |
| Continuum α_r at r_max (2026-04-19 Day 2 Thread 1a) | α_r(r=84, L=256) = 0.010 → **1.4× α_ref** (lowest measured ratio) | fast-big CPU scan | **[MEASURED]** | `DERIV_DAY2_CAMPAIGN.md` §6b; 3-scale r_max series 0.030 → 0.028 → 0.010 shows convergence toward α_ref with L |
| ~~Continuum α extrapolation via 1/L fit (Day 2)~~ | **RETRACTED** — based on under-equilibrated ticks=100 fast-big CPU data | — | ~~[MEASURED]~~ | Superseded by 4-point Phase-F measurement below |
| ~~Continuum α extrapolation (Phase F, 2026-04-19)~~ | ~~Alleged plateau at 1.8–3.6× α_ref~~ | — | ~~[MEASURED]~~ | **RESOLVED by Phase G**: the measurement is not a coupling constant at all. See next row. |
| Emergent V(r) Green's function match (Phase G, 2026-04-19) | α_r(r,L) = 2·r·G_L(r), zero free parameters, R² = 1.0000 at L=384 in Coulomb tail (median 0.07%, max 0.43%) | cubic lattice, 7-pt Laplacian | **[THEOREM]** | Engine `emergent_forces` mode is unit-charge geometric Coulomb. Gauss law ∇·J = s carries no coupling constant. The "3.6×" plateau was a category error. `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` + `scripts/benchmarks/fit_geometric_coulomb.py`. |

**Phase 2 goal.** Both entries above should be *measured* by real-space blocking (`scripts/benchmarks/measure_beta_function.py`), not imported. If the measured β(g) matches the one-loop form, these upgrade to [DERIVED]. If it deviates, the deviation is a Phase 2 discovery.

---

## 9 · Decay Rates & Widths — all [PARAMETRIC]

~22 quantities total. All use imported Fermi theory, electroweak tree-level formulas, or QED phase-space integrals with FTD-supplied masses and couplings.

| Quantity | Formula (imported) | FTD inputs | Source |
|---|---|---|---|
| τ_μ (muon lifetime) | 192π³ℏ/(G_F²·m_μ⁵) | G_F, m_μ | `proof_complete_sm.py:339` |
| τ_τ (tau lifetime) | τ_μ·(m_μ/m_τ)⁵·BR | m_τ | `proof_complete_sm.py:346` |
| Γ_Z (Z width) | (α/(3 sin²θ_W cos²θ_W))·M_Z·N_gen·χ | α, sin²θ_W, M_Z | `proof_complete_sm.py:296` |
| …and ~19 more | Phase-space + matrix element | Various | `proof_complete_sm.py` decay block |

---

## 10 · Precision QED — all [PARAMETRIC]

| Quantity | Formula (imported) | FTD inputs | Experimental agreement | Source |
|---|---|---|---|---|
| a_e (electron g−2) | 5-loop QED expansion + HVP + EW | α | 2.55 ppb | `proof_complete_sm.py:388` |
| a_μ (muon g−2) | 3-loop QED + HVP + HLBL + EW | α, m_μ, G_F | Standard-Model prediction | `proof_complete_sm.py:402` |
| Lamb shift | One-loop Mohr + Uehling VP | α, m_e, m_p | 0.23% | `proof_complete_sm.py:416` |

**Note on a_e.** The famous "2.55 ppb" result is genuine evidence for α's value (Phase 2 can confirm this from measured α), but the g−2 calculation itself is the standard QED multi-loop expansion. What FTD predicts is the *input* α; what FTD does not predict is the *functional form* of the loop expansion. Report as "FTD's α, plugged into standard QED, matches a_e to 2.55 ppb."

---

## 10b · Atomic Dynamics & Structure — EMPIRICAL / [PARAMETRIC] / ABSENT (FTD-0270)

Full inventory: [`AUDIT_ATOMIC_DYNAMICS_STATUS.md`](AUDIT_ATOMIC_DYNAMICS_STATUS.md). FTD derives ~0% of atomic quantum dynamics; the ingredients below are standard chemistry/QM with FTD constants, empirical lookups, or absent. (Listed here because the catalog previously omitted the atomic domain.)

| Quantity | Origin | FTD status | Source |
|---|---|---|---|
| Orbital clouds (s,p,d,f) | hydrogenic wavefns + Slater screening, applied universally, display-tuned | EMPIRICAL / visualization | `orbitals.js` |
| Slater shielding / Z_eff | Slater's 1930 empirical rules | EMPIRICAL | `quantum-chemistry.js` |
| Atomic radius | `R_BOHR / Z^{1/3}` (Thomas-Fermi) | EMPIRICAL | `atom_engine.h` |
| Periodic table / valence / max-bonds | 118-element hard-coded tables | EMPIRICAL lookup | `elements.js`, `atom_engine.h` |
| Inter-atomic forces (ionic/vdW/covalent) | classical Coulomb / Lennard-Jones / harmonic | [PARAMETRIC] | `atom_forces.cpp` |
| Quantum kinetic operator / ℏ / discrete levels | — | ABSENT / [DECLINED] (FC-1) | — |
| Pauli exclusion (multi-electron) / exchange energy | — | [CONJECTURE] / ABSENT | `phase_forces.cpp:200` (CPU no-op) |
| Fine structure / spin-orbit / hyperfine | — | ABSENT | — |

**Structural reason** (FTD-0270): FTD's flux wave equation is 2nd-order in time → linear dispersion ω∝k (cavity-like), the wrong dispersion for the hydrogen Rydberg 1/n² (which needs the Schrödinger ω∝k²). And the electron is a manifested cluster, not a wavefunction. Atomic spectra are not substrate-derivable.

---

## 11 · Neutrino Absolute Masses — [SELECTION]

| Quantity | Value | Mechanism | Tag | Source |
|---|---|---|---|---|
| m_ν_3 | 49.6 meV | Type-I seesaw: m_D²/M_R with m_D = v·α, M_R = (3/4)v/α⁴ | [SELECTION] | `proof_complete_sm.py:240` |
| m_ν_2 | 8.6 meV | Seesaw hierarchy | [SELECTION] | `proof_complete_sm.py:247` |
| m_ν_1 | 4.1 neV | Seesaw hierarchy | [SELECTION] | `proof_complete_sm.py:248` |

The mass *ratios* (§7.1) are [DERIVED]; the *absolute scale* is set by the seesaw ansatz's choice of m_D = v·α, which is [SELECTION].

---

## 12 · Higgs Sector

| Quantity | Value | Derivation | Tag | Source |
|---|---|---|---|---|
| v (Higgs VEV) | 246.09 GeV | **Imported** — reference input from SM | [IMPOSED] | `constants.py:375` |
| M_H | 124.8 GeV | M_H = (N_eff/α²)·m_e | [SELECTION] (0.24% error; formula argued, not derived) | `proof_complete_sm.py:199` |
| M_W | 80.4 GeV | M_Z · cos θ_W | [DERIVED] given M_Z, θ_W | `proof_complete_sm.py:288` |
| M_Z | 91.1876 GeV | **Imported** | [IMPOSED] | `proof_complete_sm.py:262` |
| G_F | 1/(√2·v²) | [DERIVED] given v | [DERIVED]* | `proof_complete_sm.py:291` |

*Conditional on v, which is [IMPOSED].

---

## 13 · QCD Sector

| Quantity | Formula | FTD inputs | Tag | Source |
|---|---|---|---|---|
| Λ_QCD (1-loop) | M_Z · exp(−2π/(b₀·α_s)) | M_Z, b₀ = 23/3, α_s = 7/59 | [PARAMETRIC] (imported RG) | `proof_complete_sm.py:268` |
| Λ_QCD (2-loop) | 1-loop × exp(0.85) | Imported NLO correction | [PARAMETRIC] | `proof_complete_sm.py:271` |
| σ (string tension) | −ln(x_−/(x_−+1)) | x_− = 3.024 | [DERIVED] | `proof_complete_sm.py:275` |

---

## 14 · Cosmological Parameters — [SELECTION]

| Quantity | Value | Argument | Tag | Source |
|---|---|---|---|---|
| Ω_Λ | 2/3 | Ternary ground-state symmetry | [SELECTION] | `proof_complete_sm.py:448` |
| Ω_matter | 1/3 | Ternary complement | [SELECTION] | `proof_complete_sm.py:449` |
| r (tensor-to-scalar) | N_c · α | Power-law inflation ansatz | [SELECTION] | `proof_complete_sm.py:452` |
| n_s (spectral index) | 1 − 2α | Slow-roll ansatz | [SELECTION] | `proof_complete_sm.py:455` |

The inflation framework is imported; FTD supplies the couplings.

---

## 15 · Cross-Sections — [DERIVED]

| Quantity | Formula | Tag | Source |
|---|---|---|---|
| r_e (classical electron radius) | α·ℏc/m_e | [DERIVED] | `proof_complete_sm.py:361` |
| σ_Thomson | (8π/3)·r_e² | [DERIVED] | `proof_complete_sm.py:364` |
| M_Coulomb (lattice propagator) | −α / (2λ(q)), λ(q) = 2(3−cos q_x−cos q_y−cos q_z) | [THEOREM] | `proof_complete_sm.py:313` |

These follow from QED once α is known and from lattice Feynman rules directly.

---

## 16 · Structural Null-Predictions — [THEOREM]

Absence claims are testable and count as evidence. FTD's ternary + lattice structure *forbids* each of the following:

| Prediction | Mechanism | Tag | Source |
|---|---|---|---|
| τ_proton = ∞ | Charge conservation is exact on lattice (Gauss constraint) | [THEOREM] | `proof_complete_sm.py:467` |
| N_monopole = 0 | div(B) ≡ div(curl J) = 0 identity | [THEOREM] | `proof_complete_sm.py:495` |
| N_SUSY = 0 | Ternary {−1,0,+1} carries no fermionic grading | [THEOREM] | `proof_complete_sm.py:500` |
| Extra dimensions = 0 | \|Aut(E)\|² = 2^D·(D−1)! forces D = 3 | [THEOREM] | `proof_complete_sm.py:503` |

**Experimental status.** Proton decay, monopoles, SUSY partners, and extra dimensions have all been *searched for and not found*, consistent with FTD null-predictions.

---

## 17 · Upgrade Candidates for the EFT Recovery Program

The EFT program (Phases 1–4) can plausibly move these entries from [PARAMETRIC] to [DERIVED]:

| Entry | Current | Target after EFT phase | Phase |
|---|---|---|---|
| α(Q²) running | [PARAMETRIC] (imported QED form) | [DERIVED] (measured β from blocking) | Phase 2 |
| α_s(Q²) running | [PARAMETRIC] (imported QCD form) | [DERIVED] if lattice reproduces β₀ = 7 | Phase 2 |
| σ (string tension) | [DERIVED] (formula only) | [DERIVED] + measured Creutz ratio at multiple lattice sizes | Phase 2 |
| Operator dimensions (~12 ops) | Not catalogued yet | [DERIVED] per operator | Phase 3 |
| Lorentz-invariance anisotropy | [ASSERTED] in theory docs | [DERIVED] with measured residual exponent | Phase 1 |
| Ward identity closure | [ASSERTED] | [DERIVED] to measured precision (target: permille) | Phase 1 |
| v (Higgs VEV) | [IMPOSED] | [DERIVED] if Phase 4A shows dynamical EWSB | Phase 4 |
| W/Z masses | [IMPOSED] reference | [DERIVED] if Phase 4A succeeds | Phase 4 |
| Three-generation count | [THEOREM] topologically, [OPEN] dynamically | [DERIVED] dynamically if Phase 4B cold-start produces 3 | Phase 4 |
| Continuum-limit α | Implicit | [DERIVED] by a → 0 extrapolation | Phase 4 |
| 5 structural ops | — | [DERIVED] scaling dimensions | Phase 3 |
| Continuum Wilson-loop σ | Single-point | [DERIVED] with finite-size scaling | Phase 4 |

**Expected net upgrades:** 5–15 items, depending on how cleanly the measurements work. A successful program reduces the right-hand columns of §2 by that count.

---

## 18 · What This Catalog Does NOT Do

1. **Does not question the standard-physics formulas being used.** Fermi theory, ChPT, seesaw, Regge trajectories are well-established. Plugging FTD values into them is legitimate cross-checking.
2. **Does not propose new [DERIVED] upgrades outside the EFT program.** Any reclassification from [PARAMETRIC] → [DERIVED] requires a proof document, not a catalog edit.
3. **Does not enumerate every meson and baryon individually.** The hadron-spectroscopy block treats these as *categories* (§6) because all ~90 hadron masses use the same imported formula family; individual entries add rows without adding epistemic content.
4. **Does not tag proof_complete_sm.py results that are explicit identities** (e.g., r_e = α·ℏc/m_e) as parametric — these are *definitions* once α and m_e are fixed.

---

## 19 · Maintenance

**This catalog must be updated whenever:**
- A new [PARAMETRIC] entry is added to `proof_complete_sm.py` → add a row.
- An EFT-program phase completes → move affected rows left, update `DERIV_*.md` citation.
- A reviewer challenges a tag → audit in issue tracker, adjust here if warranted.

**Do not update if:** you are merely tweaking numerical precision on an existing row — that belongs in the proof script, not the catalog.

**Cross-references:**
- `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` — narrative discussion of the honest accounting
- `docs/theory/07_assessment/REF_CLAIMS_MATRIX.md` — structured claim-by-claim dependency matrix
- `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` — [OPEN] items, many of which block upgrades here
- `docs/theory/10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md` — the program that aims to reduce the [PARAMETRIC] column
- `scripts/proofs/proof_complete_sm.py` — primary source of row data
- `scripts/constants.py` — framework integers {N_c, N_base, N_eff, b_3, α, G_C, G*}
