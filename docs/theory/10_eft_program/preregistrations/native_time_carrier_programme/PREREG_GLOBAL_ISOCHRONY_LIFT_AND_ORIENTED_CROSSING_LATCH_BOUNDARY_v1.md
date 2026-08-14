# FTD-0958 pre-registration — Global isochrony lift and oriented crossing-latch boundary v1

**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** proof-only continuation of FTD-0956/0957. No engine, CMake,
`Voxel`, production field, constant, toggle, default tick phase, numerical
search, Born/Bell path, or `G*` law may change.

## 1. Question

Can the selected FTD-0957 conservative synchronization sector be made exactly
isochronous and release the already-global FTD-0955 controller at each
zero-phase crossing without resetting phase, losing clockwise/counterclockwise
orientation, or importing unbooked memory?

Five verdicts are separate:

1. global isochrony of a smooth periodic natural phase well;
2. exact isochrony on a lifted phase;
3. the information needed to distinguish the two crossing orientations;
4. one-shot eligibility and reversible clearing of a crossing latch; and
5. no-reset alignment of an independently rotating controller.

A lifted harmonic witness may not be counted as a globally single-valued law
on the phase circle. Crossing detection may not be counted as controller
alignment or as the missing `G*` gearbox.

## 2. Frozen sources

| source | SHA-256 | role |
|---|---|---|
| `THEOREM_RELATIVE_ACTION_CURVATURE_SYNCHRONIZATION_AND_CROSSING_SECTION_ENERGY_BOUNDARY_v1.md` | `589A0B4D1C5906510B4432841BC86E0DA4C3B9F1FB1F1FA6C3EBF817C24BD8A7` | positive synchronization pair, non-isochrony, crossing ledger |
| `proof_relative_action_curvature_synchronization_crossing_section_energy_v2.py` | `28E1CB38FCC5653D984D2555BFB0D94B916DCD7C952E3A03661D6F531127323D` | repaired `111/111 + 12/12` proof of record |
| `THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md` | `6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D` | `s^2` eligibility and gate-zero switch-work boundary |
| `THEOREM_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_AND_RESET_BOUNDARY_v1.md` | `F52BE0CD97FAE06CF6A39C6E0784EC75746F7B8ABF9843C4EF78B37181C8D2CC` | retained oriented signal and reversible actual-layer clearing |
| `THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md` | `73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98` | exact harmonic isochrony and gate-zero actuation ledger |
| `SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md` | `E5E21BCB0D9F16825ED4FEEE9B915E2835F16F9446F0D636C801A4316CB0D0C5` | signed phase-current requirement and symmetric-square loss |

## 3. Registered periodic natural class

Consider

\[
 H(\delta,\Pi)={\Pi^2\over2M}+V(\delta),
 \qquad M>0,                                           \tag{1}
\]

where `V` is a nonconstant real-analytic `2pi`-periodic function with a stable
minimum and a finite-order barrier maximum bounding its libration basin. If
the barrier energy is `E_b`, the full period is

\[
 T(E)=\sqrt{2M}\int_{\delta_-(E)}^{\delta_+(E)}
 {d\delta\over\sqrt{E-V(\delta)}}.                    \tag{2}
\]

Near a barrier of even order `2r`, `r>=1`, the terminal contribution is
bounded below by a positive multiple of

\[
 I_r(\epsilon)=\int_0^a{dx\over\sqrt{\epsilon+x^{2r}}},
 \qquad \epsilon=E_b-E.                               \tag{3}
\]

The certificate must prove `I_r(epsilon)` diverges as `epsilon -> 0+`:
logarithmically for `r=1` and at least as a positive power for `r>1`.
Therefore no member of this registered smooth periodic natural class is
isochronous across its complete libration basin.

This is a scoped theorem. It does not exclude a local lifted phase, a
nonsmooth wall, an energy-dependent kinetic law, a selected action
reparameterization, or feedback.

## 4. Lifted harmonic witness and its price

Introduce a retained winding label

\[
 w\in\mathbb Z,
 \qquad
 \widetilde\delta=\delta+2\pi w.                       \tag{4}
\]

Freeze the selected lifted harmonic witness

\[
 \boxed{
 H_{\rm lift}={\Pi^2\over2M}+{K\over2}\widetilde\delta^2,}
 \qquad K>0.                                           \tag{5}
\]

It must give

\[
 \dot{\widetilde\delta}={\Pi\over M},
 \qquad
 \dot\Pi=-K\widetilde\delta,                         \tag{6}
\]

with exact energy-independent period

\[
 \boxed{T_{\rm iso}=2\pi\sqrt{M/K}.}                  \tag{7}
\]

Equation (5) is not single-valued on the phase circle at fixed `w`. It becomes
consistent only as an atlas on the cover with the branch update in section 5.
The winding label and harmonic law are selected reference types, not substrate
derivations.

## 5. Oriented branch transition

At the phase branch cut define

\[
 s=\operatorname{sign}\Pi\in\{-1,+1\}.                \tag{8}
\]

For the outgoing boundary selected by `s`, apply

\[
 \boxed{
 \delta'=\delta-2\pi s,
 \qquad w'=w+s,
 \qquad \Pi'=\Pi.}                                    \tag{9}
\]

For `s=+1`, this is `(+pi,w)->(-pi,w+1)`; for `s=-1`, it
is `(-pi,w)->(+pi,w-1)`. The certificate must prove

\[
 \widetilde\delta'=\widetilde\delta,                  \tag{10}
\]

so (5), the canonical one-form `Pi d delta`, and orientation are preserved.
The inverse is obtained by the opposite oriented crossing.

The symmetric eligibility value `s^2=1` cannot choose between `w+1` and
`w-1`. Thus clockwise/counterclockwise information is exactly the information
lost by the symmetric square.

## 6. One-shot ternary crossing latch

At a zero-phase crossing extend (8) by

\[
 s=\operatorname{sign}\Pi\in\{-1,0,+1\},              \tag{11}
\]

where `s=0` is the exact lock/no-crossing state. Among even polynomials of
degree at most two, the unique context-blind eligibility map satisfying

\[
 e(0)=0,
 \qquad e(+1)=e(-1)=1                                  \tag{12}
\]

must be

\[
 \boxed{e(s)=s^2.}                                     \tag{13}
\]

The retained signed `s`, not (13), supplies branch orientation. Conditional
on the existing FTD-0867/0871 reference components, the outgoing oriented
signal may retain `s` and reversibly uncompute the local ternary latch after
one completed compiler transaction.

For an interaction `e C(varphi,z)` with `C(0,z)=0`, changing `e` at
`varphi=0 mod 2pi` has exact zero switching-energy difference. This does not
make latch acquisition, barrier crossing, controller dynamics, or output
transport free.

## 7. Finite-memory boundary

The one-shot ternary label stores one crossing orientation; it does not store
the unbounded winding `w`. No finite state set can injectively represent all
of `Z`. Over a declared horizon `|w|<=W`, exact storage needs at least

\[
 2W+1                                                     \tag{14}
\]

distinguishable states. A length-`n` ternary rail can suffice only if

\[
 \boxed{3^n\ge2W+1.}                                  \tag{15}
\]

Indefinite global lifting therefore needs unbounded export, recurrence/
identification, or another retained state family.

## 8. No-reset controller alignment

Let

\[
 \kappa=\sqrt{K/M}                                    \tag{16}
\]

be the lifted harmonic frequency, and let an independent controller obey

\[
 \dot\varphi=\Omega.                                  \tag{17}
\]

Successive same-orientation zero crossings are separated by `2pi/kappa`, so
the controller phase increment is

\[
 \Delta\varphi_{\rm same}=2\pi{\Omega\over\kappa}.     \tag{18}
\]

All successive zero crossings are separated by `pi/kappa`, giving

\[
 \Delta\varphi_{\rm all}=\pi{\Omega\over\kappa}.       \tag{19}
\]

If the controller starts at gate zero, it returns to gate zero at every
same-orientation crossing iff

\[
 {\Omega\over\kappa}\in\mathbb Z,                    \tag{20}
\]

and at every crossing iff

\[
 {\Omega\over\kappa}\in2\mathbb Z.                  \tag{21}
\]

A latch which only changes eligibility does not change `varphi`; therefore it
cannot repair failure of (20) or (21). Without phase reset, exact release
requires the commensurate gearbox and initial phase origin, or an active
clutch/coupling with its own work and inverse ledger.

No relation between `Omega/kappa` and `G*` is frozen or inferred here.

## 9. Frozen outcomes

| outcome | required result | interpretation |
|---|---|---|
| A | A smooth globally periodic natural phase well is isochronous over its complete basin and the ternary latch aligns an arbitrary independent controller without a lift, gearbox, reset, or work law | global native cadence and engagement closure |
| B | Global periodic natural isochrony is closed negative; the lifted harmonic witness is exact but costs winding history; signed ternary crossing data supplies one-shot orientation/eligibility, while indefinite lift and no-reset controller alignment require memory and a commensurate or active gearbox | exact lift/orientation result and priced boundary |
| C | The lift, oriented transition, one-shot latch, or alignment conditions fail algebraically | reject the construction |
| D | Hash, algebra, source, scope, or classifier fails | no theorem |

The frozen expected classifier is Outcome B.

## 10. Acceptance gates

The exact certificate must check:

1. every frozen hash and scope marker;
2. the period formula (2) and divergence comparison (3);
3. the scoped global-isochrony closed negative;
4. positivity, equations, exact period, symplecticity, and inverse of the
   lifted harmonic witness;
5. failure of fixed-`w` `2pi` single-valuedness;
6. both oriented branch transitions and invariance of lifted phase, energy,
   one-form, and sign;
7. the inability of `s^2` to select winding direction;
8. uniqueness of ternary eligibility (13);
9. gate-zero switching-energy difference and its scope;
10. finite-state and ternary-rail winding-capacity bounds;
11. controller increments (18)--(19) and commensurability conditions
    (20)--(21);
12. failure of eligibility-only latching to align an arbitrary controller;
13. no promotion to native lift, unbounded memory, autonomous clutch,
    `G*` gearbox, attraction, production, Born/Bell, Lorentz hiding, or
    completeness; and
14. the frozen outcome classifier.

No numerical parameter search, floating tolerance, empirical substitution,
or completed-infinity limit is permitted.

## 11. Promotion boundary

Outcome B would prove that exact isochrony is not free: global circle
periodicity, stable natural libration, and fixed cadence cannot all be retained
in the registered smooth class without a lift or a more elaborate selected
law. It would also prove that the substrate-relevant oriented crossing current
contains exactly the direction information that symmetric eligibility loses.

Still open would be:

- a substrate-native winding/history carrier or a finite-horizon export law;
- a physical acquisition trajectory for the ternary crossing latch;
- derivation of a commensurate controller/synchronizer gearbox and phase
  origin, including any relation to the critical-quartic `G*` calendar;
- an active no-reset clutch with exact work, reserve, and inverse;
- full nonlinear repeated charge/Routh/sync stability and positive history
  export if attraction is required;
- native reservoir/source formation, finite 3D routing/recycling, mobility,
  collision, erasure, mass, scale, `gamma`, production, Born/Bell, Lorentz
  hiding, and completeness.
