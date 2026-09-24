/** Budgeted overlay sweeps. Data builders, worker lifetime, and job dispatch have separate owners. */
import { SCALE0_MASS_GRAVITY_SCENARIOS } from '../../../config/toggles.js';
import { getActiveScale0Capability, getActiveLatticeSize, getActiveScale0Bridge, getFlowLineSettings, isKnotTrackingActive } from '../state/store.js';
import { overlayWorkActive } from './knot-streamline-plan.js';
import { computeStreamlineParams } from './streamline-integrator.js?v=2';
import { createFieldSampleCache, createForceFieldCache } from './field-sample-cache.js?v=2';
import { ensureOverlaySched, buildOverlayJobs, runJob, fluidPanelLive, syncNativeTransportObservation } from './overlay-jobs.js';
import { cancelStreamlineJobs } from './overlay-worker-client.js';
// Preserve the public facade used by scenario and renderer contract consumers.
export { buildForceOverlayData } from './overlay-force-data.js';
export { buildDerivedSubstrateData } from './overlay-derived-data.js';

const POISSON_LATENCY_KIND_OVERRIDE = Object.freeze({ latency: 'poissonLatency' });

/**
 * Native mass-gravity scenarios expose the actual latency-Poisson solution as
 * FTS2 kind 17. Other scenarios and all proxy-oriented panels retain kind 8's
 * normalized |J|^2 view.
 */
export function scale0FieldKindOverrides(ctx, state) {
    const active = getActiveScale0Bridge(ctx, state) ?? ctx?.bridge;
    return active?.isNativeGPU && SCALE0_MASS_GRAVITY_SCENARIOS.has(state.currentScenarioId)
        ? POISSON_LATENCY_KIND_OVERRIDE
        : null;
}

// ══════════════════════════════════════════════════════════════════════
// Amortized overlay scheduler (web/engine-optimization-2026-05-31)
// ──────────────────────────────────────────────────────────────────────
// PROBLEM: the throttled overlay-update frame used to build EVERY active
// overlay — E/B/flux streamlines (RK4 over a freshly-built spatial index),
// up to four force-flow streamline fields, and ~15 scalar topology sheets —
// and upload them all inside a SINGLE animate() frame. The median frame is
// cheap; only this one frame spiked (40–55 ms with one light overlay,
// 160–215 ms with streamlines, 130–146 ms with all 33). Pure CPU work
// landing in one tick.
//
// FIX: split the build+apply of each active overlay into independently
// schedulable "jobs", give each a cost weight, and process them across N
// consecutive frames under a per-frame work budget. A persistent round-
// robin cursor remembers where the previous frame stopped, so the next
// frame resumes the same sweep. The expensive streamline jobs (EM/flux/
// force-flow) carry a large weight, so at most ONE of them lands per frame;
// the cheap scalar jobs pack in behind it up to the budget.
//
// OBSERVATION SCOPE: each kind is copied on its first lazy read and retained
// across this sweep. Different kinds can represent different engine ticks;
// this scheduler does not implement an atomic multi-field snapshot. Rendering
// is passive and bounded by job budgets, not a same-tick scientific certificate.
//
// SKIP-UNCHANGED: a fresh sweep is only started when the underlying data
// actually changed since the last sweep finished (a tick advanced, or the
// user toggled/dirtied an overlay). If the field is static between throttle
// boundaries, no sweep runs and zero overlay CPU is spent.

// Per-frame compute budget, in abstract "cost units". One streamline-field
// rebuild ≈ COST_STREAMLINE units; one scalar/topology pass ≈ COST_SCALAR.
// The budget is sized so a single streamline job fits in a frame but a
// second one is deferred — that is the whole point (spread the streamlines
// across frames). Cheap scalar jobs keep packing until the budget is spent.
export const OVERLAY_FRAME_BUDGET = 50;

// Safety-valve ceiling on how many frames a single sweep may span. This is a
// LAG cap, not the primary spreading mechanism: because the budget loop always
// runs at least the first remaining job each frame (the first-job exception
// below), a sweep of N jobs already finishes in ≤ N frames on its own. The
// ceiling only matters if a future change ever made a frame run zero jobs; it
// then force-drains the remainder so an overlay can never be stranded. It is
// set well above the realistic active-overlay count (~21 jobs at all-33) so it
// does NOT force a large catch-up batch in normal operation — that would
// re-stack the very spike we are removing. Worst-case overlay lag at all-33 is
// therefore the natural ~N-frame spread (≈ a few hundred ms), not this ceiling.
const OVERLAY_SWEEP_MAX_FRAMES = 30;

export function disposeFieldOverlayRuntime(state) {
    const sched = state?.overlaySched;
    if (!sched) return;
    // Full application teardown is the only path that destroys the committed
    // worker-owned tracker namespace. Ordinary overlay/sweep cancellation
    // keeps it so IDs and event histories survive temporary UI changes.
    cancelStreamlineJobs(sched, { terminate: true });
    sched.active = false;
    sched.sampled = null;
    sched.sampleCache = null;
    sched.forceCache = null;
}

export function updateFieldOverlays(ctx, state, viewportAdapter) {
    const recordOwner = getActiveScale0Bridge(ctx, state);
    if (recordOwner?.isFiniteRecord) {
        if (state.fieldFlags.showStateField) viewportAdapter.applyStateField(recordOwner.getSamplerOr('state'));
        return; // This law has no imported E/B/force/metric samplers.
    }
    state.fieldFrame += 1;
    const latticeSize = getActiveLatticeSize(ctx, state);
    const fieldThrottle = latticeSize > 96 ? 12 : (latticeSize > 48 ? 6 : 3);
    const sched = ensureOverlaySched(state);
    const knotTrackingActive = isKnotTrackingActive(state);
    syncNativeTransportObservation(ctx, state, sched);

    if (state.authoritativeLoad != null) {
        // This is a source-owner boundary. Do not let an old worker task block
        // the incoming authoritative scenario or retain its tracker namespace.
        cancelStreamlineJobs(sched, { terminate: true });
        sched.active = false;
        sched.sampled = null;
        sched.sampleCache = null;
        sched.forceCache = null;
        return;
    }

    const fluidLive = fluidPanelLive();
    if (fluidLive !== sched.fluidLive) { state.fieldNeedsUpdate = true; sched.fluidLive = fluidLive; }
    if (!overlayWorkActive(state.anyFieldActive || fluidLive, knotTrackingActive)) {
        // No visual overlays and no knot tracking — abandon any half-finished
        // sweep so a later re-activation starts clean rather than resuming
        // stale jobs. The job pool itself persists (its slots are reused next
        // sweep); only the sweep liveness + shared snapshot are cleared.
        getActiveScale0Bridge(ctx, state)?.replaceSamplerWants?.('overlays', []);
        sched.lastWantKeys = [];
        cancelStreamlineJobs(sched);
        sched.active = false;
        sched.sampled = null;
        sched.sampleCache = null;
        sched.forceCache = null;
        return;
    }

    // Sampler deliveries also set fieldNeedsUpdate, so that bit alone cannot
    // preempt an asynchronous worker job: doing so terminated every request
    // just before its response arrived. Only ownership changes invalidate an
    // in-flight immutable snapshot. Toggle-off is safe without cancellation
    // because every apply path re-checks current visibility before painting.
    const loadOwnershipChanged = sched.active
        && sched.loadGeneration !== (ctx._loadGeneration || 0);
    const flowSettingsChanged = sched.active
        && sched.flowLineSettingsVersion !== (state.flowLineSettingsVersion || 0);
    if (loadOwnershipChanged || flowSettingsChanged) {
        // A new authoritative load owns a new field namespace. A display-only
        // flow-line setting change merely discards provisional candidates; its
        // committed knot identities survive exactly as before.
        cancelStreamlineJobs(sched, { terminate: loadOwnershipChanged });
        sched.active = false;
        sched.sampled = null;
        sched.sampleCache = null;
        sched.forceCache = null;
    }

    // A sweep already in flight always continues to completion regardless of
    // the throttle — its field snapshot is fixed, and finishing it is what
    // bounds overlay lag. Only the START of a new sweep is throttle/dirty
    // gated.
    const sweepInFlight = sched.active && sched.cursor < sched.jobCount;

    if (!sweepInFlight) {
        // ── Trigger gate for a NEW sweep ─────────────────────────────────
        // A sweep starts only on a throttle boundary (or an explicit dirty),
        // and only when the underlying field data actually changed since the
        // last sweep:
        //
        //   • fieldNeedsUpdate — a one-shot dirty (overlay toggle / style
        //     change / scenario load), honoured even under global pause so the
        //     user sees a single frame of the frozen state after toggling.
        //
        //   • version moved    — `fieldDataVersion` (monotonic, bumped once per
        //     real tick in tick.js; a counter we own, NOT the frame-sync-
        //     consumed `latticeNeedsUpload` flag) differs from the value we
        //     latched at the previous sweep ⇒ a tick advanced the field.
        //
        // SKIP-UNCHANGED (optimization point 2): if neither holds — the field
        // is byte-for-byte the state we last rendered — we run NO sweep and
        // spend zero overlay CPU, whether the sim is globally paused or merely
        // scenario-paused (residual-motion mode). A static field therefore
        // shows static overlays. This intentionally drops the old behaviour
        // where importance-sampled streamlines re-randomised their seeds every
        // throttle frame against a frozen field (a visible jitter, and wasted
        // work); frozen field → frozen lines is both cheaper and more correct.
        // The live-physics hot path (running + scenario-running) advances the
        // version every frame, so it is unaffected.
        const version = state.fieldDataVersion || 0;
        const onBoundary = state.fieldNeedsUpdate || state.fieldFrame % fieldThrottle === 0;
        const dataChanged = state.fieldNeedsUpdate || version !== sched.lastVersion;
        if (!onBoundary || !dataChanged) return;

        // Latch the trigger and open a fresh sweep: attach a lazy sample cache
        // so each sampler kind runs at most once when a job first needs it.
        // buildOverlayJobs refills the persistent slot pool IN PLACE and sets sched.jobCount;
        // it allocates no new job array/objects in steady state.
        state.fieldNeedsUpdate = false;
        sched.lastVersion = version;
        sched.loadGeneration = ctx._loadGeneration || 0;
        sched.flowLineSettingsVersion = state.flowLineSettingsVersion || 0;
        const activeScale0Bridge = getActiveScale0Bridge(ctx, state) ?? ctx.bridge;
        const flowLines = getFlowLineSettings();
        const params = computeStreamlineParams(latticeSize, {
            inThreadWasm: !!activeScale0Bridge?.isWasm && !activeScale0Bridge?.isWorker,
            density: flowLines.density,
            length: flowLines.length,
        });
        const fieldCapability = getActiveScale0Bridge(ctx, state)?.capabilities?.scale0
            ?? ctx.bridge.capabilities.scale0;
        const acScale0ForSnapshot = (getActiveScale0Capability(ctx, state) ?? ctx.bridge.capabilities.scale0);
        sched.sampleCache = createFieldSampleCache(
            fieldCapability,
            acScale0ForSnapshot,
            params.stride,
            scale0FieldKindOverrides(ctx, state),
        );
        sched.forceCache = createForceFieldCache(fieldCapability);
        sched.sampled = sched.sampleCache.sampled;
        if (state.fieldFlags.showEField || state.fieldFlags.showBField || knotTrackingActive) {
            sched.sampleCache.ensureParticleData();
        }
        sched.running = !!ctx.running;
        sched.cursor = 0;
        sched.sweepFrames = 0;
        sched.forceAnimated = false;
        sched.forceFrame = null;
        buildOverlayJobs(ctx, state, sched, viewportAdapter, latticeSize, params);
        sched.active = true;
        // An empty job list (all active flags gated out by zero-count samples)
        // is a completed no-op sweep.
        if (sched.jobCount === 0) {
            sched.active = false;
            sched.sampled = null;
            sched.sampleCache = null;
            sched.forceCache = null;
            return;
        }

        // The post-load forced-repaint window (ctx._samplersPending, set by
        // loadScale0Scenario) is satisfied once a sweep actually produces jobs
        // from real sampler data. Disarm it HERE — not on the first worker
        // postFrame — because the worker proxy returns EMPTY on the first
        // _wantSampler(kind) call (the want is only just registered; the data is
        // a frame away). Clearing on that first empty frame would close the
        // window before any overlay rendered, leaving tick-0 overlays blank
        // until the next tick. Clearing on jobCount>0 guarantees the first
        // sweep that genuinely had data is the one that disarms the window.
        if (ctx && ctx._samplersPending) ctx._samplersPending = false;
    }

    // ── Drain jobs under the per-frame budget ────────────────────────────
    // Run jobs from the cursor until the budget is spent or the sweep ends.
    // A job whose individual cost exceeds the whole budget (a single heavy
    // streamline job) is still allowed to run when it is the FIRST job this
    // frame — otherwise it could never make progress. The hard frame ceiling
    // forces the remainder through if a sweep has dragged on too long. The loop
    // indexes into the persistent pool and dispatches via runJob — no per-frame
    // allocation.
    sched.sweepFrames += 1;
    const forceFinish = sched.sweepFrames >= OVERLAY_SWEEP_MAX_FRAMES;
    let spent = 0;
    while (sched.cursor < sched.jobCount) {
        const job = sched.jobs[sched.cursor];
        const isFirstThisFrame = spent === 0;
        if (!forceFinish && !isFirstThisFrame && spent + job.cost > OVERLAY_FRAME_BUDGET) break;
        const completed = runJob(sched, job);
        spent += job.cost;
        if (completed) sched.cursor += 1;
        else break;
    }

    if (sched.cursor >= sched.jobCount) {
        // Publish sampler ownership only after the WHOLE sweep has run. A
        // multi-frame sweep discovers dependencies incrementally: E/B may run
        // on frame 1 while Poynting runs on frame 2. Publishing the frame-1
        // partial set temporarily unwants Poynting; frame 2 then wants it again.
        // On a paused worker every new want immediately posts a sampler frame,
        // whose dirty callback starts another sweep and repeats the cycle at
        // ~30 Hz. Besides redundant worker work, each delivery costs a missed
        // UI frame. The cache persists for the entire sweep, so at completion
        // requestedKeys() is the exact, full dependency set.
        const wantKeys = [];
        if (sched.sampleCache?.requestedKeys) wantKeys.push(...sched.sampleCache.requestedKeys());
        if (sched.forceCache?.requestedKeys) wantKeys.push(...sched.forceCache.requestedKeys());
        sched.lastWantKeys = wantKeys;
        getActiveScale0Bridge(ctx, state)?.replaceSamplerWants?.('overlays', wantKeys);

        // Sweep complete. Release the snapshot so the next trigger re-samples;
        // the slot pool persists for reuse on the next sweep.
        sched.active = false;
        sched.sampled = null;
        sched.sampleCache = null;
        sched.forceCache = null;
    }
}
