# Derivation: FTD-Native Linear Generator

**Date:** 2026-04-23
**Status:** [PARTIAL] bridge gate 2A; linear source/flux sector closed, nonlinear state-history measure still open
**Purpose:** Derive the minimal source-coupled generator that reproduces the fixed native response tuple without using physical alpha, QED matter, or Standard Model targets.

---

## Executive result

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

in the bare native scheme of `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md`.

The status is deliberately narrow:

```text
native linear source/flux generator      [SELECTION] canonical G18 flux-energy generator
response from that generator             [THEOREM] by constrained minimization
full FTD state-history measure           [OPEN]
physical QED alpha                       [OPEN] / not present
Dirac or SM matter loops                 [OPEN] / not present
```

---

## Inputs and conventions

Use the native finite-volume scheme:

```text
boundary:      periodic L^3 cell
zero mode:     omitted
source unit:   one ternary source unit, rho = s
operator:      G18 engine operator with symbol sigma_18(k)
time unit:     one tick
speed:         c_FTD = 1/sqrt(3)
```

The G18 positive scalar symbol is:

```text
sigma_18(k) =
  4
  - (2/3)(cos kx + cos ky + cos kz)
  - (2/3)(cos kx cos ky + cos kx cos kz + cos ky cos kz).
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

---

## Step 1: constrained longitudinal flux

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

---

## Step 2: static response coefficient

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

---

## Step 3: transverse linear generator

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

---

## Step 4: source insertions

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

---

## Step 5: compact generator statement

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

---

## Acceptance check

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

---

## Relation to the old static action

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

---

## What remains open

This closes only the linear native sector.

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

---

## Final bridge statement

The first honest native EFT statement is now:

```text
FTD admits a selected native Gaussian source/flux generator whose constrained
longitudinal sector yields the G18 Coulomb Green function and whose transverse
sector yields two photon-like flux modes with c_FTD = 1/sqrt(3).
```

The statement is not:

```text
FTD derives QED alpha.
```

The linear bridge is now ready for the next gate:

```text
implement the native dual-cell blocking map and measure the flow of
(C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD).
```

The blocking contract is specified in:

```text
SPEC_FTD_NATIVE_BLOCKING_MAP.md
```

The first bare-flow audit is recorded in:

```text
DERIV_FTD_NATIVE_BARE_FLOW.md
```
