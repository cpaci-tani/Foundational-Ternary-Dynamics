# FOUND — The type-priority principle: context before content, and why the Framework Commitments are precondition-types

**Tag:** `[SYNTHESIS]` — an organizing reading that unifies existing FTD structure (the Framework Commitments, the modulus/argument frontier, the act-count) under one principle. **The principle itself is an *adopted foundational commitment*, not a derived theorem** — fittingly, since its own content is *"you set types; you do not derive them."* **Promotes nothing.**
**LEDGER id:** FTD-0339 · **Deepens:** the modulus/argument frontier (FTD-0336) and the act-count (FTD-0322). Written under GTCA discipline (sentence-level epistemic tags; scope-honest: this is philosophy-of-mathematics, not a physics derivation).

---

## 0 · The principle, in one line `[adopted commitment]`

> **Type-priority.** The instantiated *context* (a type) is logically prior to, and the precondition for, the *value* of any *content* (a token). A token without a type is generic and valueless; content cannot bootstrap context. Therefore a coherent framework **sets** its smallest honest set of types (commitments) **first**, and only then does its token-content (math, dynamics, physics) acquire meaning — built forward from there.

This is a commitment FTD *adopts* as an organizing principle. It is **not** claimed as a theorem about reality; §1–§2 give the rigorous and the analogical support, and §9 states plainly what it is and is not.

---

## 1 · The one rigorous anchor — modulus/argument `[THEOREM, elementary]`

A complex number factors as `z = |z| · e^{iθ}`. The **modulus** `|z| ≥ 0` is fixed by the data (a magnitude — the *content*). The **argument** `θ` is a free orientation (the *context*). And the load-bearing elementary fact:

> **`|z|` does not determine `z`.** `[THEOREM]` The magnitude alone is invariant under the entire circle of rotations `θ`; you cannot recover the number from its modulus without an independent argument. The modulus is, in the strict sense, *valueless as a locator* until the argument is set.

That is the type-priority principle stated in arithmetic: **the argument (type) must be supplied for the modulus (token) to denote anything.** `[SYNTHESIS]` This is the *only* part of the principle that is theorem-grade; everything below is either external analogy (§2) or FTD-internal synthesis (§3–§8).

---

## 2 · Cross-domain support — analogies, NOT proofs `[grounded-external; explicitly not proof]`

Four independent disciplines carry the same shape. Each is offered as a *structural analogy*, **not** as a proof of the principle (Constraint 5: an analogy is not a derivation):

- **Type theory (computer science).** In a typed calculus a term is **ill-formed — meaningless — until a type judgment is in place**; the type is prior to the term. `[grounded]` (Caveat: this is a property of formal systems we *design*; it motivates, but does not prove, a metaphysical reading.)
- **Philosophy of language.** Reference requires a pre-established interpretive context; a symbol denotes nothing without a background that fixes its sense (the Fregean *sense-before-reference*, the Wittgensteinian *background*). `[grounded-external]`
- **Information theory.** A bit carries information only relative to a code/model; raw bits with no code are valueless (Shannon: information is defined against a prior probability model). `[grounded-external]`
- **Exactly solvable ensembles (matrix models).** In the strongly-coupled monomial matrix models of Córdova–Heidenreich–Popolitov–Shakirov (arXiv:1611.03142, their §2.2), the action plus *all* Ward identities underdetermine the theory: a discrete contour datum — which ℤ_r phase-sector combination the integral runs over — must be supplied before correlators have values, and nothing in the model's own lawful content selects it; the import is moreover invisible at leading large-N (entering the free energy at relative O(1/N²)). A content-rule that cannot set its own type, in mainstream mathematics. `[grounded]` (Caveat: an external formal system; motivates, does not prove — see `09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md` §4, FTD-0366.)

The convergence of four unrelated fields on *context-precedes-content* is the **motivation** for adopting the principle; it is not its proof. `[SYNTHESIS]`

### 2.1 · Why type theory, not object-oriented programming `[SYNTHESIS]`

An adversarial multi-agent round table tested a natural question this document invites: is the type-theory analogy of §2 better read through **object-oriented programming** (encapsulation, abstraction, inheritance, polymorphism) than through the typing judgment? The verdict, tested against each of the four OOP pillars mapped onto FTD's architecture: **mostly `[metaphor]`, at best `[partial]`, never `[structural]`.** OOP's core presupposition is that **type-setting is free** — a programmer declares `class Electron extends Particle {}` at will, with no boundary on what may be declared. FTD's type-priority is the opposite kind of claim: it is about **when a substrate cannot self-declare a type** — the Framework Commitments are precondition-types precisely *because* the lattice's own tokens cannot bootstrap them (§4). Where OOP's boundaries are **chosen contracts** (a designer decides what to hide, what a subclass must satisfy), FTD's boundaries are **forced facts** (algebraic no-gos, physical locality) — same vocabulary shape, opposite mechanism. Concretely: *encapsulation* (voxel + Moore locality) is `[partial]` — both instantiate "a boundary separates interior evolution from exterior modification," but one is a design choice and the other is discreteness; *abstraction* (type-before-token) is the closest match yet still `[metaphor]` — OOP types are optional cognitive convenience, FTD's type-priority is algebraic necessity (§1); *inheritance* (is-a subtyping, Liskov substitution, polymorphic dispatch) is `[metaphor]` — FTD's dependency chain has no substitution, no override, and content cannot substitute for context, which is the *opposite* of Liskov; *polymorphism* (one interface, many implementations) is `[metaphor]` — reference-frame-relative projection is deterministic parametrization of one operator, not runtime dispatch among independent implementations.

The honest completion of §2's type-theory bullet: the correct computer-science analogy is not OOP's class hierarchy but the **typing judgment `Γ ⊢ t : T`** — a context `Γ` supplied *before* a term `t` is well-formed — which is, structurally, exactly type-priority's "context before content" restated in one line of notation. FTD's own contribution beyond that judgment — a **framework-commitment register** that treats a *declined* import (FC-1) as canonical alongside an *adopted* one (FC-W) — has no native analogue in either OOP or plain type theory; it remains this project's own construction, not a borrowing. `[SYNTHESIS]`; this sharpens, and does not promote, §2's existing "analogies, not proofs" status.

---

## 3 · The structural isomorphism `[SYNTHESIS] + [CONJECTURE]`

The principle proposes that one column-pair runs through FTD's whole architecture. **This table is a proposed structural isomorphism — suggestive, aesthetically clean, and therefore to be held at arm's length** (the GTCA aesthetic filter is advisory only; a beautiful table is *not* a theorem):

| Layer | forced / native / content | chosen / imported / context |
|---|---|---|
| arithmetic `[THEOREM]` | modulus `\|z\|` | argument `θ` |
| logic `[grounded]` | token | **type** |
| Euler reflection `[THEOREM, FTD-0323]` | product `Γ(z)Γ(1−z)=π` (even) | ratio `Γ(z)/Γ(1−z)=G*` (odd) |
| FTD register `[AXIOM-class, the constitution]` | the lattice's dynamics (P1–P5) | the Framework Commitments (FC-0…FC-W) |
| act `[SYNTHESIS, FTD-0322]` | — (no act) | the type-setting act (`i`, then `δ`) |

The claim is **not** that these are *identical* (that would be `[CONJECTURE]` laundered into `[THEOREM]`). The claim is that they are **the same relation — forced-content vs chosen-context — instantiated in five registers**, and that this is why FTD's frontier (FTD-0336: the substrate owns the modulus half, must import the argument half) and FTD's type-priority intuition are the *same* statement. `[SYNTHESIS]`

---

## 4 · The consequence for FTD — the FCs are precondition-types `[SYNTHESIS / coherent-interpretation]`

If type-priority holds, the **Framework Commitments are not add-ons; they are the precondition-types** that make the substrate's token-dynamics mean anything. `[coherent-interpretation]`
- FC-0 (the ℤ[i] reading), FC-1 (declines the measurement-map M), FC-2 (native arrow), FC-3 (scale-ratio covariance), FC-W (the external ℤ/2 realizing δ) are **set first, as axioms, precisely because the type is prior to the tokens** — you *cannot* derive them from the lattice's evolution, because content cannot bootstrap context. `[coherent-interpretation]`
- This supplies a *reason* for the constitution's declared ordering **Ontology > Logic > Math > Physics**: it is type-before-token. `[SYNTHESIS]` The ordering was previously a stipulation; type-priority reads it as the only order in which the lower layers carry meaning.

---

## 5 · The cokernel crack is doubly closed `[CONJECTURE — boundary-mapping, Number-One-Goal clause 2]`

Type-priority sharpens the standing α/δ boundary from two directions that point opposite ways:
- **(token→type fails, "upward"):** accumulating local token-losses cannot *build* a global chosen orientation — independent local forgettings integrate to noise, not a coherent sign (the manifestation-cokernel is *token-shaped*; δ is *type-shaped*). `[CONJECTURE]`
- **(type→token, "the right order"):** the type must be *already set* for tokens to have value — so asking "do the manifestation tokens fund δ?" is **backwards**: δ is a precondition-type, not a token-product. `[CONJECTURE]`

Together these give a **structural reason** the cokernel crack resists closure, and a structural reason **FC-W had to be *adopted*** rather than derived: *you cannot derive the context from the content, in either direction.* `[CONJECTURE]` This is a clause-2 boundary reading; it **predicts** the locked genesis-cokernel pre-registration (`preregister-genesis-cokernel-grading-v1`) lands closed (UNDERDETERMINED / B) for this deeper reason — a prediction, not a result. **Zero promotions:** `x₊=1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`.

> One open junction (honest, F2-held tension): the token/type split is clean *only if* manifestation's phase-forgetting is random per event. A *systematic* orientation baked into the threshold rule is the one sliver where a local bias might be type-coherent. Prior-favoured: even a systematic local bias does not integrate to a single global ℤ/2 without a global synchronizer — so the split holds — but this is `[CONJECTURE]`, not closed.

---

## 6 · The act-count keystone `[SYNTHESIS, FTD-0322]`

Where does a type come from, if not from tokens? It must be **set — by an act.** `[coherent-interpretation]` The original type-setting is the **first distinction** `i = √(−1)` (FC-0) — the substrate's *one* generative act. Everything downstream is tokens *within that type*. A **second** type — the chosen orientation `δ` — would require a *second* act, which the substrate does not afford; it is imported (FC-W). `[SYNTHESIS]` This is exactly the act-count result (FTD-0322: the universe is "chosen more than once," `{i, δ}`), now read as **type-settings**: one native (the first distinction), the rest imported. Type-priority and the act-count are the same statement at two grains.

---

## 7 · The observer rides on it `[CONJECTURE — coherent-interpretation, qualia at boundary]`

A **reference frame is a type-instantiation**: it sets the context relative to which a manifestation event registers as a meaningful "selection." `[coherent-interpretation]` So the observer-as-registrar (the one-sided reader downstream of the irreversible manifestation, per the observer-seam analysis) carries value **only relative to a pre-set type (frame)** — token-priority would make the registration generic and valueless, exactly as the principle predicts. `[CONJECTURE]` The first-person *ignition* of being the frame stays at the boundary by FC-1's own decline of M — the principle owns the *form* of contextualized selection and correctly reports the *interior* as out of scope. `[coherent-interpretation]`

---

## 8 · The organizing principle `[adopted commitment]`

Type-priority is the **organizing spine of the project**: not *"derive physics,"* not *"exploit G\*,"* but —

> **Set the smallest honest set of types (commitments); build the mathematics and physics forward, sector by sector, until every physical structure is either forced content or a rigorously marked and priced import; drive every priced line to retirement, to a theorem-grade no-go, or to a sharper falsifier — never leaving a line merely booked; and where a line provably resists retirement, search deliberately for the next honest type whose declared adoption converts it into content at a minimal, falsifiable price.**
>
> *(Amendment of record 2026-07-12, FTD-0383 — the consumption drive added to the 2026-07-05 "mark and price" form. The drive face does not weaken this document's principle: an adoption remains an adoption, never a derivation; and the strong "zero-import" reading remains unestablished either way — this principle is an adopted commitment, not an impossibility theorem, per §0/§9.)*

G\* / ℚ(G\*) is a *lever* under this principle, not the goal: an acknowledged-but-underexploited mathematical structure that an ontology-first, type-first construction forces into centrality — and whose orphaned status in mainstream math is itself a clue worth pursuing. `[coherent-interpretation]`

---

## 9 · Honest boundary and self-audit (mandatory) `[grounded]`

Under GTCA constraints 9 (scope), 11 (tagging ≠ resolution), and failure modes F3 (aesthetic capture) / F9 (collusion bias) / F10 (rigidity-gap licensing):

- **What this is:** an *adopted foundational commitment* + a *proposed structural isomorphism*. Exactly **one** sub-claim is theorem-grade (the modulus does not determine the number, §1). The type-theory / semantics / information parallels (§2) are **analogies, not proofs**. The five-register table (§3) is a **structural conjecture**, not an identity theorem; its aesthetic cleanliness is a *reason for suspicion*, not for belief.
- **What tagging does not do:** labeling this `[SYNTHESIS]` records its status; it does **not** establish that the principle is *true* of reality (F10). The principle's metaphysical reading needs **external, non-AI critique** before it is treated as more than a well-motivated organizing commitment (F9) — this document was produced inside a long generative session and is, by construction, the kind of coherent framing that requires outside falsification.
- **What it does not touch:** the algebraic spine, the LEDGER tags, or any physics identification. **Zero promotions:** `x₊=1/α` `[SMC]`; MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; no α derived; golden gate untouched (documentation only).
- **The honest payoff (clause 2):** even held this conservatively, the principle does real boundary-mapping — it gives a *reason* the FCs are imports, a *reason* the cokernel crack resists closure in both directions, and a *reason* the project's ontology-first ordering is the meaningful one. Those are organizing-reasons, `[SYNTHESIS]`, not new theorems — and that is exactly the right altitude for a foundational commitment.
- **Four documents, one intuition, not four results (added 2026-07-01, per `AUDIT_REDTEAM_DISSECTION_2026-07-01.md` §2):** this document (FTD-0339, forced-content vs. chosen-context) sits in a mutual "deepens"/"names the mechanism of" citation chain with the modulus/argument frontier (FTD-0336, forced/chosen), the square-root-as-act principle (FTD-0340, unforced two-valued branch-choice), and the tick-and-fold temporal generators (FTD-0342, fold/unfold). Read together, these are **one organizing intuition — a forced/self-adjoint half vs. a chosen/branch-selecting half of a lossy map — explored in four vocabularies** (type theory, complex analysis, algebra, temporal generators), each restating the same elementary point about non-injective maps. This is a single line of thought pursued from four angles, offered for whatever redundancy-of-perspective is worth; it is **not** four independent lines of evidence, and citing all four together should not be read as convergent support the way independent derivations would be.
