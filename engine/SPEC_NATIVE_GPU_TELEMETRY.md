# SPEC — Native GPU Telemetry Snapshot Architecture

**Status:** `[IMPLEMENTING]`  
**Scope:** the interactive CUDA engine, native WebSocket server, and Scale-0
dashboard telemetry consumers.

## Purpose

The native renderer and the scientific side panels have different data needs.
The renderer consumes bounded visual frames; scientific telemetry requires
whole-lattice reductions.  The latter must not be driven by individual panel
reads, otherwise a smooth GPU simulation can be blocked by a serial stream of
side-panel requests.

This specification establishes a single observation boundary: the GPU owns
state mutation, a native publisher produces versioned scalar snapshots after
settled engine work, and dashboard panels consume the published store. A
single staged group set is coherent; separately cadenced cached groups retain
their own provenance rather than being presented as one instant.

## Ownership and ordering

```text
GPU tick / mutation stream
        │
        ├── increments state version
        ├── schedules a due scalar snapshot (non-blocking host enqueue)
        │       │
        │       └── fused reduction → pinned compact D2H → completion event
        │
        └── tick acknowledgement

native snapshot publisher polls the completion event
        │
        ├── merges fresh groups into immutable cache
        └── emits one telemetry_snapshot delta

WebSocketBridge → TelemetryHub store → passive panels
```

The publisher is the only component that may begin a native telemetry
reduction.  `get_telemetry` is a cache read and never launches GPU work.
Panels do not call engine getters to cause a readback.

## Invariants

1. **One state writer.** Ticks, injections, host uploads, and proper-time
   updates advance the engine state version.  A snapshot records the version
   observed when it was staged.
2. **No hidden host mirror.** Snapshot transfer is a fixed pinned scalar
   buffer; it must not call `voxels()` or download the lattice AoS.
3. **No panel-triggered reduction.** Browser demand changes publisher policy;
   it never synchronously computes a value.
4. **Per-group provenance.** Diagnostics, audit, gravity, and Lagrangian can
   have different cadence.  Each carries `{epoch, stateVersion, tick,
   snapshotVersion, stale}`.  A newer diagnostics result must not relabel an
   older audit as same-tick.
5. **Snapshot deltas are idempotent.** A client merges a group only when its
   `(epoch, snapshotVersion)` is newer than the stored group.  Reconnects and
   explicit cache reads may repeat a delta safely.
6. **Control wins over observation.** Profile, resize, reset, pause, and error
   recovery invalidate old generations.  A late snapshot from a prior scenario
   is discarded.
7. **Bulk visuals are separate.** Particles, flux volumes, slices, and sampled
   vector fields remain a bounded lower-priority visual-frame path; they do not
   share snapshot semantics or force scalar telemetry to wait behind them.

## Native protocol

The dashboard communicates its visible-consumer policy once, coalesced:

```json
{
  "cmd": "set_telemetry_demand",
  "diagnostics": true,
  "audit": true,
  "gravity": false,
  "lagrangian": false,
  "everyTicks": { "diagnostics": 1, "audit": 8 }
}
```

Missing fields retain their existing setting. Cadences are bounded by the
server. The acknowledgement reports the effective mask and cadence.

After a due GPU observation completes, the server emits a delta:

```json
{
  "type": "telemetry_snapshot",
  "snapshotVersion": 42,
  "epoch": 7,
  "tick": 1234,
  "publishedMask": 3,
  "availableMask": 7,
  "freshMask": 3,
  "pendingMask": 8,
  "groups": { "diagnostics": {}, "audit": {} },
  "groupMeta": {
    "diagnostics": { "epoch": 7, "stateVersion": 1234, "tick": 1234,
                       "snapshotVersion": 42, "stale": false }
  }
}
```

`get_telemetry` returns selected cached groups with the same metadata. It is
available for initial hydration and recovery, but is not a scheduling signal.

## Cadence policy

Cadence is a scientific observation policy, not a browser frame rate. Fast
diagnostics may be due every completed tick; deeper audit, gravity, and
Lagrangian groups are scheduled less frequently at large lattice sizes. The UI
renders the latest accepted record immediately and exposes its provenance/age
rather than presenting a lower-rate value as a current-tick measurement.

## Verification requirements

- A due snapshot is published without a follow-up client command.
- `get_telemetry` never changes the native snapshot version or starts a CUDA
  reduction.
- One snapshot uses a bounded scalar D2H transfer and no full voxel mirror.
- Tick work remains host-nonblocking while a snapshot is pending.
- Per-group epoch/tick provenance survives mixed cadence and scenario changes.
- Browser tests prove duplicate/late pushes cannot overwrite a newer store
  entry, and panels render without sending per-panel telemetry commands.
