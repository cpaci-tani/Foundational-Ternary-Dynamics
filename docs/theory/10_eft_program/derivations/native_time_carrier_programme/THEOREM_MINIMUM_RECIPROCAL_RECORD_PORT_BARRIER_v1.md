# Theorem — Minimum reciprocal record-port barrier (FTD-0856)

**Status:** `[THEOREM — ELIGIBILITY-CARDINALITY LOWER BOUND]` +
`[THEOREM — CAUSAL-ORIENTATION LOWER BOUND FOR FIRST-ORDER RAILS]` +
`[THEOREM — EXACT CONTROLLED RECIPROCAL SCATTERER]` +
`[SELECTION — IDEAL TWO-PORT BARRIER INTERFACE]` +
`[ENGINE FACT — LOCK/DUAL-TYPE FRAGMENTS]` +
`[CLOSED NEGATIVE — CURRENT PRODUCTION REALIZATION]` +
`[OPEN — PHYSICAL ELIGIBILITY, CHARACTERISTIC SEPARATION, FULL-STATE LIFT]`  
**Date:** 2026-08-10  
**Protocol:**
[`PREREG_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md)  
**Pre-run SHA256:**
`235CFE9E19A43217CBC1EB1CE74D190C6EBFC7E9005DDF15B86C9CE2B3BC74C6`  
**Certificate:** `scripts/proofs/proof_minimum_reciprocal_record_port_barrier.py`,
SHA256 `33C9A7C69D5417517BD91CCC15A6B63914A709C9E5056ACF15DCF526F5B62AA3`,
`32/32 PASS`

## 1. Why a protected event needs an eligibility coordinate

Let `M_s` be one complete local state containing a protected occupied record
`s in {-1,+1}` and no incident event pulse. Strict persistence requires a
deterministic update `F` to satisfy

\[
 F(M_s)=M_s.                                                   \tag{1}
\]

If the identical complete input is also supposed to unactualize on an event,
then the same function would have to satisfy

\[
 F(M_s)=P_s\ne M_s,                                           \tag{2}
\]

which contradicts (1). Consequently the complete input must distinguish at
least two eligibility states: hold and exchange. The distinction may be a
local activation coordinate, a phase-compliance flag, an environmental state,
or a richer dynamical variable. The theorem fixes its minimum cardinality in
this interface class; it does not derive its physical origin.

Production's Boolean `locked` coordinate demonstrates storage capacity for a
hold distinction. When true, it excludes a record from movement and
evaporation. But it is externally prepared or set by selected binding logic;
it is not a reciprocal event gate and it books no exchange work.

## 2. Why reciprocity needs causal orientation

FTD-0852's outward rail retains signed history, but a one-way first-order
shift is not by itself a forward-time absorption channel. The time reverse of
an emitted pulse is an incoming pulse. If the interface quotients both to one
unlabeled scalar amplitude, the orientation difference lies in the quotient
kernel:

\[
 (1,-1)\mapsto 0 \quad\text{under}\quad (i,o)\mapsto i+o.     \tag{3}
\]

A reciprocal forward-time realization must therefore retain the distinction
between incoming and outgoing characteristics, or an equivalent conjugate/
directional state. Two labeled ports attain this lower bound in the registered
first-order rail class. This is not a universal claim that every reversible
field needs two separate scalar arrays: a conjugate field pair or another
faithful directional representation can carry the same information.

## 3. The minimum controlled scatterer

For event energy `B>0`, set

\[
 A=\sqrt{2B}.                                                  \tag{4}
\]

Represent actual matter by `m=Ar` with `r in {-1,0,+1}`. Let `i` and `o` be
the incoming and outgoing signed characteristic amplitudes. For eligibility
`g in {0,1}`, define

\[
 \binom{m'}{o}=S_g\binom{m}{i},
 \qquad
 S_g=\begin{pmatrix}1-g&g\\g&1-g\end{pmatrix}.               \tag{5}
\]

The closed and open gates are

\[
 S_0=I,
 \qquad
 S_1=\begin{pmatrix}0&1\\1&0\end{pmatrix}.                  \tag{6}
\]

Both are symmetric orthogonal involutions. Therefore

\[
 S_g^T S_g=I,
 \qquad S_g^2=I,                                              \tag{7}
\]

and they conserve the boundary energy and signed content

\[
 H=\frac12(m^2+i^2)=\frac12((m')^2+o^2),
 \qquad m+i=m'+o.                                             \tag{8}
\]

They are equivariant under simultaneous sign reversal.

## 4. Hold, emission, and absorption

The closed gate gives strict record protection:

\[
 (sA,0)\xrightarrow{S_0}(sA,0).                              \tag{9}
\]

It also prevents an incident pulse from entering matter:

\[
 (0,sA)\xrightarrow{S_0}(0,sA).                              \tag{10}
\]

The open gate performs the reciprocal pair

\[
 (sA,0)\xrightarrow{S_1}(0,sA) \quad\text{(emission)},       \tag{11}
\]

\[
 (0,sA)\xrightarrow{S_1}(sA,0) \quad\text{(absorption)}.     \tag{12}
\]

Equation (11)'s outgoing amplitude is exactly
`s*sqrt(2B)`, the normalized cubic history-rail pulse of FTD-0855. Equation
(12) is its time-reversed actualization channel. Because `S_1` is symmetric
and self-inverse, the local boundary law is reciprocal without target fitting.

The ideal matrix switch does not establish that physical gate actuation is
free. A physical controller/activation coordinate and its work ledger remain
required if changing `g` costs energy or exports information.

## 5. Relation to global and local time

The clock cannot be the event cause by itself. A viable future eligibility law
has the form

\[
 g=g_{\rm compliance}\wedge g_{\rm activation},              \tag{13}
\]

where clock compliance is preregistered and context blind, while activation
comes from the local physical interaction. `G*` may constrain the compliant
phase cadence only if its clock programme passes; it does not select the sign,
energy, context, or outcome. Without `g_activation`, every occupied record
would emit at each open clock phase and strict stability would be lost.

## 6. Production boundary

Production contains two genuine fragments:

1. `Voxel::locked` is a two-valued hold coordinate; and
2. `(flux_L-flux_R, wave_vel_L-wave_vel_R)` supplies relative field plus a
   conjugate velocity type capable in principle of representing propagation
   orientation.

But no current production phase constructs protected incoming/outgoing
characteristics or applies equation (5). Evaporation clears the record without
depositing the outgoing amplitude. The same-sign movement branch flips only
the mover axes and resets its remainder, so FTD-0506 remains binding: it is not
a reciprocal collision or record barrier. No event phase derives `g` from a
local interaction, and the production ledger does not close the matter/dual
exchange.

Thus the theorem supplies the minimum selected interface and the exact algebra
that a production implementation must meet. It does not authorize silently
reinterpreting `locked` or the shared dual wave field as the completed
mechanism.

## 7. Remaining work

The next physical gates are:

1. derive a target-blind local activation coordinate and controller-work
   account;
2. construct or identify protected incoming/outgoing characteristics in the
   native dual field;
3. couple the scatterer to the FTD-0855 cubic history gearbox and production
   matter/dual energy ledger;
4. pass held-out emission, absorption, hold, multi-event, overlap, and finite-
   boundary tests; and
5. encode every erased label or explicitly price the residual environment.

No Born, Bell, `G*`, thermodynamic, biological, or completeness result follows.
No production code changed.

## 8. Certificate record

```text
FTD-0856 minimum reciprocal record-port barrier: 32/32 PASS
DETERMINISTIC_STRICT_HOLD_AND_EVENT_EXCHANGE_REQUIRE_DISTINCT_ELIGIBILITY
RECIPROCAL_FORWARD_TIME_RAIL_REQUIRES_RETAINED_CAUSAL_ORIENTATION
CONTROLLED_MATTER_INCOMING_OUTGOING_SWAP_IS_EXACT_RECIPROCAL_BARRIER
PRODUCTION_LOCK_AND_DUAL_TYPE_ARE_FRAGMENTS_NOT_THE_SCATTERER
VERDICT=OUTCOME_B_MINIMUM_REFERENCE_BARRIER_PRODUCTION_INCOMPLETE
```

## 9. Isolated reference implementation

The theorem was subsequently implemented without changing its scope in:

- [`reciprocal_record_port.h`](../../../../../engine/include/ftd/eft/reciprocal_record_port.h),
  SHA-256 `5973BF10BCE122304368E3BD191EA810D3DD6AB106B69B9D9022F662136D2B08`;
- [`reciprocal_record_port.cpp`](../../../../../engine/src/eft/reciprocal_record_port.cpp),
  SHA-256 `74DF9EF1943B98088B28D92197EA69F679A21BBB20A77E93E1212E3E65A4E338`;
- [`test_reciprocal_record_port.cpp`](../../../../../engine/tests/test_reciprocal_record_port.cpp),
  SHA-256 `7D552DA7B41126806AFA2E11FB364EA5DAC560940503B98B98318804127EB3FF`.

The API fails closed outside the declared ternary, positive-energy,
on-shell-input domain. It implements only the selected identity/swap contract,
reports energy and signed-content residuals, and contains no `Voxel`, physical
activation rule, production toggle, or tick-phase consumer. The focused Release
CTest passes `1/1` and direct execution reports:

```text
FTD-0856 reciprocal record-port EFT: PASS
scope=SELECTED_REFERENCE_PHYSICAL_ELIGIBILITY_OPEN
production_integration=NONE
```
