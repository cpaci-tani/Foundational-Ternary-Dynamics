# AUDIT_2026-08_scale1-discrete-particle-physics

**Date:** 2026-08-31
**Status:** `[IMPLEMENTED — LOCAL PHYSICS/SCENARIO QUALIFICATION COMPLETE; PERFORMANCE AND DEPLOYMENT OPEN]`
**Scope:** the live Scale-1 particle path: native `ParticleEngine`, WASM adapter, read-only Native Matter observation, and Scale-1 scenarios/controls/telemetry/rendering.
**Precedence:** `LEDGER.md` > active v3 constitution > `REF_SCALE1_DYNAMICS_FTD_FORM.md` > this audit.

**Redevelopment plan:** `engine/docs/PLAN_SCALE1_PARTICLE_CONTEXT_V4.md` converts
this gate audit into the target architecture and implementation sequence.

## Audit question

Does Scale 1 implement an honest particle-context approximation of the strict-discrete framework, with every active law wired to the state it claims to advance, every displayed observable sourced from that same state, and every imported or imposed element labeled without promotion?

This is not an audit of whether the current N-body model derives Standard Model particle physics. It does not. The live engine is an effective continuous-coordinate approximation advanced on a discrete ordinal tick. Its analytical forces and integrator are imported/imposed unless a narrower source explicitly establishes a stronger result.

## Current ownership boundary

- Native Matter owner: read-only `NativeMatterObserver` registered replay through the shared Scale-1 domain and native adapter.
- Effective dynamics owner: `engine/src/particle_engine.cpp` and `engine/include/ftd/particle_engine.h`.
- Browser bridge: `engine/wasm/bindings_particle.cpp` -> `engine/web/js/bridge/native-particle-engine.js` -> `engine/web/js/bridge/capabilities/scale1.js`.
- Scenario/controller owner: `engine/web/js/scales/scale1/`.
- Shared contract: `engine/include/ftd/scale1/domain.h` and `engine/src/scale1/domain.cpp`.
- Catalog particles: `[PARAMETRIC]` Zoo inputs, not emergent lattice objects.
- Retired paths: the former JS force engine, cross-section panel, decay-rate panel, Hawking toy, Scale-0/Scale-1 transfer pipeline, and legacy Scale-1/Scale-2 proximity handoff remain non-live. Their historical findings are provenance, not current-code findings. Pedagogical scale explanation belongs to the Scale Context sidepanel.

## Required gates

| Gate | Question | Required evidence | Status |
|---|---|---|---|
| S1-00 Context and ownership | Is there one live dynamics owner, and is the effective/primitive boundary explicit? | Source inventory, bridge trace, epistemic wording audit | **PASS** |
| S1-01 Record admissibility | Can invalid effective records enter or survive a transaction? | Native injection/control rejection plus pre/post-tick validation tests | **PASS** |
| S1-02 Clock and schedule | Does one call produce one deterministic ordinal transaction, with coherent classical/relativistic state? | Tick-order tests, pause/run parity, momentum/velocity transition tests | **PASS** |
| S1-03 Coulomb and gravity | Do signs, softening, action/reaction, diagnostics, and active constants agree? | Exact-pair tests, decomposition equality, conservation window | **PASS** |
| S1-04 Advanced forces | Does each of exchange, strong, Lorentz, dipole, spin-orbit, and radiation activate only on its documented record fields? | Toggle-isolation and interaction matrix; per-module validation evidence/criterion; native-registry Physics card inside Controls; no dead channels | **PASS — KERNEL-VALIDATED, PHYSICALLY QUARANTINED** |
| S1-05 Integrator and constraints | Are Verlet variants, damping, speed projection, locking, and singular limits honest and stable? | Reversibility/conservation tests with explicit non-conservative exclusions | **PASS** |
| S1-06 Contact and lifetime | Is contact removal deterministic, index-safe, and labeled as a selection rather than QED decay? | Contact geometry, simultaneous pairs, record-map cleanup | **PASS** |
| S1-07 Observables | Do energy, momentum, angular momentum, force decomposition, charts, and inspector read the integrated state? | Producer-consumer equality, all nine force-decomposition renderer channels, overlay containment/accessibility, and paused/live cadence tests | **PASS** |
| S1-08 Handoff retirement | Are runtime transfer entry points, hidden transfer state, and heuristic identity mappings absent? | Symbol/UI non-use audit, build ownership, scenario-manifest absence, retirement record | **PASS** — both the user-visible Scale-0/Scale-1 transfer and legacy Scale-1/Scale-2 proximity initializer are removed; read-only observation and direct per-scale scenarios remain separate |
| S1-09 Scenarios and controls | Does every registered scenario execute through its declared owner? | 39-row runnable-only native manifest, unified-selector coverage, per-row verdict/evidence/criterion, native-owned physics masks, and 39 runtime/profile checks | **PASS** — every registered scenario uses the exact native-owned profile and remains finite; all 48 non-executable roadmap rows were retired from the live manifest; quantum/QED references preserve their imported/effective boundaries |
| S1-10 Performance and artifact | Does the deployed WASM artifact remain responsive and match the audited commit? | Workload matrix, hardware provenance, WASM build identity | **PARTIAL** — all local WASM variants rebuilt; hardware matrix and deployed commit identity remain open |

## Closed findings

### S1-F001 — continuous effective particles were mislabeled as an axiom

The `Particle` record was described as `[AXIOM]`, and the cross-scale `OnticEntity` triple was described as state-complete at every scale. Both statements conflict with v3: primitive state is a finite complete record; Scale 1 is a coarse-grained continuous-coordinate approximation.

**Resolution:** comments now label the Scale-1 record `[IMPOSED representation]`, identify continuous coordinates/analytical forces as effective, and describe `OnticEntity` as a compact bridge/presentation projection rather than a complete primitive record.

### S1-F002 — invalid native records could propagate NaN/Inf through the force kernel

The C++ API accepted zero/negative/nonfinite mass, nonfinite vectors, nonpositive/nonfinite `dt`, and invalid softening/radius values. The JavaScript adapter rejected only nonpositive mass, so a bad native or WASM call could enter divisions by mass or contaminate all later diagnostics.

**Resolution:** native setters and injection reject inadmissible scalar/vector inputs. `validate_state()` checks the entire effective record and force-buffer cardinality before and after every tick; the transaction throws without advancing the ordinal counter on invalid state. WASM-only radius, spin-axis, and velocity mutation paths validate their inputs too.

### S1-F003 — the non-covariant relativistic force rescale was live

The former `relativistic` toggle isotropically rescaled force by a gamma factor while a separate relativistic-momentum Verlet path also existed. The rescale was neither covariant nor state-complete and could be enabled by bulk profiles.

**Resolution:** the rescale is retired and unavailable in the shared registry; attempts to enable it fail closed. Relativistic-momentum Verlet is the verified numerical baseline, with velocity/momentum synchronization tested across profile transitions.

### S1-F004 — contact removal was implicit and overclaimed

Opposite effective charges inside their radii were removed without an explicit selected-event profile or complete identity/accounting record.

**Resolution:** contact removal is an explicit `[SELECTION]`, default OFF. Candidate pairs are sorted deterministically, disjoint events preserve stable participant IDs, surviving pair references are cleaned, and multi-event batches refuse per-event energy-accounting completeness.

### S1-F005 — promotion and conservation claims were reconstructed in JavaScript

The browser owned the cluster-to-particle conversion and presented partial active-potential sums as though every enabled force had a complete energy channel.

**Resolution:** conservation ownership moved into shared C++ and no longer depends on a browser reconstruction. The intermediate projector/ledger implementation was subsequently retired with the complete handoff feature. Snapshots still carry covered/missing/non-conservative masks; advanced terms without a represented state-energy channel make drift ineligible instead of silently overclaiming conservation.

### S1-F006 — Scale 1 conflated native matter with an effective particle sandbox

The dashboard opened directly into a continuous classical scenario, giving no architectural separation between registered substrate matter evidence and imposed effective-body experiments.

**Resolution:** Scale 1 now opens in a read-only FTD-0760 Native Matter replay and exposes every registered scenario through one selector. The replay leaves mass, charge, spin, statistics, SM identity, and outgoing/radiation channels unavailable. Effective laboratories and catalog records retain distinct dynamics owners and provenance per scenario without a workspace filter.

## Evidence for the implemented gates

- Canonical MSVC 14.44 builds for the seven focused test targets and `native_app` — PASS.
- `test_particle_engine.exe` — **42 passed, 0 failed**, including integrator transitions, simultaneous contact ordering/cleanup, and complete failed-tick rollback.
- `test_scale1_domain.exe` covers registry uniqueness, all-module validation records, all-scenario verdict/evidence/criteria, exact native-owned physics masks, handoff absence, registered replay honesty, schema/provenance transfer, and conservation coverage.
- Focused Scale-1 CTest slice: `scale1_domain`, `particle_engine`, `pe_forces`, `particle_toggles`, `particle_toggles_table`, `particle_lifetime`, and `relativistic_verlet`.
- WASM32, WASM64/Memory64, and WASM32-threaded artifacts rebuilt and staged from the changed binding — PASS. The build identity is intentionally `-dirty`; this is not a deployed-commit certification.
- **Focused Playwright gates cover** the default read-only replay, the unified 39-row runnable-only selector, all setup/seed contracts, twelve Quantum Reference controls, seven QED effective references, the 12-row physics registry, retired-control disablement, conservation coverage, catalog injection, overlays, charts, diagnostics, and tooltips.
- The Scale-1 UI audit additionally verifies one exclusive Physics-panel owner for all 12 switches, correct scenario/read-only hydration, modified-profile signaling, no hidden Scale-5 mount, unique control IDs, complete advanced-force renderer wiring, synchronized `aria-pressed`, bounded scrolling, and no horizontal overflow at the registered desktop/narrow-desktop layouts.
- The r5 qualification contract assigns every physics and scenario row one of
  `contract_qualified`, `kernel_validated`, `conditional_evidence`,
  `boundary_confirmed`, `open_blocked`, or `invalid_retired`, with non-empty
  evidence and a pass criterion. Unavailable rows have a zero physics mask;
  the retired isotropic rescale cannot appear in any scenario mask.
- The 39-row WASM runtime sweep verifies exact toggle-mask parity, finite
  diagnostics after four transactions for interactive rows, and immutability
  for Native Matter rows. This validates implementation behavior only at each
  row's existing epistemic status.
- Effective Lab scenario backend claims are CPU/WASM-only. Native CUDA remains
  unqualified because a pair-force CUDA kernel does not cover the registered
  relativistic-momentum integrator or a complete scenario transaction.
- The former generic `scale_bridge` implementation, its Scale-0/Scale-1 and Scale-2/Scale-5 record converters, and all dedicated bridge campaigns are absent. Each effective engine now starts from its own explicit scenario/reference inputs; Scale Context provides presentation-only comparison.

## Remaining qualification gates

1. Run the declared CPU/WASM workload matrix with tick throughput separated from presentation FPS and hardware renderer provenance recorded; qualify a complete native-CUDA scenario only after integrator and transaction parity exist.
2. Commit, rebuild from that commit, deploy, and verify the live `wasm/build_info.json` identity before closing S1-10.
