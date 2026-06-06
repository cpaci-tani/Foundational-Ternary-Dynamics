// @ts-check
/**
 * All-scenario health sweep — loads EVERY Scale-0 registry scenario in one
 * session and records, per scenario:
 *   - mounted:     did it seed a non-trivial lattice state (flux energy or
 *                  particles)? Known-empty scenarios are allowlisted.
 *   - telemetryOK: does the active bridge return finite diagnostics
 *                  (totalEnergy finite, tick numeric — the signal the panels
 *                  read), i.e. is telemetry actually wired for this scenario?
 *   - clean:       no real console / page errors during its load.
 *
 * This is the mechanical half of the 2026-06-05 all-scenario audit (the
 * physics-sense half lives in docs/audits/AUDIT_SCALE0_SCENARIO_HEALTH.md).
 * It runs each scenario on its NATURAL owner — flux- and s0- on the JS flux
 * mock, empty/light/quantum on the WASM engine — so it reflects what a
 * user actually gets. The full table is logged for the audit; the test fails
 * only if a scenario is genuinely broken (no mount AND not known-empty, or
 * NaN/absent telemetry, or a real console error).
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

// Scenarios that are INTENTIONALLY empty (baselines / negative controls) and
// therefore exempt from the "must mount something" check.
const KNOWN_EMPTY = new Set([
    'empty',
    's0-seed-emergent-ic4-subthreshold',     // FTD-0107 sub-threshold negative control (0 voxels)
    's0-seed-emergent-ic2-thermal-runaway',  // Langevin-driven runaway — empty at load, develops over time
]);

async function waitForCtx(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
        { timeout: 20_000, message: 'window.__ftdCtx.bridge never became available' },
    ).toBe(true);
}

test.describe('Scale-0 all-scenario health sweep', () => {
    test('every registry scenario mounts + has working telemetry', async ({ page }) => {
        test.setTimeout(360_000); // ~97 scenarios × load+settle+read
        const consoleErrors = attachConsoleWatcher(page);

        await gotoAndReady(page);
        await waitForCtx(page);

        // Pull the authoritative scenario list straight from the registry.
        const scenarios = await page.evaluate(async () => {
            const m = await import('/js/scales/scale0/scenario-registry.js');
            return m.SCALE0_SCENARIOS.map((s) => ({ id: s.id, category: s.category }));
        });
        expect(scenarios.length, 'registry should expose scenarios').toBeGreaterThan(80);

        // Start running so worker-backed mocks self-tick and post diagnostics.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        /** @type {Array<{id:string,category:string,owner:string,tick:any,totalEnergy:any,manifested:any,particles:any,mounted:boolean,telemetryOK:boolean,errors:string[]}>} */
        const rows = [];

        for (const { id, category } of scenarios) {
            const before = consoleErrors.length;

            await page.evaluate((scenarioId) => {
                const sel = document.getElementById('scenario-select');
                if (![...sel.options].some((o) => o.value === scenarioId)) {
                    sel.add(new Option(scenarioId, scenarioId));
                }
                sel.value = scenarioId;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }, id);
            await page.waitForTimeout(650); // sync load + a few worker frames

            const snap = await page.evaluate(async () => {
                const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                const st = getScale0State();
                const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
                const caps = bridge.capabilities.scale0;
                const d = caps.getScale0Diagnostics?.() || {};
                let particles = 0;
                try { particles = (caps.getScale0ParticleFrame?.() || {}).count ?? 0; } catch { particles = 0; }
                // Real "is there a field?" signal — the constant WASM totalEnergy
                // baseline is NOT trustworthy for mount detection, so read the
                // actual flux volume and take its peak magnitude.
                let maxFlux = 0;
                try {
                    const fv = bridge.getFluxVolume?.();
                    if (fv && fv.length) for (let i = 0; i < fv.length; i++) { const a = Math.abs(fv[i]); if (a > maxFlux) maxFlux = a; }
                } catch { maxFlux = -1; }
                // Some scenarios seed an E or B field rather than the J flux
                // volume (e.g. s0-field-uniform-e/-uniform-b) — count those too.
                let fieldSamples = 0;
                try {
                    for (const kind of ['e', 'b']) {
                        const s = caps.getScale0FieldSamples?.({ kind, stride: 3 });
                        if (s && s.count) fieldSamples += s.count;
                    }
                } catch { /* ignore */ }
                return {
                    owner: st.useFluxMock ? 'mock' : 'wasm',
                    tick: d.tick ?? null, totalEnergy: d.totalEnergy ?? null,
                    manifested: d.manifested ?? null, particles, maxFlux, fieldSamples,
                };
            });

            const newErrors = realErrors(consoleErrors.slice(before));
            const telemetryOK = Number.isFinite(snap.totalEnergy) && typeof snap.tick === 'number';
            // Mounted = it actually seeded a field or particles (NOT the constant
            // energy baseline).
            const mounted =
                (Number.isFinite(snap.maxFlux) && snap.maxFlux > 1e-6) ||
                (typeof snap.manifested === 'number' && snap.manifested > 0) ||
                (snap.particles > 0) ||
                (snap.fieldSamples > 0);

            rows.push({ id, category, ...snap, mounted, telemetryOK, errors: newErrors });
        }

        // Emit the full table for the audit.
        const fmt = (r) => `${r.mounted ? 'M' : '·'}${r.telemetryOK ? 'T' : '·'}${r.errors.length ? 'E' : '·'} ` +
            `${r.owner.padEnd(4)} ${String(r.id).padEnd(34)} f=${Number(r.maxFlux).toFixed(3)} p=${r.particles} m=${r.manifested} E=${r.totalEnergy}` +
            (r.errors.length ? `  ERR:${r.errors[0]}` : '');
        console.log('\n=== Scale-0 scenario health (M=mounted T=telemetry E=errors) ===\n' +
            rows.map(fmt).join('\n') + `\n\nTotal: ${rows.length} scenarios`);

        // Failures = genuinely unhealthy scenarios.
        const notMounted = rows.filter((r) => !r.mounted && !KNOWN_EMPTY.has(r.id));
        const badTelemetry = rows.filter((r) => !r.telemetryOK);
        const withErrors = rows.filter((r) => r.errors.length > 0);

        expect(badTelemetry.map((r) => r.id), 'scenarios with absent/NaN telemetry').toEqual([]);
        expect(withErrors.map((r) => `${r.id}: ${r.errors[0]}`), 'scenarios that logged a real console error').toEqual([]);
        expect(notMounted.map((r) => r.id), 'scenarios that mounted nothing (and are not known-empty)').toEqual([]);
    });

    // Classify the flux-annihilation failure seen in the sweep: is it the
    // scenario, or a worker create/dispose race only triggered by rapid
    // switching? Load it FIRST on a fresh page (no prior worker to race).
    test('flux-annihilation mounts cleanly in isolation (race vs scenario bug)', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtx(page);
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 'flux-annihilation';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await page.waitForTimeout(1200);
        const snap = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            const caps = (st.useFluxMock && st.fluxMock) ? st.fluxMock.capabilities.scale0 : window.__ftdCtx.bridge.capabilities.scale0;
            const d = caps.getScale0Diagnostics?.() || {};
            return { manifested: d.manifested ?? 0, particles: (caps.getScale0ParticleFrame?.() || {}).count ?? 0 };
        });
        const errs = realErrors(consoleErrors);
        console.log(`[flux-annihilation isolation] manifested=${snap.manifested} particles=${snap.particles} errors=${JSON.stringify(errs)}`);
        expect(errs, 'flux-annihilation should not log a worker error in isolation').toEqual([]);
        expect(snap.manifested + snap.particles, 'flux-annihilation should mount its 4 particles').toBeGreaterThan(0);
    });

    // s0-vacuum-* runs on the WASM engine by default, so the all-scenario sweep
    // (above) exercises the C++ path. This guards the **MockBridge fallback**
    // path (Safari / no-COOP-COEP), where setupVacuumScenario used to throw a
    // `harness`/`this` ReferenceError (health audit §A.4). Force ?engine=mock so
    // vacuum scenarios run on the JS MockBridge.
    test('s0-vacuum-* mounts on the MockBridge fallback path (no harness ReferenceError)', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=mock' });
        await waitForCtx(page);

        const sample = ['s0-vacuum-electron', 's0-vacuum-proton', 's0-vacuum-photon', 's0-vacuum-higgs'];
        /** @type {Record<string, number>} */
        const mounted = {};
        for (const id of sample) {
            await page.evaluate((scenarioId) => {
                const sel = document.getElementById('scenario-select');
                if (![...sel.options].some((o) => o.value === scenarioId)) sel.add(new Option(scenarioId, scenarioId));
                sel.value = scenarioId;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }, id);
            await page.waitForTimeout(550);
            mounted[id] = await page.evaluate(async () => {
                const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                const st = getScale0State();
                const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
                const caps = bridge.capabilities.scale0;
                const p = (caps.getScale0ParticleFrame?.() || {}).count ?? 0;
                let maxFlux = 0;
                try { const fv = bridge.getFluxVolume?.(); if (fv) for (let i = 0; i < fv.length; i++) { const a = Math.abs(fv[i]); if (a > maxFlux) maxFlux = a; } } catch { /* ignore */ }
                return p + (maxFlux > 1e-6 ? 1 : 0);
            });
        }
        const errs = realErrors(consoleErrors);
        console.log(`[vacuum mock-path] mounted=${JSON.stringify(mounted)} errors=${JSON.stringify(errs)}`);
        expect(errs, 's0-vacuum-* must not throw on the MockBridge path').toEqual([]);
        for (const id of sample) expect(mounted[id], `${id} should mount on the mock path`).toBeGreaterThan(0);
    });
});
