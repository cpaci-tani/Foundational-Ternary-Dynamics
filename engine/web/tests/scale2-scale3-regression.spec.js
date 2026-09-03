// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

test.describe('Scale 2 AtomEngine shared-backend regression', () => {
    test('Scale 3 molecules retain an explicit stable force profile', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'molecules');

        const result = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const before = bridge.aeGetAtomData();
            for (let i = 0; i < 100; i++) bridge.aeTick();
            const after = bridge.aeGetAtomData();
            const diag = bridge.aeGetDiagnostics();
            const runtime = bridge.aeGetRuntimeState();
            const forces = bridge.aeGetForceDecomposition({ net: true });
            return {
                scenario: document.getElementById('mol-scenario-select')?.value,
                beforeCount: before.count,
                afterCount: after.count,
                finite: [...after.positions, ...after.charges, diag.totalEnergy].every(Number.isFinite),
                status: diag.lastError,
                toggles: runtime.toggles,
                forceLengths: [forces.ionic.length, forces.vdw.length, forces.bond.length,
                    forces.hbond.length, forces.angle.length, forces.dipole.length, forces.net.length],
            };
        });

        expect(result.beforeCount).toBeGreaterThan(1);
        expect(result.afterCount).toBe(result.beforeCount);
        expect(result.finite).toBe(true);
        expect(result.status).toBe('ok');
        expect(result.toggles.vdw).toBe(true);
        expect(result.toggles.bonds_force).toBe(true);
        expect(result.toggles.bonding).toBe(false);
        expect(result.forceLengths.every(length => length === result.afterCount * 3)).toBe(true);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('Scale 3 NaCl crystal remains ionic and does not auto-create covalent bonds', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'molecules');
        const result = await page.evaluate(() => {
            const select = document.getElementById('mol-scenario-select');
            select.value = 'mol-crystal';
            select.dispatchEvent(new Event('change', { bubbles: true }));
            const bridge = window._ftdBridge;
            for (let i = 0; i < 20; i++) bridge.aeTick();
            return { diag: bridge.aeGetDiagnostics(), runtime: bridge.aeGetRuntimeState() };
        });
        expect(result.diag.atomCount).toBe(27);
        expect(result.diag.bondCount).toBe(0);
        expect(result.diag.lastError).toBe('ok');
        expect(result.runtime.toggles.ionic).toBe(true);
        expect(result.runtime.toggles.bonding).toBe(false);
        expect(result.runtime.toggles.bonds_force).toBe(false);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
