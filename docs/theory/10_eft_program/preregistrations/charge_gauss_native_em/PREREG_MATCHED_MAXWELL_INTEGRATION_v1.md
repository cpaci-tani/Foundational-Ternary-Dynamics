# FTD-0428 — Matched Maxwell Integration v1

**Status:** [PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION RUN]
**Date:** 2026-07-23
**Scope:** default-off selected `RenderBridge` branch; no change to default
production dynamics

## 1. Question

Can FTD-0427's projection-free current law be integrated into the live engine
as one local longitudinal/transverse field system that:

1. begins from the minimum-energy neutral Gauss dressing;
2. remains static for stationary sources;
3. follows actual production movement without projection;
4. propagates a nontrivial transverse disturbance locally with a stable
   source-free energy invariant; and
5. exposes its centered electric field through production `Voxel::flux`?

## 2. Status lock

The branch is a **[SELECTED ENGINE EXTENSION]**. Oriented face-electric and
edge-magnetic placement, the staggered Maxwell update, and the identification
of ternary sign with its source in the isolated movement sector are adopted
mechanisms. A passing run cannot be called native `U(1)`, emergent
electromagnetism, a photon derivation, or full-production charge conservation.
FTD-0421 remains controlling for the complete reaction set.

## 3. Frozen equations

Use the FTD-0427 periodic backward-difference divergence `D`, edge-to-face curl
`C`, and its exact transpose `C^T`. The fields are staggered in time:

\[
B^{n+1/2}=B^{n-1/2}-c\,\Delta t\,C^T E^n,
\]

\[
E^{n+1}=E^n+c\,\Delta t\,C B^{n+1/2}-K^{n+1/2},
\]

with `c=C_SPEED=1/sqrt(3)` and `Delta t=1`. The integrated movement current
`K` comes from the existing one-tick finite-volume history extractor. Because
`D C=0` and `Delta s+D K=0`, Gauss propagates without projection.

For `K=0`, the frozen modified energy is

\[
H_h={1\over2}\left(\|E\|^2+\|B\|^2
-c\Delta t\langle B,C^T E\rangle\right).
\]

The full-band stability bound is `c^2 max|d(q)|^2=(1/3)*12=4`.

## 4. Minimum-energy initialization

For a neutral integer source `rho`, solve once

\[
(D D^T)\phi=\rho,
\qquad E=D^T\phi,
\qquad B=0,
\]

by zero-mean conjugate gradients with tolerance `1e-12` and at most `12L`
iterations. This is the unique minimum-norm face field modulo the scalar zero
mode. No Poisson solve or projection is permitted after initialization.

The comparison field is FTD-0427's deterministic shortest flux string with
the same endpoints. The minimum-energy field must have strictly lower
quadratic energy.

## 5. Engine integration lock

- Add one table-registered `matched_gauss_dynamics` boolean, default false,
  CPU-scoped, and excluded from bulk enable/disable profiles.
- When enabled, validation requires the isolated single-substrate periodic
  sector. All legacy field writers, reactions, forces, damping, projectors,
  and gauge-link relaxations are off; `movement` is optional.
- The branch is explicitly initialized from the current ternary state before
  its first tick.
- Immediately before production movement, capture signed state. Immediately
  after movement, extract `K`, advance the staggered fields once, and mirror
  the centered face field
  `J_i=(E_i+E_{i-e_i})/2` into `Voxel::flux`. `Voxel::wave_vel` is zeroed in
  this isolated branch.
- A GPU-backed bridge must explicitly fall back to CPU before executing the
  selected branch.
- Default-off golden hashes and RNG state must remain unchanged.

## 6. Frozen campaigns

Run Windows MSVC and WSL2 GCC at `L=32,64`.

### A. Dressing/static gate

- Neutral dipole at `(L/4,L/4,L/4)` and
  `(3L/4-1,3L/4-1,3L/4-1)`.
- Initialize once, then run 32 ticks with movement off.
- Measure Gauss, `C^T E`, surface flux at radii `2,3,4,5`, field energy,
  voxel/face synchronization, and solver convergence.

### B. Production movement gate

- Both orientations `q=+1,-1`; directions `+x,+y,+z`.
- One mobile `q`, one locked `-q`; speed `0.99*C_SPEED`.
- 12 moving ticks, then lock the source for 8 stationary ticks.
- `gauss_projection=false` for the entire run.

### C. Transverse wave gate

- Empty source field initialized exactly to zero.
- Inject the curl of one compact edge impulse of amplitude `1e-3` at the
  center; this is divergence-free by construction.
- Evolve 32 ticks; measure Gauss, the modified energy, finite support, and
  nontrivial outward propagation.

## 7. Acceptance gates

Across both compilers and both volumes:

1. minimum-energy solve converges within `12L`, with residual `<=1e-10`;
2. initialized `max|D E-rho|<=1e-10` and `max|C^T E|<=1e-10`;
3. minimum-energy field energy is strictly below the flux-string energy;
4. 32 stationary dressing ticks keep Gauss, surface error, surface telescope,
   and voxel/face sync `<=1e-9`, with relative energy drift `<=1e-9`;
5. every movement arm contains at least five actual moves, no reaction term,
   `max|D E-s|<=1e-9`, surface error/telescope `<=1e-9`, and eight final
   stationary ticks with zero current;
6. source-free transverse evolution keeps Gauss `<=1e-10`, modified-energy
   relative drift `<=1e-8`, and all values finite;
7. transverse support never appears outside Chebyshev radius
   `initial_radius+tick`; by tick 12 its radius has increased by at least 3;
8. MSVC/GCC scalar metrics agree within `1e-9` and event counts exactly;
9. existing golden tests remain bit-exact.

## 8. Locked outcomes

| outcome | interpretation |
|---|---|
| A: all gates pass | **[THEOREM — selected finite-lattice complex/minimum] + [MEASURED — integrated engine compatibility] + [SELECTED ENGINE EXTENSION]** |
| B: dressing/movement pass, wave gate fails | projection-free electrostatic transport integrates, but the selected transverse dynamics is closed negative |
| C: algebra passes, engine coupling fails | FTD-0427 remains a sidecar-only mechanism; live integration is closed negative |
| D: default/golden or observer contract fails | invalid implementation; no physical inference |

## 9. Explicit non-claims

A passing campaign does not establish a `1/r^2` force, matter feedback,
Lorentz recovery, a common matter/light cone, gauge redundancy, Ward
identities, quantization, radiative stability, or empirical charge
conservation. The one-time minimum-energy solve is selected initialization;
only subsequent propagation is local. Reactions remain excluded.
