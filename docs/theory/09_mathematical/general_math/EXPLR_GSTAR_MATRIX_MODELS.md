# EXPLR — G\* in strongly-coupled matrix models: the CHPS exact solutions as an external construction site for ℚ(G\*)

**Tag:** [SYNTHESIS] + [STRUCTURAL OBSERVATION] — external-literature map plus a machine-verified intersection with the algebraic spine. Introduces no FTD physics claim; promotes no tag.
**LEDGER id:** FTD-0366.
**Verification:** `scripts/proofs/proof_gstar_matrix_models.py` (144/144 PASS, ~80 s; per-check tags `[EXTERNAL]` for re-verified CHPS statements, `[THEOREM]` for spine-side identities) + `scripts/tests/test_gstar_matrix_models.py` (pytest wrapper, 5 tests).
**Opens:** RQ-MM-1/2/3 (TRACKER_OPEN_ITEMS §6.6–6.8; falsifiers in §6 below).
**Audience:** project owner + agents working on the spine's ℚ(G\*) positioning (Theorem 9 / FTD-0112/0127), the modulus/argument frontier, or any future matrix-model / q-deformation exploration.

---

## §0 — Scope (read first)

**This document IS:** a map of one external mathematics paper — Clay Córdova, Ben Heidenreich, Alexandr Popolitov, Shamil Shakirov, *"Orbifolds and Exact Solutions of Strongly-Coupled Matrix Models"*, **Commun. Math. Phys. 361 (2018) 1235–1274**, DOI `10.1007/s00220-017-3072-x`, **arXiv:1611.03142** ("CHPS"; section/equation numbers below refer to arXiv v1, which is the OSTI 1537659 copy) — together with the exactly-verified statements connecting its results to the field ℚ(G\*) at the center of FTD's algebraic spine. The load-bearing observation: the simplest strongly-interacting zero-dimensional QFT — the monomial quartic one-matrix model, S = Tr X⁴ — has observables valued in ℚ(G\*), and CHPS themselves single out Γ(1/4)/Γ(3/4) ≈ 2.958675 as the model's irreducible transcendental (their §3.1, discussion below eq 3.7). This de-orphans G\*: the constant an ontology-first construction forces into centrality is also the natural coupling-space of a mainstream exactly-solvable ensemble.

**This document IS NOT:** a claim that CHPS's work supports, endorses, or bears on FTD's physics. CHPS is a hep-th/math-ph paper about matrix models; it has nothing to say about `x₊ = 1/α` (FTD-0013 [SMC]), the lattice ontology, or any FTD physics identification. **Citing CHPS in FTD work is scholarly attribution for an independent mathematical exhibit — it is not, and must never be presented as, third-party validation of the framework.** This sentence is the load-bearing content of §0. No FTD tag moves on account of anything recorded here; the standing invariants (no α derived anywhere; x₊ = 1/α [SMC]; MC-T4.3 [FOUNDATIONAL OBSTRUCTION]) are untouched.

---

## §1 — The external result (what CHPS prove)

The objects are eigenvalue integrals Z⁽ʳ'ᵃ⁾_N = (1/N!)∫_{C_{r,a}} ∏ᵢ (dxᵢ/2π) ∏ᵢ<ⱼ (xᵢ−xⱼ)² e^{−Σᵢ xᵢʳ} — U(N) one-matrix models with pure monomial potential Tr Xʳ, the infinite-coupling limit of Gaussian-plus-interaction models. Their content, at the tags FTD's vocabulary would assign [all [EXTERNAL] — CHPS's theorems, re-verified here, not FTD results]:

1. **Contour eigenbasis.** The convergent closed contours form an (r−1)-dimensional basis C_{r,a} (a = 1, …, r−1), the eigenbasis of the ℤ_r symmetry X → ω_r X (their §3.2, eq 3.12). Moments on C_{r,a} are single Γ-values: ∫_{C_{r,a}} x^q e^{−xʳ} dx = δ_{r|q+1−a}·Γ((q+1)/r) (eq 3.14). *Verified: check C1, 100-digit ray quadrature.*
2. **Theorem 1 (their eq 3.20).** Z⁽ʳ'ᵃ⁾_N = δ_{r,a}(N)·(2π)^{−N}·∏_{i=0}^{N−1} Γ(⌊i/r⌋+1)·Γ(⌊(i−a)/r⌋+a/r+1), with sign δ_{r,a}(N) ∈ {0,±1} nonzero iff N ≡ 0 or a (mod r). *Verified in full — Andreief moment determinant = δ·(Γ-amplitude), sign included — for r ∈ {3,4}, all a, N ≤ 6 (check C10), plus quadrature cross-checks (C2).*
3. **Theorem 2 (their eq 3.23).** Schur averages are hook-content products with mod-r selection rules; only r-divisible Young diagrams (trivial r-core) contribute, with a ternary-valued r-signature δ_r(λ) ∈ {0,±1}. [Not re-verified here; used only in §6 RQ-MM-3.]
4. **Orbifold factorization (their Theorems 3–6).** The W(Xʳ) model factorizes into r copies of the W(X) model on pure-phase contours; the mechanism is an exact ℤ_r-projection identity for the Vandermonde (their Theorem 6). *Verified: check C9, symbolic, r ∈ {3,4}, including a δ = 0 case.* Log-deformations give an r-fold Selberg/Kadell family (their §4.2); q-deformations (Jackson integrals, Γ_q) preserve the whole structure (their §4.3), with a separate q → ω_r root-of-unity limit connecting q-models to orbifolds.
5. **Non-perturbative underdetermination (their §2.2).** The loop equations (Ward identities) — the model's entire perturbative content — do not fix the model: non-perturbatively the correlators depend on the contour choice, invisible at every order of perturbation theory. For the quartic at rank N there are (N+1)(N+2)/2 independent contours. The strong-coupling (monomial) limit is where the ambiguity resolves into the clean ℤ_r eigenbasis.
6. **Reflection positivity (their §2.3).** The quartic model on the real line is reflection-positive (the ensemble analog of unitarity); every other quartic contour is not. The cubic model is reflection-positive on **no** contour — proven at N = 1 (the operator (Tr X)e^{Tr X²/4} has zero norm), *expected* for N > 1 (CHPS's own scope split, preserved verbatim here).
7. **Large-N shadow (their §3.6).** Contour dependence enters the free energy only at relative O(1/N²) (Barnes-G asymptotics; the aãN term of eq 3.50 — *the finite-N term is verified symbolically in check C6*).

---

## §2 — The verified bridge (FTD-facing content, all machine-checked)

Every row below is an exact statement verified by `proof_gstar_matrix_models.py`; "amplitude" means the Γ-product of Theorem 1 with the δ/(2π)^N prefactor stripped (necessary N-qualification: at N = 1 the a = 3 *partition function* vanishes by the support rule; the amplitude does not).

| # | Statement | Check |
|---|---|---|
| 1 | **Sector grading (r = 4, N ≤ 12):** amplitudes lie in ℚ·Γ(1/4)^N (a=1), ℚ·π^{N/2} (a=2), ℚ·Γ(3/4)^N (a=3) — exactly the three Γ-classes | C3 |
| 2 | **Ratio = G\*-world:** amplitude ratio (4,1)/(4,3) at N=1 equals **G\***; full Z ratios (δ signs included) where both phases are supported: G\*⁴/48 at N=4, 125·G\*⁸/435456 at N=8 — ℚ·G\*^N throughout | C4 |
| 3 | **Product = π-world:** conjugate-sector amplitude product at N=1 equals Γ(1/4)·Γ(3/4) = **√2·π** (Euler reflection at z=1/4; explicitly disambiguated from √(2π)) | C4 |
| 4 | **Race-constant family realized:** the N=1 conjugate-sector amplitude ratio of the Tr Xʳ model equals **R_r = Γ(1/r)/Γ(1−1/r)** for r = 3…8 — the family of `MATH_FAMILY_OF_RACES.md`, with R₄ = G\* [DERIVED — immediate from CHPS Thm 1] | C4 |
| 5 | **Observables in ℚ(G\*):** real-line quartic ⟨Tr X²⟩ = 1/G\* (N=1), (G\*²+4)/(4G\*) (N=2), G\*(G\*²+4)/(4(G\*²−4)) (N=3); membership proven by the two-substitution reduction (reflection + G\* = Γ(1/4)²/(π√2)), values confirmed by 100/30-digit quadrature | C5 |
| 6 | **Rational sector spectra:** single-trace pure-phase correlators are rational in N: r⟨Tr Xʳ⟩ = N², r²⟨Tr X²ʳ⟩ = 2N³ + a(r−a)N (CHPS eq 3.50 cases) | C6 |
| 7 | **The d=−3 world stays separate:** the r=3 (cubic) sectors carry Γ(1/3)/Γ(2/3)-classes — the equianharmonic ratio R₃ ≈ 1.9783, *not* G\* ≈ 2.9587; its reflection product is 2π/√3, not √2π | C7 |
| 8 | **Spine tie-ins** (already [THEOREM] in the spine, pinned for convention): G\* = Γ(1/4)²/(π√2); Watson G\*²/(2π) = Γ(1/4)⁴/(4π³) | C8 |

CHPS's own words for row 1's consequence (their §3.1): after reducing Γ(2k+1)/4-values, "since Γ(1/4)/Γ(3/4) ≃ 2.958675 is transcendental, there are no obvious further simplifications" — **the paper names G\* as the model's irreducible transcendental.** [EXTERNAL]

---

## §3 — The χ₋₄-sector correspondence (stated at its honest ceiling)

The sector algebra is indexed by a ∈ ℤ/4 (the ℤ₄-Fourier index of the contour). The unit residues (ℤ/4)^× = {1, 3} carry the two lemniscatic Γ-factors; the non-unit even residue a = 2 carries the π-class. The Dirichlet character χ₋₄ is precisely the nontrivial character on (ℤ/4)^× — the object that *distinguishes sector 1 from sector 3*:

| sector combination | value | world |
|---|---|---|
| χ₋₄-symmetric (product of the conjugate pair, N=1) | Γ(1/4)·Γ(3/4) = √2·π | π-world (Euler reflection product) |
| χ₋₄-antisymmetric (ratio of the conjugate pair, N=1) | Γ(1/4)/Γ(3/4) = G\* | G\*-world |
| even sector a = 2 | ℚ·π^{N/2} | π-world |

This is the same split as the spine's §9.1 operational reading (FTD-0127: ℚ(G\*) = the field generated by the parity-twist distinguishing even- from odd-parity Dirichlet L-functions of conductor 4) and the same product/ratio dichotomy as `PAPER_RATIO_AND_THE_ARROW.tex` (product P(z)=Γ(z)Γ(1−z) exchange-symmetric → π; ratio R(z)=Γ(z)/Γ(1−z) exchange-antisymmetric → G\*), here realized inside an exactly solved ensemble rather than as a bare Γ-identity. [STRUCTURAL OBSERVATION]

**Honest ceiling.** What is established: the grading, the values, and the character bookkeeping above — exact, machine-verified. What is NOT established: any map from FTD's substrate to this ensemble; any sense in which the ensemble "is" an FTD partition function; any forcing argument selecting r=4 within FTD (the r of the model is the exponent of the potential, chosen by hand — an imported type in FTD's own vocabulary). The correspondence is a structural match of gradings, not a functor. [coherent-interpretation, at most]

---

## §4 — Frontier and type-priority exhibits `[grounded]`

Two features of CHPS §2.2–2.3 are rigorous mainstream instances of structures FTD's foundations layer names:

- **The contour datum is an imported argument-type.** The action plus all Ward identities (the model's lawful "content") underdetermine the ensemble; a discrete phase-sector datum must be chosen, and nothing in the dynamics chooses it. In frontier vocabulary (`FOUND_MODULUS_ARGUMENT_FRONTIER.md` §2): the loop equations and the even sector are modulus-half data; the C_{r,a} choice is the argument-half — and the invariant measuring the odd-sector asymmetry is exactly G\*. `[grounded]` (Caveat: this is a *shape* match — underdetermined forward data + imported selection — in an external formal system; it motivates the frontier reading, it does not prove any FTD claim.)
- **The thermodynamic shadow erases the type distinction.** The imported datum is invisible at leading large-N: contour dependence enters the free energy only at relative O(1/N²) (their §3.6). An observer with access only to the planar/thermodynamic limit cannot see which argument-type was imported. `[grounded]`, same caveat.

Cross-links: `FOUND_TYPE_PRIORITY_PRINCIPLE.md` §2 (external-discipline exhibits) and `FOUND_MODULUS_ARGUMENT_FRONTIER.md` §2 (faces table) carry one-line pointers back to this document.

---

## §5 — Non-bridges (recorded to prevent re-discovery)

Mirroring `REF_GUILLERA_CORPUS_MAP.md` §4: the following *look* like FTD bridges and are not. Recording them is the anti-pattern-matching content of this document (GTCA failure-class F1).

1. **The 48 is not |O_h|.** Z⁽⁴'¹⁾₄/Z⁽⁴'³⁾₄ = G\*⁴/48, and 48 is also FTD's hyperoctahedral order |O_h|. The 48 here is Pochhammer bookkeeping: Γ(5/4)³ = (1/4)³·Γ(1/4)³ and Γ(7/4) = (3/4)·Γ(3/4), so the rational factor is (1/4)³/(3/4) = 1/48 — a shift-factorial product from Theorem 1's recursion, carrying no octahedral symmetry. Identifying the two 48s without a mechanism would be numerology. [STRUCTURAL OBSERVATION — deflationary]
2. **The master quadratic is not in the model.** Because every sector ratio is ℚ·G\*^k, expressions like 16G\*² and 16G\*³ can be trivially *manufactured* from Z-combinations — that is a substitution identity, prohibited by the project's standing Epistemic Discipline. CHPS contains no natural quadratic with the master quadratic's coefficients. The honest claim stops at field level: **observables ∈ ℚ(G\*)** — nothing more.
3. **No α content.** Nothing here touches x₊ = 1/α (FTD-0013 [SMC]) or the MC-T4.3 obstruction. A future session tempted to "extract α from the quartic matrix model" should read this sentence and stop.
4. **Link-8 distinctness.** The orbifold factorization (CHPS Theorems 3–6) is *exact contour algebra*, not a Kadanoff blocking; it neither revives nor contradicts the CLOSED-NEGATIVE Link 8 ("master quadratic as RG-step characteristic polynomial", `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md`). Any future "master quadratic from orbifold recursion" exploration must register as a NEW hypothesis citing that closure explicitly.
5. **The paper's physics applications are not FTD bridges.** CHPS's motivations and applications (5d gauge theories on S⁵, superconformal indices on Lens spaces, AGT/para-Liouville, Witten r-spin classes) are their field's physics. Importing any of it into FTD would be exactly the kind of borrowed-authority move the Number-One Goal's ordering (Ontology > Logic > Math > Physics) exists to prevent. Cite the mathematics; leave the physics where it lives.
6. **The ternary-valued signatures are not FTD's ternary states.** δ_r(λ), δ_{r,a}(N) ∈ {0,±1} is "vanishes or contributes with a sign" — standard determinant bookkeeping, not a ternary ontology. (The genuine mod-3 combinatorial content is RQ-MM-3's subject, at survey level.)

---

## §6 — Research questions (registered [OPEN]; falsifiers here, not in the tracker)

### RQ-MM-1 — Reflection positivity as a d=−4-adjacent discriminator

**Question.** Among monomial ensembles Tr Xʳ (r ≥ 3), the **smallest interacting reflection-positive** one is the quartic — the lemniscatic (d=−4-adjacent) case; the cubic (d=−3-adjacent) is positive on no contour (CHPS §2.3; N=1 proven, N>1 expected). Does this qualitative selection statement connect structurally to the spine's Theorem 3 (FTD-0003: d=−4 unique among imaginary quadratic fields via |μ_K| = |disc K|), or is it parity-selection only (every even r plausibly RP on ℝ, with nothing distinguishing r=4 beyond minimality)? The r=6 branch is the discriminating case and must be analyzed before any strengthening. Wording discipline: "smallest", never "unique".
**Status:** [OPEN]. **Falsifier (of the FTD-relevance reading):** an RP contour for r=3 at some N (kills the discriminator outright); or a proof that the even-r family is uniformly RP with no structural feature distinguishing r=4 (demotes the reading to parity-selection — still χ₋₄-consonant, a strictly weaker claim).

### RQ-MM-2 — q-deformation as the discreteness-native language (GATED)

**Question.** CHPS §4.3 shows the entire structure survives q-deformation (Jackson finite-difference calculus, Γ_q → Γ as q → 1) — the mature mathematical language for "a discrete object whose continuum limit produces the Γ-world", which matches FTD's undefined-boundary/ε-L discipline. Does any FTD-native discrete object (candidates: the finite-L BCC Green's function G^BCC_L(0) of `proof_partition_function_gstar.py`, which converges to Watson I₁ = G\*²/(2π); the Phase-G periodic-lattice Poisson Green's function; finite-L Phase-J partition sums) admit an **exact** q-identity at a q **forced by construction** (determined by L alone, zero continuous and zero discrete-family freedom)?
**Gate (mandatory, expected exit = closure).** q must be an output of a declared structural correspondence, never an input chosen after seeing numbers. Phase A is a doc-only target-selection study (literature scan first; then non-tunability / map-uniqueness / exactness questions per candidate); if no candidate passes, RQ-MM-2 **closes negative with zero evaluations performed** — approximate q-matches are precisely the near-miss class the Epistemic Discipline bans, and a tunable q can approximate anything. If a candidate survives, a hash-locked pre-registration (PREREG + manifest row + git tag) precedes the single evaluation.
**Status:** [OPEN — gated]. **Closure conditions:** gate-out (no admissible target — itself the deliverable); or literature-answered (the identity already exists — recorded as [REFERENCE] attribution); or a pre-registered SUPPORTED/REFUTED/UNDERDETERMINED verdict. Even SUPPORTED promotes nothing: it would be a structural identity, not physics.

### RQ-MM-3 — Ternary partition combinatorics for the ℤ₃/qutrit layer (survey)

**Question.** CHPS Appendix A's mod-r toolkit at r=3 — 3-cores, 3-quotients, abacus diagrams, the 3-signature — is the standard machinery for mod-3 selection rules on partitions. Does any object in FTD's ℤ₃/qutrit layer (`THEOREM_MOORE_LAYER_DECOMPOSITION.md`, `DERIV_NC_FROM_TOPOLOGY.md`, the qutrit ℤ₃-Fourier decomposition) carry a natural partition grading that this toolkit would organize?
**Status:** [OPEN — survey level]. **Expected outcome, stated as the prior:** non-bridge (no current FTD object is partition-graded), recorded in the §5 register if confirmed. **Falsifier of the non-bridge verdict:** an FTD-native object with a genuine partition grading whose mod-3 selection rules match the 3-core/3-quotient structure exactly.

---

## §7 — Citation discipline

- Canonical citation: C. Córdova, B. Heidenreich, A. Popolitov, S. Shakirov, *Orbifolds and Exact Solutions of Strongly-Coupled Matrix Models*, Commun. Math. Phys. **361** (2018) 1235–1274, DOI 10.1007/s00220-017-3072-x, arXiv:1611.03142. Section/equation numbers in FTD documents refer to **arXiv v1** (= the OSTI 1537659 public-access copy); pin the version in any new citation.
- Cite CHPS wherever this document's verified statements are used — mandatory attribution for their Theorems 1–6. Do not cite CHPS to lend FTD legitimacy by association (§0).
- The verification script re-proves what it uses; a reader needs CHPS for the general theorems, not for the r ∈ {3,4}, N ≤ 6 cases checked here.

## §8 — Cross-references

- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` §9/§9.1 — Theorem 9 (ℚ(G\*) π-free subfield; FTD-0112) + the parity-twist operational reading (FTD-0127) this document's §3 realizes in ensemble language.
- `docs/theory/09_mathematical/general_math/MATH_FAMILY_OF_RACES.md` — the race-constant family R_q; §2 row 4 realizes it as monomial-model sector data.
- `docs/theory/09_mathematical/number_theory/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md` — the parity-twist reading and L'(s, χ₋₄) boundary identities.
- `docs/theory/09_mathematical/number_theory/EXPLR_CM_RATIO_TOWER.md` — the 9-element class-number-1 Chowla–Selberg ratio tower (RQ-MM-1's arithmetic side).
- `docs/theory/02_foundations/FOUND_MODULUS_ARGUMENT_FRONTIER.md` §2 + `FOUND_TYPE_PRIORITY_PRINCIPLE.md` §2 — the frontier/type-priority exhibits of §4.
- `docs/papers/src/PAPER_RATIO_AND_THE_ARROW.tex` — the product/ratio dichotomy §3 realizes.
- `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md` — the closed negative §5.4 keeps distinct.
- `docs/theory/09_mathematical/general_math/REF_GUILLERA_CORPUS_MAP.md` — the template this map follows (scope discipline, non-bridges register, citation rules).

---

*This map records adjacent mathematics and its verified intersection with the spine. It moves no FTD claim. The framework's open problem — the physics identification of the master-quadratic roots — is untouched by everything catalogued here.*
