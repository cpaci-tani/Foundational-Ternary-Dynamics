# Scale 0 Time and Gravity implementation v4

Date: 2026-09-08. Status: **independently reviewed implementations integrated;
55 integrated Node checks pass. Seven distinct browser cases pass across
the initial run and affected fixture rerun. Hardware: 13/16 pass, three
callback failures. Independent post-run audit complete; evidence sealed**.

This bounded observer change implements the
[Time/Gravity contract](PLAN_SCALE0_TIME_GRAVITY_PERFORMANCE_V3_2026-09-08.md).
It follows the [Wave v3 program](PROGRAM_SCALE0_WAVE_CORRECTNESS_V3_2026-09-08.md).
Wave v3 records four passing hidden/collapsed cases and four visible callback
failures, including two new worker failures relative to performance v2.
These failures remain open. The native evolution, strict candidate, WASM
artifacts, numerical definitions, sampling support and cadence are unchanged
in this wave. Reference-model observations do not establish strict physical
recovery.

## Ownership and integration order

The Time agent owns its isolated panel, Time-only retained renderer and
associated tests under `engine/build/scale0_time_v4_draft_20260908/`.
The Gravity agent owns its isolated publication helper, worker adapter,
capability, Gravity analysis/panel and associated tests under
`engine/build/scale0_gravity_v4_draft_20260908/`. Each retains the production
baselines it copied. The coordinator owns the shared interface contract,
integration, hardware harness and this program. Independent reviewers check
implementations they did not author; fixes return to the author for review.

No production source is copied until the Wave v3 evidence is sealed and
the corresponding draft passes independent correctness review. Existing
production files must match their captured baselines before integration.
Unrelated working-tree changes and earlier source/evidence archives remain
preserved. A separately identified complete candidate must be frozen before
hardware measurements.

## Time T1 contract

Cards A/B/C/E/F use dedicated retained numeric text and SVG attribute slots.
Preserve all existing formulas, formatting, 25 reference points, 12 radial
bins, tie behavior, optional states, visible history window and complete
underlying histories. Verify initial native titles and migrated
`data-ui-tooltip` behavior. The delegated F enable button and independently
owned D range input retain their existing interactions.

This is a presentation-only change. Sampler requests, aggregate reads,
qualified diagnostics, centre inspection, clock/quadrature integration,
source-boundary resets, demand release and callback cadence must match the
baseline. Golden markup and complete SVG output comparisons should cover
unavailable, empty, sparse and active states, repeated updates and structural
transitions. Stable rendering must avoid reparsing a detached tree.

## Gravity G1 batch contract amendment

The original plan's single-plane operation is extended to
`getFluxSlabsWithMaxRho(requests)`. Accept one to three validated `{axis,index}`
selectors, copied before pinning a publication and returned in caller order.
Use **one publication pin and one full-volume maximum scan** for the entire
batch. Each owned Float64 slab records `N`, axis, index, `startPlane`,
`planeCount` and the established in-plane coordinate order. Include the
available adjacent normal planes without inventing periodic padding.

All slabs share one immutable publication/configuration/buffer-generation
record and one `maxRho`. Preserve the original Double `mag * mag` scan order
and initial `1e-30`; a slab-local maximum would change every displayed value.
Publication identity is not sample time: the new record uses
`sampleTick: null`. Release pins in `finally`, including allocation/read
failures, and retain readiness, configuration, rebind and disposal fences.

A rotating refresh requests one selector. A quantity-change refresh requests
all three selectors in one batch; three separate publications would change
the previous simultaneous full-volume observation. A single-plane convenience
method may delegate to the one-selector batch. Supported worker contention
or unavailability produces an explicit unavailable view and later retry,
without a dense-copy fallback. Unsupported direct/native backends keep the
existing full-volume/FTV2 path and direct view-consumption order.

Complete array and pixel oracles must cover all axes and quantities, normal
edges, tiny and zero fields, asymmetric components and a maximum outside
every requested slab. Preserve all 18-neighbor curvature, force, dilation and
latency expressions, zero-border behavior, orientation and pixel conversion.
Test one pin/maximum scan per batch, selector ownership, owned output,
fault release, stale generation, rebind and disposal. No law change follows
from moving these observations through a smaller copy.

## Preregistered hardware scope

Profile `time-gravity-v4` will select exactly Time with
`s0-seed-time-twin-clocks` and Gravity with `s0-seed-massive-body`, in docked,
floated, collapsed and hidden states, with physics playing. Run the complete
ordered direct WASM L33 and worker WASM L97 matrix: **16 rows**. Require the
reviewed 64-hex candidate identity and retain the existing preparation,
warmup, actions, duration, measurement loop and gates.

Acceptance remains at least 600 frame intervals and 12 seconds, effective FPS
at least 59.5, frame p95 at most 17 ms and p99 at most 20 ms, no intervals over
33.4 ms or Long Tasks, inclusive interface callback p95 at most 2 ms and
maximum at most 8 ms, and ten paired visible-state actions with
dispatch-to-next-rAF p95 at most 50 ms. Require foreground hardware renderer
provenance, unchanged owner/configuration/qualification, advancing clocks,
correct mount, no hidden/collapsed canvas work and no runtime errors.
Nested owner reads remain inside the parent callback timing.

Retain raw frame and callback vectors after measurement stops, raw actions,
all reports and failure attachments. Archive transient results immediately
after the invocation before any next Playwright run. Use distinct output
paths and refuse overwrite. The console assertion may summarize failed row
identities and reasons; the complete report remains the evidence. Independently
reconstruct timing statistics and gates and reconcile source/artifact hashes.
Do not run competing browser, build or scientific workloads during timing.

These affected-interface measurements do not renew the entire v2 matrix or
waive Wave Lab, Spectrum, observation-provenance or strict recovery blockers.
If T1/G1 remain over budget, retain the failure and measure the next cost
before choosing another change. No reduced resolution, omitted updates,
retuned thresholds, skipped physical transactions or canonical adoption is
authorized by a failed performance gate.

## Evidence and disposition

The [harness review](evidence/scale0-time-gravity-v4-harness-review-2026-09-08.json)
accepted the exact two-panel/four-state/two-backend scope, all 22 invalid
admission rejections, six invalid-plan rejections and 324 identical legacy
admission comparisons. Gate, measurement and cleanup blocks retain their
prior behavior. The [harness integration record](evidence/scale0-time-gravity-v4-harness-integration-2026-09-08.json)
confirms the existing production baseline before copying the reviewed bytes.

The Time draft passed 14 focused Node checks (11 source/parity/history and
three helper binding checks). Its two isolated mounted-browser tests passed
on the coordinator's first invocation; the
[browser log](evidence/scale0-time-v4-isolated-browser-attempt1-2026-09-08.log)
records complete DOM/SVG parity and stable numeric updates without
`innerHTML` parsing, plus tooltip, delegated-control and lifecycle behavior.
The golden browser spec, reference copies and configuration remain isolated
review assets. Only the panel, private helper and portable Node regression
are proposed for production. The Gravity draft passed 21 focused Node tests
on its first authorized run. Independent implementation reviews and the
integrated/browser/hardware checks remain pending; these counts overlap
later reruns and must not be summed as distinct tests.

Further manifests, reviews, integration and hardware results will be recorded
here as they complete. Passing deterministic
observer tests is distinct from meeting callback budgets and from recovering
continuum, matter, chemistry or causal gravity. Production replay remains
deferred; strict complete checkpoints remain mandatory. No commit or public
deployment is part of this wave.

## Integrated correctness disposition

The [Time implementation review](evidence/scale0-time-v4-draft-review-2026-09-08.json)
independently reproduced 11 source/golden and three binding checks, verified
33 draft hashes and nine production baseline records, and separately
identified the coordinator's two passing browser checks. The
[Gravity implementation review](evidence/scale0-gravity-v4-draft-review-2026-09-08.json)
independently reproduced 21 tests and added 2,100 complete Float64 comparisons
against frozen dense code across signed, tiny, clamped, nonfinite, overflow
and individual-neighbor fields. Three signed publication-counter wrap cases
also passed. That reviewer verified 24 draft files, 12 production baseline
records and verbatim oracle function bodies. Neither author approved their
own implementation.

The [integration record](evidence/scale0-time-gravity-v4-integration-2026-09-08.json)
records all 11 copied implementation/test files and two preserved isolated
draft archives. All existing baselines matched and all four new paths were
absent before any copying. The separately reviewed harness is the twelfth
changed/added path. Native/WASM bytes and unrelated source remain unchanged.

The [integrated Node execution](evidence/scale0-time-gravity-v4-node-2026-09-08-execution.json)
passed 52 tests in the Gravity slab, bounded publication, worker lifecycle and
Time retained-readout groups, plus three focused existing publication-runtime
tests: **55 passes, zero failures or skips**. The raw log was archived.
The seven-case mounted-browser follow-up covers the new Gravity panel path,
existing Time dashboard, direct zero-copy consumption, native FTV2 coordinates
and reply freshness, and worker sampler readiness. The direct mounted test
asserts force/aggregate/volume/toggle call order. The native fixtures do not
supply an independent complete native-panel call-order trace; retain that
coverage limit. Hardware acceptance remains a separate pending gate.

The [first browser execution](evidence/scale0-time-gravity-v4-browser-2026-09-08-execution.json)
retains **six passing existing cases and one failed new Gravity case**. The
failure occurred before its assertions: the server returned the frozen `.mjs`
oracle with `Content-Type: text/plain`, which Chromium rejects for a module.
The archived trace identifies HTTP 200 and the correct 3,024-byte response;
this was not a missing numerical oracle. The immediate result archive retains
the full log, failure screenshot, error context, trace and last-run record.
The [independent test repair review](evidence/scale0-gravity-v4-browser-repair-review-2026-09-08.json)
verified the fixed oracle hash and the byte-identical 7,576-byte suffix from
the original `page.goto` through every final assertion. The
[repair integration amendment](evidence/scale0-gravity-v4-browser-repair-integration-2026-09-08.json)
records the test-only loader change and its preserved isolated archive.
The oracle bytes, server, product code and pixel/recovery assertions remain
fixed.

The [affected browser rerun](evidence/scale0-time-gravity-v4-browser-repair-2026-09-08-execution.json)
passed its one Gravity case, exercising all twelve quantity/axis canvases,
same-publication batch reads, unavailable blanking, fixed-version paused
recovery and hidden no-read behavior. The initial six existing cases were
unchanged and were not repeated. Thus seven distinct integrated cases pass
across two invocations; there was no subsequent unified seven-test run.
The first failure remains archived. The two isolated Time golden cases are
separate evidence.

The [integration audit](evidence/scale0-time-gravity-v4-integration-review-2026-09-08.json)
verified eight changed plus four added paths, 2,164 unchanged prior-source
entries, both complete draft archives and prior Wave/strict/WASM evidence.
That audit predates the test repair; the repair's review and integration
amendment account for its changed test hash. All three records must be read
together when validating the complete candidate.

## Frozen hardware outcome

The [candidate manifest](evidence/scale0-time-gravity-v4-candidate-manifest-2026-09-08.json)
freezes 2,176 source files, eight changed and four added relative to Wave v3,
with identity
`69b0bb82befe103572ac4710631c4242fc9c6b23990bea93c22ed5426b3ff394`.
Its separate source archive also retains eight preregistration/review/runner
snapshots. The [independent freeze review](evidence/scale0-time-gravity-v4-freeze-review-2026-09-08.json)
accepted the registered campaign after checking source, archives, repair
amendment, split software evidence and prior strict/WASM/Wave preservation.

The [hardware execution](evidence/scale0-time-gravity-v4-hardware-2026-09-08-execution.json)
completed the full 16 rows and exited 1. The
[direct L33 report](evidence/scale0-hardware-time-gravity-v4-panels-L33-2026-09-08.json)
and [worker L97 report](evidence/scale0-hardware-time-gravity-v4-panels-L97-2026-09-08.json)
record **13 passing rows and three callback p95 failures**. All registered
frame and other non-callback gates pass in the reports; no runtime errors or
Long Tasks were recorded over 11,603 retained frame intervals. This is a
bounded foreground workload result, not an always-60-FPS or release claim.

| Backend | Panel | Docked p95, ms | Floated p95, ms | Visible-state disposition |
|---|---|---:|---:|---|
| Direct L33 | Gravity | 1.780 | 1.785 | Both pass |
| Direct L33 | Time | 2.490 | 2.585 | Both fail |
| Worker L97 | Gravity | 1.930 | 2.100 | Docked passes; floated fails |
| Worker L97 | Time | 1.000 | 0.980 | Both pass |

All eight hidden/collapsed rows pass with no canvas work. Visible Gravity
rows each execute 48 canvas operations; Time uses its retained DOM/SVG view.
The two direct Time failures and floated worker Gravity failure remain
release blockers. Worker Gravity docked and worker Time floated now pass
where their v2 rows failed, leaving three of the five prior Time/Gravity
callback failures. These are descriptive comparisons between frozen runs;
they do not isolate the causal timing effect of individual code changes.

The immediate 11-file hardware archive contains both raw reports, the full
log, candidate manifest and all available transient result files, including
failure screenshots and error contexts. This matrix disables tracing; the
separate functional MIME-failure trace remains in its earlier archive.
The [coordinator's post-run check](evidence/scale0-time-gravity-v4-postrun-source-check-2026-09-08.json)
matched every current/archive source entry and every archived snapshot.
The [independent hardware audit](evidence/scale0-time-gravity-v4-hardware-review-2026-09-08.json)
reconstructed all 1,404 numeric summary fields exactly from 11,603 frame
intervals, 18,420 callback durations across 194 series and 80 actions. Every
row, log record and gate flag agreed. The audit found two cleared prior
Time/Gravity callback failures and no new ones, with all three remaining
failures preserved. It checked all 2,176 current/archive source entries,
eight archived snapshots, 11 immediate result archive members, strict
69/44/73 scopes, six WASM artifacts plus build information, and the Wave/v2
source/evidence archives. The current program's completed disposition is
separate from its immutable archived preregistration.

The [final verification](evidence/scale0-time-gravity-v4-verification-2026-09-08.json)
and [evidence archive seal](evidence/scale0-time-gravity-v4-evidence-archive-2026-09-08.json)
close this candidate's evidence collection. Bounded observer correctness is
accepted; the complete hardware callback gate and release remain failed.

The runner captures ordinary successful and nonzero test exits automatically.
Forced interruption, process-spawn failure or storage failure outside normal
subprocess return requires manual preservation of available outputs; this
limitation was recorded before measurement. This invocation returned normally
and its immediate archive verified successfully.

Wave v3's four visible callback failures and Spectrum's two prior direct
failures remain separate open work. T1/G1 are insufficient for full callback
acceptance. Further changes require attribution of acquisition, reductions,
presentation and painting within their actual callback invocations, while
retaining full normalization, histories, scientific observations and cadence.
No physical recovery gate is promoted by these engineering results.
