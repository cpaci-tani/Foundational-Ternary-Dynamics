# V3 transitive A2 source-history odometer and Born time measure Phi-v15 candidate v1

**Date:** 2026-08-25  
**Status:** **[SELECTION — PHI-v15 TRANSITIVE SOURCE-HISTORY SCHEDULER]** +
**[THEOREM — EXACT A2/HISTORY BIJECTION AND REVERSIBLE TWO-A2 ODOMETER]** +
**[THEOREM, CONDITIONAL — ONE-CYCLE UNIFORM TRIAL-CLOCK MEASURE]** +
**[THEOREM — EXACT FORMED-BANK MOMENTS, TWO-PORT BORN NORMALIZATION, AND
CONSTANT-DEADLINE PADDING]** + **[OPEN — OWNER FORMATION, CANONICAL PHI,
ROUTING, BACKREACTION, LONG-RUN MEMORY, STATE PREPARATION, AMPLIFICATION,
AND LABORATORY BELL RECOVERY]**  
**Carrier price:** two existing A2 scheduler owners for the bipartite history
odometer; the selected source stage uses two occupied A9 sources, two A2
cursors, and sixteen existing A2 reserve owners; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Formation parent:**
[`THEOREM_V3_FINITE_SOURCE_HISTORY_BORN_BANK_FORMATION_PHI_v13_CANDIDATE_v1.md`](THEOREM_V3_FINITE_SOURCE_HISTORY_BORN_BANK_FORMATION_PHI_v13_CANDIDATE_v1.md)  
**Protected-apparatus parent:**
[`THEOREM_V3_REDUNDANT_POINTER_DETECTOR_PROTECTION_AND_A2_CLICK_MEMORY_PHI_v14_CANDIDATE_v1.md`](THEOREM_V3_REDUNDANT_POINTER_DETECTOR_PROTECTION_AND_A2_CLICK_MEMORY_PHI_v14_CANDIDATE_v1.md)  
**Exact certificate:**
[`proof_v3_transitive_a2_source_history_odometer_born_time_measure_phi_v15_candidate.py`](../../../../../scripts/proofs/proof_v3_transitive_a2_source_history_odometer_born_time_measure_phi_v15_candidate.py)

---

## 1. Exact cardinality match

The Phi-v13 source transducer admits

\[
 16\times2^8=4096                                      \tag{1}
\]

complete input histories: sixteen occupied logical A9 source states and eight
advance/stall controller bits.

One existing fixed-occupancy A2 owner consists of four occupied A9 phase/
polarity digits. It therefore has

\[
 |A_2|=8^4=4096                                       \tag{2}
\]

raw physical phase states. The signed-counter interpretation used elsewhere
reserves one of these states as overflow; Phi-v15 instead uses the complete raw
phase bank as a cyclic address. No carrier state is added.

For address $a\in\{0,\ldots,4095\}$, define

\[
 q(a)=\left\lfloor\frac{a}{256}\right\rfloor,
 \qquad
 b(a)=a\bmod256.                                      \tag{3}
\]

The high four bits select one of the sixteen source states, while the low
eight bits are the retained controller word. Equation (3) is a bijection. The
certificate checks all 4,096 physical A2 payloads in both directions.

An already occupied source owner may be loaded by the selected controlled
cyclic permutation

\[
 (a,q)\longmapsto(a,q+q(a)\bmod16).                   \tag{4}
\]

The scheduler address is retained, and subtraction is the exact inverse. The
certificate checks equation (4) on all 65,536 address/source presentations.
This is classical reversible control, not copying an unknown quantum state.
Its locality and promotion into canonical $\Phi$ remain open, hence the
**[SELECTION]** tag.

---

## 2. One transitive ordered-pair scheduler

Let $(a_L,a_R)\in A_2\times A_2$. Define the base-4,096 odometer

\[
 a_L'=(a_L+1)\bmod4096,                                \tag{5}
\]

\[
 a_R'=(a_R+\mathbf1[a_L'=0])\bmod4096.                \tag{6}
\]

For the flattened address

\[
 n=a_L+4096a_R,                                        \tag{7}
\]

equations (5)--(6) give

\[
 \boxed{n'=(n+1)\bmod4096^2}.                         \tag{8}
\]

The predecessor is explicit. Consequently the two existing A2 owners form
one transitive deterministic cycle of exact period

\[
 \boxed{4096^2=16{,}777{,}216}.                       \tag{9}
\]

Every ordered pair of physical source histories occurs exactly once per
cycle. Unlike an arbitrary convex mixture of disjoint source sectors, this
particular uniform measure is the time count of one declared physical orbit.

---

## 3. Formation and renewal at every address

At each scheduler address, Phi-v15 invokes the existing Phi-v13 transducer on
two distinct tangent/polarity outcome ports. Each wing:

1. loads the source state from the high address digit;
2. reads the eight retained controller bits from the low digit;
3. converts eight A2 reserve occupancies into eight field records over eight
   ticks;
4. obtains physical phase counts $N_0,\ldots,N_3$;
5. retains the scheduler address throughout the apparatus trial; and
6. runs the exact Phi-v13 inverse to restore source, bank, cursor, and reserve.

The certificate forms 8,192 banks and checks 65,536 inverse formation ticks.
Both ports produce the same count function of their address, while their
field records and detector outcomes remain disjoint.

Thus the scheduler supplies renewal of the *prepared values* without erasing
the controller history. Genesis formation and protection of the occupied
owners, routing between source and apparatus blocks, and traffic arbitration
remain open.

---

## 4. The induced physical source-history measure

The one-cycle time mean of the four phase counts is

\[
 \boxed{\mathbb E_{\rm cycle}N=(2,2,2,2).}             \tag{10}
\]

The exact covariance is

\[
 \boxed{
 \operatorname{Cov}_{\rm cycle}(N)
 ={1\over64}
 \begin{pmatrix}
 111&-32&-47&-32\\
 -32&111&-32&-47\\
 -47&-32&111&-32\\
 -32&-47&-32&111
 \end{pmatrix}.}                                      \tag{11}
\]

For the physical Gaussian-integer readout

\[
 Z=(N_0-N_2)+i(N_1-N_3),                              \tag{12}
\]

equation (11) gives

\[
 \boxed{
 \mathbb E_{\rm cycle}Z=0,qquad
 \operatorname{Cov}_{\rm cycle}(\Re Z,\Im Z)
 ={79\over16}I_2,}                                    \tag{13}
\]

and hence

\[
 \boxed{\mathbb E_{\rm cycle}|Z|^2={79\over8}.}       \tag{14}
\]

These are exact rational time averages of the transitive finite scheduler,
not assumed random-source moments. They are not a fine-structure
normalization and are not compared with any external value.

The 4,096 addresses form 151 count vectors and the following exact manifested
count spectrum:

| $M=|Z|^2$ | Address multiplicity |
|---:|---:|
| 0 | 288 |
| 2 | 896 |
| 4 | 736 |
| 8 | 448 |
| 10 | 832 |
| 16 | 288 |
| 18 | 64 |
| 20 | 128 |
| 26 | 128 |
| 32 | 32 |
| 34 | 64 |
| 36 | 32 |
| 40 | 64 |
| 50 | 64 |
| 64 | 32 |

This spectrum is an output of the source recurrence and scheduler, not a
table supplied to the detector.

---

## 5. Exact two-port Born normalization

For scheduler address pair $(a_L,a_R)$, the two formed banks have

\[
 M_L=|Z_L|^2,qquad M_R=|Z_R|^2.                       \tag{15}
\]

The contextual pointer apparatus enumerates each compatible ordered pair
once, so for every non-dark trial

\[
 \boxed{
 f_L={M_L\over M_L+M_R}
 ={ |Z_L|^2\over |Z_L|^2+|Z_R|^2},
 \qquad f_R=1-f_L.}                                   \tag{16}
\]

The certificate checks equation (16) over all 151-by-151 attained count
classes with their scheduler multiplicities, covering exactly

\[
 \boxed{16{,}694{,}272}                               \tag{17}
\]

non-dark ordered physical history pairs.

The protected apparatus base orbit has 147,840 pointer addresses and two
additional detector macros per manifestation. Since each port has at most 64
events, a two-port trial has at most 128. Append

\[
 2(128-M_L-M_R)                                       \tag{18}
\]

reversible dark padding macros after the live apparatus returns. Then every
trial uses

\[
 \boxed{
 147{,}840+2(M_L+M_R)+2(128-M_L-M_R)
 =148{,}096}                                          \tag{19}
\]

apparatus macros. Fixed loading, formation, inverse formation, unloading, and
odometer advance add the same number of sublayers to every address. The
scheduler's one-visit-per-pair count is therefore also uniform in the declared
trial clock; variable detector dwell does not reweight the ensemble.

Over the complete exchange-symmetric odometer orbit, each port emits exactly
165,675,008 manifestations, so the aggregate reference frequency is $1/2$.
That is a property of this exhaustive symmetric source scheduler. It is not a
claim that every physically prepared laboratory state has equal outcomes.

---

## 6. Physical price and exact boundary

The finite source-side block requires 22 existing owners:

```text
two A2 scheduler addresses
two occupied A9 source owners
two A2 cursors
sixteen A2 reserve owners
```

The protected apparatus parent uses 23 Moore-block sites. One further A2
padding owner makes 24. The combined 46-owner construction therefore cannot
be hidden inside one 27-site Moore neighborhood. Causal multi-block routing is
a real remaining gate.

The twelve A2 click memories each retain at most 2,047 signed counts in the
current representation, whereas one complete scheduler orbit contains
165,675,008 manifestations per selected port. The event stream is perfectly
finite and periodic, but one apparatus block cannot retain its full-cycle
statistics. A causal record-export or memory-cascade construction is required
for operational long-run readout. No irreversible erasure is inferred from
that finite-capacity boundary.

The exact status is therefore:

```text
A2/source-history cardinality match:       exact
one transitive ordered-pair scheduler:      exact, selected
Phi-v13 formation and inverse renewal:      exact per address
uniform constant-deadline trial measure:    exact, selected
formed source-history moments:              exact
two-port |Z|^2 event normalization:         exact
owner formation/protection:                 open
canonical-Phi provenance:                   open
cross-block causal routing and traffic:     open
detector work/material backreaction:        open
full-cycle persistent memory/export:        open
state-specific lab preparation/amplification: open
Bell-correlation recovery:                  open
```

This closes the former objection that the 4,096 finite histories *must* be
mixed only in an analyst's notebook: one explicit finite physical clock can
enumerate them. It does not prove that nature selects this exhaustive
scheduler, and it does not turn a prepared absolute-square count into the
general quantum Born rule.

---

## 7. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_transitive_a2_source_history_odometer_born_time_measure_phi_v15_candidate.py
```

Expected result: `16/16` exact checks pass, with 4,096 scheduler histories, a
16,777,216-pair transitive period, 8,192 formed banks, 65,536 inverse formation
ticks, 151 count vectors, 15 click-weight classes, 16,694,272 non-dark ordered
pairs, and an exact 148,096-macro two-port apparatus deadline.

The later
[`Phi-v16 opposite-bank convoy successor`](THEOREM_V3_OPPOSITE_BANK_CONVOY_CAUSAL_BIPARTITE_ROUTING_PHI_v16_CANDIDATE_v1.md)
retains the two-A2 scheduler address while moving its two complete formed banks
along opposite one-hop-per-tick routes to separate prepared endpoints. It
closes common origination and causal bank delivery at selected corridor scope,
not measurement settings, backreaction, amplification, or laboratory Bell
recovery.
