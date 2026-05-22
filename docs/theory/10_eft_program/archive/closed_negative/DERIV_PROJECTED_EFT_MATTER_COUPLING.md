# Projected EFT Matter Coupling

**Date:** 2026-04-22
**Status:** [PARTIAL] bridge result; Dirac/QED matter remains selected
**Purpose:** Specify the matter and local-coupling step for the emergent-U(1) projected-flux branch without using the alpha target as input.

---

## Executive result

The projected EFT can now be stated without pretending that microscopic `J` is a gauge potential:

```text
native FTD variables:
    s(x,t) in {-1,0,+1}
    J_i(x,t) physical vector flux

coarse source variables:
    rho(x,t) = s(x,t)
    j_i(x,t) = signed transport current of manifested s events

projected field variables:
    J = J_L[rho] + J_T
    J_T = P_T A
    A ~ A + grad chi

local EFT coupling:
    S_int = sum_x,t [rho phi - j_T · A_T]
```

In temporal/Coulomb gauge language, the longitudinal sector is constrained by `rho`, while the transverse sector couples to the transverse current `j_T`.

This is the natural projected-EFT bridge:

```text
rho fixes Coulomb field
j_T couples to radiation field
matter representation still selected
```

The matter representation is not uniquely forced yet. The honest ranking is:

1. **Signed source/worldline matter:** supported directly by ternary `s`.
2. **Complex wavefunction matter:** selected by transverse-flux complexification.
3. **Dirac matter:** best QED-facing completion, supported by BCC-pair/spinor arguments but not fully derived dynamically.
4. **Scalar matter:** allowed as a test completion, but the natural Structure-2 scalar case failed to reproduce the Structure-1 ppb alpha closure.

---

## Native matter: signed manifestation events

The ternary state gives a theorem-level signed source alphabet:

```text
s = +1    positive manifestation
s =  0    void
s = -1    negative manifestation
```

Charge conjugation is exact:

```text
C: s -> -s.
```

At the native level this gives:

| Object | FTD meaning | Status |
|---|---|---|
| `rho = s` | signed source density | [THEOREM] as signed source |
| `rho = electric charge density` | electromagnetic interpretation | [SELECTION] |
| `+1,-1` charge units | integer charge alphabet | [THEOREM] internally |
| physical electron charge `e` | normalization of the unit | [OPEN] |

So FTD does have quantized signed source units. What remains open is the physical normalization:

```text
unit source -> electron charge e
```

That normalization is part of the alpha observable problem, not solved by the alphabet alone.

---

## Coarse current from state transport

A local EFT coupling to a transverse field requires current, not only charge density.

Define the coarse current `j_i(x,t)` as the signed transport of manifested state across lattice links between ticks. For a movement/update step:

```text
s(x,t) -> s(x+e_i,t+1)
```

contributes:

```text
j_i(x+e_i/2,t+1/2) += s(x,t).
```

The coarse continuity equation is then:

```text
Delta_t rho + div j = 0
```

provided state updates conserve total signed state except for balanced void events:

```text
0 -> (+1) + (-1).
```

Status:

- current as signed state transport: **[DEFINITION]**
- continuity under charge-conserving moves and balanced pair events: **[THEOREM]** given those update rules
- full engine-wide continuity proof for every toggle: **[OPEN]**

This is the matter-side object needed by the projected EFT.

---

## Projected coupling

The emergent-U(1) projection separates:

```text
J = J_L[rho] + J_T
```

with `J_L` fixed by `rho` and `J_T` carrying two transverse DoF.

In the projected EFT, represent the transverse sector by an auxiliary potential:

```text
J_T = P_T A
A ~ A + grad chi.
```

Then the minimal local coupling to the transverse field is:

```text
S_int,T = - sum_x,t j_T(x,t) · A_T(x,t)
```

where:

```text
j_T = P_T j
A_T = P_T A.
```

The longitudinal/source coupling is not a radiative gauge coupling. It is the constrained Coulomb sector:

```text
S_Coulomb = 1/2 sum rho (-Delta)^-1 rho
```

up to lattice normalization and boundary convention.

Therefore the old native term:

```text
s div J
```

should be read as source/constraint coupling, not as the full QED vertex. The QED-like vertex appears only after:

```text
state transport -> current j
transverse projection -> j_T
auxiliary potential -> A_T
```

Status:

- Coulomb/source sector from `rho` and Gauss projection: **[PARTIAL]**
- transverse current coupling `j_T · A_T`: **[SELECTION]** as the minimal projected EFT coupling
- full covariant QED vertex `-i e gamma_mu A_mu`: **[SELECTION]** pending Dirac matter derivation and normalization

---

## Matter representation choices

### Option 1: Worldline/source matter

Use only `rho` and `j` from manifested state transport.

Advantages:

- Closest to native FTD.
- Charge quantization is direct.
- No spinor or scalar field assumption.

Cost:

- Gives classical charged sources, not quantum field matter.
- Loop corrections to alpha are not defined until a path integral/statistical ensemble over currents is specified.

Status: **supported native coarse matter**, not enough for QED loops.

### Option 2: Scalar matter

Represent charged excitations by a complex scalar field with Peierls link phases.

Advantages:

- Simple Ward-valid gauge completion.
- Easy to test numerically.

Cost:

- Not naturally tied to spin-1/2 electron matter.
- Natural Structure-2 scalar completions S2-A through S2-E failed the ppb alpha closure threshold.

Status: **allowed test completion, negative for the tested alpha bridge**.

### Option 3: Dirac matter

Represent charged excitations by a Dirac spinor built from:

```text
transverse complexification  psi = J_1 + i J_2
BCC complementary pairs      4 spinor components
void events                  (+1,-1) particle/antiparticle structure
```

Advantages:

- Matches electron-like spin-1/2 matter.
- Compatible with existing Dirac, singlet, and QM-emergence documents.
- Gives standard QED matter loops once the projected U(1) coupling is accepted.

Cost:

- The Dirac operator is still selected by factorization of the second-order wave equation.
- The mass relation is not fully derived.
- Doubler handling and exact lattice fermion discretization remain selected.

Status: **best QED-facing completion, but still [SELECTION]**.

---

## Minimal projected-QED candidate

If the goal is a physical electromagnetic EFT, the least-contrived candidate is:

```text
field sector:
    A_T auxiliary transverse potential
    A ~ A + grad chi

source sector:
    rho = s
    j = signed state-transport current

matter sector:
    psi = complexified transverse flux / BCC-pair spinor
    Dirac operator selected by first-order factorization

coupling:
    D_mu psi = (partial_mu + i q e A_mu) psi
    q in Z from ternary charge units
```

Epistemic tags:

| Step | Status |
|---|---|
| `rho = s` signed source | [THEOREM] internally |
| `j` from signed state transport | [DEFINITION] |
| continuity for charge-conserving updates | [THEOREM] given update restrictions |
| transverse projection and auxiliary `A` redundancy | [THEOREM]/[PARTIAL] |
| `psi = J_1 + i J_2` | [SELECTION] |
| four BCC pairs as Dirac components | [SELECTION] |
| covariant derivative coupling | [SELECTION] imported from gauge-EFT consistency |
| physical charge normalization `e^2 = alpha` | closed negative under current projected action |
| physical alpha observable `x_+ = 1/alpha` | arithmetic/conjectural comparison, not derived |

This is a legitimate fixed candidate for a QED-facing comparison, but it is not a derivation of physical QED from FTD. The active replacement is FTD-native electrodynamics.

---

## What this closes

This closes the next bridge target at the level of a disciplined candidate:

```text
matter representation and local coupling: PARTIAL
```

Resolved:

- Native matter is signed source/worldline matter.
- Transverse EFT coupling must use current `j_T`, not only `rho`.
- Scalar matter is not the preferred physical bridge after Structure-2.
- Dirac matter is the best QED-facing completion.

Still open:

- derive Dirac first-order dynamics directly from FTD evolution
- derive the mass relation
- choose/lift lattice fermion doublers from FTD principles
- derive the normalization `e^2 = alpha = 1/x_+`
- select the regulator/counterterm prescription for the projected EFT

---

## Next proof obligation

The next load-bearing bridge target is:

```text
derive or select the projected Dirac operator and charge normalization
without using the alpha residual as a target.
```

Only after that should a new fixed loop calculation be run.

---

## 2026-04-22 projected-Dirac follow-up

`archive/closed_negative/DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` records the next bridge span:

```text
projected Dirac operator   partial candidate
charge normalization       still open
```

Its main result is that ternary charge gives integer `q`, but not the magnitude of `e0`. Later audits close the stiffness, response-eigenvalue, and source-current normalization routes negative under the current projected action, so the current bridge endpoint is arithmetic-only unless a new normalization theorem is supplied.
