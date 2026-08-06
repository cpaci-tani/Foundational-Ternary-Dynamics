# PRE-REGISTRATION — Travelling-wave recoil threshold v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0455`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0293`, `FTD-0438`, `FTD-0454`  
**Engine artifact:** `engine/tests/campaign_travelling_wave_recoil_threshold.cpp`  
**Campaign SHA256:** `621e89db07792f71cf10bdeac6ab0d2488c8b036e0fa246e56d8cae777e6d269`  
**Helper SHA256:** `52b9c0679e55d08008feaae894b2d48c050581293864e50b822ee941b3ff4738`

## 1. Question

Can a pre-existing exact source-free travelling mode supply enough phase-space
capacity for the FTD-0454 paired impulse to carry face-hop recoil at zero
complete energy cost? If so, does the threshold behave like a local amplitude
condition or like access to a global periodic wave reservoir?

## 2. Frozen background

- face hop `d=(+1,0,0)`, charge `q=+1`;
- selected particle speed `0.15`, work `1e-4`;
- the post-control minimal work field from FTD-0454;
- periodic volumes `L in {11,17,33}`;
- one exact transverse lattice mode, propagation along `x`, polarization
  along `y`, mode number `n=1`;
- phases `0` and `pi/2`;
- propagation signs `-1,+1`.

For `theta=2 pi x/L+phase`, the old-time mode is

```text
J_y = A sin(theta),
W_y = A[(1-cos omega)sin(theta)-sigma sin(omega)cos(theta)],
omega = 2 asin(C_WAVE sin(pi/L)).
```

This is an exact travelling eigenmode of the source-free symplectic tick, not a
continuum waveform. It is divergence-free and therefore does not change the
registered hop work.

## 3. Frozen threshold solver

For each of 12 `(L,phase,sigma)` arms, evaluate the exact global paired-impulse
minimum from FTD-0454 as a function of amplitude `A`.

- fixed bracket: `A in [0,1]`;
- require positive minimum at `A=0` and negative minimum at `A=1`, each by more
  than `1e-8`;
- if bracketed, perform exactly 80 bisection iterations, maintaining the upper
  endpoint on the non-positive side;
- no phase, mode, polarization, volume, work, speed, or bracket retuning;
- threshold residual `|minimum|<=1e-10`;
- construct the covariant-null zero-energy impulse at the non-positive endpoint
  and directly require total-energy and momentum residuals `<=1e-10`.

This is a registered root solve for a dynamical threshold, not a search for a
physical-constant near match.

## 4. Frozen scaling observables

For every arm record:

- threshold amplitude `A_*`;
- pure travelling-mode tick energy `A_*^2 E_unit`;
- effective participation sites
  `(sum |S|^2)^2/sum |S|^4` of the constructed zero-energy impulse;
- fraction of impulse norm inside the union of Moore-radius-one balls around
  source and target.

Aggregate phase/sign means at each `L`. Classify a global-reservoir signature
only if:

- all 12 arms cross;
- coefficient of variation of threshold pure-wave energy across volumes is
  `<=0.20`;
- `A_* sqrt(L)` has coefficient of variation `<=0.20`;
- mean effective support is at least `5%` of the volume at every `L`;
- mean local norm fraction at `L=33` is `<0.10`.

## 5. Locked outcomes

- `TRAVELLING_WAVE_GLOBAL_RESERVOIR_THRESHOLD_CONSTRUCTED`;
- `TRAVELLING_WAVE_THRESHOLD_CONSTRUCTED_SCALING_MIXED`;
- `NO_TRAVELLING_WAVE_THRESHOLD_IN_REGISTERED_BRACKET`;
- `MIXED_TRAVELLING_WAVE_THRESHOLD_ARMS`;
- `PROTOCOL_INVALID`.

## 6. Interpretation boundary

A crossing proves conditional kinematic capacity, not native production
mechanics. The optimizer may use the full periodic wave. Even the global-
reservoir verdict does not show that a local hop can access distant field
energy causally; that requires a separately constrained support gate.

No production dynamics are changed and no measured physical constant enters.
