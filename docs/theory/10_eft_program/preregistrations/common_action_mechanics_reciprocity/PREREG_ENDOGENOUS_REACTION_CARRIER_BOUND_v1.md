# Preregistration — Endogenous Reaction-Carrier Bound (FTD-0586)

**Status:** `[LOCKED — RUN OF RECORD NOT YET EXECUTED]`  
**Date:** 2026-07-26  
**Production effect:** none.

## 1. Question

FTD-0474 observed finite-support reaction fronts after injecting flux at
`12`, `20`, or `40` times `K_GENESIS`. FTD-0585 proved that support motion can
be reaction-source motion rather than transported matter and exposed stale
void kinematics. This gate asks a narrower prior question:

> Can a sanitized finite ternary seed generate the first new manifested site
> through its own causal state-to-flux source, without injected flux, a Gauss
> projection, a force branch, or inherited void kinematics?

The answer is derived first for clusters with at most three initial sites on
four registered periodic volumes. Live engine arms then test production
conformance. No amplitude or cluster-size scan is permitted.

## 2. Frozen source contract

| source | SHA-256 |
|---|---|
| `engine/src/render_bridge.cpp` | `A822E0FAFAF71FE5458B2A7450868A8414B1C8564089BF6C6484FC34B7559359` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/include/ftd/ontic/gauge_couplings.h` | `BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3` |
| `engine/include/ftd/ontic/particle_masses.h` | `E43E01D5F1F870EE019754BEA7E932529346C9B8EB704B40215CA559FC5A4F57` |

The isolated arm has only `wave_propagation`, `coupling`, and `genesis`
enabled, with `dual_substrate=false`, periodic field boundaries, unit tick,
and CPU execution. Damping, both Gauss mechanisms, forces, movement, pair
production, annihilation, weak transmutation, clocks, and stochastic baths
are off. Every initial and void voxel has exactly zero `velocity` and
`remainder`.

## 3. Locked analytic bound

For the production 18-point stencil, write

\[
 M(k)=4-\frac23\sum_a\cos k_a
      -\frac23\sum_{a<b}\cos k_a\cos k_b,
 \qquad a(k)=C_{\rm WAVE}^2M(k).
\]

The field sourced by one stationary polarity satisfies

\[
 W_{n+1}=W_n-aJ_n+f,
 \qquad J_{n+1}=J_n+W_{n+1},
 \qquad f_k=-iG_C\sin(k)s_k.
\]

For `0<a<4`, `cos(theta)=1-a/2`. The exact zero-field step response has
modal multiplier

\[
 r_n=1-\cos(n\theta)+\tan(\theta/2)\sin(n\theta),
 \]

and therefore

\[
 |r_n|\le D(k):=1+\sec(\theta/2)
              =1+\frac1{\sqrt{1-a(k)/4}}.
\]

If the source evaporates once at an arbitrary tick, its rectangular-pulse
response is a difference of two step responses and is bounded by `2D(k)`.
The registered pointwise one-source pulse bound is

\[
 B_L=\frac{2G_C}{C_{\rm WAVE}^2L^3}
 \sum_{k\ne0}D(k)
 \frac{\sqrt{\sum_a\sin^2k_a}}{M(k)}.
\]

By linearity and the triangle inequality, any arrangement and polarity choice
of `N` initial sources, each either permanent or removed once, obeys
`|J(x,n)| <= N B_L` before the first genesis event. The volumes are locked to
`L={9,17,33,65}`. The gate passes only if

\[
 3\max_L B_L < K_{\rm GENESIS}.
\]

This is a first-event induction: if the bound passes, no first endogenous
genesis event exists for `N<=3`. The smallest source count not excluded by
this bound must be reported; it is not a prediction that such a cluster works.

## 4. Registered live arms

Use `L={9,17}`, both polarities, all six axial orientations, initial counts
`N={1,3}`, and two source histories:

1. **constant:** initial sites are locked, so the source remains present;
2. **pulse:** initial sites are unlocked and may evaporate under the exact
   production rule.

Each of the 96 arms runs for 128 ticks. Initial flux and wave velocity are
zero. A three-site seed is an axial line through the center. A one-site seed
is translated one cell in the registered direction. Every tick records the
maximum flux, support, genesis/evaporation events, and all voxel kinematics.

Required gates:

- zero accepted genesis events in every endogenous arm;
- manifested support is always a subset of the initial support;
- observed `|J|` never exceeds the registered `N B_L` bound by more than
  `1e-12`;
- all velocity and remainder components remain exactly zero;
- the history journal changes neither state nor RNG state;
- at least one pulse source evaporates, so the off-step case is exercised.

Four external-drive sensitivity controls use a single local field magnitude
`100`, genesis alone, and distinct fixed seeds. Every control must record at
least one accepted genesis event. This is not an endogenous arm and supplies
no evidence for self-maintenance.

## 5. Verdicts and scope

- `ENDOGENOUS_N_LE_3_AUTOCATALYSIS_CLOSED`: the analytic and live gates pass.
- `BOUND_INCONCLUSIVE_AT_N_GE_4`: the count inferred from the bound is four;
  no four-site run is authorized by this preregistration.
- `EXTERNALLY_IGNITED_REACTION_FRONT_REMAINS`: the FTD-0474 result survives,
  but it is not reclassified as self-sustaining or particle-like.
- `INVALID`: a frozen hash changes, a tolerance is relaxed, or a forbidden
  mechanism enters an endogenous arm.

This result is scoped to the causal single-substrate prescribed-source sector.
It does not cover the legacy Gauss projector, dual-substrate genesis, damping,
selected forces, externally injected packets, or clusters of four or more.
It cannot establish that four sources suffice, that any reaction front carries
identity, or that a reaction front is a particle.
