# FTD-0958/0959 — Global isochrony lift and oriented crossing-latch boundary v1

**Date:** 2026-08-11  
**Status:** `[CLOSED NEGATIVE — GLOBAL ISOCHRONY OF THE REGISTERED SMOOTH PERIODIC NATURAL-WELL CLASS]` +
`[SELECTION — LIFTED HARMONIC PHASE AND INTEGER WINDING HISTORY]` +
`[THEOREM — EXACT ISOCHRONOUS LIFT/ORIENTED SYMPLECTIC ATLAS TRANSITION]` +
`[THEOREM — SIGNED CROSSING CURRENT DISTINGUISHES CLOCKWISE/COUNTERCLOCKWISE]` +
`[THEOREM, CONDITIONAL — ONE-SHOT TERNARY ELIGIBILITY/REVERSIBLE CLEARING]` +
`[THEOREM — FINITE WINDING-MEMORY CAPACITY AND NO-RESET COMMENSURABILITY]` +
`[BOUNDARY — NATIVE WINDING CARRIER, ACTIVE GEARBOX, AND G* IDENTIFICATION OPEN]`  
**Verdict:** `OUTCOME_B_EXACT_LIFT_AND_ORIENTATION_AT_EXPLICIT_MEMORY_GEARBOX_PRICE`

## 1. Result

The FTD-0957 synchronization sector cannot obtain all three of the following
for free:

1. a smooth globally periodic natural phase well;
2. exact amplitude-independent cadence across its complete libration basin;
3. no retained lift, winding history, or active feedback.

The obstruction is the basin barrier: every real-analytic periodic natural
well in the registered finite-order class has a period that diverges as its
libration energy approaches the barrier.

Exact isochrony can be recovered on the lifted phase

\[
 \widetilde\delta=\delta+2\pi w,
 \qquad w\in\mathbb Z,                                 \tag{1}
\]

with a harmonic energy. But the integer `w` is then real physical accounting:
it is the retained history that says which copy of the phase circle the system
occupies.

The direction needed to update this lift is already present in the signed
phase current

\[
 s=\operatorname{sign}\Pi.                             \tag{2}
\]

This is the exact clockwise/counterclockwise distinction. The symmetric
eligibility `s^2` detects that a crossing occurred while losing which way it
occurred. Thus the oriented current and its symmetric square have different
jobs and may not be identified.

## 2. Why a smooth periodic natural well is not globally isochronous

Consider

\[
 H(\delta,\Pi)={\Pi^2\over2M}+V(\delta),
 \qquad M>0,                                           \tag{3}
\]

with a nonconstant real-analytic `2pi`-periodic potential. The full period of
a libration of energy `E` is

\[
 T(E)=\sqrt{2M}\int_{\delta_-(E)}^{\delta_+(E)}
 {d\delta\over\sqrt{E-V(\delta)}}.                    \tag{4}
\]

Let `E_b` be the barrier energy and suppose the barrier has finite even order
`2r`, `r>=1`. With `epsilon=E_b-E`, its terminal contribution is bounded
below by a positive multiple of

\[
 I_r(\epsilon)=\int_0^a{dx\over\sqrt{\epsilon+x^{2r}}}. \tag{5}
\]

For a quadratic barrier,

\[
 I_1(\epsilon)=\operatorname{asinh}(a/\sqrt\epsilon)
 \longrightarrow+\infty.                               \tag{6}
\]

For `r>1`, the rescaling `x=epsilon^(1/(2r))y` gives the
factor

\[
 \epsilon^{(1-r)/(2r)},                                \tag{7}
\]

which also diverges. Hence

\[
 \boxed{T(E)\to+\infty\quad\text{as}\quad E\to E_b^-}. \tag{8}
\]

No finite constant period can satisfy (8). This closes global isochrony only
inside the declared analytic periodic natural class. Nonsmooth walls,
energy-dependent kinetic laws, selected action reparameterizations, local
lifts, and feedback lie outside the theorem.

## 3. Exact lifted harmonic witness

Adopt the selected reference energy

\[
 \boxed{
 H_{\rm lift}={\Pi^2\over2M}
              +{K\over2}(\delta+2\pi w)^2,}
 \qquad M,K>0.                                         \tag{9}
\]

Within one chart, Hamilton's equations are

\[
 \dot{\widetilde\delta}={\Pi\over M},
 \qquad
 \dot\Pi=-K\widetilde\delta.                         \tag{10}
\]

They have the exact energy-independent period

\[
 \boxed{T_{\rm iso}=2\pi\sqrt{M/K}.}                  \tag{11}
\]

The flow is positive, symplectic, reversible, and preserves (9) exactly.

At fixed `w`, however,

\[
 H_{\rm lift}(\delta+2\pi,w)-H_{\rm lift}(\delta,w)
 =2\pi K\widetilde\delta+2\pi^2K,                    \tag{12}
\]

which is generally nonzero. Equation (9) is therefore not a single-valued
Hamiltonian on the bare phase circle. Its exact cadence is purchased by the
lift (1).

## 4. The oriented atlas transition

At the branch cut, let `s=sign(Pi)`. Apply

\[
 \boxed{
 \delta'=\delta-2\pi s,
 \qquad
 w'=w+s,
 \qquad
 \Pi'=\Pi.}                                            \tag{13}
\]

Then

\[
 \delta'+2\pi w'=\delta+2\pi w,                       \tag{14}
\]

so the lifted phase and energy are invariant. On either fixed-orientation
branch, `d delta'=d delta`; hence

\[
 \Pi' d\delta'=\Pi d\delta.                            \tag{15}
\]

The transition is an exact symplectic atlas change with an exact inverse.

For positive current it maps

\[
 (+\pi,w)\mapsto(-\pi,w+1),                            \tag{16}
\]

while negative current maps

\[
 (-\pi,w)\mapsto(+\pi,w-1).                            \tag{17}
\]

Both have `s^2=1`. Therefore the symmetric square cannot decide whether to
apply `w+1` or `w-1`.

This is the same structural loss previously seen in the BCC/symmetric-square
question: an even representation can retain eligibility or magnitude while
identifying opposite orientations. The signed current is the minimum datum
that separates them.

## 5. One-shot ternary crossing latch

At a zero-phase crossing define

\[
 s=\operatorname{sign}\Pi\in\{-1,0,+1\},              \tag{18}
\]

where `s=0` is the exact lock/no-crossing value. In the even degree-at-most-two
class, the conditions

\[
 e(0)=0,
 \qquad e(+1)=e(-1)=1                                  \tag{19}
\]

uniquely give

\[
 \boxed{e(s)=s^2.}                                     \tag{20}
\]

Equation (20) is the eligibility clutch. The separate signed `s` is the
orientation record. Squaring `s` does not erase it if the signed latch or its
oriented outgoing signal remains present.

For a coupling `e C(varphi,z)` satisfying `C(0,z)=0`, changing eligibility at
controller gate zero has exact zero switching-energy difference. Off gate it
generally has nonzero work. This inherits the FTD-0867 gate-zero result and
does not pay latch acquisition, barrier, bath, or transport costs.

Conditional on the existing FTD-0871 oriented-signal interface, the completed
signal can reversibly uncompute the one-shot ternary latch after the compiler
transaction. No extra logical acknowledgement bit is necessary. The physical
acquisition trajectory and output transport remain open.

## 6. One crossing is not indefinite winding memory

The ternary latch has three states: no crossing and the two orientations. It
does not contain arbitrary `w in Z`.

Over a declared finite horizon `|w|<=W`, exact winding storage needs at least

\[
 2W+1                                                     \tag{21}
\]

distinguishable records. A length-`n` ternary rail can suffice only if

\[
 \boxed{3^n\ge2W+1.}                                  \tag{22}
\]

No finite `n` stores every integer winding injectively. Indefinite lifted
dynamics therefore require unbounded signed export, a physical recurrence or
identification that makes some windings equivalent, or another retained state
family. Calling `w` “just coordinates” would hide this history cost.

## 7. Crossing detection is not controller alignment

Let

\[
 \kappa=\sqrt{K/M}                                    \tag{23}
\]

and let an independent controller obey `varphi_dot=Omega`. Same-orientation
zero crossings recur after `2pi/kappa`, so

\[
 \Delta\varphi_{\rm same}=2\pi{\Omega\over\kappa}.     \tag{24}
\]

All zero crossings recur after `pi/kappa`, so

\[
 \Delta\varphi_{\rm all}=\pi{\Omega\over\kappa}.      \tag{25}
\]

If the controller begins at gate zero, it returns to gate zero at every
same-orientation crossing exactly when

\[
 \boxed{\Omega/\kappa\in\mathbb Z,}                   \tag{26}
\]

and at every crossing exactly when

\[
 \boxed{\Omega/\kappa\in2\mathbb Z.}                 \tag{27}
\]

Changing only `e=s^2` does not change `varphi`. A crossing latch can authorize
a gate that is already aligned; it cannot move an arbitrary controller to
gate zero. Without reset, exact engagement requires the commensurate ratio and
initial phase origin, or an active clutch whose work, reserve, reciprocal
reaction, and inverse are explicitly closed.

Equations (26)--(27) are the missing gearbox condition in exact form. They do
not derive that gearbox, select its integer, or identify either frequency with
the critical-quartic `G*` calendar.

## 8. Epistemic and ontology accounting

Theorem-grade:

- barrier divergence and the registered global-periodic-natural isochrony
  no-go;
- exact lifted harmonic flow, period, symplecticity, energy, and inverse;
- failure of fixed-winding single-valuedness on the circle;
- exact oriented branch transitions and preservation of lifted phase;
- necessity of signed current to distinguish the two winding updates;
- unique even quadratic ternary eligibility;
- finite winding-memory and ternary-rail capacity bounds; and
- exact no-reset controller commensurability conditions.

Selected or imposed:

- the harmonic lifted energy and positive scales;
- the integer winding-history type and branch-cut convention;
- the existing ternary latch, phase reference, actuator, and signal interfaces;
- the controller frequency and initial phase origin.

Open:

- a substrate-native winding/history carrier or finite-horizon signed export;
- a physical crossing-latch acquisition trajectory;
- an active no-reset clutch with complete energy/work/inverse closure;
- derivation of the commensurate gearbox and any relation to `G*`;
- full nonlinear repeated charge/Routh/synchronization stability;
- attraction through complete positive history export;
- native source/reservoir formation, finite 3D routing/recycling, mobility,
  collision, erasure, mass, scale, `gamma`, and production;
- Born/Bell recovery, Lorentz hiding, and completeness; and
- every engine integration.

## 9. Certificate and repair provenance

Parent protocol SHA-256:
`927F60B630584EDBFFD40922C25D1E57F97C09B2F175C696C1D2FE29C27782FE`.
The immutable parent certificate SHA-256 is
`2F8F237E01E2B60AFD7614348537345F470CEEC672020E3E93F3A3B9232898E6`.
Its first execution reached `91/93`, Outcome D, on two structural-equality
normalization defects.

Repair protocol SHA-256:
`1B31C1D074E3D455791CDD0EA5AF0CB9C3CFAEA299742FA81911EA563ECC29E0`.
The in-memory repair wrapper SHA-256 is
`9FDE28BE5D2C9433E99A60733501794A025D1646350B1ED836EF34270F557CF1`.
Its first execution passes inherited `93/93` plus repair integrity `12/12`,
Outcome B. Both parent files remain byte-preserved.

No numerical search, parameter scan, floating tolerance, empirical
substitution, engine source, CMake file, `Voxel` type, production field,
constant, toggle, or default tick phase changed.

## 10. Next gate

The next mechanism should attack the gearbox rather than add another clock:

1. preregister a finite-capacity signed winding/history rail with explicit
   backpressure and inverse;
2. test whether the existing parity rail and oriented signal carrier can
   realize (13) without a new production type;
3. construct or close negative an active no-reset clutch that enforces
   (26) or (27) with exact work and reciprocal reaction;
4. keep the quartic `G*` calendar separate until an exact substrate gearbox
   identifies its phase and rate; and
5. separately certify the nonlinear repeated map and any positive export used
   for attraction.

```text
GLOBAL_PERIODIC_NATURAL_ISOCHRONY=CLOSED_NEGATIVE_IN_REGISTERED_CLASS
LIFTED_HARMONIC_ISOCHRONY=EXACT_SELECTED_REFERENCE
WINDING_HISTORY=REQUIRED_FOR_GLOBAL_LIFT
SIGNED_PHASE_CURRENT=DISTINGUISHES_CLOCKWISE_COUNTERCLOCKWISE
SYMMETRIC_SQUARE=ELIGIBILITY_ONLY_ORIENTATION_LOST
ORIENTED_BRANCH_TRANSITION=EXACT_SYMPLECTIC
ONE_SHOT_TERNARY_CROSSING_LATCH=SUFFICIENT_CONDITIONAL
INDEFINITE_WINDING_IN_FINITE_LATCH=IMPOSSIBLE
SAME_ORIENTATION_NO_RESET_ALIGNMENT=OMEGA_OVER_KAPPA_INTEGER
EVERY_CROSSING_NO_RESET_ALIGNMENT=OMEGA_OVER_KAPPA_EVEN_INTEGER
ACTIVE_GEARBOX_AND_GSTAR_IDENTIFICATION=OPEN
NATIVE_WINDING_CARRIER_PRODUCTION_BORN_BELL_LORENTZ=OPEN
```

