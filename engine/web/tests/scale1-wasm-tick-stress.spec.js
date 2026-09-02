// @ts-check
/**
 * Direct-WASM Scale-1 tick integrity probe.
 *
 * The production animation loop normally migrates tick batches to a worker.
 * This gate deliberately advances the main ParticleEngine synchronously so
 * invalid indirect calls, heap corruption, and checkpoint-independent failures
 * surface at the exact native tick that caused them.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

async function openScale1(page) {
    const target = process.env.FTD_LIVE_URL || '/?engine=wasm';
    await gotoAndReady(page, { path: target, timeout: 90_000 });
    await page.selectOption('#engine-mode', 'particles');
    await expect.poll(
        () => page.locator('#pe-scenario-select option').count(),
        { timeout: 30_000 }
    ).toBe(36);
}

test('64-particle all-physics state survives sustained direct WASM ticks', async ({ page }) => {
    test.setTimeout(120_000);
    const consoleErrors = attachConsoleWatcher(page);
    await openScale1(page);
    const variant = await page.evaluate(() =>
        window.__ftdCtx?.bridge?.getWasmArtifactIdentity?.()?.variant?.id);
    expect(variant).toBe('wasm64');
    await page.selectOption('#pe-scenario-select', 's1-empty-zoo');

    const result = await page.evaluate(() => {
        const bridge = window.__ftdCtx?.bridge;
        if (!bridge) throw new Error('Scale 1 bridge unavailable');
        for (const spec of Array.from(bridge.peGetPhysicsRegistry()?.physics || [])) {
            if (spec.available && spec.toggle) bridge.peSetToggle(spec.toggle, true);
        }
        const side = 4;
        for (let i = 0; i < 64; ++i) {
            const x = (i % side) * 1.4 - 2.1;
            const y = (Math.floor(i / side) % side) * 1.4 - 2.1;
            const z = Math.floor(i / (side * side)) * 1.4 - 2.1;
            bridge.peAddParticle('electron', i % 2 ? -1 : 1,
                x, y, z, 0, 0, 0, 0.511, 0.08);
        }
        const startTick = Number(bridge.peGetTick());
        for (let i = 0; i < 2_000; ++i) {
            bridge.peTick();
            if ((i + 1) % 100 === 0) {
                const diagnostics = bridge.peGetDiagnostics();
                if (![diagnostics.totalEnergy, diagnostics.totalKE, diagnostics.totalPE]
                    .every(Number.isFinite)) {
                    throw new Error(`non-finite diagnostics after direct tick ${i + 1}`);
                }
            }
        }
        return {
            startTick,
            endTick: Number(bridge.peGetTick()),
            particleCount: Number(bridge.peParticleCount()),
        };
    });

    expect(result.endTick - result.startTick).toBe(2_000);
    expect(result.particleCount).toBeGreaterThanOrEqual(0);
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('every runnable scenario survives 1,000 direct ticks with all physics', async ({ page }) => {
    test.setTimeout(240_000);
    const consoleErrors = attachConsoleWatcher(page);
    await openScale1(page);
    const scenarios = await page.evaluate(() => Array.from(
        window.__ftdCtx?.bridge?.peGetPhysicsRegistry?.()?.scenarios || []
    ).map(row => ({ id: row.id, mode: row.mode })));

    for (const scenario of scenarios) {
        if (scenario.mode === 'native_matter') continue;
        await page.selectOption('#pe-scenario-select', scenario.id);
        await expect.poll(() => page.evaluate((id) =>
            window.__ftdCtx?.bridge?.peGetSnapshot?.(id)?.core?.scenario, scenario.id),
        { timeout: 15_000, message: `${scenario.id} did not settle` }).toBe(scenario.id);
        try {
            await page.evaluate(() => {
                const bridge = window.__ftdCtx.bridge;
                for (const spec of Array.from(bridge.peGetPhysicsRegistry()?.physics || [])) {
                    if (spec.available && spec.toggle) bridge.peSetToggle(spec.toggle, true);
                }
                for (let tick = 0; tick < 1_000; ++tick) bridge.peTick();
                const diagnostics = bridge.peGetDiagnostics();
                if (![diagnostics.totalEnergy, diagnostics.totalKE, diagnostics.totalPE]
                    .every(Number.isFinite)) {
                    throw new Error('non-finite diagnostics after scenario stress');
                }
            });
        } catch (error) {
            throw new Error(`${scenario.id} failed direct-WASM stress: ${error?.message || error}`);
        }
    }
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('worker checkpoint handoff preserves a live all-physics engine', async ({ page }) => {
    test.setTimeout(120_000);
    const consoleErrors = attachConsoleWatcher(page);
    await openScale1(page);
    await page.selectOption('#pe-scenario-select', 's1-empty-zoo');
    const start = await page.evaluate(() => {
        const bridge = window.__ftdCtx.bridge;
        for (const spec of Array.from(bridge.peGetPhysicsRegistry()?.physics || [])) {
            if (spec.available && spec.toggle) bridge.peSetToggle(spec.toggle, true);
        }
        for (let i = 0; i < 64; ++i) {
            const x = (i % 4) * 1.4 - 2.1;
            const y = (Math.floor(i / 4) % 4) * 1.4 - 2.1;
            const z = Math.floor(i / 16) * 1.4 - 2.1;
            bridge.peAddParticle('electron', i % 2 ? -1 : 1,
                x, y, z, 0, 0, 0, 0.511, 0.08);
        }
        document.getElementById('btn-play')?.click();
        return Number(bridge.peGetTick());
    });
    await page.waitForTimeout(20_000);
    const result = await page.evaluate(async () => {
        document.getElementById('btn-play')?.click();
        const bridge = window.__ftdCtx.bridge;
        const workerModule = await import(
            '/js/scales/scale1/particle-worker-executor.js?v=2'
        );
        const diagnostics = bridge.peGetDiagnostics();
        return {
            endTick: Number(bridge.peGetTick()),
            worker: workerModule.scale1ParticleWorkerExecutor.status(),
            finite: [diagnostics.totalEnergy, diagnostics.totalKE,
                diagnostics.totalPE].every(Number.isFinite),
        };
    });
    expect(result.endTick).toBeGreaterThan(start);
    expect(result.worker.state).toBe('ready');
    expect(result.worker.error).toBe('');
    expect(result.finite).toBe(true);
    expect(realErrors(consoleErrors)).toEqual([]);
});
