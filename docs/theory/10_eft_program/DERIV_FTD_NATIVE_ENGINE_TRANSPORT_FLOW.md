# Derivation: FTD-Native Engine Transport b=2 Flow

**Date:** 2026-04-23
**Status:** [PARTIAL] real-engine face transport histories connected to native finite-volume blocking
**Purpose:** Extract signed face currents from actual `RenderBridge::tick()` movement histories and verify native b=2 continuity flow.

---

## Executive result

Actual engine movement ticks now feed the native dual-cell continuity ledger.
For face-neighbor movements, the adapter extracts:

```text
rho_before = s(t)
rho_after  = s(t+1)
I_face     = signed source transported across a face
S_R        = 0
```

and verifies:

```text
Delta rho + div I = 0
```

before and after b=2 blocking.

Result:

```text
native_engine_transport_flow passed
```

This closes the first real-engine transport-history bridge for native RG flow.

---

## Implementation

Audit:

```text
engine/tests/test_native_engine_transport_flow.cpp
ctest --test-dir engine/build_audit_cpu -C Release -R "^native_engine_transport_flow$" --output-on-failure
```

The test runs actual `RenderBridge::tick()` movement cases and extracts
face currents by comparing before/after state snapshots. It currently supports
one-face movements along the x-axis:

```text
+ source crosses coarse boundary
- source crosses coarse boundary
internal + source transport inside a coarse block
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
reaction-only histories      -> S_R blocks correctly
face-transport histories     -> I_face blocks correctly
```

---

## What remains open

The current extractor is intentionally narrow. Remaining engine-history work:

```text
diagonal/Moore movement routing into face-current paths
annihilation as transport plus local sink/source classification
same-sign bounce no-op classification
mixed movement plus reaction in one tick
longer histories with accumulated S_R and I_face
operator-mixing measurements from blocked histories
```

The bridge is now ready for mixed-history and nonlinear-flow measurements.
