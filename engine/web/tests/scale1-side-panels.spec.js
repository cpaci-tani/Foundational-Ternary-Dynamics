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

    test('Controls owns every applicable native Scale 1 physics toggle', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectPEScenario(page, 's1-quantum-exchange-eligible');
        await openPanel(page, 'controls');

        const initial = await page.evaluate(() => {
            const physicsCard = document.querySelector('[data-scale1-physics-card]');
            const contextCard = document.querySelector('[data-scale1-control-card="context"]');
            const inputs = Array.from(physicsCard?.querySelectorAll('[data-pe-toggle]') || []);
            const registryRows = Array.from(window._ftdBridge?.peGetPhysicsRegistry?.()?.physics || []);
            const registryKeys = registryRows.map(spec => spec.toggle);
            const availableRegistryKeys = registryRows.filter(spec => spec.available)
                .map(spec => spec.toggle);
            const uiKeys = inputs.map(input => input.dataset.peToggle);
            return {
                activePanel: document.querySelector('.panel.active')?.id,
                physicsTabVisible: document.querySelector('#tab-bar .tab[data-panel="physics"]')
                    ?.checkVisibility?.(),
                physicsCardCount: document.querySelectorAll('[data-scale1-physics-card]').length,
                contextCardCount: document.querySelectorAll('[data-scale1-control-card="context"]').length,
                contextInRoot: !!document.querySelector('#panel-controls-grid > [data-scale1-control-card="context"]'),
                physicsInRoot: !!document.querySelector('#panel-controls-grid > [data-scale1-physics-card]'),
                contextInHiddenScale5: !!contextCard?.closest('.scale5-only'),
                scenarioDetailsPresent: !!document.getElementById('pe-scenario-details'),
                scenarioDetailsOpen: document.getElementById('pe-scenario-details')?.open,
                scenarioDetailsSummary: document.getElementById('pe-scenario-details-summary')?.textContent?.trim(),
                scenarioContract: document.getElementById('pe-scenario-contract')?.textContent?.trim(),
                toggleCount: uiKeys.length,
                uniqueToggleCount: new Set(uiKeys).size,
                registryKeys,
                availableRegistryKeys,
                uiKeys,
                exchangeChecked: document.getElementById('pe-exchange')?.checked,
                exchangeEnabled: !document.getElementById('pe-exchange')?.disabled,
                retiredPresent: !!document.getElementById('pe-relativistic'),
                profileButtons: physicsCard?.querySelectorAll('[data-pe-profile]').length,
                summary: document.getElementById('pe-physics-active-count')?.textContent,
                profile: document.getElementById('pe-physics-profile-state')?.textContent,
                statuses: Array.from(physicsCard?.querySelectorAll('.pe-physics-row-status') || [])
                    .map(row => row.textContent?.trim()).filter(Boolean),
                tooltipCoverage: inputs.every(input => {
                    const label = document.querySelector(`label[for="${input.id}"]`);
                    const row = input.closest('.toggle-row');
                    return input.dataset.uiTooltip?.includes('Validation:')
                        && input.dataset.uiTooltip?.includes('Evidence:')
                        && label?.dataset.uiTooltip === input.dataset.uiTooltip
                        && row?.dataset.uiTooltip === input.dataset.uiTooltip;
                }),
            };
        });

        expect(initial.activePanel).toBe('panel-controls');
        expect(initial.physicsTabVisible).toBe(false);
        expect(initial.physicsCardCount).toBe(1);
        expect(initial.contextCardCount).toBe(1);
        expect(initial.contextInRoot).toBe(true);
        expect(initial.physicsInRoot).toBe(true);
        expect(initial.contextInHiddenScale5).toBe(false);
        expect(initial.scenarioDetailsPresent).toBe(true);
        expect(initial.scenarioDetailsOpen).toBe(false);
        expect(initial.scenarioDetailsSummary).toMatch(/^Scenario details · /);
        expect(initial.scenarioContract.toLowerCase()).toContain('exchange');
        expect(initial.scenarioContract).toContain('runtime source revision:');
        expect(initial.scenarioContract).toContain('artifact revision:');
        expect(initial.scenarioContract).toContain('object provenance:');
        expect(initial.toggleCount).toBe(11);
        expect(initial.uniqueToggleCount).toBe(11);
        expect(initial.registryKeys).toContain('relativistic');
        expect([...initial.uiKeys].sort()).toEqual([...initial.availableRegistryKeys].sort());
        expect(initial.exchangeChecked).toBe(true);
        expect(initial.exchangeEnabled).toBe(true);
        expect(initial.retiredPresent).toBe(false);
        expect(initial.profileButtons).toBe(3);
        expect(initial.summary).toBe('2 active · 11 available');
        expect(initial.profile).toBe('Scenario profile');
        expect(initial.statuses).toHaveLength(11);
        expect(initial.tooltipCoverage).toBe(true);

        await page.locator('#pe-scenario-details-summary').click();
        await expect(page.locator('#pe-scenario-details')).toHaveAttribute('open', '');
        await expect(page.locator('#pe-scenario-contract')).toBeVisible();

        const floatedScenarioDetails = await page.evaluate(() => {
            const dock = window.__ftdCtx?.appShell?.panelDock;
            const win = dock?.floatPanel?.('controls', 120, 80);
            if (!win?.el) return null;
            const details = win.el.querySelector('#pe-scenario-details');
            const contract = win.el.querySelector('#pe-scenario-contract');
            const report = {
                inWindow: !!details,
                open: !!details?.open,
                contractVisible: !!contract?.checkVisibility?.(),
                clipped: !!contract && contract.scrollHeight > contract.clientHeight + 1
                    && ['hidden', 'clip'].includes(getComputedStyle(contract).overflowY),
            };
            win.dock();
            return report;
        });
        expect(floatedScenarioDetails).toEqual({
            inWindow: true,
            open: true,
            contractVisible: true,
            clipped: false,
        });

        await page.locator('label[for="pe-exchange"]').hover();
        await expect(page.locator('#ui-tooltip')).toBeVisible();
        await expect(page.locator('#ui-tooltip')).toContainText('exchange');
        await expect(page.locator('#ui-tooltip')).toContainText('Validation:');
        await expect(page.locator('#ui-tooltip')).toContainText('Evidence:');

        const roundTrips = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            return Array.from(bridge?.peGetPhysicsRegistry?.()?.physics || []).map(spec => {
                const input = document.querySelector(`[data-pe-toggle="${spec.toggle}"]`);
                const before = !!bridge.peGetToggle(spec.toggle);
                if (!spec.available) {
                    return {
                        toggle: spec.toggle,
                        controlPresent: !!input,
                        rejected: bridge.peSetToggle(spec.toggle, true) === false,
                    };
                }
                input.checked = !before;
                input.dispatchEvent(new Event('change', { bubbles: true }));
                const changed = bridge.peGetToggle(spec.toggle) === !before;
                input.checked = before;
                input.dispatchEvent(new Event('change', { bubbles: true }));
                return { toggle: spec.toggle, changed, restored: bridge.peGetToggle(spec.toggle) === before };
            });
        });
        expect(roundTrips.filter(row => row.toggle !== 'relativistic')
            .every(row => row.changed && row.restored)).toBe(true);
        expect(roundTrips.find(row => row.toggle === 'relativistic')).toMatchObject({
            controlPresent: false,
            rejected: true,
        });

        await page.locator('#btn-pe-profile-applicable').click();
        const applicable = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const specs = Array.from(bridge?.peGetPhysicsRegistry?.()?.physics || []);
            return {
                allAvailableOn: specs.filter(spec => spec.available)
                    .every(spec => bridge.peGetToggle(spec.toggle)),
                retiredOff: !bridge.peGetToggle('relativistic'),
                summary: document.getElementById('pe-physics-active-count')?.textContent,
                profile: document.getElementById('pe-physics-profile-state')?.textContent,
            };
        });
        expect(applicable).toEqual({
            allAvailableOn: true,
            retiredOff: true,
            summary: '11 active · 11 available',
            profile: 'All applicable',
        });

        await page.locator('#btn-pe-profile-verified').click();
        const verified = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const specs = Array.from(bridge?.peGetPhysicsRegistry?.()?.physics || []);
            return {
                exact: specs.every(spec => bridge.peGetToggle(spec.toggle) === !!spec.verifiedProfile),
                summary: document.getElementById('pe-physics-active-count')?.textContent,
                profile: document.getElementById('pe-physics-profile-state')?.textContent,
            };
        });
        expect(verified).toEqual({
            exact: true,
            summary: '2 active · 11 available',
            profile: 'Verified profile',
        });

        await page.locator('#btn-pe-profile-scenario').click();
        await expect(page.locator('#pe-physics-profile-state')).toHaveText('Scenario profile');

        await page.locator('#pe-exchange').uncheck();
        const modified = await page.evaluate(() => ({
            engineValue: window._ftdBridge?.peGetToggle?.('exchange'),
            summary: document.getElementById('pe-physics-active-count')?.textContent,
            profile: document.getElementById('pe-physics-profile-state')?.textContent,
            warning: document.getElementById('pe-physics-profile-state')?.classList.contains('is-modified'),
        }));
        expect(modified.engineValue).toBe(false);
        expect(modified.summary).toBe('1 active · 11 available');
        expect(modified.profile).toBe('Modified profile');
        expect(modified.warning).toBe(true);

        await expect(page.locator('[data-scale1-control-card="context"]')).toBeVisible();
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('diagnostics contain live Scale 1 telemetry without scenario settings', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await runScale1(page, 's1-cluster-pair');
        await openPanel(page, 'diagnostics');

        const diag = await page.evaluate(() => {
            const value = (row) => document
                .querySelector(`#panel-diagnostics .diag-scale1-root [data-row="${row}"] .diag-value`)
                ?.textContent?.trim();
            const configurationRows = [
                'mode', 'scenario-class', 'behavior', 'observation-cue', 'scenario',
                'validation-state', 'validation-evidence', 'dt', 'softening',
                'coulomb-on', 'gravity-on', 'damping-on', 'exchange-on', 'strong-on',
                'lorentz-on', 'magdip-on', 'spinorbit-on', 'radiation-on',
                'rel-verlet-on', 'contact-on', 'g-pe', 'alpha-g-ee',
            ];
            return {
                sectionTitles: Array.from(document.querySelectorAll('#panel-diagnostics .diag-scale1-root .diag-section-title'))
                    .map((el) => el.textContent?.trim()),
                configurationRowsPresent: configurationRows.filter(row => document
                    .querySelector(`#panel-diagnostics .diag-scale1-root [data-row="${row}"]`)),
                provenanceSectionPresent: !!document.querySelector(
                    '#panel-diagnostics .diag-scale1-root [data-section="pe-provenance-contract"]',
                ),
                staticProvenanceRowsVisible: [
                    'owner', 'source-revision', 'artifact-revision', 'epistemic',
                    'qualification', 'identity-margin', 'graph-margin', 'energy-margin',
                    'center-observers', 'loss-ledger',
                ].filter(row => Array.from(document.querySelectorAll(
                    `#panel-diagnostics .diag-scale1-root [data-row="${row}"]`,
                )).some(element => element.checkVisibility?.())),
                unavailableClaims: value('unavailable'),
                rowTooltipCoverage: Array.from(document.querySelectorAll(
                    '#panel-diagnostics .diag-scale1-root .diag-data-row',
                )).every(row => row.dataset.uiTooltip?.length > 20
                    && !!row.dataset.uiTooltipSource),
                maxForce: value('max-force'),
                legacyPanelGone: !document.getElementById('pe-telemetry'),
            };
        });

        expect(diag.sectionTitles).toEqual(expect.arrayContaining([
            'State Energy & Coverage',
            'Conservation',
            'Forces & Geometry',
        ]));
        expect(diag.sectionTitles).not.toContain('Scenario Dynamics');
        expect(diag.sectionTitles).not.toContain('FTD Provenance Contract');
        expect(diag.configurationRowsPresent).toEqual([]);
        expect(diag.provenanceSectionPresent).toBe(false);
        expect(diag.staticProvenanceRowsVisible).toEqual([]);
        expect(diag.unavailableClaims).toBeTruthy();
        expect(diag.rowTooltipCoverage).toBe(true);
        expect(diag.maxForce).not.toBe('0');
        // Legacy PE telemetry canvas panel is retired — the descriptor tables
        // are the single Scale-1 diagnostics surface.
        expect(diag.legacyPanelGone).toBe(true);

        await page.locator(
            '#panel-diagnostics .diag-scale1-root .diag-data-row[data-row="missing-mask"]',
        ).hover();
        await expect(page.locator('#ui-tooltip')).toBeVisible();
        await expect(page.locator('#ui-tooltip')).toContainText(/Bit ?mask/);

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

    test('interaction hierarchy and particle log are independent, readable, unclipped full-height panels', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await page.setViewportSize({ width: 2560, height: 1440 });
        await runScale1(page, 's1-cluster-pair');
        await openPanel(page, 'interaction-hierarchy');

        await expect.poll(() => page.evaluate(() => Number(
            document.getElementById('interaction-hierarchy-particles')?.textContent || 0,
        )), { timeout: 10_000 }).toBeGreaterThan(1);

        const hierarchy = await page.evaluate(() => {
            const engineCount = window._ftdBridge?.peGetParticleData?.()?.count || 0;
            const panel = document.getElementById('panel-interaction-hierarchy');
            const shell = panel?.querySelector('.interaction-hierarchy-shell');
            const particleRows = panel?.querySelectorAll('[data-particle-id]').length || 0;
            const clusters = panel?.querySelectorAll('.particle-log-cluster').length || 0;
            const clipped = Array.from(panel?.querySelectorAll('.particle-log-hierarchy-card, .particle-log-cluster') || [])
                .filter(el => {
                    const style = getComputedStyle(el);
                    return (style.overflowY === 'hidden' || style.overflow === 'hidden') &&
                        el.scrollHeight > el.clientHeight + 1;
                }).length;
            return {
                activePanel: document.querySelector('.panel.active')?.id,
                engineCount,
                particleRows,
                clusters,
                anchor: document.getElementById('interaction-hierarchy-anchor')?.textContent?.trim(),
                explanation: panel?.querySelector('.particle-log-explainer')?.textContent?.trim(),
                panelOverflow: panel ? getComputedStyle(panel).overflowY : '',
                shellOverflow: shell ? getComputedStyle(shell).overflowY : '',
                shellHeight: shell?.getBoundingClientRect().height || 0,
                panelHeight: panel?.getBoundingClientRect().height || 0,
                clipped,
            };
        });

        expect(hierarchy.activePanel).toBe('panel-interaction-hierarchy');
        expect(hierarchy.particleRows).toBe(hierarchy.engineCount);
        expect(hierarchy.clusters).toBeGreaterThan(0);
        expect(hierarchy.anchor).toMatch(/^#\d+$/);
        expect(hierarchy.explanation).toContain('never alter dynamics');
        expect(['auto', 'scroll']).toContain(hierarchy.panelOverflow);
        expect(['auto', 'scroll']).toContain(hierarchy.shellOverflow);
        expect(Math.abs(hierarchy.shellHeight - hierarchy.panelHeight)).toBeLessThanOrEqual(2);
        expect(hierarchy.clipped).toBe(0);

        await page.locator('#interaction-hierarchy-expand').click();
        await expect(page.locator('#panel-interaction-hierarchy .particle-log-cluster[open]')).toHaveCount(0);
        await page.locator('#interaction-hierarchy-expand').click();
        await expect(page.locator('#panel-interaction-hierarchy .particle-log-cluster[open]').first()).toBeVisible();

        const floatedHierarchy = await page.evaluate(() => {
            const dock = window.__ftdCtx?.appShell?.panelDock;
            const win = dock?.floatPanel?.('interaction-hierarchy', 120, 80);
            if (!win?.el) return null;
            const body = win.el.querySelector('.floating-window-body');
            const panel = win.el.querySelector('#panel-interaction-hierarchy');
            const shell = panel?.querySelector('.interaction-hierarchy-shell');
            const report = {
                inWindow: !!panel,
                bodyOverflow: body ? getComputedStyle(body).overflowY : '',
                panelOverflow: panel ? getComputedStyle(panel).overflowY : '',
                shellOverflow: shell ? getComputedStyle(shell).overflowY : '',
                panelMinHeight: panel ? getComputedStyle(panel).minHeight : '',
                hiddenOverflowCount: Array.from(panel?.querySelectorAll('*') || []).filter(el => {
                    const style = getComputedStyle(el);
                    return (style.overflowY === 'hidden' || style.overflow === 'hidden') &&
                        el.scrollHeight > el.clientHeight + 1;
                }).length,
            };
            win.dock();
            return report;
        });
        expect(floatedHierarchy).not.toBeNull();
        expect(floatedHierarchy?.inWindow).toBe(true);
        expect(['auto', 'scroll']).toContain(floatedHierarchy?.bodyOverflow);
        expect(floatedHierarchy?.panelOverflow).not.toBe('hidden');
        expect(floatedHierarchy?.shellOverflow).not.toBe('hidden');
        expect(floatedHierarchy?.panelMinHeight).toBe('100%');
        expect(floatedHierarchy?.hiddenOverflowCount).toBe(0);

        await openPanel(page, 'particle-log');
        await expect.poll(() => page.evaluate(() => Number(
            document.getElementById('particle-log-events')?.textContent || 0,
        )), { timeout: 10_000 }).toBeGreaterThan(0);

        const initial = await page.evaluate(() => {
            const panel = document.getElementById('panel-particle-log');
            const shell = panel?.querySelector('.particle-log-shell');
            const lifecycle = document.querySelectorAll(
                '#panel-particle-log [data-event-category="lifecycle"]',
            ).length;
            const categoryIds = Array.from(document.querySelectorAll(
                '#panel-particle-log [data-log-category]',
            )).map(input => input.dataset.logCategory);
            return {
                activePanel: document.querySelector('.panel.active')?.id,
                lifecycle,
                categoryIds,
                hierarchyInLog: !!panel?.querySelector('[data-particle-id], .particle-log-cluster'),
                explanation: panel?.querySelector('.particle-log-explainer')?.textContent?.trim(),
                panelOverflow: panel ? getComputedStyle(panel).overflowY : '',
                shellOverflow: shell ? getComputedStyle(shell).overflowY : '',
                eventListOverflow: getComputedStyle(document.getElementById('particle-log-event-list')).overflowY,
                eventListMaxHeight: getComputedStyle(document.getElementById('particle-log-event-list')).maxHeight,
                eventHeadFont: parseFloat(getComputedStyle(panel?.querySelector('.particle-log-event-head')).fontSize),
                eventTitleFont: parseFloat(getComputedStyle(panel?.querySelector('.particle-log-event-content > strong')).fontSize),
                eventDetailFont: parseFloat(getComputedStyle(panel?.querySelector('.particle-log-event-content p')).fontSize),
                eventSourceFont: parseFloat(getComputedStyle(panel?.querySelector('.particle-log-event-source')).fontSize),
                clipped: Array.from(panel?.querySelectorAll('.particle-log-events-card, .particle-log-event-list, .particle-log-event') || [])
                    .filter(el => {
                        const style = getComputedStyle(el);
                        return (style.overflowY === 'hidden' || style.overflow === 'hidden') &&
                            el.scrollHeight > el.clientHeight + 1;
                    }).length,
            };
        });

        expect(initial.activePanel).toBe('panel-particle-log');
        expect(initial.lifecycle).toBeGreaterThan(0);
        expect(initial.hierarchyInLog).toBe(false);
        expect(initial.categoryIds).toEqual(expect.arrayContaining([
            'lifecycle', 'energy', 'hierarchy', 'interaction', 'environment',
        ]));
        expect(initial.explanation).toContain('do not feed the force solver');
        expect(['auto', 'scroll']).toContain(initial.panelOverflow);
        expect(['auto', 'scroll']).toContain(initial.shellOverflow);
        expect(initial.eventListOverflow).toBe('visible');
        expect(initial.eventListMaxHeight).toBe('none');
        expect(initial.eventHeadFont).toBeGreaterThanOrEqual(11);
        expect(initial.eventTitleFont).toBeGreaterThanOrEqual(13);
        expect(initial.eventDetailFont).toBeGreaterThanOrEqual(12);
        expect(initial.eventSourceFont).toBeGreaterThanOrEqual(10);
        expect(initial.clipped).toBe(0);

        await page.locator('[data-log-category="lifecycle"]').uncheck();
        await expect(page.locator(
            '#panel-particle-log [data-event-category="lifecycle"]',
        )).toHaveCount(0);
        await expect(page.locator(
            '#panel-particle-log [data-event-category="hierarchy"]',
        ).first()).toBeVisible();

        await page.locator('#particle-log-clear').click();
        await expect(page.locator('#particle-log-events')).toHaveText('0');

        // Native selected contact removals are ingested once and are not
        // duplicated as unexplained snapshot-diff despawns.
        await page.locator('[data-log-category="lifecycle"]').check();
        await selectPEScenario(page, 's1-contact-selection');
        await openPanel(page, 'particle-log');
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });
        await expect.poll(() => page.evaluate(() => Array.from(
            document.querySelectorAll('#particle-log-event-list .particle-log-event strong'),
        ).filter(el => el.textContent?.includes('Selected contact removal')).length), {
            timeout: 10_000,
            message: 'native contact-removal events did not reach the particle ledger',
        }).toBeGreaterThan(0);
        const unexplainedRemoved = await page.evaluate(() => Array.from(
            document.querySelectorAll('#particle-log-event-list .particle-log-event strong'),
        ).filter(el => el.textContent?.includes('left the active record')).length);
        expect(unexplainedRemoved).toBe(0);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
