# FOUND — The ontic chain: from distinction to the Watson integral

**Tag:** `[SYNTHESIS]` (assembles existing results and elementary mathematics)
**+ `[THEOREM]` on the individually-proved steps, marked inline**
**+ two `[CONJECTURE]`-grade shave candidates, explicitly not registered**
**Status line (read first): NOTHING IS PROMOTED BY THIS DOCUMENT.** `D = 3`
stays `[SELECTION — declared]` (FTD-0355). `G*` stays entered-by-choice
(FTD-0794). `x₊ = 1/α` stays `[SMC]` (FTD-0013). The two shave candidates in §5
are stated at `[CONJECTURE]` and register no reduction in the axiom count.
**Companion:** `FOUND_TYPE_PRIORITY_PRINCIPLE.md` (FTD-0339),
`FOUND_MODULUS_ARGUMENT_FRONTIER.md` (FTD-0336), `SPEC_IMPORT_LEDGER.md`
(FTD-0371).

---

## 0 · What this document is

A single page tracing the shortest chain from a logical primitive to the
lattice Green's function, with **every joint tagged forced or selected**. It
introduces no new physics and attempts no construction. Its purpose is the
Number-One Goal's *second* face — **mark and price the boundary** — applied to
the framework's own foundations rather than to its physics imports.

It is written after a session in which eleven constructions were refuted, and
it deliberately does the opposite thing: instead of building a new mechanism,
it accounts for what the existing ones cost.

## 1 · The primitive

**Distinction.** The drawing of a boundary; asymmetry; the mark.

**Why it survives elimination.** Space presupposes adjacency, which is a
relation, which needs distinguishable relata. Causality and determinism are
constraints *on* transitions, so they presuppose states. Number presupposes
distinguishable units. Identity (`A = A`) is empty unless `A` differs from
non-`A`. Non-contradiction presupposes negation, which presupposes a
two-sidedness to negate across.

**Why it is a floor and not merely a starting point.** Denying distinction
requires distinguishing the denial from its negation. Self-refuting denial is
the only test a primitive can pass.

**Why not time.** `[ARGUMENT — not a theorem]` The candidate rival is
succession. The asymmetry is:

| | coherent? |
|---|---|
| distinction without time — a static two-mark configuration | **yes** |
| time without distinction — pure duration, nothing to order, nothing changing | **no** |

A static configuration carries marks and boundaries with no succession;
mathematics is full of such objects. Strip time of everything ordered and no
fact about passage remains — it is indistinguishable from no time. And the
moment one says "two moments", they have been distinguished: `before ≠ after`
*is* a distinction.

**Therefore time is composite, not primitive: time = distinction + order.**
If time carries its own `before/after` internally — which is what makes it
attractive as a primitive, since it needs no distinguisher — then it already
contains a distinction and is not the floor.

**What this rests on**, neither forced: (i) that a static configuration is
genuinely timeless, i.e. structure is independent of apprehension — a strict
verificationist rejects this; (ii) that "time with nothing to order" is not
time — a substantivalist about spacetime rejects this.

**The distinguisher problem is NOT resolved here.** "Draw" is transitive. If
distinction requires a drawer, the primitive is a relation and an observer has
entered at the ground floor; if it does not, distinction must be self-standing,
which is a metaphysical commitment rather than a result. This document takes no
position and flags the fork.

## 2 · The chain

Two operations are used throughout, and only two: **ADD** and **SQRT**.

### Step 0 — distinction `[PRIMITIVE]`
Yields the mark.

### Step 1 — presence and two-sidedness `[SELECTION — narrow]`

**Revised 2026-08-04.** This step first read *"count the mark as a value —
admitting the boundary itself as a third value"*. That is **withdrawn**: in a
discrete topology every subset is clopen, so every boundary is empty (§5.1 (i)).
The replacement does not use boundaries at all:

- **the `0`** — a site either carries a mark or does not. Forced once marks have
  **finite support on an unbounded substrate**, which is P1's own finitude
  clause: almost every site is unmarked, so a value for *nothing here* is
  required.
- **the `±1`** — a mark *is* a distinction, and a distinction is **two-sided**.
  A one-sided distinction distinguishes nothing.

Together: **`{−1, 0, +1}`**. The residual selection is that the two sides are
*labelled* rather than interchangeable — if they were interchangeable, nothing
would have been distinguished — which is close to analytic but is not proved
here.

Output: the ternary set, and on it exactly one nontrivial operation — negation.

### Step 1.5 — order `[ADDED STRUCTURE — not derived from step 0]`
Distinction alone gives no sequence. Order is a second structure, and it is
what §1 identifies as time's other half.

### Step 2 — ADD, iterated `[THEOREM — elementary]`
Closing `{−1, 0, +1}` under addition yields **ℤ**.

### Step 3 — SQRT of the only operation available `[THEOREM — elementary]`
Negation is `n : x ↦ −x`. Ask for a linear `R` with `R ∘ R = n`.

- On the three-element set: `R(1) = 0, R(0) = −1` gives `R²(1) = −1`, but
  oddness then forces `R(−1) = 0 = R(0)`. **Not injective — none exists.**
- On `ℝ`: `R(x) = ax` needs `a² = −1`. **No real solution.**
- On `ℝ²`: the quarter-turn `J = [[0,−1],[1,0]]` satisfies `J² = −I`. **Exists.**

**The minimal real space in which negation has a linear square root is the
plane.** Two dimensions are *forced* by the demand, not chosen by pointing in a
direction.

The demand itself is a selection — but a narrow one: at step 1 there is exactly
one nontrivial operation, and SQRT is the only available move that is not ADD.

### Step 4 — the algebra generated `[THEOREM — standard]`
`ℝ[I, J] ≅ ℂ`. **`i` is not a number and not primitively a rotation — it is the
half of reversal.** Everything else about it is consequence.

### Step 5 — close under ADD and multiplication by `i` `[THEOREM — elementary]`

> **ℤ[i] is the closure of the ternary set under ADD and √(negation).**

Output: the Gaussian integers, and the order of the quarter-turn,
**`|⟨i⟩| = 4`**.

### Step 6 — how many copies `[SELECTION — FTD-0355]`
Nothing in steps 0–5 says do it three times. See §5.2.

### Step 7 — which lattice `[SELECTION]`
Adjacency fixes a difference operator and its Green's function — both forced —
but **which** lattice decides whether `Γ(1/4)` appears at all. See §4.

### Step 8 — Laplacian → Green's function → Watson integral `[THEOREM — classical]`
Forced once the lattice is fixed. Watson 1939.

## 3 · The ledger

| # | act | status |
|---|---|---|
| 0 | distinction | **primitive** |
| 1 | presence (finite support) + two-sidedness → `{−1,0,+1}` | **SELECTED — narrow** |
| 1.5 | order | **ADDED** |
| 2 | ADD → `ℤ` | forced |
| 3 | SQRT of negation → the plane | forced *given the demand* |
| 4 | quarter-turn → `ℂ` | forced |
| 5 | closure → `ℤ[i]`, `\|⟨i⟩\| = 4` | forced |
| 6 | `D = 3` | **SELECTED** |
| 7 | BCC vs SC vs FCC | **SELECTED** |
| 8 | Laplacian → Green's function | forced |

**Three selections and one added structure.** Everything else is elementary or
classical mathematics.

## 4 · Where each constant enters

| constant | enters at | status |
|---|---|---|
| ternary `{−1,0,+1}` | step 1 | selected — narrow (sides are labelled); cf. FTD-0128, which instead grounds it in Axiom 0 |
| **4** `= \|⟨i⟩\| = \|Aut(E)\|` | step 5 | **forced** |
| **16** `= 4²` — the master quadratic's coefficient | step 5, squared | **forced**, given the spine's identification |
| `D = 3` | step 6 | selected (FTD-0355) |
| **`G*`** | step 7 | **selected — via the choice of BCC** |

**The `G*` row is the load-bearing one.** Verified to 15 digits:

| lattice | Watson integral | carries `Γ(1/4)`? |
|---|---|---|
| **BCC** | `Γ(1/4)⁴/(4π³) = G*²/2π = 1.39320392968568` | **yes** |
| SC | `1.5163860592` — a `Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)` product | no |
| FCC | `1.3446610732` | no |

`G* = Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2) = 2.95867511918864`, and
`G*²/2π` reproduces the BCC value exactly.

**So the chain reaches `G*` only through the choice of BCC.** This is an
independent confirmation of FTD-0794's *"`G*` enters FTD by choice"*, arrived at
from a different direction: FTD-0794 closed the **clock** door; this shows the
**Green's-function** door is also a selection, differently located. The
BCC → `Γ(1/4)` link is genuine classical mathematics; the *selection of BCC* is
not derived.

Note the SC Watson integral is the repo's `K_GENESIS = 1.5163860592`, so the
production engine's genesis threshold sits on the lattice that does **not**
carry `Γ(1/4)`.

## 5 · Two shave candidates — stated, not registered

### 5.1 `P3 ⟸ P1` — **WITHDRAWN 2026-08-04, same day it was drafted**

The original text proposed that P3 (ternary states) might not be independent of
P1, via *"a lattice requires distinguishable sites; counting the mark yields
three values"*, blocked on a selection called *the boundary is a value*.

**It is withdrawn for three independent reasons, any one sufficient.**

**(i) The move is unavailable on a discrete space.** In a discrete topology
*every* subset is clopen, so `∂A = cl(A) \ int(A) = A \ A = ∅` for every `A`.
**Every topological boundary on a discrete lattice is empty.** There is no
boundary to promote to a value. The route was not merely unproven — it was
ill-posed for the ontology it was proposed for.

**(ii) The shave already exists.** FTD-0128
(`FOUND_TERNARY_STATE_FROM_I.md`, 2026-07-03, `[SYNTHESIS]`) already grounds the
ternary values in **Axiom 0 ("i exists")** —
`i² = −1`, `|i²| = +1`, `0` the additive identity, so
`s ∈ {i², 0, |i²|} = {−1, 0, +1}` — and states outright that the axiomatic
footprint shrinks by one independent numerical choice. The proposal was
redundant.

**(iii) P3's independent content is not the ternary values.** P3 posits
`J ∈ ℝ³` continuous per voxel, `s ∈ {−1,0,+1}` ternary, **and** that `J` is
primary with `s` its threshold projection. By that third clause `s` is
*already* declared non-independent **inside P3**. The free content of P3 is the
**continuous flux field**, which neither FTD-0128 nor this chain addresses, and
which is a strictly harder problem: deriving `ℝ³`-valued continuity from an
uncontained discrete lattice.

### 5.1a What is actually on the table — **Axiom 0**, not P3 `[CONJECTURE]`

FTD-0128 and this chain run in **opposite directions**:

| | assumes | yields |
|---|---|---|
| FTD-0128 | **Axiom 0 — "i exists"** | ternary |
| this chain (§2) | distinction + the √(negation) demand | ternary **and** `i` |

FTD-0128 *posits* `i`. §2 step 3 *derives* it, on the theorem that no linear
square root of negation exists below two dimensions. **So the available shave is
Axiom 0.**

Booked honestly as a trade:

- **out:** Axiom 0 ("i exists")
- **in:** distinction + one closure demand (*negation has a half*)

This is a reduction **only if distinction counts as free** — defensible, since
any formal system capable of stating Axiom 0 already presupposes distinction,
but that is the entire question and it is not settled here. **No reduction in
the axiom count is registered.**

### 5.2 `D = 3` rigidity `[CONJECTURE — numerically unique, not forced]`

The registered route is `|Aut(E)|² = 2^D(D−1)!`. That right-hand side is the
order of the **stabilizer of one axis in the hyperoctahedral group `B_D`** —
flip that axis (2), permute and flip the remaining `D−1` (`2^(D−1)(D−1)!`).

| D | 1 | 2 | **3** | 4 | 5 | 6 |
|---|---|---|---|---|---|---|
| `2^D(D−1)!` | 2 | 4 | **16** | 96 | 768 | 7680 |

**16 occurs uniquely at `D = 3`**, and 16 is forced from step 5. The arithmetic
is rigid.

**What is not established is why the equation should be demanded.** Nothing in
the chain requires an automorphism count to equal an axis stabilizer. A
uniquely-satisfied relation is not a forced one — this is the *selection
principle laundering* failure mode, and FTD-0355's `[SELECTION — declared]` tag
is correct and stands.

## 6 · What this does not establish

- **No physics.** The chain terminates at a lattice Green's function. It
  supplies no dynamics, no clock, no carrier, no `α`.
- **`G*` is not derived** — §4 shows the opposite, and agrees with FTD-0794.
- **The distinguisher problem is open** (§1).
- **The primitive is argued, not proved.** Uniqueness of a primitive cannot be
  proved: the metatheory would need its own primitives. The self-refuting-denial
  argument establishes that distinction is *inescapable*, which is strictly
  weaker than *unique*.
- **Steps 2–5 are elementary.** That `ℤ[i]` follows from ADD and √(negation) is
  a restatement of ordinary algebra in a chosen vocabulary. Its value is
  bookkeeping — it shows where the free choices are — not novelty.

## 7 · Against the Number-One Goal

- **Derive** — steps 2–5 and 8 are `[THEOREM]`, but they build mathematics, not
  physics. No claim is promoted.
- **Mark and price the boundary** — this is the document's actual contribution:
  the framework's foundations cost **three selections** (boundary-as-value,
  `D = 3`, lattice type) plus **one added structure** (order). The `G*` selection
  is newly *located* — at the lattice choice, not only at the clock.
- **Drive** — §5 names the priced lines and the exact obstruction on each.
  `P3 ⟸ P1` was **withdrawn the day it was drafted** (§5.1): the move is
  ill-posed on a discrete space, the shave already existed as FTD-0128, and
  P3's independent content is the continuous flux rather than the ternary
  values. What replaces it (§5.1a) is narrower and better posed: **the chain
  derives `i`, where FTD-0128 assumes it**, so the candidate shave is
  **Axiom 0** — held at `[CONJECTURE]`, and a reduction only if distinction
  counts as free.
