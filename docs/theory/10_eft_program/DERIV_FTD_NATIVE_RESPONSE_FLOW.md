# Derivation: FTD-Native Static and Vertex b=2 Flow

**Date:** 2026-04-23
**Status:** [PARTIAL] Gaussian native response-flow tuple closed; nonlinear/reaction engine flow open
**Purpose:** Close the remaining Gaussian native-flow gates for `C_L^FTD(b)` and `g_sJ^FTD(b)`.

---

## Executive result

The native Gaussian bridge now has fixed b=2 flow for all four bare response
tuple entries:

```text
C_L^FTD(b=2)   = 1
K_T^FTD(b=2)   = 1
Z_j^FTD(b=2)   = 1
g_sJ^FTD(b=2)  = 1
```

This result is native and Gaussian. It does not include nonlinear
state-history effects, reaction-sector renormalization, matter loops, or QED
matching.

---

## Static response coefficient

The native Gaussian static source generator is:

```text
W_L[rho] = 1/2 sum_k rho(k)^2 / sigma(k).
```

Equivalently, the inverse source kernel is:

```text
K_L(k) = sigma(k).
```

Define:

```text
K_L(k) = sigma(k) / C_L^FTD.
```

Therefore:

```text
C_L^FTD = sigma(k) / K_L(k).
```

In the bare native generator:

```text
K_L(k) = sigma(k)
```

so:

```text
C_L^FTD = 1.
```

Under b=2 native blocking, the coarse Gaussian action is written in the same
canonical form with its coarse operator symbol. Therefore the bare Gaussian
coefficient remains:

```text
C_L^FTD(b=2) = 1.
```

This is a scheme statement about the native source kernel, not a physical
electromagnetic coupling.

---

## Current/flux vertex coefficient

The native transverse interaction is:

```text
S_int,T = - g_sJ^FTD sum I_T . J_T
```

in integrated finite-volume variables. The canonical vertex contraction is:

```text
V[I, Phi] = sum_cells V_cell (I_i / A_face) (Phi_i / A_face).
```

For b=2:

```text
V_cell' = 8
A_face' = 4
I'      = sum_{4 faces} I
Phi'    = sum_{4 faces} Phi
```

For exactly blockable long modes, current and flux densities are constant over
the block faces. Thus:

```text
V[I', Phi'] = V[I, Phi].
```

The canonical coupling remains:

```text
g_sJ^FTD(b=2) = 1.
```

---

## Implementation

Reusable helpers:

```text
native_static_response_coefficient(...)
canonical_current_flux_vertex(...)
```

in:

```text
engine/include/ftd/eft/dual_cell_flow.h
engine/src/eft/dual_cell_flow.cpp
```

Audit:

```text
engine/tests/test_native_response_flow.cpp
ctest --test-dir engine/build_audit_cpu -C Release -R "^native_response_flow$" --output-on-failure
```

Result on 2026-04-23:

```text
native_response_flow passed
```

Full native-flow battery:

```text
native_blocking_map passed
native_flow passed
native_current_flow passed
native_response_flow passed
eft_blocking passed
```

---

## Interpretation

This closes the Gaussian native response-flow tuple:

```text
R_FTD,bare(b=2) = (1, 1, 1, 1)
```

for:

```text
(C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD).
```

This is exactly what the linear generator predicts. It is not a physical alpha
derivation.

---

## What remains open

The next bridge layer is not another Gaussian identity. It is nonlinear native
flow:

```text
state-history ensemble
reaction-sector source extraction from engine runs
operator mixing under blocking
finite-L/static Green measurements beyond exactly canonical kernels
projected matter and QED matching
```

The native Gaussian EFT seed is now coherent. The full FTD EFT still needs the
nonlinear/state-history measure.
