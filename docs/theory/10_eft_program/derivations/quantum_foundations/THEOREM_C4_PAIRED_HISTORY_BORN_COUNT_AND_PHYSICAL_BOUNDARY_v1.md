# C4 paired-history Born count and physical boundary v1

**Date:** 2026-08-23
**Status:** `[THEOREM — EXACT EQUAL-WEIGHT C4 COUNTING IDENTITY]` +
`[THEOREM, CONDITIONAL — UNIFORM ORDERED-PAIR BASINS GIVE NORMALIZED SQUARED WEIGHTS]` +
`[CONJECTURE — PHASE-COMPATIBLE BOND HISTORIES CAN REALIZE THOSE BASINS]` +
`[OPEN — DYNAMICAL CANCELLATION, PAIRING, ACTUALIZATION, GENERAL AMPLITUDES]`
**Physical Born-rule status:** unchanged; not derived
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[`proof_c4_paired_history_born_count.py`](../../../../../scripts/proofs/proof_c4_paired_history_born_count.py)
exhaustively checks bounded integer multiplicities, global C4 phase
invariance, exact destructive cancellation, and normalized multi-outcome
weights. It contains no physical constant and performs no numerical search.

---

## 1. Result

For one outcome \(o\), let \(N_{o,k}\in\mathbb N\) count equal-weight
admissible histories with phase \(i^k\), \(k=0,1,2,3\). Their coherent
response is

\[
 Z_o=N_{o,0}+iN_{o,1}-N_{o,2}-iN_{o,3}
    =a_o+i b_o,                                      \tag{1}
\]

where

\[
 a_o=N_{o,0}-N_{o,2},\qquad
 b_o=N_{o,1}-N_{o,3}.                               \tag{2}
\]

Cancel opposite phases on each of the two quadrature rails. This leaves a
real residual set \(R_o\) of cardinality \(|a_o|\) and an imaginary residual
set \(I_o\) of cardinality \(|b_o|\). Define the compatible ordered-pair set

\[
 \mathcal P_o=(R_o\times R_o)\;\sqcup\;(I_o\times I_o). \tag{3}
\]

The union is disjoint and self-pairs are included. Therefore

\[
 \boxed{
 |\mathcal P_o|=|R_o|^2+|I_o|^2
 =a_o^2+b_o^2=|Z_o|^2.}                             \tag{4}
\]

Equation (4) is exact. The square is not appended as a probability rule: it
is the cardinality of a declared two-history compatibility space after
opposite-phase cancellation.

This is the precise mathematical content of the phrase **phase-compatible
paired histories**. It is also the present boundary: defining
\(\mathcal P_o\) is not evidence that the substrate constructs or samples it.

The later exact
[paired-history phase-neutral actualization theorem](../common_action_mechanics_reciprocity/THEOREM_C4_PAIRED_HISTORY_PHASE_NEUTRAL_ACTUALIZATION_SOURCE_VERTEX_v1.md)
removes one arbitrariness from this definition. On the real C4 quadrature
doublet, the unique normalized symmetric invariant contraction has values
(+1,0,-1) on equal, cross-rail, and opposite phases. Reversible dark
cancellation removes the negative pairs, so (mathcal P_o) is exactly the
positive-contraction basin of that invariant on the prepared residual bank.
Autonomous bank generation, contextual routing, and its physical trial measure
remain open.

---

## 2. Proof

On the real rail, cancel
\(\min(N_{o,0},N_{o,2})\) pairs of phases \(+1\) and \(-1\). The surviving
cardinality is

\[
 N_{o,0}+N_{o,2}-2\min(N_{o,0},N_{o,2})
 =|N_{o,0}-N_{o,2}|=|a_o|.                           \tag{5}
\]

The identical operation on \(+i,-i\) leaves \(|b_o|\). Cardinalities multiply
under Cartesian product and add under disjoint union, giving equation (4).
\(\square\)

Multiplying every history by a common phase \(i^j\) only permutes the four
counts and rotates \((a_o,b_o)\) by a signed quarter-turn. Hence both sides of
equation (4) are globally C4-phase invariant. If opposite-phase counts match
on both rails, then \(Z_o=0\) and \(\mathcal P_o\) is empty: destructive
interference is exact.

---

## 3. Conditional normalized pushforward

Suppose, in addition to the theorem, that the physical equilibrium
microstates leading to outcome \(o\) are in a target-blind bijection with
\(\mathcal P_o\), and that the common equilibrium measure is uniform over the
disjoint union \(\bigsqcup_r\mathcal P_r\). Then

\[
 \mu_{\rm eq}(B_o)
 ={\lvert\mathcal P_o\rvert\over\sum_r\lvert\mathcal P_r\rvert}
 ={|Z_o|^2\over\sum_r|Z_r|^2}.                       \tag{6}
\]

This implication is a theorem. Its antecedent is not established. In
particular, simply drawing a uniform integer and partitioning it into intervals
of lengths \(|Z_o|^2\) would install the Born weights and would not discharge
the physical debt.

---

## 4. Connection to the phase-complete bond candidate

The unadopted
[`phase-complete bond-transaction scope`](../../scopes_and_specs/SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md)
already provides the relevant finite alphabet

\[
 z_b\in\{0,1,i,-1,-i\}.                              \tag{7}
\]

This makes equation (4) structurally compatible with that candidate, but not
yet a consequence of its action. A physical realization would have to derive
all of the following from the same target-blind transaction dynamics:

1. **C4 quantization:** completed admissible histories must contribute equal
   action units with phases in C4, or the dynamics must supply an independently
   justified integer-multiplicity encoding of unequal weights.
2. **Local cancellation:** opposite phase records must annihilate or become
   unavailable through reversible local transactions, with all work and
   surviving records accounted for.
3. **Rail retention:** the real and imaginary residual channels must remain
   physically distinguishable without an external amplitude calculator.
4. **Pair formation:** the dynamics must actually construct the ordered
   compatible pairs in equation (3), including the reason self-pairs count.
5. **Basin bijection:** complete initial microstates in the outcome basin must
   map one-to-one, or with a proved common multiplicity, to those pairs.
6. **Measure:** the physical equilibrium measure must make those basin
   microstates equiprobable without reading the desired outcome weights.
7. **Composition:** independent preparations, coarse blocking, sequential
   measurement, and context changes must preserve the construction.
8. **Operational causality:** the contextual complete-history selector must
   remain compatible with local propagation and no signalling.

Until these gates pass, equation (4) is a native **candidate combinatorics for
why a square could appear**, not a physical Born-rule derivation.

The first dynamical successor is now
[THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md](THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md).
Two local cyclic memories of consecutive lengths advance one site per tick and
visit every ordered record pair exactly once. Conditional on physically
prepared residual C4 records, that target-blind deterministic orbit produces
the exact normalized count in equation (6). Reversible cancellation,
preparation, click work, reset, and general amplitudes remain open, so the
physical status declared here is not promoted to a general Born derivation.
Its
[reversible cancellation/click successor](THEOREM_REVERSIBLE_C4_CANCELLATION_AND_CLICK_CIRCUIT_v1.md)
now supplies the exact dark-record involution and nondestructive comparator.
The remaining boundary is action-generated routing, work-complete
manifestation/reset, general amplitudes, and multipartite causal composition.

---

## 5. Scope boundary

The theorem is narrower than a general path-integral identity. The action in
the bond scope also contains Hamiltonian phases that are not yet proved to lie
in C4, and physical amplitudes need not be nonnegative integer multiplicities.
Rational amplitudes can be represented conditionally by a common-denominator
replication, but irrational and continuously varying amplitudes require a
controlled blocking or limit theorem. None is supplied here.

The subsequent
[Gaussian-integer general-amplitude limit](THEOREM_C4_GAUSSIAN_INTEGER_GENERAL_AMPLITUDE_PHYSICAL_LIMIT_v1.md)
supplies that missing theorem on the prepared finite-outcome branch. Canonical
nearest-Gaussian-integer block sums have an explicit total-variation error
bound and finite record/tape price, while every finite approximant retains the
exact physical $|Z_o|^2$ event count. Native generation of the approximating
record banks, trial competition, and multipartite composition remain open.

The theorem also does not by itself resolve Bell experiments. The framework's
v2 posture remains the one stated in
[`SPEC_FTD_FRAMEWORK_V2_CONTEXTUAL_ACTUALIZATION.md`](../../../01_reference/SPEC_FTD_FRAMEWORK_V2_CONTEXTUAL_ACTUALIZATION.md):
the physical context changes the admissible complete-history ensemble, while
actual propagation remains local. The paired-history count supplies no license
to combine incompatible contexts into one context-independent ontic ensemble,
and it does not prove that the required contextual pushforward exists.

---

## 6. Falsifiers and next gate

Close or demote the physical route if:

- the action produces generic phases with no native C4 reduction or controlled
  multiplicity limit;
- cancellation requires outcome labels, nonlocal bookkeeping, or unbounded
  hidden memory;
- the physical pair basins contain cross-rail or unmatched multiplicities that
  do not reduce to equation (3);
- the equilibrium measure is nonuniform in an outcome-dependent way;
- composition or no-signalling fails; or
- a numerical Born fit succeeds only after tuning the pairing definition.

The next legitimate test is not another fit to \(|\psi|^2\). It is a
**generative paired-history gate**: derive a finite, reversible local
transaction on the bond/action state that performs opposite-phase
cancellation, retains both rails, and produces the pair basin before any
outcome probabilities are computed.
