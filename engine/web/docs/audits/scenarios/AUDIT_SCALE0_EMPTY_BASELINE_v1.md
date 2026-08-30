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
| CPU | WASM main | in-process/Embind | browser-visible exact-null and lifecycle checks on the explicit `?engine=wasm` path; fresh canonical digest published synchronously through the shared Scale 0 capability |
| CPU | WASM worker | worker | browser-visible exact-null, lifecycle, and pacing checks on the explicit `?engine=wasm` path; explicit asynchronous canonical capture plus a non-blocking initial/latest cache; O(L^3) hashing is excluded from the 60 Hz frame loop |
| GPU | native CUDA | in-process | shared device-resident digest/counters pass ordinary and interactive modes for all three boundaries through tick 64 without a full state mirror |
| GPU when served by the CUDA server | native | WebSocket | the live protocol smoke dispatches `empty`, captures at ticks 0 and 1, and verifies exact-default counters, invariant lossless 64-bit hash lanes, source/state provenance, a 32-byte result transfer, and zero full-mirror calls |

### Loaded-artifact identity evidence

Two sequential clean builds on the same pinned host/toolchain were observed
byte-identical from source revision
`a351fd49772ca1e365285295f974608dd278fbfe`: Emscripten 5.0.2, CMake 4.2.3,
the explicitly pinned `MinGW Makefiles` generator, and Windows PowerShell
5.1.26100.9168. The comparison covered all six generated modules and the
deterministic `build_info.json` (seven files total). Wall-clock
`build_info.txt` was deliberately excluded. This is intra-environment
repeatability evidence, not a cross-host/toolchain reproducibility claim. The
two complete snapshots and equality verdict are preserved in
[the repeatability evidence record](EVIDENCE_SCALE0_EMPTY_WASM_REPEATABILITY_a351fd49.json).

| Identity field | Recorded value |
|---|---|
| Canonical bundle SHA-256 | `2498629d7e42fa1f5c07fdad151a3b285f028ea5bfe635733ff322eeeb5ba7d9` |
| Deterministic manifest SHA-256 | `e29c75af11fba88bd817f77db403865ad0fc041e306675f5ff6bd09d8ea90dd2` |
| Variants | `wasm32`, `wasm64`, `wasm32-threads` |
| Artifact records | exact factory, ABI/thread mode, byte size, and SHA-256 for each loader/module pair in `engine/web/wasm/build_info.json` |

Both direct and worker loaders fetch the manifest and selected loader/module
bytes with no-store semantics, recompute the canonical manifest-record digest,
verify the selected pair's exact size/SHA-256, and only then execute the verified
loader and instantiate the already-verified WASM bytes. The bridge exposes the
actually selected, verified variant separately through the same bridge alongside
the scientific-state digest. A focused test changes one WASM byte and confirms
rejection before instantiation; real main-thread and worker paths expose the
expected verified pairs. A static test independently checks all six checked-in
artifacts. These checks establish loaded-byte consistency with the checked-in
manifest, not independent publisher authenticity; Git commit and deployment
trust remain the authenticity boundary.

The focused browser evidence against the second build includes artifact
identity `4/4`, the canonical digest and worker-configuration barrier `5/5`,
the worker bridge contract `4/4`, and the Scenario 1 qualification `5` passed
with `1` genuine hidden-tab skip. The fresh, explicitly worker-pinned absolute
panel campaign passed `5/5`: the largest qualified WASM-worker lattice
(`L = 97`) recorded 60.00 FPS for all 17 panels with p95/p99 frame intervals
of 16.67 ms, and every collapsed panel recorded zero DOM and canvas work. An
available native WebSocket server therefore cannot silently replace a claimed
WASM-worker run.

### Live scientific-mutation provenance

The dashboard now maintains a monotonic Scale 0 `mutationEpoch` independently
of scenario-load generations. An authoritative load is `pending` until the
same owner, scenario, and load generation passes a backend-specific engine
barrier. In-thread WASM qualifies from synchronous engine readback. The worker
uses a monotonic configuration token and a FIFO batch barrier, including an
empty batch: every queued command must report success and the final engine
readbacks for queued post-setup toggles and flux-boundary policy must match;
the complete final engine toggle registry is then published and reconciled
before the proxy emits `configurationApplied`. Native qualification requires an explicitly
load-generation-stamped acknowledgement whose scenario, staged toggles, and
flux boundary match, which reports a positive integer authoritative lattice
size used to resynchronize the UI, and which includes the server parameter
record. Missing or mismatched required fields reject the load. Older-token
worker frames and older/wrong-scenario
acknowledgements cannot qualify or overwrite a newer record.

Accepted dashboard scientific-write intents record reason, source, owner tick
when available, load generation, and transport dispatch status, and suspend the
live qualification exactly once. This includes injection, clear/random field
actions, physics terms, the engine flux-boundary law, Thermodynamics and P1
writes, Wave Lab reseeding, and Genesis experiments. Fire-and-forget transports
remain explicitly unacknowledged; the UI does not claim that the engine applied
the write. Visual-only boundary shape, overlay, renderer, selection, camera,
and presentation changes do not increment the mutation epoch. Reloading restores
qualification only after the matching authoritative setup barrier succeeds.

Queued Wave Lab work captures generation and owner and is cancelled when its
scenario loses applicability. Thermodynamics slider work is coalesced and a
preset cancels queued input. On the qualified WASM path, Genesis pauses the
canonical transport, uses a turnover token around every asynchronous tick, and
never restores captured playback state into a newer scenario. Its Fire/Sweep
controls are explicitly unavailable on native WebSocket because reset,
injection, and deterministic stepping do not yet share one acknowledged native
transaction. Native resize requests are serialized and
last-intent-wins. A post-dispatch/commit-uncertain resize failure remains
suspended rather than assuming the old size, while a successful acknowledgement
synchronizes every size-dependent control before qualification.

Fresh focused browser evidence after cache-versioning the complete changed
module graph: the core scientific-mutation contract passes `8/8`; the panel
mutation/race contract passes `4/4`; the worker digest/configuration barrier
passes `5/5`; Genesis Burst passes `5/5`; the explicitly worker-pinned substrate
and direct-read suites pass `3/3` and `4/4`; the strict native profile,
rejection, acknowledgement, coalescing, and prepared-resize set passes `7/7`;
and the explicitly worker-pinned Empty panel/performance file passes `5/5`.
The native coalescing case verifies that only the first and final requests
publish their own exact client load-generation tokens.

### Telemetry availability and observation provenance

Scale 0 telemetry now treats availability as part of the scientific value.
Exact numeric zero is retained. Missing, null, ABI-absent, stale, and nonfinite
channels remain unavailable and render as `—`; they do not become a calm zero,
advance statistics, or fill chart history. Signed Lagrangian terms remain
signed from the bridge through the table and trend buffers.

Diagnostics, energy audit, Lagrangian, and gravity are independent observation
groups with their own source epoch, state/snapshot version, sample tick, sample
time, and receipt time. A reused worker audit retains its original identity and
cannot be relabelled as a newer diagnostics observation. Failed/nonfinite worker
reductions clear the retained value. Direct WASM maintains a state version in
addition to the engine tick, so a paused same-tick scientific write advances
history; a null getter marks the retained value unavailable; and a source/
configuration epoch establishes a new conservation baseline rather than
reporting an intervention as drift. Native WebSocket group packets remain
immutable, and its JSON serializer emits nonfinite floating channels as JSON
`null` so one unavailable value cannot invalidate the complete frame.

Downstream consumers enforce the same contract. Diagnostics/Grid rows require
a finite current group tick; computed audit rows carry explicit audit
provenance; status and Thermodynamics accept only `audit.dynamicEnergy` or an
explicitly published `diag.dynamicEnergy`, never the observer-baseline
`Diagnostics.totalEnergy`. Conservation maintains separate diagnostics,
energy-audit, and momentum-audit observation histories, deduplicates a reused
audit independently of the newer diagnostics clock, and rejects null audit
provenance instead of coercing it to tick/version zero. Time, Thermodynamics,
Knots, charts, and Lagrangian reset or gap their derived histories across source
turnover, and unchanged observations do not cause age-driven or repeated
scientific DOM/chart work.

Fresh closure evidence: the availability/provenance suite, direct-WASM
same-tick contract, status/Thermodynamics energy contract, and conservation
suite record `10` applicable passes with `1` correctly identified
main-thread-WASM-only skip while the connected owner is native WebSocket. The
native telemetry cache/provenance regression set passes, the pinned MSVC 14.44
`ws_server` target is current, and an independent three-pass red-team closed its
final null-provenance falsifier. The final explicitly worker-pinned `L = 97`
all-panel campaign again records 60.00 FPS with p95/p99 16.67 ms for all 17
panels, no long-task gate failures, and zero collapsed DOM/canvas work.

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
| 1. Static trace | in-progress | registry, reset + empty-profile initializer, toggle profile, boundary, native CPU target, explicit WASM main/worker paths, deterministic seven-file build identity with a durable same-host repeatability record, verified loaded variant, and knowledge-base wording located and aligned | complete export metadata and remaining native-WebSocket trace |
| 2. Mathematical well-posedness | in-progress | dynamical-null invariant, evolving bookkeeping, observer baseline, prohibited claims, and the schema-versioned named-field digest domain are separated; CPU and CUDA share one parallel two-lane combination contract with exact nonfinite/nondefault counters; WASM main, worker, and native WebSocket publish that shared engine digest without reimplementing it in JavaScript and preserve each uint64 lane as fixed-width hexadecimal | publish the remaining boundary proof |
| 3. Numerical validity | in-progress | `empty_scenario_qualification` passes the native CPU product-size short matrix at `L=9,17,33,65,97`, all three flux boundaries, and checkpoints `0,1,2,8,16`; its preregistered long matrix passes `L=9,17` through ticks `256,4096` and `L=33` through tick `256` with invariant fieldwise digests; the WSL2 CUDA sibling passes ordinary/interactive modes and all three boundaries through tick 64 with exact CPU/GPU populated-state hash parity, 32-byte result transfer, and zero full-mirror calls; two sequential clean builds on the same pinned host/toolchain from `a351fd49772ca1e365285295f974608dd278fbfe` were observed byte-identical across all six WASM modules plus the deterministic manifest; direct/worker loaders reject unverified selected bytes before instantiation and publish the actual verified variant; the source-stamped WASM worker passes lifecycle/null checks at product sizes through `L=97`; worker and main-thread WASM publish identical canonical hashes for the same Empty record, and the main path remains invariant through 16 synchronous ticks; the native CUDA WebSocket smoke passes Empty digest publication at ticks 0 and 1 with invariant lanes and no full mirror | independent controls and the remaining declared browser/lifecycle campaigns remain open |
| 4. Scientific validity | not-started | null claim and falsifiers frozen | run controls without post-hoc tuning |
| 5. Scale appropriateness | in-progress | the imposed finite lattice record belongs to Scale 0; exact-ID gating correctly excludes Standard Model overlays | replace unqualified particle/void nouns, remove stale Scale-up implications, and verify all generic panel applicability states |
| 6. UI and interpretive truth | in-progress | focused browser test passes picker/current-state truth, unsupported-latency distinction, finite observer baseline, collapsed overlay demand, WASM-main/worker origin distinction, and actual verified WASM artifact identity; the knowledge entry now states the imposed null-control and non-vacuum boundary; the complete Scale 0 telemetry path preserves exact zero while keeping missing/null/nonfinite/ABI-absent/stale values unavailable, carries independent observation provenance across native, direct-WASM, and worker paths, preserves signed Lagrangian terms, prevents staggered audit relabelling and baseline-energy substitution, and makes downstream Diagnostics/Grid/status/Thermodynamics/conservation/Time/Knots/chart histories fail closed; Flux Slice, Spectrum, and Gravity declare `empty` inapplicable and stop their scientific sampling/coordinator paths; a monotonic scientific-mutation epoch now suspends qualification for every audited live write, while backend-specific, matching-generation engine barriers are required to establish a new baseline and visual-only work remains non-mutating | remaining panel applicability, export metadata, and native-WebSocket presentation paths remain open |
| 7. Performance and operational safety | in-progress | the final, explicitly worker-pinned full campaign passes all 17 visible panels at `L=97`: every panel records 60.00 FPS with p95/p99 16.67 ms, all absolute fixed-work/callback budgets pass, no long tasks occur, and every collapsed panel records zero DOM/canvas work; Diagnostics uses 5 mounted canvases; Lagrangian uses 7 mounted canvases with on-demand action trends; Flux Slice, Spectrum, and Gravity perform zero periodic work for `empty`; rapid generation/reset/resize and five-reset lifecycle probes pass | genuine hidden-tab evidence is unsupported in the current headless browser and was skipped rather than fabricated; longer foreground campaign and native-WebSocket browser evidence remain open |

## 5. UI trace blockers

The Scale 0 UI audit found the following qualification blockers. These are
recorded here so later interface work cannot silently erase or bypass them:

1. **[RESOLVED 2026-08-29]** Injection, randomization, clear/field edits,
   term and boundary changes, panel parameter writes, Wave Lab reseeds, and
   Genesis experiments now enter the centralized mutation contract and suspend
   the displayed qualification exactly once. Matching-generation authoritative
   reload readback is required to restore it; visual-only controls do not
   suspend it;
2. **[RESOLVED 2026-08-29]** Exact zero and unavailable telemetry now remain
   distinct through native, direct-WASM, worker, hub, derived-consumer, and UI
   paths. Independent group/sample provenance prevents staggered or null
   observations from being relabelled as current; raw observer-baseline energy
   is never substituted for unavailable dynamic energy;
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
