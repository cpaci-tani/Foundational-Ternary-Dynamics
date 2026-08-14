# Theorem — Phase-referenced action export rail (FTD-0862)

**Status:** `[THEOREM — PHASE-CALENDAR COHERENCE CONDITION]` +
`[THEOREM — EXACT PREPARED-BASELINE EVENT RECOVERY]` +
`[THEOREM — FINITE-RAIL EXCESS-ACTION LEDGER AND BOUND]` +
`[THEOREM — ENVIRONMENT-COMPLETED SYMPLECTIC SHIFT]` +
`[SELECTION — NONZERO BASELINE AND DIRECTED PROTECTED RAIL]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[CLOSED NEGATIVE — PRODUCTION C18 REALIZATION]` +
`[OPEN — PHASE SOURCE, CLOCK MAINTENANCE, CUBIC EMBEDDING, CONTROLLER, AND PRODUCTION LEDGER]`  
**Date:** 2026-08-11  
**Repaired protocol:**
[`PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_CERTIFICATE_REPAIR_v2.md)  
**Invalid parent:**
[`PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md)  
**Parent pre-run SHA256:**
`D5CEFB6550DD7EED1DE5C5001E970EFB1F2D6EE25F8F5C1644E0CA4A5532CE80`  
**Repair pre-run SHA256:**
`6DF12ECB3299614D568B8DA26B165209E1C9F2DF27EF8707AF3849D44AE49CE0`  
**Certificate:**
`scripts/proofs/proof_phase_referenced_action_export_rail_v2.py`, SHA256
`DC38CF600E1A2500DF53E7A9090C79239E04595065C94794B07A766486D3D4C6`,
`36/36 PASS`

## 1. Result

The sign loss found by FTD-0860 is conditional, not absolute. One unlabelled
canonical pair cannot recover a signed event on an **arbitrary** background,
but it can recover the event exactly when the input background belongs to a
prepared nonzero phase calendar and the loaded pair travels with that calendar
on a causal rail.

The minimum reference mechanism has four resources:

1. a nonzero baseline action `I_*>0`;
2. a coherent phase reference;
3. a selected outward one-cell carrier rail; and
4. an environment that supplies the incoming baseline and receives the
   complete outgoing tail pair.

These resources close a stable reusable reference port. They are not derived
production hardware.

## 2. Exact phase calendar

Let the baseline canonical pair at site `j` and global tick `n` be

\[
 \beta_j^n=\sqrt{2I_*}
 \begin{pmatrix}\cos\phi_j^n\\\sin\phi_j^n\end{pmatrix},
 \qquad
 \phi_j^n=\phi_0+\kappa j-\omega n.               \tag{1}
\]

For the outward shift

\[
 Z_{j+1}^{n+1}=Z_j^n,                              \tag{2}
\]

the reference mismatch is exactly

\[
 \phi_{j+1}^{n+1}-\phi_j^n=\kappa-\omega.         \tag{3}
\]

Therefore the phase calendar is coherent iff

\[
 \kappa-\omega\in2\pi\mathbb Z.                  \tag{4}
\]

On the principal branch `kappa=omega`, phase is constant along
`(j,n)->(j+r,n+r)`, and the upstream boundary value obeys

\[
 \beta_{-1}^{n}=\beta_0^{n+1}.                    \tag{5}
\]

Equation (4) is the exact gearbox condition between a temporal phase advance
and a spatial carrier twist. It is a kinematic compatibility theorem. It does
not select `omega`, construct the oscillator, pay its maintenance work, or
identify `omega` with a `G*` clock.

## 3. Exact event recovery

For `J(q,p)=(-p,q)`, a signed event `(s,B)` loads the next prepared input as

\[
 Y=\sqrt{\frac{I_*+B}{I_*}}\,sJ\beta,
 \qquad s\in\{-1,+1\},\quad B>0.                  \tag{6}
\]

It follows exactly that

\[
 I(Y)=I_*+B,
 \qquad \beta\cdot Y=0,
 \qquad \operatorname{sign}(\beta\wedge Y)=s.    \tag{7}
\]

Because (2) transports `Y` and its coherent reference on the same
characteristic, every downstream local readout recovers

\[
 B=I(Y)-I_*,
 \qquad s=\operatorname{sign}(\beta\wedge Y).     \tag{8}
\]

This is the simplest precise way a substrate can distinguish clockwise from
counterclockwise: not from action `I` alone, which is orientation blind, but
from the antisymmetric relational quantity `beta wedge Y`. The reference pair
is the missing hand of the clock.

FTD-0860's collision remains true outside this restricted domain:

\[
 F_{+,B}(Z)=F_{-,B}(-Z).                           \tag{9}
\]

The theorem therefore replaces “one arbitrary pair is faithful” with the
weaker and correct statement “one pair is faithful relative to a prepared
phase standard.”

## 4. Recursive port and finite energy account

On a length-`N` rail, load the new event at depth zero, shift every old pair
outward, and export the complete old tail:

\[
 Z_0^{n+1}=Y_n,
 \qquad Z_{j+1}^{n+1}=Z_j^n.                      \tag{10}
\]

The previous port value moves to depth one, so the boundary can accept a new
prepared input every tick. With excess action

\[
 E_j^n=I(Z_j^n)-I_*,
 \qquad H_{\rm ex}^n=\sum_{j=0}^{N-1}E_j^n,
\]

the exact open-system ledger is

\[
 H_{\rm ex}^{n+1}-H_{\rm ex}^{n}
 =B_n-E_{N-1}^n.                                  \tag{11}
\]

For a baseline initialization and bounded events `0<=B_n<=B_max`, at most the
last `N` event loads are retained, so

\[
 0\le H_{\rm ex}^n\le N B_{\max}.                 \tag{12}
\]

The port is therefore bounded because action is exported, not dissipated or
overwritten. A scalar tail-energy total would lose the event sign. The
complete tail pair plus its continued calendar reference retains both sign and
energy. It is the same energetic event carrier, not a second sign rail; no
double counting occurs.

## 5. Closed completion and arrow

For fixed event controls, FTD-0860's pump is symplectic on `I>0`. Add one
incoming environment pair and retain the outgoing tail pair. The total rail
step is then a permutation of canonical pairs composed with the pump, hence it
is symplectic and injective. The inverse requires the tail and the known event
control.

The selected finite subsystem is directed and open. Its apparent arrow comes
from declaring one environment boundary “input” and the other “output.” Time
reversal exchanges those roles and uses the inverse pump. No fundamental
thermodynamic arrow or irreversible deletion follows from the reference rail.

## 6. Production and clock boundary

This result closes a mathematical mechanism but not its native realization.

1. Production C18 has the exact dispersive trace obstruction proved by
   FTD-0858 and is not equation (2).
2. Production acceptance lives in the common-field quotient and does not
   determine the relative on-shell port.
3. No production field is reserved as a maintained nonzero baseline.
4. No controller supplies the phase calendar, accounts its work/dissipation,
   or exports a complete signed tail environment.
5. A cubically symmetric realization would need a preregistered six-face or
   equivalent embedding without multiplying the event energy.

`G*` could later set `omega` only after a separate clock-to-rail gearbox and
maintenance theorem. Nothing in equations (1)--(12) derives that
identification.

## 7. Isolated implementation

The reference API is
[`phase_referenced_action_rail.h`](../../../../../engine/include/ftd/eft/phase_referenced_action_rail.h),
SHA256
`19EA541D11547460CC3AA3D041E8854E5A0277B6FDF58097B087E6D2139DF5DB`.
Its implementation and focused test are:

- [`phase_referenced_action_rail.cpp`](../../../../../engine/src/eft/phase_referenced_action_rail.cpp),
  SHA256
  `4F436D2927E3FB05B54BAA198F33CA783FAFE43F78437A429A4EA4DFEA166FD0`;
- [`test_phase_referenced_action_rail.cpp`](../../../../../engine/tests/test_phase_referenced_action_rail.cpp),
  SHA256
  `9DCCCA4F5E5EE61AECB8942EE697A8F37F50A35B0470D3015E1BE3D49917ABF9`.

The API fails closed on an empty rail, invalid/incoherent calendar, malformed
event, nonfinite carrier, and phase-readout mismatch. The focused Release
CTest passes `1/1` and reports:

```text
FTD-0862 phase-referenced action rail EFT: PASS
scope=PREPARED_BASELINE_SELECTED_OUTWARD_RAIL
signed_event_readout=EXACT_ON_REGISTERED_SUBSPACE
production_c18_equivalence=REJECTED
baseline_clock_controller=OPEN
production_integration=NONE
```

## 8. Certificate record

The FTD-0861 parent returned `35/36` because C35 used one absent prose marker;
all source hashes and mathematics passed. It remains preserved invalid and
books no theorem. FTD-0862 applied the sole preregistered vocabulary repair in
memory and returned:

```text
FTD-0861 phase-referenced action export rail: 36/36 PASS
COHERENT_PHASE_CALENDAR_PRESERVES_SIGNED_QUARTER_TURN_ALONG_CAUSAL_RAIL
PREPARED_BASELINE_MAKES_EVENT_ENERGY_AND_ORIENTATION_EXACTLY_RECOVERABLE
FINITE_RAIL_HAS_EXACT_EXCESS_ACTION_LEDGER_AND_BOUNDED_RETAINED_LOAD
REVERSIBILITY_REQUIRES_INPUT_TAIL_AND_FIXED_EVENT_CONTROL_ENVIRONMENT
PRODUCTION_C18_PHASE_MAINTENANCE_AND_CONTROLLER_REMAIN_OPEN
VERDICT=OUTCOME_B_EXACT_SELECTED_REFERENCE_RAIL_PRODUCTION_UNREALIZED
```

No persistent vacuum, phase-source, Born, Bell, Hilbert-recovery, biological,
CM/substrate, `G*` cadence, thermodynamic, operational-Lorentz, production, or
completeness claim is made.
