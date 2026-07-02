# ANALYSIS — Closure of the generator-representativeness flag: the sector no-go and the permanent classification (FTD-0358)

**Tag:** `[THEOREM — relative to Definitions 1–2]` (the Moore–sector trichotomy, the hull-boundedness dichotomy, the FC-0-locked constructor closure; **conditional on Chudnovsky 1976** exactly as spine Theorem 9 / FTD-0112 / the repaired FTD-0244 / FTD-0353) + `[CONDITIONAL THEOREM]` (the sector-neutral K-BIND extension, additionally conditional on an **OPEN** joint-independence conjecture, named in §6) + `[ASSESSMENT — literature status]` (the undecidability-at-current-mathematics clauses) + `[SELECTION — declared]` (the permanent classification of the constructor basis, §7).
**LEDGER id:** FTD-0358 (row owned by the controller; **this document does not edit `LEDGER.md`, `META_INDEX.md`, any tracker, or the spine**).
**Closes:** the FTD-0244 generator-representativeness flag (raised by the FTD-0347 provisional specialist review, finding 4; standing in `FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md` §1/§5 and inherited by `THEOREM_VALUATION_4GSTAR_MINUS_1.md` §2.3) — **by permanent classification**, per the pre-stated FTD-0358 protocol under which routes (a), (b), and (c) all close the item.
**Verification artifact:** `scripts/proofs/proof_construction_class.py` — **43/43 PASS** (mpmath dps 25–400, sympy symbolic layer; 2D-reduced Watson quadratures verified against classical closed forms at ≥ 18 digits; PSLQ with positive controls and the FTD-0351/0353 spurious-PSLQ dps rule). SHA256 `4d5ef387e110…`. Read-only pure mathematics — **golden gate untouched** (`0xb604d81a3d79366e`).
**Depends on:** FTD-0244 (repaired, FTD-0351), FTD-0353 (valuation theorem + hull + radical towers), FTD-0314 (carrier narrowing, C1–C3), FTD-0341 (analytic-orientation carriers), FTD-0243 §5 (k = 1 non-forcing — untouched), FTD-0254 §1.2 (**FC-0**, the ℤ[i] reading, `[AXIOM]`-class, honesty-corrected per FTD-0249), FTD-0050/0079 (engine stencil BCC-orthogonality), Chudnovsky 1976; classical inputs: Watson 1939 (the three cubic-lattice integrals), Glasser–Zucker 1977 (SC closed form), Chowla–Selberg, Gauss multiplication.
**Precedence:** LEDGER > `SPEC_FTD_FRAMEWORK_V1.md` (constitution) > this doc.

---

## 0 · Verdict

> **Route taken: (b) NO-GO, proven at the stated grade, discharging into (c) permanent declaration — with route (a) salvaged sector-locked.** The admissible construction class **cannot be characterized sector-neutrally with hull-boundedness**: any class drawn from the five postulates alone contains, on equal postulate footing, analytic outputs in **three distinct CM sectors** (d = −4, d = −3, disc = −24), and confining them all to the FTD-0353 hull Ñ = ℚ̄(π^{1/4}, √G\*) is equivalent to Γ-function containments that contradict the standard conjectural landscape and are open either way beyond Chudnovsky 1976 / Nesterenko 1996. Every hull-bounded class containing the documented generator set therefore imports **exactly one bit**: the selection of the d = −4 datum — and that bit is **FC-0**, an `[AXIOM]`-class commitment the constitution already declares. Locked at FC-0, the five constructor families **do** close into the radical hull, upgrading FTD-0353's inventory-completeness premise from a 14-row list `[SELECTION]` to a **constructor-closure characterization** `[THEOREM given FC-0 + the declared basis, conditional on Chudnovsky 1976]`.

The FTD-0347 flag ("whether the axiomatized generators are *representative* of what the substrate can construct is a question for a working Galois/transcendence specialist") is hereby converted into a permanent three-part classification:

1. **Representative of the FC-0 sector — YES**, now at constructor-closure grade (§5), not list grade.
2. **Representative of the postulate-forced totality — PROVABLY NOT** (§4): the totality contains d ≠ −4 sectors that the hull cannot absorb, and this is a theorem, not a suspicion.
3. **The gap between 1 and 2 is one already-declared axiom (FC-0)**, plus one `[SELECTION — declared]` residue (the constructor basis itself, §7), with a single stated falsifier.

**Nothing promoted.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; FC-W (FTD-0315) stays an adopted `[AXIOM]`-class commitment; FC-0 stays a declared modelling choice — this document *locates* work on FC-0's shoulders, it does not derive FC-0; **no α is derived anywhere**. K-BIND's universal negative is **not weakened** (§6): the no-go kills *hull-boundedness of the sector-neutral class* (route (a)'s target), not δ-unreachability, which extends to the sector-neutral class conditional on a named open conjecture.

---

## 1 · The flag, precisely

FTD-0347 (provisional specialist review, finding 4, `MEDIUM / PLAUSIBLE — OPEN`): K-BIND's `[THEOREM]` closes a generator set 𝒮 "whose members are each chosen with invariants in ℚ(G\*)[, π] — Lemma 1 is near-tautological relative to that choice… Whether the axiomatized generators are *representative* of what the substrate can construct is a question for a working Galois/transcendence specialist, not resolvable by another documentation pass." FTD-0353 §2.3 inherited the same flag for its inventory-completeness premise: "the documented native tower cannot reach δ, not: no native tower can."

The flag has two readings, and the closure must answer both:

- **(R1) Breadth:** is there a native construction *outside* 𝒮 whose output escapes ℚ(G\*, π) / the hull Ñ?
- **(R2) Tautology:** is the confinement of 𝒮's invariants a discovery about the substrate, or an artifact of drawing 𝒮 exactly as wide as the confinement?

This document answers R1 **affirmatively and constructively** (§3: yes — the FCC and SC Watson self-energies, the C₃-character determinants, the ℚ(√−3) θ-nulls; all as postulate-native as anything in 𝒮), and answers R2 by **locating the drawing rule** (§4: 𝒮 is exactly the FC-0 sector of the postulate-forced class — the "choice" the reviewer smelled is FC-0, which the constitution declares as an axiom-class commitment and does not pretend to derive). What survives is not a vague flag but a classification with one named conditionality and one named falsifier.

---

## 2 · Definitions

**Definition 1 (constructor basis ℬ — the abstract class of the FTD-0358 protocol).** The admissible analytic constructor families, drawn from the five postulates (P1–P5) + FC-0:

| Family | Content | Postulate source |
|---|---|---|
| (T) | lattice translations and point-group (O_h) actions | P1 + P4 |
| (S) | the stencil algebra: finite ℚ-linear combinations and compositions of translations; structure functions = ℚ-polynomials in (cos k_x, cos k_y, cos k_z) | P1 + P4 |
| (Z) | ζ-regularized determinants of character-twisted first-order difference operators D_a (the FTD-0234 family), with twist characters drawn from finite cyclic subgroups of the point group, and ℚ(G\*, π)-rescalings thereof | P1 + P4 (+ FC-0 for *which* cyclic subgroup) |
| (Θ) | θ/η/AGM special values at a CM point of the lattice's symmetry reading | FC-0 (the reading names the point) |
| (W) | Watson-type lattice integrals: the self-energy G_λ(0) = (1/π³)∫_{[0,π]³} d³k /(1 − λ(k)) of a normalized Moore-layer structure function λ | P4 |

**Definition 2 (sector-neutrality).** A class 𝒜 built on ℬ is **sector-neutral** if whenever it admits a constructor instance at one postulate-forced symmetry datum, it admits the same constructor at every O_h-conjugate or postulate-equal datum: all three Moore layers for (W); all maximal-cyclic-axis classes of the point group for (Z); every CM point of an available symmetry reading for (Θ). The rationale is P4 itself: the 26 Moore neighbours are *one* causal set; the postulates supply the layers and axes symmetrically, and a restriction to one orbit requires a reason not found in P1–P5.

Note the internal seam in the FTD-0358 protocol's own listing: families (Z) and (Θ) arrive pre-restricted ("*J*-twisted", "at *the* CM point" — both FC-0 data), while (W) arrives generic ("Watson-type lattice integrals"). That seam is not sloppiness; it is the flag made visible, and §4 proves it cannot be smoothed away.

---

## 3 · Theorem 1 — the Moore–sector trichotomy

**Theorem 1.** `[THEOREM — combinatorial part machine-verified; closed forms classical (Watson 1939; Glasser–Zucker 1977), machine-verified at ≥ 18 digits by independent quadrature]`

1. **(Trichotomy is forced.)** P4's Moore neighborhood partitions into exactly three O_h-orbits — SC (6 face neighbours), FCC (12 edge), BCC (8 corner) — with normalized structure functions λ_SC = (c_x+c_y+c_z)/3, λ_FCC = (c_xc_y+c_yc_z+c_zc_x)/3, λ_BCC = c_xc_yc_z. The BCC orbit is the **unique** one whose structure function is multiplicative (rank-1 separable). *(Checks A1–A4.)*
2. **(Both symmetry readings are available.)** O_h contains both order-4 axes (⟨100⟩, the C₄/ℤ[i] reading FC-0 adopts) and order-3 axes (⟨111⟩, the C₃/ℤ[ω] Eisenstein reading FC-0 declines). *(Check A5.)*
3. **(The three layers evaluate into three CM sectors.)** The layer self-energies, each computed by independent 2D-reduced quadrature against its classical closed form *(checks B1–B5)*:

| Layer | G_λ(0) closed form | Value | CM sector | Hull status |
|---|---|---|---|---|
| BCC | Γ(1/4)⁴/(4π³) = G\*²/(2π) = s⁴/(2w⁴) | 1.393203929… | d = −4, h = 1 | **∈ Ñ** `[THEOREM]` |
| FCC | 9 Γ(1/3)⁶/(2^{14/3} π⁴) | 1.344661183… | d = −3, h = 1 | ∉ Ñ *expected*; **open either way** (§4) |
| SC | √6 · Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)/(32π³) | 1.516386059… | disc −24, h = 2 | same status, one field further out |

4. **(The point-group version, same split.)** The character-twisted determinants det_ζ(D_a) = √(2π)/Γ(a) (Lerch; *checks C1–C2*): C₄ characters (a ∈ ¼ℤ) give the Γ(1/4)-line — hull monomials *(check C3)*; C₃ characters (a ∈ ⅓ℤ) give √(2π)/Γ(1/3) — the d = −3 line; the order-6 improper axis closes into the same line by Gauss duplication, Γ(1/6) = 2^{−1/3}√(3/π) Γ(1/3)² *(checks C6, D2)*.
5. **(The θ-constructor version, same split.)** θ₃ at the ℤ[i] CM point τ = i is the hull monomial 2^{−1/4}s w^{−1} *(check D3)*; θ₃ at the ℚ(√−3) CM point τ = i√3 is the classical d = −3 singular value, θ₃(0, i√3)² = 3^{1/4}Γ(1/3)³/(2^{4/3}π²) — a ℚ̄-monomial over {π, Γ(1/3)} *(checks D4a–D4b)* with **no** monomial form over the d = −4 basket {π, Γ(1/4), 2, 3} *(check D5, evidence-grade)*.

One sentence of synthesis `[SYNTHESIS]`: **three independent constructor families — layer self-energies, character determinants, θ-nulls — undergo the *same* d = −4 / d = −3 split at the *same* fork (order-4 vs order-3 symmetry data), and the documented corpus sits entirely on the d = −4 branch of all three.** That coherence is FC-0 acting; it is not an accident of the inventory.

---

## 4 · Theorem 2 — the sector-neutral no-go (route (a) closed)

**Theorem 2.** `[THEOREM relative to Definitions 1–2, conditional on Chudnovsky 1976 for the field model]` + `[ASSESSMENT — literature status]` *(clause (ii))*. Let 𝒜 be any sector-neutral admissible class containing the documented generator set 𝒮 (FTD-0244 §1). Then:

**(i) 𝒜 contains d = −3 outputs.** 𝒮's generator 3 is the BCC Watson self-energy — a layer-specific instance of the layer-uniform constructor (W); 𝒮's generator 4 is the C₄-character determinant — an axis-specific instance of the axis-uniform constructor (Z). Sector-neutrality (Definition 2) then admits W_FCC = 9Γ(1/3)⁶/(2^{14/3}π⁴) and det_ζ(D_{1/3}) = √(2π)/Γ(1/3) as outputs. *(This step is definitional bookkeeping; the values are Theorem 1.)*

**(ii) Hull-boundedness of 𝒜 is unprovable-or-false.** Since W_FCC's prefactor is a hull unit *(check E1)*, W_FCC ∈ Ñ ⟺ Γ(1/3)⁶ ∈ Ñ, which forces Γ(1/3) to be algebraic over ℚ(π, Γ(1/4)), i.e. trdeg_ℚ ℚ(π, Γ(1/4), Γ(1/3)) ≤ 2. The status of that statement:

- **Expected FALSE.** The standard expectation (Lang's conjecture on Γ-values at rationals; the Grothendieck-period-conjecture family) gives trdeg = 3: the only known ℚ̄-multiplicative relations among Γ-values at rationals are the reflection/multiplication relations, which act *within* each sector (reflection closes the Γ(1/4)–Γ(3/4) pair; duplication closes Γ(1/6) into Γ(1/3)) and never across the d = −4 / d = −3 divide. PSLQ corroborates at height 10⁶, dps 400, with both positive controls firing *(checks D1, D2, D6 — evidence, not proof)*.
- **Provable NEITHER WAY at current published mathematics.** Chudnovsky 1976 gives the algebraic independence of the *pairs* {π, Γ(1/4)} and {π, Γ(1/3)}; Nesterenko 1996 gives {π, e^π, Γ(1/4)}. The *joint* independence of {π, Γ(1/4), Γ(1/3)} — equivalently, deciding clause (ii)'s containment in the refuting direction — is, to our knowledge, **open** (exactly the honesty note FTD-0353 §2.2(i) already recorded). Proving hull-boundedness would *refute* the standard conjecture — a contrarian transcendence breakthrough; disproving it needs the open joint independence.

**Consequently, route (a) of the FTD-0358 protocol — prove representativeness by showing every output of the sector-neutral class lands in the hull — is CLOSED**: it is not "unattempted" or "hard"; it is equivalent to settling a named open transcendence question, in the direction that contradicts the conjectural consensus.

**(iii) Every hull-bounded class imports the d = −4 bit.** By (i)–(ii), a class that both contains 𝒮 and is (provably) hull-bounded must break sector-neutrality: restrict (W) to the BCC layer, (Z) to the C₄ characters, (Θ) to the ℤ[i] CM point. These three restrictions are one bit expressed three times: **choose the order-4 (ℤ[i], d = −4) reading of the cubic symmetry over the order-3 (ℤ[ω], d = −3) reading.** That bit is FC-0, verbatim (`SPEC_FTD_FRAMEWORK_V1.md` §1.2: *"The cubic lattice's order-4 planar symmetry is read as the arithmetic of the Gaussian integers ℤ[i]. This reading is a modelling choice… Declaring it does not derive it."*). The reviewer's "near-tautological relative to that choice" is thereby **confirmed and discharged simultaneously**: Lemma 1's confinement is indeed downstream of a choice, and the choice is not hidden in the generator list — it is the constitution's zeroth commitment, declared with its own honesty clause. ∎

**Corollary 2.1 (what the selection is *not*).** `[DERIVED]` Three documented motivations single out the d = −4 datum *within* the trichotomy — (a) BCC is the unique multiplicative layer *(check A4; `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`)*; (b) ℚ(i) is the unique imaginary quadratic field with |μ_K| = |disc K| *(check G1; OT-1.9 T1, spine Theorem 3's arithmetic half)*; (c) the det_ζ/θ/AGM machinery all evaluate in closed form there. Each is a genuine arithmetic distinction `[THEOREM]`-grade in itself; none is a derivation of the restriction from P1–P5 — "restrict to the multiplicative layer" is extensionally the same choice re-described, and nothing in the postulates says self-energies of non-multiplicative layers are not outputs. Per the framework's own discipline this makes FC-0 a **motivated** `[AXIOM]`, not an earned theorem — exactly the tag it already carries.

**Corollary 2.2 (the dynamics corroborate the split).** `[SYNTHESIS]` The engine's own update stencil is (SC+FCC)/2 — provably BCC-orthogonal (FTD-0050/0079) — so the substrate's documented *dynamics* run on precisely the two Moore layers whose analytic sectors the documented *spine* excludes, and the Link-8 campaign measured (three independent tests, all negative) that engine correlators carry no master-quadratic content. The sector split of Theorem 1 is not an analytic curiosity; it is already visible, dynamically, in the corpus's own closed-negative record. The (SC+FCC)/2 stencil's own self-energy evaluates numerically to G_eng(0) = 1.267932172… with no documented closed form *(script INFO line; no claim made — it sits outside the declared basis, §7)*.

---

## 5 · Theorem 3 — the FC-0-locked constructor closure (route (a), sector-locked salvage)

**Theorem 3.** `[THEOREM — given FC-0 and the constructor basis of Definition 1, conditional on Chudnovsky 1976]` Lock every analytic constructor at its FC-0 datum: (W) at the BCC layer, (Z) at the C₄ characters with ℚ(G\*, π)-rescalings, (Θ) at τ = i. Then the output of **every finite composition** of the five families lies in the radical-closed hull

$$\widetilde{N}_{\mathrm{rad}} \;=\; \bigcup_{m \ge 1} \overline{\mathbb{Q}}\big(\pi^{1/4m},\, G^{*1/2m}\big) \;=\; \bigcup_{m\ge1}\overline{\mathbb{Q}}\big(w^{1/m}, s^{1/m}\big),$$

and δ = √(G\*(4G\*−1)) ∉ Ñ_rad by FTD-0353 Theorem 3.

*Proof.* (T), (S): rational matrix entries; invariants in ℚ *(FTD-0244 Lemma 1, repaired)*. (Z) at C₄: values √(2π)/Γ(k/4) ∈ {2^{1/4}s^{−1}, √2, 2^{1/4}s, √2·w²} — hull monomials *(check C3)*; rescaling by c ∈ ℚ(G\*, π)^× multiplies by c^{ζ_H(0,a)} with ζ_H(0,a) = ½ − a ∈ ℚ *(check C4)* — a **radical** of a hull element (e.g. det_ζ(G\*·D_{1/4}) = G\*^{1/4}·2^{1/4}/s, *check C5*): this is the step that requires Ñ_rad rather than bare Ñ, and it is exactly the case FTD-0353 Theorem 3 was proved for. (Θ) at τ = i: the θ₂/θ₃/θ₄/η/AGM/ϖ/Ω rows *(checks F3–F6; FTD-0353 §2.2 table)* — all ℚ̄-monomials in (s, w). (W) at BCC: G\*²/(2π) = s⁴/(2w⁴) *(checks B2–B3, F2)*. Compositions: the four field operations stay inside each ℚ̄(w^{1/m}, s^{1/m}) (a field); products of monomials are monomials *(explicit composite verified, check F7)*; constructor-on-composite instances are covered by the rescaling clause. δ-exclusion at every level of the union: FTD-0353 Theorem 3 (4S^{2m}−1 squarefree, branch locus disjoint from the coordinate cross; regression *checks E4*). ∎

**Consequence (the upgrade).** FTD-0353's §2.3 premise — "that the 14-row table exhausts what the substrate natively constructs is a judgment, not a theorem" — is **discharged into the definitions** for the FC-0 sector: the closure now quantifies over *all finite compositions of the declared families at arbitrary depth*, not over a finite list. Within the sector, the inventory `[SELECTION]` becomes a **characterization** `[THEOREM given FC-0 + Definition 1]`. What remains selected has moved one level up and gotten smaller and sharper: (1) FC-0 itself (already declared, already tagged); (2) the adequacy of the five-family basis (§7). The Chudnovsky conditionality is untouched and permanent — nothing here removes it, and no reading of this document should call the result "unconditional."

---

## 6 · Corollary 4 — the K-BIND conditionality ladder

**Corollary 4.** `[CONDITIONAL THEOREM]` The no-go of Theorem 2 kills hull-**boundedness** of the sector-neutral class; it does not touch δ-**unreachability**. If the d = −3 (resp. disc −24) outputs are admitted as native, then — **conditional on the joint algebraic independence of {π, Γ(1/4), Γ(1/3)}** (resp. plus the Γ(1/24)-family) — the enlarged hull is a higher-rank rational function field ℚ̄(w, s, y, …) in which (2s−1), (2s+1) remain prime, δ² = s²(2s−1)(2s+1) retains odd valuation, and every radical-tower argument of FTD-0353 survives verbatim *(machine-verified symbolic layer, checks E2–E4)*. The new sectors are transverse to the (4G\*−1) fiber unless a cross-sector algebraic relation exists — which would itself refute the standard conjecture.

The ladder, stated once for the record:

| Scope of the universal negative | Conditional on |
|---|---|
| K-BIND relative to 𝒮 (FTD-0244, repaired) | Chudnovsky 1976 |
| K-BIND relative to the FC-0-locked constructor closure (Theorem 3 — **new**) | Chudnovsky 1976 |
| K-BIND relative to the sector-neutral class 𝒜 (Corollary 4 — **new**) | Chudnovsky 1976 **+ OPEN joint independence of {π, Γ(1/4), Γ(1/3)}** (and the disc-−24 analogue for SC) |

This is the honest price list the flag was asking for: widening the class from "the documented list" to "everything the postulates force" costs exactly one named open conjecture — no more (no vague residue), no less (the conjecture really is open, and FTD-0353 §2.2's refusal to claim that case is hereby re-affirmed, not repaired away).

---

## 7 · The permanent classification `[SELECTION — declared]`, and the single falsifier

**Declaration.** The FTD-native analytic construction class is **declared** to be the FC-0-locked constructor basis of Definition 1: families (T), (S), (Z at C₄ + ℚ(G\*,π)-rescalings), (Θ at τ = i), (W at BCC), closed under finite composition. Its adequacy to "everything the substrate can construct" is not a theorem, and by Theorem 2 **cannot be upgraded to one sector-neutrally** without either importing the sector bit (done — FC-0, an `[AXIOM]` on the books since FTD-0254) or settling open transcendence questions (named in §6). Constructions outside the basis — ζ-determinants of operators beyond the FTD-0234 family, general-stencil Green's functions (including the engine's own (SC+FCC)/2 self-energy, §4 Corollary 2.2), θ-values off the CM points — are **not covered** and are not claimed to be.

**The falsifier (unchanged in substance, sharpened in address).** Any future escape from this classification must exhibit **a native, forced output with odd valuation at a prime over (4G\*−1)** — identical to FTD-0314 §4's surviving loophole and FTD-0353 §8's single falsifier, with the sector dichotomy now pricing the two possible addresses:

1. **Within the d = −4 sector:** the exhibit must break Theorem 3's closure — i.e. come from outside the declared basis while remaining forced (this is the FTD-0341 §6 ~10% residue; the genesis-cokernel pre-registration remains the pending attempt). Per FTD-0314 §4, value and forced ℤ/2 supplied *co-fitted* is the banned W-CRIT-2 hand-placement; the exhibit must arrive forward-derived.
2. **Cross-sector:** the exhibit must couple a d ≠ −4 output to the (4G\*−1) fiber — which requires an algebraic relation across CM sectors, i.e. **refuting the standard Γ-independence expectation**. Anyone claiming this route owes transcendence theory a paper before owing FTD anything.

**Status disposition of the flag.** FTD-0244 §1's standing flag and FTD-0353 §2.3's inheritance clause are **superseded in substance by this classification** (the owner's LEDGER row for FTD-0358 is the canonical place to record that; this document edits neither file). The correct permanent reading of FTD-0244 §5's boxed transition — the over-read FTD-0347's reviewer flagged — is: *K-BIND is closed theorem-negative relative to the declared class; the class is declared, not derived; the declaration's content is FC-0 plus Definition 1; the escape clause is the odd-(4G\*−1)-valuation falsifier.* What the FTD-0347 reviewer asked to be left "for a working Galois/transcendence specialist" is now a bounded audit task — verify Theorems 1–3 and the reduction of the residue to the named conjecture — rather than an unbounded representativeness question.

---

## 8 · Non-promotion

`x₊ = 1/α` stays `[SMC]` (FTD-0013). MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. FC-W stays an adopted `[AXIOM]`-class commitment (FTD-0315). FC-0 stays a declared modelling choice (FTD-0254 §1.2) — this document adds load to it and adds no proof of it. FTD-0243 §5's k = 1 non-forcing is untouched. FTD-0314 §4's loophole and FTD-0341 §6's residue remain `[OPEN]` — restated, priced, not closed. The joint independence of {π, Γ(1/4), Γ(1/3)} is **OPEN** and is never used as a fact — only ever as a named conditionality. The algebraic spine is untouched; the golden gate is untouched; **no α is derived anywhere in this document.**
