# PLAN — Scale 1 Particle Context for the v4 Physics Record

**Status:** `[IMPLEMENTED — LOCAL PHYSICS/SCENARIO QUALIFICATION COMPLETE; PERFORMANCE AND DEPLOYMENT OPEN]`
**Date:** 2026-09-01
**Owner surface:** shared C++ engine + native app + web dashboard
**Execution order:** complete the scientific/domain contract before expanding the UI
**Companion audit:** `docs/audits/AUDIT_2026-08_scale1-discrete-particle-physics.md`
**Scientific horizon:** canonical `LEDGER.md` through FTD-1024
**Claim precedence:** `LEDGER.md` > v3 constitution > active theory references > this plan

**Implementation revision:** `scale1-v4-particle-context-r9-behavior-contract`
**Snapshot schema:** 3
**Scenario program:** 34 executable rows in one unified scenario selector. Former
non-executable roadmap rows are retained only as documented research boundaries.

## 0. Particle-context checkpoint

Scale 1 is the particle context immediately above Scale 0. Native lattice
observation is primary. Continuous point-particle dynamics is retained only as
an explicitly effective reference laboratory.

- Shared C++ `Scale1ScenarioSpec` is the authoritative scientific manifest.
- WASM publishes status, ownership, provenance, expected observables,
  prohibited claims, performance class, and validation evidence.
- One Scale-1 scenario selector exposes the full registry. Scenario families
  organize native observations, quantum/QED references, and effective labs
  without a second workspace control.
- One native replay exposes six observation subviews over the immutable FTD-0760 M3 record.
- Thirty effective experiments and three parametric catalog views are
  runnable beside the native-anatomy views.
- Research ideas without an exact live observer, immutable artifact, or
  executable QED owner are not dashboard scenarios.
- Particle records expose identity availability, age, support/constituent
  counts, integer and fractional centers, identity/graph/energy margins,
  optional clock phase, lineage, source revision, and artifact revision.
- Every physics module and every scenario now carries a machine-readable
  validation verdict, evidence target, pass criterion, and native-owned physics
  mask. A validated imposed kernel remains imposed; a confirmed negative
  boundary remains documented outside the live registry.

The internal coherent-observation contract can publish candidate support and
center/velocity observers without mass or identity qualification. The retired
interactive capture scenario is no longer in the registry. Cross-tick identity
continuity and event detection remain research requirements, not disabled UI
entries.

## 1. Redevelopment decision

Scale 1 will no longer present one classical N-body sandbox as though it were
the particle layer of FTD. It will become two deliberately separated products
over one shared data contract:

1. **Native Matter Observatory — primary.** A read-only, particle-context view
   of qualified Scale-0 relational matter records: manifested core,
   constituent relation, field dressing, outgoing/background field, identity
   margins, motion, and event history. This view does not advance a second
   independent universe.
2. **Effective Particle Lab — secondary.** The existing continuous-coordinate
   `ParticleEngine`, retained for controlled analytical-force experiments and
   pedagogy. Its laws are imported/imposed unless a narrower source establishes
   more. It must never be presented as a derivation of particles from v3.
This split is the central design constraint. It lets Scale 1 use the latest
matter findings without converting selected research-state constructions into
production axioms or pretending that a point particle contains the complete
substrate state.

### Implementation checkpoint — 2026-09-01

| Package | State | Evidence / remaining boundary |
|---|---|---|
| R0 baseline and transaction repair | **implemented** | pre/post admissibility, atomic rollback, integrator-transition, deterministic contact, and focused native tests |
| R1 shared schema and registries | **implemented** | versioned `Scale1Snapshot`; physics/capability/observer registries; native/WASM serialization |
| R2 Native Matter | **implemented at qualified replay scope** | one immutable FTD-0760 M3 replay with six observation subviews; identity continuity, event qualification, mass, and species recovery remain research boundaries outside the scenario selector |
| R3 projection/compare | **retired and removed** | toolbar action, browser capture/transfer module, native projector/ledger, projection scenarios, source-voxel overlays, and tests removed; Scale Context owns pedagogy |
| R4 Effective Lab hardening | **implemented on CPU/WASM** | verified profile, retired rescale, explicit contact events, coverage-aware conservation, effective-record provenance; native CUDA qualification remains open |
| R5 web UI | **particle-context contract implemented** | one unified selector for 34 executable native-manifest rows; explicit behavior badges, observable routing, M3 subviews, and paired controls; registry-backed Physics card consolidated into Controls with scenario/verified/all-applicable profiles; complete nine-channel force-overlay presentation; record-driven provenance/qualification layers, quantum/QED pedagogy, and coverage diagnostics |
| R6 qualification/deployment | **local physics/scenario qualification complete** | all 12 physics modules and all 34 scenario rows have explicit verdict/evidence/criteria and native-owned profiles; hardware workload matrix and deployed clean-commit identity remain open |
| R7 retirement | **implemented** | all runtime scale-transfer UI, native/WASM APIs, generic conversion utilities, dedicated scenarios, overlays, and bridge campaigns removed; Scale Context is presentation-only |

This checkpoint does not promote the registered replay to a live production
matter generator and does not close the performance/deployment gate. There is
no runtime scale handoff; the Scale Context sidepanel owns
pedagogical scale explanation without manufacturing cross-scale state.

### Qualification vocabulary

- `contract_qualified`: the registered record, catalog, or
  baseline numerical contract passes at its declared epistemic status.
- `kernel_validated`: the implemented effective force/integrator behavior
  passes its exact code invariants; this does not validate the force as native
  FTD physics.
- `conditional_evidence`: the implementation or selected construction passes
  only under its declared price and cannot support a stronger claim.
- `boundary_confirmed`: a negative, invalid, or obstruction result is preserved
  as a tested boundary, not converted into a positive scenario.
- `open_blocked`: required evidence is absent and the scenario remains inert.
- `invalid_retired`: activation is rejected on every path.

The native scenario physics mask is authoritative. JavaScript may supply
numerical setup values such as `dt` and softening, but cannot add or remove a
physics toggle from the registered scenario profile. CUDA is no longer listed
for a complete Effective Lab scenario because the registered relativistic
momentum integrator lacks CUDA qualification; individual CUDA pair kernels do
not imply a qualified end-to-end scenario.

## 2. Scientific state of record that controls the rebuild

| Record | What Scale 1 may use | What Scale 1 must not claim |
|---|---|---|
| FTD-1023, v3 constitution | finite complete records, one ordinal tick, ternary manifestation as a quotient, one target-blind common action | primitive continuous particles, primitive real-valued phase space, or a second independent law |
| FTD-0760 | a certified finite-time selected M3 matter family at its registered scope | asymptotic stability, generic particlehood, or physical species |
| FTD-0761–0763 | a translating relational-core witness and a repaired fractional-center observer | fully balanced momentum or rigid co-motion of the complete field |
| FTD-0764 | rigid complete-field co-motion is closed negative at the registered scope | a bead carrying one frozen field coat |
| FTD-0765–0767 | the residual field is mostly under-entrained; the earlier wake reading required correction | a demonstrated persistent wake or radiation channel |
| FTD-0768 | descriptive long transport exists, but the registered execution is invalid at the reverse-recovery gate | long-horizon identity or cleared-region response at verdict grade |
| FTD-0399 | the target-blind particlehood campaign is invalid because manifestation portability failed | any native particlehood or mass observable from that campaign |
| FTD-0110 | the recorded derived substrate cluster-scaling/bridge results, with the physical cluster-to-SM-mass identification kept `[STRONGLY MOTIVATED CONJECTURE]` | a Standard Model mass derivation or universal physical mass law |
| FTD-1007 | ternary mask extension is an owner-adopted candidate with exact structural consequences | production implementation, stability, genesis viability, or a new primitive |
| FTD-1012 | diagnostic evidence that a bath term can brake/dissociate topological matter while wave remnants retain speed | a verdict-grade universal matter/radiation theorem or a validated production drag law |
| FTD-1013–1022 | selected geometric free-fall operator, sourced-well wiring, clocks/falling compatibility, and explicit probe backreaction limitations | production-default GR, strong equivalence, physical `G_N`, `1/r^2`, or lensing; FTD-1020 is a wave-lensing structural null |
| FTD-1024 | conditional tensor composition once independent conjunction/joint-label completeness is priced | native entangled preparation, exchange statistics, CAR/Fock, QFT, or Bell recovery |

These records are inputs to the product language and test gates. They are not a
request to copy research-only operators into production.

## 3. Definition of “discrete particle physics” at Scale 1

Scale 1 is discrete in transaction time, record cardinality, identity/event
bookkeeping, and its connection to the finite substrate. The current
`ParticleEngine` uses continuous floating-point position, momentum, mass, and
analytical forces. Those are **effective variables** `[IMPOSED representation]`,
not primitive v3 state.

The UI must always expose both clocks:

- **global tick** — the ordinal transaction count;
- **effective step `dt`** — an imposed numerical integration interval.

Neither is automatically proper time or an internal material clock. A material
clock panel may display only a named, qualified observer and its provenance.

## 4. Target user experience

### 4.1 Unified scenario selector

The Scale-1 toolbar contains one scenario selector populated from the complete
34-row native manifest. Scenario families remain option groups for navigation,
but there is no workspace filter or second selector. Each scenario declares its
own dynamics owner—read-only `NativeMatterObserver`, native C++
`ParticleEngine`, or parametric catalog—and selection performs the required
hard reset and visible provenance transition. Histories from unlike dynamics
owners are never appended to one chart.

### 4.2 Primary panels

1. **Identity and provenance**
   - source scale, scenario, tick, backend, record/candidate revision;
   - source kind: selected relational matter, effective seed, or
     catalog reference;
   - epistemic status and qualification/failure badges;
   - exact list of unavailable or discarded fields.
2. **Core, dressing, and environment**
   - manifested support and relational core;
   - selected bound field, actual field, residual/outgoing/background channels;
   - no rigid-coat or wake label unless the active observer licenses it.
3. **Motion and clock**
   - global tick, effective `dt`, center trajectory, internal observer when
     available, momentum/accounting status, and chart-frame definition;
   - integer/fractional center choice shown explicitly.
4. **Interaction and event ledger**
   - force or common-action transaction by source;
   - contact, formation, dissociation, and removal events as named
     event types rather than inferred particle-count changes.
5. **Conservation and coverage**
   - energy/momentum channels with a coverage mask;
   - an aggregate “total” exists only when every active conservative term has
     a matching potential/ledger contribution;
   - non-conservative terms identify their sink/source.
6. **Effective controls**
   - visible only in Effective Lab;
   - verified baseline, selected extensions, and experimental/quarantined terms
     are separate groups—never one undifferentiated “all physics” switch.

### 4.3 Scenario families

| Family | Initial content |
|---|---|
| Qualified observations | finite-time M3 family replay; translating relational-core witness; fractional-center observer comparison |
| Boundary/falsification | rigid-coat closed-negative comparison; corrected wake interpretation; long-transport invalidity; target-blind particlehood invalidity |
| Effective dynamics | two charges; force-balanced orbit; promoted-style pair; three-body system |
| Quantum controls | exchange eligibility/null/range; spin-orbit and dipole orientations; Lorentz sign controls; radiation sink; relativistic counterstream; three-color toy |
| Catalog | empty reference scene with opt-in `[PARAMETRIC]` injection |

Recorded research evidence must be visually distinct from a live production
scenario. A replay shows artifact identity and cannot be edited into a new
claim.

## 5. Shared domain architecture

### 5.1 One Scale-1 snapshot schema

Create a shared C++ domain schema, consumed by both the native app and WASM:

```text
Scale1Snapshot
  core: tick, dynamics_owner, mode, backend, scenario, source_revision
  objects[]: identity, kinematics, support summary, provenance, qualification
  fields[]: actual/bound/residual/outgoing/background observer channels
  forces[]: term, vector, status, source, conservative, accounted
  events[]: tick, type, participants, transaction/source record
  conservation: channel values, coverage mask, drift eligibility
  capabilities: available controls/observers plus reason when unavailable
```

The schema contains no UI strings as physics truth. Labels and tooltips are
generated from a table-backed registry whose IDs are shared with tests.

### 5.2 Dynamics-owner separation

- `NativeMatterObserver` reads a coherent Scale-0 boundary snapshot or a frozen
  registered artifact. It cannot call `ParticleEngine::tick()`.
- `ParticleEngine` owns Effective Lab dynamics only.
- `Scale1Adapter` hosts the active owner and publishes the common snapshot.
- Native and web controllers submit commands; neither computes forces,
  conservation quantities, identity margins, or epistemic status.

### 5.3 Retired handoff boundary

Scale-1 snapshots contain no runtime cross-scale transfer record. The former
projector and loss ledger were removed with the handoff UI. The retained
`OnticEntity` is a scale-local presentation summary only; it is not a
navigation command, transfer record, or state-complete bridge. Scale Context
supplies the pedagogical comparison.

## 6. Physics-module disposition

| Current module/term | Redevelopment action | Product status |
|---|---|---|
| Coulomb pair force | keep in Effective Lab; verify signs, softening, reciprocity, decomposition, and energy | form/coupling statuses shown separately |
| Newton pair gravity | keep as effective comparison only; do not conflate with Scale-0 `geometric_gravity` | `[IMPOSED]/[SMC-floor]` at recorded scope |
| Scale-0 geometric gravity | expose as source-observer/reference channel; no automatic point-particle port | selected, default-off source operator |
| damping | default off; move to an environment/bath experiment with explicit energy sink | `[IMPOSED]`; FTD-1012 diagnostic linked, not promoted |
| relativistic momentum Verlet | retain as an imposed integrator after velocity/momentum synchronization is fixed and tested | numerical method, not Lorentz recovery |
| isotropic `relativistic` force rescale | retire from dynamics; it is non-covariant and duplicates the better momentum path | retired visual approximation |
| contact removal | replace unconditional removal with an explicit event-law selection, default off; publish participants and removed energy | `[SELECTION]`, not annihilation, QED decay, or pair production |
| exchange/Pauli force | remove from verified physics; quarantine until exchange statistics exists | experimental toy; FTD-1024 leaves exchange open |
| strong/color force | quarantine behind an experimental profile; injected color labels remain parametric | imposed toy, no native-QCD claim |
| Lorentz/dipole/spin-orbit | retain only in an experimental effective-extension group with complete force/accounting tests | imported/imposed |
| radiation reaction | quarantine until the energy sink and emitted channel are both represented | imported/imposed, currently incomplete |
| catalog masses/species | reference-only by default; opt-in injection carries a persistent `[PARAMETRIC]` badge | no emergence claim |
| cluster mass mapping | keep only with source cardinality, mapping revision, and FTD-0110 scope shown | derived substrate scaling; physical mass identification remains `[SMC]` |
| Bell/entanglement/QFT | no runtime claim; an information panel may state the FTD-1024 conditional theorem and open physical recovery | open physical implementation |
| ternary mask extension | research replay/diagram only until separately adopted into an executable candidate | selected, unimplemented |

The “enable all verified physics” profile may enable only terms whose force,
state dependency, backend support, and diagnostics coverage all pass. It must
not enable quarantined terms merely because code exists.

## 7. Work packages

### R0 — Freeze and reconcile the current surface

**Purpose:** establish a trustworthy baseline before structural edits.

- Finish audit gates S1-02 through S1-10 from the companion audit.
- Inventory both frontends, the native `Scale1Adapter`, WASM bindings, C++
  `ParticleEngine`, scenarios, charts, inspector, and both retired handoff
  implementations.
- Record current defects without importing findings from deleted JS code.
- Resolve the velocity/momentum synchronization defect when toggling
  relativistic Verlet.
- Add conservation coverage flags before any UI redesign.
- Freeze golden output and representative screenshots/artifacts.

**Exit:** every retained current behavior is classified keep, repair,
quarantine, or retire, with a test owner.

### R1 — Build the shared scientific registry and snapshot

- Add table-backed `Scale1CapabilitySpec`, `Scale1PhysicsSpec`, and
  `Scale1ObserverSpec` registries.
- Add shared provenance, epistemic status, coverage, and unavailable-reason
  enums—no free-form status inference in JavaScript/RML.
- Expand the native `Scale1Snapshot`; bind the same schema through WASM.
- Add schema-version and source-revision fields.
- Make capability negotiation fail closed on CPU/GPU/WASM differences.

**Exit:** native and web contract tests deserialize the same fixture and render
the same statuses, controls, and unavailable reasons.

### R2 — Implement Native Matter Observatory

- Define coherent Scale-0 observation boundary records for matter core,
  constituent relation, field channels, observer center, and qualification.
- Implement live observation for fields already available in production.
- Implement immutable registered-artifact replay for research-only M3/M4
  evidence that cannot be generated by production defaults.
- Add actual/bound/residual/outgoing/background render layers.
- Add explicit qualification and failure panels.
- Prevent observer mode from mutating the source engine.

**Exit:** the primary Scale-1 mode can inspect a qualified source record without
constructing or ticking an effective point particle.

### R3 — Retired promotion/comparison bridge

This package was implemented, then retired on 2026-09-01. The transfer action,
projector, ledger, scenarios, overlays, and dedicated tests are absent. Scale
Context replaces only its pedagogical role and never creates simulation state.

### R4 — Harden the Effective Particle Lab

- Repair transaction schedule and classical/relativistic state coherence.
- Make contact events explicit and default off.
- Add per-term force/potential/accounting contracts.
- Retire the non-covariant isotropic relativistic rescale.
- Move advanced imported toys behind an experimental profile.
- Rename overclaiming scenarios (`s1-hydrogen-cloud` ->
  `s1-effective-charge-cloud`; “electron-style” remains style, not identity).
- Unify scenario definitions for native and web; remove parallel seed mirrors.
- Wire the existing CUDA particle backend only after CPU equivalence and
  snapshot parity pass.

**Exit:** each enabled term changes the live kernel, decomposes correctly,
reports honest conservation coverage, and behaves identically through native
and WASM within declared tolerances.

### R5 — Rebuild the Scale-1 UI

- Implement the four-mode toolbar and mode-scoped scenario registry.
- Replace the flat toggle list with verified, selected, and experimental
  groups driven by the shared capability registry.
- Add Identity, Core/Dressing, Motion/Clock, Event Ledger, and Conservation
  panels.
- Keep charts tick-indexed; offer windowed/all-history modes without coupling
  chart sampling to render FPS.
- Ensure all side panels are single-column when floating below the declared
  wide-layout breakpoint and remain fully scrollable above the status bar.
- Add hover explanations sourced from registry status text.

**Exit:** every visible value has a producer ID, freshness tick, units/frame,
and epistemic status.

### R6 — Cross-platform, performance, and deployment qualification

- Qualify CPU native, native CUDA where applicable, WASM32, WASM64, and
  threaded WASM separately.
- Measure simulation tick rate and presentation FPS independently.
- Require 60 FPS presentation across the declared interactive workload matrix;
  publish hardware renderer provenance and reject software rendering as a
  hardware pass.
- Establish particle/object-count QoS levels from measurements rather than
  silently slowing physics while FPS remains high.
- Verify native/web snapshot parity, source revision, and deployed WASM
  `build_info.json` identity.
- Run the full focused CTest/browser suite, golden gate, documentation links,
  and epistemic-string audit.

**Exit:** the audited commit is reproducible, deployed artifacts match it, and
all unsupported capability combinations fail closed with an explanation.

### R7 — Retire superseded Scale-1 code

- Remove old UI/controller branches only after feature and artifact parity.
- Preserve the July audit and pre-redevelopment reference as provenance.
- Archive, do not delete, any theory-facing design document whose status is
  superseded or closed.
- Delete implementation code only after `rg` ownership proof, build/test
  coverage, and a migration note establish that it is neither used nor
  intended.

**Exit:** one scenario registry, one snapshot schema, one force owner per mode,
one status registry, and no dead control or fabricated telemetry channel.

## 8. Required test matrix

### Native domain tests

- record admissibility and transaction fail-closed behavior;
- exactly one ordinal tick per transaction;
- deterministic replay and pause/step/run equivalence;
- velocity/momentum synchronization across integrator changes;
- force isolation, pair reciprocity, and decomposition equality for every term;
- potential/ledger coverage and drift eligibility;
- simultaneous contact-event determinism and identity cleanup;
- handoff symbols and UI entry points remain absent;
- Scale-0 observer coherence at one published boundary;
- CPU/CUDA equivalence where the capability registry says both apply.

### Frontend contract tests

- native and web render the same fixture status and capability set;
- unavailable controls are disabled with a reason, never silently ignored;
- mode switches clear incompatible histories and dynamics state;
- charts advance on source ticks, not animation frames;
- inspector, overlay, diagnostics, and status bar agree on object identity and
  freshness;
- all scenarios load, reset, and leave no stale overlays;
- floating/docked panels remain visible, scrollable, and above the status bar.

### Scientific claim gates

- no `[THEOREM]`, `[DERIVED]`, or `[EMERGENT]` label without a canonical source;
- no catalog identity applied to a native observer record;
- no total-energy/drift claim with incomplete coverage;
- no proper-time/internal-clock label for bare `dt` or global tick;
- no Pauli/QCD/QED/GR/Bell claim from an imposed toy;
- closed-negative and invalid evidence remains visible and cannot be selected as
  a successful live scenario.

## 9. File ownership map

| Responsibility | Primary location |
|---|---|
| shared Scale-1 records/registries | `engine/include/ftd/scale1/`, `engine/src/scale1/` (new) |
| effective dynamics | `engine/include/ftd/particle_engine.h`, `engine/src/particle_engine.cpp` |
| Scale-0 observer | shared Scale-1 module plus narrow read-only source-record API |
| native host | `engine/native/**/scale1_adapter.*`, shared snapshot/commands |
| WASM bindings | `engine/wasm/bindings_particle.cpp` plus Scale-1 snapshot binding |
| web presentation | `engine/web/js/scales/scale1/`, Scale-1 panel descriptors, viewport adapter |
| scientific audit | `docs/audits/AUDIT_2026-08_scale1-discrete-particle-physics.md` |
| canonical status cross-walk | `docs/theory/05_particles/REF_SCALE1_DYNAMICS_FTD_FORM.md` |

The new shared module must not import frontend code. Frontends must not
reimplement forces, observers, conservation accounting, or
claim classification.

## 10. Commit sequence

Use small, reviewable commits in this order:

1. `docs(scale1): ratify redevelopment contract and module dispositions`
2. `fix(scale1): close schedule and conservation-coverage audit gates`
3. `feat(scale1): add shared snapshot and physics registry`
4. `feat(scale1): add native matter observer`
5. `refactor(scale1): unify native and web scenarios`
6. `fix(scale1): harden effective particle transactions and retire rescale`
7. `feat(scale1): rebuild panels and overlays from shared capabilities`
8. `perf(scale1): qualify native CUDA and WASM workloads`
9. `refactor(scale1): retire superseded implementation paths`
10. `docs(scale1): close audit gates and record deployed artifact identity`

Do not combine physics-law changes with large UI/CSS migrations. Each physics
commit must have its native tests before a frontend begins consuming it.

## 11. Definition of done

Scale 1 is complete only when all of the following are true:

- Native Matter is the primary default and does not advance an independent
  point-particle universe.
- Effective Lab is visibly and mechanically separated from native-matter
  observation.
- Every displayed object has source provenance and an epistemic status.
- Runtime scale-handoff entry points and transfer state are absent.
- Every active force has one dynamics owner, test isolation, backend parity,
  and accounting coverage.
- No incomplete Hamiltonian produces a “conserved total energy” or drift claim.
- Global tick, effective `dt`, and any internal/material clock are distinct.
- Invalid, closed-negative, and open findings are presented as boundaries, not
  hidden or converted into positive scenarios.
- Native and web consume the same versioned snapshot/capability contract.
- The declared workload matrix holds at least 60 FPS presentation while tick
  throughput is reported separately.
- Focused native, CUDA, WASM, browser, golden, link, and epistemic audits pass.
- The deployed artifact identity matches the audited source commit.
- Superseded code has been removed or archived only after ownership proof.

## 12. Implemented executable slice

The architectural slice is now implemented:

1. S1-02 clock/schedule and S1-07 conservation-coverage defects are repaired;
2. `Scale1PhysicsSpec`, the versioned `Scale1Snapshot`, and the canonical
   scenario/capability registries are shared across native, WASM, and web;
3. the legacy cloud is explicitly an effective charge cloud;
4. Effective Lab dynamics are separated from Native Matter observation;
5. the registered FTD-0760 M3 artifact is available through six read-only
   observer views; and
6. the live registry contains only executable rows.

The next scientific gate is cross-tick identity continuity and qualified event
detection. Any future scenario must acquire an executable owner and validation
path before it enters the live manifest.
