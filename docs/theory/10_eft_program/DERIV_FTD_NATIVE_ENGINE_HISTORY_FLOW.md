# Derivation: FTD-Native Engine-History b=2 Flow

**Date:** 2026-04-23
**Status:** [PARTIAL] real-engine reaction histories connected to native finite-volume blocking
**Purpose:** Connect actual `RenderBridge::tick()` reaction histories to the dual-cell continuity ledger used by native RG flow tests.

---

## Executive result

The native finite-volume continuity ledger now accepts actual engine
before/after state histories for reaction-only ticks:

```text
rho_before = s(t)
rho_after  = s(t+1)
I          = 0
S_R        = rho_after - rho_before
```

and verifies:

```text
Delta rho + div I = S_R
```

both before and after b=2 blocking.

The engine-history audit covers:

```text
genesis
pair production
weak transmutation
stochastic evaporation / no-op
```

Result:

```text
native_engine_history_flow passed
```

---

## Implementation

Audit:

```text
engine/tests/test_native_engine_history_flow.cpp
ctest --test-dir engine/build_audit_cpu -C Release -R "^native_engine_history_flow$" --output-on-failure
```

The test converts actual `RenderBridge` snapshots into:

```text
DualCellContinuity
```

from:

```text
engine/include/ftd/eft/dual_cell_continuity.h
engine/src/eft/dual_cell_continuity.cpp
```

It then applies:

```text
block_dual_cell_continuity_b2(...)
```

and verifies that the reaction ledger still closes.

---

## Evaporation correction

The older native reaction ledger treated evaporation as deterministic. The
current engine rule is stochastic:

```text
evap_prob = exp(-local_energy / K_B^2) * 0.1.
```

Therefore a one-tick low-energy particle may either:

```text
remain manifested     delta_Q = 0
evaporate             delta_Q = -1
```

The correct invariant is not "evaporation must occur." The invariant is:

```text
if state changes, S_R = delta rho;
if state does not change, S_R = 0;
in either case Delta rho + div I = S_R.
```

`engine/tests/test_native_reaction_ledger.cpp` was updated to reflect this
current engine behavior.

---

## Combined native battery

Result on 2026-04-23:

```text
native_reaction_ledger passed
native_blocking_map passed
native_flow passed
native_current_flow passed
native_response_flow passed
native_engine_history_flow passed
```

This connects the Gaussian native RG objects to real engine reaction histories.

---

## What remains open

This is still reaction-only history extraction. The next bridge step is to add
transport-current extraction from real movement ticks:

```text
s(t), s(t+1), movement events -> I_face
```

Then the engine-history ledger can cover mixed reaction-transport ticks:

```text
Delta rho + div I = S_R
```

with both `I` and `S_R` extracted from engine dynamics rather than supplied by
hand.

The face-transport part is now started in:

```text
DERIV_FTD_NATIVE_ENGINE_TRANSPORT_FLOW.md
```
