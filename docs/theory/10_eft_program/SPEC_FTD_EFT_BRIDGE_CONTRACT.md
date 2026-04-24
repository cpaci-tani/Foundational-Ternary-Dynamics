# FTD to EFT Bridge Contract

**Date:** 2026-04-23
**Status:** [SELECTION] bridge contract; native EFT first, QED matching second
**Purpose:** Freeze the minimal contract that lets FTD become a Wilsonian EFT without using physical alpha or Standard Model targets as bridge inputs.

---

## Executive statement

The bridge is not:

```text
FTD arithmetic root x_+ numerically matches 1/alpha, therefore FTD is QED.
```

The bridge is:

```text
FTD microscopic variables
    -> native source/flux fields
    -> fixed continuum scaling prescription
    -> symmetry-allowed operator basis
    -> source-coupled generating functional or transfer measure
    -> RG/blocking flow
    -> renormalized native observables
    -> optional external QED/SM comparison
```

Until this chain is closed, QED and Standard Model comparisons are diagnostic
only. They may guide criticism, but they may not select definitions,
regulators, counterterms, or normalizations.

---

## Current fixed bridge pieces

| Piece | Contract value | Status |
|---|---|---|
| Microscopic source alphabet | `s in {-1,0,+1}` | [AXIOM] |
| Signed source density | `rho = s` | [THEOREM] internally |
| Flux variable | `J_i in R^3` physical vector flux | [SELECTION] |
| Native decomposition | `J = J_L[rho] + J_T` | [THEOREM] after projection |
| Longitudinal constraint | `div J_L = rho` | [THEOREM] for the chosen Gauss operator |
| Transverse modes | `div J_T = 0`, two propagating DoF | [THEOREM] after constraint |
| Auxiliary U(1)-like variable | `J_T = P_T A`, `A ~ A + grad chi` | [SELECTION] representation |
| Native static response | `C_L^FTD = 1` in bare engine units | [THEOREM] for `sigma_18(k) ~ k^2` |
| Native transverse stiffness | `K_T^FTD = 1` canonical normalization | [DEFINITION] |
| Native current normalization | `Z_j^FTD = 1` for signed transport | [MEASURED] movement current |
| Native source/flux vertex | `g_sJ^FTD = 1` canonical normalization | [DEFINITION] |
| Native wave speed | `c_FTD = 1/sqrt(3)` | [THEOREM] from native wave update |
| Physical QED alpha | not derived by current bridge | [OPEN] / current alpha bridge closed negative |

This contract makes the first successful EFT target:

```text
native FTD source/flux EFT
```

not:

```text
physical QED with alpha predicted from x_+
```

---

## Hard prohibition

The following moves are outside the bridge contract:

1. Choosing an operator, regulator, finite counterterm, source normalization, or observable because it improves the residual against CODATA alpha.
2. Calling a standard QED or Standard Model formula an FTD derivation after inserting FTD-selected numbers.
3. Reclassifying the arithmetic root `x_+` as physical `1/alpha` without a new normalization theorem.
4. Treating the auxiliary projected variable `A` as a primitive microscopic gauge field.
5. Using bubble-only or zero-momentum loop diagnostics as physical alpha observables without Ward-compatible contact terms and renormalization conditions.

Allowed language:

```text
x_+ is an arithmetic FTD root with a close empirical alpha match.
```

Disallowed language under the current bridge:

```text
x_+ is derived physical 1/alpha_QED.
```

---

## Required bridge gates

### Gate 1: Field dictionary

Define the continuum fields and their dimensions from FTD variables:

```text
rho_a(x,t)      = a^{-3} s(x,t)
J_a(x,t)        = Z_J(a) J_lattice(x,t)
j_a(x,t)        = Z_j(a) j_lattice(x,t)
J_T             = P_T J
J_L             = P_L J
```

Required output:

```text
field dimensions
normalization choices
projection convention
boundary/zero-mode convention
```

Status: [PARTIAL]. The dictionary exists qualitatively; the scaling and
normalization map must be frozen before continuum claims.

### Gate 2: Native action or measure

An EFT requires a generator of observables. FTD must choose one of:

```text
Euclidean action / partition function
real-time transfer matrix
Hamiltonian plus constraint surface
stationary ensemble over deterministic histories
```

Required output:

```text
Z[sources] or equivalent history measure
correlation-function definition
source insertion rules
reflection/unitarity/stability statement, as appropriate
```

Status: [PARTIAL]. `DERIV_FTD_NATIVE_LINEAR_GENERATOR.md` supplies the
linear constrained-flux generator. Existing finite-L partition-function work is
useful but not yet a general nonlinear state-history measure. The active gate
document is `OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md`.

### Gate 3: Symmetry and operator basis

List every low-dimension operator allowed by the actual FTD symmetries:

```text
translation
cubic O_h rotations
charge conjugation s -> -s
time reversal or arrow selection
source/flux constraint symmetries
projected transverse redundancy, if using A
```

Required output:

```text
complete relevant/marginal operator table through the chosen dimension
forbidden operators with symmetry reason
engineering dimensions
renormalization mixing classes
```

Status: [PARTIAL]. `SPEC_OPERATOR_BASIS.md` is a measured starter basis, not a
complete Wilsonian basis.

### Gate 4: Blocking and RG

Define a fixed blocking map:

```text
B_a->ba: (s, J, j) -> (s', J', j')
```

Required output:

```text
charge conservation under blocking
flux projection compatibility
coupling extraction rules
beta functions for native coefficients
scheme-dependence audit
```

Native coefficients to flow first:

```text
C_L^FTD(L)
K_T^FTD(L)
Z_j^FTD(L)
g_sJ^FTD(L)
```

Status: [PARTIAL]. `SPEC_FTD_NATIVE_BLOCKING_MAP.md` defines the finite-volume
native blocking contract. The Gaussian b=2 tuple flow is now closed by
`DERIV_FTD_NATIVE_BARE_FLOW.md`, `DERIV_FTD_NATIVE_CURRENT_FLOW.md`, and
`DERIV_FTD_NATIVE_RESPONSE_FLOW.md`:

```text
(C_L, K_T, Z_j, g_sJ)(b=2) = (1, 1, 1, 1)
```

Nonlinear state-history flow remains open.

`DERIV_FTD_NATIVE_ENGINE_HISTORY_FLOW.md` connects reaction-only
`RenderBridge::tick()` histories to the dual-cell continuity ledger.
`DERIV_FTD_NATIVE_ENGINE_TRANSPORT_FLOW.md` connects face-neighbor movement
ticks to the same ledger. Diagonal/Moore routing and mixed movement/reaction
history extraction remain open.

### Gate 5: Ward/projection identities

For the native branch, prove or test:

```text
Delta_t rho + div j = S_reaction
div J_L = rho
div J_T = 0
P_T grad chi = 0
```

For the projected-QED branch, additionally require:

```text
q_mu Pi_mu_nu(q) = 0
contact/seagull terms included
renormalized current conserved
```

Status: [PARTIAL]. Native movement continuity is measured; reaction toggles and
projected-QED Ward identities remain gate conditions.

### Gate 6: Matter sector

The bridge must explicitly choose the matter level:

| Level | Matter object | Claim level |
|---|---|---|
| Native | signed manifestation worldlines | FTD-native transport EFT |
| Scalar completion | complex scalar with projected U(1) links | test EFT, not electron QED |
| Dirac completion | projected lattice Dirac matter | QED-facing selected completion |
| SM completion | chiral gauge matter, Higgs/Yukawa, anomalies | separate high gate |

Required output:

```text
matter representation
charge/current definition
mass or gap term status
statistics/spin status
doubler handling if Dirac
```

Status: [OPEN] beyond native signed transport.

### Gate 7: Matching and observables

Native observables must be defined before any external comparison:

```text
C_L^FTD      static source/flux response
K_T^FTD      transverse stiffness and dispersion
Z_j^FTD      signed-current normalization
g_sJ^FTD     source/flux vertex
W_18         local Green geometry
```

External QED observables are allowed only after the native contract is fixed:

```text
static Coulomb coefficient
Thomson scattering amplitude
renormalized transverse kinetic coefficient
running coupling in a declared scheme
```

Required output:

```text
renormalization condition
counterterm policy
regulator family
uncertainty budget
comparison ledger
```

Status: [OPEN] for physical alpha; [PARTIAL] for native response tuple.

---

## Branch policy

### Branch A: native source/flux EFT

This is the active bridge branch.

```text
s       -> signed source
J       -> physical flux
J_L     -> constrained source response
J_T     -> propagating transverse flux
```

Acceptance criterion:

```text
produce a closed source-coupled Wilsonian EFT with native observables and RG
flow, without QED alpha as an input or target.
```

### Branch B: projected QED-like EFT

This is a later comparison branch.

```text
J_T = P_T A
A ~ A + grad chi
j_T couples to A_T
matter representation selected
```

Acceptance criterion:

```text
derive or explicitly ledger every extra QED-facing selection:
matter, regulator, counterterms, charge normalization, and alpha observable.
```

### Branch C: Standard Model EFT

This branch is not active until Branch A is closed and Branch B has a
disciplined matter/gauge completion.

Required additional gates:

```text
nonabelian gauge sectors
chiral fermions
anomaly cancellation
Higgs/Yukawa sector
mass/generation structure
SMEFT operator basis
```

---

## Minimum viable real EFT

FTD becomes a real native EFT when the following document set exists and is
internally consistent:

```text
1. State/flux field dictionary with scaling dimensions.
2. Source-coupled action, transfer matrix, or history measure.
3. Complete low-dimension operator basis under FTD symmetries.
4. Fixed blocking map and native RG flow.
5. Ward/projection identities for the chosen variables.
6. Native response tuple with uncertainties and scheme ledger.
```

The first publishable claim should then be:

```text
FTD defines a native source/flux effective field theory with measured
long-distance Coulomb-like response, transverse wave modes, and native RG flow.
```

The claim should not be:

```text
FTD derives QED alpha.
```

That stronger claim remains [OPEN] until the projected-QED matching branch
passes its own gates.

---

## Immediate work queue

1. Write the scaling section for `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`:
   dimensions of `rho`, `J`, `j`, `A`, and source terms.
2. [PARTIAL] Promote the native response tuple into a fixed renormalization
   scheme: finite-volume and zero-mode conventions are fixed; the native
   blocking convention is specified in `SPEC_FTD_NATIVE_BLOCKING_MAP.md` and
   awaits exact dual-cell implementation.
3. [PARTIAL] Draft the native source-coupled generator:
   `DERIV_FTD_NATIVE_LINEAR_GENERATOR.md` closes the linear constrained-flux
   sector; the nonlinear state-history ensemble remains open.
4. Extend the operator basis to include all source/flux operators through the
   chosen dimension, including `s div J`, `rho^2`, `j_T A_T`, and reaction
   source operators.
5. Add a reaction-aware continuity/Ward ledger:
   `Delta_t rho + div j = S_reaction` for every toggle class.

This queue keeps the bridge honest: it builds the EFT first, then asks what
external physics it resembles.
