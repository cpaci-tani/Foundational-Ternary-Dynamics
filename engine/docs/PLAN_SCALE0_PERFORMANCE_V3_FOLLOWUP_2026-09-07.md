# Prospective Scale-0 performance v3 follow-up

Date: 2026-09-07. Status: **[OPEN] proposed work; no implementation or new certification in this document.**

This plan records source findings made during the frozen repair-v2 hardware campaign. The latest coordinator update reports 67 direct-backend cases with passing frame gates, with six visible-panel callback failures: docked/floated WaveLab, Spectrum and Time. The full 144-panel plus 144-overlay matrix was still running. These are interim observations, not its final disposition. The existing v2 candidate, artifacts, tests and evidence remain unchanged.

Reference records: [v2 candidate manifest](evidence/scale0-performance-v2-candidate-manifest-2026-09-07.json) and [direct panel measurements](evidence/scale0-hardware-repair-v2-panels-L33-2026-09-07.json). The latter was still being collected when this plan was written. Preserve its final result, including every failed row, independently of any later candidate.

## What the WaveLab evidence establishes

The coordinator reported direct L33 WaveLab callback p95 values of 3.375 ms docked and 3.25 ms floated against the unchanged 2 ms callback gate. These rows had zero added/removed live DOM nodes, but 480/490 counted canvas operations. Whole-page owner instrumentation recorded 371/372 diagnostics calls during approximately 12 seconds, with diagnostics p95 around 2.5 ms.

These measurements establish a remaining callback failure after DOM retention. They do **not** allocate elapsed time among native sampling, JavaScript arithmetic, detached markup parsing, reconciliation, chart drawing or browser layout. The method and callback distributions are not paired timing samples; do not subtract their quantiles. In particular, diagnostics timing is not nested WaveLab timing.

The read-only source audit traced these paths:

1. [WaveLab panel](../web/js/scales/scale0/ui/overlays/wave-lab-panel.js) runs its visible update at 4 Hz and calls `WaveInfoComponent.update` on the selected active owner.
2. [WaveInfo](../web/js/scales/scale0/ui/overlays/wave-lab/wave-info.js) calls `getSpectrumComparatorMetrics`, updates audio, produces readout markup and redraws five histories when a sample is appended or plots are built.
3. [Wave metrics](../web/js/scales/scale0/analysis/wave-spectrum.js) select stride 1 at L33, call the J and E samplers, reduce their emitted vectors into lanes, and calculate eight Fourier amplitudes per lane. At N=33, this repeats 528 sine/cosine evaluations per lane per update. The clock and three toggle reads are separate scalar calls.
4. [Direct WasmBridge](../web/js/bridge/wasm-bridge.js) delegates `currentTick()` directly to the native scalar. `getFluxVectorSampled` and `getEFieldSampled` delegate through `_wasmCallOr`, which performs method-presence checks and invokes the corresponding native method. None of those paths calls `getDiagnostics` or routes through the diagnostics capability.
5. [WASM field exports](../wasm/ftd_wasm.cpp) implement J through `copy_visual_field_sample(FluxVector, ...)` and E through the direct `E = -wave_vel` sample driver. The E-only path already avoids magnetic curl computation. Both return Float32 samples that are consumed synchronously by the metric reducer.
6. [Runtime diagnostics](../web/js/scales/scale0/runtime/diagnostics.js) separately collect telemetry every second animation frame. [TelemetryHub](../web/js/telemetry-hub.js) calls the owner's diagnostics capability, which delegates to `getDiagnostics`. This explains an actual approximately 30 Hz diagnostics call path without attributing it to WaveLab.

The wave-metric source, direct bridge source and native visual-sample implementation were unchanged from the frozen 2160-file native-observation-v3 candidate-v2 archive during this review. No tests, browser runs, builds or GPU work were performed for this diagnosis.

## Correctness repair: directional component selection

**Confirmed source defect, preexisting in the frozen baseline:** `spectrumComparatorLaneParams` supplies `componentIndex`, but `getSpectrumComparatorMetrics` constructs a new accumulator record without copying it. `reduceVectorSample` selects x when `row.componentIndex === 0`, y when it equals 1, and z otherwise. The omitted property therefore selects z for the x/y-labelled lanes too.

Affected quantities are directional J/W peaks, centre probes, the centre-line Fourier input and its harmonic amplitudes. The full vector energy, vector-magnitude peaks and energy centroid use different expressions and are not changed by this omission. The broad history fallback from a zero directional peak to the vector-magnitude peak can conceal the problem; testing only total energy or those fallback histories is insufficient.

Carry the existing lane component index into the accumulator and verify it through actual metric code. This is a **value-changing correctness repair**, not a value-preserving performance optimization. Do not claim that corrected directional numbers equal the old output or that these readouts establish physical wave recovery.

Required manufactured regressions before adoption:

- For x, y and z separately, supply one centre sample with J component +3 and E component -4 on the selected axis, other components zero. The selected J/W probes must be +3/+4, selected magnitude peaks 3/4, and combined vector energy 12.5 under the existing stride-1 convention. Exercise the actual registered x/y metric paths; use the actual reducer with a declared z accumulator for the z control if no registered z scenario exists. Do not add a physical scenario merely to host a test.
- Repeat with signed mixed vectors J=(2,-3,5), E=(-7,11,-13). Selected probes must follow their named component, W must retain the established minus-E convention, and the combined vector energy must remain 188.5. Assert the directional values directly, including real zero components; a vector-norm fallback is not an oracle.
- Feed a known centre-line impulse or exact finite Fourier fixture and compare all eight returned harmonic amplitudes to an independent finite-sum oracle with the same declared sampling support and normalization. A readout test that merely checks nonempty arrays cannot detect this defect.
- Cover absent/empty buffers, owner/scenario change, same-tick display refresh and the existing unavailable behavior. Preserve the existing sampling cadence and qualified-versus-approximate labels. Treat worker sampler provenance limitations independently rather than fixing them through a convenient clock substitution.

## Value-preserving optimization candidates

### Native output capacity

[Visual sample production](../src/visual_field_sample.cpp), `RenderBridge::copy_visual_field_sample`, resets its output with `out = {}` before reserving buffers. [VisualFieldSample](../include/ftd/visual_field_sample.h) owns two `std::vector<float>` members. Resetting the aggregate discards their prior capacity even when the caller, including the WASM flux wrapper, retains a static output record. At L33/stride1, the two requested maximum capacities total 862,488 bytes: 2 × 3 × 33³ × sizeof(float). This is a source-derived reservation size, not measured allocation time or emitted payload size.

Replace whole-record assignment with clearing vector lengths and explicitly resetting metadata, while retaining capacity. Preserve empty results, effective stride, origin, component count, strongest-flux block selection, strict tie order, thresholds and emitted ordering. A separate optional stride-1 fast path can avoid repeated block-bound computations because those blocks are singletons, but it needs its own exact-output checks.

Do not cache sample values by tick. Same-tick mutations remain possible, and reused buffers remain invalid for clients retaining zero-copy views beyond their declared synchronous consumption window.

### Fourier basis and support arithmetic

Cache the eight sine/cosine basis rows using the existing `kMode = 2 * Math.PI * m / N` and `kMode * x` expressions. Bound the cache, for example to the current N. Store Float64 results and preserve each sum's iteration order and normalization. This removes repeated state-independent trigonometry without removing Fourier modes or changing sample cadence. Check exact same-engine arithmetic results against the uncached implementation and a separate mathematical fixture.

The reducer currently computes vector magnitude before checking whether any lane admits that site. For typed numeric samples, defer that arithmetic until at least one lane admits the point; reuse it across admitted lanes. Preserve lane membership, sample order and every retained contribution. Do not silently reduce spatial resolution or conflate strongest-block representative samples with point samples.

### Dedicated WaveLab readout slots

The shared retained renderer removes live-node replacement, but changed numeric markup still invokes detached template parsing and traversal of the whole readout. A dedicated WaveLab body can build its stable structure once and update formatted values, titles and attributes directly. Keep the existing renderer as an equivalence oracle for all data and unavailable states during testing.

Retain exact displayed precision, labels and scientific scope; preserve audio delegation, scenario reseed handlers, controls, owner/reset behavior and all history samples. Keep independent resize and history-window updates. Five chart updates on five newly changed histories are expected work; their count alone does not establish that an additional redraw can be omitted. Any chart optimization must preserve all series, time coordinates, scales, range controls and visible sample definitions.

## Spectrum continuation

The two reported direct Spectrum callback failures remain part of v2's evidence. The bounded-worker-read reviewer supplied the following independent source diagnosis; no additional execution accompanied it. `spectrum-panel.js:update` calls `captureSpectrumObservation`, then the Scale-0 field-sampling capability, `WasmBridge.getSamplerOr` and the sampler contract, reaching seven direct native samplers. At L33/stride1, flux and divergence visit 33³ points and five metric outputs visit the 31³ interior. Kretschmann includes two full-grid passes and an 18-neighbor interior stencil; vorticity, helicity and coherence separately recompute curl. Only flux uses the capacity-discarding `copy_visual_field_sample` path identified above. The six scalar WASM templates and Kretschmann scratch grid already retain their buffers.

Four Spectrum renderers still replace readout `innerHTML`: hero, topology and metrics when the analysis mailbox is ready, and energy on live updates, under the existing 2 Hz schedule. A bounded next candidate can retain those four sole-owner div trees and skip identical markup, preserving every numeric string, SVG attribute, tooltip and empty state. Preserve async cancellation, complete job identity, sampler resolution and publication fences.

Another separate candidate is to make owned Float32 copies of actual Float32 WASM views while keeping Float64 copies for Float64 sources. This preserves the numeric values read as JavaScript Numbers and must never detach or retain mutable producer views. Current forced Float64 copies have a source-derived full-support upper bound of 7,641,520 bytes at L33; this is not measured allocation or elapsed time. Require source-type, copy-ownership and repeated-producer-invalidation regressions before adoption.

Native sampler fusion is a higher-risk option. Preserve the current WASM expression order and intermediate rounding, including Kretschmann's float face/edge accumulation; replacing it with a different CPU formula is not automatically a value-preserving optimization. Require frozen numeric oracles and separate parity evidence. Add bounded per-sampler and per-renderer attribution before assigning the callback deficit to one stage. No Spectrum component-selection defect was established by this diagnosis.

## Time continuation

The coordinator reports direct L33 Time callback p95 of 2.855 ms docked and 2.710 ms floated, both failing the unchanged 2 ms gate. These rows recorded zero added/removed nodes, 672/671 mutations and zero canvas operations. Retaining the live DOM therefore did not close the callback gate; mutation counts alone do not identify the dominant elapsed cost.

The current Time path still performs four native samples, their JavaScript reductions and detached markup rendering. Instrument these stages separately before choosing a further change. Preserve the existing sampler support and cadence, qualified observation domains, clock provenance, proper-time quadrature and history windows. Dedicated retained numeric slots are a possible follow-up only with exact markup/value equivalence and unavailable/reset tests. Neither a common tick cache nor reduced sampling is justified by these measurements. The full matrix and any final Time disposition remain pending the coordinator's completed evidence.

## Required next-candidate gates

1. Finish and archive v2 unchanged. Freeze the exact follow-up scope and manufactured correctness fixtures before implementing v3. Identify the component repair separately from each optimization in the change record.
2. Run actual-source tests for exact directional components, sampling counts, displayed values, retained-node lifecycle, history windows and cancellation. Preserve any failures and state precisely which fixture or implementation changed in response.
3. If native capacity or traversal changes land, build a fresh isolated native/WASM candidate and verify CPU/WASM sample parity across supported ABIs, sizes, strides, seams, sparse seeds, empty outputs and ties. Check complete values and metadata, not just counts. Preserve shared CPU/CUDA sampler contracts and run the affected WSL GPU parity checks when that shared scope changes. Refresh loader/build manifests and independently verify artifact identities; a JavaScript-only test does not validate a rebuilt native sampler.
4. Independently review both the value-changing correctness fix and the value-preserving candidates. A reviewer must not approve their own implementation. Confirm that no overlay mutates or substitutes the physics owner.
5. Freeze a fresh complete source/artifact manifest and archive before hardware measurement. Retain v2 results and previous artifacts as distinct evidence. Use the same registered hardware provenance, preparations, panel states, sample durations and gates, including the 2 ms callback p95 gate. Do not tune thresholds, lower resolution, shorten history, skip mandatory updates or redefine the preparation to turn a failed row into a pass.
6. Report full-frame and callback outcomes separately. A successful bounded interface campaign would certify the measured workload only. It would not certify an always-60-FPS guarantee, strict-discrete physical recovery, continuum dynamics, matter or higher-scale emergence. Test counts are engineering evidence, not physical derivations.

## Post-campaign amendment, 2026-09-08 UTC

The v2 campaign is now complete: 288 registered rows measured, 279 pass and
nine fail the unchanged panel callback p95 gate. All 144 overlay rows and all
288 frame gates pass. The final panel failures are direct L33 Wave Lab,
Spectrum and Time in docked/floated states; worker L97 Gravity in both visible
states and Time floated. The worker Time floated value is 2.060 ms and remains
a failure. See the [v2 final disposition](PROGRAM_SCALE0_PERFORMANCE_V2_2026-09-07.md#post-campaign-evidence-and-remaining-work).
Earlier interim numbers above describe the time of the source review only.

The separate [Time/Gravity contract](PLAN_SCALE0_TIME_GRAVITY_PERFORMANCE_V3_2026-09-08.md)
records exact sampling, normalization, stencil, history and ownership
requirements. It proposes retained Time value/SVG slots and a Gravity
same-publication full-volume maximum plus owned three-plane slab. The latter
removes the dense copy while retaining the full normalization scan. A plane
maximum or unrelated diagnostic maximum would change the observable.

The isolated Wave Lab draft preserves a copy of the frozen source and has
independently passed 14 actual-module tests and 12 corrected-reference parity
cases covering 48 complete scenario comparisons. This is draft evidence, not
an integrated production or hardware pass. The two value-preserving changes
are the current-N Fourier basis and deferred arithmetic for excluded sites;
the component-index correction is explicitly value-changing.

Independent follow-up also confirms three preexisting observation issues:

- Null J/E payloads can be reduced into active zero-valued metrics. Missing
  buffers must be distinguished from completed empty observations, and both
  producer demands must still be issued during waiting.
- Wave Info's directional-peak history falls back to the vector magnitude on
  a real directional zero. The finite directional value must take precedence.
- The metrics use an owner clock, with a missing-clock fallback of zero,
  although separately cached J/E observations can have unknown sample times.
  Fixing components or availability does not establish joint sample time.

The first two issues are assigned to a separate isolated draft revision with
numerical and actual-view regressions. The clock/provenance issue remains open
and must not be closed by relabelling a publication counter or owner tick as a
common field-sample tick. The original draft and all v2 evidence are retained.

Future hardware harness revisions must preserve raw frame/callback timing
vectors for independent percentile reconstruction and archive each completed
test invocation's transient evidence before the next Playwright invocation.
The v2 reports retain aggregate timing distributions and raw interaction
samples; their panel screenshots/error-context attachments were cleared when
the overlay suite started. This limitation is recorded without changing v2's
gate outcomes or rerunning selectively to erase its failures.

## Wave v3 disposition and next observation work

The [completed Wave v3 ledger](PROGRAM_SCALE0_WAVE_CORRECTNESS_V3_2026-09-08.md)
supersedes the pending-draft status above. Component, missing/empty and true
directional-zero corrections pass their bounded tests. The eight-row hardware
follow-up retains four visible callback failures, including two new worker
failures; all frame gates pass. Independent reconstruction of retained raw
timings agrees exactly. This result does not demonstrate a performance gain
or identify the cost of an individual change.

Before another cost change, measure inclusive native acquisition, JS
validation/reduction, retained readout and chart update spans within the same
callback invocation. Keep attribution runs separate from the unmodified
acceptance harness. Subtracting independently sampled percentiles is not a
valid stage decomposition. Preserve resolution, observations, validation,
history support and scientific sampling cadence when selecting a candidate.

A further source-confirmed presentation mismatch remains in
`wave-info.js`: history stores the selected directional peaks and probes,
while its static headings still say `Peak Flux |J|`, `Peak WaveVel |W|`,
`Sample Jx` and `Sample Wx`. The RF/light lanes select y. A subsequent Wave
observer revision should label each history with its actual selected
component and retain the same numerical data. This heading repair does not
resolve the separately open common J/E sample-time or missing-binding
availability contract. It is outside the Time/Gravity v4 implementation.
