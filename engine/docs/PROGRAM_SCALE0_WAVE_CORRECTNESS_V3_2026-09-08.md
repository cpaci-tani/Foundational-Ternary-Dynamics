# Scale 0 Wave Lab correctness v3

Date: 2026-09-08. Status: **reviewed fixes integrated; 43 integrated Node
tests and five mounted-browser tests pass. Hardware follow-up: 4/8 pass;
all four visible callbacks fail. Independent evidence audit complete.**

This bounded reference-observer wave follows the completed
[performance-v2 campaign](PROGRAM_SCALE0_PERFORMANCE_V2_2026-09-07.md#post-campaign-evidence-and-remaining-work).
That campaign retains 279/288 passing rows, nine callback failures and all
144 overlay passes. Its [evidence archive seal](evidence/scale0-performance-v2-evidence-archive-2026-09-07.json)
preserves the reports and audits separately from this successor. No native
law, strict candidate, WASM artifact, scenario, cadence or resolution changes.

## Changes, ownership and acceptance scope

The Wave implementation agent owns `wave-spectrum.js`, `wave-info.js` and
their numerical/mounted-browser regressions. The Time/Gravity reviewer
independently reviewed both isolated drafts and executed their Node suites.
The coordinator owns integration, this ledger and the focused hardware
harness. The bounded-read reviewer independently reviews that harness and
the final hardware evidence. Authors do not approve their own code.

| Change | Result and boundary |
|---|---|
| C1: missing lane component index | Named x/y directional peaks, probes and Fourier inputs now use the declared component. Prior x/y lanes fell through to z. Values intentionally change. |
| C2: unavailable observations | Null, pending and malformed J/E return an inactive waiting result; completed typed count-zero observations remain valid zero. Both getter demands continue, and J is reduced before a later E call can invalidate its view. |
| C3: directional zero histories | A finite directional zero remains zero; the vector norm is a fallback only when directional data is unavailable/nonfinite. |
| O1: Fourier basis | Retain only the current N's eight Float64 sine/cosine pairs, using the original angle expressions, summation order and normalization. No sampled state enters the cache. |
| O2: excluded-site arithmetic | Defer magnitude/energy arithmetic until a lane admits the site, while validating all declared numeric components. Support, sampling order and all admitted contributions remain unchanged. |

Validation accepts the actual Float32/Float64 sample shapes with a
nonnegative safe-integer count and sufficient triple-aligned storage.
Nonfinite declared components and per-vector derived overflow return waiting
instead of publishing partial metrics. This validation adds work; performance
improvement is not inferred from the arithmetic changes.

Worker readiness is checked through the existing `hasSamplerSnapshot` API.
A pre-publication typed empty placeholder is different from a completed empty
sample. The direct bridge's missing-binding empty fallback is still
indistinguishable through this consumer contract and remains **OPEN**.
The owner-clock/fallback-zero behavior also remains **OPEN**: independent
J/E availability does not prove common sample time. Malformed lattice
dimensions, aggregate overflow beyond the bounded finite test domain and
throwing third-party bridge implementations are not comprehensively handled
by this patch. No complete observation or physical-wave certificate follows.

## Reproducible implementation evidence

Both isolated drafts remain under `engine/build/scale0_wave_v3_draft_20260908/`
and `engine/build/scale0_wave_v3_draft2_20260908/`, with source baselines,
manifests, patches and logs. The second draft's four integration files were
copied only after matching every existing production baseline byte and
checking that the new Node test path did not exist. The
[integration record](evidence/scale0-wave-v3-integration-2026-09-08.json)
identifies those exact bytes.

| Check | Outcome | Evidence |
|---|---|---|
| Draft1 independent module/parity checks | 14 + 12 tests pass | [Review](evidence/scale0-wave-v3-draft-review-2026-09-08.json) |
| Draft2 author and independent module/parity checks | 19 + 12 tests pass in each run; 48 complete scenario comparisons in the parity group | [Independent review](evidence/scale0-wave-v3-draft2-review-2026-09-08.json) |
| Integrated production Node group | 43/43 pass: 19 metrics, 15 adjacent renderer/analysis and nine redundant-read tests | [Log](evidence/scale0-wave-v3-integrated-node-2026-09-08.log) |
| Actual mounted Wave Info and retained-readout browser group | 5/5 pass, including the two new directional-zero and missing/empty/recovery cases | [Log](evidence/scale0-wave-v3-browser-2026-09-08.log) |

These groups overlap and must not be summed as distinct tests. Manufactured
fixtures assert named x/y components and a private whole-module z control,
mixed signs, vector energy, genuine directional zeros, all eight Fourier
modes, stride weighting, support boundaries, cache replacement and same-tick
mutation. An adversarial E getter detaches J's buffers after J has been
consumed. Waiting states append no false zero history; completed empty data
does append a real zero; recovery resumes without repeated-tick duplication.
Mounted tests exercise actual Wave Info rendering with a chart stub; the
hardware follow-up exercises the actual dashboard and plot library.

## Preregistered hardware follow-up

Profile `wave-v3` in
[`scale0-comprehensive-performance.spec.js`](../web/tests/scale0-comprehensive-performance.spec.js)
selects exactly Wave Lab with `s0-field-rf-lattice-wave`, four states
(docked, floated, collapsed, hidden), direct WASM L33 and worker WASM L97:
**eight registered rows**. Physics plays in every state. This affected scope
does not renew the other seven v2 callback failures or certify all panels.

All existing gates remain unchanged: at least 600 frame intervals and
12 seconds; effective FPS >=59.5; frame p95 <=17 ms and p99 <=20 ms; no interval
over 33.4 ms and no Long Tasks; inclusive callback p95 <=2 ms and maximum
<=8 ms; ten paired actions for visible states with dispatch-to-next-rAF p95
<=50 ms; hardware renderer provenance, foreground visibility, stable owner,
configuration and qualification, advancing clock, correct mount, and no
hidden/collapsed canvas work or runtime errors. Nested owner reads remain
inside callback timing; standalone owner-read entries are diagnostic only.

The new profile requires its full frozen source/artifact identity in
`FTD_AUDIT_CANDIDATE_ID` and embeds it in each report. The probe retains raw
frame and callback vectors only when requested, copying already collected
vectors after measurement stops. Collection, scheduling and timing windows
remain unchanged. Existing profiles keep their earlier output names and
sample-retention behavior. New output names are
`scale0-hardware-wave-v3-panels-L{33|97}-2026-09-08.json`; existing files cause
refusal rather than overwrite.

The profile requires the complete ordered `33,97` invocation, rejecting
subsets and duplicates. Independent review injected a raw-copy exception and
found that it could bypass probe restoration. The coordinator added
`try/finally` cleanup around report construction and copying; the same probe
is restored and its global registration cleared on both success and failure.
The failed pre-repair probe remains part of the review chronology. A 64-hex
identity field alone proves no source binding; the independent freeze and
post-run source/archive checks supply that evidence.

Before measurement, independently review the harness changes and freeze a
distinct complete candidate manifest/archive. Run serially, with no competing
browser, heavy build or scientific campaign:

```powershell
$env:FTD_HARDWARE_WEBGL = '1'
$env:FTD_AUDIT_PROFILE = 'wave-v3'
$env:FTD_AUDIT_KIND = 'panels'
$env:FTD_AUDIT_SIZES = '33,97'
# Read FTD_AUDIT_CANDIDATE_ID from the reviewed frozen candidate manifest.
npm test -- scale0-comprehensive-performance.spec.js --workers=1
```

Capture the invocation's log, raw reports and transient test-results archive
before any next Playwright invocation, whether the test exits zero or nonzero.
Independently reconstruct all available frame/callback/action summaries and
gates, and reconcile source/artifact hashes after execution. Preserve failures;
do not lower resolution, change preparation, omit updates or retune gates.

## Hardware outcome

The complete registered invocation exited 1. The
[direct L33 report](evidence/scale0-hardware-wave-v3-panels-L33-2026-09-08.json)
and [worker L97 report](evidence/scale0-hardware-wave-v3-panels-L97-2026-09-08.json)
contain all eight rows and raw timing vectors. All eight frame, action,
ownership, clock, mount and runtime-error gates pass on hardware ANGLE/NVIDIA
RTX 5090. All four visible rows fail the inclusive callback p95 <=2 ms gate.
Hidden/collapsed rows pass and perform no canvas work.

| Backend | Docked p95, ms | Floated p95, ms | Disposition |
|---|---:|---:|---|
| Direct L33 | 4.115 | 3.965 | Both visible callback failures persist |
| Worker L97 | 3.825 | 2.655 | Two new callback failures versus v2's 1.895 / 1.940 ms |

These are descriptive comparisons between recorded runs. The correctness
changes restore directional values and introduce validation work; this
campaign does not attribute the timing difference to individual changes.
All visible rows retained five canvases and 480-490 draw operations, so the
callbacks exercised live rendering. This does not certify common J/E sample
time or complete observation provenance.

The [execution record](evidence/scale0-wave-v3-hardware-execution-2026-09-08.json)
identifies an immediate 11-file result archive containing both reports, the
full log, candidate manifest and all available transient test-result files.
Screenshots and error contexts were captured before any subsequent browser
invocation. Tracing was disabled by the registered harness; no trace is
claimed. The [post-run source check](evidence/scale0-wave-v3-postrun-source-check-2026-09-08.json)
matched all 2,172 current and archived source entries. The
[independent hardware review](evidence/scale0-wave-v3-hardware-review-2026-09-08.json)
reconstructed 5,801 frame samples, 9,110 callback durations across 96 series
and 40 actions. All 696 reported numeric values matched exactly without a
tolerance, and all eight log rows and failure flags matched. The reviewer
also checked both archived preregistration snapshots, all 11 hardware result
archive members, the separately scoped strict 69/44/73 source records, six
WASM artifacts plus build information, and the v2 source/evidence archives.
The live program now records the disposition; its archived preregistration
remains unchanged.

The [final verification](evidence/scale0-wave-v3-verification-2026-09-08.json)
and [evidence archive seal](evidence/scale0-wave-v3-evidence-archive-2026-09-08.json)
close this candidate's evidence collection. This permits preparing a distinct
successor while preserving all of this candidate's failures.

Wave v3 therefore passes its bounded correctness checks and **fails its
performance gate**. The earlier v2 279/288 result and its nine failures remain
separate evidence; the two new worker failures are not hidden in that count.
The complete raw report is retained even though assertion output also dumped
its vectors. A subsequent harness may show compact failure summaries while
preserving complete report attachments.

## Remaining program boundaries

Time, Gravity and Spectrum retain their separately specified follow-up work.
The [Time/Gravity contract](PLAN_SCALE0_TIME_GRAVITY_PERFORMANCE_V3_2026-09-08.md)
requires the actual full-volume normalizer, exact stencils, observation
ownership and complete histories. Its [v4 implementation program](PROGRAM_SCALE0_TIME_GRAVITY_V4_2026-09-08.md)
records isolated drafts and the same-publication batch contract. The
[Wave Lab/Spectrum plan](PLAN_SCALE0_PERFORMANCE_V3_FOLLOWUP_2026-09-07.md)
separates native sampling costs from UI costs. Owner/sample clock provenance
and direct missing-binding availability need an explicit backend observation
contract; an owner tick or local cache revision cannot supply missing sample
identity. Production replay remains deferred, while strict checkpoints remain
mandatory. Strict continuum, restoring matter, spectra, chemistry and causal
macroscopic recovery retain their research gates. No commit, public deployment,
canonical adoption, universal FPS guarantee or full release is approved here.
