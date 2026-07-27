# FTD-0434 — Exact Vacuum-Photon Scenario Diagnostic Pre-Registration v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE DIAGNOSTIC RUN]`  
**Date locked:** 2026-07-23  
**Identifier:** `FTD-0434`

## 1. Question and scope

Run the canonical C++ `s0-vacuum-photon` initializer at the dashboard default
`L=33` and determine what the seeded state actually does under the production
tick. This is a scenario audit. It does not assume that the rendered
streamlines are a photon and does not modify the initializer before verdict.

## 2. Frozen arms

Both arms call `dispatch_scenario(rb, "s0-vacuum-photon")` on a fresh CPU
`RenderBridge(33)`:

- `dashboard`: retain the production defaults established by the bridge and
  scenario, including wave propagation and Gauss projection, with genesis off;
- `wave_only`: after dispatch, disable all terms and re-enable only production
  wave propagation. This isolates the exact seeded state from projection.

Run ticks `0,...,24`. No RNG-dependent event is active because the state field
is empty and genesis/reactions are off.

## 3. Locked observables

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

## 4. Locked interpretation

- **TRANSLATING PACKET:** by tick 20 the unwrapped centroid displacement has
  magnitude at least 8 sites, its sign is positive, mean speed is within 20%
  of `C_WAVE`, and best-shift overlap is at least 0.8.
- **NONTRANSLATING/SPLITTING SEED:** by tick 20 centroid displacement has
  magnitude below 2 sites while width grows by at least 25%, or the best-shift
  displacement remains below 2 with overlap below 0.8.
- **PROJECTION-DOMINATED INITIALIZER:** dashboard tick 1 reduces normalized
  divergence by at least a factor 100 relative to tick 0 while changing the
  flux field energy by at least 1%.
- **INVALID:** dispatch, backend, finite-value, empty-state, tick-set, or output
  gates fail.

Multiple descriptive clauses may hold; they refer to different mechanisms.
The scenario may be called a demonstrated propagating photon only if the
TRANSLATING PACKET clause holds in both arms. Passing linear plane-wave tests
elsewhere cannot substitute for this exact-scenario gate.

No result establishes photon quantization, Maxwell gauge structure, helicity,
Lorentz recovery, or empirical light-speed normalization.
