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

### Step 1 — count the mark as a value `[SELECTION — the boundary is a value]`
A distinction yields inside and outside — that is *binary*. Admitting the mark
**itself** as a third value yields **`{−1, 0, +1}`**. Natural in a calculus
where the mark is an object (Spencer-Brown), but it is a move, and §5 turns on
exactly this.

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
| 1 | the boundary is a value → `{−1,0,+1}` | **SELECTED** |
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
| ternary `{−1,0,+1}` | step 1 | selected (boundary-as-value) |
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

### 5.1 `P3 ⟸ P1` `[CONJECTURE]`

P1 posits a lattice. A lattice requires distinguishable sites. Distinguishability
is distinction. Counting the mark yields three values. **Therefore P3 (ternary
states) may not be independent of P1**, and the postulate count would fall
**5 → 4**.

**Blocked on step 1.** The derivation needs *the boundary is a value*, which is
a selection. Honest form:

> `P3 ⟸ P1 + (the boundary is a value)`

That trades a full postulate for a smaller declared selection — likely a net
gain, but it **must be booked as a trade, not a free derivation.** Registering
it as a shave without pricing the boundary-as-value move would be the FTD-0788
pattern: satisfying a constraint by quietly adopting something else.

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
- **Drive** — §5 names two priced lines and the exact obstruction on each. The
  `P3 ⟸ P1` line is the more promising: unlike the FC-2 arrow attempt, it is not
  blocked by Loschmidt, and it does not require trading an axiom for a larger
  commitment. It is blocked only on whether *the boundary is a value* can be
  motivated rather than declared.
