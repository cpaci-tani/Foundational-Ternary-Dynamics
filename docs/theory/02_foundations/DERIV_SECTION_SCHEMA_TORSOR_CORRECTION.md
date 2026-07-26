# DERIV — The torsor correction: the symmetric-fiber sharpening refuted at the L² wall and repaired

**Tag:** `[DERIVED]` (the refutation of the literal sharpening at I5) + `[THEOREM]` (the torsor structure facts, elementary/standard, machine-verified) + `[SYNTHESIS + CONJECTURE]` (the repaired universal reading).
**LEDGER id:** FTD-0518 · **Date:** 2026-07-25
**Corrects:** [`DERIV_RECORD_STATE_DICHOTOMY.md`](DERIV_RECORD_STATE_DICHOTOMY.md) (FTD-0515) §4, which named the L² wall as the natural attack point of its own conjecture. The attack was run; the conjecture's literal form died there and a strictly more precise form survives. Inputs at tags of record: FTD-0208, FTD-0508 instance table, FTD-0336.
**Verification:** `scripts/proofs/proof_l2_torsor_correction.py` (4/4 PASS, 2026-07-25).

---

## 0 · Verdict up front

FTD-0515 §4 conjectured: *owned = symmetric functions of each wall's fiber; imported = the fiber's ordering datum*, and flagged instance I5 (the L² wall) as the place to attack it. Executed here:

- **Refuted (literal form).** At I5 the fiber over a coset `v + W` is infinite — a torsor over `W` — and the invariants of the *full symmetric group* on such a fiber are the constants alone (any two points are swapped by some permutation). "Owned = Sym(fiber)-invariants" therefore degenerates to the trivial algebra at I5 and cannot equal what the substrate actually owns there (the quotient data). The universal literal reading is dead. `[DERIVED — dimension-count refutation, machine check T2]`
- **Repaired (torsor form).** Replace the symmetric group by the fiber's *structure group* — the group whose action encodes exactly the information the forward map destroyed. The corrected reading: **owned = the invariant algebra of the structure-group action on the fiber; imported = a point of the torsor (a section breaking the orbit).** This form is exact at every instance, including I5, and reduces to the old one precisely where the fiber is a finite orbit of valued points. `[SYNTHESIS + CONJECTURE at universal level]`

The falsification protocol worked as designed: the sharpening was registered with its own kill condition, the kill condition was fired, and the correction is registered as a correction — the predecessor's §4 is superseded, not silently rewritten.

## 1 · The refutation at I5 `[DERIVED]`

The I5 collapse is `q : V → V/W`. Its fiber over a coset is an affine space: infinitely many points, none distinguished. The literal sharpening asks for the symmetric functions of this fiber — invariants under *all* permutations of fiber points. On any sample of `k ≥ 2` points, every pair is exchanged by a transposition, so a fully permutation-invariant function is constant on the sample (T2, exhaustive over pairs at `k = 5`); passing to the full fiber only enlarges the permutation group. The Sym-invariant algebra is therefore trivial, while the owned data at I5 — the coset labels, the entire quotient — is not. The literal form fails not by owning too much but by *predicting the wrong owned algebra*: symmetric functions are the correct invariants only when fiber points carry distinguishing values that the group permutes (roots of a polynomial, concrete pre-merge states), which an affine fiber's points do not.

## 2 · The torsor structure of the I5 import `[THEOREM — standard, machine-verified]`

The linear complements to `W` in `V` are the graphs of `Hom(V/W, W)`, and that vector group acts on them **freely and transitively**: the difference of two complements is a unique `ψ ∈ Hom(V/W, W)` (T1). The shears `g = I + ψ∘π` fix `W` pointwise and carry any complement to any other, so **no complement is invariant under the stabilizer of `W`** — a canonical choice cannot exist without new structure. Supplying an inner product creates exactly one fixed point of the orthogonal stabilizer: the orthogonal complement. The FTD-0208 import is therefore, in the schema's corrected vocabulary, *the structure that turns a free transitive orbit into a pointed one* — and the machine check exhibits both halves: shears killing every candidate, the inner product creating the fixed point.

## 3 · The repair, checked at every instance

| Instance | Fiber | Structure group | Owned = invariants of the action | Imported = a section/point |
|---|---|---|---|---|
| I1 merge | `m` pre-merge states | `Sym(m)` permuting valued points | symmetric functions of the fiber (the merged output) | the branch digit |
| I2 transport | current class | translation by `ker(div)` (a torsor) | the endpoint class (base datum) | one 1-chain in the coset |
| I3 dressing | primitives of `ω` | translation by closed forms | the holonomy (orbit invariant) | a global primitive — nonexistent |
| I4 record | histories over an output | rotation-generated trace equivalence | the cyclic-class algebra (FTD-0515 shelf) | a separating (non-tracial) state |
| I5 L² | coset of `W` / complements | translation by `W` / `Hom(V/W, W)` (torsors) | constants per fiber = exactly the quotient data (exact over `F₅`: invariant dimension = #cosets, T3) | one complement; created by the inner product |
| I6 δ | `{x₊, x₋}` | Galois `ℤ/2` | symmetric functions `e₁, e₂ ∈ ℚ(G*)` (T3b) | the root order, carrying `δ` |

The corrected universal reading — **every wall presents a torsor over the group measuring the forward map's information loss; the substrate owns the orbit invariants; the import is a point** — is registered `[CONJECTURE]`, superseding FTD-0515 §4's form. It is sharper in two ways. First, it is now *literally* the geometry of a gauge choice: a section of a torsor bundle is a connection-type datum, which aligns I3/IMP-S4 and the frontier's "chosen adjoint" under one standard construction rather than by analogy. Second, its kill condition is cleaner: **exhibit one wall whose fiber carries no transitive structure-group action, or whose owned algebra exceeds the orbit invariants.** The dispersion boundary (FTD-0270, the frontier's one ATTEMPTED row) is now the least-formalized instance and hence the natural next attack point.

## 4 · Status line

Literal symmetric-fiber sharpening: `[REFUTED at I5 — superseded]` (its I6 arithmetic content — `e₁, e₂ ∈ ℚ(G*)`, `x₊−x₋ = 8G*δ` — is untouched and remains machine-verified). Torsor facts `[THEOREM — standard, machine-verified 4/4]`. Corrected universal reading `[CONJECTURE]`. Nothing else moves: FTD-0208 stands, FC-1/FC-2 declined, FC-W adopted, `x₊ = 1/α` `[SMC]`, forcing theorem `[OPEN]`. A conjecture registered one session ago was attacked at its own named weak point and corrected the same day — the discipline's intended lifecycle.
