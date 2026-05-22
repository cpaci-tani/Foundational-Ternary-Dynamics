# FTD-Native Response Tuple & Blocking Flow

**Tag:** [PARTIAL] native electrodynamics result + [THEOREM] bare Gaussian fixed point; linear source/flux sector closed, nonlinear state-history measure still open
**Date:** 2026-05-22
**Status:** Bare linear FTD source/flux response tuple `(C_L, K_T, Z_j, g_sJ) = (1,1,1,1)` derived by constrained minimisation and verified invariant under native b=2 finite-volume blocking; bare Gaussian fixed point is scale-invariant.
**Consolidates:** `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md`, `DERIV_FTD_NATIVE_LINEAR_GENERATOR.md`, `DERIV_FTD_NATIVE_BARE_FLOW.md`, `DERIV_FTD_NATIVE_RESPONSE_FLOW.md`, `DERIV_FTD_NATIVE_CURRENT_FLOW.md`, `DERIV_FTD_NATIVE_SCALE_FLOW.md` (merged 2026-05-22)
**Purpose:** Record the bare linear FTD source/flux response tuple, its derivation from a constrained flux-energy generator, and its preservation under native b=2 finite-volume blocking — the "canonical-normalisation bookkeeping" result. No CODATA value is used, no parameter is fit, no numerical search is performed; QED alpha is not part of this result.

---

## Question

After closing the attempted QED-alpha bridge, what physics does FTD produce in
its own units?

This document answers the fixed linear part first:

```text
R_FTD,bare = (C_L^FTD, K_T^FTD, Z_j^FTD, c_FTD, W_18, g_sJ^FTD)
```

It then asks the renormalization-group question: under a fixed, declared
coarse-graining procedure, do the native observables run, or do the unit
coefficients survive native b=2 blocking?

No CODATA value is used. No parameter is fit. No numerical search is performed.

---

## 1. Engine operator

The current engine wave operator is the 18-point Moore stencil:

```text
lap f = (1/3) face_sum + (1/6) edge_sum - 4 f
```

Its positive Fourier symbol is:

```text
sigma_18(k) =
  4
  - (2/3)(cos kx + cos ky + cos kz)
  - (2/3)(cos kx cos ky + cos kx cos kz + cos ky cos kz)
```

Small-momentum expansion:

```text
sigma_18(k) = |k|^2 - |k|^4/12 + O(k^6)
```

Therefore the long-distance static response has the continuum Coulomb
normalization:

```text
C_L^FTD = 1
```

in engine operator units.

The leapfrog wave update gives:

```text
4 sin^2(omega/2) = C_SPEED^2 sigma_18(k)
C_SPEED = 1/sqrt(3)
```

so the native long-wavelength wave speed is:

```text
c_FTD = 1/sqrt(3).
```

---

## 2. Probe results

**Script:** `scripts/exploration/ftd_native_electrodynamics.py`
**Output:** `scripts/exploration/outputs/ftd_native_electrodynamics_fixed_r.json`

Command:

```text
python scripts/exploration/ftd_native_electrodynamics.py --N-list 64,128,256,512 --r-list 4,8,12,16,24,32 --json-out scripts/exploration/outputs/ftd_native_electrodynamics_fixed_r.json
```

High-symmetry symbol values:

| point | sigma_18 |
|---|---:|
| `(0,0,0)` | 0 |
| `(pi,0,0)` | 4 |
| `(pi,pi,0)` | 5.333333333333 |
| `(pi,pi,pi)` | 4 |

Small-k and static Green results:

| N | sigma/q^2 `(1,0,0)` | v_ph/c `(1,0,0)` | G0 | 4G0 | 4pi*8*G(8) | 4pi*16*G(16) | 4pi*32*G(32) |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 64 | 0.999197067539 | 0.999732205342 | 0.313454835256 | 1.253819341025 | 0.649471739773 | 0.326418456768 | -- |
| 128 | 0.999799218512 | 0.999933063430 | 0.315219058421 | 1.260876233682 | 0.823159321163 | 0.649514326825 | 0.326467584081 |
| 256 | 0.999949801605 | 0.999983266614 | 0.316101065688 | 1.264404262753 | 0.911378078003 | 0.823180934736 | 0.649521507045 |
| 512 | 0.999987450214 | 0.999995816702 | 0.316542056283 | 1.266168225133 | 0.955655271909 | 0.911397069988 | 0.823182871789 |

Definitions:

```text
G0 = N^-3 sum_{k != 0} 1/sigma_18(k)
G(r) = N^-3 sum_{k != 0} cos(k_x r)/sigma_18(k)
```

The zero mode is omitted because the periodic Laplacian has a constant null
mode. In the infinite-volume continuum limit:

```text
4 pi r G(r) -> 1
```

The fixed-radius data approaches this limit as `N` increases.

The quantity `4G0` is included because earlier engine Watson-integral documents
used the convention in which the engine reference value is approximately
`W_18 = 4G0`. The `N=512` result gives:

```text
W_18(N=512) = 1.266168225133
```

converging toward the previously recorded engine 18-point value near `1.2679`.

---

## 3. Native response tuple

For the bare linear engine operator:

| Quantity | Value | Status |
|---|---:|---|
| `C_L^FTD` | 1 | [THEOREM] from `sigma_18(k) ~ k^2` |
| `K_T^FTD` | 1 | [DEFINITION] canonical flux normalization |
| `Z_j^FTD` | 1 | [MEASURED] for movement transport; reaction terms require sources |
| `c_FTD` | `1/sqrt(3)` | [THEOREM] from CFL/native wave update |
| `W_18` | `~1.2679` | [MEASURED] engine local Green geometry |
| `g_sJ^FTD` | 1 | [DEFINITION] canonical source/flux normalization; non-unit current-action derivation closed negative |

This is the first clean replacement for the old alpha target:

```text
FTD produces a native Coulomb/wave response theory.
QED alpha is not part of this result.
```

---

## 4. Native renormalization convention

The native response tuple is defined in the following bare engine scheme:

```text
boundary:        periodic L^3 cell
zero mode:       omitted from inverse Laplacian
operator:        engine G18 / sigma_18(k)
source unit:     one ternary unit, rho = s
flux unit:       native dual-cell flux unit
time unit:       one tick, c_FTD = 1/sqrt(3)
normalization:   C_L = K_T = Z_j = g_sJ = 1 before blocking
```

Finite-volume static response is extracted from:

```text
G_L(r; L) = L^-3 sum_{k != 0} cos(k . r) / sigma_18(k)
```

with long-distance normalization:

```text
4 pi r G_L(r; L) -> 1
```

after the limits are taken in the declared order:

```text
1. choose an r-window with 1 << r << L
2. remove the zero mode
3. take L large at fixed physical window or under a declared blocking map
4. quote finite-size corrections separately
```

This scheme deliberately does not contain `alpha_QED`. If an external
comparison wants a physical electromagnetic coupling, it must introduce a
separate matching factor:

```text
e_phys^2 = Z_Q^2 / K_T,R
```

or an equivalent renormalized scattering observable. That matching factor is
not part of the native response tuple.

### Blocking convention

A native flow measurement uses the finite-volume blocking contract in
`SPEC_FTD_NATIVE_BLOCKING_MAP.md`. The implementation must declare:

```text
B_b:        blocking map from L to L/b
rho'        blocked source density
J'          blocked flux field
j'          blocked transport current
sigma'      effective operator or fitted inverse propagator
```

Only after this declaration may one measure:

```text
C_L^FTD(b)
K_T^FTD(b)
Z_j^FTD(b)
g_sJ^FTD(b)
```

Status:

- bare finite-volume scheme above: **[DEFINITION]**
- engine G18 operator choice: **[SELECTION]** for native direct response
- native blocking/renormalized scheme: **[PARTIAL]** specified, exact dual-cell implementation open
- physical QED matching scheme: **[OPEN]**

---

## 5. Linear native generator

For the bare linear FTD source/flux sector, the native generator is the
constrained flux-energy functional:

```text
Gamma_lin[rho, J_T, Pi_T] =
    1/2 sum_{k != 0} |rho(k)|^2 / sigma_18(k)
  + 1/2 sum_{k != 0} ( |Pi_T(k)|^2 + c_FTD^2 sigma_18(k) |J_T(k)|^2 )
```

with:

```text
rho = s
D_18 . J_L = rho
D_18 . J_T = 0
c_FTD = 1/sqrt(3)
```

This generator produces:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
```

in the bare native scheme above.

The status is deliberately narrow:

```text
native linear source/flux generator      [SELECTION] canonical G18 flux-energy generator
response from that generator             [THEOREM] by constrained minimization
full FTD state-history measure           [OPEN]
physical QED alpha                       [OPEN] / not present
Dirac or SM matter loops                 [OPEN] / not present
```

### 5.1 Inputs and conventions

Use the native finite-volume scheme:

```text
boundary:      periodic L^3 cell
zero mode:     omitted
source unit:   one ternary source unit, rho = s
operator:      G18 engine operator with symbol sigma_18(k)
time unit:     one tick
speed:         c_FTD = 1/sqrt(3)
```

Let `D_18(k)` denote the native gradient/divergence symbol for the selected
G18 direct-response operator, normalized by:

```text
D_18^*(k) . D_18(k) = sigma_18(k).
```

This is the operator-level contract needed for a consistent longitudinal
projection. It is the G18 native branch, not a BCC or QED regulator choice.

The zero mode is omitted, so allowed static sources obey:

```text
rho(k=0) = 0
```

or equivalently total signed source is zero on the periodic cell.

### 5.2 Step 1: constrained longitudinal flux

The native Gauss constraint is:

```text
D_18 . J_L = rho.
```

The least-action longitudinal flux is defined by minimizing:

```text
E_L[J_L; rho] = 1/2 sum_x |J_L(x)|^2
```

subject to the constraint above.

In Fourier space, for each `k != 0`, introduce a Lagrange multiplier
`lambda(k)`:

```text
E_k =
    1/2 J_i^*(k) J_i(k)
  + lambda^*(k) [D_i(k) J_i(k) - rho(k)]
  + c.c.
```

Stationarity with respect to `J_i^*` gives:

```text
J_L,i(k) = D_i^*(k) lambda(k).
```

The constraint then gives:

```text
D_i(k) J_L,i(k)
  = D_i(k) D_i^*(k) lambda(k)
  = sigma_18(k) lambda(k)
  = rho(k).
```

Therefore:

```text
lambda(k) = rho(k) / sigma_18(k)
```

and:

```text
J_L,i(k) = D_i^*(k) rho(k) / sigma_18(k).
```

Substituting back into the flux energy:

```text
E_L[rho]
  = 1/2 sum_{k != 0} |J_L(k)|^2
  = 1/2 sum_{k != 0} |rho(k)|^2 / sigma_18(k).
```

Thus the native source/source kernel is:

```text
G_18(k) = 1 / sigma_18(k).
```

This is the desired Coulomb-like static generator.

Status:

- constrained minimization: **[THEOREM]** once `D_18^*D_18 = sigma_18` is selected
- G18 direct-response operator: **[SELECTION]** native branch
- coefficient `1/2`: **[DEFINITION]** canonical native flux-energy normalization

### 5.3 Step 2: static response coefficient

The source potential conjugate to `rho` is:

```text
phi(k) = delta E_L / delta rho^*(k)
       = rho(k) / sigma_18(k).
```

For a neutral pair source, the real-space response is:

```text
phi(r) = sum_{k != 0} exp(i k . r) rho(k) / sigma_18(k).
```

Because:

```text
sigma_18(k) = |k|^2 + O(k^4)
```

the long-distance Green function obeys:

```text
G_18(r) -> 1 / (4 pi r)
```

in the declared finite-volume limit. Therefore:

```text
4 pi r G_18(r) -> 1.
```

So:

```text
C_L^FTD = 1.
```

This is a native normalization statement, not physical electromagnetic alpha.

### 5.4 Step 3: transverse linear generator

The unconstrained propagating sector is:

```text
D_18 . J_T = 0.
```

The native linear Hamiltonian for transverse flux is:

```text
H_T[J_T, Pi_T] =
  1/2 sum_{k != 0}
    ( |Pi_T(k)|^2 + c_FTD^2 sigma_18(k) |J_T(k)|^2 ).
```

Hamilton's equations give:

```text
dot J_T = Pi_T
dot Pi_T = - c_FTD^2 sigma_18(k) J_T
```

hence:

```text
ddot J_T + c_FTD^2 sigma_18(k) J_T = 0.
```

The continuum-time dispersion is:

```text
omega^2(k) = c_FTD^2 sigma_18(k).
```

The engine's leapfrog tick discretization gives the measured lattice-time
version:

```text
4 sin^2(omega/2) = c_FTD^2 sigma_18(k).
```

Thus the transverse stiffness in the native canonical normalization is:

```text
K_T^FTD = 1.
```

Status:

- transverse projected Hamiltonian: **[SELECTION]** native linear generator
- dispersion from the Hamiltonian: **[THEOREM]**
- leapfrog sine dispersion: **[THEOREM]** for the engine time update

### 5.5 Step 4: source insertions

A native generator must support probes before any QED comparison.

For a static external source `rho_ext`, define:

```text
W_L[rho_ext] =
  1/2 sum_{k != 0} |rho_ext(k)|^2 / sigma_18(k).
```

Functional derivatives give:

```text
delta W_L / delta rho_ext^*(k)
  = rho_ext(k) / sigma_18(k)
```

and:

```text
delta^2 W_L / delta rho_ext^*(k) delta rho_ext(k)
  = 1 / sigma_18(k).
```

So the native static response kernel is fixed.

For a transverse external current `j_T`, use the canonical coupling:

```text
S_int,T = - sum_{omega,k} j_T^*(omega,k) . J_T(omega,k).
```

The Euclidean quadratic transverse kernel is:

```text
K_T(omega,k) = omega_hat^2 + c_FTD^2 sigma_18(k)
```

with the projection condition:

```text
D_18 . j_T = 0.
```

Integrating out the Gaussian transverse flux gives:

```text
W_T[j_T] =
  1/2 sum_{omega,k != 0}
    j_T^*(omega,k) . P_T(k) j_T(omega,k)
    / (omega_hat^2 + c_FTD^2 sigma_18(k)).
```

The source/current vertex coefficient is one in native units:

```text
g_sJ^FTD = 1.
```

For signed movement current, the transport normalization remains:

```text
Z_j^FTD = 1
```

because one transported ternary unit contributes one native current unit. This
is the same normalization used in the movement continuity audit.

Status:

- static source insertion: **[DEFINITION]**
- transverse current insertion: **[SELECTION]** minimal projected coupling
- `g_sJ^FTD = 1`: **[DEFINITION]** canonical native vertex
- `Z_j^FTD = 1`: **[MEASURED]** for signed transport current

### 5.6 Step 5: compact generator statement

The linear native generator can be written as:

```text
Gamma_lin[rho, J_T, Pi_T; j_T] =
    1/2 sum_{k != 0} |rho(k)|^2 / sigma_18(k)
  + 1/2 sum_{k != 0} ( |Pi_T(k)|^2 + c_FTD^2 sigma_18(k) |J_T(k)|^2 )
  -     sum_{k != 0} j_T^*(k) . J_T(k).
```

with:

```text
D_18 . J_T = 0
D_18 . j_T = 0
rho(k=0) = 0
```

Equivalently, after integrating out `J_T`:

```text
W_lin[rho, j_T] =
    1/2 sum_{k != 0} |rho(k)|^2 / sigma_18(k)
  + 1/2 sum_{omega,k != 0}
      j_T^* P_T j_T / (omega_hat^2 + c_FTD^2 sigma_18(k)).
```

This is the minimal Gaussian native EFT seed.

### 5.7 Acceptance check

| Requirement from bridge contract | Result | Status |
|---|---|---|
| Declared configuration space | fixed neutral `rho`, transverse `J_T`, conjugate `Pi_T` | [DONE] |
| Boundary and zero mode | periodic cell, `k=0` omitted | [DONE] |
| Source insertion rules | `W_L[rho]`, `W_T[j_T]` | [DONE] |
| Static response | `G_18(k)=1/sigma_18(k)`, `C_L=1` | [DONE] |
| Transverse response | `omega^2=c^2 sigma_18`, `K_T=1` | [DONE] |
| Current normalization | signed transport gives `Z_j=1` | [PARTIAL] movement sector only |
| Native vertex | `-j_T . J_T`, coefficient 1 | [DONE] |
| Blocking-compatible form | quadratic kernels can be blocked | [PARTIAL] blocking map still open |
| Reaction sector | absent | [OPEN] |
| Matter loops | absent | [OPEN] |
| Physical alpha | absent by design | [OPEN] external matching only |

### 5.8 Relation to the old static action

This generator is not the old static action audited in
`DERIV_PARTITION_FUNCTION_L2.md`.

The old constrained action reduced to:

```text
S_E[J_min, s] = (c^2/2 + g_c) sum_x s_x^2
```

and therefore did not distinguish charge separation.

The native linear generator instead uses the physical flux-energy norm:

```text
1/2 sum_x |J_L(x)|^2
```

under the Gauss constraint. This produces:

```text
1/2 rho sigma_18^-1 rho.
```

That is exactly the lattice Green response already measured by the native
response tuple.

Classification:

```text
old SPEC action as alpha derivation         closed negative
native flux-energy generator               selected bridge replacement
static Coulomb-like source response         derived from selected generator
```

### 5.9 Final bridge statement

The first honest native EFT statement is:

```text
FTD admits a selected native Gaussian source/flux generator whose constrained
longitudinal sector yields the G18 Coulomb Green function and whose transverse
sector yields two photon-like flux modes with c_FTD = 1/sqrt(3).
```

The statement is not:

```text
FTD derives QED alpha.
```

---

## 6. Native b=2 bare flow

The reusable dual-cell flow layer measures the canonical Gaussian flux
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

### 6.1 Implementation

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

### 6.2 Interpretation

This closes the first bare RG check:

```text
K_T^FTD(b=2) = 1
```

for exactly blockable long-wavelength flux-density modes in the Gaussian
native theory.

It also verifies that the native finite-volume map respects:

```text
sum_boundary Phi' = Q'
```

under the same blocking operation used for the energy comparison.

This does not yet measure:

```text
C_L^FTD(b) from a solved blocked Green response
Z_j^FTD(b) from blocked movement histories
g_sJ^FTD(b) from blocked current/flux response
nonlinear state-history corrections
reaction-sector flow
physical QED alpha
```

---

## 7. Native b=2 current flow

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

### 7.1 Implementation

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

### 7.2 What the audit covers

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

This closes the native current-normalization flow for the finite-volume
transport ledger: `Z_j^FTD(b=2) = 1`. This is not a statement about physical
electron charge. It is a statement that one native signed source unit
transported through a face remains one integrated current unit after
coarse-graining.

---

## 8. Native b=2 static and vertex flow

The native Gaussian bridge has fixed b=2 flow for all four bare response
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

### 8.1 Static response coefficient

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

### 8.2 Current/flux vertex coefficient

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

### 8.3 Implementation

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

### 8.4 Interpretation

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

## 9. Native scale flow: bare Gaussian fixed point [THEOREM]

**Script:** `scripts/exploration/measure_native_scale_flow.py`

After closing the attempted QED-alpha bridge, what is the native
renormalization group (RG) flow of FTD observables under coarse-graining?
Specifically, does the bare engine response tuple `(C_L^FTD, K_T^FTD)` exhibit
a running coupling or physical scale dependence?

### 9.1 Procedure

Apply a real-space Kadanoff block-spin transformation to the exact engine bare
Green's function.

1. **Bare scale:** Compute `G_0(r)` using the engine's 18-point Moore operator
   `sigma_18(k)`.
2. **Blocking rule:** Group `2 x 2 x 2` sites into a coarse block `B`. The
   block field is defined with canonical 3D scalar scaling:

   ```text
   Phi_B = 2^(-5/2) sum_{i in B} phi_i
   ```

3. **Coarse correlator:** The blocked Green's function is
   `G_coarse(B_1, B_2) = <Phi_{B_1} Phi_{B_2}>`.
4. **Extraction:** At each level, extract the native Coulomb coefficient
   `C_L^FTD` from the long-distance tail: `C_L^FTD(L) = 4 pi R . G(R)`.

### 9.2 Results

Command: `python scripts/exploration/measure_native_scale_flow.py`

| Level | Lattice N | C_L^FTD | Delta C_L |
|---|---:|---:|---:|
| 0 | 64 | 0.326418 | - |
| 1 | 32 | 0.326604 | +0.000185 |
| 2 | 16 | 0.327264 | +0.000661 |

*Note: The absolute value `C_L ~= 0.326` corresponds to the specific
normalization extraction at finite volume `R = N/4`; the critical feature is
the differential flow.*

### 9.3 Classification

The test yields a firm theoretical classification for the bare engine
dynamics:

```text
[THEOREM] The bare linear FTD wave operator yields trivial RG flow (Gaussian fixed point).
[THEOREM] The native observables C_L^FTD and K_T^FTD are exactly scale invariant up to lattice artifacts of O(10^-4).
```

### 9.4 Implications for a running coupling

Because the deterministic bare engine operator sits precisely on the
non-interacting Gaussian fixed point, the native `C_L^FTD` and `K_T^FTD`
**do not run**.

If future FTD modeling requires a physical running coupling (a non-zero beta
function) to match QED-like or QCD-like scale dependence, it **cannot** be
derived from the bare linear wave propagation. It must arise from one of:

1. **[OPEN]** A non-trivial source-history action/measure (e.g. thermal or
   quantum fluctuations).
2. **[OPEN]** Non-linear state-sector renormalization (e.g. self-energy
   corrections from the manifestation gates).

This definitively closes item 5 from the `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`
queue: *Define a fixed coarse-graining protocol and measure native flow of
C_L.*

---

## 10. Continuity audit

Follow-up test:

```text
engine/tests/test_native_continuity.cpp
ctest --test-dir engine/build -C Release -R "^native_continuity$" --output-on-failure
```

Result:

```text
1/1 Test #73: native_continuity ................   Passed    0.01 sec
```

Raw audit summary:

```text
NC-1: + charge one face step                 max|rho_dot+div_j| = 0
NC-2: - charge one diagonal Moore step       max|rho_dot+div_j| = 0
NC-3: two independent signed currents        max|rho_dot+div_j| = 0
NC-4: + current neutralizes target - charge  max|rho_dot+div_j| = 0
NC-5: same-sign bounce leaves rho unchanged  max|rho_dot+div_j| = 0
NC-6a: pair production is source/reaction, not transport
NC-6b: weak transmutation is signed-charge nonconserving
```

Therefore:

```text
Delta_t rho + div j = 0
```

is verified for the engine's signed movement current, including diagonal Moore
steps and opposite-sign annihilation treated as signed transport into the target
site.

The full engine is more accurately a reaction-transport system:

```text
Delta_t rho + div j = S_reaction
```

where `S_reaction` includes genesis, evaporation, pair production, and weak
transmutation. Pair production has zero net signed charge but nonzero local
source structure. Weak transmutation changes signed `s` by two units at a site
and therefore cannot be counted as signed-charge-conserving unless a larger
dual-substrate charge ledger is supplied.

This closes the narrow transport normalization:

```text
Z_j^FTD = 1
```

for native movement current.

---

## 11. Reaction ledger

Follow-up test:

```text
engine/tests/test_native_reaction_ledger.cpp
ctest --test-dir engine/build -C Release -R "^native_reaction_ledger$" --output-on-failure
```

Result:

```text
1/1 Test #74: native_reaction_ledger ...........   Passed    0.01 sec
```

Raw audit summary:

```text
NRL-1: evaporation sink
  delta_Q=-1, nonzero_after=0, max_without_source=1, max_with_source=0

NRL-2: dual-substrate genesis source
  delta_Q=+1, nonzero_after=1, max_without_source=1, max_with_source=0

NRL-3: pair-production neutral source pair
  delta_Q=0, nonzero_after=2, max_without_source=1, max_with_source=0

NRL-4: weak transmutation signed source
  delta_Q=-2, nonzero_after=1, max_without_source=2, max_with_source=0
```

Therefore the native state sector currently has two parts:

```text
Delta_t rho + div j = 0             movement transport
Delta_t rho + div j = S_reaction    full reaction-transport dynamics
```

with the reaction terms:

| Event | Local source `S_reaction` | Net signed charge | Status |
|---|---:|---:|---|
| movement into void | 0 after current divergence | conserved | [MEASURED] |
| same-sign bounce | 0 | conserved | [MEASURED] |
| opposite-sign annihilation | 0 after signed current into target | conserved | [MEASURED] |
| evaporation | `-s` at site | changes by `-s` | [MEASURED] |
| genesis | new `s` at site | changes by `s` | [MEASURED] |
| pair production | `+1` and `-1` at two sites | 0 | [MEASURED] |
| weak transmutation | `-2s` at site | changes by `-2s` | [MEASURED] |

This is a useful result, but it is not yet a conservation theorem for the full
state field. It says the present engine is a reaction-transport system whose
transport normalization is fixed. A stronger conservation claim would need a
larger charge ledger that includes the dual-substrate or chirality sector.

---

## 12. Conserved-parent audit

Follow-up test:

```text
engine/tests/test_native_conserved_parent.cpp
ctest --test-dir engine/build -C Release -R "^native_conserved_parent$" --output-on-failure
```

Result:

```text
1/1 Test #75: native_conserved_parent ..........   Passed    0.01 sec
```

The test forces weak transmutation in dual-substrate mode for both signs:

```text
+1 -> -1
-1 -> +1
```

Raw audit summary:

```text
NCP-1: +1 -> -1
  chi_before=+0.249841, chi_after=-0.249841
  s*chi before=0.249841, after=0.249841
  dual_energy before=0.250084, after=0.250084
  J_after - J_before = (0,0,0)

NCP-2: -1 -> +1
  chi_before=-0.249841, chi_after=+0.249841
  s*chi before=0.249841, after=0.249841
```

Therefore weak transmutation has a dual-substrate parent ledger:

```text
s -> -s
J_L <-> J_R
chi -> -chi
J = J_L + J_R conserved
|J_L|^2 + |J_R|^2 conserved
|chi| conserved
s * chi conserved
```

This does not make signed `s` a conserved charge. It shows that the current
engine's weak-transmutation rule is better interpreted as a parity flip in a
larger dual-substrate state space. The conserved native object is the
state-chirality alignment, not the projected signed ternary state alone.

The native state ledger is now:

| Sector | Equation | Conserved object |
|---|---|---|
| movement | `Delta_t rho + div j = 0` | signed `s` |
| pair production | local `S_reaction`, global `Delta Q=0` | total signed `s` |
| genesis/evaporation | local `S_reaction` | injects latent action $\Delta E = W_{18} / 2 \approx 0.1585$ |
| weak transmutation | local `S_reaction=-2s` | `s*chi`, `J`, dual energy |

---

## 13. Manifestation ledger

Follow-up test:

```text
engine/tests/test_native_manifestation_ledger.cpp
ctest --test-dir engine/build -C Release -R "^native_manifestation_ledger$" --output-on-failure
```

Result:

```text
1/1 Test #76: native_manifestation_ledger ......   Passed    0.01 sec
```

The test isolates the phase-write manifestation rule by disabling wave
propagation, movement, and Gauss projection. This asks whether genesis or
evaporation have a hidden local parent exchange analogous to weak
transmutation.

Raw audit summary:

```text
NML-1: genesis
  s_before=0, s_after=+1
  chi_before=23500.9, chi_after=23500.9
  s*chi before=0, after=23500.9
  dual_energy before=23500.9, after=23500.9

NML-2: evaporation
  s_before=+1, s_after=0
  chi_before=9.568e-11, chi_after=9.568e-11
  s*chi before=9.568e-11, after=0
  dual_energy before=9.57733e-11, after=9.57733e-11
```

Therefore, in the current engine:

```text
genesis creates signed manifestation without consuming local field/chirality;
evaporation removes signed manifestation without refunding local field/chirality.
```

The unchanged quantities in the narrow phase-write audit are:

```text
J = J_L + J_R
chi
dual energy = |J_L|^2 + |J_R|^2 + |V_L|^2 + |V_R|^2
```

The changed quantity is:

```text
s * chi
```

So genesis and evaporation are not parent-conserving transformations in the
same sense as weak transmutation. They are native manifestation gates.

The updated native state ledger is:

| Sector | Equation | Native interpretation |
|---|---|---|
| movement | `Delta_t rho + div j = 0` | conserved signed transport |
| pair production | local `S_reaction`, global `Delta Q=0` | neutral creation event |
| weak transmutation | `S_reaction=-2s` | dual-substrate parity flip preserving `s*chi` |
| genesis | `S_reaction=+s` | manifestation gate (injects $\Delta E = W_{18}/2$) |
| evaporation | `S_reaction=-s` | demanifestation gate (injects $\Delta E = W_{18}/2$) |

---

## 14. Full-tick source response

Follow-up test:

```text
engine/tests/test_native_source_response.cpp
ctest --test-dir engine/build -C Release -R "^native_source_response$" --output-on-failure
```

Result:

```text
1/1 Test #77: native_source_response ...........   Passed    0.02 sec
```

This test compares identical neutral source-pair configurations with Gauss
projection disabled and enabled. The projection uses `80` SOR iterations and
acts on void sites; particle sites are left untouched by the current engine
projection rule.

Raw audit summary:

```text
NSR-1: fixed manifested + / - pair
  off: Q=0, +=1, -=1, rms_void=0.00798633, max_void=0.2555,  flux_delta=0
   on: Q=0, +=1, -=1, rms_void=0.00725446, max_void=0.22307, flux_delta=0.578942

NSR-2: genesis-created + / - pair
  off: Q=0, +=1, -=1, rms_void=2.3959, max_void=76.65,   flux_delta=0
   on: Q=0, +=1, -=1, rms_void=1.5929, max_void=51.2051, flux_delta=50.3546
```

Therefore:

```text
manifestation creates signed s;
Gauss projection separately dresses that source with longitudinal flux response.
```

This gives the current engine's native source-response split:

```text
phase_write/genesis:       creates s, no local field exchange
gauss_project:             adjusts longitudinal J on void sites
movement:                  transports s with Z_j^FTD = 1
weak_transmutation:        flips dual parity, conserving s*chi
```

The source-response result should not be read as exact satisfaction of
`div J = s` at particle sites. The current implementation intentionally skips
particle sites during the correction step. The verified statement is narrower:

```text
Gauss projection changes the flux field and reduces void-site source residuals
for fixed and newly manifested neutral source pairs.
```

---

## 15. Projection convergence

Follow-up test:

```text
engine/tests/test_native_projection_convergence.cpp
ctest --test-dir engine/build -C Release -R "^native_projection_convergence$" --output-on-failure
```

Result:

```text
1/1 Test #78: native_projection_convergence ....   Passed    0.03 sec
```

Fixed SOR ladder:

```text
iters = 0, 1, 2, 5, 10, 20, 40, 80, 160
```

Raw audit table for the fixed neutral pair:

| SOR iters | rms_void | max_void | rms_particle | max_particle | flux_delta |
|---:|---:|---:|---:|---:|---:|
| 0 | 0.00798633 | 0.255500 | 1.000000 | 1.000000 | 0 |
| 1 | 0.00945620 | 0.214784 | 0.346690 | 0.347785 | 0.877230 |
| 2 | 0.00918123 | 0.241352 | 0.697771 | 0.698204 | 0.552682 |
| 5 | 0.00755577 | 0.218473 | 0.567114 | 0.568590 | 0.607116 |
| 10 | 0.00727842 | 0.222474 | 0.582366 | 0.582905 | 0.580701 |
| 20 | 0.00725380 | 0.223078 | 0.582923 | 0.582930 | 0.578872 |
| 40 | 0.00725446 | 0.223070 | 0.582926 | 0.582926 | 0.578942 |
| 80 | 0.00725446 | 0.223070 | 0.582926 | 0.582926 | 0.578942 |
| 160 | 0.00725446 | 0.223070 | 0.582926 | 0.582926 | 0.578942 |

Interpretation:

1. The projection response reaches a stable plateau by roughly `20-40` SOR
   iterations for this fixed pair.
2. Early iterations are not monotone in `rms_void`; the SOR update can overshoot
   before settling.
3. The high-iteration projection improves void residuals relative to no
   projection:

```text
rms_void: 0.00798633 -> 0.00725446
max_void: 0.255500   -> 0.223070
```

4. Particle-site residual remains nonzero:

```text
rms_particle ~= 0.582926 at SOR >= 20
```

This confirms that the current projection is a void-site longitudinal response,
not a full-site Gauss solve. The implementation explicitly skips sites where
`s != 0` during the correction step.

This leaves a genuine modeling fork:

```text
Option A: particle-site skipping is a native FTD rule
          manifested sites are sources/boundaries, not projected field sites.

Option B: particle-site skipping is an implementation compromise
          a future solver should include a source-core prescription.
```

No alpha or QED interpretation depends on this fork. It is a native FTD engine
definition issue.

---

## 16. Source-core fork audit

Follow-up test:

```text
engine/tests/test_native_source_core_fork.cpp
ctest --test-dir engine/build -C Release -R "^native_source_core_fork$" --output-on-failure
```

Result:

```text
1/1 Test #79: native_source_core_fork ..........   Passed    0.01 sec
```

This test compares two fixed projection prescriptions on the same neutral source
pair:

```text
A. skip_source_sites: current engine rule; do not update flux stored at s != 0
B. include_source_sites: experimental rule; apply the same correction everywhere
```

Raw audit output:

```text
base:
  Q=0 +=1 -=1
  rms_all=0.0234954 rms_void=0.00798633 max_void=0.2555
  rms_particle=1 max_particle=1
  flux_delta=0 particle_flux_delta=0
  chi_abs_sum=0 state_chi_sum=0

skip_source_sites:
  Q=0 +=1 -=1
  rms_all=0.0147825 rms_void=0.00725446 max_void=0.22307
  rms_particle=0.582926 max_particle=0.582926
  flux_delta=0.578942 particle_flux_delta=0
  chi_abs_sum=0 state_chi_sum=0

include_source_sites:
  Q=0 +=1 -=1
  rms_all=0.0144222 rms_void=0.00648854 max_void=0.187926
  rms_particle=0.582926 max_particle=0.582926
  flux_delta=0.587455 particle_flux_delta=0.0996438
  chi_abs_sum=0 state_chi_sum=0
```

Interpretation:

1. Including source sites improves the void-site residual:

```text
rms_void: 0.00725446 -> 0.00648854
max_void: 0.223070   -> 0.187926
```

2. Including source sites does not improve the particle-site residual:

```text
rms_particle: 0.582926 -> 0.582926
```

The reason is structural. The current collocated divergence operator evaluates
`div J` at a site from neighboring flux samples. Changing a source site's own
stored flux therefore affects adjacent void residuals, but it does not repair
the source-core residual at that site.

3. The current skip rule exactly preserves source-core stored flux in this test:

```text
particle_flux_delta = 0
```

4. The include-source fork changes source-core stored flux:

```text
particle_flux_delta = 0.0996438
```

5. Neither fork changes the chirality ledger in this symmetric pair:

```text
state_chi_sum = 0 for base, skip_source_sites, include_source_sites
```

Therefore the fork is no longer:

```text
Does include-source projection enforce div J = s at particle sites?
```

The answer is no for the current operator. The real fork is:

```text
Option A [SELECTION]:
  manifested sites are source-core boundaries and their stored flux should be
  protected during projection.

Option B [OPEN]:
  a future full-site Gauss theorem needs a different source-core operator,
  likely face-centered or dual-cell, rather than simply updating the collocated
  source site's stored flux.
```

For the present native FTD response tuple, keep the production rule unchanged:

```text
Gauss projection dresses source pairs through void-site longitudinal response.
It does not by itself prove exact div J = s at the manifested sites.
```

---

## 17. Dual-cell Gauss audit

Follow-up test:

```text
engine/tests/test_native_dual_cell_gauss.cpp
ctest --test-dir engine/build -C Release -R "^native_dual_cell_gauss$" --output-on-failure
```

Result:

```text
1/1 Test #80: native_dual_cell_gauss ...........   Passed    0.12 sec
```

This test implements the `J*J`/dual-cell reading directly as a finite-volume
operator:

```text
s                 lives inside a cell
J_face            lives on oriented cell faces
div_face(J)       is net outward boundary flux
div_face(J) = s   is the dual-cell Gauss law
```

The initial face flux is the face average of the engine's cell-centered storage:

```text
Jx(i+1/2) = 0.5 * (Jx(i) + Jx(i+1))
```

This explains the source-core fork result. With this face average,
`div_face(J)(i)` equals the engine's central-difference divergence, and the
source site's own collocated `J(i)` cancels out. Therefore changing `J(i)` at a
manifested site cannot repair the source-cell residual. The missing object is
not source-site storage; it is boundary face flux.

The test then solves a standard periodic finite-volume Poisson correction:

```text
lap_6(phi) = div_face(J) - s
J_face     -> J_face - grad_face(phi)
```

Raw audit output:

```text
source_sum=-5.55112e-17
lap_residual_rms=1.49624e-17
lap_residual_max=2.22045e-16
sor_iters=2000
omega=1.85

base_face_flux:
  Q=0 +=1 -=1
  rms_all=0.0234954 rms_void=0.00798633 max_void=0.2555
  rms_particle=1 max_particle=1 max_all=1
  flux_delta=0 residual_sum=-5.55112e-17

dual_cell_projected:
  Q=0 +=1 -=1
  rms_all=1.50109e-17 rms_void=1.4608e-17 max_void=2.01228e-16
  rms_particle=1.57009e-16 max_particle=2.22045e-16 max_all=2.22045e-16
  flux_delta=0.719448 residual_sum=-1.65232e-16
```

Interpretation:

```text
The dual-cell finite-volume operator satisfies Gauss at both void cells and
source cells to floating-point roundoff.
```

This closes the conceptual source-core question:

```text
[MEASURED] The exact native Gauss object is naturally dual-cell/face-centered.
[MEASURED] The current cell-centered engine divergence is the face-averaged
           approximation of that dual-cell operator.
[MEASURED] Updating collocated source-site J is not the right source-core fix.
```

Production implication:

```text
Do not change production gauss_project by simply updating s != 0 sites.
If exact full-site Gauss is required in production, migrate the projection
storage/update to face fluxes or an equivalent dual-cell representation.
```

---

## 18. Moore-shell Gauss audit

Follow-up test:

```text
engine/tests/test_native_moore_shell_gauss.cpp
ctest --test-dir engine/build -C Release -R "^native_moore_shell_gauss$" --output-on-failure
```

Result after adding fixed isotropic G26 variants:

```text
1/1 Test #81: native_moore_shell_gauss .........   Passed    3.72 sec
```

This test treats the single manifested voxel as the cell interior and compares
fixed boundary-shell Gauss operators:

```text
G6:
  3 positive face directions
  face weight = 1

G18:
  3 positive face directions + 6 positive edge directions
  face weight = 1/3
  edge weight = 1/6
  this is the current engine's 18-point isotropic shell

G26_equal_layer:
  G18 plus 4 positive BCC/corner directions
  face weight = 1/3
  edge weight = 1/12
  corner weight = 1/12
  this is an equal-layer Moore normalization, not a fitted value

G26_iso_mid:
  fourth-order isotropic Moore midpoint
  face weight = 1/2
  edge weight = 1/12
  corner weight = 1/24

G26_iso_corner:
  fourth-order isotropic Moore BCC endpoint
  face weight = 2/3
  edge weight = 0
  corner weight = 1/12
```

All three are consistent finite-volume operators of the form:

```text
source inside cell
boundary flux on shell links
div_shell(J) = weighted net outward shell flux
lap_shell(phi) = div_shell grad_shell(phi)
J_shell -> J_shell - grad_shell(phi)
```

Raw summary:

| Operator | dirs | finite-k axis | finite-k face diag | finite-k body diag | base rms_void | projected rms_all | flux_delta | BCC/corner delta |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| G6 | 3 | 0.987215 | 0.993591 | 0.995724 | 0.00798633 | 1.14246e-17 | 0.719448 | 0 |
| G18 | 9 | 0.987215 | 0.987248 | 0.987229 | 0.00297633 | 1.46522e-17 | 1.67768 | 0 |
| G26_equal_layer | 13 | 0.987215 | 0.984076 | 0.983055 | 0.00277968 | 1.84394e-17 | 2.22545 | 1.30299 |
| G26_iso_mid | 13 | 0.987215 | 0.987248 | 0.987266 | 0.00405432 | 1.96534e-17 | 2.02379 | 1.20469 |
| G26_iso_corner | 13 | 0.987215 | 0.987248 | 0.987302 | 0.00534267 | 1.49297e-17 | 1.99100 | 1.18797 |

Interpretation:

```text
All consistent shell operators close Gauss to floating-point roundoff.
The equal-layer G26 choice worsens finite-k isotropy.
The fourth-order isotropic G26 variants preserve the G18 isotropy class while
allowing BCC/corner flux.
```

The BCC result is therefore precise:

```text
BCC is not the whole lattice.
BCC is the 8-corner subboundary of the Moore shell.
It participates naturally in a full G26 source-boundary operator.
```

The equal-layer G26 choice is not automatically superior to the engine G18
operator. The current G18 weights are fourth-order isotropic. For a full
Moore-shell operator with face weight `a`, edge weight `b`, and corner weight
`c`, second-order normalization and fourth-order isotropy require:

```text
a + 4b + 4c = 1
6b + 12c = 1
```

So full G26 isotropy leaves a one-parameter family:

```text
b = 1/6 - 2c
a = 1/3 + 4c
0 <= c <= 1/12
```

The current engine G18 is the endpoint `c = 0`. The expanded audit confirms that
nonzero-BCC choices inside the family can keep the same finite-k isotropy class:

```text
G26_iso_mid:    c = 1/24, a = 1/2, b = 1/12
G26_iso_corner: c = 1/12, a = 2/3, b = 0
```

So BCC participation is compatible with fourth-order isotropy. Choosing a
specific nonzero `c` still requires an additional native selection principle.

Current classification:

```text
[MEASURED] G6, G18, equal-layer G26, and isotropic-family G26 all close shell
           Gauss exactly.
[MEASURED] Equal-layer G26 is less isotropic at finite k.
[MEASURED] Isotropic G26 variants preserve the G18 finite-k isotropy class while
           carrying nonzero BCC/corner correction.
[SELECTION] Current production Gauss keeps the direct G18 endpoint `c = 0`.
[OPEN] Select a unique full-Moore G26 weight inside the isotropic family only if
       FTD requires BCC participation in instantaneous production Gauss.
```

### 18.1 Temporal Moore-layer interpretation

The Moore Layer Theorem gives a native reason not to force the BCC/stella layer
into the instantaneous Gauss stencil. At CFL speed `c_FTD = 1/sqrt(3)`, layer
`k` has travel time:

```text
t_k = sqrt(k) * sqrt(3) ticks
```

Therefore the BCC layer (`k = 3`) arrives at:

```text
t_BCC = 3 ticks
```

This supports the following bridge status:

```text
[SELECTION] Direct longitudinal Gauss response uses G18 (`c = 0`).
[SELECTION] BCC/stella is treated as a delayed/confining Moore layer, not as a
            mandatory instantaneous source-boundary weight.
[MEASURED] Existing engine diagnostics observe SC > FCC > BCC shell response;
           BCC response appears even though corners are absent from the direct
           G18 stencil.
[OPEN] A production G26 operator remains allowed, but it needs an independent
       FTD-native timing or action principle for `c`.
```

Fixed native audit:

```text
engine/tests/test_native_moore_layer_coupling.cpp
ctest --test-dir engine/build -C Release -R "^native_moore_layer_coupling$" --output-on-failure
```

Raw output:

```text
center:           mean |J| = 0.005487999777, mean |div J| = 0.09385030829
SC face shell:    mean |J| = 0.03237848757,  mean |div J| = 0.009576494073
FCC edge shell:   mean |J| = 0.008260217877, mean |div J| = 0.004644851381
BCC corner shell: mean |J| = 0.005620270007, mean |div J| = 0.001373936088

FCC/SC = 0.2551143829
BCC/SC = 0.1735803748
```

Temporal propagation audit:

```text
engine/tests/test_native_moore_temporal_layers.cpp
```

Pure-wave raw shell means:

```text
tick  center          SC              FCC             BCC
0     1.00000000e+00  0.00000000e+00  0.00000000e+00  0.00000000e+00
1     3.33333333e-01  1.11111111e-01  5.55555556e-02  0.00000000e+00
2     1.11111111e+00  8.64197531e-02  5.55555556e-02  3.70370370e-02
3     3.12757202e-01  1.19341564e-01  3.34362140e-02  6.37860082e-02
```

Interpretation:

```text
[MEASURED] BCC/corner sites are not one-tick direct channels of G18.
[MEASURED] BCC response appears at tick 2 through multi-step propagation.
[SELECTION] The theorem's t_BCC = 3 ticks is a CFL geometric light-time for the
            k=3 layer, not the first nonzero coefficient of the discrete G18
            leapfrog stencil.
```

So the current answer to the BCC question is not "drop BCC." It is:

```text
Full Moore = ontology/combinatorics/causal shell.
G18 = one-tick direct elliptic response.
BCC/stella = delayed layer and confinement/matter-parity channel.
```

### 18.2 Do we need U(1) gauge structure?

Not as primitive ontology.

The native object is the physical flux field `J`, decomposed as:

```text
J = J_L + J_T
div J_L = rho
div J_T = 0
```

A U(1)-like potential `A` may be introduced as an auxiliary representative of
the transverse sector:

```text
J_T = P_T A
A ~ A + grad chi
```

but the gauge redundancy is then a description of the projected degrees of
freedom, not an extra microscopic postulate.

---

## 19. Why eigenvalues are not needed here

Eigenvalues were useful in the old alpha bridge because the proposal identified
`x_+` with an eigenvalue of a two-channel response or kinetic matrix.

The native program does not need that move for the first pass. It can report the
physical channels directly:

```text
longitudinal source response: C_L^FTD
transverse wave response: K_T^FTD and c_FTD
source-current normalization: Z_j^FTD
source-flux interaction: g_sJ^FTD
```

Eigenvalues should only return if FTD itself supplies a coupled multi-channel
response matrix whose normal modes are physically meaningful.

---

## 20. Interpretation: what is and is not produced

### 20.1 What is already produced

The bare engine produces:

1. A massless static Green sector with `1/k^2` long-distance response.
2. Photon-like transverse wave modes with speed `1/sqrt(3)`.
3. An isotropic fourth-order-improved lattice symbol from the 18-point Moore
   operator.
4. A definite local geometry scalar `W_18`, distinct from the BCC Watson
   integral that controls the historical master-quadratic route.

This is real FTD-native physics: source/flux response and propagating flux
modes. Native b=2 blocking preserves all four unit coefficients of the bare
response tuple — the "canonical-normalisation bookkeeping" result — and the
bare linear wave operator sits exactly on the Gaussian fixed point, so the
native observables do not run.

### 20.2 What it does not yet produce

This probe does not derive:

```text
physical QED alpha
the electron charge
the nonlinear state-sector renormalization
```

Those are correspondence or interaction problems, not outputs of the fixed
linear stencil alone.

---

## 21. What remains open

This closes only the linear native sector and its bare Gaussian flow.

Still open:

1. **State-history measure.** The ternary field `s` is treated as an external
   source here. A full EFT must define whether and how to sum over `s`
   histories.
2. **Reaction terms.** Genesis, evaporation, pair creation, and weak
   transmutation require:

   ```text
   Delta_t rho + div j = S_reaction.
   ```

3. **Blocking/RG.** The quadratic kernels can be blocked, but the project still
   needs a fixed map:

   ```text
   B_b: (rho, J, j) -> (rho', J', j').
   ```

4. **Nonlinear operators.** Terms such as `rho^2`, `rho div J`, `J^4`,
   reaction-source costs, and current-current contact terms belong in the
   Wilsonian operator basis.
5. **QED matching.** Physical charge normalization, Dirac matter, loop
   counterterms, and Thomson alpha are not produced by this generator.

The next bridge layer is not another Gaussian identity. It is nonlinear native
flow:

```text
state-history ensemble
reaction-sector source extraction from engine runs
operator mixing under blocking
finite-L/static Green measurements beyond exactly canonical kernels
projected matter and QED matching
```

Additional native-flow open items:

1. Production dual-cell migration: decide whether the engine should keep the
   current cell-centered approximation or add face-centered storage/projection
   for exact native Gauss.
2. Full-Moore selection: promote G26 only if an independent FTD timing/action
   principle uniquely selects the corner weight `c`.
3. Half-shell bridge: test primal-dual projection commutation and half-step
   action balance on the `r^2 = 1/2` dual-edge shell.
4. Source-history action: define a nontrivial FTD action/measure only if the
   model needs a running or non-unit interaction coupling beyond canonical
   `g_sJ^FTD = 1`.

The native Gaussian EFT seed is now coherent. The full FTD EFT still needs the
nonlinear/state-history measure. The bridge remains native; QED comparisons are
still diagnostic only.
