// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function openPanel(page, panel) {
    await page.evaluate((panelId) => {
        const tab = document.querySelector(`#tab-bar .tab[data-panel="${panelId}"]`);
        if (!tab) throw new Error(`missing tab: ${panelId}`);
        tab.click();
    }, panel);
    await page.waitForTimeout(500);
}

async function selectAEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('ae-scenario-select');
        if (!sel) throw new Error('ae-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

async function runScale2(page, scenario = 'ae-h2-form') {
    await gotoAndReady(page);
    await switchMode(page, 'atoms');
    await selectAEScenario(page, scenario);
    await expect.poll(
        () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
        { timeout: 10_000, message: `${scenario} did not seed Scale 2 atoms` },
    ).toBeGreaterThan(1);
    await page.evaluate(() => {
        const btn = document.getElementById('btn-play');
        if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
    });
    await page.waitForTimeout(1200);
}

test.describe('Scale 2 side panels', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('charts and telemetry grid switch to atom dynamics', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale2(page, 'ae-h2-form');

        await openPanel(page, 'telemetry-grid');
        const grid = await page.evaluate(() => ({
            activeScale: document.querySelector('#panel-telemetry-grid')?.dataset.activeScale,
            titles: Array.from(document.querySelectorAll('#panel-telemetry-grid .telemetry-card-title'))
                .map((el) => el.textContent?.trim()),
            tempValue: Array.from(document.querySelectorAll('#panel-telemetry-grid .telemetry-card'))
                .find((c) => c.dataset.channelKey === 'aeTemp')
                ?.querySelector('.telemetry-card-value')?.textContent?.trim(),
            values: Array.from(document.querySelectorAll('#panel-telemetry-grid .telemetry-card-value'))
                .map((el) => el.textContent?.trim()),
        }));

        expect(grid.activeScale).toBe('2');
        expect(grid.titles).toEqual(expect.arrayContaining([
            'Total Energy',
            'PE (Ionic)',
            'PE (vdW)',
            'PE (Bond)',
            'Atom Count',
            'Bond Count',
            'Energy Drift',
        ]));
        // No Scale 0 leakage…
        expect(grid.titles).not.toEqual(expect.arrayContaining([
            'Total Flux',
            'Gauss Violation',
            'Lagrangian (L)',
        ]));
        // …and no Scale 1 leakage.
        expect(grid.titles).not.toEqual(expect.arrayContaining([
            'Max Net Force',
            '2-Body Separation',
        ]));
        // Temperature is a sim-units equipartition proxy (audit P0-10) —
        // labelled "(sim)", never MK/Kelvin.
        expect(grid.tempValue).toContain('(sim)');
        expect(grid.tempValue).not.toContain('MK');
        expect(grid.values.some((v) => v && v !== '--')).toBe(true);

        await openPanel(page, 'charts');
        const charts = await page.evaluate(() => ({
            activeScale: document.querySelector('#panel-charts')?.dataset.activeScale,
            chips: Array.from(document.querySelectorAll('#panel-charts .charts-chip'))
                .map((el) => el.textContent?.trim()),
            activeCards: document.querySelectorAll('#panel-charts .chart-card:not(.is-leaving)').length,
            uplots: document.querySelectorAll('#panel-charts .uplot').length,
        }));

        expect(charts.activeScale).toBe('2');
        expect(charts.chips).toEqual(expect.arrayContaining([
            'Atomic Energy',
            'Temperature',
            'Atoms & Bonds',
            'Momentum',
            'Energy Drift',
        ]));
        expect(charts.chips).not.toEqual(expect.arrayContaining([
            'Flux & Energy',
            'Entropy',
            'Particle Energy',
        ]));
        expect(charts.activeCards).toBeGreaterThan(0);
        expect(charts.uplots).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('diagnostics summarize active Scale 2 scenario dynamics', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale2(page, 'ae-water-dimer');
        await openPanel(page, 'diagnostics');

        const diag = await page.evaluate(() => {
            const value = (row) => document
                .querySelector(`#panel-diagnostics .diag-ae-root [data-row="${row}"] .diag-value`)
                ?.textContent?.trim();
            return {
                sectionTitles: Array.from(document.querySelectorAll('#panel-diagnostics .diag-ae-root .diag-section-title'))
                    .map((el) => el.textContent?.trim()),
                rootDisplay: getComputedStyle(document.querySelector('#panel-diagnostics .diag-ae-root')).display,
                scenario: value('scenario'),
                hbonds: value('hbonds-on'),
                angle: value('angle-on'),
                bonding: value('bonding-on'),
                ionic: value('ionic-on'),
                atoms: value('atoms'),
                bonds: value('bonds'),
                total: value('total'),
                temperature: value('temperature'),
                // Legacy AE stat-card block must still render below the
                // descriptors — including the Bond Count card, which was
                // wrongly scale3-only before this pass (B5).
                legacyVisible: (() => {
                    const el = document.getElementById('ae-diag-count');
                    return !!el && getComputedStyle(el).display !== 'none';
                })(),
                legacyBondCardVisible: (() => {
                    const el = document.getElementById('ae-diag-bonds');
                    const card = el?.closest('.card');
                    return !!card && getComputedStyle(card).display !== 'none';
                })(),
            };
        });

        expect(diag.sectionTitles).toEqual(expect.arrayContaining([
            'Scenario Dynamics',
            'Phase 3 Forces',
            'Active Hamiltonian',
            'Conservation & Thermal',
        ]));
        expect(diag.rootDisplay).not.toBe('none');
        expect(diag.scenario).toContain('Water');
        // ae-water-dimer: pre-bonded waters, auto-bonding then disabled,
        // Phase 3 h-bonds + angle strain enabled (scenarios.js).
        expect(diag.hbonds).toBe('on');
        expect(diag.angle).toBe('on');
        expect(diag.bonding).toBe('off');
        expect(diag.ionic).toBe('on');
        expect(diag.atoms).toBe('6');
        expect(diag.bonds).toBe('4');
        expect(diag.total).not.toBe('0');
        expect(diag.temperature).toBeDefined();
        expect(diag.legacyVisible).toBe(true);
        expect(diag.legacyBondCardVisible).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
