// @ts-check
/**
 * Proper time τ / lapse dτ/dt / de Broglie phase φ overlay + telemetry spec
 * (2026-09-03).
 *
 * These three overlays are WASM-only: they read Voxel::tau / Voxel::phase and
 * a live causal_kinematics.h proper_time_rate() evaluation, none of which the
 * JS-side field-sampler dispatch can synthesize on a bridge that lacks the
 * native sampler. Three new C++ functions were added to expose them —
 * get_tau_sampled / get_phase_sampled / get_lapse_sampled (ftd_wasm.cpp,
 * bound in bindings_render_bridge.cpp) — but per the task constraint on this
 * change, the WASM binary was NOT rebuilt here. Until a rebuild runs,
 * `bridge._module.getTauSampled` etc. do not exist, so every bridge — WASM or
 * not — falls back to the shared EMPTY_SCALAR_SAMPLE (count 0), exactly the
 * graceful "sampler absent" path bridge-contract.js's samplerOr() already
 * guarantees for every optional Scale-0 sampler kind.
 *
 * Test design therefore splits into:
 *   1. Wiring checks that hold NOW (registry entries, toggle plumbing, and the
 *      "graceful no-op" contract) — these do not depend on the native sampler
 *      being bound and must pass today.
 *   2. The "live overlay" behavioural test (nonzero sampled count on a
 *      WASM-owned, particle-manifesting scenario with latency_field and
 *      de_broglie_clock enabled) — gated behind a runtime capability probe
 *      (`bridge._module.getTauSampled` etc). It self-skips with a clear
 *      reason pre-rebuild and will start actually asserting once the WASM
 *      module is rebuilt with the three new bindings — no spec edit required.
 *
 * See engine/wasm/ftd_wasm.cpp (get_tau_sampled/get_phase_sampled/
 * get_lapse_sampled), engine/web/js/bridge/bridge-contract.js
 * (SCALE0_SAMPLER_METHODS: tau/dbPhase/lapse), and
 * engine/web/js/scales/scale0/ui/overlays/time-panel.js (Card F).
 */

import { test, expect } from '@playwright/test';
import {
    gotoAndReady,
    selectScale0Scenario,
    attachConsoleWatcher,
    realErrors,
} from './_helpers.js';
import { SCALE0_SAMPLER_METHODS } from '../js/bridge/bridge-contract.js';

// ── 1. Static registry wiring — no browser needed ──────────────────────────
test.describe('proper-time/lapse/dbPhase sampler-kind registry', () => {
    test('SCALE0_SAMPLER_METHODS maps tau/dbPhase/lapse to the new native functions', () => {
        expect(SCALE0_SAMPLER_METHODS.tau).toBe('getTauSampled');
        expect(SCALE0_SAMPLER_METHODS.dbPhase).toBe('getPhaseSampled');
        expect(SCALE0_SAMPLER_METHODS.lapse).toBe('getLapseSampled');
    });
});

// ── 2. Live-page wiring — toggles, buttons, telemetry registry ─────────────
test.describe('Scale-0 proper-time overlay wiring', () => {
    test('showProperTime/showLapse/showDBPhase are real fieldFlags keys with rendered buttons', async ({ page }) => {
        await gotoAndReady(page);
        const inv = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const dom = await import('/js/scales/scale0/ui/dom.js');
            const keys = Object.keys(store.getScale0State().fieldFlags);
            const bindings = new Map(dom.FIELD_TOGGLE_BINDINGS);
            const targets = ['showProperTime', 'showLapse', 'showDBPhase'];
            return targets.map((key) => {
                const buttonId = [...bindings.entries()].find(([, k]) => k === key)?.[0] || null;
                return {
                    key,
                    inKeys: keys.includes(key),
                    buttonId,
                    buttonExists: buttonId ? !!document.getElementById(buttonId) : false,
                };
            });
        });
        for (const entry of inv) {
            expect(entry.inKeys, `${entry.key} should be a fieldFlags key`).toBe(true);
            expect(entry.buttonId, `${entry.key} should have a FIELD_TOGGLE_BINDINGS button id`).toBeTruthy();
            expect(entry.buttonExists, `${entry.key} button #${entry.buttonId} should be rendered`).toBe(true);
        }
    });

    test('ptime.* telemetry-grid channels are registered', async ({ page }) => {
        await gotoAndReady(page);
        const keys = await page.evaluate(async () => {
            const { SCALE0_GRID_CHANNELS } = await import('/js/telemetry/registry/scale0-grid-channels.js');
            return SCALE0_GRID_CHANNELS.filter((c) => c.buffer.startsWith('ptime.')).map((c) => c.key);
        });
        expect(keys).toEqual(expect.arrayContaining([
            'properTimeMean', 'properTimeSpread', 'lapseMean', 'dbPhaseMean', 'dbPhaseCircVar',
        ]));
    });

    test('telemetryHub exposes a ptime ring-buffer group fed by publishScale0ProperTimeMetrics', async ({ page }) => {
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            const before = telemetryHub.ptime.properTimeMean.count;
            telemetryHub.publishScale0ProperTimeMetrics({
                properTimeMean: 1.5, properTimeMin: 0.5, properTimeMax: 2.5,
                lapseMean: 0.9, dbPhaseMean: 1.2, dbPhaseCircVar: 0.05,
            }, 999_000_001);
            return {
                before,
                afterCount: telemetryHub.ptime.properTimeMean.count,
                lastMean: telemetryHub.ptime.properTimeMean.last(),
                lastSpread: telemetryHub.ptime.properTimeSpread.last(),
                lastLapse: telemetryHub.ptime.lapseMean.last(),
                lastPhase: telemetryHub.ptime.dbPhaseMean.last(),
                lastCircVar: telemetryHub.ptime.dbPhaseCircVar.last(),
            };
        });
        expect(result.afterCount).toBeGreaterThan(result.before);
        expect(result.lastMean).toBeCloseTo(1.5, 6);
        expect(result.lastSpread).toBeCloseTo(2.0, 6); // max(2.5) - min(0.5)
        expect(result.lastLapse).toBeCloseTo(0.9, 6);
        expect(result.lastPhase).toBeCloseTo(1.2, 6);
        expect(result.lastCircVar).toBeCloseTo(0.05, 6);
    });

    // Graceful no-op: the default scenario (flux-pulse) samples empty for the
    // three new WASM-only kinds — no throw, no console error, count 0. This
    // holds regardless of whether the active bridge is main-thread WASM, a
    // worker proxy, or (pre-rebuild) any bridge at all, because the native
    // functions are not yet bound anywhere — this is the SAME graceful path
    // every other optional sampler kind already takes when a bridge lacks it
    // (bridge-contract.js samplerOr()).
    test('graceful no-op: tau/lapse/dbPhase samplers are empty (not throwing) on the default scenario', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        const sampled = await page.evaluate(() => {
            const caps = window.__ftdCtx?.bridge?.capabilities?.scale0;
            if (!caps) return null;
            return ['tau', 'lapse', 'dbPhase'].map((kind) => {
                const s = caps.getScale0FieldSamples({ kind, stride: 1 });
                return { kind, count: s?.count ?? -1, hasPositions: !!s?.positions, hasValues: !!s?.values };
            });
        });
        expect(sampled, 'Scale-0 capability surface should be reachable').not.toBeNull();
        for (const s of sampled) {
            expect(s.count, `${s.kind} sampler should return count=0 (empty), not throw`).toBe(0);
            expect(s.hasPositions, `${s.kind} sampler should still return a positions array`).toBe(true);
            expect(s.hasValues, `${s.kind} sampler should still return a values array`).toBe(true);
        }

        const real = realErrors(errors);
        expect(real, `no console errors sampling absent overlay kinds:\n  ${real.join('\n  ')}`).toEqual([]);
    });
});

// ── 3. Live overlay behaviour — capability-gated, self-skipping pre-rebuild ─
test.describe('Scale-0 proper-time overlay — live sampler + telemetry (WASM-owned scenario)', () => {
    test('tau/lapse/dbPhase samplers return nonzero data and populate ptime telemetry once the native module is rebuilt', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);

        // Capability probe: is get_tau_sampled/get_phase_sampled/get_lapse_sampled
        // actually bound in the CURRENTLY DEPLOYED .wasm binary? This does not
        // depend on scenario choice — it is a static property of the compiled
        // module, checked via the module namespace the bridge wraps
        // (`bridge._module`, the same field _wasmCallOr in wasm-bridge.js gates
        // on). If any is missing, the WASM has not been rebuilt with this
        // change yet, and the live-data assertions below cannot pass — skip
        // with a clear, actionable reason rather than failing or fabricating
        // a pass.
        const capability = await page.evaluate(() => {
            const b = window._ftdBridge;
            const m = b?._module;
            return {
                reachable: !!m,
                tau: typeof m?.getTauSampled === 'function',
                phase: typeof m?.getPhaseSampled === 'function',
                lapse: typeof m?.getLapseSampled === 'function',
            };
        });
        test.skip(
            !capability.reachable || !capability.tau || !capability.phase || !capability.lapse,
            'Requires a WASM rebuild: get_tau_sampled/get_phase_sampled/get_lapse_sampled ' +
            '(engine/wasm/ftd_wasm.cpp, bound in bindings_render_bridge.cpp) are not yet compiled ' +
            `into the deployed module (capability probe: ${JSON.stringify(capability)}). ` +
            'Run engine/build_wasm.bat, redeploy engine/web/wasm/, and re-run this spec.',
        );

        // Force main-thread WASM (not the worker proxy) so `window._ftdBridge`
        // (== ctx.bridge, app.js) is definitely the instance actually running
        // the scenario, keeping the capability probe above and the sampler
        // reads below aimed at the same object.
        await page.evaluate(() => { if (window.__ftdCtx) window.__ftdCtx._wasmWorkerDisabled = true; });

        // The flag above only takes effect on the NEXT scenario load. Reload a
        // WASM-owned, non-self-seeding scenario so the ACTIVE bridge becomes
        // ctx.bridge (main thread): the Time panel's ptime.* telemetry
        // publisher samples the active bridge, and the injection below must
        // land on the same instance it reads.
        await selectScale0Scenario(page, 'empty');

        // Integration fix (2026-09-04): the original s0-seed-ee-annihilation
        // selection produced ZERO particles on the main-thread WASM instance
        // (scenario seeding lands on the active/worker bridge, not on
        // ctx.bridge). τ/φ only accumulate at manifested (state≠0) voxels, so
        // create particles the way every other WASM-owned spec does
        // (scale0-substrate-protocol-v2, scale0-sampler-lifetime): inject a
        // supercritical flux blob on ctx.bridge itself and let genesis fire.
        await page.evaluate(() => {
            const b = window.__ftdCtx?.bridge;
            if (!b?.injectFlux || !b?.tick) throw new Error('ctx.bridge lacks injectFlux/tick (proxy path?)');
            // The default (pedagogical flux) scenario's toggle defaults leave
            // genesis/movement OFF on this instance — measured via a probe on
            // 2026-09-04 — so no supercritical blob can manifest without this.
            b.setToggle('genesis', true);
            b.setToggle('movement', true);
            const N = b.latticeSize, mid = Math.floor(N / 2);
            b.injectFlux(mid, mid, mid, 50.0, 0.0, 0.0);
        });

        // Enable the two engine toggles through the REAL production checkbox
        // + 'change' event path (ui/controls/wire.js) — the same path
        // time-panel.js Card F's one-click "Enable" button uses.
        await page.evaluate(() => {
            for (const id of ['t-latency-field', 't-de-broglie']) {
                const cb = document.getElementById(id);
                if (cb && !cb.checked) {
                    cb.checked = true;
                    cb.dispatchEvent(new Event('change', { bubbles: true }));
                }
            }
            // Belt and braces: the checkbox path targets the ACTIVE bridge,
            // which may be the worker proxy; the samplers below read
            // ctx.bridge, so set the engine toggles on that instance too.
            const b = window.__ftdCtx?.bridge;
            if (b?.setToggle) { b.setToggle('latency_field', true); b.setToggle('de_broglie_clock', true); }
        });

        // Also enable the three overlay flags (proves the store/dom/viewport
        // wiring end to end, not just the underlying sampler).
        await page.evaluate(async () => {
            const { setFieldToggle } = await import('/js/scales/scale0/state/store.js');
            setFieldToggle('showProperTime', true);
            setFieldToggle('showLapse', true);
            setFieldToggle('showDBPhase', true);
        });

        // Tick enough times for τ to accumulate measurably and for manifested
        // particles to exist (pair production / seeded state needs a few ticks
        // to settle).
        await page.evaluate(async () => {
            const owner = window.__ftdCtx?.bridge;
            for (let i = 0; i < 30; i++) owner?.capabilities?.scale0?.tickScale0?.();
        });

        const sample = await page.evaluate(() => {
            const caps = window.__ftdCtx?.bridge?.capabilities?.scale0;
            const tau = caps.getScale0FieldSamples({ kind: 'tau', stride: 1 });
            const lapse = caps.getScale0FieldSamples({ kind: 'lapse', stride: 1 });
            const phase = caps.getScale0FieldSamples({ kind: 'dbPhase', stride: 1 });
            const particles = caps.getScale0ParticleFrame?.();
            return {
                particleCount: particles?.count ?? -1,
                tauCount: tau.count, tauMax: tau.count ? Math.max(...tau.values.slice(0, tau.count)) : 0,
                lapseCount: lapse.count,
                phaseCount: phase.count,
            };
        });

        expect(sample.particleCount, 'a supercritical injected flux blob should manifest particles via genesis').toBeGreaterThan(0);
        expect(sample.tauCount, 'τ sampler should return a nonzero count once manifested voxels exist').toBeGreaterThan(0);
        expect(sample.tauMax, 'accumulated τ should be positive after ticking with latency_field/de_broglie_clock ON').toBeGreaterThan(0);
        expect(sample.lapseCount, 'lapse sampler should return a nonzero count').toBeGreaterThan(0);
        expect(sample.phaseCount, 'de Broglie phase sampler should return a nonzero count').toBeGreaterThan(0);

        // Open the Time Observatory panel so Card F's update() loop runs and
        // publishes into telemetryHub.ptime (time-panel.js is gated on
        // isPanelLive — the panel must be shown for its rAF loop to sample).
        await page.evaluate(() => {
            document.querySelector('#tab-bar .tab[data-panel="time"]')?.click();
        });

        await expect.poll(async () => page.evaluate(async () => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            return telemetryHub.ptime.properTimeMean.count;
        }), {
            timeout: 15_000,
            message: 'ptime.properTimeMean telemetry channel should populate once the Time panel is live',
        }).toBeGreaterThan(0);

        const telemetry = await page.evaluate(async () => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            return {
                mean: telemetryHub.ptime.properTimeMean.last(),
                lapseMean: telemetryHub.ptime.lapseMean.last(),
            };
        });
        expect(telemetry.mean, 'published τ mean should be finite and positive').toBeGreaterThan(0);
        expect(Number.isFinite(telemetry.lapseMean), 'published lapse mean should be finite').toBe(true);
    });
});
