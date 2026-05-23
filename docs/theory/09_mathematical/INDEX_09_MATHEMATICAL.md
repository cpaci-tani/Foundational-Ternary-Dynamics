# INDEX · Number Theory & Mathematical Connections

**Tag:** [REFERENCE]
**Date:** 2026-05-22
**Status:** [REFERENCE] — local navigation index for `docs/theory/09_mathematical/`.
**Purpose:** This cluster is FTD's pure-mathematics layer: the number-theoretic and arithmetic-geometric structures behind G\*, the master quadratic, and the framework integers {3, 4, 7, 13}. It covers the algebraic-spine math (L-values, CM elliptic curves, Watson identities), the structural-uniqueness scans that underpin the central α conjecture, the Clifford/bivector algebra program for fermion emergence, cross-domain explorations (Fourcier curves, Cayley-Dickson tower, von Neumann factors), and the FQCR observer-test suite. Read it when you need a math result FTD cites, want to know what α-derivation routes are exhausted, or need to place a claim's epistemic tag.

---

## Read first

A newcomer to this cluster should read these in order:

1. [PROOF_ALPHA_FROM_SELF_DUALITY.md](PROOF_ALPHA_FROM_SELF_DUALITY.md) — α⁻¹ = 137.036 from one CM elliptic curve; the spine's flagship chain.
2. [DERIV_LFUNCTION_GSTAR_CONNECTION.md](DERIV_LFUNCTION_GSTAR_CONNECTION.md) — G\* = 8·L(E,1)/√π; ties the framework to BSD-level number theory.
3. [EXPLR_PATHS_TO_ALPHA.md](EXPLR_PATHS_TO_ALPHA.md) — honest exhaustive survey: no derivation of α exists beyond the [SMC] spine route.
4. [EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md](EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md) — the master quadratic is the unique dual-matcher in its polynomial family (target pair `(1/α, N_c)` reflects the pre-v1.4 framing; `x_- ↔ N_c` retired per v1.4 §5 — LEDGER FTD-0014 removed in commit `ca7eb61` — but the polynomial-template-uniqueness fact is independent of the target identification).
5. [MATH_FAMILY_OF_RACES.md](MATH_FAMILY_OF_RACES.md) — the Γ-ratio family G\* belongs to; structural context for q = 4.
6. [EXPLR_NUMBER_THEORY.md](EXPLR_NUMBER_THEORY.md) — where the framework integers {3, 4, 7, 13} come from across pure mathematics.

---

## The algebraic spine: G\*, L-values, CM curves

The number-theoretic core. These establish the identities the framework's central claims rest on.

| File | Tag | Purpose |
|---|---|---|
| [PROOF_ALPHA_FROM_SELF_DUALITY.md](PROOF_ALPHA_FROM_SELF_DUALITY.md) | [THEOREM] (steps 1-6) + [AXIOM] (step 7) | α⁻¹ from the CM elliptic curve E: y² = x³ − x via the master quadratic. |
| [DERIV_LFUNCTION_GSTAR_CONNECTION.md](DERIV_LFUNCTION_GSTAR_CONNECTION.md) | [THEOREM] + number-theoretic context | G\* = 8·L(E,1)/√π; coefficient 16 = |E(Q)_tors|² in the BSD formula. |
| [MATH_LOG_GSTAR_IDENTITY.md](MATH_LOG_GSTAR_IDENTITY.md) | [THEOREM] | log G\* expansion absorbing all unsolved L-values; verified to 80+ digits. |
| [MATH_ANTI_CORRELATION_THEOREM.md](MATH_ANTI_CORRELATION_THEOREM.md) | [THEOREM] | Why ζ(s) and β(s) alternate in π-reducibility at integer arguments. |
| [MATH_FAMILY_OF_RACES.md](MATH_FAMILY_OF_RACES.md) | [THEOREM] + [SELECTION] (q=4 interp.) | The q-th race constant R_q = Γ(1/q)/Γ(1−1/q); R_4 = G\*. |
| [THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md](THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md) | [THEOREM] (identity) + [CONJECTURE] (interp.) | Exact identity linking the BCC Green's function to the reflection ratio. |
| [EXPLR_HIGHER_DIM_WATSON.md](EXPLR_HIGHER_DIM_WATSON.md) | [THEOREM] | Generalised Watson identity for dimension D ≥ 3 (G\* paper Theorem 13.2). |
| [DERIV_MASTER_QUADRATIC_CM_LVALUES.md](DERIV_MASTER_QUADRATIC_CM_LVALUES.md) | [THEOREM] + [COROLLARY] | Master-quadratic coefficients as Deligne L-values; 16G\*² = 2⁹·L(Sym²E,1). |
| [EXPLR_MODULAR_QUADRATIC.md](EXPLR_MODULAR_QUADRATIC.md) | [THEOREM] + [SELECTION] | Is the master quadratic a modular equation? Definitive answers (no; L(E,1) appears). |
| [EXPLR_CM_RATIO_TOWER.md](EXPLR_CM_RATIO_TOWER.md) | [REFERENCE] / [EXPLORATORY MATH] | The 9-element tower of class-number-1 Chowla-Selberg ratios; only d=−4 anchors physics. |
| [EXPLR_CHOWLA_SELBERG_HIGHER_H.md](EXPLR_CHOWLA_SELBERG_HIGHER_H.md) | [THEORY NOTE — literature synthesis] | Analytic machinery to extend Theorem 3 to class number h ≥ 2. |
| [REF_GUILLERA_CORPUS_MAP.md](REF_GUILLERA_CORPUS_MAP.md) | [REFERENCE] / external-literature map | Guillera's Ramanujan-type-series corpus mapped to the spine; scholarly attribution only. |

## The central α conjecture: CM identification & structural-uniqueness scans

What underpins `x₊ = 1/α` — the conjecture itself plus the scans that establish its rigidity.

| File | Tag | Purpose |
|---|---|---|
| [CONJ_ALPHA_FROM_CM.md](CONJ_ALPHA_FROM_CM.md) | [CONJECTURE] | The statement: 1/α = x₊, the larger master-quadratic root from CM arithmetic. |
| [CONJ_SEVEN_TERM_PRECISION_SERIES.md](CONJ_SEVEN_TERM_PRECISION_SERIES.md) | [CONJECTURE] | The 7-term series for 1/α; 24-digit agreement, observationally underdetermined. |
| [EXPLR_PATHS_TO_ALPHA.md](EXPLR_PATHS_TO_ALPHA.md) | [SURVEY] | Exhaustive survey of α-derivation routes; honest "no new path" verdict. |
| [EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md](EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md) | [STRUCTURAL OBSERVATION] | 147k-polynomial scan: the master quadratic is the unique dual-matcher under the historical target pair `(1/α, N_c)`; polynomial-template-uniqueness fact independent of target. |
| [EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md) | [STRUCTURAL OBSERVATION] | 58-pair scan: (m=2, k=4) is uniquely close to 1/α in the (1+i)-tower family. |
| [EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md](EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md) | [EMERGENT] (negative result) | The tree-level-to-CODATA gap is NOT in the simple Q-span of tested L-values. |
| [EXPLR_TOPOLOGICAL_DRAG_ALPHA.md](EXPLR_TOPOLOGICAL_DRAG_ALPHA.md) | [CONJECTURE] + [THEOREM] (tautology) | Audit of a topological-drag α "derivation"; shown to be a tautology. |
| [EXPLR_ALPHA_OVER_42_MASS_GAP.md](EXPLR_ALPHA_OVER_42_MASS_GAP.md) | [CONJECTURE] | α/42 as a candidate 174-ppm proton/electron mass-ratio correction; numerical only. |

## Master-quadratic & lemniscatic structural readings

Interpretive re-framings of the master quadratic and the recurring integer 4 / 16.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md](EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md) | [STRUCTURAL OBSERVATION] | Consolidated: volumetric, 2×2 mixing-matrix, and conjugate-lattice readings. |
| [EXPLR_3X3_MIXING_NEGATIVE.md](EXPLR_3X3_MIXING_NEGATIVE.md) | [STRUCTURAL OBSERVATION — NEGATIVE] | The 2×2 mixing reading does not extend to 3×3; FTD's mode count is specifically 2. |
| [DERIV_INTEGER_4_UNIFICATION.md](DERIV_INTEGER_4_UNIFICATION.md) | [THEOREM] + [DERIVED] | Lemniscatic catalogue of 4's; the |μ_K| = |disc(K)| uniqueness; three-class classification. |
| [DERIV_CONJECTURE_16_5_2_CLOSURE.md](DERIV_CONJECTURE_16_5_2_CLOSURE.md) | [DERIVED] | Sym^a residual conjecture reduces (in six steps) to Paper A Theorem 17.5. |
| [DERIV_BCC_COMPLEX_STRUCTURE.md](DERIV_BCC_COMPLEX_STRUCTURE.md) | [DERIVED] | BCC complex-structure theorem; partial dual-4 unification (Roles 1+3 derived, 2+4 no-go). |
| [EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md](EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md) | [SELECTION] / [SYNTHESIS] / [DERIVED] | Consolidated: dimensional, theta-nullwert, and parity-twist readings of G\*. |
| [EXPLR_ONTIC_CONSTANT_ATLAS.md](EXPLR_ONTIC_CONSTANT_ATLAS.md) | [THEOREM] (identities) + [SELECTION] | Atlas of every constant in the ontic derivation chain; G\*=3 fixed-point analysis. |
| [EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md](EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md) | theorems vs conjectures distinguished | Rigorous analysis of the FTD curve family converging on {3, 4, 7, 13}. |

## Clifford / bivector algebra & fermion-emergence program

The Walsh-Hadamard / Cl(3,0) / Dirac-Kähler thread testing whether fermions emerge natively.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md](EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md) | [CONJECTURE] + [THEOREM] no-go | Walsh-Hadamard grading of the b=2 block vs. Cl(3,0); audits a fermion-emergence claim. |
| [DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md](DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md) | [THEOREM] (no-go) + [CONJECTURE] | Full multiplication tables: the 2³-block algebra is not Clifford. |
| [DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md](DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md) | [THEOREM] + [MEASURED] | Mode-erasure theorem for state-field readout; spin-field partial algebra. |
| [DERIV_LINK_BILINEAR_CLIFFORD_PARTIAL.md](DERIV_LINK_BILINEAR_CLIFFORD_PARTIAL.md) | [MEASURED] | Program F: first non-commutative algebra in FTD native dynamics (link-bilinear probe). |
| [DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md](DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md) | [MEASURED] (per-stage) | Consolidated Cl(3,0) multigrade campaign (F-prime / F-double-prime / Path 1). |
| [DERIV_DIRAC_KAHLER_IDENTIFICATION.md](DERIV_DIRAC_KAHLER_IDENTIFICATION.md) | [STRUCTURAL ID] + [THEOREM NEGATIVE] | FTD's 4-grade structure is a Dirac-Kähler field; Cl(3,0) cannot give mass ratios. |

## FQCR & observer-operator program

Quarter-conjugacy recurrences and the observer-term tests against QED running.

| File | Tag | Purpose |
|---|---|---|
| [REF_QCR_TRILOGY_BRIDGE.md](REF_QCR_TRILOGY_BRIDGE.md) | [REFERENCE] / [STRUCTURAL CORRESPONDENCE] | The external QCR trilogy mapped to FQCR; cross-confirmations without tag inflation. |
| [EXPLR_FQCR_OBSERVER_TESTS_SUITE.md](EXPLR_FQCR_OBSERVER_TESTS_SUITE.md) | [EXPLORATORY] | Consolidated 4-test suite: FQCR Model V vs QED running coupling. |
| [PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md](PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md) | [PROPOSAL / SKETCH] | Sketch of the C1 observer-operator extension that would advance FTD-0013. |
| [PREREG_SYM_K_C_INVARIANT_PARITY_V1.md](PREREG_SYM_K_C_INVARIANT_PARITY_V1.md) | [PRE-REGISTRATION] | Hash-locked eigenline-parity hypothesis for Sym^k(H¹(E_lemn)). |
| [EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md](EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md) | [EXPLR] | Working conventions for the symmetric period algebra of E_lemn. |

## Cross-domain explorations

Fourcier curves, the division-algebra tower, von Neumann factors, and other cross-domain bridges.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_NUMBER_THEORY.md](EXPLR_NUMBER_THEORY.md) | verified connections (some open) | The {3, 4, 7, 13} integers across modular forms, number theory, elliptic curves; the 42 nexus. |
| [EXPLR_RIEMANN_ZETA_CONNECTION.md](EXPLR_RIEMANN_ZETA_CONNECTION.md) | connections real but limited | Honest audit of seven claimed FTD–Riemann-zeta connections; most "derivations" are fits. |
| [EXPLR_CAYLEY_DICKSON_FOURCIER_ISOMORPHISM.md](EXPLR_CAYLEY_DICKSON_FOURCIER_ISOMORPHISM.md) | computationally verified | The Fourcier curve's {1,2,4,8,16} frequencies are the Cayley-Dickson algebra dimensions. |
| [EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md](EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md) | [THEOREM] / [SELECTION] / [CONJECTURE] | Counter-rotation, lobe genesis, and the trefoil bridge of the Fourcier curve. |
| [EXPLR_HALF_MOBIUS_LEMNISCATE.md](EXPLR_HALF_MOBIUS_LEMNISCATE.md) | [THEOREM] / [SELECTION] / [CONJECTURE] | Z₄ topology from period lattice to molecular orbitals; discriminant trichotomy. |
| [EXPLR_FOURIER_CURVE_LEVEL_4.md](EXPLR_FOURIER_CURVE_LEVEL_4.md) | [EXPLORATORY] | Fourier curve at level 4: triple-cusp structure and class divisibility. |
| [EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md](EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md) | [EMERGENT] / [CONJECTURE] | The Born rule visualised as a Joukowski transform (circle → lemniscate). |
| [EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md](EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md) | [EMERGENT] (honest negative) | 2D-FFT gauge-group selection diagnosed as a square-grid artifact, not physics. |
| [EXPLR_RELU_TYPE_TRANSITION.md](EXPLR_RELU_TYPE_TRANSITION.md) | formal exploration | The Softplus β interpolates between von Neumann factor types (III → I). |
| [EXPLR_COLLAPSE_GRAVITY_BRIDGE.md](EXPLR_COLLAPSE_GRAVITY_BRIDGE.md) | formal exploration | Hawking temperature links collapse to curvature via the Softplus β parameter. |
| [EXPLR_EULER_RATIO_RICCI_FLOW.md](EXPLR_EULER_RATIO_RICCI_FLOW.md) | [CONJECTURE] | The Euler reflection ratio, Gaussian flow, and the arrow of time. |

---

## Archive

`archive/` holds closed-negative / superseded material — cite only, do not act on:

- [DERIV_TIER_B_CLOSED_NEGATIVE.md](archive/DERIV_TIER_B_CLOSED_NEGATIVE.md) — [CLOSED NEGATIVE]: G\* opus Tier B targets T-B1/T-B2; the engine `N_base = 4` is a numerical coincidence, not a structural bridge to the Q(i)-arithmetic 4.
- [EXPLR_TWO_PI_GSTAR_CONNECTION.md](archive/EXPLR_TWO_PI_GSTAR_CONNECTION.md) — [CLOSED NEGATIVE]: the proposed structural connection between 1/(2π) and G\*, falsified by the Q4a measurement.

---

51 active docs in this cluster (+ 2 archived).
