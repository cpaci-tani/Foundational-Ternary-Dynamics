# State/Flux to EFT Dictionary

**Status:** [PARTIAL] bridge result; gauge redundancy not yet derived. **Scaling dimensions frozen under FTD-0059 `a_phys ≡ ℓ_P` calibration — see § "Frozen scaling contract" below (Gate 1 closure).**
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

## Frozen scaling contract (P1.1 closure)

This section upgrades the qualitative "Continuum scaling contract" above to a
**frozen** engineering-dimension contract, suitable as Gate 1 of the bridge
program. It is the reference for every downstream EFT derivation, RG flow
measurement, and observable normalization.

### Calibration anchor

Under the no-go theorem FTD-0059, FTD cannot derive a physical length from
Axiom-Zero invariants alone. The project therefore fixes once and for all:

```text
a_phys        ≡ ℓ_P                         (one voxel = one Planck length)
τ_phys        ≡ ℓ_P / (√3 · c) = t_P/√3      (from physical c = c_lat·a_phys/τ_phys)
c_lat         = 1/√3  (lattice units, CFL Courant number)
```

All dimensional claims below are conditional on this calibration. Dimensionless
ratios (α-like, mass ratios, mixing angles) are calibration-independent and
constitute the falsifiable spine.

### Engineering dimensions (SI + natural, with `a_phys = ℓ_P`)

| Lattice object | Continuum field | SI dimensions | Natural (ℏ=c=1) mass dimension |
|---|---|---|---|
| `s(x,t) ∈ {-1,0,+1}` | `ρ(x,t)` (signed source density) | `[L⁻³]` | 3 |
| `J_lattice(x,t) ∈ ℝ³` (per-site Vec3) | `J(x,t)` (signed flux density, 2-form) | `[L⁻²]` | 2 |
| `j_lattice(x,t) ∈ ℝ³` (per-link-tick) | `j(x,t)` (signed transport current density) | `[L⁻² T⁻¹]` | 3 |
| `A(x,t)` (auxiliary transverse potential, `J_T = P_T A`) | `A(x,t)` | `[L⁻¹]` | 1 |
| lattice ∇ | continuum ∇ | `[L⁻¹]` | 1 |
| lattice Δ_t | continuum ∂_t | `[T⁻¹]` | 1 |

All lattice quantities are **dimensionless integers or dimensionless reals**
in the engine; the continuum field acquires its dimension through the
Z-factor convention below.

### Z-factor scaling laws

Each lattice → continuum map is

```text
X_phys(x_phys, t_phys) = Z_X(a) · X_lattice(x, t),        x_phys = a x,  t_phys = τ t.
```

Under `a = ℓ_P` and `τ = √3 ℓ_P / c`:

```text
Z_ρ(a) = 1 / a³                                [L⁻³]
Z_J(a) = 1 / a²                                [L⁻²]
Z_j(a) = 1 / (a² τ)    =  c / (√3 a³)          [L⁻² T⁻¹]
Z_A(a) = 1 / a                                 [L⁻¹]
```

These are the **native** Z-factors — equivalent to the `Z_Q = 1` native-unit
convention of § "Density and current" above. They preserve finite-volume
Gauss:

```text
div_phys J_phys(x)  =  (1/a) div_lattice · (J_lattice / a²)  =  J_lattice / a³ · (div_lattice / 1)
                   =  s_lattice / a³            (by Gauss on the lattice)
                   =  ρ_phys(x)                 ✓
```

and reaction-transport continuity:

```text
∂_t ρ_phys + div j_phys
  = (1/τ) Δ_t (s / a³) + (1/a) div_lattice · (j_lattice / (a² τ))
  = (Δ_t s + div_lattice j_lattice) / (a³ τ)
  = S_reaction_lattice / (a³ τ)
  = S_R_phys(x)                                 ✓
```

### QED-facing normalization (Branch B only)

The native contract above is complete in the Z_Q = 1 convention. For the
QED-facing comparison branch, introduce a single **source-normalization
constant**:

```text
Z_Q = e_phys        (Coulombs per unit signed source)
```

Every continuum field rescales as:

```text
ρ_phys^QED = Z_Q · ρ_phys^native
J_phys^QED = Z_Q · J_phys^native
j_phys^QED = Z_Q · j_phys^native
A_phys^QED = Z_A^QED · A_phys^native           (canonical-field rescaling)
```

`Z_A^QED` is fixed by canonical field normalization in the chosen projected
action — **not** by matching to the α target. Under the current ledger,
`e_phys` and `Z_A^QED` are both [OPEN] matching parameters (Gate 6 + Gate 7),
and the identification `e_phys² = 1 / x_+` is [STRONGLY MOTIVATED CONJECTURE]
tagged at FTD-0001–FTD-0014.

### Zero-mode conventions

On a finite periodic lattice of extent `L`:

| Field | k = 0 mode | Convention |
|---|---|---|
| ρ (source) | `Q_total = ∑_x s(x)` | Physical configurations fix `Q_total` explicitly. Neutral systems have `Q_total = 0`; charged probes have `Q_total ∈ ℤ ≠ 0` (a conserved selection sector). |
| J (flux) | uniform flux mode (constant J across the torus) | **Zero at k=0** under dual-cell Gauss projection when `Q_total = 0`. When `Q_total ≠ 0`, the k=0 mode carries a net boundary flux; for periodic topology this mode is unobservable (no boundary) and is projected out — the physical projector is `J_k for k ≠ 0`. |
| j (current) | net loop current around the torus | Conserved integer (flux quantum); fixed by initial conditions. Zero for systems with no net current. |
| A (auxiliary) | `A(k=0)` | Pure gauge degree of freedom in the transverse representation `J_T = P_T A`. **Fixed by gauge choice: A(k=0) = 0** (equivalent to Coulomb-gauge zero-mode fix). |
| S_R (reaction) | `∑_x S_R(x,t)` | Must vanish instantaneously when integrated against global charge conservation; non-zero values signal a charge-violating toggle (pair production is the canonical example and increments/decrements `Q_total` in pairs). |

### Boundary and continuum-limit protocol

All EFT statements are made at **fixed `a = ℓ_P`** with finite `L` unless
explicitly noted.

**Standard protocol for an EFT observable `O`:**

1. Compute `O(L, a = ℓ_P)` on the engine at `L ∈ {32, 64, 128, 256, ...}`.
2. Extract the `L → ∞` limit by finite-size scaling at fixed `a`.
3. Report the resulting `O_∞(a = ℓ_P)` as the physical prediction.
4. **Do not** take `a → 0` at fixed physical volume — that limit is not
   well-posed under FTD-0059. The calibration sets `a = ℓ_P` once and for all.

**Equivalence class.** Two continuum fields `X, X'` are equivalent iff they
agree on all dimensionless ratios and differ only by a constant `Z` factor
consistent with the calibration. Different choices of finite-volume scheme
(PBC, anti-PBC, twisted BCs) define different equivalence classes for
`L < ∞`; the physical prediction is the common `L → ∞` limit across schemes.

### Symmetry action on the frozen variables

For completeness, the symmetry generators act on the frozen continuum fields
as:

| Symmetry | ρ | J | j | A |
|---|---|---|---|---|
| Cubic translation `T_a` | `ρ(x - a)` | `J(x - a)` | `j(x - a)` | `A(x - a)` |
| Cubic rotation `R ∈ O_h` | `ρ(R⁻¹ x)` (scalar) | `R J(R⁻¹ x)` (vector) | `R j(R⁻¹ x)` (vector) | `R A(R⁻¹ x)` (vector) |
| Charge conjugation `C` | `-ρ` | `-J` | `-j` | `-A` |
| Parity `P` (= -I ∈ O_h) | `ρ(-x)` | `-J(-x)` | `-j(-x)` | `-A(-x)` |
| Time reversal `T` (arrow selection) | `ρ(x, -t)` | `J(x, -t)` | `-j(x, -t)` | `A(x, -t)` or `-A` depending on gauge |

These are the structural constraints for Gate 3 (operator basis enumeration).

### Epistemic tag

| Piece | Tag | Justification |
|---|---|---|
| `a_phys ≡ ℓ_P`, `τ_phys ≡ √3 ℓ_P / c` | [THEOREM] (calibration-enforced) | FTD-0059 no-go + FTD-0030/0041 calibration |
| Engineering dimensions of ρ, J, j, A per table | [THEOREM] (under calibration) | Gauss + continuity consistency, § above |
| Z-factor formulas `Z_ρ = 1/a³`, `Z_J = 1/a²`, `Z_j = 1/(a²τ)`, `Z_A = 1/a` | [THEOREM] (under calibration + native convention) | Dimensional closure + finite-volume Gauss preservation |
| k = 0 gauge-fix `A(k=0) = 0` | [SELECTION] | Coulomb-gauge equivalent; a convention, not forced |
| `Q_total` as selection-sector label | [THEOREM] | Conserved by construction on periodic lattice |
| `a → 0` continuum limit well-posed | [CLOSED NEGATIVE] | FTD-0059 |
| `L → ∞` at fixed `a = ℓ_P` well-posed | [THEOREM] | Standard finite-size-scaling argument |
| `e_phys² = 1 / x_+` (QED-facing matching) | [STRONGLY MOTIVATED CONJECTURE] | Unchanged — not a consequence of this contract |

### What Gate 1 now provides downstream

Every subsequent bridge deliverable can now cite this frozen contract rather
than redefining dimensions. Specifically:

- **Gate 2** (native action/measure) can write `Z[J_source] = ∫ Ds exp(-S[s,J])`
  with dimensions tracked explicitly.
- **Gate 3** (operator basis) can enumerate operators by their canonical
  dimension under this contract (e.g. `ρ²` is `[L⁻⁶]`, dimension 6 — marginal
  in 3+0 Euclidean; `s div J` is `[L⁻⁵]`, relevant).
- **Gate 4** (blocking + RG) can write β-functions for
  `(C_L, K_T, Z_j, g_sJ)` as dimensionless couplings with explicit
  lattice-spacing dependence absorbed into the Z-factors.
- **Gate 5** (Ward identities) can test `∂_t ρ + div j = S_R` as a literal
  equation among frozen-dimension fields.
- **Gate 6** (matter sector) can assign canonical dimensions to any matter
  field under the same calibration.
- **Gate 7** (observables) can report every response coefficient in
  calibration-fixed physical units with explicit finite-size scaling.

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

> FTD's state/flux variables naturally give a charge-like source coupled to a physical vector flux. They do not force compact U(1) gauge redundancy.

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
State-to-field dictionary: PARTIAL  →  CLOSED for Gate 1 (scaling dimensions)
                                        (P1.1 of the EFT roadmap)
```

The derived part is:

```text
s -> signed scalar source                               [ρ] = L⁻³ (frozen)
J -> selected spatial vector field                      [J] = L⁻² (frozen)
j -> signed transport current                           [j] = L⁻² T⁻¹ (frozen)
A -> auxiliary transverse potential (J_T = P_T A)       [A] = L⁻¹ (frozen)
div J -> scalar source operator
s div J -> lowest-derivative source-vector coupling     [s ∂ J] = L⁻⁵ (relevant)
```

**P1.1 (Gate 1) closure adds:** explicit Z-factor scaling laws
`Z_ρ = 1/a³`, `Z_J = 1/a²`, `Z_j = 1/(a²τ)`, `Z_A = 1/a` under the calibration
`a ≡ ℓ_P`, `τ ≡ √3 ℓ_P / c`; k=0-mode conventions for ρ, J, j, A, S_R;
and the finite-L protocol (all claims at `a = ℓ_P`, `L → ∞` for physical
limits; `a → 0` limit [CLOSED NEGATIVE] by FTD-0059).

The revised unresolved part is:

```text
J_T = P_T J -> auxiliary U(1) potential class [A] with A ~ A + grad chi
```

The next target is therefore no longer "make microscopic J a gauge field." It is:

```text
derive the projected EFT variables, matter representation,
local coupling, regulator, counterterms, and alpha observable.
```

**Roadmap forward** (see `PLAN_EFT_COMPLETION_ROADMAP.md` when created): Gate 1
is now closed; the next sequenced deliverable is P1.2 (diagonal/Moore routing
in the engine-history transport ledger), followed by P1.3 (multi-tick
accumulated history + first non-Gaussian flow data).

See `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` for the parallel gauge-class
derivation.
