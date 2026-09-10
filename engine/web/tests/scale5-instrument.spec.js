// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

/**
 * Scale 5 (Cosmic) instrumentation build-out — one describe block per pass.
 *
 * This file starts with Pass B (Gas and SPH); Pass A appends its own
 * `test.describe` block below (Passes C and D append theirs later). Each
 * block is self-contained to its own pass's surfaces.
 *
 * Reaching the live CosmicMockBridge and forcing a headless-safe tick
 * follows the exact pattern established by scale5-gas-labs.spec.js (see
 * its header note): the headless pane runs no requestAnimationFrame, so
 * physics is driven directly via `bridge.tick()` in-page, and panel DOM is
 * synced by calling the exported `animateCosmic(ctx)` once afterward. The
 * Diagnostics panel additionally needs `diagnosticsPanel.update(true)`
 * (app.js's own rAF loop is what normally calls `diagnosticsPanel.update()`
 * on a cadence gated by panel visibility; `force=true` bypasses that gate
 * so a single explicit call is enough in a test).
 */

/** @param {import('@playwright/test').Page} page @param {string} id */
async function selectCosmicScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-scenario-select'));
        if (!sel) throw new Error('cosmic-scenario-select not found');
        if (![...sel.options].some((option) => option.value === scenarioId)) {
            throw new Error(`scenario ${scenarioId} missing from #cosmic-scenario-select`);
        }
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await page.waitForFunction(
        (scenarioId) => document.getElementById('cosmic-scenario-select')?.value === scenarioId,
        id,
    );
}

/**
 * Tick the live cosmic bridge `nTicks` times in-page, force one
 * `animateCosmic` pass (drives telemetryHub.collectScale5 + the renderer +
 * the toolbar/panel status caches), then force the Diagnostics panel to
 * redraw regardless of its own visibility/cadence gating.
 * @param {import('@playwright/test').Page} page
 * @param {number} nTicks
 */
async function tickAndRefresh(page, nTicks = 30) {
    await page.evaluate(async (n) => {
        const bridge = window.__ftdCtx?.inspector?.bridge;
        if (!bridge) throw new Error('no cosmic bridge at window.__ftdCtx.inspector.bridge');
        for (let i = 0; i < n; i++) bridge.tick();
        const mod = await import('/js/scales/scale5/controller.js');
        mod.animateCosmic(window.__ftdCtx);
        window.__ftdCtx?.diagnosticsPanel?.update(true);
    }, nTicks);
}

/** @param {import('@playwright/test').Page} page @param {string} panel */
async function openPanel(page, panel) {
    await page.evaluate((panelId) => {
        const tab = document.querySelector(`#tab-bar .tab[data-panel="${panelId}"]`);
        if (!tab) throw new Error(`missing tab: ${panelId}`);
        tab.click();
    }, panel);
    await page.waitForTimeout(300);
}

test.describe('Pass B: Gas and SPH', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
    });

    test('cosmic-gas diagnostics section mounts while SPH runs and hides when it does not', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await openPanel(page, 'diagnostics');

        // Gas-lab scenario: sph_monaghan on by default (scale5-gas-labs.spec.js).
        await selectCosmicScenario(page, 'cosmic-gas-collapse');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 60);

        const on = await page.evaluate(() => {
            const section = document.querySelector('#panel-diagnostics .diag-scale5-root [data-section="cosmic-gas"]');
            const cell = document.querySelector(
                '#panel-diagnostics .diag-scale5-root [data-section="cosmic-gas"] tr[data-row="gas-count"] .diag-value',
            );
            return {
                exists: !!section,
                display: section ? getComputedStyle(section).display : null,
                gasCount: cell ? Number(cell.textContent) : null,
            };
        });
        expect(on.exists, 'cosmic-gas section should exist in the Scale 5 diagnostics root').toBe(true);
        expect(on.display, 'cosmic-gas section should be visible while SPH is running').not.toBe('none');
        expect(on.gasCount, 'gas-count cell should read a positive body count').toBeGreaterThan(0);

        // SPH off on this scenario: section must hide again.
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const off = await page.evaluate(() => {
            const section = document.querySelector('#panel-diagnostics .diag-scale5-root [data-section="cosmic-gas"]');
            return section ? getComputedStyle(section).display : null;
        });
        expect(off, 'cosmic-gas section should hide once SPH is off').toBe('none');

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('sph_monaghan and legacy_gas_repulsion round-trip through the bridge; sph_monaghan mirrors the toolbar', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);

        const before = await page.evaluate(() => {
            const bridge = window.__ftdCtx?.inspector?.bridge;
            return {
                sph: bridge?.getToggle('sph_monaghan'),
                legacy: bridge?.getToggle('legacy_gas_repulsion'),
            };
        });
        // sph_monaghan always starts off on a non-gas-lab scenario
        // (_syncRuleTogglesFromScenario() never touches it). legacy_gas_repulsion
        // is scenario-conditional (tracks `subgrid`), so this only records the
        // starting value rather than asserting a specific one.
        expect(before.sph, 'sph_monaghan should default off on cosmic-galaxy').toBe(false);

        const afterSphOn = await page.evaluate(() => {
            const panelCb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-gas-sph'));
            if (!panelCb) throw new Error('#cosmic-gas-sph not found');
            panelCb.checked = true;
            panelCb.dispatchEvent(new Event('change', { bubbles: true }));
            const bridge = window.__ftdCtx?.inspector?.bridge;
            const toolbarCb = /** @type {HTMLInputElement|null} */ (document.getElementById('t-sph-monaghan'));
            return { bridgeValue: bridge?.getToggle('sph_monaghan'), toolbarChecked: toolbarCb?.checked };
        });
        expect(afterSphOn.bridgeValue, 'bridge should see sph_monaghan=true after the Gas-card checkbox changes').toBe(true);
        expect(afterSphOn.toolbarChecked, "toolbar's #t-sph-monaghan should mirror the Gas-card checkbox").toBe(true);

        // Reverse via the TOOLBAR checkbox this time, confirming reconciliation runs both directions.
        const afterSphOff = await page.evaluate(() => {
            const toolbarCb = /** @type {HTMLInputElement|null} */ (document.getElementById('t-sph-monaghan'));
            if (!toolbarCb) throw new Error('#t-sph-monaghan not found');
            toolbarCb.checked = false;
            toolbarCb.dispatchEvent(new Event('change', { bubbles: true }));
            const bridge = window.__ftdCtx?.inspector?.bridge;
            const panelCb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-gas-sph'));
            return { bridgeValue: bridge?.getToggle('sph_monaghan'), panelChecked: panelCb?.checked };
        });
        expect(afterSphOff.bridgeValue, 'bridge should see sph_monaghan=false after the toolbar checkbox changes').toBe(false);
        expect(afterSphOff.panelChecked, "Gas-card's #cosmic-gas-sph should mirror the toolbar checkbox").toBe(false);

        const target = !before.legacy;
        const afterLegacyFlip = await page.evaluate((next) => {
            const panelCb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-gas-legacy-repulsion'));
            if (!panelCb) throw new Error('#cosmic-gas-legacy-repulsion not found');
            panelCb.checked = next;
            panelCb.dispatchEvent(new Event('change', { bubbles: true }));
            const bridge = window.__ftdCtx?.inspector?.bridge;
            return bridge?.getToggle('legacy_gas_repulsion');
        }, target);
        expect(afterLegacyFlip, `bridge should see legacy_gas_repulsion=${target} after the Gas-card checkbox changes`).toBe(target);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('overlay colour-by selector cycles through every option without console errors', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        // Gas-lab scenario so the star AND gas clouds are both populated
        // (colour-by applies to both, Ruling J3) and so the smoothing-circle
        // cloud below actually has gas bodies to draw.
        await selectCosmicScenario(page, 'cosmic-gas-collapse');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        for (const mode of ['density', 'temperature', 'speed', 'none']) {
            const applied = await page.evaluate((value) => {
                const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-overlay-colorby'));
                if (!sel) throw new Error('#cosmic-overlay-colorby not found');
                sel.value = value;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
                return sel.value;
            }, mode);
            expect(applied).toBe(mode);
            await tickAndRefresh(page, 5);
        }

        // Smoothing-length circles: a separate WebGL code path (its own
        // ShaderMaterial, compiled lazily on first use) that the colour-by
        // loop above never exercises.
        const circlesChecked = await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-overlay-smoothing-circles'));
            if (!cb) throw new Error('#cosmic-overlay-smoothing-circles not found');
            cb.checked = true;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
            return cb.checked;
        });
        expect(circlesChecked).toBe(true);
        await tickAndRefresh(page, 10);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });
});

test.describe('Pass A: Gravity and dynamics', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
    });

    test('cosmic-dynamics diagnostics rows populate; the gas smoothing-length rows carry the corrected tag (Ruling B-M1)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await openPanel(page, 'diagnostics');

        // A multi-body, non-gas scenario so system-radius/momentum/etc. are
        // all non-trivial and the direct-sum solver runs (well under the
        // BH_N_THRESHOLD=3000 gate), then a gas-lab scenario so the
        // cosmic-gas section (and its corrected h-row tags) is present too.
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 30);

        const dynamics = await page.evaluate(() => {
            const read = (rowId) => {
                const cell = document.querySelector(
                    `#panel-diagnostics .diag-scale5-root [data-section="cosmic-dynamics"] tr[data-row="${rowId}"] .diag-value`,
                );
                return cell ? Number(cell.textContent) : null;
            };
            return {
                systemRadius: read('system-radius'),
                speedLimitClamps: read('speed-limit-clamps'),
                speedLimitMaxFactor: read('speed-limit-max-factor'),
                bodiesCulled: read('bodies-culled'),
            };
        });
        expect(dynamics.systemRadius, 'system-radius should read a positive extent for a multi-body scenario')
            .toBeGreaterThan(0);
        // Clamp/cull counters are legitimately 0 on a quiet run — assert
        // they render as finite numbers (never NaN-as-text, never null),
        // not that clamping/culling actually fired this run.
        expect(Number.isFinite(dynamics.speedLimitClamps), 'speed-limit-clamps should render a finite count').toBe(true);
        expect(Number.isFinite(dynamics.speedLimitMaxFactor), 'speed-limit-max-factor should render a finite value').toBe(true);
        expect(Number.isFinite(dynamics.bodiesCulled), 'bodies-culled should render a finite count').toBe(true);

        await selectCosmicScenario(page, 'cosmic-gas-collapse');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 60);

        const hRowTooltip = await page.evaluate(() => {
            const row = document.querySelector(
                '#panel-diagnostics .diag-scale5-root [data-section="cosmic-gas"] tr[data-row="gas-h-min"]',
            );
            return row ? row.dataset.uiTooltip : null;
        });
        expect(hRowTooltip, 'gas-h-min tooltip should exist').toBeTruthy();
        expect(hRowTooltip).toContain('[MEASURED');
        expect(hRowTooltip).not.toContain('[IMPOSED');

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('Dynamics card sliders drive the bridge live setters; speed_limit checkbox is this pass\'s one toggle wiring', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);

        // speed_limit has no prior UI surface — SCALE5_TOGGLES defaults it
        // to true, and the bridge's own _toggles seed agrees.
        const before = await page.evaluate(() => {
            const bridge = window.__ftdCtx?.inspector?.bridge;
            return bridge?.getToggle('speed_limit');
        });
        expect(before, 'speed_limit should default on').toBe(true);

        const afterToggleOff = await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-dynamics-speed-limit'));
            if (!cb) throw new Error('#cosmic-dynamics-speed-limit not found');
            cb.checked = false;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
            const bridge = window.__ftdCtx?.inspector?.bridge;
            return bridge?.getToggle('speed_limit');
        });
        expect(afterToggleOff, 'bridge should see speed_limit=false after the Dynamics-card checkbox changes').toBe(false);

        const sliderResult = await page.evaluate(() => {
            /** @param {string} id @param {string} value */
            const setSlider = (id, value) => {
                const el = /** @type {HTMLInputElement|null} */ (document.getElementById(id));
                if (!el) throw new Error(`#${id} not found`);
                el.value = value;
                el.dispatchEvent(new Event('input', { bubbles: true }));
            };
            setSlider('cosmic-dynamics-gravity', '2.5');
            setSlider('cosmic-dynamics-softening', '0.4');
            setSlider('cosmic-dynamics-dt', '0.025');
            setSlider('cosmic-dynamics-speed-limit-factor', '1.75');

            const bridge = window.__ftdCtx?.inspector?.bridge;
            const params = bridge?.getRuntimeParams?.();
            return {
                gravityScale: params?.gravityScale,
                softeningScale: params?.softeningScale,
                dt: params?.dt,
                speedLimitFactor: params?.speedLimitFactor,
                gravityLabel: document.getElementById('cosmic-dynamics-gravity-value')?.textContent,
                softeningLabel: document.getElementById('cosmic-dynamics-softening-value')?.textContent,
                dtLabel: document.getElementById('cosmic-dynamics-dt-value')?.textContent,
                speedLimitFactorLabel: document.getElementById('cosmic-dynamics-speed-limit-factor-value')?.textContent,
            };
        });
        expect(sliderResult.gravityScale).toBeCloseTo(2.5, 5);
        expect(sliderResult.softeningScale).toBeCloseTo(0.4, 5);
        expect(sliderResult.dt).toBeCloseTo(0.025, 5);
        expect(sliderResult.speedLimitFactor).toBeCloseTo(1.75, 5);
        expect(sliderResult.gravityLabel).toBe('2.50');
        expect(sliderResult.softeningLabel).toBe('0.40');
        expect(sliderResult.dtLabel).toBe('0.025');
        expect(sliderResult.speedLimitFactorLabel).toBe('1.75');

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('overlay velocity-vector and centre-of-mass-marker toggles run without console errors', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const velocityChecked = await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-overlay-velocity-vectors'));
            if (!cb) throw new Error('#cosmic-overlay-velocity-vectors not found');
            cb.checked = true;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
            return cb.checked;
        });
        expect(velocityChecked).toBe(true);
        await tickAndRefresh(page, 10);

        const comChecked = await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-overlay-com-marker'));
            if (!cb) throw new Error('#cosmic-overlay-com-marker not found');
            cb.checked = true;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
            return cb.checked;
        });
        expect(comChecked).toBe(true);
        await tickAndRefresh(page, 10);

        // Switch scenario while both overlays are on — the renderer is
        // recreated (Ruling P1); syncScale5Overlays must re-apply the
        // pending state to the fresh renderer without throwing.
        await selectCosmicScenario(page, 'cosmic-merger');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });
});
