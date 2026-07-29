// @ts-check
// "⤴ Scale up" promotion pipeline: live Scale-0 lattice clusters →
// Scale-1 continuous particles (mass = N·K_B, charge = sign·N).
import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

test.describe('Scale 0 → Scale 1 promotion', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
    });

    test('Scale up promotes lattice clusters into the native particle engine', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        // Seed a Scale-0 scenario that manifests matter.
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (!sel) throw new Error('scenario-select not found');
            sel.value = 's0-seed-hydrogen';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await page.waitForTimeout(1500);

        // Fire the promotion (async: enables knot_tracking at capture time,
        // steps the active owner, polls telemetry, falls back to voxel
        // connected-components below the tracker's min cluster size).
        await page.evaluate(() => {
            const btn = document.getElementById('btn-scale-up');
            if (!btn) throw new Error('btn-scale-up not found');
            btn.click();
        });

        // Mode switches and the promoted scenario seeds the native engine.
        await expect.poll(
            () => page.evaluate(() => ({
                mode: document.getElementById('engine-mode')?.value,
                scen: document.getElementById('pe-scenario-select')?.value,
                count: window._ftdBridge?.peGetParticleData?.()?.count || 0,
            })),
            { timeout: 15_000, message: 'promotion did not land in Scale 1 with particles' },
        ).toMatchObject({ mode: 'particles', scen: 's1-promoted-lattice' });

        const state = await page.evaluate(() => {
            const b = window._ftdBridge;
            const data = b.peGetParticleData();
            const masses = Array.from(data.masses);
            const charges = Array.from(data.charges);
            for (let i = 0; i < 20; i++) b.peTick();
            const after = b.peGetDiagnostics();
            return {
                count: data.count,
                masses,
                charges,
                tickAdvanced: after.tick,
                promoCard: document.getElementById('pe-promotion-info')?.textContent || '',
            };
        });

        expect(state.count).toBeGreaterThan(0);
        const K_B = 0.511;
        for (const m of state.masses) {
            // mass = N·K_B for integer N ≥ 1
            const n = m / K_B;
            expect(Math.abs(n - Math.round(n))).toBeLessThan(1e-9);
            expect(Math.round(n)).toBeGreaterThanOrEqual(1);
        }
        for (const q of state.charges) {
            expect(Math.abs(q)).toBeGreaterThanOrEqual(1);
            expect(Math.abs(q)).toBeLessThanOrEqual(127);   // int8 clamp
        }
        expect(state.tickAdvanced).toBe(20);
        expect(state.promoCard).toContain('cluster');
        expect(state.promoCard).toContain('N·K_B');

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('promotion with an empty lattice aborts gracefully', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        // flux-pulse manifests nothing at t=0 — the capture should time out,
        // toast, and stay in lattice mode.
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 'flux-pulse';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await page.waitForTimeout(1000);
        await page.evaluate(() => document.getElementById('btn-scale-up')?.click());
        await page.waitForTimeout(2500);

        const state = await page.evaluate(() => ({
            mode: document.getElementById('engine-mode')?.value,
            btnEnabled: !document.getElementById('btn-scale-up')?.disabled,
        }));
        expect(state.mode).toBe('lattice');
        expect(state.btnEnabled).toBe(true);   // button re-enabled after abort

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('admissibility rings render on promoted particles', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-hydrogen';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await page.waitForTimeout(1500);
        await page.evaluate(() => document.getElementById('btn-scale-up')?.click());
        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 15_000 },
        ).toBeGreaterThan(0);

        const ringCount = await page.evaluate(() => {
            const vp = window.__FTD_DEV__?.viewport;
            return vp?._particleRenderer?._admissibilityRings?.children?.length ?? -1;
        });
        expect(ringCount).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
