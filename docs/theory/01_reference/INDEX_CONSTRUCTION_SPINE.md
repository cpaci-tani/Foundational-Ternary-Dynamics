# Construction-Spine Navigation Index

**Tag:** `[REFERENCE]`
**Date:** 2026-06-02
**Companion:** [`MONOGRAPH_FTD_CONSTRUCTION.md`](MONOGRAPH_FTD_CONSTRUCTION.md) `[SYNTHESIS]`
**Purpose:** Drill from any monograph claim to its authoritative canonical source. The monograph introduces no new mathematics; authority lives in the documents below. Where this index and any source document disagree on a tag, the source document is correct.

---

## How to use this index

Each table below maps a monograph section to (a) what it constructs or states and (b) the canonical backing documents that carry that content at the appropriate proof/theorem/claim level. Links resolve from the `docs/theory/01_reference/` directory. Sections that cite scripts list the verification artifact alongside the theory document.

---

## Part 0 — The Seed

| Monograph section | What it constructs / states | Backing canonical docs |
|---|---|---|
| §0.1 — The discrete ontology | Five postulates `[AXIOM]`; two-layer ontology (flux J, state s); undefined-boundary lattice commitment | [`../07_assessment/AUDIT_INFINITY_REFRAME.md`](../07_assessment/AUDIT_INFINITY_REFRAME.md) — undefined-boundary triage and foundational commitment |
| §0.2 — What a construction is | Construction standard (derivation / parametric / match); mathematical-primitive ordering | [`SPEC_MATH_FIRST_ONTOLOGY.md`](SPEC_MATH_FIRST_ONTOLOGY.md) (FTD-0153) — math-first ontology; M0/M1/M2/M3 claim-status test |
| §0.3 — The epistemic contract | Full tag system `[AXIOM]`…`[SYNTHESIS]`; promotion/demotion asymmetry rule | [`SPEC_DOCTRINE_LEDGER.md`](SPEC_DOCTRINE_LEDGER.md) §0 (FTD-0145) — tag definitions and ladder rule; [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — per-claim tag registry |
| §0.4 — The two clauses: roadmap | Clause 1 (derive) → Part I; Clause 2 (boundary) → Part II; bridge → Part III | [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) — canonical 5-tier bedrock; [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §0 — nine-result accounting |

---

## Part I — The Constructive Reach

| Monograph section | What it constructs / states | Backing canonical docs |
|---|---|---|
| §I.1 — The seed: `i` and `ℤ[i]` | `[AXIOM]` reading of lattice quarter-turn as ℤ[i]; unit group `ℤ[i]^× ≅ ℤ/4` order 4 `[THEOREM]`; Gaussian integer prime splitting `[THEOREM]`; the integer 4 deposited into the construction | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §1 (Theorem 1, OT-1.2) — G* algebraic identity and ℤ[i] seed |
| §I.2 Route 1 — Γ-ratio `[THEOREM]` (OT-1.2) | G* = Γ(1/4)/Γ(3/4); Euler reflection; `G* = 2ϖ/√π`; canonical value `G* ≈ 2.95868` | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §1; [`../../../scripts/constants.py`](../../../scripts/constants.py) — canonical `G_STAR` value |
| §I.2 Route 2 — Watson BCC period `[THEOREM]` (OT-2.1) | Watson 1939 BCC integral W₃ = G*²/(2π); G* from lattice Green's-function period | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §5 (Theorem 5); [`SPEC_FQCR.md`](SPEC_FQCR.md) Model I (FTD-0141) |
| §I.2 Route 3 — det_ζ quarter-conjugacy `[THEOREM]` (OT-1.7, FTD-0141) | J-twisted ζ-regularized determinant ratio = G*; residue classes mod 4 = split/inert primes of ℤ[i] | [`SPEC_FQCR.md`](SPEC_FQCR.md) Model I; [`../03_derivations/foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md) §5 (OT-1.7) |
| §I.2 Route 4 — finite-N attractor `[THEOREM]` (OT-1.8, FTD-0142) | G*_N := (N+1)^{-1/2}∏(n+3/4)/(n+1/4) → G* at O(1/N²); finitary ε–L discharge | [`SPEC_FQCR.md`](SPEC_FQCR.md) Model II (FTD-0142); [`../03_derivations/foundational_mechanics/DERIV_GSTAR_FINITE_APPROX.md`](../03_derivations/foundational_mechanics/DERIV_GSTAR_FINITE_APPROX.md); verification: [`../../../scripts/proofs/proof_fqcr_convergence.py`](../../../scripts/proofs/proof_fqcr_convergence.py) |
| §I.2 — FTD-0117 warning | G* ≈ 2.95868 ≠ ϖ ≈ 2.62206; never conflate | [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0117; [`../../../scripts/constants.py`](../../../scripts/constants.py) `G_STAR` |
| §I.3 — The master quadratic `[THEOREM]` (OT-1.1, FTD-0001) | P(x) = x²−16G*²x+16G*³; discriminant Δ = 64G*³(4G*−1); roots x₊ = 137.036…, x₋ = 3.024… | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §2 (Theorem 2); [`MATH_MASTER_QUADRATIC.md`](MATH_MASTER_QUADRATIC.md) §6; verification: [`../../../scripts/proofs/proof_master_verification.py`](../../../scripts/proofs/proof_master_verification.py) (54/54 PASS) |
| §I.3 — Coefficient-16 soft spot (OT-4.1, Tier 4) | `|Aut(E)|² = 16` value-level `[THEOREM]`; structural necessity is `[CONJECTURE]` | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §4 (Theorem 4); [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) OT-4.1 |
| §I.3 — No physics in Part I | x₊ = 1/α identification deferred to Part III; x₋  N_c RETIRED (FTD-0014 removed) | [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0013; commit `ca7eb61`; [`../03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md`](../03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md) |
| §I.4 — Harmonic-invariant tower `[THEOREM]` (OT-1.3, FTD-0111) | 1/y₊ + 1/y₋ = 1 at every tower level k ≥ 3; three-line Vieta proof | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §8 (Theorem 8); [`../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md`](../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md); verification: [`../../../scripts/proofs/proof_harmonic_invariant_tower.py`](../../../scripts/proofs/proof_harmonic_invariant_tower.py) |
| §I.4 — CM-curve uniqueness `[THEOREM]` (OT-1.9) | ℚ(i) unique with `|μ_K| = |disc(K)|`; trivial-multiplier criterion load-bearing (FTD-0124) | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §3 (Theorem 3) |
| §I.4 — Q(G*) π-free `[THEOREM]` (OT-2.3, FTD-0112) | Q(G*) ∩ Q(π) = Q; conditional on Chudnovsky 1976 | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §9 (Theorem 9); verification: [`../../../scripts/proofs/proof_field_theoretic_qgstar.py`](../../../scripts/proofs/proof_field_theoretic_qgstar.py) |
| §I.4 — Tower-discriminant transcendence `[THEOREM]` (OT-2.2) | A_k transcendental for k ≥ 4; conditional on Schneider–Chudnovsky | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §8 |
| §I.4 — BCC complex structure `[THEOREM]` (OT-1.5/1.6, FTD-0122) | ℤ[BCC]⊗ℚ = V_triv²⊕V_sign²⊕V_complex²; V_complex ≅ ℤ[i]²; no-go ℤ[i]^× ↛ O_h^ab | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §10.X; verification: [`../../../scripts/proofs/proof_bcc_complex_structure.py`](../../../scripts/proofs/proof_bcc_complex_structure.py) |
| §I.4 — Lemniscatic L-value `[THEOREM]` (OT-2.4, FTD-0159) | L(E_lemn, 1) = ϖ/4 = G*√π/8; conditional on Rubin 1991 (corrected from ϖ/2) | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md); [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0159 |
| §I.4 — χ_{-4} unification, η-tower, Sym²⊕Sym³ uniqueness (OT-2.5, OT-2.6, OT-2.7) | Three further `[THEOREM]`s conditional on Deligne/Chowla–Selberg | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md); [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0163, FTD-0175 |
| §I.4 — Phase-J ultralocality `[THEOREM at L=2]` (OT-3.1) | Ultralocality proved at L=2 only; L=2 degeneracy mechanism; disconfirmed at L≥3 | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §7 (Theorem 7); verification (disconfirmation): [`../../../scripts/proofs/proof_phase_j_general_L.py`](../../../scripts/proofs/proof_phase_j_general_L.py) |
| §I.5 — The construction map | Dependency DAG: axiom → 4 → G* → P(x) → spine theorems | [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §0 (canonical accounting); [`SPEC_MATH_FIRST_ONTOLOGY.md`](SPEC_MATH_FIRST_ONTOLOGY.md) §2 (layer ordering) |

---

## Part II — The Boundary

| Monograph section | What it constructs / states | Backing canonical docs |
|---|---|---|
| §II.1 — The readout problem | Operator assembly `(Tr, Det) = (16G*², 16G*³)` required; W-CRIT-2; MC-T4.3 framing; ARC contract | [`SPEC_ALPHA_READOUT_CONTRACT.md`](SPEC_ALPHA_READOUT_CONTRACT.md) §2–§3 (FTD-0152); [`../07_assessment/audits/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`](../07_assessment/audits/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md) (FTD-0235) |
| §II.2 — The four FTD-native routes | Routes jtwist / bcc / cm / novel; 0/4 force assembly; forward-forced trace and odd source; not-forced: operator gluing | [`../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md) (FTD-0242); verification: [`../../../scripts/proofs/proof_alpha_readout_boundary.py`](../../../scripts/proofs/proof_alpha_readout_boundary.py) |
| §II.3(a) — Flip ruled out `[THEOREM]` | C₃(⟨111⟩) excluded from rank-2 readout; Legs 1–2 machine-checked | [`../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md`](../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md) (FTD-0243); verification: [`../../../scripts/proofs/proof_readout_multE_zero.py`](../../../scripts/proofs/proof_readout_multE_zero.py) (6/6), [`../../../scripts/proofs/proof_det_identity.py`](../../../scripts/proofs/proof_det_identity.py) (7/7) |
| §II.3(b) — Leg 3b closes its scope `[THEOREM]` | C₃-equivariant rank-2 restriction cannot carry `(16G*², 16G*³)`; reality→scalar-i→C₄→O chain | [`../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md`](../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md) (FTD-0243); verification: [`../../../scripts/proofs/proof_readout_reduction_collapse.py`](../../../scripts/proofs/proof_readout_reduction_collapse.py) |
| §II.3(c) — Reduction route-invariant `[THEOREM]` | Q(G*) is the Galois-fixed field of the master quadratic's ℤ/2; every symmetric datum in Q(G*) is blind to which root is 1/α | [`../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md) (FTD-0242); [`../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md`](../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md) (FTD-0243); verification: [`../../../scripts/proofs/proof_obligation_a_independence.py`](../../../scripts/proofs/proof_obligation_a_independence.py) |
| §II.4 — K-BIND `[CLOSED THEOREM-NEGATIVE]` | Irreducible obligation: substrate must natively realize √(G*(4G*−1)); ARC-D1 [CLOSED NEGATIVE]; operator calculus axiomatization closed negative | [`SPEC_ALPHA_READOUT_CONTRACT.md`](SPEC_ALPHA_READOUT_CONTRACT.md) §5D; [`../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md`](../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md) (FTD-0244); [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) OT-5.1 |
| §II.5 — Conditional theorem `[THEOREM]` (FTD-0243) | 𝔉 does not force α unless extended by W that natively realizes √(G*(4G*−1)); `𝔉∪{W}` and `𝔉∪{¬W}` both consistent (explicit witness models) | [`../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md`](../07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md) (FTD-0243) §5 |
| §II.6 — Conclusion: α dynamical not structural | MC-T4.3 remains `[FOUNDATIONAL OBSTRUCTION]`; FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`; N_c = 3 contrast | [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0013; [`SPEC_DOCTRINE_LEDGER.md`](SPEC_DOCTRINE_LEDGER.md) §14; [`../03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md`](../03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md); retracted facade: [`../04_coupling/archive/retracted/DERIV_ALPHA_READOUT_RESOLUTION.md`](../04_coupling/archive/retracted/DERIV_ALPHA_READOUT_RESOLUTION.md) (archived 2026-06-02) |

---

## Part III — The Bridge

| Monograph section | What it constructs / states | Backing canonical docs |
|---|---|---|
| §III.1 — The empirical match | x₊ = 137.036… vs α⁻¹ = 137.035999177(21) CODATA 2022; 1.26 ppm; `[STRONGLY MOTIVATED CONJECTURE]` FTD-0013 (OT-5.1) | [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0013; [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) OT-5.1 |
| §III.2 — Structural-uniqueness evidence | FTD-0189 adversarial look-elsewhere scan: 0 non-G* dual-matchers / 2.65M polynomials; rank 1 by ~130×; Bayes ~4×10⁵:1; Eisenstein-family null; h≥2 null | [`SPEC_PHYSICS_BRIDGE.md`](SPEC_PHYSICS_BRIDGE.md) §3.1 (FTD-0121, OT-3.3); [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0189 |
| §III.3 — Retired and closed-negative | x₋  N_c RETIRED (FTD-0014 removed, commit `ca7eb61`); x₋ physical-ID search `[CLOSED NEGATIVE]` FTD-0210; α-derivation routes all `[CLOSED NEGATIVE]` OT-5.1 | [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0210; [`../10_eft_program/archive/closed_negative/AUDIT_X_MINUS_CLOSED_NEGATIVE.md`](../10_eft_program/archive/closed_negative/AUDIT_X_MINUS_CLOSED_NEGATIVE.md); [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) OT-5.1 |
| §III.4 — What would close the gap | Exit 1: 6th-postulate W forcing operator assembly (K-BIND `[CLOSED THEOREM-NEGATIVE]`); Exit 2: fresh ARC-D measurement (ARC-D1 `[CLOSED NEGATIVE]`) | [`SPEC_ALPHA_READOUT_CONTRACT.md`](SPEC_ALPHA_READOUT_CONTRACT.md) §5D; [`../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md`](../10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md) (FTD-0244); [`../10_eft_program/derivations/DERIV_ALPHA_READOUT_EMPIRICAL.md`](../10_eft_program/derivations/DERIV_ALPHA_READOUT_EMPIRICAL.md) |
| §III.5 — The honest physics scope | ~162 SM quantities: ~23 `[DERIVED]`/`[THEOREM]`, ~129 `[PARAMETRIC]`, ~10 `[IMPOSED]`/`[SELECTION]`; m_e and m_p/m_e at `[STRONGLY MOTIVATED CONJECTURE]`; G_N identification `[CLOSED NEGATIVE]` FTD-0131; dimensionless vs dimensional calibration map | [`../07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`](../07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md); [`SPEC_DIMENSIONAL_MAP.md`](SPEC_DIMENSIONAL_MAP.md); [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0131; [`SPEC_PHYSICS_BRIDGE.md`](SPEC_PHYSICS_BRIDGE.md) |

---

## Coda — The map in both directions

| Monograph section | What it constructs / states | Backing canonical docs |
|---|---|---|
| Coda — Summary | Both findings together: seven theorem-grade forced results + route-invariant α boundary; FTD as philosophy-of-mathematics project; dimensionless ratios are the falsifiable spine; Born-rule layer `[SELECTION]`/`[OPEN]` | [`../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md`](../07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) (all tiers); [`SPEC_ALGEBRAIC_SPINE.md`](SPEC_ALGEBRAIC_SPINE.md) §0; [`SPEC_PHYSICS_BRIDGE.md`](SPEC_PHYSICS_BRIDGE.md); [`SPEC_DOCTRINE_LEDGER.md`](SPEC_DOCTRINE_LEDGER.md) — single-page status map |
