# Derivation: FTD-Native Engine Transport b=2 Flow

**Date:** 2026-04-24
**Status:** [PARTIAL] real-engine Moore transport, collision, mixed histories, multi-tick intervals, and GPU-native movement ledgers connected to native finite-volume blocking
**Purpose:** Extract signed face currents from actual `RenderBridge::tick()` movement histories and verify native b=2 continuity flow.

---

## Executive result

Actual engine movement ticks now feed the native dual-cell continuity ledger.
The shared extractor maps one-tick signed state snapshots into:

```text
rho_before = s(t)
rho_after  = s(t+1)
I_face     = signed source transported across oriented faces
S_R        = local reaction residue
```

and verifies:

```text
Delta rho + div I = S_R
```

before and after b=2 blocking.

For GPU movement, the kernel now emits the one-tick event ledger directly:

```text
rho_before = device state copied immediately before movement
I_face     = atomic device-side movement current
S_R        = atomic device-side annihilation reaction
rho_after  = device state after movement
```

The direct GPU ledger is tested against the host snapshot extractor on face
movement, diagonal movement, annihilation, and bounce cases.

One-tick ledgers also accumulate into interval ledgers by telescoping:

```text
rho_before(interval) = rho(t0)
rho_after(interval)  = rho(tN)
I_interval           = sum_t I_t
S_interval           = sum_t S_t
```

Result:

```text
native_engine_transport_flow passed
```

This closes the first real-engine Moore-transport/collision/interval/GPU-event
bridge for native RG flow.

---

## Implementation

Audit:

```text
engine/include/ftd/eft/dual_cell_continuity.h
engine/src/eft/dual_cell_continuity.cpp
engine/include/ftd/gpu_buffers.h
engine/cuda/gpu_buffers.cu
engine/cuda/kernels_forces.cu
engine/cuda/gpu_engine.cu
engine/tests/test_native_engine_transport_flow.cpp
engine/tests/test_native_current_flow.cpp
engine/tests/test_gpu_continuity_ledger.cpp
ctest --test-dir engine/build_gpu_always -C Release -R "^(native_current_flow|native_engine_transport_flow|gpu_continuity_ledger)$" --output-on-failure
```

The test runs actual `RenderBridge::tick()` movement cases and extracts
histories by comparing before/after state snapshots. The extractor currently
supports:

```text
face movement on x/y/z axes
negative-charge transport
diagonal Moore movement routed as deterministic x/y/z face currents
opposite-sign collision classified as local reaction
same-sign bounce classified as a continuity no-op
mixed transport plus local reaction in one snapshot pair
multi-tick interval accumulation by summing per-tick ledgers
operator moments: |Delta rho|_1, |I|_1, |div I|_1, |S_R|_1, residual_linf
GPU-native event ledger parity against snapshot inference
```

It then blocks the extracted ledger with:

```text
block_dual_cell_continuity_b2(...)
```

and checks that fine and coarse continuity residuals are zero.

---

## Indexing correction

While connecting real `RenderBridge` histories, the dual-cell containers were
aligned to the engine's `Lattice` flat-index convention:

```text
index(x,y,z) = x * L^2 + y * L + z.
```

This matters because before/after snapshots from `RenderBridge::voxels()` use
that same ordering.

Updated:

```text
engine/src/eft/dual_cell_blocking.cpp
engine/src/eft/dual_cell_continuity.cpp
```

All native dual-cell tests still pass after the alignment.

---

## Combined native battery

Result on 2026-04-23:

```text
native_continuity passed
native_reaction_ledger passed
native_blocking_map passed
native_flow passed
native_current_flow passed
native_response_flow passed
native_engine_history_flow passed
native_engine_transport_flow passed
```

This means both pieces of the engine history ledger now connect to native
blocking:

```text
reaction-only histories        -> S_R blocks correctly
face/Moore transport histories -> I_face blocks correctly
collision histories            -> reaction/no-op classification closes
multi-tick interval histories   -> telescoped continuity blocks correctly
operator moments                -> measured before/after blocking
GPU full-tick event ledgers     -> direct kernel emission matches inference
```

---

## GPU full-tick continuity ledger

The CUDA path now keeps a native per-tick EFT continuity ledger on device:

```text
rho_before:  copied from d_state at tick entry
rho_after:   downloaded from d_state after the tick
I_face:      accumulated by movement/collision routing
S_reaction:  accumulated by local state-changing events
```

Covered state-changing CUDA phases:

```text
phase_write genesis      void -> +/-1
phase_write evaporation  +/-1 -> void
pair production          void, void -> +1, -1
movement                 Moore transport routed through oriented faces
annihilation             opposite signs -> void, void
weak transmutation       q -> -q
```

Non-state phases (phase_read, Gauss projection, forces, color/Yukawa/exchange
force updates, strong/weak field stencils, triad locking) do not write charge
continuity entries because they modify fields, velocities, or metadata rather
than rho. Their effects enter the ledger only when they later drive a state
change.

`GpuEngine::continuity_step()` returns the current device ledger. `RenderBridge`
also exposes `continuity_step()` so bridge-level campaigns can consume the
latest GPU tick without falling back to snapshot differencing.

The GPU parity test now covers:

```text
face transport
diagonal Moore transport
annihilation
same-sign bounce
genesis
evaporation
pair production
weak transmutation
RenderBridge ledger exposure
```

---

## What remains open

Snapshot differencing remains the portable fallback and parity oracle. Remaining
engine-history work:

```text
systematic operator-mixing flow campaigns from blocked histories
device-side reductions for long-run ledger moment streams without host downloads
```

The bridge is now ready for mixed-history and nonlinear-flow measurements.
