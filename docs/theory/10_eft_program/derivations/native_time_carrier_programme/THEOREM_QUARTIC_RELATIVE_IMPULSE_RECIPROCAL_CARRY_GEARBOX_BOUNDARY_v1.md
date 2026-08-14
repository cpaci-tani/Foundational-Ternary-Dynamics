# Theorem — Quartic-relative impulse and reciprocal-carry gearbox boundary v1

**Identifier:** `FTD-0898`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT CONDITIONAL QUARTIC-RELATIVE IMPULSE]` +
`[THEOREM — EXACT RELATIVE ENERGY AND RECIPROCAL-CARRY COMPOSITION]` +
`[THEOREM — CONDITIONAL SIGNED-STEP REVERSAL]` +
`[THEOREM — CONTINUUM G* PERIOD FACTOR]` +
`[BOUNDARY — COMMON COUPLING/SCALE/FINITE-TICK CADENCE OPEN]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]`

## 1. Result

FTD-0898 closes one part of the FTD-0897 debt without promoting it to a
physical matter--field theory. The already selected local relative-quartic
recursion generates its own equal-and-opposite two-channel impulse, conserves
a positive energy exactly, composes exactly with the reciprocal-carry ledger,
and carries the same continuum lemniscatic period factor `G*` as the quartic
clock. The common mode remains exactly decoupled.

Let

\[
C=\frac{L+R}{\sqrt2},\qquad D=\frac{L-R}{\sqrt2},\qquad
P_C=\frac{P_L+P_R}{\sqrt2},\qquad
\Pi=\frac{P_L-P_R}{\sqrt2}.                                \tag{1}
\]

The orthogonal chart preserves the canonical one-form and quadratic momentum
norm. For `m>0`, `lambda>0`, and signed step `h`, choose the discrete-gradient
endpoint recursion

\[
D_1-D_0=\frac{h}{2m}(\Pi_1+\Pi_0),                          \tag{2}
\]

\[
\Pi_1-\Pi_0=-h\lambda
(D_1^3+D_1^2D_0+D_1D_0^2+D_0^3).                           \tag{3}
\]

It exactly preserves

\[
H_D(D,\Pi)=\frac{\Pi^2}{2m}+\lambda D^4.                  \tag{4}
\]

## 2. Generated two-channel impulse

Holding `P_C` fixed and reconstructing the channel momenta gives

\[
P_L=\frac{P_C+\Pi}{\sqrt2},\qquad
P_R=\frac{P_C-\Pi}{\sqrt2}.                                \tag{5}
\]

Therefore the recursion itself generates

\[
\Delta P_L=+\frac{\Delta\Pi}{\sqrt2},\qquad
\Delta P_R=-\frac{\Delta\Pi}{\sqrt2},                     \tag{6}
\]

and

\[
P_L'+P_R'=P_L+P_R=\sqrt2 P_C.                              \tag{7}
\]

This removes the externally supplied increment from the selected reference
recursion: the internal relative force fixes `Delta Pi`. It does not identify
`L` as matter, `R` as field or reaction, or either channel as a production
substrate degree of freedom.

## 3. Exact reciprocal-carry composition

For an imposed positive conversion unit `p_*`, define

\[
q=\frac{\Delta\Pi}{\sqrt2 p_*}.                            \tag{8}
\]

Write each dimensionless channel momentum as

\[
\frac{P_a}{p_*}=k_a+2\pi w_a,qquad k_a\in[-\pi,\pi),
\quad w_a\in\mathbb Z.                                    \tag{9}
\]

Applying the FTD-0897 transaction with increments `+q,-q` reproduces the
independently decomposed endpoint charts, including one-zone and multiple-zone
crossings. Its aggregate carry satisfies

\[
k_L'+k_R'+2\pi W'=k_L+k_R+2\pi W,                          \tag{10}
\]

where `W=w_L+w_R`. Thus the principal-label loss is repaired exactly while
the physical common momentum (7) remains invariant.

Changing `h` to `-h` regenerates the inverse relative endpoint and the inverse
carry. Full reference-state reversal is exact for the registered endpoint
equations in exact arithmetic; the isolated implementation verifies the same
identity within its declared solver tolerance. This is conditional reversal
of the selected discrete-gradient map, not a derivation of autonomous
substrate history hardware.

## 4. `G*` statement

The continuum Hamiltonian associated with (4) has turning amplitude `A` and

\[
\boxed{T A=\sqrt\pi\,G^*\sqrt{\frac{m}{2\lambda}}},
\qquad
G^*=\frac{\Gamma(1/4)}{\Gamma(3/4)}.                       \tag{11}
\]

Hence the same selected quartic relative mode supports both an internal
impulse cycle and the exact continuum quartic period factor. `G*` is not
inserted into the discrete update or the carry rule.

Equation (11) does not determine a finite integer-tick recurrence. The
energy-preserving discrete-gradient orbit is not the exact sampled continuum
flow, and no theorem here identifies its phase crossings with the global
ontic update index `n`. `G*` is therefore a continuum traversal factor in this
certificate, not yet the substrate gearbox ratio.

## 5. Why this does not yet produce matter or mass

The exact decoupling in (7) is simultaneously a strength and the decisive
boundary. It proves clean internal reaction accounting but forbids transfer
between the relative clock/reaction sector and the common sector. A physical
gearbox needs one local common action whose coupling changes the appropriate
common/matter variable while conserving the full energy and total momentum.

The chart unit also remains imposed. Under `p_* -> s p_*`, equation (8) and
the labels rescale while the dimensional channel impulse (6) is unchanged.
The carry algebra cannot fix `p_*`, the physical momentum map `B` required by
FTD-0893, or an absolute inertial mass.

Nor is the integer carry an energy reservoir. Its bookkeeping is exact, but
equation (4) accounts only for the relative quartic energy. FTD-0886's
phase-cylinder obstruction remains active: chart history cannot be promoted
to a globally Hamiltonian action battery by a phase-blind post-hoc drain.

## 6. Epistemic accounting

The following are theorem-grade inside the selected reference model:

- canonical common/relative splitting;
- exact discrete-gradient conservation of the relative quartic energy;
- generation of equal-and-opposite channel impulses from the relative state;
- exact common-momentum invariance;
- exact reciprocal-wrap/carry endpoint composition;
- conditional signed-step reversal; and
- the exact continuum beta/gamma identity yielding (11).

The following remain open:

- a substrate-derived common/relative coupling;
- physical matter--field or source--reaction channel identification;
- the momentum unit `p_*` and complete total-momentum map;
- an energy-bearing physical realization of reciprocal carry;
- exact integer-tick `G*` phase cadence;
- absolute mass, constituent formation, and stable recursive matter;
- production migration, Born recovery, Bell laboratory recovery, and
  operational Lorentz hiding.

No selected type, adoption currency, phenomenological calibration, or Born
target is added.

## 7. Certificate and implementation

The frozen preregistration SHA-256 is
`AFED9B9F633921281E770E9CEE603A1905847DC99BB6FB552401A0C44CA2086D`.
The frozen source-locked certificate
`scripts/proofs/proof_quartic_relative_impulse_reciprocal_carry_gearbox_boundary.py`
has SHA-256
`4C4203A119FD7614C164E9CD5AC7C749D98285F6D8D7EFD8487680D5C070969C`
and passed `97/97` on its first immutable execution without repair.

The isolated fail-closed reference implementation is:

- `engine/include/ftd/eft/quartic_relative_carry_gearbox.h`, SHA-256
  `9C47BFEBE75FE61070720E53BC583CF7B9CD118C6E9E59435D4FB95B7A4BF83E`;
- `engine/src/eft/quartic_relative_carry_gearbox.cpp`, SHA-256
  `835E4F68D53A07D88631A1C048556201865FD9B1A5D62DDC842869F8F12C2567`;
  and
- `engine/tests/test_quartic_relative_carry_gearbox.cpp`, SHA-256
  `539894B080076EB47D85DEEA7BEB6D8121554BF71B6CC4F491DB968A4762AD0B`.

The focused CTest passes `1/1`, and the isolated actualization chain passes
`26/26`. The implementation changes no production
`Voxel`, field, renderer, boundary, default toggle, or tick phase.

## 8. Next acceptance gate

Break the exact common/relative decoupling with one preregistered local
coupling derived from a common substrate action. It must transfer impulse
between the relative clock/reaction sector and the common matter/field sector
while preserving full energy, total momentum, orientation, causal locality,
and enough history for exact reversal. It must then either fix `p_*` or expose
that unit honestly as irreducible calibration. Integer-tick phase crossings
must be tested separately against global update order and may not be inferred
from the continuum identity (11).

```text
RELATIVE_QUARTIC_INCREMENT_ORIGIN=EXACT_INSIDE_SELECTED_REFERENCE_RECURSION
CHANNEL_IMPULSES=EXACT_EQUAL_AND_OPPOSITE
RELATIVE_ENERGY=EXACTLY_CONSERVED
RECIPROCAL_CARRY_COMPOSITION=EXACT
SIGNED_STEP_REVERSAL=EXACT_CONDITIONAL
CONTINUUM_GSTAR_PERIOD=EXACT_CONDITIONAL_ON_SELECTED_QUARTIC
COMMON_MODE_COUPLING=OPEN
MATTER_FIELD_IDENTIFICATION=OPEN
PHYSICAL_MOMENTUM_SCALE=OPEN
INTEGER_TICK_GSTAR_CADENCE=OPEN
CARRY_ENERGY_LAW=OPEN
ABSOLUTE_MASS=NOT_DERIVED
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```
