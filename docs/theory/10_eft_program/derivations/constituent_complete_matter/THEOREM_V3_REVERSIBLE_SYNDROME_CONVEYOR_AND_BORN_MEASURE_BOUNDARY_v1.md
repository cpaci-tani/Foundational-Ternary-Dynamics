# V3 reversible syndrome conveyor and Born-measure boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT FINITE REVERSIBLE ONE-DEFECT LIFT]** +
**[THEOREM — ARBITRARY REGISTERED FINITE SURVIVAL HORIZON]** +
**[THEOREM — NO PERMANENT REPAIR ATTRACTOR FOR A FINITE BIJECTION]** +
**[THEOREM — FINITE-PERMUTATION INVARIANT-MEASURE SIMPLEX]** +
**[OPEN — PHYSICAL ACTION/WORK, FORMATION, ARBITRATION, AND NATIVE BORN
PREPARATION]**  
**Scope:** the complete 37,632-state registered one-coordinate shell around
the 24 charged circulation frames  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Decoder parent:**
[`THEOREM_V3_CHARGED_FRAME_UNIQUE_ONE_DEFECT_DECODER_v1.md`](THEOREM_V3_CHARGED_FRAME_UNIQUE_ONE_DEFECT_DECODER_v1.md)  
**Carrier successor:**
[`THEOREM_V3_NEUTRAL_SYNDROME_BUNDLE_CONVEYOR_v1.md`](THEOREM_V3_NEUTRAL_SYNDROME_BUNDLE_CONVEYOR_v1.md)  
**Atomic successor:**
[`THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md`](THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md)  
**Work-port successor:**
[`THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md`](THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md)  
**Born-symmetry successor:**
[`THEOREM_V3_CUBIC_COVARIANCE_TRANSITIVE_BORN_COMPONENT_OBSTRUCTION_v1.md`](../quantum_foundations/THEOREM_V3_CUBIC_COVARIANCE_TRANSITIVE_BORN_COMPONENT_OBSTRUCTION_v1.md)  
**Exact certificate:**
[`proof_v3_reversible_syndrome_conveyor.py`](../../../../../scripts/proofs/proof_v3_reversible_syndrome_conveyor.py)

---

## 1. Why bare projection is not the reversible branch

Let `C` be the 24 exact charged-frame presentations and let `D_x` be the
registered one-defect shell whose unique parent is `x in C`. The decoder
theorem gives

\[
 |D_x|=1{,}568,qquad
 \sum_{x\in C}|D_x|=37{,}632.                           \tag{1}
\]

The bare map `y -> x` is many-to-one. That is admissible only if the selected
microscopic law treats the lost coordinate as genuine expiry. If the repair
branch is instead required to be reversible, the defect identity must remain
in another owned degree of freedom.

Introduce a ready environment state `0` and syndrome labels

\[
 i\in\{1,\ldots,1568\}.                                \tag{2}
\]

The minimum one-step environment cardinality is therefore 1,569. Exact
capacity bounds are

\[
 2^{10}<1569\le2^{11},\qquad
 3^6<1569\le3^7,\qquad
 9^3<1569\le9^4.                                       \tag{3}

Thus the abstract syndrome needs at least eleven binary exclusions, seven
balanced trits, or four A9 registers. Sixteen existing neutral
opposite-polarity pair rails at fixed weight eight would provide

\[
 {16\choose8}=12{,}870                                  \tag{4}
\]

constant-token, zero-additive-field codewords. Equations (3)--(4) are capacity
statements only; they do not yet choose a Moore-local physical placement.

---

## 2. Reversible conveyor construction

Let `Phi` be the exact period-four charged-frame tick. Fix any finite survival
horizon `H>=1`. For every `y_i in D_x`, define one complete cycle

\[
\begin{aligned}
 (y_i,0)
 &\longmapsto (\Phi x,(i,1))\\
 &\longmapsto (\Phi^2x,(i,2))\\
 &\quad\cdots\\
 &\longmapsto (\Phi^Hx,(i,H))\\
 &\longmapsto (y_i,0).                                  \tag{5}
\end{aligned}

Undisturbed states obey

\[
 (x,0)\longmapsto(\Phi x,0).                            \tag{6}

Every state in equations (5)--(6) has exactly one predecessor and one
successor. The union is therefore a finite permutation with an explicit
inverse. Distinct syndrome labels prevent the repaired states from colliding,
and the exact frame plus known conveyor age identifies the unique parent.

The certificate exhausts `H=1,2,4,8`. Their operational state counts are

| `H` | total operational states | exact-object fraction on each repaired cycle |
|---:|---:|---:|
| 1 | 75,288 | `1/2` |
| 2 | 112,920 | `2/3` |
| 4 | 188,184 | `4/5` |
| 8 | 338,712 | `8/9` |

The construction is algebraic for arbitrary finite `H`: after one tick the
object is exact for `H` consecutive ticks, while the environment retains the
defect identity and age.

This closes the inverse/history requirement at abstract finite-state level.
The carrier successor realizes the 1,569 syndrome states as constant-weight
zero-`E/B` configurations of eighteen existing field bits and transports the
complete bundle causally with an exact inverse. The atomic successor couples
that bundle to every repair mutation with an exact finite inverse and a closed
generalized record/token count. The A2 successor realizes that count in two
existing payload-complete A9 slots and preserves one selected equal-occupancy
energy. Derivation of the metric/multiplier and bundle clock-debit work remain
open.

---

## 3. Finite reversible no-attractor theorem

Every orbit of a finite bijection is a finite cycle. Suppose a defective full
state became an exact object permanently after some finite time. Recurrence of
the full state would eventually return to the original defect, contradicting
permanent exactness. Therefore

> **A finite reversible law cannot turn a defective full state into a proper
> exact-object attractor.**

Equation (5) realizes the sharp alternative: any chosen finite survival bound
is possible by retaining sufficient environment history, but the closed
finite orbit eventually returns the defect.

There are three honest physical branches:

1. **finite reversible metastability:** retain the syndrome and register a
   survival horizon;
2. **unbounded-environment limit:** carry the syndrome outward without finite
   recurrence; or
3. **genuine noninjective expiry:** allow the microscopic law to destroy the
   distinction and price the associated source/work change.

The v3 constitution presently permits the third branch. The theorem does not
forbid it; it prevents that choice from being called reversible.

---

## 4. Born-measure consequence

Let a finite permutation decompose into `c` disjoint cycles. Every invariant
probability measure is an arbitrary convex mixture of the uniform measures on
those cycles. Consequently the invariant-measure simplex has dimension

\[
 c-1,                                                     \tag{7}

\]

and the invariant probability measure is unique exactly when `c=1`—one
transitive cycle.

The operational repair law already has

\[
 c=6+37{,}632=37{,}638,                                  \tag{8}

\]

where six are the undisturbed charged-frame cycles and every defect owns one
conveyor cycle. Its global invariant measure is therefore highly nonunique.

This is the general form of the existing Born boundary:

- a prepared coprime-ring detector can give exact `|Z_o|^2` frequencies
  because the registered operational component is one transitive cycle;
- reversible dynamics alone does not choose weights among distinct prepared
  components; and
- native Born preparation must either generate one physically admissible
  transitive component, specify a noninjective basin selector, or retain the
  preparation history in an environment.

Uniform time frequency on one declared cycle is a theorem. A probability
mixture over multiple cycles remains a preparation statement.

The Born-symmetry successor further proves that the complete faithful signed-
cubic bank cannot simply be made one globally transitive cycle while retaining
exact covariance: the centralizer of one cycle is cyclic, but the bank action
is faithful and nonabelian. A physical contextual frame/sector, environment,
expiry basin, or symmetry quotient is therefore part of the preparation debt.

---

## 5. Physical status

The result advances stable matter from a noninjective decoder sketch to an
explicit finite reversible survival construction, and it gives a general
reason why the Born preparation problem cannot be solved by saying only
“deterministic and reversible.” Remaining physical debts are:

1. common-action derivation of the selected occupancy energy, its multiplier,
   and clock-debit work;
2. full signed-cubic closure of the composed repair transaction;
3. formation of the ready repair environment, funded work port, and route;
4. arbitration of ordinary traffic and multiple bundles plus homogeneous Phi
   integration;
5. repeated-defect and collision survival; and
6. native formation of the Born transitive component.

No particle, physical lifetime, or general Born-rule claim is promoted.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_reversible_syndrome_conveyor.py
```

Expected result: `12/12` exact checks pass.
