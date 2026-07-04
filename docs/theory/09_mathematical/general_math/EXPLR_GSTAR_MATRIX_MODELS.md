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
7. **Reflection positivity does not re-derive the d=−4 selection** (RQ-MM-1 verdict, 2026-07-04). RP-admissibility among monomial ensembles is a **parity** statement (odd r excluded by an exact-form zero-norm operator on every contour at N=1; every even r manifestly RP on ℝ — r=6 verified alongside r=4), so the quartic is only the *smallest interacting* positive case. The spine's d=−4 uniqueness (FTD-0003, |μ_K| = |disc K|) is a different mechanism entirely; same endpoint, no bridge. Say "smallest", never "unique". [STRUCTURAL OBSERVATION — deflationary; checks C11a–e]
8. **The 3-quotient triple is not FTD's C₃ triple** (RQ-MM-3 verdict, 2026-07-04). CHPS's 3-quotients grade *partitions* (S_n/GL representation theory); FTD's ℤ₃ structures index *axes, center classes, shells, and characters* (point-group geometry, Lie topology, abelian Fourier). The corpus carries no partition-graded object for the toolkit to organize; "ℤ₃ indexes a triple" is shape-resonance, not a bridge. Toolkit shelved for any future symmetric-function layer. [STRUCTURAL OBSERVATION — survey level]

---

## §6 — Research questions (registered [OPEN]; falsifiers here, not in the tracker)

### RQ-MM-1 — Reflection positivity as a d=−4-adjacent discriminator — **ANSWERED 2026-07-04: parity-selection + minimality; the d=−4-uniqueness analogy is a NON-BRIDGE**

**Question (as registered).** Among monomial ensembles Tr Xʳ (r ≥ 3), the smallest interacting reflection-positive one is the quartic; the cubic is positive on no contour (CHPS §2.3; N=1 proven, N>1 expected). Does this connect structurally to the spine's Theorem 3 (FTD-0003: d=−4 unique among imaginary quadratic fields via |μ_K| = |disc K|), or is it parity-selection only?

**Verdict: parity-selection only, with "4" entering as minimality — the analogy to FTD-0003's *uniqueness* is not established and is recorded in §5 as a non-bridge.** [STRUCTURAL OBSERVATION] The r=6 branch (the pre-declared discriminating case) settles it. Verified content (checks C11a–e, 155/155):

- **Odd r (every contour, N=1): RP fails, provably.** From the moment formula, the (r−1)-th moment vanishes in *every* pure phase (δ_{r|r−a} = 0 for a = 1…r−1) — equivalently x^{r−1}e^{−xʳ} = −(1/r)·d(e^{−xʳ})/dx is an exact form, integrating to zero on every closed contour. Since r−1 is even for odd r, O = x^{(r−1)/2} is a nonzero operator of zero norm: reflection positivity fails on every admissible contour at N=1 — for r = 3, 5, 7 alike [verified symbolic; extends CHPS's λ-deformed argument (their eq 2.23, also verified) to the pure monomial]. N > 1 remains CHPS's "expected" — scope preserved.
- **Even r (real line, all N): RP holds, trivially.** The measure Δ(x)²·∏e^{−xᵢʳ}dxᵢ is positive on ℝ^N for *every* even r — verified concretely via 7×7 Hankel positivity for r = 4 AND r = 6 [C11d]. Nothing distinguishes r=4 within the even family beyond being the smallest interacting case (r=2 is free).
- **The CHPS Gram bound reproduced:** the λ-deformed cubic at N=1 admits a PSD Gram completion of {1, x, x²} iff |λ| < 2^{−1/2}·3^{−7/4}, reproduced by semidefinite-feasibility bisection to 1e-6 relative [C11c].
- **Even r off the real line: still not RP** (x has zero norm on C_{4,1} since the second moment vanishes there) [C11e] — positivity selects the *real-line completion*, not the sector basis.

**Why this is a non-bridge to FTD-0003.** The RP discriminator's mechanism is mod-2 (parity of r) plus minimality; the spine's d=−4 selection mechanism is the unit-group/discriminant coincidence |μ_K| = |disc K| — a genuine uniqueness among all nine class-number-1 imaginary quadratic fields, not a parity statement. The two criteria land on the same (lemniscatic) endpoint by different mechanisms, and no functor between them was found. Coincidence-of-endpoints, recorded as §5 item 7. **Falsifier of the non-bridge verdict:** an actual mechanism connecting evenness-plus-minimality to the unit-group criterion (e.g., a structure making |ℤ[i]^×| = 4 the *reason* the minimal interacting even exponent is RP-distinguished). Residual open scrap (not tracked): extending the odd-r zero-norm obstruction to a proof at all N.

### RQ-MM-2 — q-deformation as the discreteness-native language — **CLOSED 2026-07-04 AT GATE G-A: no admissible target; zero evaluations performed**

**Question (as registered).** CHPS §4.3 shows the entire structure survives q-deformation (Jackson finite-difference calculus, Γ_q → Γ as q → 1). Does any FTD-native discrete object (candidates: the finite-L BCC Green's function G^BCC_L(0) of `proof_partition_function_gstar.py`, → Watson I₁ = G\*²/(2π); the Phase-G periodic-lattice Poisson Green's function; finite-L Phase-J partition sums) admit an **exact** q-identity at a q **forced by construction**?

**Gate protocol ran as designed; the expected exit occurred.** The gate required q to be an output of a declared structural correspondence, never an input chosen after seeing numbers, with a doc-only Phase A (literature scan, then non-tunability / map-uniqueness / exactness per candidate) and closure-without-evaluation if no candidate passes. Record:

- **A0 (literature scan, 2026-07-04, two queries).** No q-analog of Watson integrals or of finite-L lattice Green's functions exists in the literature; the lattice-Green's-function corpus (Watson, van Peype, Guttmann's all-dimensions program) lives in elliptic-integral/hypergeometric language, and the finite trigonometric sums the finite-L objects reduce to are evaluated by roots-of-unity/contour methods — a different toolkit from Γ_q. Not literature-answered.
- **Q-a (non-tunability): FAILS for all three candidates, for a structural reason.** The Jackson/q-calculus grid is *geometric* (multiplicative: points q^k accumulating at 0); the lattice torus grid is *arithmetic* (uniform: momenta 2πk/L). No construction-forced map exists between the two grid geometries. The one structurally-motivated root-of-unity assignment, q = e^{2πi/L}, collides with the meaning of the CHPS q → ω_r limit — there the root-of-unity **order is the orbifold/sector rank r (a symmetry index)**, not a volume cutoff; identifying FTD's L (system size) with r (sector count) would be a category error of exactly the §5 non-bridge type.
- **Q-b (map uniqueness): FAILS.** At least two inequivalent "natural" q-maps (q = e^{2πi/L}; q = e^{−c/L} with c a free constant) and no forcing argument between them — tunability by another name.
- **Q-c (exactness): FAILS.** No exact identity candidate exists to freeze: the finite-L objects are finite sums of rational functions of roots of unity (algebraic at each L; e.g. W^BCC at L=2 is exactly 1/4), while Γ_q at |q| < 1 is an infinite q-Pochhammer product — no candidate equality presents itself for any L-family. For the Phase-G object specifically, the finite-L structure is *already fully captured with zero free parameters* by the periodic-lattice Poisson Green's function closed form (R² = 1.0000 at L=384); there is no residual finite-L content for a q-form to explain — a q-rewrite would be redundant re-parameterization at best.

**Verdict: CLOSED — gate-out at G-A, zero q-evaluations performed.** [STRUCTURAL OBSERVATION — the closure is the deliverable] The positive residue is the structural reason itself: *the q-world's discreteness is multiplicative and its root-of-unity limit indexes symmetry sectors; FTD's discreteness is uniform-toroidal and its L indexes volume* — the two discretenesses are not the same mathematical kind, so "q-deform FTD's finite-L objects" was a category temptation, not a research program. **Re-open condition (not a tracker item):** a *derivation* — not a choice — of a multiplicative structure on an FTD-native object (e.g., an exact self-similar L → 2L relation with geometric weights) would create the missing forced q-map and would re-enter through a fresh pre-registration per the original gate design.

### RQ-MM-3 — Ternary partition combinatorics for the ℤ₃/qutrit layer — **ANSWERED 2026-07-04: NON-BRIDGE (the declared prior confirmed at survey level)**

**Question (as registered).** CHPS Appendix A's mod-r toolkit at r=3 — 3-cores, 3-quotients, abacus diagrams, the 3-signature — is the standard machinery for mod-3 selection rules on partitions. Does any object in FTD's ℤ₃/qutrit layer carry a natural partition grading that this toolkit would organize?

**Verdict: no — the two mod-3 structures live in different categories, and no FTD object is partition-graded.** [STRUCTURAL OBSERVATION — survey level] The mapping table:

| mod-3 structure | what its "3" indexes | category |
|---|---|---|
| CHPS 3-core | the obstruction class to removing length-3 rim hooks from a Young diagram | S_n / GL(N) representation theory (partitions) |
| CHPS 3-quotient | a ℤ₃-indexed triple of independent sub-partitions (λ ↔ core + (λ⁽⁰⁾,λ⁽¹⁾,λ⁽²⁾)) — the combinatorial shadow of the ℤ₃-orbifold factorization (their Thm 4) | same |
| FTD C₃ ⊂ O_h | the 3 face-diagonal planes of the cuboctahedron / ⟨111⟩ axis rotations (`DERIV_NC_FROM_TOPOLOGY.md`; NCT-3 [SELECTION] identifies it with Z(SU(3))) | finite point-group geometry |
| FTD π₁(SU(3)/ℤ₃) = ℤ₃ | winding/center classes | Lie-group topology |
| FTD Moore shells | binomial 3-cube counts C(3,k)·2^k = 6/12/8 (octahedron / cuboctahedron / stella octangula) | lattice geometry |
| FTD ternary states | the state alphabet {−1, 0, +1} with its ℤ₃-Fourier characters | abelian character theory |

Corpus-wide scan (2026-07-04): outside this program's own files, the corpus contains **zero** occurrences of Young diagrams, Schur functions, 3-cores, rim hooks, or abacus structures — FTD has no symmetric-function or Fock-graded layer for the toolkit to organize. The one genuine shape-resonance — "ℤ₃ indexes a triple of independent sub-objects" (CHPS 3-quotients ↔ the corpus's three-ℤ[i]-planes-under-C₃ readings, e.g. FTD-0237) — is a resonance of shape with no object carrying both structures, which is precisely what §5 exists to deflate. Recorded as §5 item 8.

**Toolkit pointer (the survey's positive residue):** if FTD ever constructs a multiparticle / second-quantized layer with symmetric-function observables (e.g., cluster statistics as power sums), CHPS Appendix A is the ready-made mod-3 selection machinery — a tool on the shelf, not a bridge. **Falsifier of the non-bridge verdict:** an FTD-native object with a genuine partition grading whose mod-3 selection rules match the 3-core/3-quotient structure exactly.

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
