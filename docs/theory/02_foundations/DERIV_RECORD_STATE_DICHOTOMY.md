# DERIV — The record-state dichotomy: traciality, the necklace shelf, and the symmetric-fiber sharpening

**Tag:** `[THEOREM]` (Theorems A–C, elementary and machine-verified) + `[SYNTHESIS]` (the six-instance sharpening roll-up) + `[CONJECTURE]` (the universal symmetric-fiber reading and the loop-observable identification).
**LEDGER id:** FTD-0513 · **Date:** 2026-07-25
**Deepens:** [`DERIV_FOUR_WALLS_SECTION_SCHEMA.md`](DERIV_FOUR_WALLS_SECTION_SCHEMA.md) (FTD-0508), whose proven edge was algebra-level; this document proves the state-level form and sharpens the schema. Inputs at tags of record: FTD-0499, FTD-0243, FTD-0494, FTD-0336, FTD-0244/0314.
**Verification:** `scripts/proofs/proof_record_state_dichotomy.py` (5/5 PASS, 2026-07-25).

---

## 0 · Thesis

FTD-0508 proved that the record algebra of any fiber-resolving lift of the frozen projection is non-commutative. The frontier's vocabulary for the FC-1 wall, however, is not "non-commutative algebra" but **non-tracial state** (FTD-0336 §2, probabilistic face). This document closes that gap: it proves that on the record monoid, *traciality is exactly constancy on cyclic word classes* (Theorem A); that the quotient state is tracial while **any history-separating state is forced non-tracial** (Theorem B); and that a strict intermediate shelf exists — tracial states finer than the quotient (Theorem C, the necklace states). It then extracts the sharpening these results suggest and checks it against all six schema instances: **the substrate owns the symmetric functions of each wall's fiber; the import is always the fiber's ordering datum.** For the δ wall this is exact and machine-verified: `x₊+x₋ = 16G*²` and `x₊x₋ = 16G*³` lie in `ℚ(G*)`, while the antisymmetric combination `x₊−x₋ = 8G*δ` carries the surd.

Nothing here derives α, moves any FC, or promotes any identification.

## 1 · Theorem A — traciality is cyclic-class constancy `[THEOREM]`

Let the record monoid be the free monoid on `m ≥ 2` digits, acting by the registered control `h′ = m·h + b` (FTD-0499 §3), and let a *state* be a linear functional τ on the span of words. Call τ *tracial* if `τ(uv) = τ(vu)` for all words `u, v`.

**Theorem A.** τ is tracial iff τ is constant on cyclic classes of words.

*Proof.* For `w = uv`, the word `vu` is the rotation of `w` by `|u|`; conversely every rotation of `w` by `k` is the swap of the prefix of length `k` with its complement. So the pairs `{uv, vu}` generate exactly the cyclic equivalence, and constancy on cyclic classes is equivalent to the trace condition. ∎ (Machine check E1, exhaustive to length 6, `m ∈ {2,3}`.)

## 2 · Theorem B — the dichotomy `[THEOREM]`

**Theorem B.** (i) The quotient state — any functional of word length alone, which is all the projected raw output supports (FTD-0499 §1/§4) — is tracial. (ii) Any state that separates all histories is non-tracial.

*Proof.* (i) Rotations preserve length (E2). (ii) For every `m ≥ 2` and length `N ≥ 2` there exist distinct histories in one cyclic class — e.g. the digit strings `01` and `10`, whose radix encodings differ (`1 ≠ 2`); machine check E3 confirms every cyclic class of size ≥ 2 consists of pairwise-distinct histories. A separating state takes distinct values on such a pair, hence is non-constant on a cyclic class, hence non-tracial by Theorem A. ∎

**Corollary (state-level edge).** Purchasing the FC-2 section object requires, at the level of states and not merely algebras, exactly a non-tracial state — the literal object named on the probabilistic face of the frontier table (FTD-0336 §2). The FC-2 ⇒ FC-1 edge of FTD-0508 is now proven at both levels. The scope guard of FTD-0508 §3 carries over unchanged: this is Tomita–Takesaki *vocabulary alignment* at signature level, not a derivation of modular theory, Hilbert space, or the Born rule.

## 3 · Theorem C — the necklace shelf `[THEOREM]`, and its reading `[CONJECTURE]`

**Theorem C.** Strictly between the quotient state and the separating states lies a nonempty shelf: states constant on cyclic classes (hence tracial) yet strictly finer than length-only. Witness at `m = 2, N = 4`: the indicator of the cyclic class of `0011` is tracial and distinguishes `0011` from `0101` — two histories of equal length and equal digit multiset in different cyclic classes. The class count is the necklace number `(1/N)Σ_{d|N} φ(d) m^{N/d}` (= 6 at `m=2, N=4`), strictly between 1 and `m^N`. ∎ (Machine check E4.)

The tracial-accessible closure of the record is therefore exactly the **cyclic-invariant algebra** — more than the quotient, less than the section. The suggestive reading: cyclic invariants of a composition path are *loop observables*, and the one loop observable in the 2026-07-25 engine arc is the FTD-0494 plaquette holonomy — which the substrate *measured natively* (0.438) on the very connection whose global primitive it provably cannot possess. The pattern "the modulus half sees the cyclic shadow of what it cannot own linearly" is registered as `[CONJECTURE — structural reading]`: the monoid-algebra statement and the lattice one-form statement live in different formalisms and no functor between them is exhibited here.

## 4 · The symmetric-fiber sharpening `[SYNTHESIS + CONJECTURE]`

Theorems A–C suggest a sharper form of the schema: what the substrate owns of each wall's fiber is its *symmetric* (permutation-invariant) function algebra; what it imports is the fiber's *ordering datum*. Checked against all six FTD-0508 instances, each fact at its existing tag:

| Instance | Owned (symmetric functions of the fiber) | Imported (ordering datum) |
|---|---|---|
| I1 merge fiber | the merged output — invariant under branch permutation | which branch: the digit `b` |
| I2 transport | the endpoint multiset | the orientation (CW vs CCW = a cyclic *order* on the loop's traversal) |
| I3 dressing | the plaquette holonomy — a loop (cyclic) invariant, natively measured | the linear primitive `C(n)` — provably nonexistent globally |
| I4 record | cyclic-class observables (Theorem C shelf) | the separating (non-tracial) state |
| I5 L² budget | the quotient `V/W` | the choice of complement |
| I6 δ wall | `x₊+x₋ = 16G*²`, `x₊x₋ = 16G*³` — the elementary symmetric functions, in `ℚ(G*)` `[THEOREM, machine check E5]` | the root order: `x₊−x₋ = 8G*δ`, the antisymmetric function, carrying the surd (transcendental over `ℚ(G*)` conditional on Chudnovsky, per FTD-0314) |

The I6 row is the exact Galois prototype: a base field always owns the symmetric functions of a polynomial's roots; selecting a root is the extension. The universal reading — *every FTD wall is a Galois-type extension problem: owned symmetric algebra, imported ordering* — is registered as `[CONJECTURE]`, the sharpened form of the FTD-0336 §3 meta-conjecture. Its value is that it is more falsifiable than the original: **exhibit one wall whose owned half exceeds the symmetric algebra of its fiber, or whose import is not an ordering datum, and the sharpening dies** while the original frontier conjecture may survive. The L² wall (I5) is again the natural attack point, since "choice of complement" is an ordering datum only under a reading (an ordered direct-sum decomposition) that a skeptic may reject.

## 5 · Status line

Theorems A–C `[THEOREM — elementary, machine-verified 5/5]`; state-level FC-2 ⇒ FC-1 edge proven; necklace shelf established. Universal symmetric-fiber reading `[CONJECTURE]`; loop-observable identification `[CONJECTURE — structural reading]`. Nothing promoted: `x₊ = 1/α` stays `[SMC]` (the E5 sanity bound is the 1.26 ppm match of record, not an equality); MC-T4.3 stays `[OPEN — SCOPED NO-GO PACKAGES]`; FC-1/FC-2 declined; FC-W adopted; the four-walls forcing theorem stays `[OPEN]`, now with both of its proven edges at state level.
