# FTD-0582 — Native Active-Mode Backreaction

> **FTD-0585 scope amendment (2026-07-26):** the zero-kinematics invariant is
> reaction-free and assumes zero kinematic data on the manifested site and all
> void sites that may later manifest. Production evaporation preserves hidden
> `velocity`/`remainder`, and genesis can expose those old values without
> writing a new impulse. The 144 registered FTD-0582 arms disabled reactions,
> so their result is unchanged; the broader unsanitized-reaction reading is
> retracted.

**Status:** `[THEOREM — HASH-LOCKED PRODUCTION SOURCE GRAPH/ZERO-KINEMATICS INVARIANT]` +
`[MEASURED — ENERGETIC NATIVE FIELD MODES]` +
`[CLOSED NEGATIVE — FROZEN NATIVE ACTIVE FIELD AS RECIPROCAL MOVER]` +
`[OPEN — NEWLY DERIVED COMMON-ACTION EXTENSION]`  
**Date:** 2026-07-26  
**Verdict:**
`FROZEN_NATIVE_FIELD_IS_ONE_WAY_TO_MATTER_ACTIVE_TRAVERSAL_CLOSED`

## 1. Scope

FTD-0581 left one frozen-variable escape from the chord Peierls barrier: a
phase-carrying native `(J,W)` excitation might store at least `C_d/4` energy
and transfer it to manifested momentum. FTD-0582 tests whether the current
production tick contains the required transfer channel before attempting a
phase-locking calculation.

## 2. Exact source graph

The hash-locked tick has the following ordinary isolated-sector dataflow:

\[
 (s,u)\longrightarrow(J,W)
 \quad\text{through `phase_read` coupling},
\]

but, when `forces=false`, there is no reverse arrow

\[
 (J,W)\not\longrightarrow u.
\]

Specifically:

1. `phase_read` and `phase_write` update `flux` and `wave_vel`. Their source
   terms read manifested state and stored velocity, but neither file writes
   `velocity` or `remainder`.
2. `phase_forces` contains the field-dependent velocity write. The tick calls
   that phase only under `if (toggles.forces)`.
3. The registered isolated movement branch updates

   \[
   r_{n+1}=r_n+u_n\,dt
   \]

   and changes the anchor only if a component crosses `+/-1`.
4. All collision, boundary, reaction, damping, strong-energy, and external
   drive branches are absent from the registered domain.

Therefore, with `u_0=r_0=0`, induction gives

\[
 \boxed{u_n=0,\qquad r_n=0,\qquad x_n=x_0}
 \quad\text{for every tick},
\]

independently of the amplitude, phase, or evolution of `(J,W)`.

The other movement-phase velocity writes are transport of an already moving
state, causal projection, boundary/collision reflection, or reset. None
creates momentum from a native field in the registered isolated sector.

## 3. Dynamic campaign

The observer executed 144 CPU arms:

- `L=17,33`;
- both polarities;
- `<100>`, `<110>`, and `<111>` spatial modes;
- four exact `(J,W)` quadrature phases;
- initial native field energies `2`, `8`, and `32` times the largest FTD-0581
  barrier;
- 128 ticks per arm, for 18,432 field-evolution ticks total.

Wave propagation, state--flux coupling, and ordinary movement were enabled.
Every selected force, reaction, damping, projection, clock, drive, and
alternative integrator was disabled.

Every field hash changed. Thus the native fields were dynamically evolving,
not frozen controls. Nevertheless, in every arm:

\[
 \max|u|=0,\qquad \max|r|=0,
\]

\[
 \max|x-x_0|=0,\qquad N_{\rm move}=0.
\]

The maximum initial energy-normalization residual was `2.84e-13`; the smallest
actual energy ratio was `1.99999999999945`.

## 4. Sensitivity controls

The null result is not caused by a broken movement or field path:

- all 12 ballistic controls moved, with at least four legitimate history
  events and maximum speed drift `5.56e-17`;
- all six source-present/source-absent coupling pairs developed different
  field hashes;
- four selected emergent-force controls developed nonzero velocity
  (`>=0.04168`) with exact polarity mirroring.

The last control proves that the engine can turn a field gradient into motion
when a selected force is enabled. It does not make that force the reciprocal
partner of the FTD-0574--0580 action.

## 5. Consequence for the flux interpretation

In the frozen tick, the observed flux dressing can be built by, attach to, and
trail a carrier whose velocity was already supplied. It cannot be the cause
of that carrier's motion when selected forces are absent. In this precise
sense, the current dressing/wake visualization is a one-way response field,
not yet an autonomous pilot or reciprocal motor.

FTD-0581's finite internal-energy budget does not rescue the frozen model:
the energy can reside and evolve in `(J,W)`, but the production source graph
contains no channel by which it changes manifested momentum.

## 6. Program decision

The face-flux plan's failure rule now fires for the frozen production tick.
The FTD-0481 `common_action_face_dynamics` toggle and a reciprocal-motion
scenario are not licensed. Creating them would be a new selected dynamical
extension, not discovery of behavior already latent in production.

The mathematically open route is narrower but explicit: derive a new local
matter--field transaction from the common action, then evaluate it as a
default-off candidate under the original energy, Gauss, continuity,
invertibility, cubic-covariance, and Peierls gates. Such a branch must be
labelled selected until it is independently forced by the ontology.

FTD-0583 subsequently closes one proposed forcing route: the existing real
matched face/edge complex has only global continuously-valued plane-flux
cohomology, and every localized zero-harmonic field contracts to vacuum. A
localized protected carrier therefore requires nonlinear or explicitly new
compact/singular structure; it is not latent in the present noncompact face
variables.

No production state, source, force, movement phase, toggle, default, scenario,
renderer, particle claim, or Lorentz claim changes.

The locked preregistration SHA-256 is
`5A488BB1E9B9B25DA4363B0C8B27CDA9EA48B7FD6822124666179A3B5D948BEE`.
