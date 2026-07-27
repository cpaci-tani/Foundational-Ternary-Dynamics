# FTD-0585 — Native Motion / Reaction-Front Trichotomy

**Status:** `[THEOREM — EXACT FINITE-VOLUME CURRENT/SOURCE MOMENT IDENTITY]` +
`[THEOREM — HASH-LOCKED REACTION-FREE ZERO-KINEMATICS INVARIANT]` +
`[ENGINE FACT — EVAPORATION/GENESIS REUSES HIDDEN VOID KINEMATICS]` +
`[BOUNDARY — REACTION FRONT IS NOT A CONSERVED PARTICLE WORLDLINE]`  
**Date:** 2026-07-26  
**Verdict:**
`TRANSPORT_REACTION_FRONT_AND_STALE_MEMORY_DISTINGUISHED_RECIPROCAL_NATIVE_PARTICLE_MOTION_STILL_CLOSED`

## 1. Scope

This theorem answers the remaining same-variable active-core escape left open
by FTD-0584. It does not ask whether a manifested pattern can move. It asks
what mathematical mechanism moves it:

1. transported polarity with an oriented face current;
2. destruction/recreation represented by a reaction source;
3. initialized or selected kinematics written outside the reciprocal native
   field sector.

Those mechanisms can produce the same two endpoint snapshots. They cannot be
identified from snapshots alone.

## 2. Exact transport/source identity `[THEOREM]`

The native finite-volume convention is

\[
 \Delta\rho+\operatorname{div}I=S.
\]

On a non-wrapping support patch, let

\[
 Q=\sum_x\rho_x,
 \qquad
 M=\sum_x x\rho_x.
\]

Summing continuity gives

\[
 \boxed{\Delta Q=\sum_x S_x}.
\]

Multiplying by `x`, summing, and shifting the incoming-face index gives

\[
 \sum_x x\operatorname{div}I=-\sum_f I_f.
\]

Therefore

\[
 \boxed{\Delta M=\sum_f I_f+\sum_x xS_x.}
\]

Now take one polarity `q=+-1` at site `a` and the same polarity at
`a+e` one tick later.

**Transport history**

\[
 I_{a,a+e}=q,\qquad S=0.
\]

**Reaction-supported history**

\[
 I=0,\qquad S_a=-q,\qquad S_{a+e}=q.
\]

Both have `Delta Q=0` and `Delta M=q e`. The first has a nonzero local
current; the second has a globally balanced but locally nonzero reaction
source. Thus global signed-polarity conservation and moving support do not
prove local transport or a particle worldline.

The native observer checks both signs, all six face directions, and three
translated copies. All 36 transport and 36 reaction histories have identical
endpoint snapshots. All 72 moment identities and all continuity/global-balance
identities close exactly in binary64.

## 3. Reaction-free matter at rest stays at rest `[THEOREM]`

In the registered isolated sector:

- reactions, legacy/selected forces, strong projection, and external drives are
  disabled;
- movement remains enabled;
- native wave propagation and state-to-field coupling remain enabled;
- the manifested voxel starts with `velocity=0` and `remainder=0`.

The frozen `phase_read` and `phase_write` evolve field variables but do not
write matter velocity or remainder. The movement recurrence is

\[
 r_{n+1}=r_n+\Delta t\,u_n.
\]

With no kinematic writer and `u_0=r_0=0`, induction gives

\[
 u_n=r_n=0
\]

for every tick, so no hop occurs. Twelve live arms covering both polarities and
all face directions ran for 384 aggregate ticks with exactly zero velocity,
remainder, displacement, and history events while their fields evolved.

The sensitivity control initializes speed `C_SPEED/2`. All twelve controls
hop at least four times. The negative result is therefore not a disabled
movement path.

This theorem is intentionally narrower than “nothing dynamic can emerge.” A
reaction-supported traveling structure remains possible. What is closed is an
isolated reaction-free field excitation accelerating matter from rest through
the frozen native source coupling.

## 4. A reaction front can move without transporting a particle `[BOUNDARY]`

Genesis, evaporation, pair production, annihilation, and weak transmutation
can alter the ternary support. A sequence of local death and rebirth can move
the location of a visible excitation just as a flame front moves while its
constituent molecules do not follow one persistent worldline.

That is a legitimate discrete dynamical hypothesis. Its required ledger is

\[
 S\ne0.
\]

It is therefore an open-system or reaction-medium object until the model also
derives:

- a reservoir/environment paying the energy and information balance;
- a persistence criterion identifying one excitation across events;
- an event-resolved current distinct from the reaction source;
- a stable propagation law rather than an imposed or noise-triggered sequence.

FTD-0567--0573 already prove that the production genesis/evaporation law is
noncanonical on frozen `(J,W)` and that a reversible energy-preserving lift
requires additional environment variables. The flame-like route is not
excluded, but it is not the reciprocal particle route specified by FTD-0479
and FTD-0481.

## 5. Hidden void kinematics are reused `[ENGINE FACT — DEFECT]`

The source audit exposes a stronger implementation problem.

Production evaporation executes

```text
s -> 0
particle_id -> -1
spin -> 0
color -> 0
```

but does not clear `velocity` or `remainder`. Production `manifest_at()` then
sets state, ID sentinel, spin, and color without initializing `velocity` or
`remainder`. Pair production has the same omission.

Consequently a void voxel can retain hidden kinematics from an evaporated
occupant. Later genesis can expose those old values as the kinematics of a
newly manifested state even though neither the field action nor a force
generated them.

This is not merely a source-reading inference. In twelve live CPU arms:

1. a moving manifested site evaporated in at most 16 ticks;
2. its velocity and remainder remained bit-exact while `s=0`;
3. deterministic supercritical genesis remanifested both polarity signs;
4. the new state inherited the previous velocity and remainder with zero
   residual;
5. forces and movement were disabled throughout the remanifestation step.

Thus a moving reaction-created state is not evidence of field-to-matter
backreaction unless its pre-event void kinematics are separately zeroed or
accounted for. This theorem does not change production behavior; it records the
defect because the baseline is frozen.

## 6. The complete frozen motion trichotomy

| mechanism | what can move | exact diagnostic | status |
|---|---|---|---|
| reaction-free native field sector | fields, not rest matter | `I=0`, `S=0`, zero kinematics | reciprocal motion closed |
| initialized ballistic motion | manifested state | movement event and face current | kinematic input, not emergence |
| reaction-supported front | manifested support | `S!=0`, possibly `I=0` | dynamical pattern open; particle claim absent |
| selected force/strong projection | matter velocity | optional force/projection writer | selected phenomenology |
| stale void memory | newly manifested kinematics | pre-genesis void `velocity,remainder != 0` | implementation confound |

The current production engine therefore contains dynamic behavior, but no
single observation that “the bright core moved” identifies which row produced
it.

## 7. Consequence for the face-flux plan

FTD-0478 remains exact for a specified subcell trajectory: it converts a
worldline segment into an oriented face current satisfying continuity. It does
not construct the worldline.

FTD-0479/0480 closed negative because the frozen common-action gather did not
produce a unique, energy-consistent reciprocal transaction. FTD-0481--0483
therefore remain unexecuted. The reaction-front route does not reopen those
gates: replacing a missing worldline by `S_reaction` would change the object
from conserved mobile matter into a reactive excitation.

The next admissible native research target is consequently narrower:

> Find a stable, propagating reaction pattern in histories whose void
> kinematics are explicitly sanitized, then measure its source, reservoir
> payment, persistence, and perturbation spectrum without calling it a
> particle.

Only if its long-distance source term flows to zero while a conserved event
current and positive pole survive can it re-enter the particle/common-cone
program.

## 8. Verification

- preregistration SHA-256:
  `972F221AAE2BA9CBE1C95C9E71CA9789D3082A1DD5695B56F6996ACD29ABFC1B`;
- native observer: 12 rest arms, 384 ticks, 12 ballistic controls, 36 transport
  fixtures, 36 reaction fixtures, 72 moment identities, and 12 live stale
  kinematic cycles;
- all registered algebraic and kinematic residuals: exactly zero;
- independent proof: 58/58 PASS;
- production defaults, toggles, scenarios, and tick rules: unchanged.

No Lorentz, unitarity, particle, photon, charge-emergence, or scenario claim is
licensed by this theorem.
