# Mind's Eye SR conventions and portable API

**Status:** adopted Minkowski reference instrument, `c = 1`, signature `+---`.
These are reference-model and numerical implementation conventions. Passing
their tests does not establish recovery of SR from native FTD. The Observer
worker owns this experiment; its lattice adapter remains read-only.

## Force input and observer steering

`coordinateForcePerMass` is a finite three-vector in the simulation coordinate
frame, with components bounded by the session's existing `±1e12` data limit:

\[
\mathbf u=\gamma\mathbf v,\qquad
\frac{d\mathbf u}{dt}=\mathbf f
=\frac{1}{m}\frac{d\mathbf p}{dt}.
\]

Only point-clock and beacon markers accept nonzero prescribed force in SR.
Their graphics are markers; this input does not prescribe a Born-rigid
accelerated extended body. Playground continues to use its classical Rapier
impulses and force gun. Changing to Playground clears this SR-only control.

For an existing clock/beacon ID, the session command is:

```js
await workspace.command({
    type: 'update', id,
    patch: { coordinateForcePerMass: [0, 0.5, 0] },
});
```

The SR steering setting still has the settings/input key `acceleration` for
compatibility. Its label describes a **coordinate force / rest mass limit**:
it limits `|du/dt|` while approaching the requested velocity. The explicit
worker input `coordinateForcePerMass` overrides steering for scheduled force
replays. That input is rejected in Playground. The Playground steering setting
continues to mean coordinate acceleration.

Coordinate force is not a general comoving accelerometer reading. Its invariant
proper-acceleration magnitude obeys

\[
\alpha^2=\gamma^2\left(|\mathbf f|^2-(\mathbf v\cdot\mathbf f)^2\right).
\]

For `v = [0.8,0,0]`, `f = [0,0.5,0]`, this gives `alpha = 5/6`.
The assistant requests clarification for proper acceleration or unspecified
object acceleration; an explicit coordinate-force request uses the supported
control. A comoving acceleration mode would require a transported frame and is
outside this implementation.

## Shared evolution and optical intervals

The observer and accelerated markers use `integrateFourVelocity`, with method
identifier `sr-coordinate-force-kdk-v2`. For an uncapped step `h`, constant `f`:

\[
u_1=u_0+h f,\quad u_d=(u_0+u_1)/2,\quad
v_d=\frac{u_d}{\sqrt{1+|u_d|^2}},\quad
v_1=\frac{u_1}{\sqrt{1+|u_1|^2}}.
\]

The primitive returns endpoint `velocity`, `driftVelocity`,
`displacement = h * driftVelocity`, `properElapsed = h / sqrt(1+|u_d|^2)`,
and `capApplied`. The stored interval uses the drift velocity, starting pose
and center-clock offset. The endpoint placeholder uses the post-kick velocity.
Kicks take zero time. Each finite interval therefore satisfies exactly, up to
floating-point roundoff,

\[
\Delta x=v_d\Delta t,\quad
\Delta\tau=\Delta t\sqrt{1-|v_d|^2},\quad
(\Delta\tau)^2=(\Delta t)^2-|\Delta x|^2.
\]

This is a piecewise-inertial discrete worldline with second-order position and
proper-clock convergence toward smooth constant-coordinate-force motion.
It is not an exact finite-step smooth accelerated worldline. Segment intervals
are half-open `[start,end)`; an exact boundary belongs to the following
interval. Same-time empty intervals are removed, while identity, revisions and
previous emitted light are preserved. Prepared reversals, collisions and all
light-clock events split a step at the event before evolving its remainder.

The application ceiling remains **0.99c**. It is a control policy, not the
physical SR speed limit. If the requested endpoint momentum exceeds
`uMax = 0.99 / sqrt(1 - 0.99^2)`, project that endpoint onto the `uMax` ball.
Use the midpoint between the initial and projected endpoint for the drift and
clock. Convexity keeps both velocities within the ceiling. `capApplied` records
this intervention on the entity/observer and its new interval. Capped steps
are not compared against an uncapped analytic trajectory.

The production owner retains its 120 Hz fixed tick, elapsed-time accumulator,
FIFO worker, epoch guards and backlog pause. Identical controls scheduled at
identical ticks reproduce the same state regardless of presentation batching.
Different live keyboard sampling histories do not imply identical experiments.

## Schema 1 migration

The portable format remains `ftd-observer-world`, `schemaVersion: 1`; changes
are additive. New entities/exported data use only `coordinateForcePerMass`.
The deprecated `properAcceleration` alias retains its **historical coordinate
force** meaning. Import, commands and assistant schemas accept it; mirrored
authoring reflects either spelling as a polar vector. Equal old/new values are
accepted and normalized once. Conflicting values, invalid vectors and stale
requests are rejected without applying the edit.

New snapshots and segments carry `integratorVersion`. A document without this
field migrates to `sr-legacy-euler-drift-v1`. Migration copies retained positions,
velocities, clocks and interval bounds verbatim. It does not recompute an old
history, including any old numerical inconsistency. The first new evolution
closes the legacy continuation at its present time and opens v2 intervals.
Saving, loading, undo and scrub branching preserve per-interval provenance.
Continuation after migration is not promised to be bit-identical to the old
integrator. The method tag describes SR evolution, not Rapier physics.

## Optical and spectral conventions

`math.js` boosts into a moving coordinate frame; `optics.js` boosts from an
object's rest frame into world coordinates. They are inverses when the velocity
sign is reversed. Both helpers, the overlay projector and GLSL use the stable
coefficient `gamma^2/(gamma+1) = (gamma-1)/|v|^2`, avoiding tiny-speed
cancellation without discarding the first-order time term.

Optical intersections evaluate retained rest geometry on the observer's past
light cone. Center clocks follow the stored drift. Extended inertial geometry
uses Einstein synchronization across the rest shape; marker surface graphics do
not establish accelerated-body rigidity. Doppler uses the shared world photon
propagation direction, with ratio

\[
D=\frac{\gamma_o(1-v_o\cdot n)}{\gamma_e(1-v_e\cdot n)}
=\frac{d\tau_e}{d\tau_o},\qquad \lambda_o=\lambda_e/D.
\]

The derivative identity concerns one fixed emitter/material point within a
continuous interval, not a changing closest-surface point or a revision jump.
Reflected hits retain source identity and clock; the apparent/image geometry
is reported separately from the physical source position.

Materials and procedural sky/ground output are interpreted as weights of the
declared **610, 545, 455 nm** reference lines. The common Gaussian display
conversion `S(weights,Dcolor)` runs in both toggle states:

```text
Dcolor = opticalView && dopplerColorEnabled ? Dphysical : 1
linearColor = S(weights, Dcolor) * emission
if beamingEnabled: linearColor *= Dphysical^4
```

In simultaneous/Playground view the physical optical factor is one. At
`Dphysical = 1`, enabling/disabling Doppler color gives identical linear and
final pixels. Disabling color shift leaves requested beaming independent.
These line weights are not a reconstruction of an arbitrary RGB spectrum.
The Gaussian widths remain 35, 30, 25 nm; tone mapping, shading, vignette and
feedback are presentation choices.

The intensity convention is **integrated line specific intensity**. The
vacuum invariant `I_nu/nu^3` gives
`I_nu,obs(nu) = D^3 I_nu,em(nu/D)`; frequency integration gives `D^4`.
The shader applies that factor once, independently of the approximate Gaussian
response. See [Vincent et al., GYOTO, sections 2.2.1–2.2.2](https://arxiv.org/html/1109.4769v1).
Independent toggles are explanatory controls, not a complete radiation solver.

## Finite history and read-only diagnostics

The retained window is at most 60 coordinate-time units and 65,536 segments.
The session pauses before exhausting the history budget. `historyStart` is a
data boundary, not a physical horizon. Optical emission before it is rejected;
there is no current-pose substitution. Legitimate negative prehistory clocks
remain negative.

The existing HUD shows the available interval and observer clock-origin ID and
reason. Camera relocation intentionally resets the clock; resuming after a
paused relocation retains the existing additional origin transition. Preparation,
profile change and scrub branch have named origins. Physical acceleration does
not reset the accumulated clock.
Paused keyboard navigation retains the frozen clock until resume; wheel dolly
starts its declared origin immediately. These existing control paths remain
distinct from physical evolution.

The existing dev registry exposes a read-only current-ray export:

```js
const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
const evidence = workspace.exportObservationDiagnostics(0, 0);
console.log(JSON.stringify(evidence, null, 2));
```

The envelope includes `sessionId`, `epoch`, `observerWorldlineId`, `tick`,
`integratorVersion`, clock-origin reason/start, observer event/velocity/clock,
`historyStart`, available interval, `opticalMode` and `capApplied`. A nullable
`hit` contains source ID/revision, interval bounds/method, emission time,
physical `emissionPosition`, `apparentPosition`, source proper time, Doppler,
distance and reflection flag. A null hit never fabricates an emission event.
`before-history-start` is used only when a still-retained interval witnesses the
cutoff rejection; an ordinary empty ray is `no-hit`. Already-pruned evidence
cannot be reconstructed or classified from present geometry.

The export performs no worker tick or lattice write. Additional optional GPU
readback returns event/clock/position offsets against Float64 CPU bases; large
absolute time/clock origins are not sent through Float32 as accurate readings.
Display clock dials use their fractional phase. Diagnostics require a live
context and `EXT_color_buffer_float`; unsupported readback throws explicitly.

Implementation and test evidence: [SR repair ledger](AUDIT_MINDS_EYE_SR_REPAIR.md).
