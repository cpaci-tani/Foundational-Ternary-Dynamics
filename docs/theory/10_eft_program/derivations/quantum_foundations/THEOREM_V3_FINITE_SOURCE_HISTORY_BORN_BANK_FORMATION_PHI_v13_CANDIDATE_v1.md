# V3 finite source-history Born-bank formation Phi-v13 candidate v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — FINITE SOURCE-TO-BANK PHI-v13 BRANCH]** +
**[THEOREM, CONDITIONAL — EXACT EIGHT-TICK RESOURCE-CONSERVING BANK
FORMATION]** + **[THEOREM — EXACT RETAINED-HISTORY INVERSE]** +
**[THEOREM — FORMED COMPATIBLE-PAIR COUNTS AND TWO-HISTORY BORN
NORMALIZATION]** + **[BOUNDARY — SOURCE/CONTROLLER/CHART AND APPARATUS
FORMATION OPEN]**  
**Additional carrier price:** one existing A9 source clock, eight retained
fixed-occupancy A2 controller-bit owners, one fixed-occupancy A2 cursor, and
eight existing A2 reserve occupancies; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Prepared-count parent:**
[`THEOREM_V3_FIELD_BANK_GAUSSIAN_BORN_READOUT_v1.md`](THEOREM_V3_FIELD_BANK_GAUSSIAN_BORN_READOUT_v1.md)  
**Renewal/apparatus parent:**
[`THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md`](THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md)  
**Physical finite-counter parent:**
[`THEOREM_V3_ROTOR_GREEN_A2_PHYSICAL_MEMORY_AND_PHASE_PROTECTION_BOUNDARY_v1.md`](../gravity_cosmology/THEOREM_V3_ROTOR_GREEN_A2_PHYSICAL_MEMORY_AND_PHASE_PROTECTION_BOUNDARY_v1.md)  
**Certificate:**
[`proof_v3_finite_source_history_born_bank_formation_phi_v13_candidate.py`](../../../../../scripts/proofs/proof_v3_finite_source_history_born_bank_formation_phi_v13_candidate.py)

---

## 1. The missing map

The prepared field-bank theorem proves

\[
 Z=(N_0-N_2)+i(N_1-N_3),
 \qquad M=|Z|^2,                                      \tag{1}
\]

and the contextual apparatus deterministically enumerates every compatible
ordered pair exactly once. Those theorems did not explain how a physical
source writes the phase counts `N_k` into the bank.

Phi-v13 selects one finite source-history branch. It does not choose desired
counts and then populate them. It reads the current native C4 phase of an
actual A9 source at each tick and lets the source's retained finite controller
history determine whether that clock advances or stalls.

---

## 2. Physical finite input

Fix one native tangent/polarity outcome port in one retained oriented chart.
The v3 field bank has exactly eight intrinsic channels in each of its four C4
phase bins:

\[
 \#\mathcal B_k=8,qquad k\in\mathbb Z_4.              \tag{2}
\]

The source block contains:

1. one valid occupied A9 source clock;
2. eight retained controller bits `b_0,...,b_7` represented by existing
   fixed-occupancy A2 phase states;
3. one fixed-occupancy A2 cursor `c in {0,...,8}`; and
4. eight A2 reserve occupancies.

The controller word is a physical source/environment history record, not a
random draw and not a probability table. The proof exhausts all `2^8` words
and all sixteen initial A9 states.

---

## 3. One target-blind local write

At cursor value `c<8`, read the current source phase `k`. In the intrinsic
chart order, select the first clear channel of `B_k`. Then execute

```text
one A2 reserve occupancy -> one field-bank record at phase k
if b_c = 1: advance the source by one native A9 tick
if b_c = 0: retain the source phase
advance the fixed-occupancy cursor c -> c+1
```

The next address is determined by current bank occupancy; no desired final
count is read. If the cursor is complete, its designated reserve is already
spent, or the target bin is full, the branch fails closed.

Every tick has relative role delta

\[
 \Delta(N_F,N_{A2})=(+1,-1),                          \tag{3}
\]

while the source, eight controller owners, and cursor remain at fixed
occupancy. Over the full window,

\[
 (N_F,N_{A2}^{\rm reserve}):(0,8)\longmapsto(8,0).     \tag{4}
\]

Thus the already registered equal-occupancy ray is conserved tickwise and in
total.

---

## 4. The formed bank is the source history

Let `k_t` be the source phase read at tick `t`. After eight writes,

\[
 \boxed{N_k=\#\{t\in\{0,\ldots,7\}:k_t=k\}}.          \tag{5}
\]

The certificate verifies equation (5) on all

\[
 16\times2^8=4,096                                    \tag{6}
\]

source/controller histories. They produce 151 distinct phase-count vectors,
all satisfying `sum_k N_k=8` and `0<=N_k<=8`.

Because the branch always writes the first clear intrinsic address, the final
physical bank is exactly the canonical bank formed from those counts. The
construction commutes with all 48 signed-cubic chart transformations; the
certificate verifies 7,248 covariance rows.

---

## 5. Exact inverse and the role of memory

The controller word survives. To invert one tick, read `b_{c-1}`:

1. reverse the A9 tick if that bit was one;
2. recover the phase that was read before the forward update;
3. remove the last occupied intrinsic channel in that phase bin;
4. return it to the designated A2 reserve slot; and
5. decrement the cursor.

All 32,768 inverse tick rows close exactly. Detailed timing therefore need
not be reconstructed epistemically by an observer; it remains in a finite
physical memory kernel while the bank is being used.

Distinct controller histories can nevertheless produce the same surviving
count vector. All 151 attainable count classes have more than one timing
history in the exhaustive census. The downstream detector reads only the
formed bank, so these timing differences do not alter `Z` or `M`. If a later
expiry branch discards their controller distinction, equation (1) remains as
the physical consequence.

---

## 6. Formed Born counts, not assumed probabilities

For every formed bank, canonical opposite-phase cancellation gives equation
(1). Direct enumeration of residual compatible ordered pairs verifies

\[
 M=(N_0-N_2)^2+(N_1-N_3)^2=|Z|^2.                    \tag{7}
\]

For every nonzero pair of the 151 physically attainable count vectors, the
normalized event count is

\[
 {M_a\over M_a+M_b}
 ={ |Z_a|^2\over |Z_a|^2+|Z_b|^2}.                    \tag{8}
\]

The certificate checks all 22,792 nontrivial two-history rows exactly.
Equation (8) is a deterministic finite-cardinality pushforward. No stochastic
choice, amplitude primitive, or ensemble measure is inserted.

The result is conditional in the same sense as an ordinary measurement law:
different physical source histories form different banks. Phi-v13 supplies
the source-history-to-bank map; it does not assert that all sources must have
one universal phase count.

---

## 7. Exact boundary

Established:

1. eight native physical channels per port/phase;
2. existing fixed-occupancy representation of all controller bits and cursor
   states;
3. exact formation for all 4,096 finite source histories;
4. exact equality of bank counts and source phase visits;
5. exact eight-tick inverse on all 32,768 tick rows;
6. exact `M=|Z|^2` on every formed bank;
7. exact Born normalization on 22,792 two-history rows;
8. tickwise field/A2 relative-occupancy conservation;
9. 7,248 signed-cubic covariance rows; and
10. fail-closed spent/malformed reserve behavior.

Not established:

1. formation of the A9 source, controller word, oriented chart, and reserve;
2. autonomous renewal beyond the finite eight-tick window;
3. formation and environmental protection of the pair-enumeration apparatus;
4. detector amplification, reciprocal backreaction, and traffic;
5. multipartite source splitting and laboratory routing; or
6. promotion of Phi-v13 into canonical state-complete Phi.

Reproduce with:

```powershell
python scripts/proofs/proof_v3_finite_source_history_born_bank_formation_phi_v13_candidate.py
```

Expected result: `12/12` exact checks pass.

The honest conclusion is:

> **FTD now has an exact finite physical source-history-to-Born-bank
> transducer. It does not yet have a theorem that forms and protects the
> source/controller/apparatus complex in a laboratory environment.**

The later
[`Phi-v14 apparatus successor`](THEOREM_V3_REDUNDANT_POINTER_DETECTOR_PROTECTION_AND_A2_CLICK_MEMORY_PHI_v14_CANDIDATE_v1.md)
protects the prepared pointers/detector against every one-copy valid-symbol
substitution and stores each manifested outcome in surviving A2 click memory.
It closes apparatus protection after bank formation, while source/chart and
apparatus formation, reciprocal detector work, traffic, macro-amplification,
paired routing, and canonical Phi remain open.

The later
[`Phi-v15 transitive scheduler successor`](THEOREM_V3_TRANSITIVE_A2_SOURCE_HISTORY_ODOMETER_BORN_TIME_MEASURE_PHI_v15_CANDIDATE_v1.md)
uses the exact identity `8^4=16*2^8` to biject one existing A2 phase bank with
this theorem's complete 4,096-history input space. Two A2 owners then enumerate
all ordered history pairs in one reversible constant-deadline cycle. This
closes one selected physical reference time measure; it does not form the
owners, implement the scheduler under canonical Phi, route source blocks to
the apparatus, export long-run records, or select a laboratory state.
