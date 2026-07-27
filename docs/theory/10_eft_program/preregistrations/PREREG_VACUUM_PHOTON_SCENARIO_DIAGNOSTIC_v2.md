# FTD-0434 — Exact Vacuum-Photon Scenario Diagnostic Pre-Registration v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REVISION-2 RUN]`  
**Date locked:** 2026-07-23  
**Identifier:** `FTD-0434`  
**Revision:** 2

## 1. Correction scope

Revision 1 correctly locked and executed the `wave_only` arm, but its
`dashboard` arm was invalid: a fresh native `RenderBridge` does not reproduce
the browser loader's `SCALE0_TOGGLES` reset. In particular, native defaults
leave gravity, Lorentz force, dual substrate, and weak transmutation enabled
and use a periodic flux boundary. The browser profile disables those terms and
uses the non-reflective dispersal boundary.

The revision-1 output is preserved as
`engine/results/ftd_0434/invalid_v1_native_defaults_L33.csv`. No revision-1
dashboard number is reused. The valid revision-1 `wave_only` result remains an
independent frozen observation, but revision 2 reruns both arms from source.

Revision 2 changes only dashboard-profile setup and its validation contract.
All observables, tick sets, thresholds, and interpretation clauses below are
unchanged from revision 1.

## 2. Question and scope

Run the canonical C++ `s0-vacuum-photon` initializer at the dashboard default
`L=33` and determine what the seeded state actually does under the production
tick. This is a scenario audit. It does not assume that the rendered
streamlines are a photon and does not modify the initializer before verdict.

## 3. Frozen arms

Both arms call `dispatch_scenario(rb, "s0-vacuum-photon")` on a fresh CPU
`RenderBridge(33)`:

- `dashboard`: before dispatch, reproduce the browser Scale-0 reset profile
  exactly for all exposed physics toggles: wave propagation, coupling,
  damping, genesis, Gauss projection, forces, movement, Poisson Coulomb, and
  selective damping ON; gravity, Lorentz force, Larmor radiation, dual
  substrate, confinement, color, strong, exchange, weak transmutation, and de
  Broglie clock OFF. Use `FluxBoundaryMode::Dispersal`. Dispatch then sets
  genesis OFF for this scenario.
- `wave_only`: after dispatch, disable all terms and re-enable only production
  wave propagation. Retain the native periodic boundary. This isolates the
  exact seeded state from projection and dashboard boundary handling.

Run ticks `0,...,24`. No RNG-dependent event is active because the state field
is empty and genesis/reactions are off.

## 4. Locked observables

At every tick record:

- occupancy and signed state;
- flux and wave-velocity energy by Cartesian component;
- total quadratic field-plus-wave energy;
- circular energy centroid along `x`, unwrapped from tick zero;
- energy width around that centroid;
- normalized flux divergence
  `sqrt(sum(div J)^2 / max(1e-30,sum|J|^2))`;
- normalized `J dot curl J` proxy, explicitly not a topological invariant;
- normalized overlap of the current `x`-slice energy profile with every
  circular shift of the initial profile, recording the best shift and overlap.

At tick zero also evaluate the necessary right-moving-packet relation for the
declared `J_z` polarization,

\[
 W_z\simeq-C_{\rm WAVE}D_xJ_z,
\]

using the centered derivative. Record the normalized residual and the fraction
of total wave-velocity energy in each Cartesian component.

## 5. Locked interpretation

- **TRANSLATING PACKET:** by tick 20 the unwrapped centroid displacement has
  magnitude at least 8 sites, its sign is positive, mean speed is within 20%
  of `C_WAVE`, and best-shift overlap is at least 0.8.
- **NONTRANSLATING/SPLITTING SEED:** by tick 20 centroid displacement has
  magnitude below 2 sites while width grows by at least 25%, or the best-shift
  displacement remains below 2 with overlap below 0.8.
- **PROJECTION-DOMINATED INITIALIZER:** dashboard tick 1 reduces normalized
  divergence by at least a factor 100 relative to tick 0 while changing the
  flux field energy by at least 1%.
- **INVALID:** dispatch, backend, finite-value, empty-state, tick-set, output,
  or exact toggle-profile gates fail.

Multiple descriptive clauses may hold; they refer to different mechanisms.
The scenario may be called a demonstrated propagating photon only if the
TRANSLATING PACKET clause holds in both arms. Passing linear plane-wave tests
elsewhere cannot substitute for this exact-scenario gate.

No result establishes photon quantization, Maxwell gauge structure, helicity,
Lorentz recovery, or empirical light-speed normalization.
