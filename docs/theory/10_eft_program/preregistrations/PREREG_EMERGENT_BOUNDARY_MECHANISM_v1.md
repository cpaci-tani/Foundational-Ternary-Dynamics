# FTD-0474 — Emergent Boundary Mechanism Discriminator v1

**Status:** [PRE-REGISTRATION — RELOCKED/RUN]
**Date:** 2026-07-25
**Production tick:** frozen; observer-only campaign
**Question:** Does a finite manifested structure in the current engine acquire its extent from a mechanically active membrane, from an explicit environment, from periodic recirculation, or from none of these?

## 1. Ontology and scope

FTD's substrate is uncontained and has no defined outer wall. “Finite” refers to
the extent/support of a realized structure or a finite computational probe. A
periodic `L^3` window is therefore a computational quotient, not the ontology.

This campaign distinguishes four claims that must not be conflated:

1. a threshold-defined manifested **reaction front**;
2. a mechanically active **membrane** with surface stress;
3. support supplied by an explicit **environment/bath**;
4. apparent support supplied by **periodic recirculation**.

The wave-action stress used below is a selected observer of the written free
flux action. It is not promoted to a full matter-field stress tensor. The
current mobile production force is not a common-action partner of the written
field source (FTD-0467), so a positive mobile result would remain
`[MEASURED — CURRENT ENGINE]`, not a substrate theorem.

## 2. Frozen source and run matrix

The source is the already-qualified axial genesis-response fixture:

`J(center) = A K_GENESIS e_x`, with `A in {12,20,40}`.

The selected amplitudes are existing scenario points (`subknee`, response, and
`superknee`); no search is permitted. Volumes are `L in {24,32}`. Seeds are
`0xE0102000` through `0xE0102003`. Every run is 300 ticks. Tail samples are
fixed at ticks `{150,180,210,240,270,300}`.

Four arms are frozen:

| Arm | Active terms | Purpose |
|---|---|---|
| `reaction_periodic` | wave, Gauss projection, genesis/evaporation; periodic flux boundary | Existing reaction-front control; tests recirculation exposure |
| `reaction_dispersal` | same, but dispersal/open-window flux boundary | Uncontained-local proxy: outgoing boundary flux is not returned |
| `thermal_dispersal` | reaction-dispersal plus Langevin `T=0.005`, `gamma=0.02` | Explicit selected environment/bath |
| `mobile_dispersal` | reaction-dispersal plus coupling, production forces, and movement; gravity/Poisson/Lorentz/strong/weak terms off | Mechanical-eligibility arm using the current production matter rule |

`reflective_boundary=false` in every arm. `dual_substrate=false`. The history
journal is enabled on CPU and must remain state/RNG neutral.

Total registered runs: `4 arms * 2 volumes * 3 amplitudes * 4 seeds = 96`.

## 3. Frozen observer

At each tail sample, the observer extracts the largest 26-connected manifested
component and records:

- `N`: component occupancy;
- `R_rms`: RMS radius about the periodic-unwrapped component centroid;
- `B`: number of component sites with at least one void 6-neighbour;
- `I=N-B`: one-cell morphological interior;
- `C_A=B/N^(2/3)`: surface-area coefficient;
- manifested centroid displacement;
- wave kinetic energy;
- genesis, evaporation, movement, and annihilation event counts.

For the free flux action

`L_J = 1/2 |W|^2 - c^2/2 sum_i |D_i J|^2`,

the selected spatial stress observer is

`T_ij = c^2 (D_i J).(D_j J)
        + delta_ij [ |W|^2/2 - c^2 sum_k |D_k J|^2/2 ]`.

The radial traction at a cell is `T_rr=n_i T_ij n_j`. The observer records:

- `Delta p_J`: mean `T_rr` on component boundary cells minus the mean on the
  unique exterior face-neighbour cells;
- `C_L=Delta p_J R_V`, where `R_V=(3N/4pi)^(1/3)`;
- `f_interface`: fraction of the total window gradient energy carried by the
  union of component boundary and exterior face-neighbour cells.

These definitions are frozen before execution. No alternative stress sign,
shell width, radius definition, or post-hoc fit may replace them in v1.

## 4. Eligibility and tolerances

A run is a **stable finite structure** only when all six tail samples have
`4 <= N <= 0.01 L^3`, `CV(N) <= 0.20`, and `CV(R_rms) <= 0.15`.

A stable run is **dynamically active** when its tail has at least one journaled
genesis, evaporation, movement, or annihilation event, or mean wave kinetic
energy above `1e-10`.

A run is **mechanically eligible** only when it is in `mobile_dispersal`, is
stable and dynamically active, has at least one tail movement event, and has
`I>0` in at least four of six tail samples. Reaction-only arms can establish a
reaction front but can never, by themselves, establish a membrane.

For each `(arm,L,A)`, at least 3/4 seeds must be eligible for that cell to pass.

## 5. Frozen verdicts

### `MECHANICAL_MEMBRANE_SUPPORTED`

This verdict requires all of:

1. all six `(L,A)` cells of `mobile_dispersal` pass mechanical eligibility;
2. all six `reaction_dispersal` cells pass stable/dynamic eligibility;
3. across the three amplitudes at each volume, the seed-averaged `C_A` has
   `CV <= 0.30`;
4. seed-averaged `C_L` is nonzero, has one sign, and has `CV <= 0.35` across
   amplitudes at each volume;
5. mean `f_interface >= 0.50` in both dispersal arms;
6. the `L=24` and `L=32` coefficients agree within 30%.

This is evidence for a membrane-like current-engine mechanism, not a proof of
continuum Young-Laplace law and not an ontological theorem.

### `EXPLICIT_ENVIRONMENT_SUPPORT_ONLY`

This verdict requires the membrane gate to fail, at least 75% of all
`thermal_dispersal` runs to be stable/dynamic, and at most 25% of all
`reaction_dispersal` runs to be stable/dynamic. The result states dependence on
the selected Langevin bath; it does not make that bath native.

### `PERIODIC_RECIRCULATION_SUPPORT_ONLY`

This verdict requires the membrane gate to fail, at least 75% of
`reaction_periodic` runs to be stable/dynamic, and at most 25% of
`reaction_dispersal` runs to be stable/dynamic.

### `REACTION_FRONT_ONLY`

This verdict applies when one or more reaction arms produce stable/dynamic
finite components but the mobile mechanical-eligibility gate fails and neither
environment-only nor periodic-only thresholds are met.

### `NO_QUALIFIED_FINITE_STRUCTURE`

This verdict applies when fewer than 25% of every arm's runs are stable/dynamic.

### `MIXED_OR_UNRESOLVED_BOUNDARY_MECHANISM`

This is the residual registered verdict. It cannot be promoted by visual
inspection.

## 6. Structural validity gates

- exactly 96 run rows and 576 tail-sample rows are produced;
- every recorded scalar is finite;
- journal-enabled and journal-disabled controls have identical selected-state
  hashes and identical RNG hashes after 64 ticks;
- the observer never writes `Voxel`, `TermToggles`, event ordering, or RNG;
- no production source file or tick phase is modified;
- CSV and manifest hashes are recorded after execution;
- focused CTest and the seven-test golden merge gate pass.

A physics verdict may be negative without failing CTest. CTest fails only a
structural-validity gate.

## 7. Locked implementation

Campaign source: `engine/tests/campaign_emergent_boundary_mechanism.cpp`

Observer header: `engine/include/ftd/eft/emergent_boundary_observer.h`

**Locked SHA-256:**

- campaign source: `D219827B4D1D07AAEC2899A7008A11406B0CAE00B7C97F53FA6F40FEBC45F6A2`
- observer header: `913789453A934EF8765414B9F523078E1BFA6E542BB67C0CA8D608EA7B651FC2`

Both hashes were recorded after a successful compile and before first campaign
execution. Any source change requires a documented relock before rerun.

### Execution note 1 — invalidated artifact route

The first locked execution completed structurally and displayed provisional
verdict `REACTION_FRONT_ONLY`, but CTest's build-directory working path routed
the files to `engine/build/engine/results/ftd_0474/` rather than the canonical
run-of-record directory. That execution is not the run of record. The only
authorized repair replaces the relative output path with an `__FILE__`-anchored
path. No fixture, estimator, tolerance, gate, or verdict logic may change. The
provisional verdict was already visible before repair; the rerun is therefore
an exact reproducibility check, not a blind first look.

**Relocked SHA-256 after the authorized I/O-only repair:**

- campaign source: `04F4D0D72879427EFC6BB1354B3D904C8F2214BE4B9F70912E5362F22F66135F`
- observer header: `913789453A934EF8765414B9F523078E1BFA6E542BB67C0CA8D608EA7B651FC2`

The observer hash is unchanged. The campaign-source diff is restricted to the
output-directory expression.

### Execution note 2 — run of record

The canonical rerun reproduced verdict `REACTION_FRONT_ONLY`: periodic and
dispersal reaction arms each pass 20/24 stable/dynamic runs, the explicit
thermal arm passes 24/24, and the mobile arm passes 0/24 despite 977 tail
movement events. The membrane Laplace and interface-localization gates fail.
All 96 run rows and 576 tail-sample rows are finite; observer state/RNG
neutrality is exact. See `AUDIT_EMERGENT_BOUNDARY_MECHANISM.md`.
