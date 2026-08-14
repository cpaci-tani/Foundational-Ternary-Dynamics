# Theorem — Minimum odd event receiver (FTD-0851)

**Status:** `[THEOREM — MINIMUM SIGN-COMPLETE EVENT RECEIVER]` +
`[THEOREM — POSITIVE-EXPORT ODD-AMPLITUDE COMPRESSION]` +
`[CONDITIONAL CONSTRUCTION — BALANCED BILATERAL PULSE]` +
`[CLOSED NEGATIVE — CURRENT MOVEMENT/JOURNAL AS PHYSICAL RECEIVER]` +
`[SELECTION/OPEN — RECEIVER ENERGY, PROPAGATION, AND NATIVE BARRIER]`  
**Date:** 2026-08-10  
**Protocol:**
[`PREREG_MINIMUM_ODD_EVENT_RECEIVER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_MINIMUM_ODD_EVENT_RECEIVER_v1.md)  
**Pre-run protocol SHA256:**
`374F571E155DEF0DE4A4CBF3A17C84E5D5EB60ED471308F3C02C5A1F8FBA8DDA`  
**Certificate:** `scripts/proofs/proof_minimum_odd_event_receiver.py`,
SHA256 `28030DDE523026CBF0587E82DDDE885C05D16D58D227D78E4923835A2662F805`,
`30/30 PASS`

## 1. Result

Consider a local actual-record erasure with two signed preimages,

\[
 e(+1)=e(-1)=0,
\]

and a declared nonnegative exported event energy `B`.

1. An energy-only receiver cannot retain the erased sign because `B` is even
   under `s -> -s`.
2. A receiver valid also at `B=0` therefore needs at least two distinguishable
   outputs. The minimum reference type is

   \[
     R_{\rm gen}(s,B)=(\chi,B)=(s,B),
     \qquad H_R(\chi,B)=B.                    \tag{1}
   \]

3. On the positive-export subdomain `B>0`, the two duties compress into one
   signed real coordinate,

   \[
     a=s\sqrt{2B},\qquad H_a=\frac{a^2}{2}=B,\qquad
     s=\operatorname{sign}(a).                \tag{2}
   \]

4. A selected bilateral representation is

   \[
     L=s\sqrt B,\qquad R=-s\sqrt B.           \tag{3}
   \]

   In common/relative coordinates,

   \[
     C=\frac{L+R}{\sqrt2}=0,
     \qquad D=\frac{L-R}{\sqrt2}=s\sqrt{2B},
     \qquad \frac{L^2+R^2}{2}=B.              \tag{4}
   \]

Thus a positive-energy event can export exactly one odd pulse: it adds no
common component, places the erased orientation in the relative component,
and carries the full declared event energy.

This is a minimum information-and-energy architecture, not evidence that the
current dual substrate implements it.

## 2. Proof of the minimum

Let `Y` be the receiver output set. Sign completeness requires

\[
 R(+1,B)\ne R(-1,B)
\]

at every registered `B`. Hence `|Y|>=2` on each fixed-energy fibre. An
energy-only receiver factors through `B` and has identical output on the two
preimages, so it fails.

Equation (1) attains the two-output lower bound on the sign coordinate and
closes energy by projection to `B`. For `B>0`, equation (2) is injective on
`{+1,-1} x {B}`; squaring returns `B` and the sign of `a` returns `s`. At
`B=0`, both amplitudes equal zero, proving why a separate odd label is needed
for a receiver whose domain includes zero export.

Equations (3)--(4) follow by substitution. They use one continuous relative
degree of freedom represented on two channels under the constraint `C=0`.
They must not be booked as two independent imported bits.

## 3. What movement contributes—and what it does not

The source-locked production movement path contains three relevant fragments.

### 3.1 Barrier fragment

Same-sign contact flips the mover's attempted axes and prevents site
co-occupation. But it resets the mover remainder, leaves the target and field
unchanged, and emits no event history. FTD-0506 measured that the rule fails
subcell specularity, pair momentum/current closure, and inverse recovery.
It is an infinite-wall-style heuristic, not a reciprocal stable-latch
transaction.

### 3.2 Exhaust fragment

Opposite-sign contact clears both states, velocities, remainders, identifiers,
and internal labels, then distributes the two pre-existing flux vectors over
their six-neighbour shells. The distribution contains no term odd in the
erased state labels. Holding the continuous pre-event fields fixed and
swapping `+/-` therefore gives the same output. The branch exports some field
content but not the distinguishing sign, and the aggregate energy audit has no
event receiver input.

### 3.3 Observer fragment

The optional history journal stores complete before/after voxel copies, so it
can distinguish the erased preimages diagnostically. Its own source contract
is explicit: it is an observer, disabled by default, consumes no randomness,
and writes no lattice, voxel, toggle, or integrator state. It is neither a
dynamical receiver nor an energy reservoir.

The three fragments do not compose into equation (1), (2), or (3). Current
movement plus journaling is therefore closed negative as the physical event
receiver.

## 4. Recursive stability condition

A receiver overwritten in place by the next event retains only the newest
sign. Four two-event histories collapse to two final outputs. Exact repeated
reception therefore requires one of three architectures:

1. the old odd pulse propagates causally into fresh environmental degrees of
   freedom before the local receiver is reused;
2. retained receiver state grows with event history; or
3. the model is declared open and exports the old state to an external
   environment.

The first is the smallest substrate-native candidate. It gives the desired
cycle:

\[
 \text{acquire}\to\text{hold}\to\text{erase}\to
 \text{odd energy pulse}\to\text{causal propagation}\to\text{ready}. \tag{5}
\]

Equation (5) is stable recursion only if the hold step has a genuine invariant
barrier and the pulse propagation is energy closed. Those dynamics are not
proved here.

## 5. Interpretation

The receiver formalizes “unactualization” without saying that information
vanishes from a closed universe. The actual ternary record may forget local
microdetail while an odd potential-field pulse carries the branch distinction
and energy outward. This matches the v2 actual/potential split: lossy actual
records can coexist with deterministic finer dynamics when the receiver is
included.

The bilateral form also explains the role of the two channels without a brain
identification. Their common coordinate can remain neutral while the relative
coordinate carries orientation. “Left/right brain” remains analogy only.

## 6. Scope and open physical gate

The following remain `[SELECTED/OPEN]`:

- the physical receiver energy and coupling;
- whether production `flux_L/flux_R` can carry the balanced odd pulse;
- a reciprocal strict barrier rather than the current remainder-reset wall;
- exact pulse propagation and local energy/current closure;
- the receiver's microscopic environmental and thermal interpretation;
- coupling to the FTD-0848 latch, selector/Born programme, and clock gate.

The next decisive test is not another probability curve. It is a source-locked
common/relative event-transaction discriminator: can a local signed removal
deposit equation (3) into a propagating relative field while preserving the
common channel, total energy, local current, and causal support? The test must
read only pre-event local state. No context, selected outcome target, Born
weight, `G*`, or cadence target may enter.

No production code changed. No Born, Bell, biological, thermodynamic,
Landauer, or finite-tick `G*` conclusion follows.

## 7. Certificate record

```text
FTD-0851 minimum odd event receiver: 30/30 PASS
TWO_RECEIVER_OUTPUTS_ARE_MINIMUM_FOR_SIGN_COMPLETE_ZERO_EXPORT
POSITIVE_EXPORT_COMPRESSES_SIGN_AND_ENERGY_TO_ONE_ODD_AMPLITUDE
BALANCED_BILATERAL_PULSE_IS_ONE_SELECTED_ZERO_COMMON_REALIZATION
PRODUCTION_BARRIER_EXHAUST_AND_JOURNAL_ARE_INCOMPLETE_FRAGMENTS
VERDICT=OUTCOME_B_MINIMUM_RECEIVER_DERIVED_PRODUCTION_INCOMPLETE
```
