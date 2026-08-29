# Plan — Scientific Scenario Qualification

**Status:** active plan; no scientific claim is established by this document.

**Epistemic status:** **[OPEN]** implementation and qualification program.

**Scope:** every scenario exposed by the web application, every scale on which
it is presented, every engine/backend that can execute it, and every UI surface
that describes or visualizes it.

**Progression rule:** qualify one scenario at a time. Do not begin the next
scenario until the current scenario has a recorded disposition and all evidence
required by that disposition is reproducible. A failed physical identification
is preserved as **[CLOSED NEGATIVE]** provenance; it is not hidden, renamed into
success, or silently retried under a new presentation.

This plan turns the scenario library into an auditable scientific instrument.
It does not presume that an FTD interpretation is true. Simulation is legitimate
evidence only when the mathematical model, imposed inputs, numerical method,
measured outputs, uncertainty, and interpretation are kept distinct.

## 1. Governing principles

1. **Mathematics before presentation.** Every scenario must identify the state
   space, discrete update, initial data, boundary conditions, units, and
   observables before its name or visualization receives a physical meaning.
2. **No visual promotion.** Resemblance to a familiar object is not evidence of
   physical identity. A rendered electron-like shape is not an electron result.
3. **Provenance is part of the result.** Every constant, field, seed, toggle,
   conversion, and comparison target must state whether it is primitive,
   selected, imposed, calibrated, derived, parametric, or measured.
4. **Controls are mandatory.** A candidate needs null controls, perturbation
   controls, resolution controls, boundary controls, and an independent
   observable appropriate to its claim.
5. **Acceptance criteria are fixed first.** Thresholds and failure conditions
   must be written before measurement. Post-hoc tuning may generate a new
   preregistered trial; it cannot validate the trial used to tune it.
6. **Scale is a contract.** A scenario may only claim structures available at
   its scale. Cross-scale interpretation requires an explicit, tested bridge.
7. **Scientific state and presentation state are separate.** Rendering may be
   decimated to maintain responsiveness; the simulation may not silently skip,
   alter, or invent scientific state.
8. **Negative results remain first-class.** A **[CLOSED NEGATIVE]** scenario is
   a useful control and a guard against repeating a falsified identification.
9. **The UI reports status; it does not upgrade it.** Names, badges, overlays,
   tooltips, catalog values, and exported data must reproduce the recorded
   qualification exactly.
10. **One-at-a-time closure.** Work advances only after the current scenario's
    audit record, tests, UI wording, and disposition agree.

## 2. Scenario representation classes

These are presentation/qualification classes, not replacements for the
project's canonical epistemic tags.

| Class | Meaning | Required UI wording |
|---|---|---|
| Native experiment | Direct probe of the declared discrete rule and state | “Native experiment”; list enabled native terms |
| Derived-given-selection | Consequence of a declared selected extension | “Conditional on [SELECTION] …” |
| Candidate | Physical identification under unresolved tests | “[object] candidate”; display unmet gates |
| Parametric model | Standard/effective formula with imported parameters | “[PARAMETRIC]”; identify the imported law and values |
| Pedagogical demonstration | Explanatory or illustrative behavior | “Pedagogical”; prohibit evidentiary interpretation |
| Reference only | Catalog data not produced by the running simulation | “Reference”; visually separate it from live observables |
| Closed negative | A preregistered identification failed | “[CLOSED NEGATIVE]”; link the failure evidence |
| Control | Null, mirror, calibration, or regression comparison | Name the controlled variable and expected result |

The underlying claim must also carry the canonical epistemic tag appropriate to
its evidence. In particular, an **[IMPOSED]** seed is not **[EMERGENT]**; a
standard formula evaluated with FTD values is **[PARAMETRIC]**, not derived; and
a proposed particle interpretation remains **[CONJECTURE]** or **[OPEN]** until
its qualification gates are passed.

## 3. Mathematical scenario contract

Every scenario must have a machine-readable contract with the following fields.
No scenario is scientifically qualified while a required field is missing.

### 3.1 Identity and ownership

- Stable scenario ID and display name.
- Owning scale and engine/backend.
- Scenario representation class.
- Canonical epistemic status and provenance reference.
- Responsible source module and validation test.
- Version of the scenario contract and last qualification date.

### 3.2 Mathematical definition

- State space and the subset populated by the scenario.
- Evolution map, phase ordering, and enabled terms.
- Initial conditions, including whether each feature is imposed or generated.
- Boundary conditions and finite-domain interpretation.
- Tick size, lattice spacing or effective integration step.
- Declared symmetries, conservation laws, invariants, and expected violations.
- Randomness source, seed policy, ensemble size, and determinism requirement.
- Singular, exceptional, and out-of-domain states.

### 3.3 Parameter and unit provenance

- Native dimensionless parameters.
- Every imposed, selected, calibrated, measured, or imported value.
- Native units and the complete physical-unit conversion.
- Valid resolution, timestep, volume, density, and boundary ranges.
- Any standard-physics formula used, explicitly marked **[PARAMETRIC]**.
- Uncertainty sources and error propagation for displayed physical values.

### 3.4 Claims and observables

- Exact claim the scenario is permitted to support.
- Exact claims the scenario is prohibited from implying.
- Primary and secondary observables, with measurement definitions.
- Null and alternative hypotheses.
- Acceptance thresholds and falsification criteria fixed before execution.
- Required controls and independent comparison data.
- Known limitations and currently unmet gates.

### 3.5 Execution and presentation

- Supported CPU, WASM, worker, WebSocket, and GPU backends.
- Required backend-parity tolerance.
- Expected simulation cost and memory ceiling.
- Rendering and telemetry demand policy.
- UI name, badge, explanation, default overlays, and prohibited overlays.
- Export schema sufficient to reproduce the reported result.
- Declared performance budget and reference hardware/browser profile.

## 4. Canonical scenario manifest

Create one machine-readable manifest as the single source of truth for scenario
identity, status, and presentation. The final format may be JSON, a generated JS
module, or another schema-validated representation, but it must not require
hand-maintained copies of scientific meaning in multiple UI files.

Minimum conceptual schema:

```text
id
display_name
scale
scenario_class
epistemic_status
mathematical_model
state_space
initial_conditions
boundary_conditions
enabled_terms
parameter_provenance
native_units
physical_calibration
observables
accepted_claims
prohibited_claims
validation_protocol
acceptance_gates
falsification_gates
known_limitations
backend_support
backend_tolerances
resolution_domain
cross_scale_inputs
visual_defaults
performance_budget
evidence_links
contract_version
```

The scenario picker, contextual description, overlays, help text, knowledge base,
tests, export metadata, and documentation must derive from or validate against
this manifest. Catalog/reference quantities must use a visibly separate channel
from quantities measured in the current run.

The migration must preserve existing scenario IDs and historical evidence. If a
display name changes, the old name remains in provenance and release notes.

## 5. Scale contracts

The registry inventory determines the exact active scale list. No undocumented
scale or scenario is exempt. The initial contracts are:

| Scale | May represent | Must not imply without an explicit passed bridge |
|---|---|---|
| Scale 0 — lattice/substrate | Finite lattice records, manifestation, flux, tick dynamics, and candidate structures measured from them | Established electrons, Standard Model identity, atoms, QED, GR, or continuum physics |
| Scale 1 — particles | Coarse-grained particles and clearly identified parametric catalog particles | That catalog particles emerged from Scale 0 |
| Scale 2 — atoms | Atomic models with declared electronic, nuclear, and shielding approximations | First-principles FTD atoms when standard atomic formulas are imported |
| Scale 3 — molecules | Molecular geometry, bonding, and declared interaction potentials | Derived chemistry when force fields or geometries are imposed or calibrated |
| Scale 4 — planetary | Finite classical N-body experiments with declared units, integrator, and ephemeris inputs | Substrate-derived gravity without a tested Scale 0-to-4 bridge |
| Scale 5 — cosmic | Declared effective astrophysical/cosmological models and initial conditions | Emergent FTD cosmology merely because standard equations are simulated |
| Scale 6 — structural/meta | Moore-neighborhood geometry, parity, and finite structural decompositions | Physical dynamics or particle identity from geometry alone |
| Any experimental/context scale | Research or pedagogy under its own mathematical contract | Production-grade physical identification by naming or visual analogy |

Prepared higher-scale structures may be inspected at Scale 0 as substrate
experiments, but their names must say “seed,” “template,” “candidate,” “control,”
or “reference” as appropriate. Otherwise they are relocated to the scale whose
mathematics they actually execute.

## 6. Per-scenario qualification gates

Each scenario receives a dedicated audit record. Gates run in this order; later
gates cannot compensate for failure or omission of an earlier one.

### Gate 1 — static trace

- Locate every seed write, toggle override, default, and hidden dependency.
- Enumerate the full active equation/update path.
- Trace each parameter and unit conversion to its source.
- Identify backend ownership and cross-scale inputs.
- Compare source, registry, UI, tests, and documentation for drift.

### Gate 2 — mathematical well-posedness

- State the finite equations and update schedule without relying on UI prose.
- Verify dimensional consistency and domain restrictions.
- State symmetry, conservation, covariance, or discrete-invariance expectations.
- Define observables without circular reference to the desired interpretation.
- Record a theorem, derivation, selection, imposed choice, conjecture, open item,
  or closed negative only at the status the evidence supports.

### Gate 3 — numerical validity

- Reproducibility and determinism where required.
- CPU/WASM/worker/WebSocket/GPU parity within preregistered tolerances.
- Lattice-size, volume, boundary, and integration-step sensitivity.
- Floating-point stability and bounded-error behavior.
- Multi-seed/ensemble statistics where stochastic behavior is claimed.
- No race, lifecycle, stale-state, or order-dependent result.

### Gate 4 — scientific validity

- Preregister the null, candidate claim, observables, and thresholds.
- Run null, mirror, perturbation, and negative controls.
- Do not tune and validate on the same output.
- Require at least one independent observable for a physical identification.
- Quantify uncertainty, finite-size effects, and calibration dependence.
- Retain failures and ambiguous outcomes without rhetorical promotion.

### Gate 5 — scale appropriateness

- Confirm that the scenario's executed mathematics belongs to its displayed scale.
- Audit every physical noun in the name and explanation.
- Qualify higher-level vocabulary as candidate, proxy, template, parametric, or
  reference until a tested cross-scale map exists.
- Confirm that no downstream scale upgrades the status of its inputs.

### Gate 6 — UI and export truth

- Match the scenario name, badge, summary, limitation text, and help content to
  the recorded disposition.
- Separate simulated observables, derived analysis, calibrated conversions, and
  catalog reference data visually and in exported data.
- Only expose overlays for quantities the scenario or scale can support.
- Show unmet gates and **[CLOSED NEGATIVE]** status without requiring the user to
  inspect source code.
- Include enough metadata in screenshots/exports to identify the scenario,
  contract version, backend, boundary, resolution, tick, and units.

### Gate 7 — performance and concurrency

- Maintain a 16.67 ms visual frame budget (60 frames/s) on the declared reference
  browser and hardware under the scenario's supported operating envelope.
- Record median, p95, p99, worst-frame, long-task, memory, and GPU timing rather
  than relying on a single average FPS number.
- Keep simulation stepping, rendering, overlays, and telemetry independently
  scheduled and demand-gated.
- Coalesce DOM work to one render-frame commit; do not poll hidden panels.
- Reuse buffers and geometries; prohibit unbounded allocation in frame loops.
- Test resize, scenario switching, background/foreground transitions, and rapid
  toggle input for races and stale async results.
- When scientific workload exceeds visual capacity, reduce presentation sampling
  or enter an explicit degraded-rendering mode. Never silently drop scientific
  ticks or alter the mathematical state to preserve the displayed FPS.

“Always 60 FPS” therefore means the UI meets the declared 60 FPS envelope in
repeatable performance tests and truthfully reports when a requested workload is
outside it. It is not an untestable claim covering every device and arbitrary
workload.

## 7. Dispositions and the stop/go rule

One of the following dispositions closes a scenario audit:

- **Passed:** every gate required by the stated claim passed.
- **Conditional:** passed only within an explicit parameter/backend/domain bound.
- **Open:** evidence is incomplete; the scenario is labeled as an open candidate.
- **Relocated:** the mathematics belongs on another scale.
- **Renamed:** the implementation is valid but its old physical name overclaimed.
- **Pedagogical only:** useful explanation, not scientific evidence.
- **Reference only:** displays catalog information, not a simulated observable.
- **Closed negative:** the tested identification failed and is preserved.

The next scenario may begin only after:

1. the disposition is written in its audit record;
2. contract, manifest, source, tests, UI, and documentation agree;
3. reproduction commands pass or the failure is explicitly part of the verdict;
4. performance evidence is stored for the supported envelope; and
5. old contradictory names or active-path claims have been reconciled.

An **Open** or **Closed negative** disposition counts as procedural closure; it
does not count as scientific success. This distinction prevents the program from
stalling while also preventing unsupported promotion.

## 8. Scale 0 qualification order

Scale 0 is first because claimed emergence and every later scale bridge depend on
its substrate behavior. Process the registry in the following dependency order,
one scenario at a time within each family:

1. Empty lattice, reset, deterministic replay, and backend baselines.
2. Native propagation, wave packets, and dispersion controls.
3. Periodic, reflecting, absorbing, and other boundary probes.
4. Gauss-law, divergence, charge-continuity, and polarity controls.
5. Genesis, manifestation, evaporation, and lifecycle controls.
6. Matter-field reciprocity, force, momentum exchange, and movement controls.
7. Stable localized proto-matter and perturbation-basin experiments.
8. Electron and positron candidate/control scenarios.
9. Remaining leptons and neutrino candidate/control scenarios.
10. Gauge- and scalar-boson candidate/control scenarios.
11. Quark, color, confinement, and conjugation candidate/control scenarios.
12. Composite hadron templates and candidate bound states.
13. Atomic or molecular proxies currently presented at Scale 0.
14. Gravity, proper-time, and reference-frame scenarios.
15. Thermodynamic, phase, macroscopic, phenomenological, and pedagogical demos.

The existing Scale-0 qualification audit remains evidence for its frozen tests;
this plan does not promote its physical interpretations. Each prior disposition
must be imported into the canonical manifest with its provenance intact.

## 9. Electron qualification program

The existing imposed radial-wave “electron” remains a null/control scenario at
its recorded status. It must not be tuned until it resembles an electron and
then treated as independent validation.

A new electron candidate must pass these gates in order:

1. **Formation:** autonomous formation, or an admissibly declared finite seed
   whose imposed content is fully accounted for.
2. **Localization:** persistent localization with a nonzero perturbation basin,
   not a fixed source or visually selected frame.
3. **Charge:** dynamically conserved signed charge with satisfied discrete Gauss
   and continuity laws.
4. **Reciprocity:** reciprocal field-matter energy and momentum exchange under
   the same state-complete update.
5. **Rest structure:** amplitude-independent rest pole and convergent finite-
   lattice dispersion relation.
6. **Spin:** dynamically carried structure with the required transformation and
   measurement behavior, not a seeded scalar label.
7. **Chirality:** signed chiral state or response with a defined observable and
   transport law, not an unsigned amplitude proxy.
8. **Conjugation:** electron/positron conjugation under the declared discrete
   transformation, including equal rest properties and opposite charge.
9. **Convergence:** stability under volume, boundary, resolution, timestep, seed,
   and supported-backend variation.
10. **Independent comparison:** only after structural gates pass, compare
    preregistered dimensionless observables with electron physics while exposing
    all calibrations and imports.

Passing gates 1–9 earns “electron candidate” at the status supported by the
evidence. Physical identification requires gate 10 as well; a catalog card or
mass calibration does not substitute for these gates.

## 10. Cross-scale handoffs

After Scale 0 is dispositioned, qualify bridges in dependency order:

1. Scale 0 → Scale 1: coarse-grained position, momentum, mass, charge, spin,
   uncertainty, lifetime, and identity; distinguish promoted clusters from
   parametric catalog particles.
2. Scale 1 → Scale 2: atomic degrees of freedom, potentials, nuclear assumptions,
   electron treatment, units, and calibration inheritance.
3. Scale 2 → Scale 3: molecular geometry, bonding law, force-field provenance,
   energy accounting, and uncertainty propagation.
4. Gravity handoffs: distinguish Scale-0 lattice gravity from effective
   classical N-body and cosmological equations; test unit bridges separately.
5. Structural/context handoffs: prevent Moore geometry, reference-frame
   pedagogy, or catalog metadata from becoming physical dynamics by implication.

Every handoff must specify:

- input and output state schemas;
- lossy/coarse-grained information;
- unit and calibration map;
- conservation and uncertainty behavior;
- applicable parameter domain;
- validation and falsification tests; and
- the rule that output epistemic status cannot exceed input evidence plus the
  independently validated content of the bridge.

## 11. UI truth requirements

Every scenario-facing UI surface must answer, without ambiguity:

- What mathematics is running?
- Which values were imposed or imported?
- What is measured from the current run?
- What physical interpretation is being tested?
- What status has that interpretation earned?
- Which gates remain open or failed?
- What scale, units, backend, boundary, resolution, and tick are active?

Required presentation changes include:

- standardized Native, Candidate, Parametric, Pedagogical, Reference, Open, and
  Closed Negative badges linked to their definitions;
- a provenance drawer generated from the scenario contract;
- visible separation of live telemetry from catalog/reference quantities;
- scenario-specific overlay applicability derived from the manifest;
- export headers containing the complete execution contract; and
- names such as “electron candidate,” “electron radial-wave control,” or
  “electron catalog reference” instead of an unqualified “electron” until the
  corresponding qualification exists.

## 12. Performance qualification program

Performance evidence is collected per scenario, scale, backend, lattice/body
size, active overlay set, and visible sidepanel. A fast default view does not
qualify an expensive telemetry or visualization configuration.

For each scenario:

1. Establish an idle/UI baseline.
2. Measure the scientific step without visualization.
3. Add the viewport renderer.
4. Add overlays one at a time.
5. Add each sidepanel one at a time.
6. Test worst supported combinations and rapid interaction.
7. Profile allocations, long tasks, worker messages, GPU uploads, layout/paint,
   and async lifecycle.
8. Record the supported 60 FPS envelope and any explicit degraded mode.

Sidepanels must consume snapshot data, update at the lowest scientifically useful
rate, and commit visual changes on animation frames. Chart sampling frequency and
chart draw frequency are separate controls. Hidden or collapsed panels perform
no polling, layout, chart rendering, or field extraction.

## 13. Deliverables

1. A schema-validated canonical scenario manifest.
2. A scale-contract specification for every registered scale.
3. One versioned audit record per scenario.
4. Automated contract, provenance, parity, convergence, race, and UI-truth tests.
5. Reproducible scientific result bundles with metadata and uncertainty.
6. Backend-parity and numerical-convergence reports.
7. A scenario atlas showing class, status, passed gates, failed gates, and links.
8. UI status badges, provenance views, and simulated-versus-reference separation.
9. Per-scenario/interface performance reports and 60 FPS operating envelopes.
10. A cross-scale bridge ledger preserving provenance and epistemic status.
11. A closed-negative register that prevents falsified identifications from
    reappearing without a new preregistered hypothesis.

## 14. Initial execution sequence

Before qualifying the first scenario:

1. Freeze and enumerate the current scenario and scale registries.
2. Define and validate the manifest schema.
3. Import existing statuses and evidence without promotion.
4. Build automated drift checks between manifest, engines, UI, and docs.
5. Create the audit-record template and performance harness.
6. Select the empty-lattice/reset baseline as Scenario 1.

Thereafter, follow Section 8 strictly. Publish the disposition and synchronize
all representations before opening the next scenario audit.

## 15. Relationship to existing evidence

- [`SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`](SPEC_SCALE0_SCENARIO_ARCHITECTURE.md)
  describes the current Scale-0 scenario layers and lifecycle.
- [`audits/AUDIT_SCALE0_SCENARIO_QUALIFICATION_2026-07-24.md`](audits/AUDIT_SCALE0_SCENARIO_QUALIFICATION_2026-07-24.md)
  records the prior frozen-catalog qualification and physical-promotion gates.
- [`PLAN_SCALE0_UI_INTERFACE_AUDIT_60FPS.md`](PLAN_SCALE0_UI_INTERFACE_AUDIT_60FPS.md)
  supplies the one-interface-at-a-time UI performance discipline.
- [`SPEC_SCALE0_PERF_TELEMETRY_PANELS.md`](SPEC_SCALE0_PERF_TELEMETRY_PANELS.md)
  defines demand-gated telemetry and panel-performance architecture.

Those documents are inputs and provenance. This plan neither supersedes their
evidence nor certifies that any physical scenario has passed the new program.
