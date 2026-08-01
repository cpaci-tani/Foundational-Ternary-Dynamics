# FTD-0731 — Multi-pass formation persistence v1

**Status:** `[PRE-REGISTRATION — LOCKED / NOT YET RUN]`  
**Identifier:** `FTD-0731`  
**Date:** 2026-07-29  
**Parent:** `FTD-0730`  
**Scope:** extend the qualified two-volume local recurrence through another
full horizon and distinguish durable multi-pass capture from recurrent
scattering or later release; no action, coefficient, state type, classifier
retrofit, production default, toggle, or scenario change.

## 1. Questions locked before execution

1. After the direction-dependent third transition, does the initially unbound
   `p=0.0120` pair remain inside a negative-energy sector, continue cycling,
   or leave again by tick 192?
2. Do the `p=0.0060/0.0095` negative cores persist through the doubled horizon?
3. Are transition sequences and final classes still matched on `L=33/65`?

## 2. Parent lock

- FTD-0730 protocol SHA-256:
  `50582DF6FAE3DBBC27AF4E9B271F4E141597BE04E1EF55FE0DF6C137C9ABEB83`;
- FTD-0730 JSON SHA-256:
  `ADA8931C266E860FD7D38C2D9FC14435FDCD615DCBFF5A0BB9257CE98E706DB4`;
- FTD-0730 verdict:
  `P012_REENTRY_LOCAL_DYNAMICS_VOLUME_STABLE`;
- parent transition classes:
  - common entry/exit ticks `7/26`;
  - face re-entry `63`;
  - edge re-entry `79`;
  - body-diagonal re-entry `96`;
- selected root tolerance `2e-14`, at most 384 iterations.

## 3. Frozen matrix

Run `L={33,65}`, periodic fields, `dt=1/4`, 192 forward and 192 state-only
reverse steps.

At each volume use directions:

- face `0_0_1`;
- edge `0_1_-1`;
- body diagonal `1_1_1`;

and both polarity orders for:

- initially unbound `p=0.0120`, separation `1.30`;
- persistent parents `p={0.0060,0.0095}`, separation `1.30`;
- pre-bound control `p=0.015`, separation `1.00`.

Total: 24 histories per volume, 48 complete histories.

Retain the exact action, initial periodic dress, current, field update,
normalization, compact-well parameters, root realization, and all rowwise
gates used by FTD-0730.

## 4. Locked observables and classifiers

Record every graph-transition tick through tick 192, pair and field energy at
every tick, dynamic-field morphology at ticks `48,96,128,160,192`, state-only
inverse recovery, and rowwise residual maxima.

For initially unbound `p=0.0120`:

- `durable_multipass_capture` requires:
  1. positive-energy outside start;
  2. at least one entry, exit, and later re-entry;
  3. the final graph transition through tick 192 is an entry;
  4. graph membership and pair internal energy `<-1e-6` for every tick
     `129--192`;
  5. positive net field-energy gain, dynamic-field norm `>1e-8`, magnetic
     energy `>1e-10`, and a doubled median dynamic-field radius `>=5` at
     least one registered morphology checkpoint;
  6. exact energy, recoil, inverse, and paired-volume gates.
- `recurrent_scattering` requires at least four transitions with no
  64-tick final negative-inside tail.
- `later_release` requires a negative-inside interval after the parent
  re-entry followed by a final exit and positive outside state at every tick
  `185--192`.

For `p=0.0060/0.0095`, extended persistence requires negative graph membership
at every tick `97--192`. Pre-bound controls must satisfy the same tail.

## 5. Volume and acceptance gates

- `L=33/65` must reproduce the parent first three transition ticks within two
  ticks for every matched `p=0.0120` arm;
- complete transition counts and corresponding ticks through 192 must match
  within two ticks across volumes;
- final physical class must match for every paired volume arm;
- all 48 histories must pass action, energy, recoil, inverse, and control
  gates.

## 6. Locked verdict map

- All controls pass and all six `p=0.0120` arms per volume satisfy durable
  capture: `MULTIPASS_RADIATIVE_CAPTURE_VOLUME_STABLE`.
- At least one matched direction class, but not all three, satisfies durable
  capture on both volumes: `DIRECTIONAL_MULTIPASS_CAPTURE_VOLUME_STABLE`.
- No durable capture; all direction classes continue multi-pass cycling:
  `VOLUME_STABLE_RECURRENT_SCATTERING`.
- No durable capture; at least one class later releases:
  `DIRECTIONAL_LATER_RELEASE`.
- Any matched transition or final class differs by volume:
  `MULTIPASS_DYNAMICS_VOLUME_SENSITIVE`.
- Any lower-energy parent or bound control loses its locked tail:
  `LONG_HORIZON_BOUND_CORE_UNSTABLE`.
- Any execution, action, energy, recoil, or inverse gate fails:
  `MULTIPASS_FORMATION_TRANSACTION_UNRESOLVED`.

Even durable multi-pass capture is a selected finite-volume classical
formation witness. Perturbative stability, moving composite dynamics,
spectral positivity, particle poles, mass, spin, statistics, and production
adoption remain separate gates.
