# Open Problem: Native Action or Measure for the FTD EFT

**Date:** 2026-04-23
**Status:** [PARTIAL] bridge gate 2 from `SPEC_FTD_EFT_BRIDGE_CONTRACT.md`; linear source/flux generator derived, full state-history measure open
**Purpose:** Define the source-coupled generator required before FTD-native source/flux dynamics can be called a Wilsonian EFT.

---

## Problem statement

The native bridge currently has:

```text
rho = s
J = J_L[rho] + J_T
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
```

in bare engine units. This is a fixed response dictionary, but it is not yet a
full EFT. A Wilsonian EFT must specify what generates observables:

```text
correlators
response functions
operator mixing
blocking flow
renormalized couplings
```

Therefore the next bridge gate is:

```text
construct a native source-coupled action, transfer matrix, or deterministic
history measure whose observables reproduce the measured source/flux response.
```

No physical alpha value, Standard Model mass, or CODATA input is allowed in this
construction.

---

## Why the existing static action is not enough

`DERIV_PARTITION_FUNCTION_L2.md` showed that the current static analytical
action, after imposing:

```text
div J = s
```

collapses to an ultralocal state cost:

```text
S_E[J_min, s] = (c^2/2 + g_c) sum_x s_x^2.
```

That action counts manifested sites but does not distinguish charge separation.
It therefore does not generate the Coulomb-like Green response used by the
native response tuple.

The engine has Coulomb-like response through:

```text
dual-cell Gauss / Poisson response
field-energy diagnostics
emergent force extraction
```

The bridge must now decide which of these belongs inside the generator of the
EFT, and how.

---

## Candidate generators

### Option A: constrained flux-energy action

Define the native static generator by:

```text
S_native[J, rho] =
    (K_L/2) sum_x |J_L(x)|^2
  + (K_T/2) sum_x |grad J_T(x)|^2
  + constraint[div J_L - rho]
```

or, after integrating out constrained longitudinal flux:

```text
S_eff[rho, J_T] =
    (C_L/2) sum_{k != 0} |rho(k)|^2 / sigma_18(k)
  + (K_T/2) sum_k sigma_18(k) |J_T(k)|^2.
```

Advantages:

```text
matches the native response tuple directly
produces Coulomb-like source response
keeps QED alpha out of the definition
```

Cost:

```text
this is a new native generator, not the old static action
must be justified from engine energy ledger or dual-cell flux ontology
```

Status: [SELECTION] candidate.

### Option B: real-time transfer map

Treat the deterministic tick update as the fundamental object:

```text
U_tick: (s_t, J_t) -> (s_{t+1}, J_{t+1})
```

and define observables through long-time histories:

```text
<O> = history average over initialized ensembles and fixed toggles.
```

Advantages:

```text
closest to the engine
handles reaction-transport dynamics naturally
does not invent an Euclidean action
```

Cost:

```text
Wilsonian blocking of deterministic histories must be defined
stationary ensemble and source insertions remain open
reflection positivity / unitarity analog is not automatic
```

Status: [OPEN] candidate.

### Option C: Euclidean history measure

Define a path weight over histories:

```text
Z[eta, A_ext] =
  sum_{s(t)} int DJ(t)
    exp(-S_history[s,J] + eta rho + A_ext . j_T).
```

The action would include:

```text
Gauss constraint
flux kinetic/gradient terms
transport current cost
reaction source cost
native source/flux response
```

Advantages:

```text
closest to standard EFT and RG machinery
supports source insertions and loop expansion
```

Cost:

```text
largest new theoretical commitment
must avoid retrofitting coefficients to external targets
```

Status: [OPEN] candidate.

---

## Required source terms

A valid generator must support external probes:

```text
eta(x,t) rho(x,t)           static source response
h_i(x,t) J_T,i(x,t)         transverse flux response
a_i(x,t) j_T,i(x,t)         current/radiation coupling
lambda_R(x,t) S_reaction    reaction-source ledger
```

These probes define the native correlators:

```text
<rho rho>       -> C_L^FTD
<J_T J_T>       -> K_T^FTD and dispersion
<j_T j_T>       -> Z_j^FTD
<j_T J_T>       -> g_sJ^FTD after projection
<S_reaction O>  -> reaction-sector corrections
```

No external QED observable appears in these definitions.

---

## Acceptance criteria

A proposed native generator passes this gate only if it provides:

1. A declared configuration space:

```text
s histories, J histories, or both
boundary conditions
zero-mode convention
enabled toggle class
```

2. A declared weight or evolution rule:

```text
action, transfer matrix, or deterministic ensemble
```

3. Source insertion rules:

```text
functional derivatives that define C_L, K_T, Z_j, g_sJ
```

4. Agreement with the fixed native response tuple in the bare linear limit:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
c_FTD = 1/sqrt(3)
```

5. A blocking-compatible form:

```text
B_b S_native -> S_native' with transformed couplings
```

6. An error and scheme ledger:

```text
finite L
zero mode
operator choice
blocking choice
reaction toggles
CPU/GPU parity if engine-measured
```

---

## Current best route

The least speculative route is:

```text
Option A for the linear source/flux sector
Option B for reaction-transport extensions
Option C deferred until a history measure is required for loops
```

This gives a staged bridge:

```text
1. constrained flux-energy generator for native linear response
2. deterministic history ledger for nonlinear/reaction updates
3. optional Euclidean history measure for loop/RG calculations
```

The first milestone is now:

```text
DERIV_FTD_NATIVE_LINEAR_GENERATOR.md
```

with no external constants and no QED matching.

It closes the linear constrained-flux sector:

```text
Gamma_lin[rho, J_T, Pi_T] =
    1/2 rho sigma_18^-1 rho
  + 1/2 (Pi_T^2 + c_FTD^2 sigma_18 J_T^2)
```

and leaves the nonlinear state-history measure open.

---

## Non-goals

This gate does not try to:

```text
derive physical QED alpha
derive electron charge
derive Dirac matter
match Standard Model parameters
```

Those belong to later matching branches. This gate only decides what native
FTD object generates native EFT observables.
