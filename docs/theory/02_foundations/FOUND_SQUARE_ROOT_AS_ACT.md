# FOUND — The square root as an act of selection: the algebraic signature of type-setting

**Tag:** `[SYNTHESIS]` (the organizing reading) + `[CONJECTURE]` (the "act / intent / positing" interpretation, which needs external non-AI critique) with **one `[THEOREM]`-grade anchor** — the branch of a genuinely two-valued root is determined by no field operation (made precise in §1). **Promotes no tag, derives no new result, imports no claim.**
**LEDGER id:** FTD-0340
**Reuse (not re-derived) / deepens:**
- `FOUND_TYPE_PRIORITY_PRINCIPLE.md` (FTD-0339, `[SYNTHESIS]`) — context precedes content; the Framework Commitments are precondition-types. **This document names the algebraic mechanism of a type-setting.**
- `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (FTD-0336, `[SYNTHESIS]`+`[CONJECTURE]`) — the substrate owns the *modulus* and must import the *argument*. **The argument is exactly the branch a root cannot fix.**
- `FOUND_ACT_REDUCTION_COUNT.md` (FTD-0322, `[SYNTHESIS]`) — the act-taxonomy `{i, δ}`: the two unforced ℤ/2 branch-choices FTD's chain takes.
- `FOUND_ARROW_AS_SQUARE_ROOT.md` (FTD-0323, `[SYNTHESIS]`) — the arrow is the half-derivative `∂_t^{1/2} = √(∂_t)`.
- `FOUND_TERNARY_STATE_FROM_I.md` (FTD-0128, `[…]`) / FC-0 — `i = √(−1)` is FTD's one *generative* act.
- `AUDIT_ANALYTIC_ORIENTATION_CARRIERS.md` (FTD-0341, `[DERIVED]`+`[SYNTHESIS]`) — the magnitude/phase result this document's §3 cites: the substrate's native orientations are *phases*; `δ` is a real *magnitude*.

**Precedence:** LEDGER > `SPEC_FTD_FRAMEWORK_V1.md` (constitution) > this doc. Golden gate `0xb604d81a3d79366e` untouched (docs only).

---

## 0 · The principle in one line

> The four field operations `+ − × /` are **single-valued** — given their inputs, the answer is forced; they *find*. The **square root of a genuinely two-valued quantity is undetermined by its inputs** — taking it requires *choosing a branch the structure leaves open*; it *makes*. Every place FTD's chain crosses from a forced structure to a chosen one, the crossing is **a square root**: `i = √(−1)` (the one native making) and `δ = √(G*(4G*−1))` (the imported world-choice). The square root is the **algebraic signature of a type-setting act**.

This document records that reading and marks exactly where it is checkable mathematics and where it is interpretation.

---

## 1 · The checkable spine `[THEOREM]` / `[DERIVED]`

Three established facts, none new here, are what the reading rests on. They are the load-bearing layer; everything in §2–§4 is interpretation built on top.

**(a) Field operations are single-valued; a two-valued root's branch is not a field operation.** In any field, `+ − × /` are functions — one output per input tuple. The equation `x² = a` has, when `a` is not a square in the field, **two** solutions that no field operation distinguishes: the field's automorphisms that fix the base may *swap* them. Selecting one is therefore not an operation *of* the structure; it is an extra datum. This is the precise, elementary content of "a root is an act": **an act is the choice of branch in a two-valued root where the base structure singles out no branch.** A principal root forced by an order (e.g. `√3 > 0`, or any positive real magnitude) is **not** an act — positivity already singles out one branch. The acts are the genuinely undetermined branches. `[THEOREM]` (elementary field theory)

**(b) `δ` is such a root, and its branch is route-invariant-unforced.** The master quadratic `x² − 16G*²x + 16G*³ = 0` (`[THEOREM]`, FTD-0001) has roots `x± = 8G*² ± 4G*·δ` with `δ = √(G*(4G*−1)) ≈ 5.66185` the **sole irrational distinguishing the two roots**. Over `Q(G*)` (a rational-function field, since `G*` is transcendental conditional on Chudnovsky 1976), `Q(G*)(δ)/Q(G*)` is a **genuine degree-2 extension** — `4t²−t` is square-free over `Q(t)` — so a ℤ/2 Galois symmetry swaps `x₊ ↔ x₋` and **no native invariant breaks it**: 0/4 FTD-native forcing routes succeed (FTD-0242, `[SMC no-go]`), every native algebraic invariant lies in the Galois-fixed `Q(G*)` (FTD-0244/0314). Verified: `scripts/proofs/proof_delta_weight_zero.py` (17/17). `[THEOREM]` / `[DERIVED, conditional on Chudnovsky 1976]`

**(c) `i` is the other such root — the one the substrate *does* take.** `i = √(−1)` is FTD's unique *generative* act (FC-0 / FTD-0128): the two branches `±i` are swapped by complex conjugation and singled out by no real-field operation; adopting one is the act that seeds `ℤ[i]`, the ternary state, and the entire spine. The arrow's operator is also a root — `∂_t^{1/2} = √(∂_t)` (FTD-0323) — though its *direction* is forced-given-FC-2 (FTD-0324), so the arrow is a root the dynamics can take without a free branch-choice.

So the checkable claim is narrow and true: **wherever FTD must move from a forced (single-valued) structure to a chosen (branch-undetermined) one, the move is a square root**, and the two such moves in the chain are `i` (taken natively) and `δ` (which cannot be — hence imported as FC-W).

---

## 2 · The interpretation `[SYNTHESIS]` / `[CONJECTURE]`

The reading that gives §1 its name — *and which is interpretation, not theorem*:

**Nature folds for free; the un-fold is the chosen act.** Squaring is everywhere in the dynamics and is *irreversible by forgetting*: `v²`, `|ψ|²`, area, the Born rule — each maps two pre-images (`±`) to one image, discarding a sign or phase. The square is a 2-to-1 fold the substrate performs without choosing anything. Its inverse, the square root, must **restore** the discarded distinction — and because the fold destroyed it, the root cannot read it off; it must **posit** it. To take `√a` is to assert an object `x` *defined by* `x² = a`, choosing which of the two the world shall mean. The root is the inverse of an irreversible fold, and an inverse of a forgetting is always a choice.

**The square root is the type-setting act that posits its own object.** This is the FTD-0339 mechanism stated algebraically: a token (content) cannot bootstrap its context (type); equally, a field operation (which only ever *combines existing values*) cannot perform a branch-choosing root (which *introduces a value the field did not contain*). "Taking the root" is precisely "setting the type" — positing the object the subsequent content will be values of. The Pythagorean `√2` reads, under this lens, as the first such positing in mathematics' own history: a magnitude geometry forces to exist that arithmetic alone does not contain — the birth of a new type (the irrational) out of an act the prior structure could not perform.

This is the **geometry of how a distinction is taken**: not the value chosen (that is content) but the *act of there being a branch to choose at all* (that is the type). The reading is `[CONJECTURE]`-grade — it is a coherent organizing metaphor with one elementary theorem anchor, not a proof that "nature never roots." (It does not: the dynamics compute principal roots like `1/√3` freely; those are §1(a)'s *forced* roots, not acts.)

---

## 3 · `i` and `δ`: the two makings, and the magnitude/phase refinement

The act-count (FTD-0322) found exactly two unforced branch-choices in the chain: `{i, δ}`. Under this document's reading they are the two type-settings:

- **`i = √(−1)` — the native making.** Forced *to be available* by the capacity for information itself (a distinction needs two sides; `ℤ[i]` is the minimal arena), then *taken* as FC-0. It is a **unit phase**: a pure orientation, magnitude 1.
- **`δ = √(G*(4G*−1))` — the imported making.** The world-choice the substrate **cannot** take natively (§1b) and therefore adopts as **FC-W** (FTD-0315). Under FC-W, `x₊ = 1/α` is a `[CONDITIONAL THEOREM given W]`, **not** `[DERIVED]`.

**The refinement (from FTD-0341).** The two makings are not the same *kind* of root. The substrate's native orientations — `i`, the arrow, chirality, the spectral-asymmetry η, the AGM branch — are all **phases** (unimodular: they choose a *direction*). But `δ` is a real, **magnitude-bearing** surd: it sets the *size* of the choice, not merely its direction. The four-carrier closure (FTD-0341) shows every native analytic orientation lands in `Q(√2)·G*` carried by a phase, while `δ` requires the real factor `√(4G*−1)` that no phase supplies. So the boundary sharpens to: **the substrate can choose a direction (a phase) but not the size of the choice (a magnitude).** `i` it can take because `i` is pure direction; `δ` it cannot, because `δ` carries a magnitude — and a magnitude is a value the forgetting fold did not leave behind.

---

## 4 · Where this sits — and what it sharpens

This document does **not** add a result; it names the mechanism three existing boundary statements already describe:

| boundary statement | what this reading adds |
|---|---|
| **Type-priority** (FTD-0339): content cannot bootstrap context | the *algebraic form* of a type-setting is a branch-choosing square root; "set the type" = "take the root that posits the object" |
| **Modulus/argument frontier** (FTD-0336): substrate owns the modulus, imports the argument | the **argument** is exactly the branch a root leaves undetermined; "import the argument" = "import the branch-choice" — and §3 sharpens it to *magnitude vs phase* |
| **Act-count** (FTD-0322): `{i, δ}` | the two acts are the two branch-choosing roots; `i` taken (phase), `δ` imported (magnitude) |

It also **predicts** (no new claim, a consistency note) that the locked genesis-cokernel pre-registration — the last structurally-distinct carrier for `δ` — lands **closed**: a cokernel is what a 2-to-1 fold forgets, and recovering it is the positing act the substrate cannot perform on its own.

---

## 5 · The honest boundary — what this does NOT claim (GTCA F3 / F9 / F10)

- **It promotes nothing.** `x₊ = 1/α` stays `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013); MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FC-W stays an `[AXIOM]`-class adopted import; the master quadratic and the algebraic spine are untouched; **no α is derived anywhere.**
- **The theorem/interpretation seam is explicit.** Only §1 is checkable (elementary field theory + FTD-0001/0242/0244/0314, verified by `proof_delta_weight_zero.py`). §2–§4 are `[SYNTHESIS]`/`[CONJECTURE]` — a reading, not a derivation. The claim "a root is an act" is rigorous **only** in the narrow §1(a) sense (a two-valued branch is no field operation); the "intent / positing / nature-folds-for-free" gloss is metaphysics and is offered as such.
- **Aesthetic cleanliness is a reason for suspicion, not belief (F3).** That the principle unifies `i`, the arrow, `δ`, and the Pythagorean `√2` under one image is attractive *and therefore to be distrusted* until it survives external critique. Tagging it `[SYNTHESIS]` records its status; it does not establish its truth (F10).
- **It needs external non-AI critique (F9).** This is a long-session organizing reading produced inside the project's own frame; the operative risk is that it is a *defensible* gloss rather than an *independent* one. The mathematics of §1 stands regardless; the reading awaits an outside interlocutor.

---

## 6 · Status line

**Nothing is promoted.** Tag `[SYNTHESIS]` + `[CONJECTURE]`, one `[THEOREM]` anchor (§1a). `i` stays FC-0's generative act; `δ` stays imported as FC-W; FTD-0013 stays `[SMC]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; P1–P5 and the FC register untouched; golden gate untouched; no α derived. This document is exposition: it names, under one image, the type-setting mechanism that FTD-0339/0336/0322/0323 already describe — *the square root is where a forced structure becomes a chosen one.*
