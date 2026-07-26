# DERIV — D = 3 is independent of the dimension-blind register fragment

**Tag:** `[THEOREM — fragment independence, elementary]` + `[DERIVED — conditional corollary on the type-priority commitment]` + `[OPEN — unconditional status of D]`.
**LEDGER id:** FTD-0510 · **Date:** 2026-07-25
**Relation to record:** sharpens FTD-0355 (D = 3 `[SELECTION — declared]`; the automorphism route's circularity a declared permanent feature) from an audit finding into a structural necessity; companion to [`DERIV_D3_FROM_AUTOMORPHISM.md`](DERIV_D3_FROM_AUTOMORPHISM.md), whose arithmetic half is untouched. No tag on any existing line moves.
**Verification:** `scripts/proofs/proof_d3_fragment_independence.py` (3/3 PASS, 2026-07-25).

---

## 0 · Purpose

The Number-One Goal requires that a priced line never rest as merely booked: it retires by derivation, hardens to a theorem-grade no-go, or acquires a sharper falsifier. The D = 3 line has stood since FTD-0355 as `[SELECTION — declared]` with a named circularity in its best-known forcing route. This document drives the line to a scoped no-go: it proves that *no* route confined to the dimension-blind fragment of the axiom register can force D = 3 — the circularity FTD-0355 found in one route is not an accident of that route but a necessity of the fragment.

## 1 · The dimension-blind fragment

Write the postulate register as a schema in the dimension parameter. P1(D): an uncontained, undefined-boundary discrete lattice of spatial rank D (at every specified position, axis-adjacent sites exist along each of the D axes). P2 (discrete time), P3 (ternary states), P4 (Moore-neighborhood local causality), P5 (determinism) contain no reference to D beyond generic rank. Define the *dimension-blind fragment* F as {P1(D) with D a free parameter} ∪ {P2, P3, P4, P5} together with every Framework Commitment whose statement carries no D-dependence.

The canonical register instantiates F at D = 3 via P1 = P1(3). The question is whether the instantiation is forced by F itself.

## 2 · Theorem (fragment independence)

**Theorem.** D = 3 is not a consequence of F: the fragment has models at D = 1, 2, 3, 4 (and, by the same construction, at every positive integer rank).

*Proof.* By the model-existence criterion, a statement is not derivable from a fragment if the fragment has a model in which the statement fails. Exhibit, for each D, the following structure: state space {−1, 0, +1} on the rank-D lattice; update rule "next state at a site is the sign of the sum of the site's own state and its Moore-radius-1 neighbors, with sign(0) = 0"; synchronous ticks. This rule is deterministic (a function of the neighborhood), local (radius 1 by construction), ternary-closed, and preserves finite support over the tested horizon; the same rule schema — with no branch on D anywhere in its definition — satisfies every axiom of F at each tested rank. The verification script instantiates the identical code path at D = 1, 2, 3, 4 and confirms each fragment property directly, including an exhaustive locality probe (no perturbation at Moore distance ≥ 2 alters the origin update). Since F has a model at D = 4 (among others), "D = 3" fails in a model of F and is therefore not derivable from F. ∎

The theorem is elementary; its value is what it makes structural.

## 3 · Corollary (every forcing route must import D-dependence)

**Corollary 1.** Any derivation of D = 3 must invoke at least one premise outside F — that is, a premise whose statement carries D-dependence. Available premise classes: (i) P1(3) itself, which makes the derivation circular; (ii) a D-dependent selected or imported content item, which relocates rather than discharges the selection.

*Proof.* Immediate from the theorem: a sound derivation from D-blind premises alone is excluded by the D = 4 model. ∎

This is the FTD-0355 finding upgraded from instance to necessity. The automorphism route's RHS target 16, justified via |O_h|/D = 48/3, is a class-(i) premise (O_h is the point group of the D = 3 lattice); the audit's "circularity named" is now "circularity forced": the route could not have been otherwise, and neither can any successor route built inside F. A future claimed derivation of D = 3 can be triaged in one step — locate its D-dependent premise; the theorem guarantees one exists.

**Corollary 2 (conditional on the type-priority commitment, FTD-0339).** Under the adopted principle that context is prior to and precondition for the value of content, the lattice rank D is part of the lattice *type*: a precondition-type, adopted rather than derived, exactly as the Framework Commitments are. The `[SELECTION — declared]` tag of record is then not a temporary embarrassment awaiting a cleverer route but the *correct terminal status* within the current ontology — `[DERIVED — conditional on FTD-0339; the commitment is itself adopted, not proven]`.

## 4 · Placement and honest scope

Three restrictions. First, the no-go is fragment-relative: a derivation of D = 3 from *new* honest types — a future adoption enriching the register at a declared price — remains possible and is Front B's business; this document closes only the free routes. Second, the theorem concerns derivability, not plausibility: nothing here weakens the arithmetic uniqueness `f(D) = 2^D(D−1)! = 16 ⟺ D = 3` (rechecked, D ≤ 50), which stands as the `[THEOREM]` half of the automorphism document and remains suggestive content *given* D-dependent input. Third, this no-go sits one meta-level above the modulus/argument frontier (FTD-0336): the frontier maps what a *fixed* instantiation cannot self-supply; this document maps a datum of the instantiation itself. Both instantiate the same adopted principle — content cannot bootstrap its context — but the frontier's §3 meta-conjecture is neither used nor advanced here.

**Falsifier.** Exhibit a sound derivation of D = 3 all of whose premises are dimension-blind. By the theorem this requires refuting the model construction — e.g., showing the D ≠ 3 models violate some F-axiom under a reading of P1(D) not captured here. The construction is deliberately minimal to make that attack surface small and precise.

## 5 · Status line

Fragment-independence `[THEOREM — elementary, machine-verified]`; forcing-route triage `[THEOREM — corollary]`; terminal-status reading `[DERIVED — conditional on FTD-0339]`. D = 3 itself: unchanged, `[SELECTION — declared]` (FTD-0355), now backed by a scoped no-go rather than a single named circularity. The import ledger's selected-type line for D = 3 is not repriced. Nothing promoted.
