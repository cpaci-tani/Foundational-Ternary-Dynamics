# Theorem — Cubic odd event deposit (FTD-0853)

**Status:** `[THEOREM — EXACT READY-PORT CUBIC DEPOSIT]` +
`[THEOREM — REDUCED INJECTIVITY AND ENERGY CLOSURE]` +
`[THEOREM — FIRST-SHELL FULL-CUBIC ORBIT MINIMUM]` +
`[SELECTION — DUAL ENERGY AND SIX-ARM TRANSACTION]` +
`[CLOSED NEGATIVE — CURRENT PRODUCTION IMPLEMENTATION]` +
`[OPEN — EVENT-ENERGY PROVENANCE, PORT FORMATION, BARRIER, FULL-STATE LIFT]`  
**Date:** 2026-08-10  
**Protocol:**
[`PREREG_CUBIC_ODD_EVENT_DEPOSIT_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_CUBIC_ODD_EVENT_DEPOSIT_v1.md)  
**Pre-run protocol SHA256:**
`F89BAB6F49566CC2EC38CCBA6F7EDFB5B0E8319A4ED3AEB89949D9F8B26B2AF3`  
**Certificate:** `scripts/proofs/proof_cubic_odd_event_deposit.py`, SHA256
`902815340FC6B830D41036337B18DE3D6556BBE98215E5F2859D8E21254BA5AD`,
`32/32 PASS`

## 1. Local deposit law

Let a local record event erase `s in {-1,+1}` and release `B>0`. Use the six
face directions

\[
 \mathcal F=\{\pm e_x,\pm e_y,\pm e_z\}.         \tag{1}
\]

At neighbour `x+nu`, define the pre-event dual wave velocities
`W_{L,nu},W_{R,nu}` and the radial relative occupation

\[
 Q_0=\sum_{\nu\in\mathcal F}
 \nu\cdot(W_{L,\nu}-W_{R,\nu}).                 \tag{2}
\]

The receiver port is compliant exactly when

\[
 Q_0=0.                                          \tag{3}
\]

Set `p=sqrt(B/6)` and apply

\[
 W'_{L,\nu}=W_{L,\nu}+sp\nu,
 \qquad
 W'_{R,\nu}=W_{R,\nu}-sp\nu.                   \tag{4}
\]

The actual record maps `s -> 0`; its event-energy account decreases by `B`.
Equation (4) changes no common wave velocity because its two increments cancel
arm by arm.

## 2. Exact energy transaction

Adopt the reference dual kinetic energy

\[
 K_{LR}=\frac12\sum_{\nu\in\mathcal F}
 (|W_{L,\nu}|^2+|W_{R,\nu}|^2).                 \tag{5}
\]

Expanding equation (4) gives

\[
 \Delta K_{LR}=spQ_0+6p^2.                     \tag{6}
\]

On the compliant surface (3),

\[
 \Delta K_{LR}=B,
 \qquad \Delta(H_{\rm record}+K_{LR})=0.       \tag{7}
\]

This is an exact event transaction, not an expected or fitted energy balance.
Off the ready surface, the same naive amplitude has the exact uncancelled
cross-energy defect `spQ_0`. The gate is therefore load-bearing; it cannot be
replaced by a tolerance after looking at outcomes.

## 3. Sign and energy recovery

The radial coordinate after the event is

\[
 Q_1=Q_0+12sp=s\sqrt{24B}.                      \tag{8}
\]

Hence

\[
 s=\operatorname{sign}(Q_1),
 \qquad B=Q_1^2/24.                              \tag{9}
\]

After extracting `(s,B)`, subtracting the known six impulses in (4) recovers
every arbitrary pre-event shell velocity. Therefore the map is injective on
the declared reduced domain `(s,B,{W_L,W_R})` subject to (3).

It is not injective on the complete production state. The pulse does not encode
particle ID, spin, color, remainder, or every other label erased by current
events. FTD-0395's full-state noninjectivity remains binding. A full natural
extension would need additional receiver coordinates or a separately declared
lossy environment.

## 4. Why six faces

The full cubic signed-permutation group has 48 elements. Acting on the 26
nonzero first-Moore-shell directions gives exactly three directed orbits:

| representative | orbit | size |
|---|---|---:|
| `(1,0,0)` | faces | 6 |
| `(1,1,0)` | edges | 12 |
| `(1,1,1)` | corners | 8 |

These disjoint orbits exhaust the shell. Six faces are therefore the minimum
nonzero directed orbit for a one-tick equal-orbit deposit with full signed-
permutation covariance. This is a scoped minimum: onsite scalar receivers,
unequal multi-orbit constructions, and multi-tick encodings are outside the
registered class.

The face vectors sum to zero. Consequently the L impulses and R impulses each
have zero net vector sum, while every relative radial projection has sign `s`.
The construction is also covariant under

\[
 (s,L,R)\mapsto(-s,R,L).                         \tag{10}
\]

It therefore retains orientation without selecting a spatial axis.

## 5. Relation to the causal history carrier

Equation (4) is the missing local write operation for FTD-0852's relative
history carrier. It gives the combined reference sequence

\[
 \text{record erasure}\to
 \text{six-face odd deposit}\to
 \text{relative propagation}\to
 \text{fresh compliant port}.                  \tag{11}
\]

At reference scope, (11) explains how an actual record can discard its sign
while the potential layer retains the signed event as a causal field pattern.
The common observable remains unchanged at the deposit tick.

## 6. Production boundary

Production has every storage coordinate needed by (4) and exposes the six face
neighbours. But it does not implement the transaction:

1. no current event writes the six opposite L/R wave-velocity impulses;
2. production has no derived event-energy `B` tied to the exact pre/post event;
3. `update_energy_ledger_cpu` squares only common L/R sums, so it assigns zero
   receiver energy to equation (4)'s pure-relative pulse;
4. no ready-port gate tests (2)--(3);
5. production propagation has not shown port clearing or reduced inverse
   recovery; and
6. the same-sign record barrier remains nonreciprocal and phase-erasing.

Thus the theorem supplies an exact selected reference transaction and a
concrete implementation interface, not a production discovery.

## 7. What this means—and does not mean

The actual/potential split is now constructive at the smallest local symmetric
event level: the actual ternary state can become zero while its erased
orientation and released energy appear as a cubically symmetric relative-field
shell. Some local detail is intentionally absent from the reduced record, but
the declared sign/energy account is not lost.

No probability enters. The ready-port gate reads only local pre-event field
state, and the pulse reads only `(s,B)`. It does not read a measurement context,
chosen outcome target, Born weight, `G*`, or clock cadence. Consequently this
mechanism neither derives nor assumes Born frequencies.

Event-energy provenance, port formation, production dual energy/current,
propagation compliance, the reciprocal protected-record barrier, full-state
natural extension, microscopic bath, thermodynamics, biology, and `G*` cadence
remain `[OPEN]`.

No production code changed.

## 8. Certificate record

```text
FTD-0853 cubic odd event deposit: 32/32 PASS
SIX_FACE_ODD_DEPOSIT_IS_P4_LOCAL_CUBICALLY_BALANCED_AND_ZERO_COMMON
READY_PORT_Q0_ZERO_GIVES_EXACT_EVENT_ENERGY_AND_REDUCED_INVERSE
SIX_FACES_ARE_THE_MINIMUM_FULL_CUBIC_ORBIT_ON_THE_FIRST_MOORE_SHELL
PRODUCTION_DEPOSIT_DUAL_LEDGER_BARRIER_AND_FULL_STATE_EXTENSION_REMAIN_OPEN
VERDICT=OUTCOME_B_EXACT_SELECTED_DEPOSIT_PRODUCTION_INCOMPLETE
```
