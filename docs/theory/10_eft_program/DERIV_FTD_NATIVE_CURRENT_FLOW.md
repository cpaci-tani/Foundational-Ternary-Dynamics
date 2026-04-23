# Derivation: FTD-Native Current b=2 Flow

**Date:** 2026-04-23
**Status:** [PARTIAL] native RG current gate closed for finite-volume transport/reaction ledger
**Purpose:** Verify that signed transport-current normalization remains canonical under native b=2 blocking.

---

## Executive result

The native continuity equation is:

```text
Delta rho + div I = S_reaction
```

where `I` is integrated signed current through oriented cell faces during one
tick. Under b=2 finite-volume blocking:

```text
Q'_before(X) = sum_{x in block X} rho_before(x)
Q'_after(X)  = sum_{x in block X} rho_after(x)
I'(F)        = sum_{fine faces f in coarse face F} I(f)
S'_R(X)      = sum_{x in block X} S_R(x)
```

The blocked equation is:

```text
Delta Q' + div I' = S'_R.
```

All internal fine-face currents cancel by opposite orientation. Therefore:

```text
Z_j^FTD(b=2) = 1
```

for native integrated transport current.

---

## Implementation

Reusable code:

```text
engine/include/ftd/eft/dual_cell_continuity.h
engine/src/eft/dual_cell_continuity.cpp
```

Core API:

```text
DualCellContinuity
div_current_at()
continuity_residual_at()
max_continuity_residual()
block_dual_cell_continuity_b2()
```

Audit:

```text
engine/tests/test_native_current_flow.cpp
ctest --test-dir engine/build_audit_cpu -C Release -R "^native_current_flow$" --output-on-failure
```

Result on 2026-04-23:

```text
native_current_flow passed
```

The combined native blocking/flow battery also passed:

```text
native_blocking_map passed
native_flow passed
native_current_flow passed
eft_blocking passed
```

---

## What the audit covers

The audit constructs:

1. A signed transport event crossing a coarse boundary.
2. A signed transport event internal to a coarse block.
3. A reaction source inside a coarse block.

It verifies:

```text
max |Delta rho + div I - S_R| = 0
```

on both fine and blocked fields.

It also verifies:

```text
sum Q_before preserved
sum Q_after preserved
sum S_R preserved
coarse boundary current keeps unit signed transport
```

---

## Interpretation

This closes the native current-normalization flow for the finite-volume
transport ledger:

```text
Z_j^FTD(b=2) = 1.
```

This is not a statement about physical electron charge. It is a statement that
one native signed source unit transported through a face remains one integrated
current unit after coarse-graining.

---

## What remains open

Still open:

```text
C_L^FTD(b)      static Green/source response flow
g_sJ^FTD(b)     transverse current/flux vertex flow
state histories nonlinear ensemble
toggle-by-toggle extraction of S_R from full engine runs
physical QED matching
```

The current result is a native RG invariant, not an external QED comparison.
