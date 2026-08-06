# FTD-0734 — Energy-adapted mixed capture corners v1

**Status:** `[PRE-REGISTRATION — LOCKED / NOT YET RUN]`  
**Date:** 2026-07-29  
**Parents:** FTD-0732 and FTD-0733  
**Scope:** observer-only selected dynamics; no production rule, action,
coefficient, state type, toggle, scenario, or physical-constant target changes

## Question

Do the FTD-0731 tick-128 captured states survive finite perturbations in which
relative position, all three relative-momentum axes, and dynamic field
amplitude are changed simultaneously while every initial state remains
strictly inside the exact FTD-0733 negative-energy domain?

This is a dynamical mixed-corner test. It is not a search for a passing
percentage and not an open-basin theorem.

## Frozen parents

- FTD-0732 protocol `1A93899A…0903` and CSV `15926F9E…E2AD`;
- FTD-0733 protocol `E4C639DC…E26DB` and certificate
  `0574272D…812C` (`654/654 PASS`);
- the same tick-128 reconstruction, `dt=1/4`, `p=0.0120`, selected compact
  well, matched field normalization, sparse current, local residual solve,
  root tolerance `2e-14`, and inverse used by FTD-0732.

## Momentum corners

For each `L=33` center and its registered orthonormal frame `(u,t_1,t_2)`, use
the eight sign triples `(sigma_r,sigma_1,sigma_2) in {-1,+1}^3`. Add the
equal-and-opposite constituent impulse

```text
delta p = (0.0006/sqrt(3))
          (sigma_r u + sigma_1 t_1 + sigma_2 t_2).
```

Every corner has total impulse norm exactly `0.0006`, equal to the FTD-0732
single-axis impulse norm. The direction is new; the magnitude is not enlarged.
No corner is selected from output.

## Energy-adapted radial points

For each momentum corner, compute its instantaneous kinetic energy `K` before
changing position. Apply the exact FTD-0733 theorem to isolate
`d_-(K),d_+(K)`. With `d_0=r_parent^2`, require

```text
d_-(K) < d_0 < d_+(K).
```

Define

```text
m_K = min(d_0-d_-(K), d_+(K)-d_0)
```

and use exactly two radial points:

```text
d_in  = d_0 - m_K/2,
d_out = d_0 + m_K/2.
```

The factor `1/2` is fixed as the midpoint of the nearest certified margin. It
is not inferred from a desired survival result. Reconstruct the static dress
at each new matter position and retain the parent's dynamic residual before
the field-amplitude operation.

Every radial point must satisfy graph membership, causal speed, Gauss residual
`<=1e-12`, exact total constituent momentum preservation, and pair energy
`<-1e-6`. Any invalid initialization returns the unresolved verdict; it is not
replaced.

## Dynamic-field corners

For every momentum/radial corner, multiply the divergence-free dynamic
electric and magnetic residual by exactly `0.95` and `1.05`, leaving the
fresh static dress unchanged. This gives

```text
8 momentum corners x 2 radial points x 2 field amplitudes = 32
```

mixed histories per direction and polarity. Add one unperturbed center control.
Across three directions and two polarity orders, Stage A contains `198`
`L=33` histories.

## Held-out volume selectors

After Stage A, choose exactly three names per direction/polarity for `L=65`:

1. the unperturbed center;
2. the mixed corner with the smallest continuation energy margin
   `min_t(-E_pair(t)/D)`;
3. the distinct mixed corner with the smallest graph margin
   `min_t(sqrt(3/2)-r(t))`, using lexicographic sign/radial/field name as the
   final tie-break.

Reconstruct each selected corner from its own `L=65` parent and its own exact
`K`-dependent interval. Stage B contains `18` histories. Total campaign size:
`216` histories.

## Evolution and gates

Run every initialized state 256 steps forward from parent tick 128 to tick 384
and 256 state-only inverse steps. Persist all 257 scalar states.

`survives` requires at every stored state:

- graph membership and `E_pair<-1e-6`;
- nonnegative matched field energy;
- no graph transition;
- common-action residual `<=1e-10`;
- recoil defect `<=1e-9`;
- pair-plus-field energy defect `<=1e-8`;
- final inverse recovery `<=1e-8`.

Require the center controls to reproduce FTD-0732, matched polarities to share
classes, and selected `L=33/65` classes to agree. Corresponding transition
ticks, if any, may differ by at most two.

## Verdict map

- Any parent reconstruction, root isolation, initialization, action, Gauss,
  energy, recoil, causal, persistence, or inverse infrastructure gate fails:
  `ENERGY_ADAPTED_MIXED_CAPTURE_TRANSACTION_UNRESOLVED`.
- Any center fails:
  `CAPTURE_CENTER_LONG_HORIZON_UNSTABLE`.
- Any matched polarity class differs:
  `CAPTURE_MIXED_CORNERS_POLARITY_SENSITIVE`.
- Any selected volume class differs:
  `CAPTURE_MIXED_CORNERS_VOLUME_SENSITIVE`.
- Centers and comparison controls pass, but at least one valid mixed corner
  does not survive:
  `CAPTURE_MIXED_DYNAMICAL_BOUNDARY_WITNESSED`.
- All 216 histories survive:
  `CAPTURE_ENERGY_ADAPTED_MIXED_CORNERS_SURVIVE`.

Even the final verdict is finite-direction, finite-amplitude,
finite-volume, and finite-horizon evidence. It does not establish a
dissipative attractor, invariant open basin, asymptotic particle, physical
mass, quantum state, or postulate-native interaction law.
