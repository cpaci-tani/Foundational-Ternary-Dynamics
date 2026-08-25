# V3 contextual neutral-pointer Born renewal apparatus v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON A PREPARED V3 FIELD BANK AND SELECTED
APPARATUS CHART — TARGET-BLIND COMPLETE PAIR ENUMERATION]** +
**[THEOREM — EXISTING-CARRIER NEUTRAL POINTERS AND ONE REUSABLE TERNARY
DETECTOR]** +
**[THEOREM, CONDITIONAL — EXACT BORN EVENT FREQUENCIES AND FINITE-EVENT-WINDOW
BOUND]** +
**[OPEN — BANK/APPARATUS FORMATION, AMPLIFICATION, BACKREACTION, PHI, AND
MULTIPARTITE COMPOSITION]**  
**Scope:** one finite prepared 384-channel v3 field bank in a selected
Moore-local apparatus block  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Prepared-readout parent:**
[`THEOREM_V3_FIELD_BANK_GAUSSIAN_BORN_READOUT_v1.md`](THEOREM_V3_FIELD_BANK_GAUSSIAN_BORN_READOUT_v1.md)  
**Context boundary:**
[`THEOREM_V3_CUBIC_COVARIANCE_TRANSITIVE_BORN_COMPONENT_OBSTRUCTION_v1.md`](THEOREM_V3_CUBIC_COVARIANCE_TRANSITIVE_BORN_COMPONENT_OBSTRUCTION_v1.md)  
**Exact certificate:**
[`proof_v3_contextual_neutral_pointer_born_renewal_apparatus.py`](../../../../../scripts/proofs/proof_v3_contextual_neutral_pointer_born_renewal_apparatus.py)

---

## 1. Fixed carrier problem

The prepared-readout parent proves that opposite C4 phases cancel to a
Gaussian integer and that compatible residual pairs number `|Z_o|^2`. It did
not provide a physical finite pointer, renewal cycle, or exclusive detector
event in the ratified v3 carrier.

One v3 site contains exactly 384 field-channel addresses:

\[
 (t,n,h,k,\epsilon),                                  \tag{1}
\]

with twelve tangent/polarity outcome ports and eight normal/hand channels at
each of four C4 phases. A finite state-only lookup pairs the ordered phase-0
and phase-2 records and the ordered phase-1 and phase-3 records. All dark
records remain present; the unmatched records form the residual set `R`.

For every outcome `o`,

\[
 \#\{(a,b)\in R^2:\text{same outcome and rail}\}
 =|Z_o|^2.                                             \tag{2}
\]

The certificate exhausts all `9^4=6,561` locally possible one-port phase
count vectors. Equation (2) is read from actual occupied channel addresses,
not supplied as a desired weight.

---

## 2. Two neutral physical pointers

A selected oriented apparatus chart orders all 384 channels by their
intrinsic finite descriptor. One pointer has a 384-state cycle. The second
uses the same 384 addresses plus one blank delay state, giving a 385-state
cycle. Since

\[
 \gcd(384,385)=1,                                     \tag{3}
\]

their joint period is

\[
 T=384\cdot385=147{,}840,                              \tag{4}
\]

and visits every ordered address/address-or-delay pair exactly once.

Each channel address is represented by:

1. the existing opposite-polarity pair associated with its one-particle
   Hodge/C4 state, contributing exactly zero additive `E/B`; and
2. one existing A9 polarity token.

This gives 384 unique pointer configurations. The selected controller's blank
configuration supplies the single delay state. No integer counter, unbounded
identity, random draw, or new carrier alphabet is introduced.

The fixed ordering does not evade the full-cubic transitive-cycle
obstruction. It is explicitly contextual: transforming the oriented apparatus
chart transforms the address order. The certificate verifies all

\[
 384\cdot48=18{,}432                                  \tag{5}
\]

pointer/context rows. Thus the law is covariant with the apparatus; one fixed
cycle is not falsely required to commute with all frames simultaneously.

---

## 3. One reusable ternary detector

The primitive detector state is exactly the balanced ternary site value

\[
 q\in\{-1,0,+1\}
 =\{\text{recovery},\text{ready},\text{manifested}\}. \tag{6}
\]

At a dark address pair the two pointers advance immediately. At a compatible
residual pair the local rule is

```text
ready -> manifested -> recovery -> advance-ready.
```

Misprepared nonready states at a dark address form a quarantined two-cycle.
The complete pointer-times-trit state space is a finite permutation with an
explicit inverse. The certificate checks all 443,520 states in both
directions.

Bank, two pointer controllers, and ternary detector occupy four sites inside
one Moore cube. At each manifested state the outcome tangent and polarity
route one canonical eight-record Gauss-event seam. The event is exclusive:
one compatible ordered pair produces one manifested detector state before
renewal.

---

## 4. Exact frequencies and finite event windows

One complete operational orbit contains

\[
 T+2B,
 \qquad B=\sum_o|Z_o|^2,                              \tag{7}
\]

states and exactly `B` manifested events. Therefore

\[
 \boxed{
 f_o={|Z_o|^2\over\sum_r|Z_r|^2}.
 }                                                     \tag{8}
\]

This is deterministic time counting on one prepared component, not a
fundamental random choice.

Let `B` also denote the length of the cyclic manifested-event word. Any `N`
consecutive **heralded events**, from any entry phase, contain complete cycles
plus a remainder shorter than `B`. Their total-variation discrepancy obeys

\[
 \boxed{d_{\rm TV}< {B\over N}.}                      \tag{9}
\]

The certificate checks 10,443 finite-window rows in the registered
multi-outcome fixture. Equation (9) is an event-window bound; it does not yet
assert that a formed laboratory clock or amplifier supplies those heralds.

---

## 5. What is and is not closed

Closed conditional on the prepared bank and selected apparatus chart:

1. fixed target-blind enumeration of every ordered v3 channel pair;
2. explicit zero-`E/B` finite pointer encodings;
3. a reusable balanced-ternary detector rather than a prepared event tape;
4. one compatible pair to one exclusive manifested event;
5. exact prepared Born frequencies; and
6. the `B/N` manifested-event-window bound.

Still open after the finite source-history successor:

1. formation/protection of the A9 source, retained controller, chart, and
   reserve that supply the finite source-history bank branch;
2. formation and protection of the oriented apparatus chart;
3. an amplified persistent record and reciprocal detector backreaction;
4. overlapping source/apparatus traffic arbitration;
5. integration into homogeneous `Phi`, including a physical expiry/reset
   branch; and
6. multipartite composition with operational no-signalling.

The result closes the v3 **enumeration/renewal/exclusivity** subgate. It does
not yet close the general physical Born rule.

The later
[`finite source-history Phi-v13 theorem`](THEOREM_V3_FINITE_SOURCE_HISTORY_BORN_BANK_FORMATION_PHI_v13_CANDIDATE_v1.md)
supplies an exact eight-tick source-to-bank transducer with retained inverse,
resource conservation, and formed `|Z|^2` counts. The remaining formation debt
is now the source/controller/chart and this apparatus itself, together with
renewal, protection, amplification/backreaction, and traffic.

The
[`matter-anchored event-seam successor`](../common_action_mechanics_reciprocity/THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)
physically transfers each bright pointer token into one dressed charged
source, its exact stress, and a reversible neutral gravity-source pair while
retaining the prepared `|Z_o|^2` counts. This closes the isolated source-
composition seam, not native bank/apparatus formation, amplification, or
multipartite no-signalling.

The later
[`Phi-v14 protected-apparatus successor`](THEOREM_V3_REDUNDANT_POINTER_DETECTOR_PROTECTION_AND_A2_CLICK_MEMORY_PHI_v14_CANDIDATE_v1.md)
triples both pointer residues and the detector, corrects every one-copy valid-
symbol substitution before COMMIT, and writes manifested outcomes into twelve
fixed-occupancy A2 counters. It closes finite protection and persistent click
memory on the prepared apparatus, not its formation, reciprocal measurement
work, macroscopic amplification, traffic, or bipartite routing.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_contextual_neutral_pointer_born_renewal_apparatus.py
```

Expected result: `15/15` checks pass, including 18,432 context-covariance
rows, 6,561 bounded bank rows, 443,520 permutation rows, and 10,443 finite
event-window rows.
