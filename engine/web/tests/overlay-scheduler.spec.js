// @ts-check
/**
 * Scale-0 amortized overlay-update SCHEDULER regression spec (ticket W2-3).
 *
 * The web/engine-optimization-2026-05-31 campaign replaced the monolithic
 * "build EVERY active overlay in one throttled frame" path with an amortized
 * scheduler in `js/scales/scale0/runtime/field-overlays.js`:
 *
 *   - sampled ONCE per sweep (sched.sampled), every job reads that snapshot;
 *   - the build+apply of each overlay is a "job" with a cost weight;
 *   - jobs drain across consecutive animate frames under a per-frame budget
 *     (OVERLAY_FRAME_BUDGET=100), with a persistent round-robin cursor;
 *   - a NEW sweep only starts when the underlying data actually changed
 *     (fieldNeedsUpdate dirty, or the monotonic fieldDataVersion advanced).
 *
 * A sibling spec (reconcile-claims.spec.js test 4) already SMOKE-tests
 * preempt-on-toggle (toggle → fieldNeedsUpdate raised → consumed within a few
 * frames). THIS spec goes deeper and pins the three load-bearing scheduler
 * invariants so a future refactor cannot silently regress the perf work:
 *
 *   1. SKIP-UNCHANGED  — paused, no toggle ⇒ no new sweep; the data version
 *      the scheduler latched (sched.lastVersion) does not advance and no
 *      rebuild churn happens. Positive control: bumping fieldDataVersion DOES
 *      re-open a sweep (proves the guard is the gate, not an unrelated freeze).
 *   2. PREEMPT-ON-TOGGLE (deep) — toggling mid-sweep drops the in-flight sweep
 *      + its stale snapshot (sched.active→false, sched.sampled→null), consumes
 *      the dirty, and re-samples against current flags so no stale partial is
 *      stranded and the new flag is honored.
 *   3. WORK-BUDGET TIME-SLICING — a heavy overlay set (≥2 COST_STREAMLINE jobs)
 *      cannot complete in one frame because COST_STREAMLINE === the whole
 *      per-frame budget, so at most ONE streamline lands per frame and the
 *      build spreads across frames (observed: sweeps span ≥2 frames / a
 *      mid-sweep partial cursor is caught), with a timing backstop that no
 *      single frame stalls for hundreds of ms.
 *
 * ── Introspection surface PINNED (file:line at authoring time, 2026-06-01) ──
 * The store's `state` singleton IS the same object updateFieldOverlays(ctx,
 * state, …) mutates, so getScale0State() reads the live scheduler directly:
 *   - getScale0State / setFieldToggle / resetFieldFlags / markFieldDirty
 *                                  ← scales/scale0/state/store.js:90,105,99,164
 *   - state.fieldFlags             ← store.js:63 (the toggle bag; off-by-default)
 *   - state.fieldNeedsUpdate       ← store.js:65 (one-shot dirty/preempt flag)
 *   - state.anyFieldActive         ← store.js:66 (gates the whole pipeline)
 *   - state.fieldDataVersion       ← set in runtime/tick.js:64 (monotonic, +1
 *                                     per real field tick; NEVER cleared)
 *   - state.overlaySched           ← runtime/field-overlays.js:599 (the scheduler)
 *        .active      :603   sweep in flight
 *        .jobCount    :605   live job count for the current sweep
 *        .cursor      :606   index of next job to run
 *        .sampled     :607   the one shared field snapshot for the sweep
 *        .sweepFrames :610   frames elapsed in the current sweep
 *        .lastVersion :611   fieldDataVersion latched at last sweep start (init -1)
 *   - constants OVERLAY_FRAME_BUDGET=100, COST_STREAMLINE=100,
 *     OVERLAY_SWEEP_MAX_FRAMES=30   ← field-overlays.js:515,519,534
 *   - trigger gate (skip-unchanged): dataChanged = fieldNeedsUpdate ||
 *     version !== sched.lastVersion; if (!onBoundary || !dataChanged) return;
 *                                  ← field-overlays.js:962-965
 *   - preempt: if (state.fieldNeedsUpdate && sched.active){ active=false;
 *     sampled=null; }              ← field-overlays.js:926-929
 *   - one-streamline-per-frame budget gate:
 *     if (!forceFinish && !isFirstThisFrame && spent+job.cost > BUDGET) break;
 *                                  ← field-overlays.js:1002
 *
 * The scheduler is pumped on its OWN every rAF frame: app.js:728 calls
 * Scale0Controller.animateLattice → controller.js:314 updateFieldOverlays —
 * unconditionally, even while ctx.running is false (only the physics TICK is
 * gated by running, runtime/tick.js:6). So tests drive state + let rAF frames
 * flow; they do not call updateFieldOverlays by hand.
 *
 * ROBUSTNESS (mirrors reconcile-claims.spec.js / toggle-coverage.spec.js):
 *   - clicks via DOM dispatch el.click() inside page.evaluate, NOT page.click()
 *     (the panel-scale-header overlaps the toolbar in headless layout);
 *   - a one-frame-consumed signal is read SYNCHRONOUSLY in the same evaluate as
 *     the mutation that raised it (no rAF can fire mid-synchronous-block);
 *   - everything else polls with expect.poll and frame allowances;
 *   - console assertions go through the shared realErrors()/KNOWN_NOISE filter;
 *   - each test is independent and normalises overlay state at its own start.
 */

import { test, expect } from '@playwright/test';
import {
    gotoAndReady,
    attachConsoleWatcher,
    realErrors,
} from './_helpers.js';

const STORE = './js/scales/scale0/state/store.js';

test.beforeEach(async ({ page }) => {
    // WASM compile + Three.js + module graph need headroom on slower machines.
    page.setDefaultTimeout(20_000);
});

// Wait until the Scale-0 ctx is published (controller.js:206 sets window.__ftdCtx
// on enter, a hair after the bridge is ready). All three tests need it because
// they read ctx.running and let the controller's animate loop pump the scheduler.
async function waitForScale0Ctx(page) {
    await expect.poll(
        () => page.evaluate(() => !!window.__ftdCtx),
        { timeout: 15_000, message: 'window.__ftdCtx (Scale-0 ctx) never became available' },
    ).toBe(true);
}

test.describe('Scale-0 overlay scheduler invariants', () => {

    // ────────────────────────────────────────────────────────────────────
    // 1. SKIP-UNCHANGED guard.
    //
    // The scheduler tracks fieldDataVersion (monotonic, bumped only on a real
    // field tick) and latches it as sched.lastVersion at sweep start. With the
    // sim PAUSED and no toggle, the version never advances, so the trigger gate
    // (dataChanged = dirty || version!==lastVersion) is false and NO new sweep
    // runs — zero overlay rebuild CPU. This is the optimization that stopped
    // the per-frame importance-sampled streamline rebuild against a frozen
    // field.
    //
    // We assert the OBSERVABLE consequence two ways:
    //   (a) NEGATIVE — across many paused frames with one overlay on and no
    //       toggle, neither state.fieldDataVersion NOR sched.lastVersion
    //       advances (the scheduler is not re-sweeping / not re-latching a new
    //       version → it is skipping the unchanged field).
    //   (b) POSITIVE CONTROL — manually bumping state.fieldDataVersion (what a
    //       real tick does) makes the very next overlay frame open a fresh
    //       sweep: sched.lastVersion catches up to the new version. This proves
    //       the freeze in (a) is the skip-unchanged guard doing its job, not an
    //       unrelated dead loop.
    // ────────────────────────────────────────────────────────────────────
    test('skip-unchanged: paused + no toggle does NOT re-sweep; a version bump DOES', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForScale0Ctx(page);

        // PAUSE the sim and enable exactly one cheap overlay so the pipeline is
        // live (anyFieldActive) but the field is frozen. Clear the boot dirty so
        // the only sweep that can run is the one the toggle triggers.
        await page.evaluate(async () => {
            const { getScale0State, resetFieldFlags, setFieldToggle } = await import('./js/scales/scale0/state/store.js');
            window.__ftdCtx.running = false;      // global pause → tick.js returns early → no version bump
            resetFieldFlags();
            setFieldToggle('showEField', true);   // one overlay active
            const st = getScale0State();
            st.fieldNeedsUpdate = false;          // discard the toggle's dirty for a clean baseline
        });

        // QUIESCE first to kill a startup race: sched.lastVersion initializes to
        // -1 (field-overlays.js:611), so the FIRST boundary opens one sweep that
        // latches lastVersion to the current version (e.g. -1 → 0) even while
        // paused/dirty-cleared, because version!==lastVersion is momentarily true.
        // We poll until the scheduler has consumed that initial bump
        // (lastVersion === fieldDataVersion). Only THEN is the field genuinely
        // "unchanged since the last sweep" — the precise precondition the
        // skip-unchanged guard gates on. Comparing before/after this point is
        // race-free.
        await expect.poll(
            () => page.evaluate(async () => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                const st = getScale0State();
                const sched = st.overlaySched;
                return !!sched && sched.lastVersion === (st.fieldDataVersion || 0);
            }),
            { timeout: 10_000, message: 'scheduler never quiesced (lastVersion never caught up to the frozen version)' },
        ).toBe(true);

        const before = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            const st = getScale0State();
            const sched = st.overlaySched || null;
            return {
                hasSched: !!sched,
                version: st.fieldDataVersion || 0,
                lastVersion: sched ? sched.lastVersion : null,
                dirty: !!st.fieldNeedsUpdate,
                active: sched ? sched.active : null,
                running: !!window.__ftdCtx.running,
            };
        });
        // The scheduler must exist and we must really be paused.
        expect(before.hasSched, 'overlaySched must be instantiated').toBe(true);
        expect(before.running, 'sim must be paused for the skip-unchanged check').toBe(false);

        // (a) NEGATIVE: wait a comfortable window of paused frames with NO
        // toggle and NO version bump; nothing about the data changed.
        await page.waitForTimeout(750);
        const after = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            const st = getScale0State();
            const sched = st.overlaySched || null;
            return {
                version: st.fieldDataVersion || 0,
                lastVersion: sched ? sched.lastVersion : null,
                dirty: !!st.fieldNeedsUpdate,
            };
        });

        // Paused ⇒ no real tick ⇒ the monotonic data version is frozen. If this
        // advanced while paused, tick.js leaked a version bump (the skip-unchanged
        // signal would be meaningless).
        expect(after.version, 'fieldDataVersion must not advance while paused (no real tick happened)')
            .toBe(before.version);
        // The scheduler must not have re-latched a new version — i.e. it did not
        // open a fresh sweep against the unchanged field. A regression where the
        // scheduler rebuilds every throttle frame regardless of data would show
        // up as lastVersion churning or jumping.
        expect(after.lastVersion, 'sched.lastVersion must stay pinned while the field is unchanged (skip-unchanged guard)')
            .toBe(before.lastVersion);
        // No residual dirty should have appeared on its own.
        expect(after.dirty, 'no dirty should be raised by idle paused frames').toBe(false);

        // (b) POSITIVE CONTROL: simulate a real tick by bumping the data version
        // (exactly what runtime/tick.js:64 does). The next overlay throttle
        // boundary must open a fresh sweep and re-latch lastVersion to the new
        // value — proving the version IS the gate the skip-unchanged guard reads.
        const bumpedTo = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            const st = getScale0State();
            st.fieldDataVersion = (st.fieldDataVersion || 0) + 1; // mimic one tick advance
            return st.fieldDataVersion;
        });

        await expect.poll(
            () => page.evaluate(async () => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                return (getScale0State().overlaySched || {}).lastVersion ?? null;
            }),
            {
                timeout: 8_000,
                message: 'sched.lastVersion never caught up to the bumped fieldDataVersion — a real data change did NOT trigger a sweep (the skip-unchanged gate is stuck closed)',
            },
        ).toBe(bumpedTo);

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });

    // ────────────────────────────────────────────────────────────────────
    // 2. PREEMPT-ON-TOGGLE (deeper than the reconcile smoke test).
    //
    // The reconcile spec only checks "toggle → dirty raised → dirty consumed".
    // Here we pin the PREEMPT MECHANICS that protect against a stranded stale
    // partial result: when a toggle dirties the store while a sweep is IN
    // FLIGHT, updateFieldOverlays (field-overlays.js:926-929) drops that sweep
    // (sched.active→false) AND its now-stale shared snapshot (sched.sampled→
    // null) BEFORE the trigger gate, then the gate opens a fresh sweep against
    // current flags and consumes the dirty (:971). So the half-finished sweep's
    // partial paint is abandoned, not blended with the new flag set.
    //
    // We force an in-flight sweep with a heavy, particle-rich config (so a sweep
    // spans several frames and is reliably catchable mid-drain), then toggle a
    // DIFFERENT flag and assert:
    //   (i)   the toggle synchronously flips its flag AND raises fieldNeedsUpdate;
    //   (ii)  the in-flight sweep is preempted — within a couple frames the
    //         scheduler is NOT still draining the pre-toggle sweep against the
    //         old snapshot (dirty consumed back to false, and a fresh sweep
    //         cycle has occurred), so no stale partial is stranded;
    //   (iii) the newly toggled flag stays ON (the preempt honors it, doesn't
    //         silently revert it).
    // ────────────────────────────────────────────────────────────────────
    test('preempt-on-toggle: a mid-sweep toggle drops the stale sweep + snapshot and honors the new flag', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForScale0Ctx(page);

        // Load a particle-rich scenario and a HEAVY overlay set on the LIVE
        // bridge so the scheduler has many jobs (multiple streamline jobs +
        // scalar sheets) — a sweep then takes several frames, giving us a real
        // in-flight window. Run the sim so versions advance and sweeps re-open.
        await page.evaluate(async () => {
            const { getScale0State, resetFieldFlags, setFieldToggle } = await import('./js/scales/scale0/state/store.js');
            const b = window._ftdBridge;
            if (typeof b?.setupScenario === 'function') {
                b.setupScenario('s0-vacuum-proton'); // manifests quarks → E/B/force samplers non-empty
                for (let i = 0; i < 8; i++) b.tick();
            }
            resetFieldFlags();
            // Heavy streamline + scalar load (each streamline = COST_STREAMLINE).
            for (const k of ['showEField', 'showBField', 'showFluxLines',
                'showPsiSquared', 'showPhase', 'showEmEnergy', 'showVorticity', 'showHelicity']) {
                setFieldToggle(k, true);
            }
            const st = getScale0State();
            st.fieldNeedsUpdate = false;
            window.__ftdCtx.running = true;       // live: versions advance, sweeps re-open
        });

        // Wait until the scheduler is actually building multi-job sweeps (jobCount
        // grew past 1) — confirms the heavy config produced a real sweep to preempt.
        await expect.poll(
            () => page.evaluate(async () => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                return (getScale0State().overlaySched || {}).jobCount ?? 0;
            }),
            { timeout: 10_000, message: 'scheduler never built a multi-job sweep from the heavy overlay set' },
        ).toBeGreaterThan(1);

        // (i) Toggle a DIFFERENT, currently-off flag and read the dirty
        // SYNCHRONOUSLY in the same evaluate (setFieldToggle raises it; the rAF
        // loop would consume it a frame later, so a second round-trip would race).
        const onToggle = await page.evaluate(async () => {
            const { getScale0State, setFieldToggle } = await import('./js/scales/scale0/state/store.js');
            const st = getScale0State();
            const wasActive = !!(st.overlaySched && st.overlaySched.active);
            const cursorAtToggle = st.overlaySched ? st.overlaySched.cursor : -1;
            const jobCountAtToggle = st.overlaySched ? st.overlaySched.jobCount : -1;
            setFieldToggle('showDivField', true);  // a new overlay not previously on
            const after = getScale0State();
            return {
                wasActive,
                cursorAtToggle,
                jobCountAtToggle,
                flag: !!after.fieldFlags.showDivField,
                dirty: !!after.fieldNeedsUpdate,
            };
        });
        expect(onToggle.flag, 'toggling showDivField on must flip the flag').toBe(true);
        expect(onToggle.dirty, 'toggling an overlay must raise fieldNeedsUpdate (the preempt signal)').toBe(true);

        // (ii) The preempt must be processed: within a couple frames the dirty is
        // consumed (the scheduler opened a fresh sweep against current flags rather
        // than continuing to drain the stale one). If the preempt path were dead,
        // fieldNeedsUpdate would linger (the stale sweep would finish first).
        await expect.poll(
            () => page.evaluate(async () => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                return !!getScale0State().fieldNeedsUpdate;
            }),
            { timeout: 8_000, message: 'fieldNeedsUpdate never consumed after a mid-sweep toggle — the preempt path is dead (stale-partial regression)' },
        ).toBe(false);

        // Deeper preempt assertion: once the dirty is consumed, the scheduler's
        // snapshot is the FRESH one (re-sampled at preempt), not the stranded
        // pre-toggle snapshot. We can't read snapshot identity, but we CAN assert
        // the sweep state is coherent: at any observed moment after the consume,
        // either a sweep is cleanly idle (active=false, sampled=null) or an
        // in-flight sweep has a non-null snapshot with cursor within bounds — i.e.
        // no "active sweep with a null/stale snapshot" stranded state exists.
        const coherent = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            // Sample a handful of consecutive animate frames and check the
            // active⇒snapshot invariant on each.
            return await new Promise((resolve) => {
                let bad = null;
                let frames = 0;
                const tick = () => {
                    const s = getScale0State().overlaySched;
                    if (s) {
                        // INVARIANT: an active sweep must own a snapshot and a
                        // cursor that has not run past its job count (a stranded
                        // partial would violate one of these).
                        if (s.active && (s.sampled == null || s.cursor > s.jobCount)) {
                            bad = { active: s.active, hasSampled: s.sampled != null, cursor: s.cursor, jobCount: s.jobCount };
                        }
                    }
                    if (bad || ++frames >= 12) { resolve({ bad, frames }); return; }
                    requestAnimationFrame(tick);
                };
                requestAnimationFrame(tick);
            });
        });
        expect(coherent.bad, `stranded sweep state observed (active sweep without a valid snapshot/cursor): ${JSON.stringify(coherent.bad)}`).toBeNull();

        // (iii) The newly toggled flag must survive the preempt.
        const flagStillOn = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            return !!getScale0State().fieldFlags.showDivField;
        });
        expect(flagStillOn, 'showDivField must stay enabled after the preempt (the new flag is honored)').toBe(true);

        // Leave the page benign for serial runs.
        await page.evaluate(() => { window.__ftdCtx.running = false; });

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });

    // ────────────────────────────────────────────────────────────────────
    // 3. WORK-BUDGET TIME-SLICING.
    //
    // The scheduler caps work per frame at OVERLAY_FRAME_BUDGET=100. A single
    // streamline job costs COST_STREAMLINE=100 — the ENTIRE budget — so the
    // budget gate (field-overlays.js:1002) admits exactly ONE streamline per
    // frame and defers the rest of the sweep. Therefore a sweep that contains
    // ≥2 streamline jobs (E + B + flux + force-flows) CANNOT finish in one
    // frame; the build SPREADS across frames.
    //
    // We assert the observable consequence directly off the scheduler's own
    // counters, collected over a short rAF-driven window on the page (so we
    // sample many sweeps without racing a single read):
    //   (a) DIRECT — over the window, at least one sweep took ≥2 frames to
    //       complete (max observed sched.sweepFrames ≥ 2) AND/OR a mid-sweep
    //       partial was caught (0 < cursor < jobCount on some frame). A
    //       scheduler that builds everything in one frame would show
    //       sweepFrames pinned at 1 and cursor only ever 0 or jobCount — that
    //       is the regression this guards, and we fail loudly on it.
    //   (b) BACKSTOP (timing, perf-baseline idiom) — no single animate frame
    //       blocks for hundreds of ms while the heavy overlay builds; the
    //       per-frame gap stays bounded even as streamlines come online.
    // ────────────────────────────────────────────────────────────────────
    test('work-budget: a heavy streamline overlay set spreads its build across frames (≤1 streamline/frame)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForScale0Ctx(page);

        // Particle-rich scenario + a streamline-heavy overlay set so the sweep
        // carries ≥2 COST_STREAMLINE jobs (E, B, flux all anchor to the quarks).
        // Run the sim so the version advances and sweeps re-open every throttle.
        await page.evaluate(async () => {
            const { getScale0State, resetFieldFlags, setFieldToggle } = await import('./js/scales/scale0/state/store.js');
            const b = window._ftdBridge;
            if (typeof b?.setupScenario === 'function') {
                b.setupScenario('s0-vacuum-proton');
                for (let i = 0; i < 8; i++) b.tick();
            }
            resetFieldFlags();
            // 3 streamline fields + a stack of scalar sheets = a fat sweep.
            for (const k of ['showEField', 'showBField', 'showFluxLines',
                'showPsiSquared', 'showPhase', 'showLagrangianDensity',
                'showEmEnergy', 'showChargeDensity', 'showVorticity',
                'showHelicity', 'showKretschmann']) {
                setFieldToggle(k, true);
            }
            window.__ftdCtx.running = true;
        });

        // PRECONDITION for the time-slice invariant: the sweep must actually
        // carry ≥2 streamline-cost jobs (each COST_STREAMLINE=50, half the
        // OVERLAY_FRAME_BUDGET=100 — lowered from 100→50 in the 2026-05-31
        // web-optimization campaign so E and B both fit one frame). A streamline
        // job is only emitted when its field sample is non-empty, so if this
        // scenario yielded count=0 for E/B/flux the sweep would be scalar-only
        // and drain in one frame — and a time-slicing assertion would then
        // FALSE-FAIL on a correct scheduler. We therefore poll the LIVE job pool
        // and require ≥2 streamline jobs before asserting spread; if they never
        // appear we fail with a scenario-setup message, not a time-slicing one.
        // (We count by cost===COST_STREAMLINE across the live jobCount slots —
        // kind-agnostic, so force-flow streamline jobs count too.)
        await expect.poll(
            () => page.evaluate(async () => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                const s = getScale0State().overlaySched;
                if (!s || !s.jobs) return 0;
                let streamlineJobs = 0;
                for (let i = 0; i < s.jobCount; i++) {
                    if (s.jobs[i] && s.jobs[i].cost === 50) streamlineJobs++;  // COST_STREAMLINE (was 100 pre-2026-05-31)
                }
                return streamlineJobs;
            }),
            {
                timeout: 12_000,
                message: 'heavy overlay set never produced ≥2 streamline jobs (E/B/flux field samples were empty) — scenario setup did not establish the time-slice precondition',
            },
        ).toBeGreaterThanOrEqual(2);

        // (a) DIRECT observation. Drive a window of rAF frames on the page and
        // record, per frame, the scheduler's (cursor, jobCount, sweepFrames).
        // We compute:
        //   - maxSweepFrames : the largest sweepFrames any sweep reached (a sweep
        //     that completes in one frame can never exceed 1);
        //   - sawPartial     : whether we ever caught 0 < cursor < jobCount, i.e.
        //     a sweep mid-drain (some jobs done, some deferred to a later frame);
        //   - maxJobCount    : the fattest sweep seen (context for the report).
        const slice = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            return await new Promise((resolve) => {
                let maxSweepFrames = 0;
                let maxJobCount = 0;
                let maxStreamlineJobs = 0;
                let sawPartial = false;
                let frames = 0;
                const FRAMES = 90; // ~1.5s at 60Hz — covers many throttle boundaries
                const step = () => {
                    const s = getScale0State().overlaySched;
                    if (s) {
                        if (s.jobCount > maxJobCount) maxJobCount = s.jobCount;
                        if (s.sweepFrames > maxSweepFrames) maxSweepFrames = s.sweepFrames;
                        // Count streamline-cost jobs in the live pool (proves the
                        // ≥2-streamline precondition held during observation).
                        if (s.jobs) {
                            let sl = 0;
                            for (let i = 0; i < s.jobCount; i++) {
                                if (s.jobs[i] && s.jobs[i].cost === 50) sl++;  // COST_STREAMLINE (was 100 pre-2026-05-31)
                            }
                            if (sl > maxStreamlineJobs) maxStreamlineJobs = sl;
                        }
                        // A genuine mid-drain partial: the sweep is active, some
                        // jobs have run, but not all — only possible if the budget
                        // deferred jobs to a later frame.
                        if (s.active && s.cursor > 0 && s.cursor < s.jobCount) sawPartial = true;
                    }
                    if (++frames >= FRAMES) { resolve({ maxSweepFrames, maxJobCount, maxStreamlineJobs, sawPartial }); return; }
                    requestAnimationFrame(step);
                };
                requestAnimationFrame(step);
            });
        });

        // The observation window must itself have seen the ≥2-streamline sweep
        // (the precondition that MAKES time-slicing observable). If it didn't,
        // the spread check below is moot — fail here with the precondition.
        expect(
            slice.maxStreamlineJobs,
            `observation window never saw ≥2 streamline jobs (maxStreamlineJobs=${slice.maxStreamlineJobs}, maxJobCount=${slice.maxJobCount}) — time-slice precondition not held`,
        ).toBeGreaterThanOrEqual(2);

        // THE invariant: with ≥2 streamline jobs and a budget that fits exactly
        // one streamline per frame, the build MUST spread — either a sweep
        // visibly took ≥2 frames, or we caught it mid-drain. If BOTH are false
        // the scheduler built everything in a single frame (no time-slicing) —
        // a real regression of the perf work. Report the evidence loudly.
        const spread = slice.maxSweepFrames >= 2 || slice.sawPartial;
        expect(
            spread,
            `overlay build did NOT spread across frames — TIME-SLICING REGRESSION. ` +
            `maxSweepFrames=${slice.maxSweepFrames} (expected ≥2 for a multi-streamline sweep), ` +
            `sawPartial(0<cursor<jobCount)=${slice.sawPartial}, ` +
            `maxStreamlineJobs=${slice.maxStreamlineJobs}, maxJobCount=${slice.maxJobCount}. ` +
            `With COST_STREAMLINE===OVERLAY_FRAME_BUDGET a sweep of ≥2 streamlines cannot finish in one frame.`,
        ).toBe(true);

        // (b) TIMING BACKSTOP (perf-baseline.spec.js idiom): sample rAF gaps over
        // a window with the heavy overlay live and assert no single frame stalled
        // for hundreds of ms. Time-slicing's whole point is to keep the worst
        // frame bounded; a monolithic build would spike one frame to completion.
        const timing = await page.evaluate(async () => {
            return await new Promise((resolve) => {
                const deltas = [];
                let last = 0;
                let frames = 0;
                const FRAMES = 90;
                const tick = () => {
                    const now = performance.now();
                    if (last > 0) deltas.push(now - last);
                    last = now;
                    if (++frames >= FRAMES) {
                        deltas.sort((a, b) => a - b);
                        const n = deltas.length;
                        resolve({
                            n,
                            median: n ? deltas[Math.floor(n / 2)] : 0,
                            max: n ? deltas[n - 1] : 0,
                        });
                        return;
                    }
                    requestAnimationFrame(tick);
                };
                requestAnimationFrame(tick);
            });
        });
        // 300ms is deliberately generous (headless GPU/CI noise); a monolithic
        // all-streamline build was observed at 160–215ms PER overlay frame and
        // would stack well past this with multiple streamlines on. Time-slicing
        // keeps the worst frame far under it.
        expect(timing.n, 'should have sampled a meaningful number of frames').toBeGreaterThan(20);
        expect(
            timing.max,
            `worst single-frame gap ${timing.max.toFixed(1)}ms (median ${timing.median.toFixed(1)}ms) exceeded 300ms — a heavy overlay frame stalled, suggesting the build is NOT time-sliced`,
        ).toBeLessThan(300);

        await page.evaluate(() => { window.__ftdCtx.running = false; });

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });
});
