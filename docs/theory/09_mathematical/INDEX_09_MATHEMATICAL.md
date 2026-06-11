# INDEX · Number Theory & Mathematical Connections

**Tag:** [REFERENCE]
**Date:** 2026-05-22
**Status:** [REFERENCE] — local navigation index for `docs/theory/09_mathematical/`.
**Purpose:** This cluster is FTD's pure-mathematics layer: the number-theoretic and arithmetic-geometric structures behind G\*, the master quadratic, and the framework integers {3, 4, 7, 13}. It covers the algebraic-spine math (L-values, CM elliptic curves, Watson identities), the structural-uniqueness scans that underpin the central α conjecture, the Clifford/bivector algebra program for fermion emergence, cross-domain explorations (Fourcier curves, Cayley-Dickson tower, von Neumann factors), and the FQCR observer-test suite. Read it when you need a math result FTD cites, want to know what α-derivation routes are exhausted, or need to place a claim's epistemic tag.

---

## Read first

A newcomer to this cluster should read these in order:

1. [PROOF_ALPHA_FROM_SELF_DUALITY.md](general_math/PROOF_ALPHA_FROM_SELF_DUALITY.md) — α⁻¹ = 137.036 from one CM elliptic curve; the spine's flagship chain.
2. [DERIV_LFUNCTION_GSTAR_CONNECTION.md](number_theory/DERIV_LFUNCTION_GSTAR_CONNECTION.md) — G\* = 8·L(E,1)/√π; ties the framework to BSD-level number theory.
3. [EXPLR_PATHS_TO_ALPHA.md](general_math/EXPLR_PATHS_TO_ALPHA.md) — honest exhaustive survey: no derivation of α exists beyond the [SMC] spine route.
4. [EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md](general_math/EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md) — the master quadratic is the unique dual-matcher in its polynomial family (target pair `(1/α, N_c)` reflects the pre-v1.4 framing; `x_-  N_c` retired per v1.4 §5 — LEDGER FTD-0014 removed in commit `ca7eb61` — but the polynomial-template-uniqueness fact is independent of the target identification).
5. [MATH_FAMILY_OF_RACES.md](general_math/MATH_FAMILY_OF_RACES.md) — the Γ-ratio family G\* belongs to; structural context for q = 4.
6. [EXPLR_NUMBER_THEORY.md](general_math/EXPLR_NUMBER_THEORY.md) — where the framework integers {3, 4, 7, 13} come from across pure mathematics.

---

## The algebraic spine: G\*, L-values, CM curves

The number-theoretic core. These establish the identities the framework's central claims rest on.

| File | Tag | Purpose |
|---|---|---|
| [PROOF_ALPHA_FROM_SELF_DUALITY.md](general_math/PROOF_ALPHA_FROM_SELF_DUALITY.md) | [THEOREM] (steps 1-6) + [AXIOM] (step 7) | α⁻¹ from the CM elliptic curve E: y² = x³ − x via the master quadratic. |
| [DERIV_LFUNCTION_GSTAR_CONNECTION.md](number_theory/DERIV_LFUNCTION_GSTAR_CONNECTION.md) | [THEOREM] + number-theoretic context | G\* = 8·L(E,1)/√π; coefficient 16 = |E(Q)_tors|² in the BSD formula. |
| [MATH_LOG_GSTAR_IDENTITY.md](number_theory/MATH_LOG_GSTAR_IDENTITY.md) | [THEOREM] | log G\* expansion absorbing all unsolved L-values; verified to 80+ digits. |
| [MATH_ANTI_CORRELATION_THEOREM.md](general_math/MATH_ANTI_CORRELATION_THEOREM.md) | [THEOREM] | Why ζ(s) and β(s) alternate in π-reducibility at integer arguments. |
| [MATH_FAMILY_OF_RACES.md](general_math/MATH_FAMILY_OF_RACES.md) | [THEOREM] + [SELECTION] (q=4 interp.) | The q-th race constant R_q = Γ(1/q)/Γ(1−1/q); R_4 = G\*. |
| [THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md](general_math/THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md) | [THEOREM] (identity) + [CONJECTURE] (interp.) | Exact identity linking the BCC Green's function to the reflection ratio. |
| [EXPLR_HIGHER_DIM_WATSON.md](general_math/EXPLR_HIGHER_DIM_WATSON.md) | [THEOREM] | Generalised Watson identity for dimension D ≥ 3 (G\* paper Theorem 13.2). |
| [DERIV_MASTER_QUADRATIC_CM_LVALUES.md](number_theory/DERIV_MASTER_QUADRATIC_CM_LVALUES.md) | [THEOREM] + [COROLLARY] | Master-quadratic coefficients as Deligne L-values; 16G\*² = 2⁹·L(Sym²E,1). |
| [DERIV_MASTER_QUADRATIC_FROM_PERIOD_ALGEBRA.md](number_theory/DERIV_MASTER_QUADRATIC_FROM_PERIOD_ALGEBRA.md) | [THEOREM] | Proves the algebraic derivation of the master quadratic from motivic symmetric period algebra invariants. |
| [DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md](general_math/DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md) | [THEOREM] + [SELECTION] | Derivation of the Pythagorean-Fermat bridge and the master quadratic from number-theoretic boundary constraints. |
| [EXPLR_MODULAR_QUADRATIC.md](general_math/EXPLR_MODULAR_QUADRATIC.md) | [THEOREM] + [SELECTION] | Is the master quadratic a modular equation? Definitive answers (no; L(E,1) appears). |
| [EXPLR_CM_RATIO_TOWER.md](number_theory/EXPLR_CM_RATIO_TOWER.md) | [REFERENCE] / [EXPLORATORY MATH] | The 9-element tower of class-number-1 Chowla-Selberg ratios; only d=−4 anchors physics. |
| [EXPLR_CHOWLA_SELBERG_HIGHER_H.md](number_theory/EXPLR_CHOWLA_SELBERG_HIGHER_H.md) | [THEORY NOTE — literature synthesis] | Analytic machinery to extend Theorem 3 to class number h ≥ 2. |
| [REF_GUILLERA_CORPUS_MAP.md](general_math/REF_GUILLERA_CORPUS_MAP.md) | [REFERENCE] / external-literature map | Guillera's Ramanujan-type-series corpus mapped to the spine; scholarly attribution only. |
| [DERIV_JONES_INDEX_THRESHOLD_RATIO.md](general_math/DERIV_JONES_INDEX_THRESHOLD_RATIO.md) | [CONJECTURE] | Jones Index and manifestation threshold ratio: KB/KC = 4sqrt(2) derived from the hyperfinite Type III subfactor inclusion Jones Index [N:M] = 32. |


## The central α conjecture: CM identification & structural-uniqueness scans

What underpins `x₊ = 1/α` — the conjecture itself plus the scans that establish its rigidity.

| File | Tag | Purpose |
|---|---|---|
| [CONJ_ALPHA_FROM_CM.md](general_math/CONJ_ALPHA_FROM_CM.md) | [CONJECTURE] | The statement: 1/α = x₊, the larger master-quadratic root from CM arithmetic. |
| [CONJ_SEVEN_TERM_PRECISION_SERIES.md](general_math/CONJ_SEVEN_TERM_PRECISION_SERIES.md) | [CONJECTURE] | The 7-term series for 1/α; 24-digit agreement, observationally underdetermined. |
| [EXPLR_PATHS_TO_ALPHA.md](general_math/EXPLR_PATHS_TO_ALPHA.md) | [SURVEY] | Exhaustive survey of α-derivation routes; honest "no new path" verdict. |
| [EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md](general_math/EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md) | [STRUCTURAL OBSERVATION] | 147k-polynomial scan: the master quadratic is the unique dual-matcher under the historical target pair `(1/α, N_c)`; polynomial-template-uniqueness fact independent of target. |
| [EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md](number_theory/EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md) | [STRUCTURAL OBSERVATION] | 58-pair scan: (m=2, k=4) is uniquely close to 1/α in the (1+i)-tower family. |
| [EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md](number_theory/EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md) | [THEOREM] (§3.1) + [STRUCTURAL OBSERVATION] + [CLARIFICATION] | **Theorem GE-1** (§3.1): $(2,4)$ is the unique $a^b=b^a$ ($a\neq b$) integer solution, so $16=2^4=4^2$ is the canonical Gaussian coefficient (proof via $(\ln x)/x$ monotonicity). Surrounding: no Eisenstein twin ($3^6\neq6^2$); G\*³'s odd exponent = D=3 over three Z[i] planes (organized by C₃/⟨111⟩), not Z[ω]; reframes the MC-T4.3 trace/determinant odd-term gap. FTD-0237. |
| [EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md](number_theory/EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md) | [EMERGENT] (negative result) | The tree-level-to-CODATA gap is NOT in the simple Q-span of tested L-values. |
| [EXPLR_TOPOLOGICAL_DRAG_ALPHA.md](general_math/EXPLR_TOPOLOGICAL_DRAG_ALPHA.md) | [CONJECTURE] + [THEOREM] (tautology) | Audit of a topological-drag α "derivation"; shown to be a tautology. |
| [EXPLR_ALPHA_OVER_42_MASS_GAP.md](general_math/EXPLR_ALPHA_OVER_42_MASS_GAP.md) | [CONJECTURE] | α/42 as a candidate 174-ppm proton/electron mass-ratio correction; numerical only. |

## Master-quadratic & lemniscatic structural readings

Interpretive re-framings of the master quadratic and the recurring integer 4 / 16.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md](general_math/EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md) | [STRUCTURAL OBSERVATION] | Consolidated: volumetric, 2×2 mixing-matrix, and conjugate-lattice readings. |
| [EXPLR_3X3_MIXING_NEGATIVE.md](general_math/EXPLR_3X3_MIXING_NEGATIVE.md) | [STRUCTURAL OBSERVATION — NEGATIVE] | The 2×2 mixing reading does not extend to 3×3; FTD's mode count is specifically 2. |
| [DERIV_INTEGER_4_UNIFICATION.md](number_theory/DERIV_INTEGER_4_UNIFICATION.md) | [THEOREM] + [DERIVED] | Lemniscatic catalogue of 4's; the |μ_K| = |disc(K)| uniqueness; three-class classification. |
| [DERIV_CONJECTURE_16_5_2_CLOSURE.md](general_math/DERIV_CONJECTURE_16_5_2_CLOSURE.md) | [DERIVED] | Sym^a residual conjecture reduces (in six steps) to Paper A Theorem 17.5. |
| [DERIV_BCC_COMPLEX_STRUCTURE.md](general_math/DERIV_BCC_COMPLEX_STRUCTURE.md) | [DERIVED] | BCC complex-structure theorem; partial dual-4 unification (Roles 1+3 derived, 2+4 no-go). |
| [EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md](number_theory/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md) | [SELECTION] / [SYNTHESIS] / [DERIVED] | Consolidated: dimensional, theta-nullwert, and parity-twist readings of G\*. |
| [EXPLR_ONTIC_CONSTANT_ATLAS.md](number_theory/EXPLR_ONTIC_CONSTANT_ATLAS.md) | [THEOREM] (identities) + [SELECTION] | Atlas of every constant in the ontic derivation chain; G\*=3 fixed-point analysis. |
| [EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md](general_math/EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md) | theorems vs conjectures distinguished | Rigorous analysis of the FTD curve family converging on {3, 4, 7, 13}. |
| [EXPLR_COLOR_EXCESS_CLOSED_FORM.md](general_math/EXPLR_COLOR_EXCESS_CLOSED_FORM.md) | [THEOREM] / [CLOSED NEGATIVE] | High-precision PSLQ and algebraic closed-form analysis of the color excess delta_c. |

## Clifford / bivector algebra & fermion-emergence program

The Walsh-Hadamard / Cl(3,0) / Dirac-Kähler thread testing whether fermions emerge natively.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md](algebra/EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md) | [CONJECTURE] + [THEOREM] no-go | Walsh-Hadamard grading of the b=2 block vs. Cl(3,0); audits a fermion-emergence claim. |
| [DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md](algebra/DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md) | [THEOREM] (no-go) + [CONJECTURE] | Full multiplication tables: the 2³-block algebra is not Clifford. |
| [DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md](number_theory/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md) | [THEOREM] + [MEASURED] | Mode-erasure theorem for state-field readout; spin-field partial algebra. |
| [DERIV_LINK_BILINEAR_CLIFFORD_PARTIAL.md](algebra/DERIV_LINK_BILINEAR_CLIFFORD_PARTIAL.md) | [MEASURED] | Program F: first non-commutative algebra in FTD native dynamics (link-bilinear probe). |
| [DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md](algebra/DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md) | [MEASURED] (per-stage) | Consolidated Cl(3,0) multigrade campaign (F-prime / F-double-prime / Path 1). |
| [DERIV_DIRAC_KAHLER_IDENTIFICATION.md](algebra/DERIV_DIRAC_KAHLER_IDENTIFICATION.md) | [STRUCTURAL ID] + [THEOREM NEGATIVE] | FTD's 4-grade structure is a Dirac-Kähler field; Cl(3,0) cannot give mass ratios. |

## FQCR & observer-operator program

Quarter-conjugacy recurrences and the observer-term tests against QED running.

| File | Tag | Purpose |
|---|---|---|
| [REF_QCR_TRILOGY_BRIDGE.md](general_math/REF_QCR_TRILOGY_BRIDGE.md) | [REFERENCE] / [STRUCTURAL CORRESPONDENCE] | The external QCR trilogy mapped to FQCR; cross-confirmations without tag inflation. |
| [EXPLR_FQCR_OBSERVER_TESTS_SUITE.md](fqcr_program/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md) | [EXPLORATORY] | Consolidated 4-test suite: FQCR Model V vs QED running coupling. |
| [PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md](general_math/PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md) | [PROPOSAL / SKETCH] | Summary observer extension that would advance FTD-0013. |
| [PREREG_SYM_K_C_INVARIANT_PARITY_V1.md](general_math/PREREG_SYM_K_C_INVARIANT_PARITY_V1.md) | [PRE-REGISTRATION] | Hash-locked eigenline-parity hypothesis for Sym^k(H¹(E_lemn)). |
| [EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md](number_theory/EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md) | [EXPLR] | Working conventions for the symmetric period algebra of E_lemn. |
| [DERIV_FQCR_EM_CONNECTED_RESPONSE.md](fqcr_program/DERIV_FQCR_EM_CONNECTED_RESPONSE.md) | [THEOREM] + [SELECTION] | Tree-level limit of the FQCR connected response $R_{\text{conn}} \to 1$ and recovery of the bedrock master quadratic root $x_+$. |


## Cross-domain explorations

Fourcier curves, the division-algebra tower, von Neumann factors, and other cross-domain bridges.

| File | Tag | Purpose |
|---|---|---|
| [EXPLR_NUMBER_THEORY.md](general_math/EXPLR_NUMBER_THEORY.md) | verified connections (some open) | The {3, 4, 7, 13} integers across modular forms, number theory, elliptic curves; the 42 nexus. |
| [EXPLR_RIEMANN_ZETA_CONNECTION.md](number_theory/EXPLR_RIEMANN_ZETA_CONNECTION.md) | connections real but limited | Honest audit of seven claimed FTD–Riemann-zeta connections; most "derivations" are fits. |
| [EXPLR_CAYLEY_DICKSON_FOURCIER_ISOMORPHISM.md](algebra/EXPLR_CAYLEY_DICKSON_FOURCIER_ISOMORPHISM.md) | computationally verified | The Fourcier curve's {1,2,4,8,16} frequencies are the Cayley-Jackson dimensions. |
| [EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md](algebra/EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md) | [THEOREM] / [SELECTION] / [CONJECTURE] | Counter-rotation, lobe genesis, and the trefoil bridge of the Fourcier curve. |
| [EXPLR_HALF_MOBIUS_LEMNISCATE.md](general_math/EXPLR_HALF_MOBIUS_LEMNISCATE.md) | [THEOREM] / [SELECTION] / [CONJECTURE] | Z₄ topology from period lattice to molecular orbitals; discriminant trichotomy. |
| [EXPLR_FOURIER_CURVE_LEVEL_4.md](general_math/EXPLR_FOURIER_CURVE_LEVEL_4.md) | [EXPLORATORY] | Fourier curve at level 4: triple-cusp structure and class divisibility. |
| [EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md](general_math/EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md) | [EMERGENT] / [CONJECTURE] | The Born rule visualised as a Joukowski transform (circle → lemniscate). |
| [EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md](general_math/EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md) | [EMERGENT] (honest negative) | 2D-FFT gauge-group selection diagnosed as a square-grid artifact, not physics. |
| [EXPLR_RELU_TYPE_TRANSITION.md](general_math/EXPLR_RELU_TYPE_TRANSITION.md) | formal exploration | The Softplus β interpolates between von Neumann factor types (III → I). |
| [EXPLR_COLLAPSE_GRAVITY_BRIDGE.md](general_math/EXPLR_COLLAPSE_GRAVITY_BRIDGE.md) | formal exploration | Hawking temperature links collapse to curvature via the Softplus β parameter. |
| [EXPLR_EULER_RATIO_RICCI_FLOW.md](number_theory/EXPLR_EULER_RATIO_RICCI_FLOW.md) | [CONJECTURE] | The Euler reflection ratio, Gaussian flow, and the arrow of time. |

---

## Archive

`archive/` holds closed-negative / superseded material — cite only, do not act on:

- [DERIV_TIER_B_CLOSED_NEGATIVE.md](archive/DERIV_TIER_B_CLOSED_NEGATIVE.md) — [CLOSED NEGATIVE]: G\* opus Tier B targets T-B1/T-B2; the engine `N_base = 4` is a numerical coincidence, not a structural bridge to the Q(i)-arithmetic 4.
- [EXPLR_TWO_PI_GSTAR_CONNECTION.md](archive/EXPLR_TWO_PI_GSTAR_CONNECTION.md) — [CLOSED NEGATIVE]: the proposed structural connection between 1/(2π) and G\*, falsified by the Q4a measurement.

---

56 active docs in this cluster (+ 2 archived).
