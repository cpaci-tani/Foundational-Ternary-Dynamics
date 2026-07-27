# PRE-REGISTRATION — Mechanical history and hop work v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0449`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0443`, `FTD-0447`, `FTD-0448`  
**Engine artifact:** `engine/tests/campaign_mechanical_history_hop_work.cpp`  
**Campaign SHA256:** `c5bc17df6e3442efdc1fb5cc348bd023a62e182f7b7e311009e6e8b1dceb59d7`  
**Journal header SHA256:** `4a9aedc650fe882c0cb6421901784095da4ea079d3ccbc985dd412148583955a`  
**Journal implementation SHA256:** `94ebb526f3f31cb53d8907109ba29bd207e3d8e3828dcca6d2c2c7b31b620b91`  
**Observer-neutrality test SHA256:** `61ebff46f3f90ca1decd4022f8b5b7fa5a2c4f2b6bbbd2b0fdbce0e071c2110d`

## 1. Questions

1. Does the observer journal retain enough local state to reconstruct a
   mechanical movement event?
2. In an actual frozen production hop with a fixed field and nonzero exact
   endpoint action work, does `phase_movement` apply that work to particle
   energy?

## 2. Observer extension

The pre-existing `HistorySiteState` retained only index, state, chirality sign,
and `J/J_L/J_R`. It omitted velocity, remainder, wave velocities, persistent
IDs, spin/color/flavor, strong/weak fields, and timing scalars. FTD-0449 adds a
read-only full `Voxel` copy to each before/after site record while preserving
the legacy fields for existing consumers.

This is instrumentation only. It may not write any lattice, RNG, toggle, or
production phase state.

## 3. Frozen fixture

- `L=9`, CPU, all toggles off except movement;
- positive particle at `(4,4,4)`;
- velocity `(0.25,0,0)`, initial remainder `(0.80,0,0)`;
- source and target flux exactly zero;
- remote `J_x=2` at `(6,4,4)`, giving central-stencil
  `divJ(source)=0`, `divJ(target)=1`;
- no portable self-field transfer, so the field must remain byte-hash fixed;
- exact endpoint work `G_C` is therefore nonzero;
- one x-directed movement event occurs on the next tick.

## 4. Locked gates

- a fully populated standalone `Voxel` survives capture with identical
  explicit-field hash;
- journal-enabled and journal-disabled bridges have identical full explicit
  state hashes and RNG hashes after the tick;
- exactly one movement event records source/target indices, particle ID,
  velocity, and remainder transfer with residuals `<=1e-14`;
- fixed-field hash is unchanged;
- endpoint work magnitude is at least `1e-3`;
- production particle energy change is `<=1e-14`;
- work mismatch equals `-endpoint_work` to `1e-14`.

## 5. Locked outcomes

- `MECHANICAL_HISTORY_SUFFICIENT_HOP_WORK_NOT_APPLIED`: every gate passes;
  movement transports mechanics unchanged while nonzero action work is absent.
- `MECHANICAL_HISTORY_CAPTURE_INCOMPLETE`: snapshot/event gates fail.
- `PRODUCTION_HOP_APPLIES_ACTION_WORK`: nonzero work matches particle-energy
  change to `1e-14`.
- `PROTOCOL_INVALID`: any other result.

## 6. Interpretation boundary

A kinematic result confirms the production movement event is transport, not
the event-native work exchange proposed after FTD-0443. It does not by itself
say where the existing earlier continuous force phase obtained its energy.

The observer extension does not make a link transaction physical. It supplies
the minimum evidence needed to construct and reverse one without losing local
mechanical state.

## 7. Banned moves

- No fixture, hashes, energy convention, work, tolerance, or outcome label may
  change after first execution.
- No production phase ordering or dynamics changes.
- No claim that complete unitarity follows from a complete event snapshot.
