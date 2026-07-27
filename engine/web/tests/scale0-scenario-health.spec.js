// @ts-check
/**
 * Scale-0 admission and catalog-health suite. It separately verifies the
 * evidence-gated production menu and mechanically loads the complete catalog,
 * recording per scenario:
 *   - mounted:     did it seed a non-trivial lattice state (flux energy or
 *                  particles)? Known-empty scenarios are allowlisted.
 *   - telemetryOK: does the active bridge return finite diagnostics
 *                  (totalEnergy finite, tick numeric — the signal the panels
 *                  read), i.e. is telemetry actually wired for this scenario?
 *   - clean:       no real console / page errors during its load.
 *
 * This is the mechanical half of the 2026-06-05 all-scenario audit (the
 * physics-sense half lives in docs/audits/AUDIT_SCALE0_SCENARIO_HEALTH.md).
 * It runs each scenario through the dashboard's selected bridge owner, so it
 * reflects the real loader/worker path. The full table is logged for the audit;
 * the test fails only if a scenario is genuinely broken (no mount AND not
 * known-empty, or NaN/absent telemetry, or a real console error).
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

// Scenarios that are INTENTIONALLY empty (baselines / negative controls) and
// therefore exempt from the "must mount something" check.
const KNOWN_EMPTY = new Set([
    'empty',
    's0-seed-emergent-ic4-subthreshold',     // FTD-0107 sub-threshold negative control (0 voxels)
    's0-seed-emergent-ic2-thermal-runaway',  // Langevin-driven runaway — empty at load, develops over time
    's0-seed-thermal-ignition',               // interactive subcritical bath — panel raises T across T_up
]);

async function waitForCtx(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
        { timeout: 20_000, message: 'window.__ftdCtx.bridge never became available' },
    ).toBe(true);
}

async function waitForActiveTelemetry(page, timeoutMs = 8_000) {
    const started = Date.now();
    while (Date.now() - started < timeoutMs) {
        const ready = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
            const d = bridge?.capabilities?.scale0?.getScale0Diagnostics?.() || {};
            return Number.isFinite(d.totalEnergy) && typeof d.tick === 'number';
        });
        if (ready) return Date.now() - started;
        await page.waitForTimeout(100);
    }
    return Date.now() - started;
}

test.describe('Scale-0 scenario admission and catalog health', () => {
    test('dropdown contains only evidence-gated scenarios', async ({ page }) => {
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const registry = await import('/js/scales/scale0/scenario-registry.js');
            const visible = [...document.querySelectorAll('#scenario-select option')]
                .map((option) => option.value)
                .filter(Boolean)
                .sort();
            return {
                visible,
                expected: registry.SCALE0_SCENARIOS.map((s) => s.id).sort(),
                validationCount: Object.keys(registry.SCALE0_SCENARIO_VALIDATION).length,
                catalogCount: registry.SCALE0_SCENARIO_CATALOG.length,
            };
        });
        expect(result.visible).toEqual(result.expected);
        expect(result.visible).toHaveLength(result.validationCount);
        expect(result.catalogCount).toBe(result.visible.length);
    });

    test('complete admitted catalog remains mechanically healthy', async ({ page }) => {
        test.setTimeout(420_000); // 115 scenarios × load+first-frame+read
        const consoleErrors = attachConsoleWatcher(page);

        await gotoAndReady(page);
        await waitForCtx(page);

        // Pull the complete internal catalog. This remains a mechanical
        // mount/telemetry audit separate from each entry's behavioral test.
        const scenarios = await page.evaluate(async () => {
            const m = await import('/js/scales/scale0/scenario-registry.js');
            return m.SCALE0_SCENARIO_CATALOG.map((s) => ({ id: s.id, category: s.category }));
        });
        expect(scenarios.length, 'research catalog should retain the full scenario inventory')
            .toBeGreaterThan(100);

        // Start running so worker-backed mocks self-tick and post diagnostics.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        /** @type {Array<{id:string,category:string,owner:string,tick:any,totalEnergy:any,manifested:any,particles:any,readyMs:number,mounted:boolean,telemetryOK:boolean,errors:string[]}>} */
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
            // A fixed sleep produced a false failure for quantum-tunnel: its
            // three full locked sheets take longer to construct than most
            // scenarios. Wait on the real readiness contract instead.
            const readyMs = await waitForActiveTelemetry(page);

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
                    owner: st.useFluxMock ? 'wasm-worker' : 'wasm',
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

            rows.push({ id, category, ...snap, readyMs, mounted, telemetryOK, errors: newErrors });
        }

        // Emit the full table for the audit.
        const fmt = (r) => `${r.mounted ? 'M' : '·'}${r.telemetryOK ? 'T' : '·'}${r.errors.length ? 'E' : '·'} ` +
            `${r.owner.padEnd(4)} ${String(r.id).padEnd(34)} ready=${String(r.readyMs).padStart(4)}ms f=${Number(r.maxFlux).toFixed(3)} p=${r.particles} m=${r.manifested} E=${r.totalEnergy}` +
            (r.errors.length ? `  ERR:${r.errors[0]}` : '');
        console.log('\n=== Scale-0 research-catalog mechanical health (NOT physics validation) ===\n' +
            rows.map(fmt).join('\n') + `\n\nTotal: ${rows.length} scenarios`);

        // Failures = genuinely unhealthy scenarios.
        const notMounted = rows.filter((r) => !r.mounted && !KNOWN_EMPTY.has(r.id));
        const badTelemetry = rows.filter((r) => !r.telemetryOK);
        const withErrors = rows.filter((r) => r.errors.length > 0);

        expect(badTelemetry.map((r) => r.id), 'scenarios with absent/NaN telemetry').toEqual([]);
        expect(withErrors.map((r) => `${r.id}: ${r.errors[0]}`), 'scenarios that logged a real console error').toEqual([]);
        expect(notMounted.map((r) => r.id), 'scenarios that mounted nothing (and are not known-empty)').toEqual([]);
    });

    // Permanent regression for the historical worker create/dispose failure.
    // The qualified scenario now intentionally removes both states on tick two,
    // so manifested count is not a valid post-run mount assertion.
    test('flux-annihilation mounts cleanly in isolation', async ({ page }) => {
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
        expect(Number.isFinite(snap.manifested + snap.particles), 'collision telemetry should remain finite').toBe(true);
    });

    test('quantum-tunnel reaches a finite mounted worker frame in isolation', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtx(page);
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 'quantum-tunnel';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        const readyMs = await waitForActiveTelemetry(page);
        const snap = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
            const caps = bridge.capabilities.scale0;
            const d = caps.getScale0Diagnostics?.() || {};
            return {
                tick: d.tick ?? null,
                totalEnergy: d.totalEnergy ?? null,
                manifested: d.manifested ?? null,
                particles: (caps.getScale0ParticleFrame?.() || {}).count ?? 0,
            };
        });
        const errs = realErrors(consoleErrors);
        console.log(`[quantum-tunnel isolation] ready=${readyMs}ms tick=${snap.tick} E=${snap.totalEnergy} manifested=${snap.manifested} particles=${snap.particles}`);
        expect(errs, 'quantum-tunnel should not log a worker error in isolation').toEqual([]);
        expect(Number.isFinite(snap.totalEnergy) && typeof snap.tick === 'number',
            'quantum-tunnel must publish finite diagnostics').toBe(true);
        expect(Math.max(snap.manifested ?? 0, snap.particles),
            'the three locked source sheets must be mounted').toBeGreaterThan(0);
    });

});
