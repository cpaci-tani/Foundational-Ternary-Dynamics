// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

/**
 * Scale 5 (Cosmic) instrumentation build-out — one describe block per pass.
 *
 * This file starts with Pass B (Gas and SPH); Pass A and Pass C append their
 * own `test.describe` blocks below (Pass D appends its own later). Each
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

test.describe('Pass C: Cosmology and expansion', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
    });

    test('cosmic-expansion diagnostics section reads live cosmology fields, the scale factor advances with ticks, and the Omega_Lambda/DM-fraction tooltips carry their epistemic caveats', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await openPanel(page, 'diagnostics');

        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 30);

        const readRow = async (rowId) => page.evaluate((id) => {
            const cell = document.querySelector(
                `#panel-diagnostics .diag-scale5-root [data-section="cosmic-expansion"] tr[data-row="${id}"] .diag-value`,
            );
            return cell ? Number(cell.textContent) : null;
        }, rowId);

        const scaleFactorBefore = await readRow('scale-factor');
        expect(scaleFactorBefore, 'scale-factor should render a finite positive value').toBeGreaterThan(0);
        expect(await readRow('hubble'), 'hubble row should render a finite positive rate').toBeGreaterThan(0);
        expect(await readRow('redshift'), 'redshift row should render a finite value').not.toBeNull();
        expect(await readRow('hubble0'), 'hubble0 anchor row should render a finite positive value').toBeGreaterThan(0);
        expect(await readRow('omega-m'), 'omega-m row should render 1/3').toBeCloseTo(1 / 3, 5);
        expect(await readRow('omega-l'), 'omega-l row should render 2/3').toBeCloseTo(2 / 3, 5);
        expect(await readRow('dm-fraction'), 'dm-fraction row should render a percent in (0, 100]').toBeGreaterThan(0);
        expect(await readRow('clock-gain'), 'clock-gain row should render the default gain (40)').toBeCloseTo(40, 5);
        const boxBefore = await readRow('box-comoving');
        expect(boxBefore, 'box-comoving row should render a finite positive size').toBeGreaterThan(0);

        // Expansion is on by default: advancing ticks should grow both the
        // scale factor and (by construction, boxComoving = boxSize * a) the
        // comoving box size row beside it.
        await tickAndRefresh(page, 120);
        const scaleFactorAfter = await readRow('scale-factor');
        const boxAfter = await readRow('box-comoving');
        expect(scaleFactorAfter, 'scale factor should have advanced after 120 more ticks with expansion on')
            .toBeGreaterThan(scaleFactorBefore);
        expect(boxAfter, 'comoving box size should have grown along with the scale factor')
            .toBeGreaterThan(boxBefore);

        // Ruling C-1: the plan's instruction to replace the hard-coded Omega/
        // DM strings in the "Cosmology (FTD)" info card with live readouts
        // was overridden — those tooltips carry load-bearing epistemic
        // caveats this diagnostics section restates rather than replaces.
        // Confirm BOTH surfaces still carry the caveats: the pre-existing
        // static card (untouched by this pass) and this pass's own new rows.
        // Scoped to the pre-existing STATIC "Cosmology (FTD)" info card by
        // its card-title text, not just its `.scale-info-copy` class — this
        // pass's own new Cosmology CONTROLS card also uses that class for
        // an unrelated note, and a class-only selector would conflate them.
        // Note: ui/components/tooltips/component.js hoists every `title=`
        // attribute into `data-ui-tooltip` (removing `title` itself) on a
        // global pass, so by the time the page has settled the caveat text
        // lives in `data-ui-tooltip`, not `title`.
        const staticCardTooltips = await page.evaluate(() => {
            const cards = document.querySelectorAll('#panel-controls-grid-scale5 .card');
            const ftdCard = Array.from(cards).find((c) => c.querySelector('.card-title')?.textContent === 'Cosmology (FTD)');
            const cells = ftdCard ? ftdCard.querySelectorAll('.scale-info-copy > div[data-ui-tooltip]') : [];
            return Array.from(cells).map((el) => el.dataset.uiTooltip);
        });
        expect(staticCardTooltips.some((t) => /does NOT match the observed/i.test(t)), 'static Cosmology (FTD) card should still carry the Omega_Lambda mismatch caveat').toBe(true);
        expect(staticCardTooltips.some((t) => /RETIRED/i.test(t) && /FTD-0131/i.test(t)), 'static Cosmology (FTD) card should still carry the G_N RETIRED caveat').toBe(true);
        expect(staticCardTooltips.some((t) => /does NOT match Planck 2018/i.test(t)), 'static Cosmology (FTD) card should still carry the DM-fraction Planck mismatch caveat').toBe(true);

        const omegaLRowTooltip = await page.evaluate(() => {
            const row = document.querySelector(
                '#panel-diagnostics .diag-scale5-root [data-section="cosmic-expansion"] tr[data-row="omega-l"]',
            );
            return row ? row.dataset.uiTooltip : null;
        });
        expect(omegaLRowTooltip, 'omega-l row tooltip should exist').toBeTruthy();
        expect(omegaLRowTooltip).toContain('[CONJECTURE]');
        expect(omegaLRowTooltip).toContain('NOT match the observed');

        const dmRowTooltip = await page.evaluate(() => {
            const row = document.querySelector(
                '#panel-diagnostics .diag-scale5-root [data-section="cosmic-expansion"] tr[data-row="dm-fraction"]',
            );
            return row ? row.dataset.uiTooltip : null;
        });
        expect(dmRowTooltip, 'dm-fraction row tooltip should exist').toBeTruthy();
        expect(dmRowTooltip).toContain('[SELECTION]');
        expect(dmRowTooltip).toContain('NOT match Planck 2018');

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('Cosmology card: expansion toggle freezes a(t); clock-gain slider drives the bridge; DM-fraction select reseeds without touching the scenario dropdown', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 20);

        // Expansion toggle: off should freeze a(t) exactly (not merely slow
        // its growth) since _stepFriedmann is skipped entirely while off.
        // The app runs a real background rAF loop even in this "headless"
        // config (ctx.running is true after scenario load) — reading the
        // pre-toggle value and flipping the checkbox must happen in the
        // SAME page.evaluate() call, or a background tick can land in the
        // gap between two separate round trips and make `beforeFreeze`
        // stale relative to when the freeze actually takes effect.
        const beforeFreeze = await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-cosmology-expansion'));
            if (!cb) throw new Error('#cosmic-cosmology-expansion not found');
            const a = window.__ftdCtx?.inspector?.bridge?.getDiagnostics()?.scaleFactor;
            cb.checked = false;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
            return a;
        });
        await tickAndRefresh(page, 30);
        const afterFreeze = await page.evaluate(() => window.__ftdCtx?.inspector?.bridge?.getDiagnostics()?.scaleFactor);
        expect(afterFreeze, 'scale factor should be frozen exactly once expansion is toggled off').toBe(beforeFreeze);

        // Re-enable so the rest of the run reflects normal behaviour.
        await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-cosmology-expansion'));
            cb.checked = true;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
        });

        // Clock-gain slider: a bridge live setter, no second UI surface.
        const clockGainResult = await page.evaluate(() => {
            const el = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-cosmology-clock-gain'));
            if (!el) throw new Error('#cosmic-cosmology-clock-gain not found');
            el.value = '90';
            el.dispatchEvent(new Event('input', { bubbles: true }));
            const bridge = window.__ftdCtx?.inspector?.bridge;
            return {
                clockGain: bridge?.getRuntimeParams?.().clockGain,
                label: document.getElementById('cosmic-cosmology-clock-gain-value')?.textContent,
            };
        });
        expect(clockGainResult.clockGain).toBeCloseTo(90, 5);
        expect(clockGainResult.label).toBe('90');

        // DM-fraction pre-load select: reseeds the CURRENT scenario (the
        // bridge instance is recreated) without touching the scenario
        // dropdown's own value or its option list (constraint: this is not
        // a scenario option, and scientific-scenario-inventory.spec.js pins
        // the 16 Scale-5 ids).
        const scenarioBefore = await page.evaluate(() => document.getElementById('cosmic-scenario-select')?.value);
        const dmResult = await page.evaluate(() => {
            const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-cosmology-dm-fraction'));
            if (!sel) throw new Error('#cosmic-cosmology-dm-fraction not found');
            sel.value = '0.84';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
            const bridge = window.__ftdCtx?.inspector?.bridge;
            return {
                override: bridge?._dmFractionOverride,
                scenario: bridge?._scenarioName,
                scenarioSelectValue: document.getElementById('cosmic-scenario-select')?.value,
            };
        });
        expect(dmResult.override).toBeCloseTo(0.84, 5);
        expect(dmResult.scenario, 'DM-fraction change should reload the SAME scenario, not switch to a different one')
            .toBe(scenarioBefore);
        expect(dmResult.scenarioSelectValue, "the scenario dropdown's own value must not change")
            .toBe(scenarioBefore);

        // Review finding (Important): the dm-fraction tooltip must stay true
        // even with the Planck override active, when the displayed row reads
        // ~84% — the exact figure the OLD tooltip wording claimed the value
        // "does NOT match". The fix attributes the mismatch to FTD's native
        // 17/27 default rather than to the displayed number, so it holds in
        // every override state. Confirm the tooltip text and tag survive
        // unchanged with the override live, and that the row itself now
        // reads close to 84% (proving this is exactly the state that broke
        // the old wording).
        await openPanel(page, 'diagnostics');
        await tickAndRefresh(page, 10);
        const dmRowTooltipWithOverride = await page.evaluate(() => {
            const row = document.querySelector(
                '#panel-diagnostics .diag-scale5-root [data-section="cosmic-expansion"] tr[data-row="dm-fraction"]',
            );
            const cell = row?.querySelector('.diag-value');
            return { tooltip: row ? row.dataset.uiTooltip : null, value: cell ? Number(cell.textContent) : null };
        });
        expect(dmRowTooltipWithOverride.value, 'dm-fraction row should read well above the ~63% native default with the 0.84 Planck override active')
            .toBeGreaterThan(75);
        expect(dmRowTooltipWithOverride.tooltip, 'dm-fraction row tooltip should exist with the override active').toBeTruthy();
        expect(dmRowTooltipWithOverride.tooltip).toContain('[SELECTION]');
        expect(dmRowTooltipWithOverride.tooltip).toContain('17/27');
        expect(dmRowTooltipWithOverride.tooltip).toContain('NOT match Planck 2018');
        expect(dmRowTooltipWithOverride.tooltip, "must attribute the mismatch to FTD's native default, not to the displayed value")
            .toContain("FTD's native dark-matter fraction");

        // Switching back to "default" must restore the unset-override
        // behaviour (?? DM_FRACTION), not merely stop reflecting 0.84.
        const backToDefault = await page.evaluate(() => {
            const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-cosmology-dm-fraction'));
            sel.value = 'default';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
            return window.__ftdCtx?.inspector?.bridge?._dmFractionOverride;
        });
        expect(backToDefault).toBeNull();

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('comoving reference-grid overlay toggle runs without console errors and survives a scenario switch', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const gridChecked = await page.evaluate(() => {
            const cb = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-overlay-comoving-grid'));
            if (!cb) throw new Error('#cosmic-overlay-comoving-grid not found');
            cb.checked = true;
            cb.dispatchEvent(new Event('change', { bubbles: true }));
            return cb.checked;
        });
        expect(gridChecked).toBe(true);
        await tickAndRefresh(page, 10);

        // Ruling P1 hazard: the renderer is recreated on every scenario
        // load, so syncScale5Overlays must re-apply the pending comoving-
        // grid state to the FRESH renderer without throwing.
        await selectCosmicScenario(page, 'cosmic-merger');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });
});

test.describe('Pass D: Stellar, events, and camera', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(60_000);
    });

    test('Ruling D-1 defect fix: emergent_black_holes no longer fires on a gas laboratory past its known tick-2536 conversion point', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-gas-cloud-collision');
        await page.waitForTimeout(300);

        // Drive the live bridge directly (not via tickAndRefresh's small
        // batches) past the documented tick-2536 conversion point. Before
        // the fix, a body on this exact scenario converted to BLACK_HOLE at
        // tick 2536 then swallowed two neighbours; after the fix
        // (emergent_black_holes AND-gated with _enableSubgrid, which the
        // three gas laboratories set false) it must never convert.
        const result = await page.evaluate(() => {
            const bridge = window.__ftdCtx?.inspector?.bridge;
            if (!bridge) throw new Error('no cosmic bridge at window.__ftdCtx.inspector.bridge');
            const TYPE = bridge.constructor.TYPE;
            const initialCount = bridge._bodies.length;
            // The live app may already be auto-ticking the bridge in the
            // background (rAF physics cadence) during the waitForTimeout
            // calls above, so bridge._tick is not necessarily 0 here --
            // measure ticks actually run by this loop as a delta, not as
            // an absolute bridge._tick value.
            const startTick = bridge._tick;
            let conversionTick = null;
            let iterationsRun = 0;
            for (let i = 1; i <= 2600; i++) {
                bridge.tick();
                iterationsRun++;
                const hasBH = bridge._bodies.some((b) => b.type === TYPE.BLACK_HOLE || b.type === TYPE.QUASAR);
                if (hasBH && conversionTick === null) { conversionTick = bridge._tick; break; }
            }
            return {
                enableSubgrid: bridge._enableSubgrid,
                initialCount,
                finalCount: bridge._bodies.length,
                startTick,
                endTick: bridge._tick,
                iterationsRun,
                conversionTick,
            };
        });

        expect(result.enableSubgrid, 'the gas-cloud-collision lab should have _enableSubgrid = false').toBe(false);
        expect(result.conversionTick, 'no body should convert to BLACK_HOLE/QUASAR past tick 2536 on this lab after the fix').toBeNull();
        expect(result.iterationsRun, 'the loop should run all 2600 iterations, i.e. never break early on a conversion').toBe(2600);
        expect(result.endTick - result.startTick, 'exactly 2600 physics ticks should have been driven by this loop').toBe(2600);
        expect(result.endTick, 'the bridge should have ticked well past the documented tick-2536 conversion point').toBeGreaterThan(2536);
        expect(result.finalCount, 'body count should stay at its initial 500 with no conversion/swallowing').toBe(result.initialCount);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('cosmic-population diagnostics section renders the full nine-type census', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await openPanel(page, 'diagnostics');

        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 30);

        const rowIds = [
            'n-dark-energy', 'n-quasar', 'n-black-hole', 'n-dark-matter',
            'n-gas', 'n-star', 'n-neutron-star', 'n-nebula', 'n-white-dwarf',
        ];
        const counts = await page.evaluate((ids) => {
            return ids.map((id) => {
                const cell = document.querySelector(
                    `#panel-diagnostics .diag-scale5-root [data-section="cosmic-population"] tr[data-row="${id}"] .diag-value`,
                );
                return cell ? Number(cell.textContent) : null;
            });
        }, rowIds);

        for (let i = 0; i < rowIds.length; i++) {
            expect(Number.isFinite(counts[i]), `${rowIds[i]} should render a finite count`).toBe(true);
        }
        const starIdx = rowIds.indexOf('n-star');
        const dmIdx = rowIds.indexOf('n-dark-matter');
        expect(counts[starIdx], 'cosmic-galaxy should have a positive star count').toBeGreaterThan(0);
        expect(counts[dmIdx], 'cosmic-galaxy should have a positive dark-matter count').toBeGreaterThan(0);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('Physics Rules card: all eleven remaining toggles round-trip through the bridge', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);

        const idByKey = {
            gas_cooling: 'cosmic-rules-gas-cooling',
            radiation_pressure: 'cosmic-rules-radiation-pressure',
            tidal_stretch: 'cosmic-rules-tidal-stretch',
            tidal_disruption: 'cosmic-rules-tidal-disruption',
            star_formation: 'cosmic-rules-star-formation',
            bondi_accretion: 'cosmic-rules-bondi-accretion',
            horizon_absorption: 'cosmic-rules-horizon-absorption',
            mergers: 'cosmic-rules-mergers',
            emergent_black_holes: 'cosmic-rules-emergent-bh',
            stellar_evolution: 'cosmic-rules-stellar-evolution',
            hawking_evaporation: 'cosmic-rules-hawking-evaporation',
        };

        for (const [key, id] of Object.entries(idByKey)) {
            const result = await page.evaluate(([toggleKey, domId]) => {
                const cb = /** @type {HTMLInputElement|null} */ (document.getElementById(domId));
                if (!cb) return { error: `#${domId} not found` };
                const before = cb.checked;
                cb.checked = !before;
                cb.dispatchEvent(new Event('change', { bubbles: true }));
                const bridge = window.__ftdCtx?.inspector?.bridge;
                return { before, after: !before, bridgeValue: bridge?.getToggle(toggleKey) };
            }, [key, id]);
            expect(result.error, result.error).toBeUndefined();
            expect(result.bridgeValue, `bridge should see ${key}=${result.after} after #${id} changes`).toBe(result.after);
        }

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('Physics Rules card event log renders bounded entries from the bridge event ring', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        // A merger scenario reliably produces horizon-absorption/merger
        // events within a short tick budget.
        await selectCosmicScenario(page, 'cosmic-merger');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 200);

        const logState = await page.evaluate(() => {
            const bridge = window.__ftdCtx?.inspector?.bridge;
            const events = bridge?.getEventLog?.() || [];
            const list = document.getElementById('cosmic-event-log-list');
            return {
                bridgeEventCount: events.length,
                listItemCount: list ? list.querySelectorAll('.cosmic-event-log-item').length : null,
                hasEmptyMarker: list ? !!list.querySelector('.cosmic-event-log-empty') : null,
            };
        });
        expect(logState.listItemCount, '#cosmic-event-log-list should exist').not.toBeNull();
        if (logState.bridgeEventCount > 0) {
            expect(logState.listItemCount, 'event log list should render at least one row once the bridge has events').toBeGreaterThan(0);
            expect(logState.hasEmptyMarker, 'the empty-state marker should be gone once events exist').toBe(false);
        }

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('overlay Visibility, Black holes, and Trails & camera controls run without console errors and survive a scenario switch', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-galaxy');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const checkboxIds = [
            'cosmic-overlay-show-dm', 'cosmic-overlay-show-gas', 'cosmic-overlay-show-stars',
            'cosmic-overlay-show-bh', 'cosmic-overlay-show-disks',
            'cosmic-overlay-bh-markers', 'cosmic-overlay-accretion-markers',
            'cosmic-overlay-trails',
        ];
        for (const id of checkboxIds) {
            const checked = await page.evaluate((cbId) => {
                const cb = /** @type {HTMLInputElement|null} */ (document.getElementById(cbId));
                if (!cb) throw new Error(`#${cbId} not found`);
                cb.checked = !cb.checked;
                cb.dispatchEvent(new Event('change', { bubbles: true }));
                return cb.checked;
            }, id);
            expect(typeof checked).toBe('boolean');
        }
        await tickAndRefresh(page, 10);

        // 'Type' colour-by option (extends Pass B's existing select).
        const typeApplied = await page.evaluate(() => {
            const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-overlay-colorby'));
            if (!sel) throw new Error('#cosmic-overlay-colorby not found');
            sel.value = 'type';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
            return sel.value;
        });
        expect(typeApplied).toBe('type');
        await tickAndRefresh(page, 10);

        // Body-size-scale slider.
        const bodySizeResult = await page.evaluate(() => {
            const el = /** @type {HTMLInputElement|null} */ (document.getElementById('cosmic-overlay-body-size'));
            if (!el) throw new Error('#cosmic-overlay-body-size not found');
            el.value = '2.5';
            el.dispatchEvent(new Event('input', { bubbles: true }));
            return document.getElementById('cosmic-overlay-body-size-value')?.textContent;
        });
        expect(bodySizeResult).toBe('2.50');
        await tickAndRefresh(page, 10);

        // Renderer is recreated on scenario switch (Ruling P1 hazard) —
        // every pending overlay state above must re-apply without throwing.
        await selectCosmicScenario(page, 'cosmic-black-hole');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    test('camera: gas-lab preset and follow-a-body / centre-of-mass-lock options run without console errors', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        await selectCosmicScenario(page, 'cosmic-gas-collapse');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const setCamera = (value) => page.evaluate((v) => {
            const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-camera-select'));
            if (!sel) throw new Error('#cosmic-camera-select not found');
            if (![...sel.options].some((o) => o.value === v)) throw new Error(`camera option ${v} missing`);
            sel.value = v;
            sel.dispatchEvent(new Event('change', { bubbles: true }));
            return sel.value;
        }, value);

        expect(await setCamera('gaslab')).toBe('gaslab');
        await tickAndRefresh(page, 5);

        expect(await setCamera('com-lock')).toBe('com-lock');
        const posAfterEngage = await page.evaluate(() => {
            const p = window.__ftdCtx?.inspector?._cosmicRenderer?.camera?.position;
            return p ? { x: p.x, y: p.y, z: p.z } : null;
        });
        expect(posAfterEngage).not.toBeNull();
        await tickAndRefresh(page, 60);
        const posAfterTicks = await page.evaluate(() => {
            const p = window.__ftdCtx?.inspector?._cosmicRenderer?.camera?.position;
            return p ? { x: p.x, y: p.y, z: p.z } : null;
        });
        expect(posAfterTicks).not.toBeNull();

        expect(await setCamera('follow-heaviest')).toBe('follow-heaviest');
        await tickAndRefresh(page, 30);

        // A static preset must disengage follow mode without throwing.
        expect(await setCamera('overview')).toBe('overview');
        await tickAndRefresh(page, 10);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });

    // I1 fix verification: the two continuous camera-follow modes used to
    // set camera.position/lookAt() but never touch controls.target, so the
    // very next viewport.render() -> controls.update() (OrbitControls
    // unconditionally recomputes the camera's offset from controls.target
    // and re-runs lookAt(controls.target) every call) undid the follow
    // repositioning using the stale (origin) target. The console-errors-only
    // assertion above passed even with that defect live -- nothing threw,
    // the camera just silently pointed at the wrong place. This test reads
    // the actual camera/controls/body state to prove the follow modes work:
    // controls.target tracks the followed point as it moves, the camera's
    // offset from that target is held fixed (a genuine "follow", not a
    // teleport), interactive orbit is suspended while a follow mode is
    // active (per the toolbar's own tooltip claim), and everything restores
    // when a static preset disengages the follow.
    test('camera follow: controls.target tracks the followed body/COM and interactive orbit is genuinely suspended (I1)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);
        // A gravitationally-collapsing gas ball: individual body positions
        // (including whichever body is instantaneously heaviest) move
        // substantially over a few hundred ticks, unlike a scenario tuned
        // to sit near equilibrium.
        await selectCosmicScenario(page, 'cosmic-gas-collapse');
        await page.waitForTimeout(300);
        await tickAndRefresh(page, 10);

        const setCamera = (value) => page.evaluate((v) => {
            const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-camera-select'));
            if (!sel) throw new Error('#cosmic-camera-select not found');
            sel.value = v;
            sel.dispatchEvent(new Event('change', { bubbles: true }));
            return sel.value;
        }, value);

        /** Reads the current heaviest-body position straight from the live
         *  bridge, mirroring CosmicRenderer._findHeaviestBodyPosition's own
         *  O(N) max-mass scan, plus the renderer's camera/controls state. */
        const readState = () => page.evaluate(() => {
            const bridge = window.__ftdCtx?.inspector?.bridge;
            const renderer = window.__ftdCtx?.inspector?._cosmicRenderer;
            if (!bridge || !renderer) return null;
            let best = null, bestMass = 0;
            for (const b of bridge._bodies) {
                if (b.mass > bestMass) { bestMass = b.mass; best = b; }
            }
            const cam = renderer.camera.position;
            const target = renderer._controls?.target;
            return {
                heaviest: best ? { x: best.x, y: best.y, z: best.z } : null,
                camera: { x: cam.x, y: cam.y, z: cam.z },
                target: target ? { x: target.x, y: target.y, z: target.z } : null,
                controlsEnabled: renderer._controls?.enabled ?? null,
            };
        });

        const dist = (a, b) => Math.hypot(a.x - b.x, a.y - b.y, a.z - b.z);

        expect(await setCamera('follow-heaviest')).toBe('follow-heaviest');
        // One physics-frame tick so update() -> _applyCameraFollow() runs
        // at least once and captures the initial follow offset.
        await tickAndRefresh(page, 2);

        const s0 = await readState();
        expect(s0, 'renderer/bridge state should be available').not.toBeNull();
        expect(s0.heaviest, 'a heaviest body should exist').not.toBeNull();
        expect(s0.controlsEnabled, 'interactive orbit must be suspended while follow-heaviest is active').toBe(false);
        expect(dist(s0.target, s0.heaviest), 'controls.target should equal the heaviest body\'s position right after engaging').toBeLessThan(0.05);
        const offset0 = { x: s0.camera.x - s0.target.x, y: s0.camera.y - s0.target.y, z: s0.camera.z - s0.target.z };

        // Drive enough physics ticks for the collapsing gas ball to move
        // its heaviest body measurably.
        await tickAndRefresh(page, 400);

        const s1 = await readState();
        expect(s1.heaviest).not.toBeNull();
        const bodyMoved = dist(s0.heaviest, s1.heaviest);
        expect(bodyMoved, 'the heaviest body should have actually moved over 400 ticks (otherwise this test cannot distinguish follow from a static camera)').toBeGreaterThan(0.01);

        // The camera must still be pointed at (controls.target still equal
        // to) the CURRENT heaviest-body position, not the stale one from
        // when follow was engaged -- this is exactly the bug I1 describes:
        // without driving controls.target, the target stays at the origin
        // and controls.update() would drag the camera/orientation away from
        // the moving body every frame.
        expect(dist(s1.target, s1.heaviest), 'controls.target should still track the CURRENT heaviest-body position after ticking').toBeLessThan(0.05);
        expect(s1.controlsEnabled, 'interactive orbit should still be suspended while follow-heaviest remains active').toBe(false);

        // The camera keeps the SAME relative offset from its target that it
        // captured at engage time (a "follow" translates the camera with
        // the body; it does not re-frame to a canned view every tick).
        const offset1 = { x: s1.camera.x - s1.target.x, y: s1.camera.y - s1.target.y, z: s1.camera.z - s1.target.z };
        expect(dist(offset0, offset1), 'the camera-to-target offset should stay fixed while following (translation, not re-framing)').toBeLessThan(0.05);

        // Switching to a static preset must disengage the follow and
        // restore normal interactive orbit + the origin target every
        // static preset uses.
        expect(await setCamera('overview')).toBe('overview');
        await tickAndRefresh(page, 2);
        const s2 = await readState();
        expect(s2.controlsEnabled, 'a static preset must restore interactive orbit').toBe(true);
        expect(dist(s2.target, { x: 0, y: 0, z: 0 }), 'a static preset must restore the origin target').toBeLessThan(0.001);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });
});
