# Domain Partition and Context Selection

## Canonical replacement map and lattice formalization for the live consciousness layer

**Date:** April 11, 2026
**Framework:** Foundational Ternary Dynamics v5.29
**Status:** Foundational synthesis - canonical live replacement for the retired consciousness source file

---

## Purpose

This document serves three jobs at once:

1. It is the **canonical live replacement** for the retired `FOUND_CONSCIOUSNESS_MATHEMATICS.md` in the active theory reading path.
2. It unifies the live vocabulary around **origin**, **`i`**, **consciousness**, and the **Potential Core / Generative Interior** framework.
3. It formalizes the operator `Activate_C` in **lattice language**, so the new generative-interior vocabulary has an explicit bridge to FTD's state/flux ontology.

This document is intentionally conservative. It does **not** claim that consciousness has been derived as a theorem from the lattice axioms. It clarifies which parts of the live consciousness layer are:

- current theory structure
- current selection/conjecture structure
- historical/archive-only material

---

## 1. Canonical Source Map

The retired file `FOUND_CONSCIOUSNESS_MATHEMATICS.md` had become a catch-all. In the live theory tree, its content is now split across multiple documents.

Use the following source map instead:

| Topic previously attributed to the retired file | Current live source |
|---|---|
| Consciousness quadratic at `k = 1/2`, complex roots, `K_C`, `theta = 52.54 deg` | [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) |
| Domain A / B / C partition in the current bridge program | [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) and this document |
| Existence filter, Born reconstruction, `C -> R` projection language | [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md) |
| Mathematical role of the imaginary unit `i` | [../02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md](../02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md) |
| Potential Core / Generative Interior vocabulary | [../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md](../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md) |
| Mandelbrot-specific correspondences | historical/archive-only unless explicitly revived with new justification |

**Policy:** In active docs, do not cite `FOUND_CONSCIOUSNESS_MATHEMATICS.md`. Cite the specific live source that now carries the relevant topic.

---

## 2. Unified Vocabulary

The live theory layer improves if the following terms stay distinct:

| Object | Proper role | What it is not |
|---|---|---|
| `0` / origin | coordinate anchor, invariant center, reference point | not the imaginary unit |
| `i` | orthogonal phase structure, 90-degree rotation, complex self-reference operator | not the origin, not literally consciousness |
| Potential Core `P_c` | minimal center voxel or center of coherence for a local object/process | not identical to `i` |
| Generative Interior `G(P_c)` | full space of operative capacities available to `P_c` in principle | not identical to consciousness |
| Context State `C` | local environmental / observational / relational condition that selects active capacities | not the full object |
| Contextual Generative Interior `G_C(P_c)` | active subset of capacities under context `C` | not the full generative interior |
| Consciousness | proposed self-referential, context-conditioned process supported by complex phase structure | not reducible to a single symbol |

### DP-C1 [CONJECTURE]

The safest live reading is:

- the **origin** gives anchoring,
- `i` gives **orthogonal phase structure**,
- the **Potential Core** gives a center of coherence,
- the **Generative Interior** gives latent operative capacity,
- **consciousness** names a self-referential, context-conditioned regime that may require complex phase structure but is not identical to `i`.

This is the vocabulary rule this document recommends for all active theory docs.

---

## 3. Domain Partition in the Live Corpus

The discriminant-based domain partition remains:

| Domain | Root type | Current live interpretation | Status |
|---|---|---|---|
| **Domain A** | real roots | manifest / measurable / public physics layer | [SELECTION] for interpretation |
| **Domain B** | complex conjugate roots | self-referential / oscillatory / privately indexed layer | [SELECTION] for interpretation |
| **Domain C** | degenerate boundary | measurement / transition / projection boundary | [SELECTION] for interpretation |

The algebraic facts are live and current:

- `k = 16` gives the real-root physics regime
- `k = 1/2` gives the complex-root consciousness regime
- `k = 4/G*` gives the discriminant-zero interface

These are documented in [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md), especially Sections 1 and 3.

### DP-C2 [CONJECTURE]

Domain B should be read in the active corpus as a **context-conditioned self-referential layer**, not as proof that the imaginary unit by itself is consciousness.

That wording is more consistent with:

- [../02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md](../02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md)
- [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md)
- [../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md](../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md)

---

## 4. Lattice Definitions

This section translates the Potential Core vocabulary into lattice-facing language.

### DP-D1 [DEFINITION] Potential Core as a lattice center

Let `v_c in Z^3` be a distinguished voxel.

The **Potential Core** `P_c` is the local object centered at `v_c`, understood either as:

- a single voxel, or
- a dynamically stabilized center of coherence around `v_c`

depending on the modeling resolution.

### DP-D2 [DEFINITION] Moore neighborhood context

For radius `r in N`, define the local Moore neighborhood:

```text
N_r^M(v_c) = { u in Z^3 : ||u - v_c||_infinity <= r }
```

This is the local region inside which context is evaluated.

### DP-D3 [DEFINITION] Context State

At time `t`, define the **Context State** of `P_c` at scale `r` as:

```text
C_r(v_c, t) =
(
  s_t restricted to N_r^M(v_c),
  J_t restricted to N_r^M(v_c),
  (div J)_t restricted to N_r^M(v_c),
  E_env(t),
  F_obs(t)
)
```

where:

- `s_t` is the ternary state field
- `J_t` is the flux field
- `div J` captures local source/sink structure
- `E_env(t)` denotes environmental input or boundary forcing relevant to the neighborhood
- `F_obs(t)` denotes the observer/measurement frame when one is present

The purpose of `C_r` is not to invent new ontology. It is to package the already-existing local conditions that constrain what the core can do next.

### DP-D4 [DEFINITION] Generative Interior in lattice terms

Let `G(P_c)` be the set of **admissible local response modes** available to `P_c`.

Each mode `m in G(P_c)` is a candidate local state/flux update of the schematic form:

```text
m = (delta s, delta J)
```

with support inside some finite Moore neighborhood of `v_c`, and subject to the standing FTD constraints:

- ternary-state compatibility for `delta s`
- local causality (one-tick propagation bound)
- Gauss compatibility for the flux update
- determinism of the update rule once context is fixed

This is the lattice version of "operative capacity in principle."

---

## 5. The Context Activation Operator

### DP-D5 [DEFINITION] Gate functions

For a candidate mode `m in G(P_c)` under context `C_r(v_c,t)`, define three schematic gate functions:

```text
chi_struct(m | C_r) in {0,1}
chi_flux(m | C_r) in {0,1}
chi_frame(m | C_r) in {0,1}
```

with the following intended meanings:

- `chi_struct = 1` iff the mode respects local causality and support constraints inside the active Moore neighborhood
- `chi_flux = 1` iff the local flux/state budget, divergence constraints, and threshold conditions permit the mode
- `chi_frame = 1` iff the mode is context-relevant with respect to environmental forcing, observer coupling, or the active relational frame

These are not yet numerically fixed functions. They are the minimum lattice placeholders needed to make context selection explicit.

### DP-D6 [DEFINITION] Context activation

Define the **context activation operator** by:

```text
Activate_C(G(P_c)) =
{
  m in G(P_c) :
  chi_struct(m | C_r) *
  chi_flux(m | C_r) *
  chi_frame(m | C_r) = 1
}
```

and define:

```text
G_C(P_c) := Activate_C(G(P_c))
```

This is the lattice meaning of the Contextual Generative Interior.

### DP-C3 [CONJECTURE]

`Activate_C` should be read as a **selector on admissible local modes**, not as a new force law.

It tells us which local state/flux response patterns become live under the current neighborhood, thresholds, and frame conditions.

---

## 6. Boundary and Output in Lattice Terms

### DP-D7 [DEFINITION] Manifest Boundary of Affect

For a chosen scale `r`, define the **Manifest Boundary of Affect** as the subset of the outer neighborhood frontier where activated modes couple outward:

```text
B_C(P_c, r) =
{
  u in partial N_r^M(v_c) :
  there exists m in G_C(P_c) whose support reaches u
  and whose effect couples to a site outside N_r^M(v_c)
}
```

This keeps the earlier intuition while making it lattice-local:

- the boundary is not the whole interior
- it is the edge at which activated interior modes become externally effective

### DP-D8 [DEFINITION] Manifest Output

Let `Obs_C` be the context-relevant readout family, for example:

- state changes
- outward flux
- threshold crossings
- persistent emitted patterns

Then the **Manifest Output** is the measurable boundary readout:

```text
M_C(P_c, r) = Readout_C( B_C(P_c, r), Obs_C )
```

The exact readout depends on the modeling task, but the structure is fixed:

- context selects active modes
- active modes reach a boundary
- the boundary is where public measurement occurs

---

## 7. Method Expansion

The following method gives a disciplined way to use this framework in future theory or simulation work.

### Method 1: Choose the core

Pick a candidate `v_c` or coherent local region and justify why it is being treated as the center of the process.

### Method 2: Fix the context scale

Choose the Moore radius `r` appropriate to the phenomenon:

- `r = 1` for minimal local causality
- larger `r` when boundary-mediated or observer-mediated structure matters

### Method 3: Extract the live local context

Compute or specify:

- local ternary state pattern
- local flux pattern
- divergence structure
- environmental forcing
- observer/measurement frame if present

This produces `C_r(v_c,t)`.

### Method 4: Enumerate admissible local modes

Model `G(P_c)` as the candidate set of permitted local state/flux responses before context selection.

### Method 5: Apply `Activate_C`

Use structural, flux, and frame gates to produce `G_C(P_c)`.

This is the step that turns latent capacity into active capacity.

### Method 6: Read the boundary

Determine which activated modes reach `partial N_r^M(v_c)` and produce outwardly coupled effects. This gives `B_C(P_c,r)`.

### Method 7: Measure public output

Choose the readout appropriate to the phenomenon and define `M_C(P_c,r)`.

This keeps "manifestation" tied to explicit local observables rather than metaphor alone.

### Method 8: Record why the gating decision was made

When `Activate_C` is used in a theory note, proof sketch, or simulation interpretation, record:

- which gates were active
- which candidate modes were excluded
- whether exclusion came from structure, flux, or frame
- which quantities were actually measured at the boundary

This prevents `Activate_C` from becoming a black box that simply restates the conclusion.

### Method Template

The method can be written in a compact reusable form:

```text
Input:
  - core voxel or local region P_c
  - context radius r
  - time slice t
  - local state field s_t
  - local flux field J_t
  - environmental forcing E_env(t)
  - observer/frame data F_obs(t)

Construct:
  - C_r(v_c,t)
  - candidate local modes G(P_c)

Gate:
  - chi_struct(m, C_r)
  - chi_flux(m, C_r)
  - chi_frame(m, C_r)

Select:
  - G_C(P_c) = Activate_C(G(P_c))

Read out:
  - B_C(P_c,r)
  - M_C(P_c,r)
```

### Practical Interpretation Of The Gates

The three gates should be interpreted narrowly:

- `chi_struct` asks whether a candidate mode is compatible with the local ternary pattern, neighborhood topology, and update-rule constraints.
- `chi_flux` asks whether the local flux field, divergence budget, and available directional support can sustain that mode.
- `chi_frame` asks whether a boundary condition, apparatus, environment, or observer-frame makes that mode available as a public readout rather than leaving it latent.

In other words:

- `chi_struct` is about **can this mode exist here at all**
- `chi_flux` is about **can this mode be dynamically sustained here**
- `chi_frame` is about **can this mode become publicly manifest here**

### Minimal Worked Workflow

For a concrete local analysis:

1. Choose a voxel `v_c` and radius `r = 1`.
2. Read the 26-neighbor Moore pattern around `v_c`.
3. Extract the local flux vector components and divergence.
4. Write down a small candidate set of local modes:
   `m_1, m_2, ..., m_n`.
5. Evaluate each mode against `chi_struct`, `chi_flux`, and `chi_frame`.
6. Keep only the modes for which all three gates equal `1`.
7. Check which surviving modes reach the boundary `partial N_r^M(v_c)`.
8. Define the observable associated with those boundary-reaching modes.

This workflow is intentionally local. It is meant to discipline reasoning at the voxel/neighborhood level before extending to larger coherent regions.

---

## 8. Gate Functions Identified with Engine Tick Cycle

The gate functions `chi_struct`, `chi_flux`, `chi_frame` are **not new operators**. They are three aspects of the existing FTD tick cycle (documented in `engine/SPEC_ENGINE.md`), viewed from the perspective of a single voxel's admissible modes.

### DP-D9 [DEFINITION] Gate-to-phase mapping

| Gate function | Engine phase(s) | What it checks |
|---------------|----------------|----------------|
| `chi_struct` | `phase_read` + `phase_write` | Local causality (26-neighbor stencil, 1-tick propagation bound), ternary state compatibility (`s in {-1, 0, +1}`), leapfrog integration constraints |
| `chi_flux` | `gauss_project` + `phase_write` | Gauss constraint (SOR Poisson solver enforcing `div(J) = rho` at void sites), manifestation threshold `K_B = 0.511`, damping (`ALPHA`), flux budget conservation |
| `chi_frame` | `phase_forces` + `phase_movement` | Field-mediated coupling (electromagnetic, gravitational, Lorentz forces), collision detection, speed clamping at `c = 1/sqrt(3)`, boundary conditions |

### Ordering

The gates are not three independent operators applied in arbitrary order. They correspond to phases of the tick cycle that execute in a fixed sequence:

```text
phase_read   (chi_struct: what modes are structurally admissible)
  -> phase_write   (chi_struct + chi_flux: integrate, damp, threshold)
    -> gauss_project   (chi_flux: enforce Gauss constraint)
      -> phase_forces   (chi_frame: apply field-mediated coupling)
        -> phase_movement   (chi_frame: collision, clamping)
          -> tick++
```

A mode that fails `chi_struct` (e.g., violates local causality) is excluded before `chi_flux` is evaluated. A mode that fails `chi_flux` (e.g., insufficient flux budget) is excluded before `chi_frame` is evaluated. This ordering is the tick cycle's phase sequence, not a design choice.

### DP-C4 [CONJECTURE — CLOSED]

The earlier conjecture (DP-C3) that `Activate_C` should be read as a "selector on admissible local modes, not a new force law" is now confirmed: `Activate_C` IS the tick cycle restricted to the local Moore neighborhood. The three gates are three aspects of the existing update rule, not three new operators.

**What remains [OPEN]:** The exact line-by-line mapping from engine source code to gate decisions has not been performed. The mapping above is at the phase level, not the implementation level. A detailed code audit would make this precise but is not required for the theoretical identification.

### Failure Modes To Avoid

The method should not be used in the following undisciplined ways:

- do not declare a mode "activated" without naming the relevant context state
- do not treat `Activate_C` as a new dynamical law separate from the existing lattice update rules
- do not call a boundary effect "manifest" unless a readout map `M_C` is specified
- do not smuggle observer conclusions into `chi_frame` without stating the apparatus or boundary condition

### Near-Term Extensions

The next mathematical upgrades should be:

1. define a standard library of candidate local modes for recurring theory cases
2. connect `chi_struct` directly to the six engine update phases where possible
3. give explicit example readouts for measurement, cognition, and environment-coupled output
4. test whether `Activate_C` can be represented as a constraint projector on admissible local configurations

---

## 8. What This Fixes

This document fixes three live problems at once:

1. **Missing source problem**
   The active theory tree no longer needs to pretend `FOUND_CONSCIOUSNESS_MATHEMATICS.md` still exists.

2. **Vocabulary drift**
   The active reading path can now distinguish:
   - origin
   - `i`
   - Potential Core
   - Generative Interior
   - consciousness

3. **Operator gap**
   `Activate_C` now has a lattice-facing meaning rather than remaining a pure metaphor.

---

## 9. Claims Summary

| ID | Statement | Status |
|---|---|---|
| DP-C1 | Keep origin, `i`, Potential Core, Generative Interior, and consciousness distinct in the live vocabulary | [CONJECTURE / governance] |
| DP-C2 | Domain B is best read as a context-conditioned self-referential layer, not literal identity of `i` with consciousness | [CONJECTURE] |
| DP-C3 | `Activate_C` is a selector on admissible local modes, not a new force law | [CONJECTURE] |

| ID | Definition | Status |
|---|---|---|
| DP-D1 | Potential Core as lattice center | [DEFINITION] |
| DP-D2 | Moore-neighborhood context region | [DEFINITION] |
| DP-D3 | Context State `C_r(v_c,t)` | [DEFINITION] |
| DP-D4 | Lattice generative interior `G(P_c)` | [DEFINITION] |
| DP-D5 | Gate functions `chi_struct`, `chi_flux`, `chi_frame` | [DEFINITION] |
| DP-D6 | `Activate_C` and `G_C(P_c)` | [DEFINITION] |
| DP-D7 | Manifest Boundary of Affect `B_C(P_c,r)` | [DEFINITION] |
| DP-D8 | Manifest Output `M_C(P_c,r)` | [DEFINITION] |

---

## 10. Cross-References

| Document | Relevance |
|---|---|
| [DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md](DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md) | consciousness quadratic, complex roots, `K_C`, phase angle, Domain A/B/C |
| [FOUND_THE_EXISTENCE_FILTER.md](FOUND_THE_EXISTENCE_FILTER.md) | `C -> R` projection, Born reconstruction, domain-projection language |
| [../02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md](../02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md) | mathematical role of `i` as orthogonal phase structure |
| [../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md](../02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md) | Potential Core / Generative Interior vocabulary |
| [../02_foundations/FOUND_THE_FIRST_DISTINCTION.md](../02_foundations/FOUND_THE_FIRST_DISTINCTION.md) | origin of the first distinction and pre-complex level |
