# Derivation: FTD-Native Bare b=2 Flow

**Date:** 2026-04-23
**Status:** [PARTIAL] native RG seed; bare Gaussian dual-cell flow implemented
**Purpose:** Record the first native flow measurement built on the finite-volume blocking map, without QED or Standard Model matching.

---

## Executive result

The reusable dual-cell flow layer now measures the canonical Gaussian flux
energy under native b=2 blocking:

```text
E[Phi] = 1/2 sum_cells V_cell sum_i (Phi_i / A_face)^2.
```

For microscopic fields:

```text
V_cell = 1
A_face = 1
```

After one b=2 block:

```text
V_cell' = 8
A_face' = 4
```

The implemented audit verifies:

```text
uniform long mode:      E_coarse / E_fine = 1
short/internal mode:    E_coarse / E_fine < 1
Gauss law:              div Phi = rho preserved before and after blocking
source conservation:    sum rho preserved
```

This is the expected Wilsonian behavior:

```text
long Gaussian modes keep canonical K_T = 1
short/internal structure is integrated out
```

---

## Implementation

Reusable code:

```text
engine/include/ftd/eft/dual_cell_flow.h
engine/src/eft/dual_cell_flow.cpp
```

Core API:

```text
canonical_flux_energy(fields, cell_volume, face_area)
measure_native_b2_flow(fine)
```

Audit:

```text
engine/tests/test_native_flow.cpp
ctest --test-dir engine/build_audit_cpu -C Release -R "^native_flow$" --output-on-failure
```

Result on 2026-04-23:

```text
native_flow passed
```

The existing native blocking and RenderBridge adapter tests also pass:

```text
native_blocking_map passed
eft_blocking passed
```

---

## Interpretation

### What this closes

This closes the first bare RG check:

```text
K_T^FTD(b=2) = 1
```

for exactly blockable long-wavelength flux-density modes in the Gaussian
native theory.

The remaining Gaussian tuple entries are closed in:

```text
DERIV_FTD_NATIVE_CURRENT_FLOW.md
DERIV_FTD_NATIVE_RESPONSE_FLOW.md
```

It also verifies that the native finite-volume map respects:

```text
sum_boundary Phi' = Q'
```

under the same blocking operation used for the energy comparison.

### What it does not close

This does not yet measure:

```text
C_L^FTD(b) from a solved blocked Green response
Z_j^FTD(b) from blocked movement histories
g_sJ^FTD(b) from blocked current/flux response
nonlinear state-history corrections
reaction-sector flow
physical QED alpha
```

Those are separate native-flow gates.

---

## Next native flow gates

1. **Static Green flow**

```text
solve native dual-cell Poisson on fine and blocked cells
extract C_L^FTD(b) from the small-K source response
```

2. **Transport-current flow**

```text
block signed movement histories
verify Delta_t Q' + div I' = S'_reaction
extract Z_j^FTD(b)
```

3. **Vertex flow**

```text
couple blocked transverse current to blocked transverse flux
measure g_sJ^FTD(b)
```

4. **Nonlinear/reaction flow**

```text
add reaction source ledger S_R
measure whether reaction toggles renormalize the native tuple
```

The bridge remains native. QED comparisons are still diagnostic only.
