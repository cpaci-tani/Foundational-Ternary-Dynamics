# Scale 0 performance repair v2: implementation, preregistration and results

Date: 2026-09-07 local registration; hardware execution completed on
2026-09-08 UTC. Status: **288/288 registered rows measured; 279 pass,
nine panel callback failures remain. All 144 overlays and all 288 frame
gates pass. Release remains blocked.**

The final disposition is in [Post-campaign evidence and remaining work](#post-campaign-evidence-and-remaining-work).
The intervening registration and prerequisite chronology retain their earlier
pending states as provenance; those states are superseded only by the explicit
results below. The unchanged registration-time document remains in the frozen
candidate archive.

This successor preserves the [earlier performance audit and failed repair-v1
campaign](AUDIT_SCALE0_PERFORMANCE_FOLLOWUP_2026-09-07.md) and the
[native observation wave](PROGRAM_NATIVE_OBSERVATION_V3_2026-09-07.md).
It reduces observation and presentation work without changing a physical law.
Production Scale 0 remains a floating reference engine. This registration
does not approve release, deployment, replay or physical recovery.

## Implementation and review ownership

| Scope | Implementation | Independent review and current evidence |
|---|---|---|
| Spectrum | Spectrum agent: panel and `spectrum-analysis-client/core/worker.js` | Bounded-read agent accepted source and numerical/lifecycle scope; 43 focused Node tests passed independently after the timer repair |
| Bounded worker observations and P1 | Bounded-read agent: publication helper, WASM proxy and anisotropy component | Spectrum agent accepted after the pin-release repair; 52 focused Node tests and four additional independent probes passed |
| Time | Time agent: retained cards and visible history-window copying | Coordinator accepted the source-scoped change; six focused Node tests passed |
| Shared retained readouts, Wave Lab, Conservation and dual rendering | Coordinator | Time agent independently accepted the bounded source scopes; coordinator reports three retained-readout browser tests passed; broader integration pending |
| Measurement harness and candidate integration | Coordinator | This preregistration fixes scope and gates; final candidate review and hardware evidence remain pending |

The review record is
[scale0-performance-v2-independent-review-2026-09-07.json](evidence/scale0-performance-v2-independent-review-2026-09-07.json).
These are independent AI agent reviews, not external human certification.
Each implementation returns to its owner for fixes; authors do not approve
their own changes. Before the timer repair, the coordinator reported
**124/124 combined Node tests**:
42 Spectrum, 52 bounded-read/adjacent runtime, six Time, 15 renderer/analysis
and nine redundant-read tests. The additional independent probes are separate
from that count. Browser and hardware results are not inferred from Node tests.

The first integrated browser attempt subsequently exposed `Illegal invocation`
in Spectrum's default timer calls. Storing browser timer functions as client
methods supplied the wrong receiver; injected Node fixture timers had hidden
this difference. Arrow wrappers now call `globalThis.setTimeout/clearTimeout`,
and the author passes 43 Spectrum tests including a receiver-sensitive
regression. A new browser test exercises default timers and the real module
worker through cancellation, completion and disposal. Independent re-audit
accepted the fix and reproduced all 43 tests, including a probe that fails on
the prior bare timer defaults. The real-browser worker-client test and both
L33/L97 Spectrum provenance/Deep tests subsequently passed. The coordinator stopped the
initial nine-test attempt after startup failures. The
[failure record](evidence/scale0-performance-v2-browser-startup-failure-2026-09-07.json)
preserves its logs/traces in `engine/build/scale0_performance_v2_browser_startup_failure_20260907.zip`,
SHA256 `f221a8d61bcc73c4cf0c7b409ac00b4d74bb5c681e2ffbca053f369da9de2c0a`.
That attempt is not converted into a pass by the repair. The subsequent
[13-test functional suite](evidence/scale0-performance-v2-browser-functional-2026-09-07.log)
recorded **11 passed, one skipped and one failed**. The skip is a direct-only
vacuum diagnostic when the worker owns the scene, and supplies no evidence
for core physical energy. The failure is Time's old literal `IR convergence`:
the author reproduced unchanged frozen/current card-D content carrying the
historical finite-size-residual caveat. A test-only correction and independent
review are in progress; the focused Time rerun is pending. The three retained
readout tests passed. This partial suite is not a complete integration pass.

Spectrum now transfers owned copies of completed samples to a dedicated
observation-only module worker. Dense reconstruction, finite-value scans,
magnitudes, FFTs, connectivity and metric reductions execute there. One active
job and at most one latest pending job bound analysis demand. Live completion
replaces one result mailbox; the registered 2 Hz callback performs live DOM
publication. Deep Measure supersedes live work, shares the existing absolute
five-second budget between sample availability and computation, and freezes
only the hero. Later topology observations remain live without another FFT.

The live strides, 32-cube FFT, eight-cube large-lattice FFT, 64-cube Deep FFT,
0.5 defect threshold, 0.35 connectivity threshold and arithmetic order remain
unchanged. Real module-worker comparisons match the retained numerical oracles
exactly at L33/stride1/M32, L97/stride7/M8 and Deep M64. Completed count-zero
samples retain the prior zero reconstructed grid/no resolved spectral-range
meaning; missing samples and undefined metric statistics remain unavailable.
Unknown sample clocks remain unknown. Owner, load/scenario generation and
request identities reject obsolete results. Hide, empty/load intent, owner
replacement, cancellation and disposal terminate obsolete work. Review found
duplicate global visibility notifications could bypass the cadence; the fix
reacts only to a real visibility transition, with a 20-notification regression.

The WASM proxy can copy one N-squared plane or 1-128 selected cells while one
published slot is pinned. Returned Float64 arrays own their storage and do not
populate the full-volume cache. Selection is validated and captured before
pinning; configuration, buffer generation, resize and disposal are checked
again before publication. P1 uses the same 128 nearest-cell circular probes,
rounding, periodic wrapping, trigonometry and two-pass mean/variance order.
Direct and native compact-volume fallbacks retain their existing semantics.
Repeated display of the same retained P1 result avoids rebuilding its SVG.

Independent review reproduced a pin leak when constructing the shared typed
view threw before the caller received a release handle. The owner added
exception cleanup inside `acquire`; regressions cover both view construction
and output allocation. Additional independent probes cover same-buffer rebind,
resize and disposal during adversarial selection access. The publication
counter remains a signed, wrapping local buffer counter. Metadata retains
`sampleTick: null`; it does not establish a joint cross-field observation or
global state lineage.

The shared readout helper reconciles text, attributes and HTML/SVG nodes inside
sole-owner instrument containers. **Detached markup parsing and allocations
remain.** Its reviewed scope is the actual div-wrapped readouts, not arbitrary
containers or controls with independent form state. Wave Lab retains its
numeric markup and five histories; chart redraw suppression applies only when
neither history append nor chart reconstruction occurred. Independent resize
and history controls remain active. Conservation retains fixed headline nodes
while preserving separate diagnostic, energy and momentum clocks, source
resets, baselines and hysteresis. Time retains cards A/B/C/E/F, rewrites imposed
card D only when its selected value changes, and copies only the visible
history window while retaining the complete history. None of these changes
merges separately sampled records into one claimed state.

The dual-renderer repair clears the dual mesh for invalid dual samples,
including nonfinite/Float32-overflow cases, without accessing the chirality
mesh. Valid coordinates, colors and sizes retain their tested values. This
confirmed logic repair is not a demonstrated cause of the earlier frame stalls.

## Registered workload matrix

The new profile is `FTD_AUDIT_PROFILE=repair-v2` in
[`scale0-comprehensive-performance.spec.js`](../web/tests/scale0-comprehensive-performance.spec.js).
All registered Scale 0 panels are included because shared Conservation and
worker observation paths changed. Selection is not limited to earlier failures.

| Matrix | Interfaces | States | Backends | Logical rows |
|---|---:|---|---|---:|
| Panels | 18 | Docked, floated, collapsed, hidden | Direct WASM L33; worker WASM L97 | 144 |
| Overlays | 36 | Paused, playing | Direct WASM L33; worker WASM L97 | 144 |

The 18 panels are Controls, Diagnostics, Telemetry Grid, Charts, Lagrangian,
Inspector, Scene, Flux Slice, Wave Lab, P1 Observables, Spectrum, Dispersion,
Knots, Transactions, Gravity, Time, Thermo and Scale Context. The 36 overlay
IDs are the 33 `FIELD_TOGGLE_BINDINGS` plus Flux Volume, Flux Slice and the
Standard Model reference overlay. The exact IDs are recorded in the review
JSON and must match the frozen registries before measurement.

Panel preparations remain `flux-pulse`, except Wave Lab uses
`s0-field-rf-lattice-wave`, Gravity uses `s0-seed-massive-body`, Time uses
`s0-seed-time-twin-clocks`, and Thermo uses `flux-thermalization`. Inspector
must select the center voxel on the correct authoritative owner. Panels run
with physics playing in every visibility state.

Overlay preparation selection retains the existing algorithm: choose the
first applicable scenario from the ordered preference list `flux-pulse`,
`flux-dual-substrate`, `flux-baryon`, `flux-random-genesis`,
`s0-seed-time-twin-clocks`, `s0-seed-de-broglie-clock`,
`s0-seed-massive-body`, followed by the remaining frozen scenario-registry
order. Save the resolved plan before collecting measurements. Unavailable
preparations are reported, not replaced by invented data or silent successes.

**Damping Zones has a separately registered modified reference preparation**
for its four previously unmeasured logical rows. Profile
`damping-zones-modified-v1` loads `s0-seed-sloop` with its 12 markers, then uses
the actual UI to enable damping and selective damping. The qualification must
be explicitly **`suspended`**, reflecting these overrides; the record must not
claim that the original scenario remains within contract. Require 288 damping
geometry vertices and unchanged remaining term profile, authoritative owner,
load anchor/configuration and the appropriate paused/playing clock behavior.
This is a 12-marker workload, not a 100-glyph worst-case claim.

The preparation helper is
[`scale0-damping-performance-preparation.js`](../web/tests/scale0-damping-performance-preparation.js).
Its [two actual browser preparation checks](evidence/scale0-performance-v2-damping-preparation-2026-09-07.log)
passed on L33/L97 in 56.3 seconds, including the 12 particles, 288 vertices,
retained owner and suspended modified profile. Hardware execution remains a
prerequisite. All four rows remain unmeasured until their runs complete; if the real UI,
profile, qualification or geometry cannot be established, retain the failed
preparation and leave those rows unmeasured. Do not count one interface-level
failure as two measured states. No modified preparation is retroactively
credited to an older campaign. The full registration contains **288 logical
rows**, 144 panels and 144 overlays.

## Unchanged acceptance gates

Every measured row must satisfy the existing numerical thresholds:

| Gate | Required evidence |
|---|---|
| Foreground sampling | At least 600 frames and 12 seconds; visible page |
| Frame cadence | Effective FPS >=59.5; frame p95 <=17 ms; p99 <=20 ms; zero intervals >33.4 ms |
| Long tasks | Instrumentation supported and zero recorded Long Tasks |
| Interface callback | Inclusive callback p95 <=2 ms and maximum <=8 ms |
| Visible interaction | Ten paired actions; synchronous-dispatch-to-next-rAF p95 <=50 ms |
| Hardware | Nonempty unmasked WebGL renderer; reject software/SwiftShader; retain hardware ANGLE/RTX provenance |
| Owner and configuration | Same WASM owner, load generation, authoritative load and settled lifecycle before/after; `within-contract` qualification except the four explicitly modified Damping rows require `suspended` |
| Clock | Exact nonnegative safe-integer recorded ticks; playing advances, paused does not |
| Visibility/lifecycle | Correct connected dock/floating/collapsed/hidden mount before/after; no hidden/collapsed canvas work |
| Runtime | No instrumented or captured runtime errors; required panel-update instrumentation present |

The existing probe times whole callback invocation. **Nested owner-read time
is included in that elapsed callback time.** Standalone `method:ownerRead.*`
rows are diagnostic and excluded from the callback gate as separate entries;
their duration is not subtracted from enclosing callbacks. `sampleFluxAtCells`
joins the recorded owner-read methods. Full-frame timing includes all
foreground work. Earlier prose describing pure DOM/canvas callback timing is
not the interpretation used here; the old raw measurements remain unchanged.
All numerical performance thresholds remain unchanged. The explicit Damping
qualification exception registers a new modified preparation; it does not
relax the existing within-contract rows.

Ten interactions apply to overlays and visible docked/floated panels, with
paired toggles restoring the original state. Dispatch-to-next-rAF is not an
actual display-present timestamp. Deep Measure is user-triggered and requires
its separate browser lifecycle/numerical checks; the steady live matrix alone
does not certify Deep latency or every possible overlay combination.

## Execution and evidence disposition

Before hardware collection, the coordinator freezes the candidate source and
artifacts into a new manifest/archive, checks browser integration and resolves
any failed prerequisite. The native-observation v2 baseline has 2,160 files
and identity `a833ab9bc9c163872edd554cd349453c02a8c5d934a4d9bfbdac423c9267167b`;
it is provenance, not the new candidate identity. New identity, archive and
post-run source/hash reconciliation are pending.

Run one browser suite at a time from `engine/web/tests`, with no concurrent
heavy builds, scientific GPU campaign or second browser suite. Preserve the
user's separate server/session. The registered commands are:

```powershell
$env:FTD_HARDWARE_WEBGL = '1'
$env:FTD_AUDIT_PROFILE = 'repair-v2'
$env:FTD_AUDIT_SIZES = '33,97'
$env:FTD_AUDIT_KIND = 'panels'
npm test -- scale0-comprehensive-performance.spec.js --workers=1
$env:FTD_AUDIT_KIND = 'overlays'
npm test -- scale0-comprehensive-performance.spec.js --workers=1
```

Use distinct `scale0-hardware-repair-v2-{panels|overlays}-L{33|97}-2026-09-07.json`
outputs and corresponding separate logs. The harness refuses to overwrite
existing campaign output. Retain every failure, runtime error and unmeasured
row. Any subsequent repair needs a separately identified source revision and
new output names; do not tune thresholds or retrospectively relabel failures.

The original overlay baseline remains 138 passing of 140 measured rows, two
frame failures and four logical Damping rows unmeasured. The original panel
baseline remains 126 passing of 144, with 18 callback failures. Repair-v1
remains 49 passing of 64, with 15 callback failures, even though all its frame,
owner, clock and visibility gates passed. Earlier artifacts, suite failures,
strict manifests and unrelated working-tree changes remain preserved.

## Post-campaign evidence and remaining work

Execution completed at `2026-09-08T05:23:36.0834920Z`. The candidate freezes
2,171 source/artifact files, identity
`8ad614edd7616b43622b437d29a5d3800a614f35ad4d9da9fb3226f725fd0177`.
Its archive is `engine/build/scale0_performance_v2_candidate_20260907.zip`,
SHA256 `3278a805f0ac37156036925dbae4ffe7ddcd20a9a7ff012c9e86b75ae252503f`.
Compared with the observation baseline, 15 files changed and 11 were added;
none were removed. Native kernels and all six WASM artifacts are unchanged.
The [freeze review](evidence/scale0-performance-v2-freeze-review-2026-09-07.json)
accepted the reviewed browser prerequisites before collection. Coordinator
and independent post-run checks found zero source or archive drift, including
both frozen preregistration snapshots. This remains a local dirty candidate.

| Registered matrix | Passed / measured | Failed gate | Raw evidence |
|---|---:|---|---|
| Direct WASM L33 panels | 66 / 72 | Six callback p95 failures | [Report](evidence/scale0-hardware-repair-v2-panels-L33-2026-09-07.json) |
| Worker WASM L97 panels | 69 / 72 | Three callback p95 failures | [Report](evidence/scale0-hardware-repair-v2-panels-L97-2026-09-07.json) |
| Direct WASM L33 overlays | 72 / 72 | None in registered scope | [Report](evidence/scale0-hardware-repair-v2-overlays-L33-2026-09-07.json) |
| Worker WASM L97 overlays | 72 / 72 | None in registered scope | [Report](evidence/scale0-hardware-repair-v2-overlays-L97-2026-09-07.json) |

All measured frame, interaction, ownership, clock, qualification, visibility
and runtime-error gates pass. Renderer provenance is hardware ANGLE on the
NVIDIA RTX 5090. All four Damping rows now have measured evidence for the
registered modified preparation: all 46 canonical toggle values, exactly two
UI changes, suspended qualification, 12 prepared markers and 288 geometry
vertices. They are not retroactive passes for the older unmeasured rows.

| Remaining callback failure | Docked p95, ms | Floated p95, ms |
|---|---:|---:|
| L33 Wave Lab | 3.375 | 3.250 |
| L33 Spectrum | 6.605 | 6.795 |
| L33 Time | 2.855 | 2.710 |
| L97 Gravity | 3.055 | 3.190 |
| L97 Time | Passed at 1.955 | 2.060 |

The unchanged limit is 2 ms p95; the 2.060 ms result remains a failure.
No maximum-callback failure occurred. The shell's nonzero result records the
two failed panel matrix tests; both overlay matrix tests passed. The
[execution record](evidence/scale0-performance-v2-campaign-execution-2026-09-07.json),
[panel log](evidence/scale0-performance-v2-panels-hardware-2026-09-07.log)
and [overlay log](evidence/scale0-performance-v2-overlays-hardware-2026-09-07.log)
preserve these outcomes.

Independent reviews cover the [panels](evidence/scale0-performance-v2-panels-review-2026-09-07.json),
[direct overlays](evidence/scale0-performance-v2-overlays-L33-review-2026-09-07.json)
and [integrated results](evidence/scale0-performance-v2-integrated-review-2026-09-07.json).
The [final verification](evidence/scale0-performance-v2-verification-2026-09-07.json)
binds retained sources, artifacts, reviews and execution evidence. These are
independent AI agent audits, not external human or physical certification.

The final combined Node run passed **125/125**. The final affected Damping/Time
browser group passed **3/3**, including all 46 canonical toggle keys and the
actual Time caption and tooltip channel. Across the functional suite and
affected reruns, **14 distinct browser cases pass and one direct-only case
remains skipped on the worker owner**. There was no unified 15-case rerun.
The original 11-pass/one-fail/one-skip suite and both Time fixture failures
remain separate historical outcomes. The startup and subsequent Time fixture
failure archives retain their recorded hashes.

Evidence limits remain explicit. Raw frame and callback timing vectors were
not retained; reviewers check their aggregates and gate consistency. Raw
interaction samples are retained and independently recomputed. Panel JSON
reports and full failure logs are preserved, but the subsequent overlay
Playwright invocation cleared transient panel screenshots/error-context
attachments before archival. No archived panel screenshots or traces are
claimed. Outside the explicit Damping checks, this hardware matrix does not
independently prove nonzero geometry for each overlay. Its workload-specific
frame pass does not establish an always-60-FPS guarantee.

The [next Wave Lab/Spectrum plan](PLAN_SCALE0_PERFORMANCE_V3_FOLLOWUP_2026-09-07.md)
records a newly confirmed directional-component defect separately from further
optimization. An isolated Wave Lab draft is outside this frozen candidate and
does not alter its results. Null J/E payloads appearing as active zero metrics
and the owner-clock/fallback-zero readout also remain separate observation
limitations; the component fix alone cannot close them. The
[Time/Gravity contract](PLAN_SCALE0_TIME_GRAVITY_PERFORMANCE_V3_2026-09-08.md)
preserves exact sampling, histories and full-volume normalization while
specifying the next bounded changes. Each successor needs its own source
identity, actual numerical/lifecycle regressions and unchanged hardware gates.

The [alignment-law proposal](SPEC_PHI_ALIGNMENT_SUCCESSOR_PROPOSAL_V1.md) and
its [independent document review](evidence/phi-alignment-proposal-v1-review-2026-09-07.json)
are a separate, unimplemented scientific proposal. No formation trajectory,
restoring binding, general continuum closure or higher-scale recovery is
certified by this performance campaign. Production replay stays deferred;
strict complete-state checkpoints remain mandatory. No commit, deployment,
canonical adoption or release approval occurred.

**Current disposition:** implementation/source review is accepted at the
scopes above. Candidate freeze, integrated browser checks, all repair-v2
hardware rows and post-campaign audit remain pending. No new FPS result,
universal performance guarantee, strict physical recovery, full production
replay, release or deployment approval follows from this record.


## Final integration checks before freezing

The final combined Node group passes **125/125**, without skips, after the
browser-timer regression was added. The [captured log](evidence/scale0-performance-v2-node-2026-09-07.log)
records that run. The original integrated browser run remains **11 passed,
one skipped and one failed**: the direct-only vacuum check skipped on a
worker owner; Time expected a caption absent from the frozen baseline.
The Time test now checks the existing historical finite-size caption, both
charts and the explicit continuum-recovery caveat. An initial replacement
reader incorrectly looked only at `title`; the real tooltip manager moves it
to `data-ui-tooltip`. That failure has a separately retained log and trace
archive in the review JSON. No Time numerical definition was changed.

Damping's initial two passing preparation cases checked 28 UI toggles.
Independent review required the complete 46-name engine registry, including
hidden research terms. The final preparation now requires all 46 baseline
values false and exactly damping/selective_damping true afterward, and binds
each measurement to its exact recorded modified qualification and lifecycle.
The original within-contract cases retain their existing qualification gate.
The focused Time and complete-profile Damping rerun remains pending here.

The final [focused browser run](evidence/scale0-performance-v2-damping-time-final-2026-09-07.log)
passes all three cases in 1.4 minutes: both 46-toggle Damping preparations and
Time, including slider interaction and source-reset history clearing. Across
functional runs, 14 distinct cases pass; the direct-only vacuum case remains
skipped on its worker owner. Earlier failures retain their separate disposition.
No full 15-case unified rerun is claimed. Candidate freeze and hardware results
are recorded separately below when available.
