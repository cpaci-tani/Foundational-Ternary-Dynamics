// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

/**
 * Scale 2 physics gate — numeric assertions against the LIVE engine
 * (the JS mock is the production AE backend on every bridge). All
 * dynamics are driven with deterministic bridge.aeTick() loops inside
 * page.evaluate — never wall-clock playback — so tick counts are exact.
 * The page stays paused throughout (switchMode loads scenarios paused).
 */

async function selectAEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('ae-scenario-select');
        if (!sel) throw new Error('ae-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

async function loadScenario(page, id, expectedAtoms) {
    await selectAEScenario(page, id);
    await expect.poll(
        () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
        { timeout: 10_000, message: `${id} did not seed atoms` },
    ).toBe(expectedAtoms);
}

test.describe('Scale 2 physics invariants', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
    });

    test('temperature is the equipartition identity 2·KE/(3N) in sim units', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadScenario(page, 'ae-ar-cluster', 8);

        const t = await page.evaluate(() => {
            const b = window._ftdBridge;
            for (let i = 0; i < 50; i++) b.aeTick();
            const d = b.aeGetDiagnostics();
            const expected = d.atomCount > 0 ? (2 * d.totalKE) / (3 * d.atomCount) : 0;
            return { temperature: d.temperature, expected, ke: d.totalKE, n: d.atomCount };
        });

        // Same-snapshot identity (mock-atom-engine computes T from the same
        // KE sum) — must hold to float precision, not approximately.
        expect(Math.abs(t.temperature - t.expected)).toBeLessThanOrEqual(1e-12 * Math.max(1, Math.abs(t.expected)));
        expect(t.ke).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('momentum is conserved through a head-on LJ collision', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadScenario(page, 'ae-collision', 2);

        const p = await page.evaluate(() => {
            const b = window._ftdBridge;
            const before = b.aeGetDiagnostics();
            for (let i = 0; i < 300; i++) b.aeTick();
            const after = b.aeGetDiagnostics();
            return {
                before: [before.momentumX, before.momentumY, before.momentumZ],
                after: [after.momentumX, after.momentumY, after.momentumZ],
                keAfter: after.totalKE,
            };
        });

        // Equal masses, opposite velocities: p starts exactly 0 and the LJ
        // pair force is Newton's-3rd symmetric — each component stays ~0.
        // (Scenario defaults: thermostat/damping/bonding off; speeds well
        // under AE_SPEED_MAX so the limiter never clamps.)
        for (let k = 0; k < 3; k++) {
            expect(Math.abs(p.before[k])).toBeLessThan(1e-9);
            expect(Math.abs(p.after[k])).toBeLessThan(1e-6);
        }
        expect(p.keAfter).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('energy drift stays bounded over 500 conservative vdW ticks', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadScenario(page, 'ae-ar-cluster', 8);

        const e = await page.evaluate(() => {
            const b = window._ftdBridge;
            const e0 = b.aeGetDiagnostics().totalEnergy;
            for (let i = 0; i < 500; i++) b.aeTick();
            const e1 = b.aeGetDiagnostics().totalEnergy;
            return { e0, e1 };
        });

        // Velocity Verlet + LJ with damping off. 5% is a regression
        // tripwire for integrator/force bugs, not a symplectic-order claim.
        if (Math.abs(e.e0) > 1e-6) {
            expect(Math.abs(e.e1 - e.e0) / Math.abs(e.e0)).toBeLessThan(0.05);
        } else {
            expect(Math.abs(e.e1 - e.e0)).toBeLessThan(0.05);
        }

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('H₂ forms exactly one bond and keeps it', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadScenario(page, 'ae-h2-form', 2);

        const formed = await page.evaluate(() => {
            const b = window._ftdBridge;
            let bondCount = 0;
            let ticks = 0;
            for (let chunk = 0; chunk < 20 && bondCount === 0; chunk++) {
                for (let i = 0; i < 100; i++) b.aeTick();
                ticks += 100;
                bondCount = b.aeGetDiagnostics().bondCount;
            }
            const atFormation = bondCount;
            for (let i = 0; i < 200; i++) b.aeTick();
            const after = b.aeGetDiagnostics().bondCount;
            return { atFormation, after, ticks };
        });

        expect(formed.atFormation, `no bond within ${formed.ticks} ticks`).toBe(1);
        // No spurious break (break threshold 3.5·r_eq) and no double-bond
        // (H max_bonds = 1).
        expect(formed.after).toBe(1);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('VSEPR angle strain relaxes H₂O toward the 104.5° bent geometry', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadScenario(page, 'ae-vsepr-bent', 3);

        const result = await page.evaluate(() => {
            const b = window._ftdBridge;
            // The scenario seeds at 150° with angle strain on but no
            // dissipation — enable damping IN-TEST so the oscillation
            // relaxes instead of ringing forever.
            b.aeSetDamping(true);
            const angleNow = () => {
                const d = b.aeGetAtomData();
                // Seed order: O at index 0, H at 1 and 2 (scenarios.js).
                const ox = d.positions[0], oy = d.positions[1], oz = d.positions[2];
                const v1 = [d.positions[3] - ox, d.positions[4] - oy, d.positions[5] - oz];
                const v2 = [d.positions[6] - ox, d.positions[7] - oy, d.positions[8] - oz];
                const m1 = Math.hypot(...v1), m2 = Math.hypot(...v2);
                const cos = (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) / (m1 * m2);
                return (Math.acos(Math.max(-1, Math.min(1, cos))) * 180) / Math.PI;
            };
            const start = angleNow();
            const trace = [];
            for (let chunk = 0; chunk < 12; chunk++) {
                for (let i = 0; i < 500; i++) b.aeTick();
                trace.push(+angleNow().toFixed(2));
            }
            return { start, final: angleNow(), trace };
        });

        expect(result.start).toBeGreaterThan(140);   // seeded at 150°
        expect(
            Math.abs(result.final - 104.5),
            `H-O-H angle ${result.final.toFixed(2)}° after relaxation (trace: ${result.trace.join(', ')})`,
        ).toBeLessThanOrEqual(3);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('Berendsen thermostat pulls the gas to its target temperature', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadScenario(page, 'ae-thermal-gas', 12);

        const t = await page.evaluate(() => {
            const b = window._ftdBridge;
            for (let i = 0; i < 1500; i++) b.aeTick();
            const d = b.aeGetDiagnostics();
            const rt = b.aeGetRuntimeState();
            return { temperature: d.temperature, target: rt.thermostatTemp, thermostatOn: rt.toggles.thermostat };
        });

        expect(t.thermostatOn).toBe(true);
        expect(t.target).toBeCloseTo(1.0, 5);
        // ±30% band — the thermostat is weak coupling (tau = 10) on a
        // 12-atom gas, so equilibrium fluctuations are large.
        expect(t.temperature).toBeGreaterThan(0.7);
        expect(t.temperature).toBeLessThan(1.3);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('SEMF binding-energy curve peaks near Fe-56', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        const ba = await page.evaluate(async () => {
            const { atomicEnergy } = await import('./js/atomic-energy.js');
            return {
                he4: atomicEnergy(2).bindingPerNucleon,
                fe56: atomicEnergy(26).bindingPerNucleon,
                u238: atomicEnergy(92).bindingPerNucleon,
            };
        });

        // Wapstra-coefficient SEMF: B/A(Fe-56) ≈ 8.79 MeV experimentally;
        // accept the formula within (8.3, 9.2) and require the curve SHAPE
        // (iron above both the light and heavy ends).
        expect(ba.fe56).toBeGreaterThan(8.3);
        expect(ba.fe56).toBeLessThan(9.2);
        expect(ba.fe56).toBeGreaterThan(ba.he4);
        expect(ba.fe56).toBeGreaterThan(ba.u238);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
