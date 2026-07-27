# FTD-0584 — Configuration-Space Carrier Necessity Gate v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date locked:** 2026-07-26  
**Program:** FTD face-flux mobile matter, observer-only successor to FTD-0583  
**Production status:** frozen; no tick, toggle, default, scenario, or ontology change

## 1. Question

After the matched real face/edge variables are restricted by a fixed ternary
source, Gauss law, and fixed global plane flux, can the existing frozen state
space carry a localized topological matter sector? If it cannot, what is the
minimum additional mathematical structure required for each standard defect
class, and which same-variable nonlinear escape remains logically open?

This gate distinguishes three claims that must not be conflated:

1. topology of a fixed-source continuous field fibre;
2. disconnected ternary snapshot labels;
3. an invariant of the actual production transition graph.

## 2. Frozen variables and source provenance

The registered field variables are ordinary real arrays. The relevant frozen
source hashes at lock are:

| source | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/eft/matched_gauss_transport.h` | `1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033` |
| `engine/src/eft/matched_gauss_transport.cpp` | `12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028` |
| `engine/include/ftd/eft/conserved_charge_basis.h` | `556D949304C5051197BAB171EF7925C384B0855D093B141DA3C811D8C8587F83` |
| `engine/src/eft/conserved_charge_basis.cpp` | `1BA6989AFF6D73D172CAF85C9FA1D2F3A0A589B2274226FC7E248937CC89D7B5` |
| `engine/include/ftd/eft/native_field_discrete_action.h` | `85B8BD24D10CCAC8D79F49B64DE97C9C861E34C79556C6757388668BDD5481DF` |
| `engine/src/eft/native_field_discrete_action.cpp` | `EBDB91ED0A4C10647E0A698D707A72B2BC1A69F87EAAFA8B63C33234378D0077` |

The tested configuration fibre is

\[
 \mathcal F_{s,h}=\{(E,B,J,W,\ldots)\in\mathbb R^M:
 A F=b(s),\ H F=h\},
\]

where `A` contains only frozen linear matched operators and `H` fixes the
global harmonic/plane-flux coordinates. No normalized direction, compact
phase, branch integer, singular puncture, or fitted function of `J` may be
introduced.

## 3. Exact theorem gates

### G1 — finite periodic affine-fibre theorem

For every nonempty fixed-source/fixed-harmonic fibre, prove constructively that

\[
 H_t(F)=F_0+t(F-F_0),\qquad 0\le t\le1,
\]

stays in the same fibre. Therefore the fibre is convex and contractible, and
every continuous integer-valued observable is constant on it.

The native observer must exercise `L in {3,4,5,8}`, both dipole signs, three
dipole axes, two independent divergence-free deformations, four fixed
harmonic backgrounds, and `t in {0,1/4,1/2,3/4,1}`. It must measure Gauss,
harmonic-flux, affine, and energy-polynomial residuals below `1e-12`.

### G2 — uncontained finite-energy theorem

Prove separately that the finitely supported real cochains, their `l2`
completion, and any nonempty affine fixed-source fibre of those linear spaces
remain contractible by the same homotopy. This is the registered interpretation
of a finite but uncontained excitation; no container wall or periodic topology
may be used in this proof.

### G3 — snapshot versus dynamical conservation

Prove that

\[
 \{-1,0,+1\}^{N}\times\mathbb R^M
\]

is a disjoint union of contractible continuous fibres. Record that the ternary
labels are disconnected only as a snapshot topology. Reproduce the exact
registered transition-matrix rank/nullity from FTD-0421 for the feature basis
`(occupancy, signed state, chirality, signed-state*chirality)`. The allowed
conclusion is restricted to that registered additive basis; no universal
no-invariant claim is licensed.

### G4 — vacuum-manifold/defect classification

For three spatial dimensions, register the exact minimum topological data:

| topology | carrier geometry | minimum vacuum data |
|---|---|---|
| `pi_0(M)` | wall | disconnected vacuum manifold |
| `pi_1(M)` | line/vortex | noncontractible loop, e.g. an `S1` phase |
| `pi_2(M)` | point/hedgehog | noncontractible two-sphere, e.g. fixed-magnitude `S2` direction at infinity |
| `pi_3(M)` | texture | noncontractible three-cycle plus an energetic size stabilizer |

Show that the frozen free-field vacuum is the zero configuration, so its
vacuum manifold is a point and all four groups are trivial. A shell degree
computed from `J/|J|` does not define a protected sector while `J=0` is allowed.

### G5 — energetic escape and Derrick scaling

For a static three-dimensional same-variable continuum energy with a
two-derivative term and nonnegative onsite potential, derive under size scaling

\[
 E(R)=R E_2+R^3E_0,
\]

so shrinking removes the energy and no finite-radius static soliton is
stabilized by those terms alone. Record the minimum common escapes without
promoting any of them:

- a four-derivative term, contributing `E_4/R`;
- a gauge/compact/singular sector with its own constraints;
- a time-periodic active nonlinear localized mode;
- lattice pinning, which is metastability and must separately pass the mobile
  Peierls gate.

### G6 — compact-link scope

State exactly that compact `U(1)` links add phase/branch structure but do not
automatically produce a conserved electric charge or a stable particle.
Integer magnetic-flux sectors require an admissibility/smoothness restriction
or else can change through a plaquette branch crossing. No compact link is to
be added in this gate.

## 4. Documentation consistency gate

After the blind theorem status is fixed, compare the result with canonical
charge-quantization prose. Any statement that derives conserved topological
electric charge merely from `s in {-1,0,+1}` or from a `Z[i]^2` projection must
be corrected to distinguish a discrete label from a loop, bundle, or
transition-graph invariant. Proven group decompositions and the already
recorded underdetermined alpha-readout verdict must be preserved.

## 5. Verdict map

- `CURRENT_FIXED_SOURCE_FIBRES_CONTRACTIBLE`: G1–G3 pass.
- `CURRENT_VACUUM_HAS_NO_DEFECT_HOMOTOPY`: G4 passes.
- `STATIC_TWO_DERIVATIVE_NONLINEAR_CORE_UNSTABLE`: G5 passes.
- `MINIMUM_ENLARGEMENT_CLASSIFIED_NOT_DERIVED`: G6 passes.
- `INVALID`: any frozen hash changes, any observer alters production state, or
  any theorem claims more than its registered topology/dynamics scope.

The combined negative verdict closes a *topologically protected localized
carrier in the frozen ordinary-real fixed-source variables*. It does not close
all nonlinear time-dependent localized solutions and does not authorize a new
primitive.

## 6. Required artifacts and verification

- observer-only `ConfigurationSpaceCarrierResult` C++ record;
- native CTest with exact/`1e-12` gates and JSON run of record;
- independent symbolic Python proof with no empirical constants;
- theorem, audit, ledger, tracker, indexes, engine spec, and changelog updated
  together;
- golden defaults unchanged and golden CTests passing;
- source hashes rechecked after execution.

No parameter search, physical-constant comparison, scenario qualification,
Lorentz claim, particle claim, or production implementation is permitted.
