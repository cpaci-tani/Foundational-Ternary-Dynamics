# Theorem — Clock-gated Hamiltonian exchange and quartic load boundary (FTD-0865)

**Status:** `[THEOREM — MINIMUM TWO-MODE SYMPLECTIC SWAP LIFT]` +
`[THEOREM — AUTONOMOUS HARMONIC CLOCK-GATED HOLD/SWAP]` +
`[THEOREM — EXACT TRANSIENT REFERENCE-ACTION RESERVE LEDGER]` +
`[THEOREM — STRICTLY CONVEX CLOCK LOAD-DEPENDENT PULSE AREA]` +
`[IMPOSED REFERENCE HAMILTONIAN LAW — CONSUMES EXISTING PHASE-RAIL TYPES]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[CLOSED NEGATIVE — UNIVERSAL LOAD-BLIND QUARTIC G* SWAP IN THE REGISTERED MINIMAL CLASS]` +
`[OPEN — DYNAMIC ELIGIBILITY, COMPENSATION, QUARTIC ACTION-ANGLE REALIZATION, CUBIC TRANSPORT, PRODUCTION COUPLING, AND OPERATIONAL HIDING]`  
**Date:** 2026-08-11  
**Invalid parent:** FTD-0864, exact certificate `39/40`; all sources and
mathematics passed, C34 used structural equality on equivalent expressions  
**Repair protocol:**
[`PREREG_CLOCK_GATED_HAMILTONIAN_EXCHANGE_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_CLOCK_GATED_HAMILTONIAN_EXCHANGE_CERTIFICATE_REPAIR_v2.md),
pre-run SHA256
`6564693B8397CBDBC5C3119ADA2342708F4AFC6DBAE30919A20B83473998B127`  
**Repaired certificate:**
`scripts/proofs/proof_clock_gated_hamiltonian_exchange_v2.py`, SHA256
`F551824660A40B1D0CE1DE333793BE3EB54EDF118CACFC5048531B74144CA950`,
inherited `40/40 PASS`

## 1. Result

FTD-0863's reciprocal matter/signal transaction has an exact autonomous
Hamiltonian lift, but the lift reveals two costs hidden by the instantaneous
swap abstraction.

1. A scalar two-channel swap is orientation reversing and cannot be a
   Hamiltonian time map on one canonical pair. Matter and signal must each
   retain a complete canonical mode.
2. A phase-controlled swap is not free. The clock reference transiently lends
   action to the interaction and must satisfy a load-dependent reserve bound.

For an isochronous harmonic action clock, the borrowed action returns exactly
after one cycle and the swap angle is independent of the event load. For a
strictly convex nonlinear clock—including the pure quartic clock whose
action-space Hamiltonian is proportional to `I^(4/3)`—the backreaction changes
the clock rate and makes the pulse area strictly load dependent.

Therefore the minimum construction does **not** identify the quartic `G*`
clock with a universal exact matter/signal gearbox. A complete architecture
must either separate the isochronous orientation reference from the quartic
eligibility calendar, add a compensating dynamical reservoir/controller, or
restrict and declare the admitted load sector.

## 2. Why the scalar swap is not Hamiltonian

For one proposed canonical pair `(m,a)`, the FTD-0856 open branch is

\[
 S=\begin{pmatrix}0&1\\1&0\end{pmatrix},
 \qquad \det S=-1,
 \qquad S^TJS=-J.                                \tag{1}
\]

Every Hamiltonian time map is symplectic and orientation preserving. Thus (1)
cannot be generated as a closed two-dimensional Hamiltonian flow.

Let matter and signal instead be complete modes

\[
 M=(q_m,p_m),\qquad D=(q_d,p_d),                 \tag{2}
\]

and define

\[
 C=\frac{M+D}{\sqrt2},\qquad
 R=\frac{M-D}{\sqrt2}.                          \tag{3}
\]

This four-dimensional transform is orthogonal and symplectic. The full mode
swap is `C -> C`, `R -> -R`; it has determinant `+1` and preserves the complete
symplectic form. With the clock pair included, the minimum registered closed
phase space is six dimensional.

This is a degree-of-freedom lower bound inside the declared instantaneous
swap semantics. It does not prove that these modes are already localized in
production.

## 3. Autonomous harmonic phase gate

Let

\[
 I_c=\frac{|C|^2}{2},\qquad
 I_r=\frac{|R|^2}{2},\qquad
 A=I_c+I_r=\frac{|M|^2+|D|^2}{2}.               \tag{4}
\]

For a frozen eligibility sector `epsilon in {0,1}`, impose

\[
 H_\epsilon
 =\omega I+\nu A
  +\epsilon\chi(1-\cos\theta)I_r.              \tag{5}
\]

Equation (5) is autonomous and onsite. It is an **imposed reference law**, not
a consequence of P1--P5 or production C18.

Hamilton's equations are

\[
 \dot\theta=\omega,
 \qquad
 \dot I=-\epsilon\chi\sin\theta,I_r,
 \qquad
 \dot I_c=\dot I_r=0.                           \tag{6}
\]

Starting at a gate zero `theta=0`, the exact solution is

\[
 I(\theta)=I_0-epsilon\frac\chi\omega
 I_r(1-\cos\theta).                             \tag{7}
\]

Substitution into (5) gives the constant energy

\[
 H_\epsilon=\omega I_0+\nu A.                  \tag{8}
\]

The reference energy lost during the pulse is exactly the interaction energy
gained. Nothing is assigned to an unbooked controller reservoir.

## 4. Exact stroboscopic hold and swap

One clock cycle has `T=2pi/omega`. The common mode rotates through

\[
 \Phi_c=\frac{2\pi\nu}{\omega},                 \tag{9}
\]

and the active relative mode acquires the additional angle

\[
 \Xi=\frac\chi\omega\int_0^{2\pi}(1-\cos\theta)d\theta
    =\frac{2\pi\chi}{\omega}.                  \tag{10}
\]

Therefore

\[
 \frac\nu\omega\in\mathbb Z,
 \qquad
 \frac{2\chi}{\omega}\in2\mathbb Z+1          \tag{11}
\]

makes the inactive branch identity and the active branch the exact full-mode
swap. The minimum positive winding is

\[
 \nu=\omega,\qquad\chi=\frac\omega2.           \tag{12}
\]

No target probability or event energy enters (11)--(12). For every admitted
load, the same harmonic winding exchanges

\[
 (M,D)\longmapsto(D,M).                         \tag{13}
\]

Emission and absorption are the same stroboscopic involution.

## 5. Exact reserve and recursive reuse

Because `0<=1-cos(theta)<=2`, an active cycle retains a physical positive
reference action only if

\[
 I_0>\frac{2\chi}{\omega}I_r.                  \tag{14}
\]

At the minimum winding this is `I_0>I_r`. For emission from `D=0` with event
energy

\[
 B=\frac{|M|^2}{2},
 \qquad I_r=\frac B2,                           \tag{15}
\]

the exact capacity condition is

\[
 I_0>\frac B2.                                 \tag{16}
\]

The minimum action is `I_min=I_0-B/2`; the maximum reference-energy loan is
`omega B/2`. At the cycle endpoint, `I(T)=I_0` and the interaction energy is
zero. Consequently completed, nonoverlapping cycles do not cumulatively
deplete the ideal harmonic reference.

This proves recursive reuse only for a complete cycle inside the reference
Hamiltonian. Interrupted pulses, overlapping events, perturbations, formation,
and synchronization recovery remain open.

## 6. Why the quartic clock is not the same gearbox

Replace the isochronous term `omega I` by a nonlinear clock Hamiltonian `K(I)`.
Energy conservation gives

\[
 K(I(\theta))+\chi(1-\cos\theta)I_r=K(I_0).     \tag{17}
\]

The clock rate is now `theta_dot=K'(I)`, so the relative pulse area is

\[
 \Xi(I_r)=\int_0^{2\pi}
 \frac{\chi(1-\cos\theta)}{K'(I(\theta;I_r))}
 \,d\theta.                                    \tag{18}
\]

Implicit differentiation gives

\[
 \frac{d\Xi}{dI_r}
 =\chi^2\int_0^{2\pi}
 \frac{(1-\cos\theta)^2K''(I)}{K'(I)^3}
 \,d\theta.                                    \tag{19}
\]

If `K'>0`, `K''>0`, and the pulse is nontrivial, (19) is strictly positive.
Thus no single fixed `chi` gives the same exact swap angle for two distinct
nonzero loads in the registered minimal class.

For the pure quartic oscillator, homogeneity yields

\[
 K(I)=cI^{4/3},\qquad
 K'(I)=\frac{4c}{3}I^{1/3}>0,qquad
 K''(I)=\frac{4c}{9}I^{-2/3}>0.                 \tag{20}
\]

The exact `G*` factor in the quartic period is not disputed. It specifies the
waveform traversal of the adopted quartic Hamiltonian. It does not make
`K''` vanish and therefore does not cancel the load dependence (19).

The closed-negative claim is scoped to (17)--(20). It does not exclude:

- a separate harmonic orientation pilot gated by a quartic calendar;
- an additional compensating action reservoir;
- a controller whose state is dynamically correlated with the admitted load;
- a fixed single-load sector; or
- approximate weak-backreaction operation.

Each alternative carries a distinct, testable cost.

## 7. Eligibility boundary

The value `epsilon` selects hold or exchange in (5), but it is frozen. It is
not a derived event controller. A closed dynamic eligibility mechanism must
add at least one distinguishable controller state and its conjugate/work or
environment ledger, then show how production target-blind acceptance actuates
that state without reading an outcome or Born weight.

Thus FTD-0865 closes clock-timed Hamiltonian exchange, not actualization as a
whole.

## 8. Isolated implementation

The isolated API is
[`clock_gated_hamiltonian_exchange.h`](../../../../../engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h),
SHA256
`0BDEF8D6278FDF352F89C739F995F337B76AECC8C4FE716DF899B4058DE8A29E`.
Its source and focused test are:

- [`clock_gated_hamiltonian_exchange.cpp`](../../../../../engine/src/eft/clock_gated_hamiltonian_exchange.cpp),
  SHA256
  `5961CCDFBA652B473C6D035B6243B52DEBBAEBCAAB63F88C424EE547E78683C3`;
- [`test_clock_gated_hamiltonian_exchange.cpp`](../../../../../engine/tests/test_clock_gated_hamiltonian_exchange.cpp),
  SHA256
  `570F95D8EE38DC00896FFF856A4882472908F45BB57D45E2AD5CF17BE7EDD7E2`.

The API evaluates the exact one-cycle flow, reports common/relative actions,
winding compliance, minimum reference action, maximum interaction energy,
reference-energy loan, and endpoint energy residual. It fails closed on an
invalid gate phase, frequency, coupling, mode, eligibility, or insufficient
strict reserve. The focused Release CTest passes `1/1`.

No `Voxel`, production field, toggle, default, tick phase, dynamic eligibility,
quartic controller, or `G*` parameter was added.

## 9. Certificate record

The invalid parent is preserved at `39/40`. The FTD-0865 one-repair wrapper
returned:

```text
FTD-0864 clock-gated Hamiltonian exchange: 40/40 PASS
SCALAR_SWAP_REQUIRES_TWO_MODE_SYMPLECTIC_LIFT
AUTONOMOUS_HARMONIC_PHASE_GATE_GIVES_EXACT_HOLD_SWAP_AND_RESERVE_LEDGER
STRICTLY_CONVEX_QUARTIC_CLOCK_HAS_LOAD_DEPENDENT_SWAP_ANGLE
DYNAMIC_ELIGIBILITY_COMPENSATION_GSTAR_GEARBOX_AND_PRODUCTION_REMAIN_OPEN
VERDICT=OUTCOME_B_EXACT_HARMONIC_LIFT_QUARTIC_CONTROLLER_BOUNDARY
FTD-0865 certificate repair: PASS
REPAIR_SCOPE=C34_EXACT_SIMPLIFIED_DIFFERENCE_ONLY
PARENT_FTD0864_PRESERVED_INVALID
```

No Born-frequency, Bell-correlation, Hilbert-recovery, biological,
consciousness, production, CM/substrate-identification, operational-Lorentz,
or whole-framework completeness claim is made.
