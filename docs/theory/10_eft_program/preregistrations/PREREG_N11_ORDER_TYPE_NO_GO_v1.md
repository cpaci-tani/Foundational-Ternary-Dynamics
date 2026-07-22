# PREREG — `n=11` order-type no-go from the FTD-0084 multiset

**Prospective claim ID:** FTD-0397 (registry rechecked at lock time; FTD-0396 is the current maximum).  
**Tag:** [PRE-REGISTRATION — EXACT GROUP-ACTION PROOF] · LOCK-STD v1 · git tag `preregister-n11-order-type-no-go-v1`.  
**Scope:** a new argument class about unordered data and permutation invariance. It is not another ordering search and contains no empirical target.

## 1. Frozen statement

Let `O` be the 12 distinct linear orderings of the unordered FTD-0084 multiset `{3,3,4,6}`. Let `S4` act on `O` by permuting the four positions.

The scoped target is:

> `O` is one `S4` orbit. Therefore every function of the multiset that is invariant under position permutation is constant on `O` and cannot distinguish one ordering or select a cumulative ladder position. With ladder start 4 and the frozen readout `n(o)=4+o_0+o_1`, the attainable positions are `{10,11,13,14}`. Selecting `n=11` requires an additional order-bearing datum or independently derived symmetry-breaking dynamics.

The theorem says nothing about whether future dynamics can derive such data. It does not alter FTD-0084's multiset theorem. The exponent `n=11` remains `[SELECTION]` under every outcome unless a separately locked mechanism supplies order.

## 2. Frozen exact verifier

Instrument: `scripts/proofs/proof_n11_order_type_no_go.py`, frozen with this lock. It must:

1. generate all 12 distinct orderings exactly;
2. generate all 24 position permutations and confirm the base ordering's orbit equals all 12 orderings;
3. confirm the orbit decomposition has one component;
4. recompute cumulative-position counts `{10:2,11:4,13:4,14:2}`;
5. recompute representative multiset invariants (sorted tuple, multiplicities, power sums, product, elementary symmetric polynomials) and find one signature;
6. use orbit components, not the representative list alone, to prove that every invariant Boolean selector is constant and therefore selects either 0 or all 12 orderings;
7. show the order-bearing negative control `o_0` varies over `{3,4,6}`.

The source may contain no particle mass, Planck scale, fine-structure value, CODATA datum, fit residual, tolerance, or empirical viability test. Integer equality is the only comparator.

## 3. Correctness and vacuity gates

| Gate | Frozen requirement | Failure |
|---|---|---|
| G1 | census GREEN and FTD-0397 next at tag cut | lock cannot be cut |
| G2 | multiset multiplicity gives exactly `4!/2!=12` distinct orderings | INVALID |
| G3 | the implemented action is the full `S4` position action and closes on `O` | INVALID |
| G4 | the orbit decomposition has exactly one component of size 12 | INVALID |
| G5 | cumulative positions and multiplicities match §2 exactly | INVALID |
| G6 | forbidden empirical/mass inputs are absent | INVALID |
| G7 | two executions are byte-identical | INVALID |

Vacuity controls: replacing the multiset by four distinct entries changes the expected orbit size to 24 and fails G2; the position readout `o_0` varies across the real orbit, so the verifier can detect an order-bearing function. The invariant-selector theorem passes because the action is transitive, not because every tested function is declared invariant.

Quantifier audit: the general statement “every invariant function is constant” is carried by orbit transitivity: for any `x,y in O`, some `g in S4` has `g.x=y`, hence invariance gives `f(y)=f(g.x)=f(x)`. The listed signatures are adequacy checks, not the proof's quantifier basis.

## 4. Frozen outcomes and partition

Correctness gates have absolute precedence.

1. **PROVEN-SCOPED:** all gates pass, `O` is one orbit, and no invariant singleton selector exists.
2. **COUNTEREXAMPLE:** all representation gates pass and an order is selected using only position-permutation-invariant native data.
3. **INVALID:** any correctness gate fails or the action/multiset theorem is misrepresented.

Partition: evaluate INVALID first. On a valid representation, the exact predicate `invariant_singleton_exists` is Boolean: true gives COUNTEREXAMPLE and false gives PROVEN-SCOPED. No dataset fires two rows. No numerical ties exist; exact set equality is the tie-break. Normative criteria and this precedence outrank prose.

## 5. Licensed interpretation and propagation

PROVEN-SCOPED licenses only the stated information-theoretic/group-action no-go: unordered permutation-invariant data alone cannot choose an order. It does not say `n=11` is false and does not exclude a future native order-bearing dynamics. COUNTEREXAMPLE must exhibit the invariant datum and exact singleton. INVALID licenses nothing.

On PROVEN-SCOPED, the result commit must update LEDGER, tracker, META index, manifest, and the canonical electron-mass status document. The reconciliation commit must audit the corpus for stale `n=11 [DERIVED]`, `MC-T3.2-closed`, and S1/S2-forced language, correcting active documents without deleting provenance. Lock, result, and reconciliation remain separate commits.

## 6. Execution window and executor

Executor: the current Codex repository session on branch `codex/invariant-quotient-roadmap-2026-07-20`. Window: tag creation +72 hours. A missed execution or unbooked verdict creates F10 debt and blocks the next lock.

**LOCKED CONTENT ENDS HERE.** Preregistration and instrument SHA256 values are recorded in `REF_PREREGISTER_MANIFEST.md`; normative changes require v2.
