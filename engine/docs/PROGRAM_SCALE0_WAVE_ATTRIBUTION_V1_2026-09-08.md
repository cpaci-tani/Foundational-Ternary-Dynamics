# WaveLab component timing, diagnostic v1

Status: independent source/Node reviews and mounted-browser equivalence pass;
baseline diagnostics pass eight states; routed diagnostics pass four L33 states
and fail the L97 test's initial app startup before measurement. Independent
reconstruction accepts the retained measurements; the complete matrix is open.

This program identifies work inside the four visible WaveLab callback failures
retained by [Wave v3](PROGRAM_SCALE0_WAVE_CORRECTNESS_V3_2026-09-08.md).
It changes no production code and grants no performance or physical approval.
The complete current production baseline is the 2,176-file
[Time/Gravity v4 manifest](evidence/scale0-time-gravity-v4-candidate-manifest-2026-09-08.json),
source identity `69b0bb82befe103572ac4710631c4242fc9c6b23990bea93c22ed5426b3ff394`.
The strict law, scientific preparation, resolution and sampling cadence stay fixed.

## Frozen scope before measurement

The isolated probe lives in `engine/build/scale0_wave_attribution_probe_20260908/`.
Its manifest binds six original modules, six routed copies, the recorder,
analyzer, route map, tests and reservation/cleanup helpers. Every served
original module must match its pinned hash before routing; all six deliveries
are required in each full-app diagnostic. The revised manifest and independent
reviews will be recorded below before timing begins.

Execution order is held-input browser equivalence, baseline diagnostic, then
routed diagnostic. The baseline serves the original pinned modules with the
recorder installed but no instrumentation calls. The routed run adds timing
spans. Each diagnostic contains L33 direct WASM and L97 worker WASM, each in
docked, floated, collapsed and hidden states, in that order. The scenario is
`s0-field-rf-lattice-wave`. Each state retains at least 600 frames and 12 seconds;
each visible state adds ten existing visibility actions. Diagnostic tracing is
disabled in both variants. Hardware ANGLE provenance, owner, generation,
configuration, live mount, advancing clocks and scientific qualification must
pass their integrity checks.

The recorder stores at most 256 roots with 512 spans per root per state.
Capacity overflow invalidates the capture. Original scheduled-callback start
and end timestamps and timing-vector indices identify each root. J acquisition
and consumption precede E acquisition, preserving transient-view lifetimes.
Stages cover acquisition, validation/reduction, Fourier work, readouts,
controls/audio, history preparation and actual chart/native calls. Existing
unknown field sample times remain unknown. Native internal phases and worker
sampling CPU are outside this measurement's attribution scope.

Analysis uses matching invocations. Exclusive time is a parent's duration less
the union of its immediate children's intervals. A complete tree must
reconstruct each root and match its original callback vector. No independent
percentiles are subtracted. Visible active updates, waiting/empty observations,
unchanged-history callbacks and standalone resize/window work remain separate
cohorts. Baseline/routed comparisons are descriptive and include observer
effects; they cannot establish an uninstrumented speedup.

## Execution and evidence contract

Root owns serial browser execution; agents review independently. No other
test, build or GPU workload runs during timing. The coordinator runner
`engine/build/run_scale0_wave_attribution_20260908.py` verifies the reviewed
manifest and all baseline source hashes before and after each invocation.
Exclusive run reservations precede Playwright output cleanup. Per-row raw data
is saved before assertions; audit-stop failure still attempts recorder export.
An invalid or absent result stays invalid or absent.

Every invocation receives a new run ID. Its raw reports, reservation/log and
browser failure artifacts are archived before another invocation. Failed
attempts remain evidence. A source or probe repair requires a new preserved
revision, re-review of the affected contract and a new run ID; observed failures
do not authorize changing the workload or acceptance thresholds.

After valid attribution, propose only changes supported by the measured work.
Any production successor requires separate value/lifecycle equivalence and the
unmodified performance acceptance gate. Spectrum attribution, Wave's component
headings, common J/E sample provenance, remaining Time/Gravity callback failures
and physical recovery remain open work outside this diagnostic.

## Review and disposition

The sealed probe manifest SHA256 is
`a9f763396e72a59cde8aba4927a547c5fa12deeaf0fcc9969b7bafaec7120259`
(37 files). Preparation has 14 passing isolated checks, independently repeated
by the campaign reviewer. A separate transform reviewer reproduced five focused
semantic checks, including the 48 held-analysis comparisons. These counts
overlap; they are not 33 distinct checks. Both reviewers report no blocking
defect in the admitted scope.
The [transform review](evidence/scale0-wave-attribution-v1-transform-review-2026-09-08.json)
has SHA256 `4f892129bc44e5ec444a12a1cd31954303d568891a4277193e7f8f7482caab50`;
the [campaign review](evidence/scale0-wave-attribution-v1-campaign-review-2026-09-08.json)
has SHA256 `fe73f40e1f3538493d6fb841f4c9d0222ed9ab6301e22ac773497d72ba762e99`.
Their pending-browser statements describe review time and are preserved.

The coordinator's [mounted-browser equivalence invocation](evidence/waveattr-v1-equivalence-a1-execution.json)
passed one test covering baseline/routed DOM, actual uPlot data, histories,
waiting/empty recovery, invalidated J views, window controls, reset and unmount.
All 2,176 production and 37 probe files matched before and after execution.
Its immediate archive is `engine/build/waveattr-v1-equivalence-a1-results.zip`,
SHA256 `993f526187b15640cec297744a2154da7932bba0316bf9b425b7a2f443303c98`.

The coordinator runner SHA256 is
`e3aa96cd051c64ea63a743f3317871eee07a4ffe22c2541fa0ed3fa1d07c9b8a`.
Its initial revision lacked a fixed baseline-manifest pin and is preserved as
`engine/build/run_scale0_wave_attribution_20260908.pre-review1.py`.
The reviewed revision pins the baseline manifest, recomputes its source-list
identity, enforces 2,176 entries and rechecks runner/manifest bytes after each run.
These are diagnostic integrity results, not production performance improvement.

## First diagnostic execution

The [45-file pre-timing freeze](evidence/scale0-wave-attribution-v1-freeze-2026-09-08.json)
binds the probe, reviews, coordinator, program and passed browser invocation.
Its archive is `engine/build/scale0_wave_attribution_v1_candidate_20260908.zip`,
SHA256 `08414b4e6f49c8c116de02af563bfd9e47b64e572f1522dc2f3008674b1bbf6c`.

The [baseline invocation](evidence/waveattr-v1-baseline-a1-execution.json)
passed both sizes and all eight states. Its immediate nine-file archive,
`engine/build/waveattr-v1-baseline-a1-results.zip`, has SHA256
`e90d65e656253e3b3625affc74993e83fa620651a1588a662f3148a8b0e93d62`.

The [routed invocation](evidence/waveattr-v1-routed-a1-execution.json)
passed all four L33 states, then failed while waiting 90 seconds for
`window._ftdBridge` during the second test's initial navigation. L97 has zero
measurement rows. This navigation precedes L97 selection and worker qualification;
the failure is not evidence of a 97-cubed worker allocation problem.
Its retained console errors contain
`Failed to load resource: net::ERR_CONNECTION_REFUSED`; the failed resource URL
was not captured, so this message does not establish the cause of startup failure.
The immediate 11-file archive, `engine/build/waveattr-v1-routed-a1-results.zip`,
has SHA256 `67e46f4b97ca32eddac3916b00533563e9e6afa2cb87390832b28d9683f24fd7`.
It preserves the empty L97 report, screenshot, error context and complete log.
Both invocations verified zero drift in the 2,176 production and 37 probe files.

The routed L33 data remains available for independent reconstruction. No worker
attribution or complete two-backend diagnostic is claimed. A separate startup
investigation must preserve this failed invocation and original probe bytes;
its outcomes cannot replace or erase this first attempt.

## Independently reconstructed result and next cost work

The [independent results review](evidence/scale0-wave-attribution-v1-results-review-2026-09-08.json),
SHA256 `ca4d5e03afc0fef4f4ac2204c904b31e4118fc71ef466ad1bea77f6d1e9c6244`,
reconstructs the records without importing the probe author's analyzer.
All 1,068 aggregate fields, 8,114 span timing fields and 234 root totals match.
The retained populations contain 8,703 frames, 15,874 callback samples across
148 series, 60 actions and 4,057 spans. Twelve measured states pass diagnostic
integrity; four instrumented L97 states are absent.

The direct-WASM active cohorts contain 48 docked and 49 floated scheduled
callbacks. One docked callback has no metrics, 96 callbacks are hidden/collapsed,
and 40 resize roots are separate foreground work. Keeping those populations
separate, the paired exclusive cost shares are:

| Active L33 callback work | Docked | Floated |
|---|---:|---:|
| Acquisition, including native/embind boundary | 41.28% | 43.50% |
| Qualification and reduction | 32.32% | 31.88% |
| Formatted readouts and retained DOM | 19.42% | 18.79% |

These instrumented callback shares include measurement overhead. They do not
measure pure native CPU, worker sampling CPU or all asynchronous chart work,
and they cannot be subtracted from an uninstrumented percentile.

The next candidate should first address avoidable native sample-buffer work
and then consider exact reducer/readout changes. Preserve complete sample
support, ordering, thresholds/ties, transient-view lifetime, finite validation,
numeric expression order, histories and cadence. Any optimization must receive
independent exact-output checks and a fresh unmodified acceptance campaign.
The measured small Fourier/chart share provides no basis for prioritizing
further Fourier or synchronous chart micro-optimizations.

## Startup follow-up and second-attempt registration

The separately [reviewed startup harness](evidence/scale0-wave-startup-forensic-review-2026-09-08.json)
retains the same initial URL and 90-second readiness wait, with no lattice or
worker override. Its [forensic invocation](evidence/scale0-wave-startup-forensic-execution-2026-09-08.json)
passes both ordered baseline/routed cases, with zero failed requests or page
errors in 891/894 captured events. The original timeout was not reproduced and
its cause remains unknown. Trace and event-capture overhead prevent a startup
performance claim. The complete 21-file archive is
`engine/build/scale0_wave_startup_forensic_a1_20260908.zip`, SHA256
`ea1a7c10c1e78d7a0cef8b81da834eb5a2bb8de4128033f5049bb752ed8e209d`.

Before its execution, register one additional full routed invocation,
`waveattr-v1-routed-a2`, using the same 37-file probe, coordinator, scenario,
sizes, state order, warmups, durations, actions, 90-second startup wait and
trace-off policy. It addresses missing L97 observations after the first
attempt's startup failure. Both L33 attempts remain evidence; no best-run
selection or pooled percentile is authorized. Baseline a1 remains a separate
descriptive control. The first startup failure stays open even if a2 succeeds.
No production file, scientific operation or acceptance threshold changes.

The measured next-candidate contract is recorded separately in
`engine/build/scale0_wave_native_cost_v4_20260908/PLAN.md`, SHA256
`ba82b94ac581071e51d5ba26f76157f4d1fd57c7e15f82613f50941f0c8cc02e`.
Only WASM J reaches the CPU sampler's capacity-discarding assignment; E already
retains its own arrays. The first isolated implementation is limited to
preserving CPU output-vector capacity with exact metadata/value/lifetime tests.
A stride-one shortcut is a separate candidate. Drafting grants no adoption or
speedup claim and leaves the measured production baseline untouched.
