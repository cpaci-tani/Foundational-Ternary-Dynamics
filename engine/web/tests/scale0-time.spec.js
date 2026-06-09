// @ts-check
/**
 * Time Observatory panel — integration test.
 *
 * On a gravity-well Time scenario the panel must: tick a lab clock
 * (physicalTime > 0), measure a radial dτ/dt profile (Card B), accumulate a
 * growing twin-clock Δτ (Card C), and render the baked FTD-0252 kinematic data
 * + the [IMPOSED]/[M] tags (Card D). Verifies the whole chain:
 * getDiagnostics → lab clock; getScale0FieldSamples('latency') → radialProfile
 * → Cards B/C; static ftd0252-reference → Card D.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 Time Observatory', () => {
    test('lab clock ticks, twin Δτ grows, and the kinematic card renders baked FTD-0252 data', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdTimePanel), { timeout: 20_000 }).toBe(true);

        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) { sel.value = 's0-seed-time-twin-clocks'; sel.dispatchEvent(new Event('change', { bubbles: true })); }
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
            document.querySelector('#tab-bar .tab[data-panel="time"]')?.click();
        });

        // Card A — the lab clock advances (physical time ticks up).
        await expect.poll(async () => page.evaluate(() => {
            const m = window.__ftdTimePanel?.lastMetrics;
            return m ? m.physicalTime : 0;
        }), { timeout: 15_000, message: 'physical time should advance on a Time scenario' }).toBeGreaterThan(0);

        const a = await page.evaluate(() => {
            const m = window.__ftdTimePanel.lastMetrics;
            return { pt: m.physicalTime, fMin: m.fMin, dtauMin: m.dtauMin, gammaMax: m.gammaMax };
        });
        expect(a.pt, 'physical time > 0').toBeGreaterThan(0);
        expect(a.fMin, 'lapse f_min in (0,1]').toBeGreaterThan(0);
        expect(a.fMin, 'lapse f_min ≤ 1').toBeLessThanOrEqual(1.0000001);
        expect(a.dtauMin, 'dτ/dt min in (0,1]').toBeGreaterThan(0);
        expect(a.gammaMax, 'FTD γ_max ≥ 1').toBeGreaterThanOrEqual(0.999999);

        // Card C — twin Δτ accumulates and grows monotonically while playing.
        await expect.poll(async () => page.evaluate(() => window.__ftdTimePanel?.twin?.active),
            { timeout: 15_000, message: 'twin clocks should activate on the gravity well' }).toBe(true);
        const dtau0 = await page.evaluate(() => {
            const t = window.__ftdTimePanel.twin;
            return t.tauFar - t.tauDeep;
        });
        await expect.poll(async () => page.evaluate(() => window.__ftdTimePanel.historyLength),
            { timeout: 12_000, message: 'Δτ history grows as ticks advance' }).toBeGreaterThan(1);
        const tw = await page.evaluate(() => {
            const t = window.__ftdTimePanel.twin;
            return { tauDeep: t.tauDeep, tauFar: t.tauFar, dtau: t.tauFar - t.tauDeep };
        });
        expect(tw.tauDeep, 'τ_deep accumulating').toBeGreaterThan(0);
        expect(tw.tauFar, 'τ_far accumulating').toBeGreaterThan(0);
        // The far clock runs at least as fast as the deep clock (Δτ ≥ 0), and the
        // accumulated proper time has grown past the first-sample baseline.
        expect(tw.dtau, 'Δτ = τ_far − τ_deep ≥ 0').toBeGreaterThanOrEqual(-1e-9);
        expect(tw.dtau, 'Δτ grew from the initial latch').toBeGreaterThanOrEqual(dtau0 - 1e-9);

        // Card D — the kinematic card renders the baked FTD-0252 data + the
        // honest [IMPOSED] / [M] tags + the slider.
        const d = await page.evaluate(() => {
            const txt = document.getElementById('panel-time')?.textContent || '';
            const slider = document.getElementById('time-panel-vslider');
            return {
                txt,
                hasSlider: !!slider,
                sliderMax: slider ? slider.getAttribute('max') : null,
            };
        });
        expect(d.hasSlider, 'imposed-v slider present').toBe(true);
        expect(d.sliderMax, 'slider max = 0.95').toBe('0.95');
        expect(d.txt.includes('[IMPOSED]'), '[IMPOSED] velocity tag present').toBe(true);
        expect(d.txt.includes('[M]'), '[M] measured tag present').toBe(true);
        expect(d.txt.includes('[T]'), '[T] theory tag present').toBe(true);
        expect(d.txt.includes('[D]'), '[D] derived tag present').toBe(true);
        expect(d.txt.includes('FTD-0252'), 'FTD-0252 provenance rendered').toBe(true);
        expect(d.txt.includes('IR convergence'), 'IR-convergence mini-chart rendered').toBe(true);

        // The slider is interactive: moving it updates the imposed-v readout.
        await page.evaluate(() => {
            const s = document.getElementById('time-panel-vslider');
            if (s) { s.value = '0.80'; s.dispatchEvent(new Event('input', { bubbles: true })); }
        });
        const vval = await page.evaluate(() => document.getElementById('time-panel-vval')?.textContent || '');
        expect(vval, 'slider updates the v readout').toBe('0.80');

        // Charts paint: the panel renders multiple SVG chart paths (curves +
        // sparkline) — at least the kinematic dual-curve + IR mini-chart.
        const charts = await page.evaluate(() =>
            document.querySelectorAll('#panel-time .time-chart').length);
        expect(charts, 'kinematic + IR + radial charts present').toBeGreaterThanOrEqual(2);
    });
});
