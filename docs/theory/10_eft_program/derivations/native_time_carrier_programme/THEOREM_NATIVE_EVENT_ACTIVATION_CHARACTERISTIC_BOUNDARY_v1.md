# Theorem — Native event activation and characteristic boundary (FTD-0858)

**Status:** `[ENGINE FACT — DETERMINISTIC MOORE-LOCAL TARGET-BLIND ACCEPTANCE GIVEN FIXED INPUTS]` +
`[THEOREM — COMMON/RELATIVE TRIGGER-KERNEL OBSTRUCTION]` +
`[THEOREM — EXACT INCOMING/OUTGOING ENERGY-CURRENT CHART]` +
`[THEOREM — AXIAL C18 ONE-CELL-SHIFT OBSTRUCTION]` +
`[THEOREM — ZERO SIGNAL-WORK ACCOUNT]` +
`[SELECTION — PRODUCTION HAZARDS AND CHARACTERISTIC INTERPRETATION]` +
`[CLOSED NEGATIVE — CURRENT PRODUCTION ON-SHELL RECIPROCAL PORT]` +
`[OPEN — RELATIVE ACTIVATION, RESERVED MODE, CONTROLLER COST, FULL-STATE LIFT]`  
**Date:** 2026-08-10  
**Repaired protocol:**
[`PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_CERTIFICATE_REPAIR_v2.md)  
**Invalid parent protocol:**
[`PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md)  
**Repair pre-run SHA-256:**
`A43367B3BF46918ED5DBDFDF988E53DE75274170E314B61328AB40EB9BFBE1F3`  
**Certificate:**
`scripts/proofs/proof_native_event_activation_characteristic_boundary_v2.py`,
SHA-256 `E2A6D22946E0E3BD9A5CE208EB7C440567AA72B97C28F507C099F06E93740204`,
`40/40 PASS`

## 1. Production has pre-event acceptance coordinates

Fix the engine seed, site `x`, primitive tick `n`, and the complete local data
read by `phase_write`. The keyed genesis and evaporation draws are then fixed.
In the dual path define the common field and momentum

\[
 C=J_L+J_R,\qquad V=W_L+W_R.                         \tag{1}
\]

Production accepts dual genesis according to

\[
 a_g=
 \mathbf 1_{s=0}
 \mathbf 1_{|C|>K_G}
 \mathbf 1_{u_g<1-\exp[-(|C|-K_G)/K_M]}.             \tag{2}
\]

For the site and its six face neighbours, let

\[
 E_7=|C_x|^2+|V_x|^2+
 \sum_{y\sim_6x}(|C_y|^2+|V_y|^2).                  \tag{3}
\]

Production accepts evaporation according to

\[
 a_e=
 \mathbf 1_{s\ne0}
 \mathbf 1_{\neg\mathrm{locked}}
 \mathbf 1_{u_e<e^{-E_7/K_M^2}K_E\,d\tau}.          \tag{4}
\]

Equations (2)--(4) are deterministic once their complete inputs are fixed and
are Moore-local: genesis reads one site plus its keyed draw, while evaporation
reads one site, its six face neighbours, local proper-time data, and its keyed
draw. The frozen event slice reads no measurement context, selected outcome
target, Born weight, `G*`, or reciprocal-port target.

This is an `[ENGINE FACT]`, not a derivation of the hazard law. `K_G`, `K_M`,
`K_E`, the exponential forms, and the keyed pseudo-random streams are selected
production machinery. “Target blind” means only that the source predicate does
not read the forbidden targets.

Genesis precedes evaporation in the same sequential site loop. A site can
therefore experience the ordered decision pair `(a_g,a_e)=(1,1)` after genesis
makes the record occupied. Four possible ordered pairs cannot be faithfully
compressed to one unlabeled bit. The event kind and order remain part of the
controller input.

## 2. The trigger lives in the common-field quotient

Define the relative field and momentum

\[
 D=J_L-J_R,\qquad P=W_L-W_R.                         \tag{5}
\]

The change of coordinates is invertible:

\[
 J_L=\frac{C+D}{2},\quad J_R=\frac{C-D}{2},\qquad
 W_L=\frac{V+P}{2},\quad W_R=\frac{V-P}{2}.          \tag{6}
\]

For arbitrary `delta_J,delta_W`, the antisymmetric perturbation

\[
 (J_L,J_R)\mapsto
 (J_L+\tfrac12\delta_J,J_R-\tfrac12\delta_J),
\quad
 (W_L,W_R)\mapsto
 (W_L+\tfrac12\delta_W,W_R-\tfrac12\delta_W)        \tag{7}
\]

fixes `(C,V)` and shifts `(D,P)` by `(delta_J,delta_W)`. Consequently both
acceptance predicates are constant on an arbitrarily large relative-field
fibre. For example,

\[
 (W_L,W_R)=(0,0)\quad\text{and}\quad(1,-1)          \tag{8}
\]

have the same common momentum `V=0` but relative momenta `P=0` and `P=2`.
Their common-energy evaporation data agree while any relative incoming
amplitude built from `P` may differ.

Therefore production acceptance does not determine either reciprocal-port
condition

\[
 i=s\sqrt{2B}\quad\text{for absorption},
 \qquad i=0\quad\text{for ready emission}.           \tag{9}
\]

Dual genesis computes chirality for the record sign after acceptance. That may
distinguish some relative configurations, but it does not determine the full
relative amplitude, receiver energy, or readiness. A sign functional cannot
invert the kernel in (7).

This is the central obstruction: multiplying the production acceptance bit by
a compliant clock bit changes **when** an accepted event is eligible, but it
does not manufacture the missing on-shell relative port.

## 3. The relative pair admits an exact characteristic chart

On an outward-oriented face bond, let `p` be normalized relative momentum and
`g` the outward-oriented relative strain. Define

\[
 i=\frac{p+g}{\sqrt2},\qquad
 o=\frac{p-g}{\sqrt2}.                               \tag{10}
\]

Then

\[
 p=\frac{i+o}{\sqrt2},\qquad
 g=\frac{i-o}{\sqrt2},                               \tag{11}
\]

and exactly

\[
 \frac{p^2+g^2}{2}=\frac{i^2+o^2}{2},
 \qquad
 pg=\frac{i^2-o^2}{2}.                               \tag{12}
\]

Thus `(i,o)` is an invertible two-port energy/current chart. Reversing the
spatial orientation sends `g -> -g` and swaps `i` with `o`. Physical time
reversal sends `p -> -p` and

\[
 (i,o)\mapsto(-o,-i).                                \tag{13}
\]

This supplies precisely the directional information that FTD-0856 proved a
reciprocal forward-time interface must retain. It is a local coordinate theorem,
not yet protected characteristic dynamics.

## 4. The frozen C18 tick is not the exact history shift

Subtracting the matched L/R production equations cancels their equal matter
source and leaves a homogeneous relative wave pair. On fields constant over
planes transverse to one face axis, the frozen 18-point stencil reduces
exactly to

\[
 (\Delta_{1D}D)_j=D_{j+1}-2D_j+D_{j-1}.              \tag{14}
\]

For the default primitive kick--drift and a Fourier mode `e^{ikj}`, let

\[
 a(k)=4c^2\sin^2(k/2),\qquad c^2=C_{\rm WAVE}^2=\frac13.
                                                               \tag{15}
\]

The one-tick matrix on `(D,P)` is

\[
 U(k)=
 \begin{pmatrix}1-a(k)&1\\-a(k)&1\end{pmatrix},
 \qquad \det U=1,
 \qquad \operatorname{tr}U=2-a(k).                 \tag{16}
\]

Its elliptic phase satisfies

\[
 \sin^2\!\frac{\theta(k)}2
 =c^2\sin^2\!\frac{k}2.                            \tag{17}
\]

Two exact one-cell rails have eigenvalues `e^{+ik},e^{-ik}` and trace

\[
 2\cos k=2-4\sin^2(k/2).                             \tag{18}
\]

Similarity preserves trace and eigenvalues. Equations (16) and (18) agree for
all `k` only if `c^2=1`, whereas production selects `c^2=1/3`. Their exact trace
defect is

\[
 \operatorname{tr}U-2\cos k
 =4(1-c^2)\sin^2(k/2)
 =\frac83\sin^2(k/2).                               \tag{19}
\]

The frozen C18 wave therefore cannot be renamed as the exact one-cell FTD-0855
history shift. This does not forbid dispersive causal packets, nonlocal modal
projectors, a wider port-clearing compliance window, or new protected
directional storage. It closes only the literal primitive-tick identification.

## 5. Signal work closes; controller work does not

FTD-0856's controlled barrier is identity for `g=0` and swap for `g=1`. Both
matrices are orthogonal, hence on the declared signal account

\[
 H(m,i)=\frac{m^2+i^2}{2},\qquad
 W_{\rm signal}=H(S_g(m,i))-H(m,i)=0.                \tag{20}
\]

This is an exact zero **signal-work residual**, not a claim of cost-free
control. Production contains no state that records the gate actuation, no
switching-work/dissipation term, and no ordered controller ledger for
genesis-then-evaporation. The physical controller cost remains `[OPEN]`.

## 6. Verdict and next type debt

The result separates four questions that had been conflated:

| question | verdict |
|---|---|
| Does production have a local pre-event acceptance coordinate? | yes, `[ENGINE FACT]`, conditional on selected hazards and fixed inputs |
| Does that coordinate determine an on-shell relative port? | no, exact common/relative kernel obstruction |
| Can the relative canonical pair be charted as incoming/outgoing energy and current? | yes, exact local chart |
| Does the frozen primitive C18 tick protect those coordinates as one-cell rails? | no, exact dispersion obstruction |

Accordingly the next honest type/dynamics must be one of:

1. a target-blind **relative-channel activation/transducer** that reads and
   closes the same relative energy/readiness it actuates;
2. a declared reserved directional rail with its own production ledger; or
3. a preregistered dispersive port-compliance law showing that a C18 packet
   clears the event port within a finite window while preserving sign, energy,
   and operational causality.

Any option must retain ordered event kind, clock compliance as a separate
factor, controller work/dissipation, and the full-state/environment account.
Clock cadence cannot repair the common/relative kernel.

No Born, Bell, `G*`, thermodynamic, biological, or completeness result follows.
No production code changed.

## 7. Certificate record

The FTD-0857 parent execution passed all seven source hashes and then aborted on
a nonexistent source-slice marker. The FTD-0858 verifier-only repair changed
four registered comparison/slice defects in memory and returned:

```text
FTD-0857 native event activation and characteristic boundary: 40/40 PASS
PRODUCTION_EVENT_ACCEPTANCE_IS_DETERMINISTIC_LOCAL_AND_TARGET_BLIND_GIVEN_FIXED_INPUTS
COMMON_FIELD_TRIGGERS_DO_NOT_DETERMINE_THE_RELATIVE_ON_SHELL_RECORD_PORT
RELATIVE_EDGE_PAIR_HAS_AN_EXACT_INCOMING_OUTGOING_ENERGY_CURRENT_CHART
FROZEN_C18_DISPERSION_IS_NOT_THE_EXACT_ONE_CELL_HISTORY_RAIL
SIGNAL_WORK_CLOSES_ZERO_WHILE_PHYSICAL_CONTROLLER_COST_REMAINS_OPEN
VERDICT=OUTCOME_B_NATIVE_TRIGGER_AND_CHART_PRODUCTION_PORT_INCOMPLETE
```

## 8. Isolated reference implementation

The separated contracts were subsequently implemented without changing their
scope in:

- [`native_event_characteristics.h`](../../../../../engine/include/ftd/eft/native_event_characteristics.h),
  SHA-256 `F4A49A1DBF693CF468BC7942264C69B7B25ED9DC41E61059F6F251B696679393`;
- [`native_event_characteristics.cpp`](../../../../../engine/src/eft/native_event_characteristics.cpp),
  SHA-256 `DD23D2CB0BE3AC65462079779BCA9C8FDFB30B53AA56A9AACE710D83395EFCB8`;
- [`test_native_event_characteristics.cpp`](../../../../../engine/tests/test_native_event_characteristics.cpp),
  SHA-256 `1B1FFD910A68653C1B9D95B77577F2B820A803E3105A04A90A982CD9A1FC1AE5`.

The API deliberately exposes event acceptance and relative characteristic
coordinates as separate types. It reproduces the selected source predicates,
retains all four ordered event cases, fails closed on invalid domains, verifies
the common/relative kernel, and reports the C18 one-cell trace defect. It does
not construct a common-to-relative transducer, protected rail, clock gate,
controller ledger, `Voxel` consumer, or production phase.

The focused Release CTest passes `1/1`; direct execution reports:

```text
FTD-0858 native event characteristics EFT: PASS
scope=SOURCE_ACCEPTANCE_PLUS_CHARACTERISTIC_CHART
common_to_relative_transducer=OPEN
protected_production_rail=OPEN
production_integration=NONE
```
