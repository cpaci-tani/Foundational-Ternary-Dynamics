# Scenario Qualification — Scale 0 `empty` Baseline v1

**Record status:** **[OPEN]** qualification in progress; partial measurements
are recorded below and do not yet authorize a scenario-level claim.

**Scenario ID:** `empty`

**Contract version:** `1`

**Audit opened:** 2026-08-29

**Audit disposition:** `in-progress`

This is a null-control experiment. Passing it may establish exact invariance of
the implemented zero record under a declared finite configuration. It cannot
establish vacuum physics, zero-point energy, or physical emptiness.

## 1. Frozen identity

| Field | Recorded value |
|---|---|
| Display name | Empty Lattice — Null Control |
| Scale | Scale 0 — lattice/substrate |
| Model relation | native operator |
| Experimental role | null control |
| Record lifecycle | open |
| Input/claim status | all-zero initial record is **[IMPOSED]**; invariance qualification is **[OPEN]** |
| Registry source | `engine/web/js/scales/scale0/scenario-registry.js:183-193` |
| Initialization owner | fresh `RenderBridge` construction/reset followed by the `empty` profile; `scenarios.cpp:230-240` only disables terms |
| Existing behavior test | `engine/tests/test_scenario_behavior.cpp:811-829` |
| Declared toggle profile | `engine/web/js/config/toggles.js:353` |
| Declared boundary | periodic, `engine/web/js/config/toggles.js:574` |
| Historical aliases | none |

## 2. Scientific contract

### Mathematical model

- Finite state: a fresh/reset bridge supplies the default voxel record. The
  qualification serializer covers ternary storage, `J`, wave velocity, dual
  fields, matter velocity, remainder, latency, tau, phase, locks and IDs,
  spin/color/flavor, acceleration, strong/weak fields, scalar potentials,
  force buffers, active indexes, and event counters. Legitimate sentinels such
  as `particle_id = -1` and `pair_id = -1` are recorded as sentinels, not zero.
- Production profile: `configure_static_seed_terms`; every production term is
  disabled so the null is not the dashboard's full default stack applied to a
  zero field.
- Update: run the ordinary deterministic finite tick schedule with the declared
  zero profile.
- Expected invariant: declared dynamical lattice degrees remain at their exact
  initialized null/sentinel values. Clock, provenance, scheduler, and energy-
  ledger bookkeeping fields advance under separate tick-indexed expectations.
- Native units: lattice sites and ordinal ticks. No physical calibration is
  applied or inferred.

### Claims

| Claim ID | Exact wording and scope | Epistemic tag | Supporting gates/evidence | Limitations | Disposition |
|---|---|---|---|---|---|
| `empty-native-null-invariance` | The declared dynamical lattice fields remain at their initialized null/sentinel values for each configuration that passes its recorded resolution, boundary, execution-path, lifecycle, and race protocol. | **[OPEN]** | Gates 2–4; native CPU and focused browser evidence are partial | Applies only to tested finite configurations; excludes physical-vacuum interpretation and nonzero scenarios | proposed |

| Prohibited claim ID | Exact prohibited wording/scope | Reason |
|---|---|---|
| `empty-is-physical-vacuum` | This scenario establishes physical vacuum, zero-point energy, cosmological vacuum, or absence of all ontology. | The scenario is an **[IMPOSED]** finite all-zero control. |
| `empty-validates-nonzero-scenarios` | Passing the null control validates any nonzero scenario or physical identification. | A control verifies only its own declared implementation contract. |

- Integrity observables: the native qualification target implements a
  schema-versioned fieldwise dynamical-state digest over named persistent
  fields, normalizes signed zero, and excludes raw padding, clocks,
  bookkeeping, telemetry, RNG/counters, and temporary scratch. Direct
  reductions over the same stored arrays are redundant integrity checks, not
  independent evidence. Executable probes check signed-zero normalization,
  clock/bookkeeping exclusion, and flux sensitivity. Any independent oracle
  must be separately implemented and identified before it is counted as such.

## 3. Frozen acceptance protocol

The following tiered grid is preregistered before new candidate measurements:

- Product-path resolutions: `L = 9, 17, 33, 65, 97`. Native-core-only evidence
  may additionally use requested `L = 8`; every result records requested and
  actual `L` and fails on a silent mismatch.
- Boundaries: periodic, reflective, and dispersal.
- Short invariant matrix: every resolution/boundary combination at tick
  checkpoints `0, 1, 2, 16`.
- Long invariant matrix: product `L = 9, 17` through ticks `256, 4096`, and
  `L = 33`
  through tick `256`, for every boundary.
- Large-domain operating envelope: `L = 65, 97` uses the short invariant
  matrix plus timed foreground runs. A separately recorded campaign is required
  before claiming 4096-tick evidence at those volumes; CI must not silently
  substitute a shorter run and call it equivalent.
- Execution paths are frozen separately as compute backend (`CPU` or `GPU`),
  runtime (`native`, `WASM main`, or `WASM worker`), and transport (`in-process`,
  worker messages, or WebSocket). Each verdict records build hash, compiler
  flags, device/driver/browser, availability, and unsupported paths. WASM main
  and worker are CPU compute paths; WebSocket is a transport, not a backend.
- Lifecycle repetitions: first load, reload, resize/reinitialize, rapid
  `empty -> nonempty -> empty` switching, hidden-tab recovery, and teardown.

Current execution-path evidence remains deliberately non-equivalent:

| Compute | Runtime | Transport | Current Scenario 1 evidence |
|---|---|---|---|
| CPU | native | in-process | deep fieldwise state, diagnostics, audit, ledger, Lagrangian, gravity, and digest checks |
| CPU | WASM main | in-process/Embind | browser-visible exact-null and lifecycle checks on the explicit `?engine=wasm` path; no canonical digest export |
| CPU | WASM worker | worker | browser-visible exact-null, lifecycle, and pacing checks on the explicit `?engine=wasm` path; no canonical digest export |
| GPU | native CUDA | in-process | shared device-resident digest/counters pass ordinary and interactive modes for all three boundaries through tick 64 without a full state mirror |
| GPU when served by the CUDA server | native | WebSocket | protocol-v2 smoke exists, but it does not exercise `empty` or expose the canonical digest |

The checked-in WASM bundle was rebuilt from a clean source revision with
Emscripten 5.0.2 for wasm32, wasm64/Memory64, and wasm32-threads. Its
`build_info.txt` records the exact source commit, compiler identity, variant
flags, and SHA-256 of all six generated modules. The browser qualification was
rerun against that generation after the execution path was pinned explicitly;
an available native WebSocket server can no longer silently replace a claimed
WASM run.

Required checks:

1. the canonical fieldwise dynamical-lattice hash is unchanged from tick 0
   within one `(actual L, boundary, compute, runtime, transport)` configuration;
2. the execution-contract hash records those configuration fields and is not
   expected to match across different configurations;
3. clock/bookkeeping records match exact tick-indexed expectations;
4. specifically named activity channels are zero: state counts, field/velocity
   maxima, charge/events, and `EnergyAudit` field, wave, particle, dynamic, and
   audit-total energies. `Diagnostics::total_energy` and Born-Infeld/
   Lagrangian totals carry a state-independent observer baseline and must remain
   finite and match that documented baseline, not zero;
5. nonzero neutral defaults such as cell volume and gravity/projection identity
   sentinels are checked against their named expected values;
6. all reported values are finite and origin-labeled;
7. resize and reload are idempotent;
8. stale callbacks, workers, samplers, and telemetry from the intervening
   nonempty scenario cannot mutate the restored null;
9. collapsed/hidden panels perform no polling, layout, chart drawing, or field
   extraction;
10. after steady state, the null control performs no scenario-specific
   allocation or overlay sampling;
11. presentation decimation does not change tick count or scientific state;
12. on the reference machine/browser at `L = 97`, the foreground UI is measured
    against the 16.67 ms frame target with frame-pacing evidence, not an average
    FPS label alone.

The all-panel coexistence campaign now covers all 17 visible Scale 0 panel
interfaces at `L = 97`. Each panel was warmed, measured for at least 240
foreground frames, collapsed, and checked for stopped work using absolute gates
(`FPS >= 59`, `p95 <= 16.9 ms`, `p99 <= 25 ms`, no long tasks, and bounded
callback/DOM/canvas/resource deltas). All 17 recorded 60.00 FPS with p95/p99
16.67 ms and zero collapsed DOM/canvas work. The historical
`perf-current-results.json` full-physics `L=33` result is approximately 12 FPS
and uses a relative-regression gate; it is neither Scenario 1 evidence nor an
acceptable substitute for the absolute 60 FPS contract.

Any non-null dynamical lattice field, backend disagreement outside a preregistered
tolerance, stale-generation mutation, non-finite telemetry, or presentation-
dependent scientific result is a failure. Performance failure blocks UI
qualification but does not rewrite a scientific zero/nonzero result.

## 4. Gate ledger

| Gate | Status | Current evidence | Blocking work |
|---|---|---|---|
| 1. Static trace | in-progress | registry, reset + empty-profile initializer, toggle profile, boundary, native CPU target, explicit WASM main/worker paths, clean build identity, and knowledge-base wording located and aligned | complete export metadata and remaining native-WebSocket trace |
| 2. Mathematical well-posedness | in-progress | dynamical-null invariant, evolving bookkeeping, observer baseline, prohibited claims, and the schema-versioned named-field digest domain are separated; CPU and CUDA share one parallel two-lane combination contract with exact nonfinite/nondefault counters | publish the boundary proof and extend the canonical digest contract through WASM and WebSocket surfaces |
| 3. Numerical validity | in-progress | `empty_scenario_qualification` passes the native CPU product-size short matrix at `L=9,17,33,65,97`, all three flux boundaries, and checkpoints `0,1,2,8,16`; its preregistered long matrix passes `L=9,17` through ticks `256,4096` and `L=33` through tick `256` with invariant fieldwise digests; the WSL2 CUDA sibling passes ordinary/interactive modes and all three boundaries through tick 64 with exact CPU/GPU populated-state hash parity, 32-byte result transfer, and zero full-mirror calls; the clean, source-stamped WASM worker passes lifecycle/null checks at product sizes through `L=97`; the clean WASM-main path passes exact-null checks through 16 synchronous ticks and rapid-generation switching | native-WebSocket evidence and WASM/WebSocket canonical-digest parity remain open |
| 4. Scientific validity | not-started | null claim and falsifiers frozen | run controls without post-hoc tuning |
| 5. Scale appropriateness | in-progress | the imposed finite lattice record belongs to Scale 0; exact-ID gating correctly excludes Standard Model overlays | replace unqualified particle/void nouns, remove stale Scale-up implications, and verify all generic panel applicability states |
| 6. UI and interpretive truth | in-progress | focused browser test passes picker/current-state truth, unsupported-latency distinction, finite observer baseline, collapsed overlay demand, and WASM-main/worker origin distinction; the knowledge entry now states the imposed null-control and non-vacuum boundary; Diagnostics renders unresolved values as `—` while retaining exact measured zero; Lagrangian exposes the state-independent observer reference, baseline-subtracted `Δℒ`, stale/unavailable states, and an explicit non-vacuum/non-zero-point limitation; Flux Slice, Spectrum, and Gravity declare `empty` inapplicable and stop their scientific sampling/coordinator paths | live mutation does not yet suspend the displayed qualification; upstream telemetry still erases some unavailable diagnostics/audit/Lagrangian values into zero before formatting; remaining panel applicability, export metadata, and native-WebSocket presentation paths remain open |
| 7. Performance and operational safety | in-progress | the final full WASM-worker coexistence campaign passes all 17 visible panels at `L=97`: every panel records 60.00 FPS with p95/p99 16.67 ms, all absolute fixed-work/callback budgets pass, no long tasks occur, and every collapsed panel records zero DOM/canvas work; Diagnostics passes with 5 mounted canvases/about 95 mutations; Lagrangian passes with 7 mounted canvases and on-demand action trends; Flux Slice, Spectrum, and Gravity perform zero periodic work for `empty`; rapid generation/reset/resize and five-reset lifecycle probes pass; the clean rebuilt bundle repeats the focused `L=97` pacing result at 60.00 FPS with p95/p99 16.67 ms | genuine hidden-tab evidence is unsupported in the current headless browser and was skipped rather than fabricated; longer foreground campaign and native-WebSocket browser evidence remain open |

## 5. UI trace blockers

The Scale 0 UI audit found the following qualification blockers. These are
recorded here so later interface work cannot silently erase or bypass them:

1. injecting, randomizing, or field-editing the live lattice does not suspend
   the displayed `empty` qualification even though toggle edits do;
2. Diagnostics now preserves unresolved values as unavailable, but upstream
   telemetry paths still convert some unavailable, stale, or nonfinite data
   into numeric zero before the panel receives them;
3. generic Time, Thermodynamics, Dispersion, P1 Observatory, and Knots surfaces
   need explicit `empty` applicability states;
4. Scale 0 labels such as `Particle Count`, `Particle Display`, `void`, and
   `entangled pair` exceed what a ternary manifestation record alone establishes;
5. stale Scale 1/user-guide text still describes a removed `Scale up` action,
   while the latent handoff payload lacks build, backend/runtime/transport,
   boundary, toggle, mutation, source-epoch, and qualification provenance;
6. no live scientific export/share implementation exists. A JSON-first export
   must preserve run provenance, per-channel availability, exact observer
   baselines, and the canonical digest; stale export claims in the user guide
   are not evidence that such a path exists;
7. global FAQ text about vacuum and ternary state predates the strict v3
   complete-record/manifestation-quotient distinction and must not be read as
   Scenario 1 evidence.

These items are fixed and verified one interface at a time. A passing frame-rate
measurement does not waive an interpretive-truth failure.

## 6. Disposition

**Disposition:** `not-set`

**Next scenario may open:** `no`

The next scenario remains blocked until every gate above has a reproducible
verdict and the canonical manifest, implementation, tests, UI, export metadata,
documentation, and scenario atlas agree.
