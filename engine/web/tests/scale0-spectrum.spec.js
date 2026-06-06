// @ts-check
/**
 * Lattice Spectroscopy panel — integration + science check.
 *
 * The decisive test is Parseval: on a real flux field the panel's E(k) must
 * satisfy ΣE(k) = Σ|J|² (ratio ≈ 1). That exercises the whole pipeline —
 * sampler → dense reconstruction → 3-D FFT → radial binning — end to end, so a
 * pass means the instrument is genuinely correct, not just non-crashing.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 Lattice Spectroscopy', () => {
    test('E(k) is Parseval-consistent on a live flux field; Deep Measure widens the band', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdSpectrumPanel), { timeout: 20_000 }).toBe(true);

        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) { sel.value = 'flux-pulse'; sel.dispatchEvent(new Event('change', { bubbles: true })); }
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
            document.querySelector('#tab-bar .tab[data-panel="spectrum"]')?.click();
        });

        // Wait for the live spectrum to populate from the flux field.
        await expect.poll(async () => page.evaluate(() => {
            const s = window.__ftdSpectrumPanel?.lastSpec;
            return !!(s && s.spec && s.spec.E.some((v) => v > 0));
        }), { timeout: 12_000, message: 'live E(k) should populate on flux-pulse' }).toBe(true);

        const live = await page.evaluate(() => {
            const s = window.__ftdSpectrumPanel.lastSpec;
            return { parseval: s.parseval, kPeak: s.peak.kPeak, M: s.M, kNyq: s.spec.kNyq, nonZero: s.spec.E.filter((v) => v > 0).length };
        });
        // ── the science check ──
        expect(Math.abs(live.parseval - 1), `Parseval ΣE/Σ|J|²=${live.parseval}`).toBeLessThan(0.05);
        expect(live.kPeak, 'a dominant mode k* exists').toBeGreaterThan(0);
        expect(live.M, 'live grid is 32³').toBe(32);
        expect(live.nonZero, 'spectrum has populated k-bins').toBeGreaterThan(0);

        // ── all four sections render ──
        const dom = await page.evaluate(() => {
            const p = document.getElementById('spectrum-panel');
            return {
                topoRows: p.querySelectorAll('#spectrum-panel-topo .spec-row').length,
                metricRows: p.querySelectorAll('#spectrum-panel-metrics .spec-metric-row').length,
                energyBar: !!p.querySelector('#spectrum-panel-energy .spec-energy-bar'),
                specPath: !!p.querySelector('#spectrum-panel-spec path'),
            };
        });
        expect(dom.topoRows, 'topology rows').toBeGreaterThanOrEqual(4);
        expect(dom.metricRows, '5 metric rows').toBe(5);
        expect(dom.energyBar, 'energy partition bar').toBe(true);
        expect(dom.specPath, 'E(k) polyline drawn').toBe(true);

        // ── Deep Measure: full band (M=64, higher Nyquist), still Parseval-consistent ──
        await page.evaluate(() => window.__ftdSpectrumPanel.deepMeasure());
        await expect.poll(async () => page.evaluate(() => window.__ftdSpectrumPanel?.lastSpec?.M),
            { timeout: 10_000, message: 'Deep Measure should produce a 64³ spectrum' }).toBe(64);
        const deep = await page.evaluate(() => {
            const s = window.__ftdSpectrumPanel.lastSpec;
            return { parseval: s.parseval, kNyq: s.spec.kNyq, mode: window.__ftdSpectrumPanel.mode };
        });
        expect(deep.kNyq, 'Deep Measure resolves higher k than the live band-limited view').toBeGreaterThan(live.kNyq);
        expect(Math.abs(deep.parseval - 1), `deep Parseval=${deep.parseval}`).toBeLessThan(0.05);
        expect(deep.mode).toBe('deep');
    });
});
