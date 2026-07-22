# Theorem — Unordered FTD-0084 data cannot select `n=11`

**LEDGER:** FTD-0397.  
**Tag:** [THEOREM — scoped to permutation-invariant data].  
**Lock:** `preregister-n11-order-type-no-go-v1`, commit `7012c12b`; preregistration SHA256 `c2f33663b583f6012ab3e4a8029904493850b5c8f4e447600f1301e0ef713c1d`.  
**Verifier:** `scripts/proofs/proof_n11_order_type_no_go.py`, SHA256 `49b8e5b0939547e3cb73635e79e190cf34ba9aea3ecb312f3a52ffcd1cfdb707`.  
**Frozen outcome:** PROVEN-SCOPED.

## 1. Statement

Let `O` be the set of distinct linear orderings of the FTD-0084 multiset `{3,3,4,6}`. The symmetric group `S4` acts by permuting positions. Then:

1. `O` has 12 elements and is a single `S4` orbit.
2. Every position-permutation-invariant function on `O` is constant.
3. Therefore unordered FTD-0084 data and any data determined only by that multiset cannot select one ordering or one cumulative ladder position.
4. With the pre-existing ladder readout `n(o)=4+o_0+o_1`, the possible values are `{10,11,13,14}`. Choosing `11` requires an additional order-bearing datum or independently derived symmetry-breaking dynamics.

This leaves FTD-0084's multiset theorem unchanged and keeps the electron exponent `n=11` at `[SELECTION]`.

## 2. Proof

There are `4!/2!=12` distinct orderings because only the two entries equal to 3 are interchangeable. Acting on the base ordering `(3,3,4,6)` with all 24 position permutations generates all 12 distinct orderings, so the action is transitive.

Let `f:O->X` be invariant: `f(g.o)=f(o)` for every `g in S4`. For arbitrary `o_1,o_2 in O`, transitivity supplies `g` with `g.o_1=o_2`. Hence

```text
f(o_2) = f(g.o_1) = f(o_1).
```

Thus `f` is constant. In particular, an invariant Boolean selector can select either zero orderings or all 12; it cannot select exactly one. The verifier independently constructs the one orbit component and recomputes these selector cardinalities as `[0,12]`.

The 12 exact ladder readouts are:

| `n` | orderings |
|---:|---:|
| 10 | 2 |
| 11 | 4 |
| 13 | 4 |
| 14 | 2 |

Representative multiset invariants—sorted tuple, multiplicity vector, sum, product, squared power sum, and all elementary symmetric polynomials—recompute to one signature. Those examples are checks; transitivity carries the universal quantifier over every invariant function.

## 3. Correctness and non-vacuity

All frozen gates passed. The verifier generated 24 group elements, 12 orderings, one orbit of size 12, the exact position table above, and byte-identical duplicate output. Its only comparisons are exact integer/set equalities. It contains no empirical target or fitting computation.

The order-bearing control `o_0` takes values `{3,4,6}` across the orbit. The instrument therefore detects the distinction between invariant and order-bearing data; constancy is not hardcoded.

## 4. Scope

This theorem excludes one information class, not every future mechanism. It does not prove that `n=11` is false, that no physical ordering exists, or that native dynamics can never break the permutation symmetry. It proves only:

> A theorem about an unordered multiset cannot, without additional order-bearing content, be used as a theorem selecting one of its orderings.

FTD-0390 previously showed that the particular S1/S2 choice was reverse-engineered rather than independently forced. FTD-0397 supplies the stronger target-blind reason: no permutation-invariant refinement of FTD-0084's unordered data can repair that selection. A future promotion attempt must derive genuinely order-bearing dynamics under a fresh lock.

Raw execution record: `engine/results/n11_order_type_no_go_2026-07-20/verifier.txt`.
