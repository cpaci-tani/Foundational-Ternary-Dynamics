// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectPEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('pe-scenario-select');
        if (!sel) throw new Error('pe-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

test.describe('Scale 1 native-engine scenarios and overlays', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('default Coulomb orbit loads on the native engine with live data', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'default scenario did not seed Scale 1 particles' },
        ).toBeGreaterThanOrEqual(2);

        const state = await page.evaluate(async () => {
            const reg = await import('./js/scales/scale1/scenario-registry.js');
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            const ext = b.peGetExtendedData();
            const forces = b.peGetForces();
            const diag = b.peGetDiagnostics();
            const caps = b.peGetBackendCapabilities();
            let maxSpeed = 0;
            for (let i = 0; i < data.count; i++) {
                maxSpeed = Math.max(maxSpeed, Math.hypot(
                    data.velocities[i * 3] || 0,
                    data.velocities[i * 3 + 1] || 0,
                    data.velocities[i * 3 + 2] || 0));
            }
            return {
                preset: reg.getScale1ScenarioPreset(reg.DEFAULT_SCALE1_SCENARIO),
                backend: caps.backend,
                nativeForces: caps.nativeForces,
                count: data.count,
                velocityLength: data.velocities?.length || 0,
                massLength: data.masses?.length || 0,
                spinAxesLength: data.spinAxes?.length || 0,
                maxSpeed,
                extCount: ext?.count || 0,
                forceCount: forces.count,
                maxForce: forces.maxForce,
                coulombPE: diag.coulombPE,
                totalEnergy: diag.totalEnergy,
                descShown: !!document.getElementById('s1-scenario-desc-text')?.textContent?.length,
            };
        });

        expect(state.backend).toBe('wasm');
        expect(state.nativeForces).toBe(true);
        expect(state.preset.physics.coulomb).toBe(true);
        expect(state.count).toBeGreaterThanOrEqual(2);
        expect(state.velocityLength).toBe(state.count * 3);
        expect(state.massLength).toBe(state.count);
        expect(state.spinAxesLength).toBe(state.count * 3);
        expect(state.maxSpeed).toBeGreaterThan(0);          // orbit IC applied
        expect(state.extCount).toBe(state.count);
        expect(state.forceCount).toBe(state.count);
        expect(state.maxForce).toBeGreaterThan(0);
        expect(Math.abs(state.coulombPE)).toBeGreaterThan(0);
        expect(state.totalEnergy).toBeLessThan(0);          // bound orbit
        expect(state.descShown).toBe(true);                 // epistemic status rendered

        const decomp = await page.evaluate(() => {
            const d = window._ftdBridge?.peGetForceDecomposition?.();
            return d ? { count: d.count, maxCoulomb: d.maxCoulomb, maxNet: d.maxNet } : null;
        });
        expect(decomp?.count).toBe(state.count);
        expect(decomp?.maxCoulomb).toBeGreaterThan(0);
        expect(decomp?.maxNet).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('cluster pair carries the N·K_B mass law and ±N charges', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-cluster-pair');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'cluster pair did not seed' },
        ).toBe(2);

        const state = await page.evaluate(() => {
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            for (let i = 0; i < 30; i++) b.peTick();
            const diag = b.peGetDiagnostics();
            return {
                masses: Array.from(data.masses),
                charges: Array.from(data.charges),
                totalEnergy: diag.totalEnergy,
                tick: diag.tick,
            };
        });

        const K_B = 0.511;
        expect(state.masses[0]).toBeCloseTo(20 * K_B, 6);
        expect(state.masses[1]).toBeCloseTo(20 * K_B, 6);
        expect(state.charges.slice().sort((a, b) => a - b)).toEqual([-20, 20]);
        expect(state.totalEnergy).toBeLessThan(0);   // bound binary
        expect(state.tick).toBe(30);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('empty-zoo scenario is genuinely empty and Zoo injection works', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-empty-zoo');

        const result = await page.evaluate(() => {
            const b = window._ftdBridge;
            const before = b.peGetParticleData().count;
            const id = b.peAddParticle('electron', -1, 5, 0, 0, 0, 0, 0, 0.511, 0.1);
            const after = b.peGetParticleData().count;
            const types = b.peGetParticleTypes();
            return { before, after, id, taggedElectron: types.get(id) === 'electron' };
        });

        expect(result.before).toBe(0);
        expect(result.after).toBe(1);
        expect(result.id).toBeGreaterThanOrEqual(0);
        expect(result.taggedElectron).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
