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

async function selectPEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('pe-scenario-select');
        if (!sel) throw new Error('pe-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

async function runScale1(page, scenario = 's1-coulomb-orbit') {
    await gotoAndReady(page);
    await switchMode(page, 'particles');
    await selectPEScenario(page, scenario);
    await expect.poll(
        () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
        { timeout: 10_000, message: `${scenario} did not seed particles` },
    ).toBeGreaterThan(1);
    await page.evaluate(() => {
        const btn = document.getElementById('btn-play');
        if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
    });
    await page.waitForTimeout(1200);
}

test.describe('Scale 1 side panels', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('charts and telemetry grid switch to particle dynamics', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale1(page, 's1-coulomb-orbit');

        await openPanel(page, 'telemetry-grid');
        const grid = await page.evaluate(() => ({
            activeScale: document.querySelector('#panel-telemetry-grid')?.dataset.activeScale,
            titles: Array.from(document.querySelectorAll('#panel-telemetry-grid .telemetry-card-title'))
                .map((el) => el.textContent?.trim()),
            values: Array.from(document.querySelectorAll('#panel-telemetry-grid .telemetry-card-value'))
                .map((el) => el.textContent?.trim()),
        }));

        expect(grid.activeScale).toBe('1');
        expect(grid.titles).toEqual(expect.arrayContaining([
            'Total Energy',
            'Coulomb PE',
            'Gravity PE',
            'Max Net Force',
            '2-Body Separation',
        ]));
        expect(grid.titles).not.toEqual(expect.arrayContaining([
            'Total Flux',
            'Gauss Violation',
            'Lagrangian (L)',
        ]));
        expect(grid.values.some((v) => v && v !== '--')).toBe(true);

        await openPanel(page, 'charts');
        const charts = await page.evaluate(() => ({
            activeScale: document.querySelector('#panel-charts')?.dataset.activeScale,
            chips: Array.from(document.querySelectorAll('#panel-charts .charts-chip'))
                .map((el) => el.textContent?.trim()),
            activeCards: document.querySelectorAll('#panel-charts .chart-card:not(.is-leaving)').length,
            uplots: document.querySelectorAll('#panel-charts .uplot').length,
        }));

        expect(charts.activeScale).toBe('1');
        expect(charts.chips).toEqual(expect.arrayContaining([
            'Particle Energy (active potential terms)',
            'Momentum & Angular Momentum',
            'Net Forces',
            'Virial & RMS Velocity',
        ]));
        expect(charts.chips).not.toEqual(expect.arrayContaining([
            'Flux & Energy',
            'Charge Balance',
            'Entropy',
        ]));
        expect(charts.activeCards).toBeGreaterThan(0);
        expect(charts.uplots).toBeGreaterThan(0);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('diagnostics summarize the active Scale 1 scenario dynamics', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale1(page, 's1-cluster-pair');
        await openPanel(page, 'diagnostics');

        const diag = await page.evaluate(() => {
            const value = (row) => document
                .querySelector(`#panel-diagnostics .diag-scale1-root [data-row="${row}"] .diag-value`)
                ?.textContent?.trim();
            return {
                sectionTitles: Array.from(document.querySelectorAll('#panel-diagnostics .diag-scale1-root .diag-section-title'))
                    .map((el) => el.textContent?.trim()),
                scenario: value('scenario'),
                coulomb: value('coulomb-on'),
                gravity: value('gravity-on'),
                damping: value('damping-on'),
                softening: value('softening'),
                maxForce: value('max-force'),
                legacyPanelGone: !document.getElementById('pe-telemetry'),
            };
        });

        expect(diag.sectionTitles).toEqual(expect.arrayContaining([
            'Scenario Dynamics',
            'Active Hamiltonian',
            'Conservation',
            'Forces & Geometry',
        ]));
        expect(diag.scenario).toContain('A Pair of Orbiting Charges');
        expect(diag.coulomb).toBe('on');
        expect(diag.gravity).toBe('on');
        expect(diag.damping).toBe('off');
        expect(Number(diag.softening)).toBeCloseTo(0.1, 5);
        expect(diag.maxForce).not.toBe('0');
        // Legacy PE telemetry canvas panel is retired — the descriptor tables
        // are the single Scale-1 diagnostics surface.
        expect(diag.legacyPanelGone).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('energy drift re-baselines on scenario switch', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale1(page, 's1-coulomb-orbit');

        // Telemetry collection is demand-gated: open diagnostics NOW so
        // collectScale1 runs and _peInitialEnergy latches to this scenario.
        await openPanel(page, 'diagnostics');
        await page.waitForTimeout(800);

        // Switching scenarios must reset telemetryHub Scale-1 state so drift
        // is measured against the NEW scenario's initial energy. (The hub
        // additionally re-latches on any particle-count/toggle change.)
        await selectPEScenario(page, 's1-cluster-pair');
        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.peGetParticleData?.()?.count || 0),
            { timeout: 10_000, message: 's1-cluster-pair did not seed particles' },
        ).toBeGreaterThan(1);
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });
        await page.waitForTimeout(1200);

        await openPanel(page, 'diagnostics');
        const driftText = await page.evaluate(() => document
            .querySelector('#panel-diagnostics .diag-scale1-root [data-row="drift"] .diag-value')
            ?.textContent?.trim());

        expect(driftText).toBeTruthy();
        const drift = Number(driftText);
        expect(Number.isFinite(drift)).toBe(true);
        // A stale cross-scenario baseline would read as tens of percent;
        // re-baselined it stays small over a short run.
        expect(Math.abs(drift)).toBeLessThan(10);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('diagnostics sections track active overlay toggles (contextual telemetry)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale1(page, 's1-coulomb-orbit');
        await openPanel(page, 'diagnostics');

        const before = await page.evaluate(() => !!document
            .querySelector('#panel-diagnostics [data-section="pe-system"]')
            ?.checkVisibility?.());
        expect(before).toBe(false); // System overlay starts off

        await page.evaluate(() => document.getElementById('toggle-pe-system')?.click());
        await page.waitForTimeout(500);

        const after = await page.evaluate(() => !!document
            .querySelector('#panel-diagnostics [data-section="pe-system"]')
            ?.checkVisibility?.());
        expect(after).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
