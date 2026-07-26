# DERIV — The degeneracy criterion: why sections are imported exactly where symmetry protects the fiber

**Tag:** `[THEOREM]` (the equivariance no-selector direction, elementary and machine-verified) + `[SYNTHESIS]` (the per-wall symmetry roll-up; the vacuum-choice reading) + `[CONJECTURE]` (the converse, supported by three supplied-section instances).
**LEDGER id:** FTD-0566 · **Date:** 2026-07-26
**Sharpens:** the torsor form of [`DERIV_SECTION_SCHEMA_TORSOR_CORRECTION.md`](DERIV_SECTION_SCHEMA_TORSOR_CORRECTION.md) (FTD-0518) by adding the missing *criterion* — which fibers demand imports and which the dynamics sections itself. Inputs at tags of record: FTD-0499/0502/0494 (protected instances), FTD-0503/0549/0551/0552 (supplied instances), FTD-0520 (branch blindness), FTD-0243 (commutativity), FTD-0244/0314 (Galois closures), FTD-0509 (intensive/extensive), FTD-0517 (observer equivalence).
**Verification:** `scripts/proofs/proof_degeneracy_criterion.py` (3/3 PASS, 2026-07-26).

---

## 0 · The criterion

> A wall's section is **self-suppliable** iff the coupled dynamics assigns fiber points different values of some dynamical functional (the fiber is *split*); it **demands an import** iff the fiber's structure group acts by exact symmetries of the dynamics (the fiber is *protected*). `[Forward direction THEOREM below; converse CONJECTURE]`

This is Curie's principle in the schema's vocabulary: a variational selector cannot choose between states an exact symmetry makes identical.

## 1 · Theorem (protection ⇒ no self-supplied section) `[THEOREM — elementary]`

Let the structure group `G` act freely and transitively on the fiber `F`, and suppose every datum available to the substrate is `G`-invariant (the action commutes with the update and preserves every dynamical functional). Then no substrate-definable selector picks a point of `F`.

*Proof.* A selector definable from `G`-invariant inputs satisfies `s(g·d) = g·s(d)` with `g·d = d`, so `s(d)` is a `G`-fixed point of `F`; a free action on a transitive orbit has none. ∎ (Machine check K1: exhaustive for `m = 2, 3, 4`.)

**The hypothesis holds at each protected wall, by results of record:** the merge fiber's permutation of identical preimages is a symmetry of production (FTD-0526's permutation quotient; FTD-0504); the frequency fiber's conjugation commutes with the real update (FTD-0520 G2); the δ fiber's Galois ℤ/2 fixes everything the native calculus reaches — that is precisely the content of FTD-0244/0314's `ℚ(G*)`-closures; the complement fiber's shears fix `W` pointwise (FTD-0518 T1); the record fiber's rotations preserve the quotient (FTD-0515 Theorem A). The four walls plus the record are therefore protected fibers, and the theorem explains their resistance *uniformly*: every closed-negative route in the corpus that searched for a native selector on these fibers was attempting the impossible direction of this theorem. The caveat carried by all such results carries here too: "substrate-definable" is relative to the declared calculi and observables; a mechanism outside every declared class is not quantified over (the MC-T4.3 scoping discipline).

## 2 · The converse instances `[CONJECTURE — three supporting cases, no proof]`

Where the fiber is split, the dynamics has so far always supplied the section by extremization: the free-transport fiber (FTD-0503 — Legendre residual splits permutations); the within-tick schedule fiber (FTD-0549/0551 — schedules matching endpoints, endpoint velocities, *and* midpoint velocity still differ in source moment by exactly `ε/30 = ε·∫τ²(1−τ)²dτ`, machine-verified in exact rationals (K2), and the DG transaction selects one); the subcell position fiber (FTD-0552 and the FTD-0565 toy — corrugation exactly `1/64` at B2 (K3), minima selecting half-cell positions, dynamically resolving what FTD-0500 proved kinematically impossible). The converse — *every* split fiber gets sectioned — is not a theorem; it is the observed pattern, and its failure mode is named below.

## 3 · Consequences `[SYNTHESIS — promotes nothing]`

1. **The forcing theorem's shape.** The four-walls-are-one program (FTD-0508 §5) reduces, under this criterion, to one uniform statement: *the four structure groups are exact dynamical symmetries* — proven piecewise already. What remains open is exactly what was open before (the cross-category transfer), but the criterion explains why the walls cluster: they are the symmetry-protected sector of the substrate's fiber inventory.
2. **Adoptions are vacuum choices.** An import that picks one point of a protected orbit is structurally a spontaneous-symmetry-breaking datum: the dynamics cannot derive it, a boundary condition declares it. FC-W is a choice of vacuum in a two-point Galois orbit. With FTD-0509 this closes into one economics: **vacuum choices are intensive (O(1)); history tracking is extensive (O(N)); the framework adopts vacua and declines histories** — the same policy every spontaneously-broken physical theory enacts.
3. **Triage value.** Any future "native derivation of X" claim can be triaged in one step: exhibit the fiber and ask whether its structure group is a dynamical symmetry. If yes, the claim must either break the symmetry somewhere (find it) or smuggle the section (find that).

## 4 · Falsifiers

- **Kill the theorem's reach:** exhibit a substrate-definable selector on a protected fiber — equivalently, a dynamical functional the structure group fails to preserve (this would also break the cited symmetry result of record for that wall).
- **Kill the converse:** exhibit a split fiber whose section the dynamics provably cannot supply — an energy-split orbit with no realizable extremizing selection. The FTD-0552 *subtraction* branch is a live candidate: if the self-force must be subtracted for consistency elsewhere, the position fiber would be split-yet-unsectioned in the repaired theory.
- **Kill the elegance honestly:** the criterion's chief risk is aesthetic capture — it is loved most where it is tested least. The L² wall's "energy degeneracy" (no functional without an inner product) is the thinnest instance and the right place to attack first.

## 5 · Status line

Forward direction `[THEOREM — elementary, machine-verified 3/3]` with per-wall hypotheses at their existing tags; converse `[CONJECTURE — 3 instances]`; vacuum-choice reading `[SYNTHESIS]`. Nothing promoted: FC-1/FC-2 declined, FC-W adopted, `x₊ = 1/α` `[SMC]`, MC-T4.3 `[OPEN — SCOPED NO-GO PACKAGES]`, forcing theorem `[OPEN]`. The criterion is the four-walls program's first candidate *law*; it should be attacked before it is cited.
