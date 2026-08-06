# Preregistration — Collective Source-History Bound (FTD-0588)

**Status:** `[LOCKED — RUN OF RECORD NOT YET EXECUTED]`  
**Date:** 2026-07-26  
**Production effect:** none.

## 1. Question

FTD-0586 bounded arbitrary one-time source removals by adding the absolute
response of each source and therefore closed endogenous genesis only through
`N=3`. This gate asks whether the exact spatial orthogonality of distinct
ternary sites closes more of the collective-source branch without choosing a
successful geometry or tuning an amplitude.

The target is a theorem for every arrangement and polarity choice on four
registered periodic quotients. Two history classes are separated:

1. all sources have one common temporal history (permanent or removed at one
   shared tick);
2. sources begin together but are removed once at arbitrary individual ticks.

No cluster-shape, source-count, removal-time, or field-amplitude search is
permitted.

## 2. Frozen production contract

| source | SHA-256 |
|---|---|
| `engine/src/render_bridge.cpp` | `A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/include/ftd/ontic/gauge_couplings.h` | `BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3` |
| `engine/include/ftd/ontic/particle_masses.h` | `E43E01D5F1F870EE019754BEA7E932529346C9B8EB704B40215CA559FC5A4F57` |

The live sector is the FTD-0586 sector: CPU, periodic field quotient, unit
tick, single substrate, zero initial flux/wave velocity/matter kinematics,
and only wave propagation, state-gradient coupling, and genesis/evaporation.
Both Gauss mechanisms, damping, forces, movement, pair production, reactions,
clocks, and baths remain off.

## 3. Locked derivation

Use the production symbols

\[
 M(k)=4-\frac23\sum_a c_a-\frac23\sum_{a<b}c_ac_b,
 \qquad c_a=\cos k_a,
 \qquad g(k)^2=\sum_a\sin^2 k_a,
\]

and the FTD-0586 modal step envelope

\[
 D(k)=1+\frac1{\sqrt{1-C_{\rm WAVE}^2M(k)/4}}.
\]

Let `p=sum(c_a)` and `q=sum(c_a^2)`. The exact identity

\[
 3\bigl(M-g^2\bigr)
 =4\left(q-\frac{p^2}{3}\right)+\frac{(p-3)^2}{3}\ge0
\]

must be proved. Equality is confined to the zero mode, so `g(k)^2/M(k)<1`
for every nonzero finite-volume mode.

For `N` distinct unit ternary sources with arbitrary signs and locations,

\[
 S(k)=\sum_{j=1}^{N}q_j e^{-ik\cdot x_j},\qquad q_j\in\{-1,+1\},
\]

finite-group Parseval must give

\[
 \sum_k|S(k)|^2=L^3N.
\]

Cauchy--Schwarz then gives the geometry-independent common-step bound

\[
 |J(x,n)|\le C_L\sqrt N,
 \qquad
 C_L=\frac{G_C}{C_{\rm WAVE}^2}
 \sqrt{\frac1{L^3}\sum_{k\ne0}\frac{D(k)^2}{M(k)}}.
\]

A synchronized rectangular pulse is the difference of two common steps and
obeys `|J| <= 2 C_L sqrt(N)`.

For arbitrary individual one-time removals, decompose the field into the
common initial step plus one delayed negative step for each removed source.
If `r` sources have been removed,

\[
 |J(x,n)|\le C_L\sqrt N+rB_L^{\rm step},
\]

where `B_L^step` is the one-source triangle bound from FTD-0586. The locked
finite-volume gates are

\[
 2C_L\sqrt5<K_{\rm GENESIS},
 \qquad
 2C_L+4B_L^{\rm step}<K_{\rm GENESIS}
\]

for every `L={9,17,33,65}`. The first closes common histories through `N=5`;
the second closes arbitrary one-time removals through `N=4`.

For `N=5`, `C_L sqrt(5)+4B_L^step<K_GENESIS` must also be checked. It would
prove that an independently evaporating five-source cluster cannot generate
a first descendant while even one original source remains. The all-originals-
removed residual-field tail remains outside the theorem if
`C_L sqrt(5)+5B_L^step >= K_GENESIS`.

## 4. Independently specified live fixtures

The geometry is fixed by the smallest four-point cubic-lattice orbit with
zero first moment and isotropic second moment:

\[
 (1,1,1),\ (1,-1,-1),\ (-1,1,-1),\ (-1,-1,1).
\]

Its complementary chirality is also used. The five-source fixture adds the
orbit center. This choice is made from moment isotropy before execution; it
is not a genesis-maximizing geometry.

Run 64 deterministic common-history arms:

- `L={9,17}`, `N={4,5}`, both global polarities, both tetrahedral
  chiralities, and center/+x translations;
- histories: permanently locked step and externally prescribed synchronous
  removal at tick 16;
- 128 ticks per arm.

Run 64 native unlocked arms:

- `L={9,17}`, `N={4,5}`, both global polarities, both chiralities;
- four fixed seeds beginning at `0x05880000`;
- production evaporation, 128 ticks.

The synchronous removal is a prescribed-source conformance history, not a
claim that production evaporation synchronizes.

## 5. Acceptance gates

- every frozen source hash and the preregistration hash match;
- the stencil identity and finite-group Parseval identity are independently
  verified;
- all four spectral evaluations agree between C++ long double and independent
  Python binary64 within `5e-15`;
- all common-history arms record zero genesis and respect
  `2 C_L sqrt(N)+1e-12`;
- all unlocked `N=4` arms record zero genesis and respect
  `2 C_L+4B_L^step+1e-12`;
- an unlocked `N=5` first genesis event, if any, is invalid unless every
  original source had already vanished before its causal update;
- velocity and remainder remain bit-exact zero;
- the history journal is state/RNG neutral;
- no production/default/toggle/scenario file is modified by the observer.

The unlocked `N=5` null result is recorded but does not close all possible
removal schedules. At least one unlocked arm must remove all original sources
so that the residual-tail branch is exercised.

## 6. Verdicts

- `COMMON_N_LE_5_ASYNC_N_LE_4_CLOSED`: both exact finite-volume inequalities
  and all conformance gates pass.
- `N5_RESIDUAL_TAIL_GENESIS_OBSERVED`: a registered unlocked `N=5` arm fires
  only after all original sources disappear; this is a measured residual-field
  event, not a self-sustaining particle.
- `N5_RESIDUAL_TAIL_UNRESOLVED`: no such event occurs in the registered arms;
  the untested-history branch remains open.
- `INVALID`: a frozen hash changes, a forbidden mechanism enters, an analytic
  bound is exceeded, or a tolerance is relaxed.

This gate cannot establish mobile matter, reciprocal recoil, a particle pole,
or a self-maintaining reaction front. It only prices the exact collective
source capacity of the frozen causal linear sector.
