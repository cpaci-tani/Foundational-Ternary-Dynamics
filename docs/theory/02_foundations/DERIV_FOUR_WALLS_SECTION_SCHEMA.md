# DERIV — The section schema: the framework imports as sections of exhibited lossy collapses, with one proven edge

**Tag:** `[SYNTHESIS]` (schema roll-up of proven instances — promotes nothing) + `[THEOREM]` (Lemmas 1–3, elementary and machine-verified) + `[OPEN]` (the four-walls-are-one forcing theorem itself).
**LEDGER id:** FTD-0508 · **Date:** 2026-07-25
**Deepens:** [`FOUND_MODULUS_ARGUMENT_FRONTIER.md`](FOUND_MODULUS_ARGUMENT_FRONTIER.md) §7 (FTD-0336), which poses the four-walls-are-one forcing theorem "with no proof sketch." This document supplies the skeleton and proves one edge.
**Inputs (all at their tags of record):** FTD-0494 (cusp-dressing integrability), FTD-0499 (finite-memory reversible lift), FTD-0502 (worldline current kernel), FTD-0243 (commutativity independence), FTD-0253 (Lorentzian-metric gap), FTD-0208 (L²-not-L¹), FTD-0314/0315 (carrier narrowing / FC-W), FTD-0323 (cyclotomic branch sign), FTD-0244 (K-BIND).
**Verification:** `scripts/proofs/proof_four_walls_record_monoid.py` (5/5 PASS, 2026-07-25).

---

## 0 · Thesis

The frontier document FTD-0336 conjectures that FTD's four standing imports — FC-1 (the measurement map), FC-2 (the backward pairing), FC-W (the δ branch), and the L²-not-L¹ budget — are one import: the chosen adjoint of the substrate's lossy forward map. The 2026-07-25 engine arc (FTD-0494/0499/0502) produced, independently of the frontier, three exact theorems each exhibiting a specific non-injective map together with a proof that resolving it requires imported choice-structure. This document does three things.

1. It defines the common schema (§1) and verifies that each wall and each engine theorem instantiates it exactly (§2). This part is a roll-up: every instance row is already proven in its source document; no instance is promoted here.
2. It proves one new edge (§3): the registered FTD-0499 history control — the constructive object whose adoption would discharge the FC-2 import for the frozen projection — carries an exactly non-commutative record algebra, with commutator defect `(m−1)(b1−b2)`. Since the unlifted substrate is commutative (FTD-0243), the algebraic signature of the FC-1 import object appears automatically on the section side. This is the first proven implication between two of the four walls.
3. It states precisely what remains missing for the full forcing theorem (§5): a cross-category transfer to the δ fiber, and it derives from the closures of record (FTD-0244/0314/0326/0327) a conditional corollary about where such a transfer can and cannot live.

Nothing here derives α, promotes `x₊ = 1/α` above `[SMC]`, or converts any FC from adopted to derived.

---

## 1 · The schema

**Definition 1 (lossy collapse; section demand).** A *lossy collapse* is a pair `(X, q)` where `q : X → Y` is a non-injective map arising as a forced construction of the substrate (a dynamics step, a quotient of representation, a coefficient assembly). The *section demand* of `(X, q)` is the datum required to resolve its fibers: a section `s : Y → X` with `q∘s = id`, or equivalently a fiber-labelling rule (a disintegration). The *import* of the collapse is the section demand, when the substrate's own forward operations provably cannot supply it.

This is FTD-0336 §2's adjoint/section/non-tracial-state column, specialized so each instance names its `q` explicitly. The schema claim of this document is only that the objects below share this form; the forcing claim (that one section supplies all) is not part of the schema and remains `[OPEN]`.

---

## 2 · Instance table

Every row cites the document in which its content is proven; the tag column is the source tag, unchanged.

| # | Collapse `q` (exhibited where) | Fiber content lost | Section demand = the import | Status of record |
|---|---|---|---|---|
| I1 | Raw threshold/manifestation map `f : S → S`, `m ≥ 2` preimages of one output (FTD-0497 §, FTD-0499 §1) | which manifested anchor entered the merge | backward pairing across the kernel — the FC-2 object. No finite hidden lift exists (deficit `(m−1)\|H\|`); exact resolution costs `log2(m)` bits per merge, exported or unbounded | `[THEOREM]` (FTD-0499) + FC-2 `[AXIOM — declined]` |
| I2 | Endpoint quotient `q : {oriented transport 1-chains} → {endpoint multisets}` (FTD-0502 §1–2) | a divergence-free 1-cycle, `dim ker(div) = 2V+1`; the cycle changes field evolution | the transport 1-chain, selected atomically with motion | `[THEOREM]` (FTD-0502); verdict `WORLDLINE_PATH_IS_REQUIRED_STATE` |
| I3 | Hop-representation gluing: `(n, e_a) ~ (n+e_a, 0)` demands a site scalar with `C(n+e_a) − C(n) = ω_a(n)` (FTD-0494 §2) | a global primitive of the lattice one-form `ω` | none exists — plaquette holonomy `0.438` on a source-free contractible plaquette; the resolving object is connection-type, the carrier class priced as IMP-S4 | `[THEOREM]` (FTD-0494); `[CLOSED NEGATIVE — frozen local dressing energy]` |
| I4 | Observable restriction to the frozen quotient: any observable factoring through `f` is blind to fiber content (FTD-0499 §4; FTD-0243) | order and identity of merged branches | a fiber-distinguishing, order-sensitive pairing — the FC-1 object `M` | FC-1 `[AXIOM — declined]`; substrate commutativity `[THEOREM]` (FTD-0243) |
| I5 | Linear quotient `V → V/W` with no canonical complement | the orthogonal complement / transverse phase | an inner product (the Pythagorean pairing); a continuous SO(3) datum the discrete `O_h` cannot supply | FTD-0208 `[stands]` (structural incompatibility, v3) |
| I6 | Coefficient assembly of the master quadratic over `ℚ(G*)`: the polynomial does not order its own roots | the branch of the two-element Galois orbit `{x₊, x₋}` under the root-swap ℤ/2 | the surd `δ = √(G*(4G*−1))`, transcendental over `ℚ(G*)`; no native operator (FTD-0244) or finite symmetry (FTD-0314/0326/0327) reaches it | FC-W `[AXIOM — adopted]`; `x₊ = 1/α` `[SMC]` unconditionally |

Two remarks. First, I2 and I3 are not among the frontier's original five rows: they are new instances supplied by the engine arc, and I3 lands on the same carrier type the import ledger had already priced qualitatively (IMP-S4) before FTD-0494 existed. The dynamics rediscovered a priced line, which is evidence that the schema is tracking a real structure and not a narrative convenience — evidence, not proof. Second, I1 and I4 name the *same* collapse `f` read twice: I1 asks for state recovery (a section), I4 asks for record keeping (a disintegration). §3 makes that identification exact in one direction.

---

## 3 · Lemma 1 — the record monoid is non-commutative `[THEOREM]`

**Setting.** The constructive half of FTD-0499 is the registered history control: on an `m`-way merge, push the branch digit `b ∈ {0, …, m−1}` by

```text
h' = m·h + b,
```

reversed exactly by quotient and remainder. Adopting this control (with unbounded or exported capacity) is precisely what purchasing the FC-2 section for the frozen projection would mean: FTD-0499 §6 lists it as repair (3), the only repair that preserves the raw ontology and transition.

**Lemma 1.** Let `P_b(h) = m·h + b` be the push map. Then for all `h` and all digits `b1, b2`:

```text
(P_{b2} ∘ P_{b1})(h) − (P_{b1} ∘ P_{b2})(h) = (m − 1)(b1 − b2),
```

which vanishes iff `b1 = b2`. The monoid generated by `{P_b}` under composition is therefore non-commutative for every `m ≥ 2`, and is the free monoid on `m` generators acting faithfully by base-`m` digit concatenation.

*Proof.* `P_{b2}(P_{b1}(h)) = m²h + m·b1 + b2` and `P_{b1}(P_{b2}(h)) = m²h + m·b2 + b1`; the difference is `(m−1)(b1−b2)`. Faithfulness of the digit-string action is base-`m` uniqueness. ∎ (Machine check: `proof_four_walls_record_monoid.py` C2, all digit pairs, `m ∈ {2,3,8}`.)

**Lemma 2 (quotient commutativity).** Any observable that factors through the frozen projection `f` is invariant under push order — the projected raw output does not depend on the order in which branch digits were absorbed. (Direct from `pr_S F(s,h) = f(s)`; check C3.) Combined with FTD-0243, the unlifted substrate's observable algebra is commutative; the non-commutativity of Lemma 1 lives strictly on the section side.

**Corollary 3 (one directed edge of the four-walls graph).** Purchasing the FC-2 import object for the frozen projection — the fiber-resolving record — automatically constructs an order-sensitive, non-commutative record algebra on the lifted state, at zero additional adopted structure. The algebraic *signature* of the FC-1 import object (a pairing for which `ω(ab) ≠ ω(ba)`) is therefore not a second independent purchase: it is entailed by the first.

**Scope guard (mandatory).** Corollary 3 establishes an implication at the level of algebraic signature only. It does not produce Hilbert space, the canonical commutation relations, ℏ, the Born rule, or any quantitative content of quantum mechanics' `M`; whether the record monoid's non-commutativity has any bearing on physical measurement is `[CONJECTURE]`. FC-1 remains declined; FC-2 remains declined; this corollary changes no tag. What it changes is the *shape of the import surface*: the four walls are not four independent points — at least one ordered pair is connected by a proven arrow.

---

## 4 · Established edges and their directions

- **FC-2 ⇒ FC-1-signature.** Proven (Corollary 3). The converse — that adopting a non-tracial pairing forces the full backward pairing — is not proven and is plausibly false at signature level (a pairing can be order-sensitive on a subalgebra without resolving every fiber). `[OPEN]`.
- **FC-2-decline ⇒ I2, I3 demands.** If histories are not retained, the transport 1-chain (I2) cannot be reconstructed from state and must be selected atomically with motion — this is FTD-0502's verdict restated; and the work ledger's missing primitive (I3) cannot be a state function, forcing the connection-type object. `[DERIVED — restatement of the source verdicts in schema vocabulary]`.
- **FC-W ⇔ anything.** No edge proven in either direction. This is the missing step, stated next.

---

## 5 · The missing step: the transfer problem `[OPEN]`

The forcing theorem of FTD-0336 §7 requires that adopting any one section canonically constructs the others. Instances I1–I5 live in the *event category*: finite sets, merge maps, digit alphabets, chain complexes. Instance I6 lives in the *arithmetic category*: `ℚ(G*)`-algebras with Galois action. The theorem therefore needs a transfer:

> **Transfer problem.** Exhibit a structure-preserving assignment from the substrate's event data (branch digits of I1, cycles of I2, holonomy data of I3) to the arithmetic category, carrying a binary merge's ℤ/2 branch datum to the root-swap ℤ/2 of `ℚ(G*)(δ)/ℚ(G*)`, such that a section on the event side induces a section on the arithmetic side.

Two closures of record constrain where such a transfer can live, and the constraint is itself derivable:

**Corollary 4 (conditional).** If the transfer exists, it is not valued in the native operator calculus and not mediated by finite symmetries. *Proof sketch:* the operator calculus's reachable field is `ℚ(G*)` and δ is transcendental over it (FTD-0244, K-BIND); every native finite-symmetry carrier is Galois-blind to δ (FTD-0314/0326/0327); the native square-root-of-time branch sign is cyclotomic, in `ℚ(G*)` (FTD-0323), so the half-step arrow cannot carry it either. The only carrier class not closed by those results is event data itself — history strings, merge digits, collision-vertex data. `[DERIVED — conditional on the cited closures; the sketch is a routing argument, not a construction]`.

This corollary is the document's second substantive output: **the four-walls-are-one theorem, if true, will be proven in the event-data category or not at all.** It sharpens the search space for Front C of the Consumption Program and is consistent with (and independent of) the Front-B candidate registered in [`SCOPE_P6C_CANDIDATE_TEMPORAL_PHASE.md`](../01_reference/SCOPE_P6C_CANDIDATE_TEMPORAL_PHASE.md) (FTD-0511), which proposes event-attached phase as an adoption precisely because events are the unclosed carrier class.

What would count as the theorem: a construction realizing the transfer plus the three remaining edges (FC-1 ⇒ FC-2, L² ⇔ any, FC-W ⇒ any), or a proof that no transfer exists (which would *refute* the four-walls-are-one reading and split the frontier into at least two irreducible imports — an equally valuable outcome under the Number-One Goal, since it would price the boundary more finely).

**Falsifier of the schema reading itself:** exhibit one of the four walls whose imported object is provably *not* the section demand of any substrate collapse — i.e., an import with no associated non-injective `q`. The L² wall (I5) is the most promising place to attempt this refutation, since its quotient is representation-theoretic rather than dynamical.

---

## 6 · Status line

`[SYNTHESIS]` + Lemmas 1–3 `[THEOREM — elementary, machine-verified]` + Corollary 4 `[DERIVED — conditional]` + main forcing theorem `[OPEN]`. Nothing promoted: `x₊ = 1/α` stays `[SMC]`; MC-T4.3 stays `[OPEN — SCOPED NO-GO PACKAGES]`; FC-1/FC-2 stay declined `[AXIOM]`; FC-W stays adopted `[AXIOM]`; the frontier meta-conjecture stays `[CONJECTURE]`. The four-walls graph now has one proven directed edge and a derived routing constraint on the rest.
