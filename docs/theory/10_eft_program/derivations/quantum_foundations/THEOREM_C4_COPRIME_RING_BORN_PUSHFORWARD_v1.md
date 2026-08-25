# C4 coprime-ring Born pushforward v1

**Date:** 2026-08-23
**Status:** `[THEOREM — EXACT DETERMINISTIC COPRIME-ADDRESS PAIR ENUMERATOR]` +
`[THEOREM, CONDITIONAL — FINITE EQUAL-WEIGHT C4 CLICK FREQUENCIES]` +
`[SELECTION CANDIDATE — SHARED-BANK L/R DETECTOR MEMORY REALIZATION]` +
`[OPEN — REVERSIBLE CANCELLATION ACTION, PREPARATION, WORK, RESET, GENERAL AMPLITUDES, MULTIPARTY CAUSALITY]`
**Physical Born-rule status:** advanced to an explicit finite deterministic
pushforward candidate; the general physical derivation remains open
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[`proof_c4_coprime_ring_born_pushforward.py`](../../../../../scripts/proofs/proof_c4_coprime_ring_born_pushforward.py)
checks the complete joint orbit for consecutive ring lengths through 32,
exhausts all one-outcome C4 multiplicities from 0 through 5, and verifies
multi-outcome normalized frequencies using exact integer and rational
arithmetic. It performs no fit and contains no physical target value.

---

## 1. What is new here

The
[`C4 paired-history counting theorem`](THEOREM_C4_PAIRED_HISTORY_BORN_COUNT_AND_PHYSICAL_BOUNDARY_v1.md)
proved the identity

\[
 |\mathcal P_o|=|Z_o|^2,                              \tag{1}
\]

but did not give the substrate a mechanism that visits the members of
\(\mathcal P_o\). This document supplies the minimum visible deterministic
enumerator. It does not sample from a table of Born weights. Two cyclic address
heads advance by the same target-blind one-hop rule over one shared record
bank, and their consecutive periods force them to visit every ordered pair.

---

## 2. Residual C4 records

For physical outcome port \(o\), let

\[
 Z_o=N_{o,0}+iN_{o,1}-N_{o,2}-iN_{o,3}
    =a_o+i b_o.                                      \tag{2}
\]

After opposite-phase cancellation, the active real and imaginary record sets
have cardinalities

\[
 r_{o,R}=|a_o|,\qquad r_{o,I}=|b_o|.                 \tag{3}
\]

Records remain distinguishable even when their phase and port labels agree.
Let

\[
 \mathcal R=\bigsqcup_o(R_o\sqcup I_o),\qquad
 N=|\mathcal R|.                                     \tag{4}
\]

Choose any fixed hardware capacity \(L\geq\max(1,N)\). Load the active records
once into a shared \(L\)-slot physical memory bank. Two cyclic address heads
read that bank:

- head \(A\) has period \(L\) and reads address \(x_n\);
- head \(B\) has period \(L+1\), reads address \(y_n\) for \(y_n<L\), and
  encounters one explicit delay/blank cell at \(y_n=L\);
- unused bank addresses are explicit blanks.

No history record is duplicated. The two heads are independent physical read
contexts for the same ontic memory. Preparing and clearing that memory still
requires a reciprocal transaction and work.

---

## 3. Coprime-ring theorem

Let \(x_n\in\mathbb Z_L\) and \(y_n\in\mathbb Z_{L+1}\) be the two read
pointers. Advance both by one slot on every global tick:

\[
 x_{n+1}=x_n+1\pmod L,\qquad
 y_{n+1}=y_n+1\pmod{L+1}.                            \tag{5}
\]

### Theorem 1 — complete deterministic pair orbit

The joint orbit of equation (5) has period \(L(L+1)\) and visits every member
of

\[
 \mathbb Z_L\times\mathbb Z_{L+1}                   \tag{6}
\]

exactly once.

### Proof

Consecutive integers are coprime. By the Chinese remainder theorem, for every
pair \((x,y)\) there is exactly one residue
\(n\pmod{L(L+1)}\) satisfying \(n=x\pmod L\) and
\(n=y\pmod{L+1}\). Equation (5) therefore forms one transitive cycle over the
whole product. \(\square\)

In particular, every ordered pair of occupied bank addresses appears exactly
once, independently of the number and placement of blanks. The joint cycle
contains no stochastic update and its rule does not inspect phase, outcome, or
target frequency.

---

## 4. Physical compatibility readout

At a joint pointer state, emit a detector transaction at port \(o\) precisely
when both selected slots are nonblank and carry:

1. the same physical outcome-port label \(o\); and
2. the same quadrature-rail label \(R\) or \(I\).

The sign within a rail need not be inspected because opposite signs have
already left the active register. The number of compatible pointer states for
port \(o\) in one complete deterministic cycle is therefore

\[
 C_o=r_{o,R}^2+r_{o,I}^2
    =a_o^2+b_o^2
    =|Z_o|^2.                                        \tag{7}
\]

Conditioning the long-run event stream on a compatible detector transaction
gives

\[
 \boxed{
 f_o={C_o\over\sum_r C_r}
 ={ |Z_o|^2\over\sum_r|Z_r|^2}.}                    \tag{8}
\]

This is an exact deterministic pushforward for the declared finite record
model. Uniformity is not inserted as a probability table: equation (5) is one
transitive finite cycle, so its complete-period time average and its unique
invariant measure are uniform over the joint pointer states.

The outcome label in the compatibility test is not an abstract answer supplied
to the selector. It must be the record's physical route through the apparatus
to a localized port. Changing context changes those routes and therefore the
admissible record list, not the mixer law.

---

## 5. Local action embedding

The transport portion is compatible with the constraint family of the
unadopted
[`phase-complete bond-transaction action`](../../scopes_and_specs/SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md).
For one-hot address-head records \(A_{j,n}\) and \(B_{k,n}\), add only the
local shift constraints

\[
 \begin{aligned}
 \mathcal C^A_{j,n}&=A_{j,n+1}-A_{j-1,n}=0,\\
 \mathcal C^B_{k,n}&=B_{k,n+1}-B_{k-1,n}=0,
 \end{aligned}                                      \tag{9}
\]

with cyclic indices and their Lagrange multipliers in the same action. Every
record moves one memory edge per global tick. The ring geometry, rather than a
nonlocal counter, supplies the complete-pair schedule.

`[SELECTION CANDIDATE]` The two address heads may be realized by the existing
L/R substrate rails around one shared memory kernel, with one additional delay
cell on one closed loop. This identity is not proved. A production realization
must show finite Moore-local ownership, formation, dual-port readout, and
collision handling.

When equation (7) fires, the proposed detector transaction is the same neutral
manifestation event used by the common action,

\[
 (0,0;0;z)\longleftrightarrow(\epsilon,-\epsilon;z;0), \tag{10}
\]

not a separate stochastic click primitive. The action has not yet derived the
work transfer, refractory behavior, or competition that ensures one registered
macroscopic outcome per preparation.

---

## 6. Reversible cancellation requirement

Equation (3) must be produced physically. Deleting opposite records would
violate the framework's complete-history ontology. The minimum reversible
port has the form

\[
 (h_{+e},h_{-e};D=0)
 \longleftrightarrow
 (0,0;D=(h_{+e},h_{-e})),\qquad e\in\{1,i\}.          \tag{11}
\]

The records leave the active interference rails but survive in a dark
environment/memory channel \(D\), along with their energy and inverse. A fair
local circulation and repeated application of equation (11) leave exactly
\(|N_{o,0}-N_{o,2}|\) real records and
\(|N_{o,1}-N_{o,3}|\) imaginary records, independent of which opposite pairs
meet first.

The exact
[reversible cancellation/click successor](THEOREM_REVERSIBLE_C4_CANCELLATION_AND_CLICK_CIRCUIT_v1.md)
constructs equation (11) as an involutive payload-complete gate, verifies a
canonical finite cancellation queue, and adds a nondestructive click
comparator that retains the self-address terms. Its local router, finite memory
ownership, work, manifestation, and reset remain ungenerated by the common
action. Nothing is ontically erased merely because the active amplitude
cancels.

---

## 7. What this does and does not close

### Exact advance

For a prepared finite equal-weight C4 history multiset, the framework now has:

```text
opposite-phase residual counts
    -> two finite local cyclic memories
    -> target-blind one-hop deterministic evolution
    -> every ordered pair exactly once
    -> exact normalized |Z_o|^2 click frequencies.
```

This is stronger than defining a basin with the desired cardinality: an
explicit finite dynamics visits the basin states without reading their labels.

### Still open

It is not yet the general physical Born rule. The common action must still
derive:

1. the production and routing of complete-history records;
2. the action-generated local cancellation router and its dark-record capacity;
3. the shared record bank and two physical address heads without an external
   amplitude compiler;
4. detector work, manifestation, recovery, and single-outcome competition;
5. robustness when trials do not span a complete mixer period;
6. native formation of the finite Gaussian-integer block sequence used by the
   later controlled general-amplitude limit;
7. sequential measurement and composition laws; and
8. multipartite context dependence with operational no-signalling.

The contextual posture remains the one declared in
[`SPEC_FTD_FRAMEWORK_V2_CONTEXTUAL_ACTUALIZATION.md`](../../../01_reference/SPEC_FTD_FRAMEWORK_V2_CONTEXTUAL_ACTUALIZATION.md).
The mixer supplies a candidate FC-CA4 pushforward only after its physical
preconditions are generated by the ontology.

---

## 8. Falsifiers and next gate

Close or demote this route if:

- the two residual registers cannot be formed by finite local transactions;
- reversible cancellation requires unbounded memory within a finite trial;
- the detector must inspect \(Z_o\), \(|Z_o|^2\), or desired frequencies rather
  than physical record labels;
- incomplete-period or reset dynamics bias the registered outcomes;
- the single-outcome competition changes equation (8);
- context composition permits signalling; or
- the general-amplitude limit depends on tuned multiplicities.

The next locked desk target is a **work-complete actualization gate**: retain
the proved shift, cancellation, and click permutations, then derive the
detector reserve, neutral manifestation work, amplification, and reciprocal
reset from the same finite local action without changing equations (7)--(8).

The finite prepared-domain detector step is now completed by the
[physical Born actualization tape](THEOREM_C4_PHYSICAL_BORN_ACTUALIZATION_TAPE_v1.md).
Each joint pointer state controls one separately owned detector token; bright
pairs manifest that token at the physical outcome route, including
self-address pairs without copying the signal record. The finite tape counts
remain exactly equation (8), and a second closed orbit reverses the complete
tape. Native preparation, router/tape formation, exclusive single-trial
competition, and multipartite no-signalling remain open.

The later
[Gaussian-integer general-amplitude theorem](THEOREM_C4_GAUSSIAN_INTEGER_GENERAL_AMPLITUDE_PHYSICAL_LIMIT_v1.md)
closes the mathematical prepared-bank limit: for every finite normalized
complex response, finite C4 banks give physical event frequencies converging
with an explicit total-variation bound and finite resource price. The common
action still has not generated those banks, incomplete-window robustness, or
exclusive trial outcomes.

The later
[autonomous reversible renewal detector](THEOREM_C4_AUTONOMOUS_REVERSIBLE_BORN_RENEWAL_DETECTOR_v1.md)
uses this same target-blind orbit with one reusable balanced-ternary detector
and one reusable cotangent source packet. It produces one exclusive event at
each compatible pair and resets autonomously, eliminating the prepared
$L(L+1)$ detector/source tape. The shared bank and coprime rings themselves
remain prepared selection candidates, and externally heralded one-click
source trials remain open.
