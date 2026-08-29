# Scenario Qualification — Scale 0 `empty` Baseline v1

**Record status:** **[OPEN]** preregistration; measurement gates have not run.

**Scenario ID:** `empty`

**Contract version:** `1`

**Audit opened:** 2026-08-29

**Audit disposition:** `not-started`

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

- Integrity observables: a canonical fieldwise dynamical-lattice hash and
  direct reductions over the same stored arrays are redundant integrity checks,
  not independent evidence. Any independent oracle must be separately
  implemented and identified before it is counted as such.

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

Any non-null dynamical lattice field, backend disagreement outside a preregistered
tolerance, stale-generation mutation, non-finite telemetry, or presentation-
dependent scientific result is a failure. Performance failure blocks UI
qualification but does not rewrite a scientific zero/nonzero result.

## 4. Gate ledger

| Gate | Status | Current evidence | Blocking work |
|---|---|---|---|
| 1. Static trace | in-progress | registry, reset + empty-profile initializer, toggle profile, boundary, native CPU target, and browser path located | complete export/help and remaining backend trace |
| 2. Mathematical well-posedness | in-progress | dynamical-null invariant, evolving bookkeeping, observer baseline, and prohibited claims separated | publish canonical fieldwise serializer/hash definition and boundary proof |
| 3. Numerical validity | in-progress | `empty_scenario_qualification` passes native CPU at `L=8,17,33,65,97`, all three flux boundaries, ticks `0,1,2,8,16`; browser worker lifecycle/null checks pass at product sizes through `L=97` | long invariant matrix, WASM-main/native-WebSocket/GPU evidence, and canonical hashes remain open |
| 4. Scientific validity | not-started | null claim and falsifiers frozen | run controls without post-hoc tuning |
| 5. Scale appropriateness | not-started | lattice null belongs to Scale 0 | audit all UI nouns and cross-scale implications |
| 6. UI and interpretive truth | in-progress | focused browser test passes picker/current-state truth, unsupported-latency distinction, finite observer baseline, and collapsed overlay demand | export metadata, help/knowledge-base wording, and non-worker paths remain open |
| 7. Performance and operational safety | in-progress | focused WASM-worker `L=97` run records 240 frames at 60.00 FPS with median/p95/p99/max 16.67 ms; rapid generation/reset/resize and collapsed demand pass | repeatable hardware/build artifact, long-frame campaign, native-GPU, hidden-tab recovery, teardown allocation, and all-panel envelope remain open |

## 5. Disposition

**Disposition:** `not-set`

**Next scenario may open:** `no`

The next scenario remains blocked until every gate above has a reproducible
verdict and the canonical manifest, implementation, tests, UI, export metadata,
documentation, and scenario atlas agree.
