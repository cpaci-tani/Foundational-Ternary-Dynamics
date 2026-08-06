# FTD-0768 — Long-transport dynamic-response clearing v1

**Status:** `[CONSUMED — FINAL OUTCOME LONG_TRANSPORT_EXECUTION_INVALID]`

**Result:** [`AUDIT_LONG_TRANSPORT_DYNAMIC_RESPONSE_v1.md`](../../07_assessment/AUDIT_LONG_TRANSPORT_DYNAMIC_RESPONSE_v1.md) — continuous reverse recovery
`3.8786822642578e-9 > 1e-10`; certificate `2220/2221`.

**Scope:** default-off research branch; unchanged FTD-0761/0763 dynamics

**Parent correction:** FTD-0767

## 1. Question

Can the registered mobile relational core remain a state-only member long
enough to clear a fixed laboratory witness slab from its selected near-field
window, and what field response remains there after passage?

This protocol qualifies long mobile identity and the first spatial-clearing
event.  It does not call a single cleared checkpoint a wake, particle pole,
photon, aura, pilot wave, or radiation field.

## 2. Frozen parent and run

- Use the unchanged FTD-0761/0763 connected opposite-polarity parent.
- Use the unchanged common-action options, compact selected interaction,
  face/link field, normalization, implicit solve, and state-only observer.
- Registered volume: `L=321`, periodic computational quotient.
- Formation: 160 ticks; unboosted preparation age: 128 further ticks.
- Branch the identical aged state into:
  - matched rest control `q=0`;
  - moving discovery arm `q=+0.030 d_hat`, `d_hat=(0,0,1)`.
- Evolve both arms for exactly 768 ticks.
- Record at `tau={0,64,128,...,768}`.  No adaptive stopping, extension, arm
  replacement, or alternate boost is allowed.
- Run a complete 768-step state-only reverse history for the moving arm from
  its final state.  The rest arm retains the registered one-step inverses at
  every checkpoint.

The maximum source-free causal reach from the start of formation through the
end of discovery is

```text
(160+128+768) C_SPEED dt
 = 1056/(4 sqrt(3))
 = 152.42047... sites.                                    (1)
```

Adding the initial radius-four compact support remains below the face distance
to the `L=321` quotient boundary.  Any contrary boundary-contact diagnostic
invalidates the run.

## 3. Fixed laboratory clearing witness

Let `c_0` be the moving core center at `tau=0`.  Freeze a one-site-thick central
laboratory slab

```text
Omega_0 = {x: |(x-c_0) dot d_hat| <= 1/2,
              ||(x-c_0)_perp||_infinity <= 4}.            (2)
```

The selected near-field radius is `R_N=8`.  The slab is certified outside the
current near window only when

```text
d(tau) = [c(tau)-c_0] dot d_hat > R_N + 1/2 = 8.5.         (3)
```

Use the registered integer clearing label

```text
CLEARING_REACHED iff d(tau)>=9 at a registered checkpoint. (4)
```

If no checkpoint satisfies (4), return `CORE_CLEARING_NOT_REACHED`.  This is a
valid physical result, not an execution failure, and no wake label is evaluated.

## 4. Paired field-response observer

Evolve rest and moving states from exactly the same aged parent.  At identical
lab faces/edges define direct differences

```text
delta E = E_q-E_0,
delta B = B_q-B_0.                                        (5)
```

Repeat (5) for the FTD-0763 actual-minus-selected-bound residual fields.  For
each actual and residual channel record in `Omega_0`, the current bound window,
the current near window, and the radius-48 causal window:

```text
Delta U     = U(F_q)-U(F_0),
U_delta     = (1/2)||delta F||_H^2,
U_cross     = <F_0,delta F>_H,
r_energy    = Delta U-U_cross-U_delta.                    (6)
```

Require `|r_energy|<=1e-12 max(1,U(F_q),U(F_0))`.  Record signed longitudinal
first moments of `Delta U`, `U_delta`, and `U_cross` separately.  No complete
field download is allowed; the paired reductions remain device resident and
return scalar summaries only.

The actual regional energy difference in (6) is the energetic observable.
`U_delta` is a nonnegative morphology norm.  Neither may be silently
substituted for the other.

## 5. Energy-flow record

At each checkpoint record:

- moving and rest matter kinetic, binding, and complete field energies;
- their differences from `tau=0`;
- common-action matter work and field work;
- oriented energy flux through the fixed slab boundary and through the moving
  radius-eight boundary;
- the accumulated regional balance residual;
- matter momentum plus the existing local and spline pseudomomentum candidates,
  without assigning their defect to an invented substrate momentum.

For each interval `[tau_i,tau_{i+1}]`, the lab-slab balance must close as

```text
Delta U_Omega - W_source + Phi_boundary = 0              (7)
```

within `1e-10` relative to the maximum recorded energy scale.  The sign
convention and discrete boundary quadrature must be unit-tested before the
registered artifact can be written.

## 6. Qualification gates

Before `engine/results/ftd_0768` may be written, paired CPU/CUDA fixtures at
`L={17,33}` must establish:

- zero response for identical state pairs;
- exact sign and magnitude on an independently constructed one-face electric
  difference and one-edge magnetic difference;
- (6) within `1e-12` for affine, quadratic, and mixed fixtures;
- integer translation, charge conjugation, and proper cubic covariance within
  `1e-11`;
- a complete reflected-parent signed pair agrees after reflection within
  `1e-11`;
- scalar-only CUDA telemetry with no complete field download;
- all FTD-0763, FTD-0764, FTD-0766, and FTD-0767 certificates remain green.

## 7. Execution validity

Every tick must retain:

- valid state-only matter membership and derived interaction graph;
- common-action, continuity, Gauss, work, and complete-energy residuals
  `<=1e-12`;
- causal-speed excess `<=1e-12`;
- minimum root singular value `>=1e-3` and condition number `<=1e4` at
  checkpoints;
- one-step state-only inverse residual `<=1e-12` at checkpoints;
- no periodic-boundary contact;
- rest-core displacement `<=1e-12`.

The moving arm's full forward/reverse recovery must restore discrete state
exactly and continuous matter/field state within `1e-10`.  Failure of any item
returns `LONG_TRANSPORT_EXECUTION_INVALID`.

## 8. Locked outcomes

If execution is valid:

1. `CORE_CLEARING_NOT_REACHED` if (4) never fires.
2. `CORE_CLEARING_REACHED_RESPONSE_UNRESOLVED` if (4) fires but fewer than two
   registered post-clearing checkpoints exist.
3. `CLEARED_LOCAL_RESPONSE_DECAYS` if at least two post-clearing checkpoints
   exist and both `|Delta U_Omega|` and `U_delta,Omega` decrease monotonically
   toward the observer floor.
4. `CLEARED_LOCAL_RESPONSE_PERSISTS` if at least two post-clearing checkpoints
   exist and either response remains above
   `1e-6` of the initial moving-minus-rest complete energy at every such
   checkpoint without monotonic decay below that scale.
5. `CLEARED_LOCAL_RESPONSE_MIXED` for all other valid cleared results.

These are response classifications only.  A spatial-wake candidate requires a
fresh persistence campaign after the clearing time and is not an FTD-0768
outcome.

## 9. Matter consequences

- Failure of core identity before clearing closes long mobile matter for this
  selected family at the registered boost and horizon.
- Valid clearing with a decaying slab response favors a mobile kernel moving
  through an environment without a durable trail.
- Valid clearing with persistent, energy-balanced response licenses a later
  wake-persistence test but does not establish a wake here.
- A nonclosing regional energy ledger exposes an incomplete observer or a
  missing boundary term; it does not license post-hoc energy assignment.
- No outcome changes production defaults, primitives, scenarios, constants,
  mass formulas, or Lorentz claims.
