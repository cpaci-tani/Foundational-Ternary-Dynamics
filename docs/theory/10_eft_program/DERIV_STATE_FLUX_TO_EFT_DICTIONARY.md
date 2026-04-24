# State/Flux to EFT Dictionary

**Date:** 2026-04-22
**Status:** [PARTIAL] bridge result; gauge redundancy not yet derived
**Purpose:** Extract the minimal EFT dictionary from FTD state/flux variables without using the alpha target as input.

---

## Executive result

The FTD variables support a clean long-wavelength **source-coupled vector-field dictionary**:

```text
s(x) in {-1,0,+1}          -> signed scalar source / charge-like density
J_i(x) in R^3              -> spatial vector field on the cubic graph
div J                      -> local scalar sourced by s
s * div J                  -> minimal O_h-invariant source-vector coupling
```

What is not derived microscopically is the stronger statement:

```text
J_i is a U(1) gauge potential A_i with a true gauge redundancy
J -> J + grad chi.
```

That stronger statement should now be read as an **emergent EFT representation**, not as a primitive ontology. The current FTD action-like structure treats `J` as a physical flux/vector field. A physical vector field is not automatically a gauge potential.

This is good news epistemically: the first bridge span is now localized. The missing theorem is not "compute more alpha loops." It is:

> derive U(1) as an emergent redundancy of the transverse projected flux description, or explicitly add/select the gauge projection as part of the EFT matching rule.

---

## Inputs

From Axiom Zero:

| Input | Status | Notes |
|---|---|---|
| Cubic position graph `x in Z^3` | [AXIOM] | Undefined-boundary cubic graph |
| Ternary state `s in {-1,0,+1}` | [AXIOM] | Balanced signed state |
| Charge conjugation `s -> -s` | [THEOREM] | Automorphism of the ternary state set |
| Continuous vector field `J in R^3` | [SELECTION] | Minimal continuous extension compatible with `O_h` |
| Coupling `s div J` | [SELECTION] | Simplest local scalar coupling between `s` and vector `J` |

From engine/EFT audits:

| Constraint | Source | Consequence |
|---|---|---|
| Engine kinetic operator is `(SC+FCC)/2` | FTD-0050 / Link 8 | Engine-native EFT does not automatically access BCC arithmetic |
| BCC one-loop residual is regulator-specific | FTD-0056 | Counterterm/regulator choice cannot be skipped |
| Structure-2 scalar gauge completion fails | FTD-0058 | Natural scalar U(1) completion does not reproduce Structure-1 ppb closure |

---

## Minimal dictionary

### 1. Ternary state as signed source

The state set has an exact involution:

```text
C: s -> -s
```

with `0` fixed and `+1`, `-1` exchanged. Therefore `s` can consistently act as a signed scalar source.

Status:

- `s` as a signed scalar: **[THEOREM]**
- `s` as electric charge: **[SELECTION]**

Reason:

The sign structure is intrinsic. The identification with physical electric charge requires choosing the electromagnetic EFT sector.

### 2. Flux as spatial vector field

The minimal continuous extension in Axiom Zero assigns:

```text
J(x) = (J_x, J_y, J_z) in R^3
```

at each site or local cell of the cubic graph.

Status:

- `J` as an `O_h` vector field: **[SELECTION]**
- `J` as a U(1) gauge potential: **[OPEN]**

Reason:

A vector field is the simplest continuous object that transforms correctly under cubic rotations. But gauge potential status requires redundancy, not just vector transformation.

### 3. Divergence as local scalar

Given a vector field on a cubic graph, the centered lattice divergence

```text
div J(x) = sum_i [J_i(x+e_i) - J_i(x-e_i)] / 2
```

is an `O_h` scalar.

Status:

- divergence operator given `J`: **[THEOREM]**
- physical interpretation as Gauss-law charge density: **[SELECTION]**

Reason:

The operator is forced once `J` is chosen. Its interpretation as electromagnetic Gauss law depends on the gauge-field dictionary.

### 4. Source-vector coupling

The local scalar coupling

```text
L_int = - g_c s(x) div J(x)
```

is the lowest-derivative local scalar coupling between a ternary scalar source and a vector field.

Status:

- lowest-derivative `O_h` scalar coupling: **[SELECTION]**
- QED minimal coupling: **[OPEN]**

Reason:

QED minimal coupling is normally phase/gauge-covariant coupling of matter to a connection. `s div J` is source-vector coupling unless the gauge dictionary has already been established.

---

## Continuum scaling contract

To turn the dictionary into an EFT, introduce an explicit lattice spacing `a`
and tick spacing `tau`. Lattice coordinates are:

```text
x_phys = a x
t_phys = tau t
```

The native speed fixes only the ratio:

```text
c_FTD = a / tau * 1/sqrt(3)
```

Choosing physical units for `a` is a separate dimensional-conversion problem.
It is not a coupling derivation.

### Density and current

The signed state is an integer cell charge:

```text
Q_cell(x,t) = s(x,t).
```

The continuum source density is therefore:

```text
rho_a(x_phys,t_phys) = Z_Q(a) s(x,t) / a^3.
```

For the native theory:

```text
Z_Q(a) = 1
```

in integer source units. For a QED-facing theory:

```text
Z_Q(a) = e_phys
```

would be a matching normalization, not a theorem from the alphabet alone.

A transport step across one lattice link defines current by charge per area per
time:

```text
j_a = Z_Q(a) j_lattice / (a^2 tau).
```

Native movement audits fix:

```text
Delta_t s + div_lattice j_lattice = 0
```

for signed transport moves. The full engine has the reaction-transport form:

```text
Delta_t s + div_lattice j_lattice = S_reaction.
```

Status:

- `rho_a = s/a^3` in native source units: **[DEFINITION]**
- `j_a = j/(a^2 tau)` in native source units: **[DEFINITION]**
- physical electric charge normalization: **[OPEN]**
- reaction-aware continuity for all toggles: **[OPEN]**

### Flux scaling

The Gauss constraint should have a finite continuum limit:

```text
div_phys J_phys = rho_phys.
```

Since:

```text
div_phys = a^-1 div_lattice
rho_phys = Z_Q s / a^3
```

the flux scaling compatible with Gauss is:

```text
J_phys = Z_Q J_lattice / a^2.
```

In native source units:

```text
J_phys = J_lattice / a^2.
```

This is the finite-volume interpretation of `J` as flux through a cell face:
charge per area. It also matches the dual-cell Gauss reading where sources live
inside cells and flux lives on the boundary.

Status:

- flux as cell-boundary density: **[MEASURED]** by dual-cell Gauss tests
- `J_phys = J/a^2` in native units: **[DEFINITION]**
- exact production storage as face-centered flux: **[OPEN]**

### Auxiliary potential scaling

The auxiliary projected variable is defined only for the transverse
representation:

```text
J_T = P_T A.
```

If `P_T` is dimensionless, then `A` has the same native scaling as `J_T`:

```text
A_phys = Z_Q A_lattice / a^2.
```

If a QED-like gauge potential is introduced through a covariant derivative:

```text
D_mu = partial_mu - i e A_mu
```

then the normalization of `A_mu` changes by canonical field normalization. That
step belongs to the projected-QED matching branch, not the native flux
dictionary.

Status:

- auxiliary `A` for transverse flux: **[SELECTION]**
- canonical QED gauge potential normalization: **[OPEN]**

### Native response dimensions

With the above native units, the bare engine response tuple is dimensionless
after factoring the Green function conventions:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
```

These are not physical electromagnetic constants. They are canonical native
normalizations. A non-unit physical electric coupling can enter only through:

```text
source normalization Z_Q
field canonical normalization
renormalized response flow
external matching condition
```

Therefore this dictionary still does not derive:

```text
e_phys^2 = 1/x_+
alpha_QED = 1/x_+
```

Those remain projected-matching problems.

---

## Gauge redundancy test

The decisive question is whether the action is invariant under:

```text
J -> J + grad chi
```

For the source coupling:

```text
div(J + grad chi) = div J + Delta chi
```

so

```text
delta L_int = -g_c s Delta chi.
```

After lattice integration by parts on a finite periodic cell:

```text
sum_x s Delta chi = sum_x chi Delta s.
```

This does not vanish for a general ternary state configuration.

Therefore:

```text
s div J is not gauge-invariant under J -> J + grad chi
```

unless an additional constraint, transformation law, or projection is imposed.

The usual gauge-invariant options would be:

1. Treat `J` as an electric field `E`, with Gauss law `div E = rho`. Then `J` is physical, not a gauge potential.
2. Introduce a gauge potential `A` with field strength terms depending on `curl A`, and couple matter through link phases. This is a new gauge completion, not automatically identical to the original `s div J` coupling.
3. Project out longitudinal modes and retain only gauge-equivalence classes. This is a matching prescription that must be stated explicitly.

This is the first hard bridge result:

> FTD's current state/flux variables naturally give a charge-like source coupled to a physical vector flux. They do not yet force compact U(1) gauge redundancy.

---

## Two possible EFT branches

### Branch A: physical-flux EFT

Interpret `J` as a physical electric/dispositional flux field.

Dictionary:

```text
J      -> E-like or polarization-like field
div J  -> source density / manifestation pressure
s      -> signed source
```

Advantages:

- Closest to Axiom Zero and engine language.
- Does not require hidden gauge redundancy.
- Compatible with engine Gauss-projection language.

Cost:

- Not automatically QED.
- Does not by itself justify `x_+ = 1/alpha`.
- Loop corrections are not standard gauge-theory vacuum polarization unless a gauge completion is added.

### Branch B: gauge-potential EFT

Interpret `J` as `A`, a U(1) gauge connection.

Dictionary:

```text
J_i      -> A_i
curl J   -> B
dot J    -> E in temporal gauge
s        -> charged matter/source
```

Advantages:

- Connects to standard lattice QED.
- Ward identities, seagull terms, and transverse polarization are available.
- Makes Structure-2-style tests meaningful.

Cost:

- Requires a gauge redundancy theorem or an explicit gauge-projection selection.
- Requires matter content and link-coupling prescription.
- Natural scalar completion already failed to reproduce Structure-1 ppb closure.

---

## Consequence for alpha

The state/flux dictionary alone does not identify `x_+` with physical `1/alpha`.

It supports this weaker chain:

```text
ternary signed source + cubic vector flux
    -> source-coupled vector EFT
    -> Coulomb-like long-distance behavior under suitable kinetic operator
```

It does not yet support this stronger chain:

```text
ternary signed source + cubic vector flux
    -> compact U(1) gauge theory with unique matter content
    -> Ward-valid electromagnetic coupling
    -> x_+ = physical 1/alpha
```

The stronger chain remains the FTD-to-EFT matching problem.

---

## What this closes

This document resolves the first bridge target from `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md`:

```text
State-to-field dictionary: PARTIAL
```

The derived part is:

```text
s -> signed scalar source
J -> selected spatial vector field
div J -> scalar source operator
s div J -> lowest-derivative source-vector coupling
```

The revised unresolved part is:

```text
J_T = P_T J -> auxiliary U(1) potential class [A] with A ~ A + grad chi
```

The next target is therefore no longer "make microscopic J a gauge field." It is:

```text
derive the projected EFT variables, matter representation,
local coupling, regulator, counterterms, and alpha observable.
```

See `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`.
