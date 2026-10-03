# Mind's Eye SR repair evidence

**Scope:** the supplied `MINDS_EYE_SR_AGENT_HANDOFF.md`, items A–E.
**Baseline:** `4e3853a06a458e0d5eaac2fc3d7c4bdfffe4596f` on `main`.
**Repair branch:** `codex/minds-eye-sr-repair`; the patch commit is the commit
containing this ledger (`git log -1 -- engine/web/docs/AUDIT_MINDS_EYE_SR_REPAIR.md`).
**Recorded:** 2026-10-02, America/Chicago.

The initial checkout contained unrelated Scale-0 UI/viewport/test changes and
two HTML deletions. Those changes were preserved and excluded from the repair
commit. Full-dashboard browser checks ran in that shared checkout; the small
SR shader harness imports the production Observer session/renderer directly.
The adopted Minkowski model remains separate from native FTD. No `G*`
correction or new physical assumption was introduced.

The [API/schema and physics conventions](REF_MINDS_EYE_SR_CONVENTIONS.md) are
part of this patch. The tracked [JSON evidence](evidence/MINDS_EYE_SR_REPAIR.json)
contains device/precision, residuals, boundary cases, actual accelerated optical
events, fixed-resolution timings and hardware gate outputs. LF-normalized source
SHA-256s identify the tested Observer files across Git line-ending conversion.
Verbose local logs are retained under
`engine/web/validation/observer-sr-*.log` (ignored generated evidence).

## Discrepancy ledger

| Item | Disposition | Reproduction and repair evidence |
|---|---|---|
| A: force semantics | **FIXED** | The transverse fixture already produced invariant acceleration `5/6`; the old field/UI called its input proper acceleration. Canonical `coordinateForcePerMass`, deprecated same-meaning alias, conflict rejection, point-marker restriction, Playground boundary, polar reflection and assistant clarification now agree. `observer-sr-migration`, `observer-sr-conventions`, `assistant-grounding`, real browser authoring. |
| B: motion and clocks | **FIXED** | Before repair, real observer/emitter positions differed by `0.00372677996249967` at 60 steps; a helper interval violated its displacement/clock Minkowski identity. Shared KDK now gives matching observer/emitter state and second-order convergence. Stored interiors, clocks, half-open joins, reversals, clipping and event splits are exercised. An explicit 7.6-unit light-clock step also reproduced a missing event at time 5 and now retains all events. |
| C: unit-D color | **FIXED** | Actual prepatch final pixels differed on/off: `[0.8118102,0.8118102,0.8118102]` versus `[0.8460173,0.8332134,0.8121393]`. One three-line conversion now runs in both branches. Unit-D equality passes on the compiled shader; 192 linear samples cover materials, motion and all color/beaming combinations. Sky/ground unit-D paths also match exactly. |
| D: executed runtime/GPU coverage | **FIXED** | Real session, worker, full browser and compiled WebGL float readback now cover the audit's missing execution paths. 720 optical cases: 698 stable hits, 18 shared misses and 4 separately conditioned cases; zero stable-hit failures. No screenshot or CPU shader translation is used as GPU evidence. |
| D: tiny boosts/projection | **FIXED** | A prepatch `1e-12` boost discarded a measurable time term, and the cached projector returned `x=0.5` instead of `0.500000000000866`. Stable Lorentz coefficients now preserve it; invalid velocity behavior is retained. |
| D: established SR identities | **ALREADY_FIXED_AND_VERIFIED** | Doppler velocity formula, opposite boost conventions, retarded roots, inertial rest geometry and point-collision momentum laws were retained, with seeded independent checks. Label-center root evaluation was rationalized to share the stable root convention. |
| E: cutoff/reset meaning | **FIXED** | Cutoff rejection and intentional relocation were preserved; HUD and read-only export now expose available history and clock-origin provenance. Negative prehistory readings and physical/reflected positions survive. Known cutoff witnesses differ from ordinary empty rays; no missing event is fabricated. |
| E: bounded ownership/lifecycle | **ALREADY_FIXED_AND_VERIFIED** | Existing fixed tick/FIFO, epochs, pause/backlog, save/load/undo/scrub, revision/deletion, context restoration and native lattice isolation regressions remain passing. |

No handoff acceptance test remains blocked. Extra assistant-model inference
evidence and unrun platform/scope limits are reported below, rather than counted
as successful SR certification.

## Environment and test counts

Windows 11; Node **24.11.0**, TypeScript **5.9.3**, Playwright **1.59.1**,
vendored Three.js **r169**, Rapier compatibility **0.20.0**. Browser:
**Chromium 147.0.7727.15**. GPU:
`ANGLE (NVIDIA, NVIDIA GeForce RTX 5090 (0x00002B85) Direct3D11 vs_5_0 ps_5_0, D3D11)`.
`EXT_color_buffer_float` readback available; fragment `highp` has **23 bits**
of precision and exponent range **127/127**. Hardware runs explicitly set
`FTD_HARDWARE_WEBGL=1`; renderer identity was recorded, not inferred from the
machine specification.

| Execution | Pass | Fail | Skip | Interpretation |
|---|---:|---:|---:|---|
| Existing Observer Node baseline | 205 | 0 | 0 | Before production edits. |
| Initial focused reproduction | 4 | 3 | 0 | Exposed observer/emitter mismatch, inconsistent helper drift clock and tiny boost. |
| Actual unit-D GPU reproduction | 0 | 1 | 0 | Exposed toggle mismatch before shader repair. |
| Supplied ZIP original mathematics | 19 | 0 | 0 | Its bundled historical source. |
| Supplied tests with current import paths only | 19 | 0 | 0 | Includes historical transcriptions; see limitation below. |
| Original browser selection, software baseline | 53 | 1 | 0 | Environment-preset loop exceeded its 120-second bound under SwiftShader. |
| Original browser selection, hardware after repair | 54 | 0 | 0 | Same functional acceptance, unchanged thresholds. |
| Final Observer Node | **224** | **0** | **0** | Existing + new session, optical, migration and spectrum regressions. |
| Assistant Node | **230** | **0** | **0** | Grounding, actions, receipts, schemas and stale guards. |
| Final Observer browser selection | **61** | **0** | **0** | Original 54 + 7 new GPU/color/worker/performance tests. |
| Hardware workload gate, initial overlapping run | 2 | 2 | 0 | Overlapped another GPU measurement; retained as failed evidence. |
| Hardware workload gate, isolated run | **4** | **0** | **0** | All original thresholds retained. |
| Fixed-resolution before/after cost comparison | 2 | 0 | 0 | One deterministic 120-frame run per version, sequentially. |
| Additional assistant browser smoke | 13 | 1 | 0 | Local-model automatic scenario design failed measurement validation; separate from SR. |
| Isolated local-model comparisons | 2 | 0 | 0 | Original assistant modules passed once; current modules passed once on retry. |

Observer/assistant typechecks, targeted ESLint, Observer dependency check
(42 modules, 74 dependencies), and `git diff --check` pass. The final Node
suite also checks the cached tiny-velocity projection. Browser context-loss
and restoration checks execute the existing actual context extension.

The supplied review's entity loop is explicitly a source transcription, and its
last spectral test is a CPU translation demonstrating a historical palette
difference. Passing those tests is retained audit provenance; it is **not**
counted as session execution or GPU repair evidence. The new tests execute the
actual production owners and compiled shader instead.

## Simulation accuracy

For `f=0.5`, `T=1`, from rest, below the cap, independent smooth analytic
oracles give `x=(sqrt(1.25)-1)/0.5` and `tau=asinh(0.5)/0.5`.
Observer and marker agree to `1e-12` across the convergence fixture; oblique,
reversal and clipped replay agreement is checked to `2e-12`.

| Steps | Absolute position error | Absolute proper-time error |
|---:|---:|---:|
| 60 | 1.6462055783e-6 | 2.0704534431e-6 |
| 120 | 4.1154480032e-7 | 5.1760958486e-7 |
| 240 | 1.0288578700e-7 | 1.2940215843e-7 |
| 480 | 2.5721419966e-8 | 3.2350522372e-8 |

Observed order is approximately **2.0**, exceeding 1.8; both 120-step errors
are below `2e-6`. Interval position/proper time and joins are independently
checked, including scrub evaluations at interior times. A separate stable
3D closed-form integral covers nonzero initial velocity, oblique force,
deceleration through rest, rotated and zero-force cases. Application clipping
is compared against the declared constrained policy, not that smooth oracle.

2,000 seeded Float64 cases (`0x5eed2026`, `|v|<=0.99`) yield maxima:
interval relative residual **1.34e-14**, inverse boost **9.94e-15**, null ray
**5.33e-15**, retarded-root relative residual **1.43e-15**, and scaled
Doppler/clock derivative residual **2.16e-10**. Another 2,000 seeded point
collisions give maximum relative four-momentum residual **2.67e-15**.
The fixed-clock derivative is also independently bisected through real
accelerated session history, within one segment rather than across a kick.

The actual browser worker replays 120 identical scheduled tick inputs as
individual advances versus eight batches of 15 ticks. Its observer clock is
`0.9624241677287918`; a forced beacon has the same clock and 121 evolving
intervals. States and retained histories agree, reset/load advance
provenance and the stale epoch response is rejected.

## Optical accuracy and display approximation

The GPU tests render a 1-pixel diagnostic target using the **production GLSL**.
CPU Float64 first-hit geometry is compared against GPU index-derived identity,
revision, distance, emission event, proper clock, source position and Doppler.
Float64 CPU bases plus small GPU offsets retain precision after `1e7`
coordinate/time/clock-origin translations. Actual position offsets, not CPU
reconstruction of the shader intersection, are read back.

The fixed matrix covers 20 shapes, observer speeds 0/0.6/0.99, nonzero object
rotation and camera yaw/pitch/roll, both reflected states, and three sample rays.
The visible historical source revision is tested with its current entity marked
deleted. Stable interior budgets were declared before patched evaluation:
`0.003` absolute distance/event/clock/position and `2e-4` scaled Doppler error.
Stability means both `±1e-4` neighboring rays preserve revision/reflection and
distance within `0.02`. This condition separates the four reported plane
boundary cases rather than removing arbitrary mismatches. Their identity and
distance are checked separately (`0.05` conditioning budget).

| Quantity | Maximum observed error |
|---|---:|
| Distance | 2.2066495951e-5 absolute; 2.6001457956e-6 scaled relative |
| Emission time | 2.2066757083e-5 absolute |
| Proper source clock | 1.8854203919e-5 absolute |
| Physical source position component | 2.1463260055e-5 absolute |
| Doppler | 7.7055366518e-7 absolute/scaled relative |

All 698 stable hits pass. All 18 shared misses agree. Four boundary cases are
reported in the JSON. Independent sphere silhouette rays at offsets `±1e-4`
check hit/miss and grazing precision separately; near/far center distances
`0.01` and `50` retain a `0.003` distance budget. At an exactly tangent ray,
either Float32 hit/miss classification is permitted, but a reported hit must be
within `0.004` of the analytic tangent distance.

The actual accelerated marker event (revision 109) reports CPU/GPU proper time
`0.8760965386591321 / 0.8760965765592603`, emission time
`0.9043489086138895 / 0.9043489098548878`, and Doppler
`0.9112097361141634 / 0.911209762096405`.

192 spectral readbacks cover black, white, reference lines, mixed weights,
approaching/receding/oblique sources, moving observers and all four toggle
combinations. Maximum scaled linear-display error against the CPU spectral
reference is **1.2389683287e-6** (fixed budget `3e-5`). Unit-D equality is exact
for material final pixels and the sky/ground linear paths. Shading, overlays,
vignette and feedback are disabled for linear comparisons; their ordinary
enabled behavior is exercised separately by existing browser regressions.
This certifies the declared three-line approximation on the recorded device,
not arbitrary source-spectrum reconstruction or device-independent precision.

The read-only diagnostic fixture in the JSON has observation time/observer clock
`0`, source revision `1`, emission time/source clock `-7.5`, Doppler `1`,
history `[-60,0]`, and physical/apparent emission position `[0,3,-1.5]`.
Tests preserve the snapshot and session byte-for-byte across the export and
separately verify reflected/source distinction and cutoff versus empty rays.

## Performance and failures retained

The fixed 320×200 scene has 12 accelerated beacons, two simulation seconds,
120 warmed frames with `gl.finish()`, fixed quality and no feedback. The baseline
test routes only Observer JS from the audit commit; it never resets the working
tree or changes dependencies. Runs are sequential. These are measurements,
not assertions of universal throughput:

| Cost | Baseline | Repaired |
|---|---:|---:|
| Retained segments | 2,892 | 2,904 |
| Mean step time (ms) | 0.014375 | 0.015333 |
| Mean completed render (ms) | 6.123792 | 6.219583 |
| Render p95 (ms) | 8.520 | 8.790 |
| Mean frame interval (ms) | 16.417500 | 16.444417 |
| Frame p95 (ms) | 18.005 | 18.080 |

There is one additional retained interval per marker. Shared midpoint work and
clock-base packing add bounded work: measured render mean changes about 1.6%,
frame mean about 0.16%. Timings have run-to-run variability; a separate final
suite sample had render mean 5.035 ms. No unbounded history or per-pixel full-world
scan was introduced. BVH tracing and the single authoritative owner remain.

The existing 1920×1080 requested workload gate includes 2,000 static + 64 moving
instances, 99,998 triangles, 75,371 mesh BVH nodes, eight layers, 32 labels,
mirroring and camera echoes. It uses its existing adaptive resolution:

| Configuration | Isolated FPS | p95 / p99 (ms) | Final image resolution |
|---|---:|---:|---:|
| Default immersion | 59.8031 | 16.67 / 16.67 | 1555×874 |
| Maximum echoes, Julia | 59.8031 | 16.67 / 16.67 | 1020×573 |
| Mandelbulb, maximum echoes | 59.9026 | 16.67 / 16.67 | 1020×573 |
| Wave/spacetime instruments | 59.5065 | 16.67 / 16.67 | 1399×787 |

Original gates are FPS ≥59.5, p95 ≤17 ms, p99 ≤20 ms. All four isolated tests
pass unchanged. Maximum single intervals still reach 33–50 ms; the result is
not a no-stutter guarantee or native-1080p guarantee. The earlier overlapping
run failed Julia (59.2129 FPS, 33.33 ms p99) and wave/spacetime (59.0188 FPS,
33.33 ms p99). Those failures remain recorded. GPU contention is a possible
cause; the isolated pass does not identify every individual delay.

The software baseline's environment-loop timeout was retained and the same
test subsequently passed on hardware. No tolerance or timeout was loosened.

## Reproduction

From the repository root, using installed locked dependencies:

```powershell
npm run test:observer
npm run test:assistant
npm run typecheck:observer
npm run check:assistant-graph
npx eslint engine/web/js/observer/*.js engine/web/js/assistant/observer-control.js engine/web/js/assistant/grounding.js engine/web/js/assistant/model.js engine/web/js/assistant/decision-context.js
npx depcruise engine/web/js/observer --config .dependency-cruiser.json
$env:FTD_HARDWARE_WEBGL='1'
npm run test:observer:browser -- --output=../validation/sr-final-complete
npm --prefix engine/web/tests test -- observer-performance.spec.js --workers=1 --output=../validation/sr-performance-isolated
$env:FTD_SR_BASELINE='1'
npm --prefix engine/web/tests test -- observer-sr-performance.spec.js --workers=1 --output=../validation/sr-performance-before-isolated
Remove-Item Env:FTD_SR_BASELINE
npm --prefix engine/web/tests test -- observer-sr-performance.spec.js --workers=1 --output=../validation/sr-performance-after-isolated
npm run test:assistant:browser:smoke
git diff --check
```

The supplied ZIP was extracted without overwriting sources into
`engine/web/validation/supplied-review/minds_eye_sr_review`:

```powershell
node --test engine/web/validation/supplied-review/minds_eye_sr_review/tests/*.node.test.mjs
# Copies with only their math import changed to ../../js/observer/math.js:
node --test engine/web/validation/supplied-current/*.node.test.mjs
```

Prepatch reproduction uses the new conventions test's original first seven
fixtures and the `observer-sr-baseline.spec.js` D=1 fixture against the baseline
source; logs retain the original failing values. Later long-step event and tiny
projector reproductions are retained separately. Use an isolated baseline
checkout for reproducing old failures; do not reset a shared worktree.

## Remaining limits and additional evidence

The required SR regressions are available and executed; **BLOCKED: none** in
that scope. Other devices/backends, remote JEV approval, a full-project build,
and C++/CUDA/WSL physical campaigns are **NOT_RUN** and are not inferred from
these results. The extra assistant browser suite uses an offline approval
fixture; its actual local-model test runs WebGPU inference separately.

The additional local model (`SmolLM2-360M-Instruct-q4f16_1-MLC`) initially
generated duplicate `vacuumBaselineEnergy` entries plus unsupported measurement
names for automatic scenario design. Existing validation rejected the draft
with “Choose 1 to 12 unique measurements from the template allowlist,” recording
zero of four requested ticks. The 13 other smoke tests passed. An isolated
comparison using the four original assistant modules passed once; a current-code
retry also passed once. These samples do not establish deterministic model
reliability. The failed generated response remains
in the evidence, and no measurement allowlist was weakened for this SR repair.

Point-marker acceleration, finite piecewise-inertial steps, approximate reference
spectra, the 0.99c application ceiling, finite retained history, and Float32
silhouette conditioning remain explicit supported boundaries. Native FTD
relativity recovery remains a separate open scientific problem.
