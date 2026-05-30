# Specification: FTD-Native Blocking Map

**Date:** 2026-04-23
**Status:** [SELECTION] native Wilsonian blocking contract; engine adapter partially implemented
**Purpose:** Define the fixed coarse-graining map required to measure native RG flow of `(C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD)` without using QED or Standard Model targets.

---

## Executive result

The native Wilsonian blocking map is a finite-volume map on source, flux, and
current:

```text
B_b: (rho, J_face, j_face, S_reaction) -> (rho', J'_face, j'_face, S'_reaction)
```

For a spatial block factor `b = 2`:

```text
Q'(X)          = sum_{x in block X} rho(x)
Phi'_i(F)      = sum_{fine faces f in coarse face F} Phi_i(f)
I'_i(F_t)      = sum_{fine spacetime faces in coarse face F_t} I_i(f_t)
S'_R(X)        = sum_{x in block X} S_R(x)
```

This is the exact native map because it preserves the integral forms:

```text
sum_boundary Phi = Q
Delta_t Q + sum_boundary I = S_R
```

The corresponding coarse density variables are obtained by dividing by the
coarse cell volumes/areas:

```text
rho'_density = Q' / b^3
J'_density   = Phi' / b^2
j'_density   = I' / b^2          # for spatial blocking with tick unchanged
```

The engine's existing `eft::block_full()` is an adapter for RenderBridge's
cell-centered ternary storage. It preserves total signed charge, but it is not
the theorem-level native blocking map because:

```text
blocked Wilsonian source Q' can be -8..+8, not only {-1,0,+1}
exact Gauss preservation is naturally face-centered / dual-cell
cell-centered flux averaging is a density approximation, not boundary-flux blocking
```

Therefore:

```text
native blocking map                      [SELECTION] finite-volume Wilsonian map
Gauss preservation under native map       [THEOREM]
reaction-continuity preservation          [THEOREM]
current RenderBridge blocking adapter     [PARTIAL] charge total only, cell-centered
exact engine dual-cell blocking adapter   [OPEN]
```

---

## Inputs

Use the native bridge conventions:

```text
rho = s                         signed source density on fine cells
J_face                          oriented dual-cell boundary flux
j_face                          signed transport current through faces
S_reaction                      non-transport reaction source
D_face J = rho                  native Gauss law
Delta_t rho + D_face j = S_R    reaction-transport continuity
```

The face-centered notation is not decorative. It is the finite-volume form
that made exact dual-cell Gauss possible in the native audits:

```text
source lives inside a cell
flux lives on oriented cell faces
divergence is net outward boundary flux
```

Cell-centered `RenderBridge::voxel.flux` can approximate this, but the
Wilsonian map is defined on the finite-volume object.

---

## Block source

For a coarse cell `X`, let:

```text
B_X = { bX + r | r_i in 0..b-1 }.
```

The blocked integrated source is:

```text
Q'(X) = sum_{x in B_X} rho(x).
```

For `b = 2` and microscopic ternary `rho = s`:

```text
Q'(X) in {-8, -7, ..., +7, +8}.
```

This is not a bug. Wilsonian block variables are effective variables. They
need not live in the microscopic ternary alphabet.

The density form at coarse lattice spacing `a' = b a` is:

```text
rho'(X) = Q'(X) / b^3
```

in native source-density units.

Status:

- integrated source blocking: **[THEOREM]** finite-volume conservation
- density rescaling: **[DEFINITION]**
- forcing `Q'` back to ternary `{-1,0,+1}`: **[ADAPTER]**, not the Wilsonian map

---

## Block flux

For a coarse face `F_i(X+1/2 e_i)`, sum the oriented fine faces crossing the
same boundary:

```text
Phi'_i(F) = sum_{f in F} Phi_i(f).
```

For block factor `b`, there are `b^2` fine faces in a coarse face.

The density form is:

```text
J'_i(F) = Phi'_i(F) / b^2.
```

This preserves uniform flux density:

```text
J_i(f) = J0_i  for all fine faces
=> J'_i(F) = J0_i.
```

It also preserves exact Gauss in integrated form:

```text
sum_{coarse boundary faces F of X} Phi'_out(F)
  = sum_{x in B_X} sum_{fine boundary faces of x} Phi_out(f)
  = sum_{x in B_X} rho(x)
  = Q'(X).
```

All internal fine faces cancel by opposite orientation.

Status:

- boundary-flux blocking: **[THEOREM]** finite-volume Gauss preservation
- density normalization by `b^2`: **[DEFINITION]**
- current cell-centered average-flux implementation: **[PARTIAL]** approximation

---

## Block current

For signed transport current, use the same finite-volume logic on spacetime
faces. With spatial blocking `b = 2` and tick unchanged:

```text
I'_i(F_t) = sum_{fine transport events crossing coarse face F during tick t} I_i(f_t)
j'_i(F_t) = I'_i(F_t) / b^2.
```

If time is also blocked by factor `b_t`, then:

```text
I'_i(F_T) = sum over b_t ticks and b^2 fine spatial faces
j'_i(F_T) = I'_i(F_T) / (b^2 b_t)
```

depending on whether `j'` is stored as integrated current per coarse tick or
as current density. The first native flow pass uses spatial blocking only:

```text
b_t = 1.
```

Status:

- current-face summation: **[DEFINITION]**
- spatial-only first pass: **[SELECTION]**
- spacetime blocking for Lorentz-covariant RG: **[OPEN]**

---

## Block reaction source

The full engine is reaction-transport:

```text
Delta_t rho + D j = S_R.
```

Block the reaction source by cell summation:

```text
S'_R(X,t) = sum_{x in B_X} S_R(x,t).
```

Then:

```text
Delta_t Q'(X,t) + sum_boundary I'(F_t) = S'_R(X,t)
```

because summing the fine continuity equation over the block cancels all
internal fine-face currents.

Status:

- reaction-ledger preservation under finite-volume blocking: **[THEOREM]**
- toggle-by-toggle `S_R` extraction from engine: **[OPEN]**

---

## Native response flow definitions

After blocking, define the native running coefficients from the same response
forms used at the fine scale.

### Static source response

Fit the blocked inverse kernel:

```text
W_L'[Q'] = 1/2 sum_{K != 0} |Q'(K)|^2 G_L'^{-1}(K).
```

For the fixed native scheme:

```text
G_L'^{-1}(K) = C_L^FTD(b)^-1 sigma_b(K) + irrelevant terms.
```

The running `C_L^FTD(b)` is the coefficient of the small-`K` `1/sigma_b`
response after the declared rescaling.

### Transverse stiffness

For blocked transverse flux:

```text
omega^2(K) = c_b^2 K_T^FTD(b) sigma_b(K) + higher operators.
```

In the bare Gaussian theory:

```text
C_L^FTD(b) = 1
K_T^FTD(b) = 1
```

up to finite-size and irrelevant-operator corrections. Deviations in engine
measurements therefore identify nonlinear/reaction/implementation flow.

### Current normalization and vertex

Using blocked current:

```text
Delta_t Q' + D' I' = S'_R
```

fixes:

```text
Z_j^FTD(b) = 1
```

for pure transport under the native integrated-current convention.

The projected coupling:

```text
S_int,T' = - g_sJ^FTD(b) sum I'_T . A'_T
```

has:

```text
g_sJ^FTD(b) = 1
```

in the Gaussian native generator unless the state-history measure or nonlinear
operator mixing renormalizes it.

---

## Engine adapter policy

The existing C++ blocking module:

```text
engine/include/ftd/eft/blocking.h
engine/src/eft/blocking.cpp
```

implements:

```text
flux:   average cell-centered voxel flux
state:  charge-conserving ternary embedding with overflow
```

This remains valid for old Phase-2 measurement infrastructure, but its status
for the native bridge is:

```text
RenderBridge adapter, not theorem-level Wilsonian map.
```

Reasons:

1. The theoretical blocked source `Q'` can have magnitude greater than one.
2. Exact Gauss preservation is naturally face-centered.
3. Cell-centered flux averaging preserves uniform density but not exact
   boundary flux for arbitrary configurations.
4. Overflow embedding changes local source distribution even when total charge
   is preserved.

The next engine task is therefore not to delete the existing adapter. It is to
add a separate native audit layer:

```text
DualCellBlockedFields
block_native_dual_cell(...)
test_native_blocking_gauss(...)
test_native_blocking_continuity(...)
```

---

## Acceptance tests

A native blocking implementation should pass:

1. **Block source conservation**

```text
sum_X Q'(X) = sum_x rho(x)
```

2. **Exact blocked Gauss**

```text
D'_face Phi' = Q'
```

for arbitrary neutral source/flux configurations satisfying fine Gauss.

3. **Uniform flux preservation**

```text
J_i(f) = constant => J'_i(F) = constant.
```

4. **Internal face cancellation**

Fine flux on faces internal to a block must not contribute to coarse boundary
flux.

5. **Reaction-continuity preservation**

```text
Delta_t Q' + D' I' = S'_R.
```

6. **Adapter comparison**

Compare the cell-centered RenderBridge adapter to the native dual-cell map and
record the discrepancy as an implementation approximation, not as a physics
flow.

Current reusable implementation:

```text
engine/include/ftd/eft/dual_cell_blocking.h
engine/src/eft/dual_cell_blocking.cpp
engine/include/ftd/eft/dual_cell_flow.h
engine/src/eft/dual_cell_flow.cpp
engine/include/ftd/eft/dual_cell_continuity.h
engine/src/eft/dual_cell_continuity.cpp
```

Current audit:

```text
engine/tests/test_native_blocking_map.cpp
ctest --test-dir engine/build_audit_cpu -C Release -R "^native_blocking_map$" --output-on-failure
```

Result on 2026-04-23:

```text
native_blocking_map passed
native_flow passed
native_current_flow passed
native_response_flow passed
```

The test is intentionally finite-volume and independent of `RenderBridge`. It
proves:

```text
block source conservation
exact blocked Gauss
internal fine-face cancellation
uniform flux-density preservation after b^2 area rescaling
```

The existing RenderBridge adapter remains covered by:

```text
ctest --test-dir engine/build_audit_cpu -C Release -R "^eft_blocking$" --output-on-failure
```

which also passed on 2026-04-23.

The bare-flow interpretation is recorded in:

```text
DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md
DERIV_FTD_NATIVE_NONLINEAR_FLOW.md
```

---

## Non-goals

This blocking map does not:

```text
derive physical alpha
select BCC over G18
define Dirac matter
fit a beta function to QED
force blocked variables back into microscopic ternary ontology
```

It only defines the native Wilsonian coarse-graining needed before measuring
FTD-native flow.

---

## Immediate next implementation step

The reusable finite-volume structure now exists:

```text
struct DualCellFields {
    int L;
    vector<int> rho_cell;       // integrated cell source
    vector<Vec3> phi_face;      // or three face arrays
    vector<Vec3> current_face;
    vector<int> reaction_cell;
};
```

and implements:

```text
block_dual_cell_b2(fine) -> coarse
div_face(coarse.phi_face) == coarse.rho_cell
```

The Gaussian native response-flow tuple is now closed:

```text
C_L^FTD(b=2)  = 1
K_T^FTD(b=2)  = 1
Z_j^FTD(b=2)  = 1
g_sJ^FTD(b=2) = 1
```

The next implementation step is nonlinear native flow:

```text
extend transport extraction to diagonal/Moore movement and annihilation
block mixed reaction/transport state histories over longer runs
measure operator mixing under reaction/genesis/movement toggles
compare measured flow against the Gaussian fixed tuple
```
