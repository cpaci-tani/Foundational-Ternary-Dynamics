# PRE-REGISTRATION — Injective local permutation event v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0466`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0465`  
**Engine artifact:** `engine/tests/campaign_injective_local_permutation_event.cpp`

**Locked campaign SHA-256:**
`BD8155674364B23F57E359CBD2E70DA271BFB2B95DA8826DD7CC6C7F13CA92DB`

## 1. Question

Does the smallest injective control on the FTD-0464 36-site endpoint-local
support admit the particle hop while closing energy and momentum?

## 2. Frozen event map

Use the exact `L=33`, 48-tick, speed `0.15`, packet amplitude `0.02`, source
history, and 42 attempt times of FTD-0464/0465, with the initial dressing off
and on. At each event, act on the complete local `J/W` field, not a
provenance-separated source component.

For each of the nine `(y,z)` columns in the union of the source and target
Moore cubes, cyclically permute the four `x` positions

`source_x-1 -> source_x -> source_x+1 -> source_x+2 -> source_x-1`.

All six `J/W` components move by the same permutation. Fields outside the
36-site union remain unchanged. Move the manifestation from source to target.
The leading ambient face is transferred to the vacated trailing face rather
than being added to the moved coat. No amplitude, phase, support, or
counterterm is fitted.

## 3. Gates

- the support contains exactly 36 sites;
- a deterministic full-field fixture passes forward/inverse permutation to
  `1e-12`;
- every actual event passes its field inverse and zero-outside-support tests to
  `1e-12`;
- record particle kinematic validity after assigning the exact opposite of the
  complete field-plus-interaction energy change;
- for every kinematically valid event, require particle energy closure and
  compare the central field-momentum change with the required particle recoil
  at `1e-12`;
- add no compensating field impulse.

## 4. Locked classification

- `INJECTIVE_LOCAL_PERMUTATION_ENERGY_MOMENTUM_CLOSES`: all 84 events are
  kinematically valid and close momentum;
- `INJECTIVE_LOCAL_PERMUTATION_KINEMATIC_VETO`: at least one event has no real
  production-dispersion particle update;
- `INJECTIVE_LOCAL_PERMUTATION_MOMENTUM_MISMATCH`: all particle updates are
  valid but at least one event fails momentum closure;
- `PROTOCOL_INVALID`: support, inverse, locality, finiteness, or energy gates
  fail.

## 5. Interpretation boundary

This cyclic permutation is a selected control, not native production dynamics.
Success would establish only the existence of one information-preserving local
event family; it would still need cubic covariance, native selection, and
sequential no-reset stability. Kinematic or momentum failure closes this
smallest permutation control and requires a different injective local
transformation, not hidden provenance or an after-the-fact recoil.

## 6. Execution record

The deterministic and all 84 actual field inverses closed exactly, with zero
field change outside the 36-site support. All particle updates remained
kinematically valid and closed energy. Momentum passed `0/84`; residual RMS
was `0.00353528` dressing-off and `0.00353452` dressing-on. Locked verdict:

`INJECTIVE_LOCAL_PERMUTATION_MOMENTUM_MISMATCH`.
