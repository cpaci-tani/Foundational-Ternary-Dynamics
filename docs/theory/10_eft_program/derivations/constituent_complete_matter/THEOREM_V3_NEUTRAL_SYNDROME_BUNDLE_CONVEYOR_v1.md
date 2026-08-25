# V3 neutral syndrome-bundle conveyor v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXISTING-CARRIER CONSTANT-WEIGHT SYNDROME ALPHABET]** +
**[THEOREM — ZERO-$E/B$ STATE-ONLY BUNDLE]** +
**[THEOREM — ISOLATED RADIUS-ONE REVERSIBLE CONVEYOR]** +
**[SELECTION — EIGHTEEN-RECORD CLOCK DEBIT]** +
**[OPEN — PHYSICAL WORK/ACTION, FORMATION, COLLISIONS, AND PHI INTEGRATION]**  
**Carrier price:** eighteen occupied existing field slots at the bundle site;
no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Abstract parent:**
[`THEOREM_V3_REVERSIBLE_SYNDROME_CONVEYOR_AND_BORN_MEASURE_BOUNDARY_v1.md`](THEOREM_V3_REVERSIBLE_SYNDROME_CONVEYOR_AND_BORN_MEASURE_BOUNDARY_v1.md)  
**Atomic successor:**
[`THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md`](THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md)  
**Work-port successor:**
[`THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md`](THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md)  
**Exact certificate:**
[`proof_v3_neutral_syndrome_bundle_conveyor.py`](../../../../../scripts/proofs/proof_v3_neutral_syndrome_bundle_conveyor.py)

---

## 1. Required syndrome capacity

The charged-frame decoder has 1,568 one-defect states per exact parent. With
one ready state, a physical environment port needs

\[
 N_{\rm req}=1{,}569                                    \tag{1}

\]

distinguishable states. The abstract reversible theorem proved this capacity
requirement but did not place it in the selected finite carrier.

The 192 one-polarity field controllers decompose into sixteen disjoint native
period-twelve clock orbits. Choose one orbit for a header and the other
fifteen as syndrome rails. Occupy exactly seven of those fifteen rails. The
constant-weight alphabet has

\[
 \boxed{{15\choose7}=6{,}435>1{,}569.}                  \tag{2}

\]

The first 1,569 target-free lexicographic codewords supply the ready state and
all 1,568 defect syndromes. This is a finite injection, not an amplitude or
probability assignment.

---

## 2. Neutral eighteen-record bundle

For a controller `q`, define the opposite-polarity pair

\[
 U(q)=\{(q,+),(q,-)\}.                                  \tag{3}

\]

Let `R` be the native period-twelve internal tick. A syndrome bundle is

\[
 \mathcal S(q;A)
 =U(q)\cup U(R^4q)\cup\bigcup_{a\in A}U(a),             \tag{4}

\]

where `A` contains seven controllers from seven distinct nonheader clock
orbits.

The pair `q,R^4q` is the unique `R^4`-related controller pair in equation
(4). It therefore supplies a state-only header and direction. The remaining
seven singleton clock orbits are the syndrome payload. Equation (4) contains

\[
 2(2+7)=18                                               \tag{5}

\]

field records. Every controller occurs with both polarities, so on every C3
layer

\[
 \boxed{E_{\rm additive}=B_{\rm additive}=0.}           \tag{6}

\]

The bundle can coexist as a neutral environment record without changing the
object's additive electromagnetic source.

---

## 3. Local conveyor transaction

On a collision-free destination, select the whole-bundle transfer

\[
 (x,\mathcal S)\longmapsto(x+d(q),\mathcal S),           \tag{7}

\]

where `d(q)` is the signed SC direction read from the header. All eighteen
internal records are stalled for the transfer tick. This is the explicit
clock debit.

The transaction has the following exact properties:

1. every dependency and displacement is radius one;
2. eighteen records exist before and after;
3. the complete syndrome payload is unchanged;
4. additive `E/B` remains zero;
5. the output header gives the unique predecessor `x-d(q)`; and
6. every signed-cubic transform maps equation (7) to the corresponding
   transformed transaction.

The certificate checks all 6,435 codewords at three positions, giving 19,305
forward/inverse transactions. It also checks all 1,569 operational syndrome
states under all 48 signed-cubic maps, giving 75,312 covariance rows.

The clock stall is a selection already familiar from the charged plaquette
candidate. It has not yet been derived from a common work action.

---

## 4. Arbitrary finite conveyor horizon

Repeated application of equation (7) moves the syndrome one lattice site per
global tick. For any registered finite `H`, the packet position is an owned
age record inside the causal cone. Applying the explicit inverse `H` times
returns the complete bundle to its starting site and codeword.

The certificate verifies `H=1,2,4,8,16,32` for ready, first-syndrome, and
last-syndrome codewords. The construction is exact for arbitrary finite `H`
on a prepared collision-free ray.

This realizes two debts of the abstract conveyor theorem:

```text
abstract syndrome state -> existing field-bank codeword
abstract age transport  -> radius-one owned packet position
```

The atomic successor now attaches the departing bundle to exact correction of
every registered one-defect charged frame and retains the unique inverse. Its
work closure is followed by an A2 successor that realizes the `0/1/2` count in
two existing payload-complete A9 slots and conserves one selected equal-
occupancy energy. Derivation of that metric, its multiplier, and the bundle
clock-debit work remains open.

---

## 5. Stable-matter boundary

The carrier and atomic successor reduce the remaining task to a physical
common-action implementation rather than a memory-capacity question. It must:

1. derive or falsify the selected equal-occupancy energy from the common
   action, fix its multiplier, and price the bundle clock debit;
2. form the ready bundle, funded work port, and collision-free route without an external
   compiler;
3. arbitrate multiple bundles and ordinary field traffic;
4. close the composed transaction under the full signed-cubic group;
5. integrate the transaction into homogeneous Phi; and
6. prove repeated perturbation, scattering, and mass/dispersion survival.

Together the two theorems close existing-carrier **placement, capacity,
atomic syndrome emission, causal transport, inverse, and isolated-carrier
covariance**. They do not yet promote the charged frame to stable physical
matter.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_neutral_syndrome_bundle_conveyor.py
```

Expected result: `12/12` exact checks pass.
