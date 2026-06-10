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

test.describe('Scale 1 particle scenarios and overlays', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('hydrogen loads with rich particle data and default overlays active', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'hydrogen did not seed Scale 1 particles' },
        ).toBeGreaterThan(1);

        const state = await page.evaluate(async () => {
            const presets = await import('./js/scales/scale1/scenarios.js');
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            const ext = b.peGetExtendedData();
            const forces = b.peGetForces();
            const src = b.peGetFieldSources();
            const diag = b.peGetDiagnostics();
            const maxMass = data.masses ? Math.max(...Array.from(data.masses)) : 0;
            let maxSpeed = 0;
            for (let i = 0; i < data.count; i++) {
                const vx = data.velocities[i * 3] || 0;
                const vy = data.velocities[i * 3 + 1] || 0;
                const vz = data.velocities[i * 3 + 2] || 0;
                maxSpeed = Math.max(maxSpeed, Math.hypot(vx, vy, vz));
            }
            return {
                preset: presets.getPEScenarioPreset('pe-hydrogen'),
                count: data.count,
                velocityLength: data.velocities?.length || 0,
                massLength: data.masses?.length || 0,
                lockedLength: data.locked?.length || 0,
                maxMass,
                maxSpeed,
                extCount: ext?.count || 0,
                forceCount: forces.count,
                maxForce: forces.maxForce,
                sourceMassMax: src.masses?.length ? Math.max(...Array.from(src.masses)) : 0,
                coulombPE: diag.coulombPE,
                gravityPE: diag.gravityPE,
                buttons: {
                    velocities: document.getElementById('toggle-velocities')?.classList.contains('active') || false,
                    trails: document.getElementById('toggle-trails')?.classList.contains('active') || false,
                    potential: document.getElementById('toggle-pe-potential')?.classList.contains('active') || false,
                    forces: document.getElementById('toggle-pe-forces')?.classList.contains('active') || false,
                    gravity: document.getElementById('toggle-pe-gravity')?.classList.contains('active') || false,
                    damping: document.getElementById('toggle-pe-damping')?.classList.contains('active') || false,
                },
            };
        });

        expect(state.preset.physics.coulomb).toBe(true);
        expect(state.preset.physics.gravity).toBe(false);
        expect(state.count).toBeGreaterThanOrEqual(2);
        expect(state.velocityLength).toBe(state.count * 3);
        expect(state.massLength).toBe(state.count);
        expect(state.lockedLength).toBe(state.count);
        expect(state.maxMass).toBeGreaterThan(100);
        expect(state.maxSpeed).toBeGreaterThan(0);
        expect(state.extCount).toBe(state.count);
        expect(state.forceCount).toBe(state.count);
        expect(state.maxForce).toBeGreaterThan(0);
        expect(state.sourceMassMax).toBeGreaterThan(100);
        expect(Math.abs(state.coulombPE)).toBeGreaterThan(0);
        expect(Math.abs(state.gravityPE || 0)).toBe(0);
        expect(state.buttons).toMatchObject({
            velocities: true,
            trails: true,
            potential: true,
            forces: true,
            gravity: false,
            damping: false,
        });

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('micro black hole preset switches to gravity-only overlays and masses', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 'pe-micro-bh');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 'micro-BH did not seed Scale 1 particles' },
        ).toBeGreaterThan(4);

        const state = await page.evaluate(async () => {
            const presets = await import('./js/scales/scale1/scenarios.js');
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            const forces = b.peGetForces();
            const src = b.peGetFieldSources();
            const diag = b.peGetDiagnostics();
            return {
                preset: presets.getPEScenarioPreset('pe-micro-bh'),
                count: data.count,
                maxMass: data.masses?.length ? Math.max(...Array.from(data.masses)) : 0,
                sourceMassMax: src.masses?.length ? Math.max(...Array.from(src.masses)) : 0,
                maxForce: forces.maxForce,
                coulombPE: diag.coulombPE,
                gravityPE: diag.gravityPE,
                controls: {
                    coulomb: document.getElementById('pe-coulomb')?.checked || false,
                    gravity: document.getElementById('pe-gravity')?.checked || false,
                    damping: document.getElementById('pe-damping')?.checked || false,
                    softening: document.getElementById('pe-soft-slider')?.value || '',
                },
                buttons: {
                    gravityDynamics: document.getElementById('toggle-pe-gravity')?.classList.contains('active') || false,
                    gravityField: document.getElementById('toggle-pe-gravity-field')?.classList.contains('active') || false,
                    potential: document.getElementById('toggle-pe-potential')?.classList.contains('active') || false,
                    forces: document.getElementById('toggle-pe-forces')?.classList.contains('active') || false,
                },
            };
        });

        expect(state.preset.physics.coulomb).toBe(false);
        expect(state.preset.physics.gravity).toBe(true);
        expect(state.count).toBeGreaterThan(4);
        expect(state.maxMass).toBeGreaterThanOrEqual(5000);
        expect(state.sourceMassMax).toBeGreaterThanOrEqual(5000);
        expect(state.maxForce).toBeGreaterThan(0);
        expect(Math.abs(state.coulombPE || 0)).toBe(0);
        expect(state.gravityPE).toBeLessThan(0);
        expect(state.controls).toMatchObject({
            coulomb: false,
            gravity: true,
            damping: false,
            softening: '1',
        });
        expect(state.buttons).toMatchObject({
            gravityDynamics: true,
            gravityField: true,
            potential: false,
            forces: true,
        });

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
