# FTD Engine Constructor Contract

**Date:** 2026-04-26
**Status:** [SPECIFICATION] / [AUDIT]
**Scope:** Formal engine obligations implied by
`docs/theory/02_foundations/FOUND_MINIMAL_INSTANTIATED_UNIVERSE.md`.

This document translates the minimal-universe constructor into engine-facing
contracts. A passing engine is not automatically "all of physics"; it is a
constructive, auditable candidate system whose domains are explicitly declared,
tested, and tagged.

---

## 1. Contract Signature

```text
construct_engine_universe(
    required: EngineInstantiation,
    optional: EnginePhysics = {}
) -> EngineRun
```

Required engine declarations:

```text
EngineInstantiation {
    context_contrast: declared scenario, lattice domain, and nonzero carrier rule
    ontic_units: manifested states and/or field carriers
    individuation: particle IDs, transport paths, or history-ledger carriers
    closure: lattice/cell/boundary accounting domain
    admissibility: locality, propagation, constraint, and ledger checks
}
```

Optional engine physics:

```text
EnginePhysics {
    relation_space
    time_order
    state_alphabet
    flux_field
    locality_rule
    update_rule
    symmetry_redundancy
    observable_map
    constraint
    continuity_ledger
    energy_ledger
    probability_measure
    thermodynamic_accounting
    metric_geometry
    topology_defects
    blocking_map
    continuum_limit
    many_body_structure
    sector_decomposition
    action_or_generator
    stochastic_extension
}
```

---

## 2. Domain Obligations

| Domain | Engine obligation | Current evidence | Status |
|---|---|---|---|
| Instantiation | Scenario/lattice/context must declare what counts as nonzero manifestation or field carrier. | `RenderBridge` lattice + constructors; `s != 0`; flux injection paths. | [PARTIAL] |
| Identity | Carriers must have explicit persistence criteria. | `Injector` particle IDs; continuity/history ledgers. | [PARTIAL] |
| Relation | Neighbor relation must be explicit and stable. | 3D lattice, Moore neighborhood, flat indexing tests. | [PRESENT] |
| Frame/symmetry | Tests must separate coordinate relabeling from intrinsic dynamics. | Backend parity and anisotropy tests exist; no general frame/gauge contract. | [PARTIAL] |
| Time/dynamics | Tick order must be observable and deterministic unless a stochastic toggle is declared. | `tick()`, `run()`, `dt`, deterministic tests. | [PRESENT] |
| Transport | Mobility requires oriented current and continuity accounting. | Native continuity ledger; GPU full-tick ledger. | [PRESENT] |
| Constraint | Production constraint representation must be selected and tested. | Gauss, dual-cell, matched-Poisson tests. Final representation open. | [PARTIAL] |
| Locality/causality | Update support and propagation bound must be bounded. | Moore support, CFL constants, locality tests. Formal propagation audit open. | [PARTIAL] |
| Conservation | Closure domain and ledgers must reconcile changes. | Continuity/reaction/energy ledgers. | [PARTIAL] |
| Observables | Every physics claim must name an observable map. | Diagnostics, operator moments, sim observables. Central registry open. | [PARTIAL] |
| Probability | Random/stochastic claims require a declared measure and seed contract. | Langevin seed plumbing; ensemble tests. GPU Langevin open. | [PARTIAL] |
| Thermodynamics | Entropy/arrow claims require coarse records and accounting. | `compute_entropy`, thermodynamics tests. Coarse arrow program open. | [PARTIAL] |
| Metric/geometry | Distance/angle/curvature claims require metric declaration. | Lattice metric, anisotropy, Lorentz recovery. Continuum geometry open. | [PARTIAL] |
| Topology/defects | Sector/topology claims require defect/loop observables. | Wilson topology, BCC/stella sector work. GPU port open. | [PARTIAL] |
| Many-body/correlation | Mixing claims require multiplicity and correlation observables. | Correlations, many-body tests, GPU histories. Nonlinear matrix open. | [PARTIAL] |
| Blocking/EFT | EFT claims require a declared blocking map. | Native b=2 blocking and Gaussian flow. Nonlinear flow open. | [PARTIAL] |
| Continuum limit | Smooth physics claims require scaling/continuum protocol. | Lorentz/aniso tests and docs. Full proof open. | [OPEN] |
| Action/generator | Variational/generator claims require an engine-native action or transition generator. | Lagrangian tests; Gaussian generator docs. Full native measure open. | [PARTIAL] |
| Phenomenology | Real-world claims require predeclared observable extraction and comparison. | Some campaigns exist. Native bridge still incomplete. | [PARTIAL] |

---

## 3. Constructor Invariants For Tests

Every production physics test should declare these fields in comments, metadata,
or telemetry:

```text
domain
epistemic_tag
required_constructor_inputs
optional_physics_inputs
observable_map
closure_domain
backend_policy
expected_invariant
failure_meaning
```

Minimum engine-level rejects:

```text
mobility without continuity ledger
mixing claim without many-body or sector multiplicity
EFT coupling claim without blocking map
continuum claim without scaling protocol
observable claim without observable map
stochastic claim without seed/measure declaration
```

---

## 4. Formalization Targets

Near-term engine work:

```text
1. Add a small constructor-contract test helper for declaring domain metadata.
2. Add an "EFT quick suite" CTest label covering constructor-critical tests.
3. Add an observable registry for native ledgers, blocked operators, and couplings.
4. Add a production nonlinear-flow campaign that emits blocked operator matrices.
5. Add GPU ports for CPU-only topology, dual-cell, and coupling diagnostics.
6. Decide the production Gauss representation.
7. Define the native history measure/action or explicitly demote action claims.
```

The engine is constructor-serious when each domain row has:

```text
spec declaration
implementation path
unit/regression test
campaign or scaling test where applicable
epistemic tag
known failure mode
```

