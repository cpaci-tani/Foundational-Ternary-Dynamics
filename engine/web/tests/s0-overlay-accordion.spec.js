// @ts-check
/**
 * Scale-0 Visualization panel — layer-inspector redesign (2026-08-29).
 *
 * Pins the three behaviours added when the always-open 6-column grid became a
 * compact collapsible accordion (see overlays/panel-shell.js):
 *   - progressive-disclosure accordion cards with independent persisted state;
 *   - a compact Active rail of removable chips derived from button state;
 *   - unified scalar/vector presentation controls;
 *   - filter/clear behavior and contextual subcontrols.
 * The toggle wiring itself is covered by toggle-coverage.spec.js and is unchanged.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const expand = (page) => page.evaluate(() => {
    const c = document.querySelector('#viewport-overlay .s0-overlay-collapse');
    if (c && c.getAttribute('aria-expanded') === 'false') c.click();
});

test.describe('Scale-0 Visualization accordion', () => {
    test('uses a centered one-column list, starts Organic off, and owns Flow from Scene', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(500);
        await expand(page);

        const result = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const panel = document.getElementById('viewport-overlay');
            const volume = panel.querySelector('[data-col="volume"]');
            const fields = panel.querySelector('[data-col="fields"]');
            if (fields.classList.contains('is-collapsed')) {
                fields.querySelector('.s0-overlay-col-head').click();
            }
            const layerButton = document.getElementById('toggle-e-field');
            const content = layerButton.querySelector('.s0-toggle-content');
            const swatch = layerButton.querySelector('.field-swatch');
            const organic = document.getElementById('toggle-flux-organic');
            const sceneFlow = document.getElementById('scene-force-flow');
            const forceRowFlow = document.querySelector('#force-style-row [data-style="flow"]');
            const before = {
                organicActive: organic.classList.contains('active'),
                organicPressed: organic.getAttribute('aria-pressed'),
                rendererOrganic: window.__ftdCtx.viewport._fluxRenderer._fluxOrganic,
                forceRowFlow: !!forceRowFlow,
                sceneFlow: !!sceneFlow,
                columns: getComputedStyle(volume).gridTemplateColumns.split(/\s+/).filter(Boolean).length,
                button: {
                    display: getComputedStyle(layerButton).display,
                    align: getComputedStyle(layerButton).alignItems,
                    justify: getComputedStyle(layerButton).justifyContent,
                    text: getComputedStyle(layerButton).textAlign,
                },
                contentTransform: getComputedStyle(content).transform,
                swatchBackground: getComputedStyle(swatch).backgroundImage !== 'none'
                    ? getComputedStyle(swatch).backgroundImage
                    : getComputedStyle(swatch).backgroundColor,
            };

            organic.click();
            const afterOrganic = {
                active: organic.classList.contains('active'),
                pressed: organic.getAttribute('aria-pressed'),
                rendererOrganic: window.__ftdCtx.viewport._fluxRenderer._fluxOrganic,
            };

            sceneFlow.click();
            const afterFlow = {
                style: getScale0State().forceStyle,
                active: sceneFlow.classList.contains('active'),
                pressed: sceneFlow.getAttribute('aria-pressed'),
                panelActive: [...document.querySelectorAll('#force-style-row .style-btn.active')]
                    .map((button) => button.dataset.style),
            };
            document.querySelector('#force-style-row [data-style="arrows"]').click();
            const afterArrows = {
                style: getScale0State().forceStyle,
                sceneActive: sceneFlow.classList.contains('active'),
                scenePressed: sceneFlow.getAttribute('aria-pressed'),
            };
            return { before, afterOrganic, afterFlow, afterArrows };
        });

        expect(result.before).toMatchObject({
            organicActive: false,
            organicPressed: 'false',
            rendererOrganic: false,
            forceRowFlow: false,
            sceneFlow: true,
            columns: 1,
            button: { display: 'flex', align: 'center', justify: 'center', text: 'center' },
        });
        expect(result.before.contentTransform).toContain('0.95');
        expect(result.before.swatchBackground).not.toBe('none');
        expect(result.afterOrganic).toEqual({
            active: true, pressed: 'true', rendererOrganic: true,
        });
        expect(result.afterFlow).toEqual({
            style: 'flow', active: true, pressed: 'true', panelActive: [],
        });
        expect(result.afterArrows).toEqual({
            style: 'arrows', sceneActive: false, scenePressed: 'false',
        });
    });

    test('mounts the compact layer-inspector hierarchy without reserved dead space', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        const s = await page.evaluate(() => {
            const cols = [...document.querySelectorAll('#viewport-overlay .s0-overlay-col')];
            return {
                hasSearch: !!document.getElementById('s0-overlay-search'),
                hasStrip: !!document.getElementById('s0-overlay-active'),
                width: Math.round(document.getElementById('viewport-overlay').getBoundingClientRect().width),
                categories: cols.length,
                openCategories: cols.filter((c) => !c.classList.contains('is-collapsed')).map((c) => c.dataset.col),
                stripHidden: document.getElementById('s0-overlay-active').hidden,
                stripHeight: Math.round(document.getElementById('s0-overlay-active').getBoundingClientRect().height),
                chips: [...document.querySelectorAll('#s0-overlay-active .s0-overlay-chip-label')].map((e) => e.textContent),
                summary: document.getElementById('s0-overlay-summary')?.textContent,
                renderRows: document.querySelectorAll('.s0-overlay-render-row').length,
                searchClearHidden: document.getElementById('s0-overlay-search-clear')?.hidden,
                volumeExpanded: document.querySelector('[data-col="volume"] .s0-overlay-col-head')?.getAttribute('aria-expanded'),
            };
        });
        expect(s.hasSearch && s.hasStrip).toBe(true);
        expect(s.categories).toBe(8);
        expect(s.openCategories, 'only the primary active category opens by default').toEqual(['volume']);
        expect(s.width, 'panel is a narrow inspector, not the old wide grid').toBeGreaterThanOrEqual(320);
        expect(s.width).toBeLessThanOrEqual(370);
        // flux-volume is on by default → strip shown with a Flux Volume chip.
        expect(s.stripHidden).toBe(false);
        expect(s.stripHeight, 'one active chip must not reserve multiple empty rows').toBeLessThanOrEqual(38);
        expect(s.chips.some((c) => /Flux Volume/.test(c))).toBe(true);
        expect(s.summary).toBe('1 active');
        expect(s.renderRows).toBe(2);
        expect(s.searchClearHidden).toBe(true);
        expect(s.volumeExpanded).toBe('true');

        await page.setViewportSize({ width: 1024, height: 768 });
        await page.waitForTimeout(100);
        const responsive = await page.evaluate(() => {
            const panel = document.getElementById('viewport-overlay');
            const viewport = document.getElementById('viewport');
            const body = panel.querySelector('.s0-overlay-body');
            const panelRect = panel.getBoundingClientRect();
            const viewportRect = viewport.getBoundingClientRect();
            return {
                width: Math.round(panelRect.width),
                bounded: panelRect.left >= viewportRect.left
                    && panelRect.right <= viewportRect.right
                    && panelRect.top >= viewportRect.top
                    && panelRect.bottom <= viewportRect.bottom,
                bodyOverflow: getComputedStyle(body).overflowY,
            };
        });
        expect(responsive).toEqual({ width: 326, bounded: true, bodyOverflow: 'auto' });
    });

    test('keeps every expanded toggle contained and reachable through panel scrolling', async ({ page }) => {
        await page.setViewportSize({ width: 1024, height: 768 });
        await gotoAndReady(page);
        await page.waitForTimeout(500);
        await expand(page);

        await page.evaluate(async () => {
            for (const col of document.querySelectorAll('#viewport-overlay .s0-overlay-col')) {
                if (getComputedStyle(col).display === 'none') continue;
                const head = col.querySelector('.s0-overlay-col-head');
                if (head?.getAttribute('aria-expanded') === 'false') head.click();
            }
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            document.querySelector('#viewport-overlay .s0-overlay-body').scrollTop = 0;
        });

        const initial = await page.evaluate(() => {
            const panel = document.getElementById('viewport-overlay');
            const viewport = document.getElementById('viewport');
            const body = panel.querySelector('.s0-overlay-body');
            const panelRect = panel.getBoundingClientRect();
            const viewportRect = viewport.getBoundingClientRect();
            const bodyRect = body.getBoundingClientRect();
            const columns = [...panel.querySelectorAll('.s0-overlay-col')]
                .filter((col) => getComputedStyle(col).display !== 'none');
            const toggles = [...panel.querySelectorAll('.s0-overlay-body .view-toggle')]
                .filter((button) => {
                    const style = getComputedStyle(button);
                    const rect = button.getBoundingClientRect();
                    return style.display !== 'none' && style.visibility !== 'hidden'
                        && rect.width > 0 && rect.height > 0;
                });

            return {
                bounded: panelRect.left >= viewportRect.left
                    && panelRect.right <= viewportRect.right
                    && panelRect.top >= viewportRect.top
                    && panelRect.bottom <= viewportRect.bottom,
                bodyOverflow: getComputedStyle(body).overflowY,
                bodyScrollable: body.scrollHeight > body.clientHeight,
                internallyClippedColumns: columns
                    .filter((col) => col.scrollHeight > col.clientHeight + 1)
                    .map((col) => col.dataset.col),
                horizontallyOverflowingToggles: toggles
                    .filter((button) => {
                        const rect = button.getBoundingClientRect();
                        return rect.left < bodyRect.left - 0.5 || rect.right > bodyRect.right + 0.5;
                    })
                    .map((button) => button.id),
                lastToggleId: toggles.at(-1)?.id,
            };
        });

        expect(initial.bounded, 'the complete panel stays inside the viewport').toBe(true);
        expect(initial.bodyOverflow).toBe('auto');
        expect(initial.bodyScrollable, 'expanded cards create body scroll range').toBe(true);
        expect(initial.internallyClippedColumns, 'cards retain natural height instead of hiding toggles').toEqual([]);
        expect(initial.horizontallyOverflowingToggles, 'toggle buttons stay within the panel width').toEqual([]);
        expect(initial.lastToggleId).toBeTruthy();

        await page.evaluate(() => {
            const body = document.querySelector('#viewport-overlay .s0-overlay-body');
            body.scrollTop = body.scrollHeight;
        });
        await page.waitForTimeout(100);

        const reachedBottom = await page.evaluate((lastToggleId) => {
            const body = document.querySelector('#viewport-overlay .s0-overlay-body');
            const last = document.getElementById(lastToggleId);
            const bodyRect = body.getBoundingClientRect();
            const lastRect = last.getBoundingClientRect();
            return {
                scrollTop: body.scrollTop,
                fullyVisible: lastRect.top >= bodyRect.top - 0.5
                    && lastRect.bottom <= bodyRect.bottom + 0.5,
            };
        }, initial.lastToggleId);
        expect(reachedBottom.scrollTop).toBeGreaterThan(0);
        expect(reachedBottom.fullyVisible, 'the final visible toggle is reachable at the bottom').toBe(true);
    });

    test('clicking a category header toggles it independently (not mutually exclusive)', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        const isOpen = (col) => page.evaluate((c) => !document.querySelector(`#viewport-overlay [data-col="${c}"]`).classList.contains('is-collapsed'), col);
        const clickHead = (col) => page.evaluate((c) => document.querySelector(`#viewport-overlay [data-col="${c}"] .s0-overlay-col-head`).click(), col);

        // Volume starts open and Fields starts closed. Opening Fields does not
        // close Volume; each card remains independently controllable.
        expect(await isOpen('volume'), 'volume starts expanded').toBe(true);
        expect(await isOpen('fields'), 'fields starts collapsed').toBe(false);
        await clickHead('fields');
        expect(await isOpen('fields'), 'fields opens independently').toBe(true);
        expect(await isOpen('volume'), 'volume remains open').toBe(true);
        await clickHead('volume');
        expect(await isOpen('volume'), 'volume collapses on click').toBe(false);
        expect(await isOpen('fields'), 'fields untouched, stays open (multi-state independence)').toBe(true);
        await clickHead('volume');
        expect(await isOpen('volume'), 'volume re-expands on second click').toBe(true);
        expect(await isOpen('fields'), 'fields stays open throughout').toBe(true);
    });

    test('active strip chip × turns the overlay off', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        expect(await page.evaluate(() => document.getElementById('toggle-flux-volume').classList.contains('active'))).toBe(true);
        await page.evaluate(() => document.querySelector('#s0-overlay-active .s0-overlay-chip-x').click());
        await page.waitForTimeout(150);
        const after = await page.evaluate(() => ({
            active: document.getElementById('toggle-flux-volume').classList.contains('active'),
            chips: document.querySelectorAll('#s0-overlay-active .s0-overlay-chip').length,
            stripHidden: document.getElementById('s0-overlay-active').hidden,
        }));
        expect(after.active, 'flux-volume turned off by the chip ×').toBe(false);
        expect(after.chips, 'chip removed').toBe(0);
        expect(after.stripHidden, 'strip hidden when nothing active').toBe(true);
    });

    test('filter hides non-matches and auto-expands matching categories', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        await page.evaluate(() => { const s = document.getElementById('s0-overlay-search'); s.value = 'vortic'; s.dispatchEvent(new Event('input', { bubbles: true })); });
        await page.waitForTimeout(150);
        let r = await page.evaluate(() => ({
            topoShown: !document.querySelector('#viewport-overlay [data-col="topology"]').classList.contains('is-filtered-out'),
            topoOpen: !document.querySelector('#viewport-overlay [data-col="topology"]').classList.contains('is-collapsed'),
            volHidden: document.querySelector('#viewport-overlay [data-col="volume"]').classList.contains('is-filtered-out'),
            vorticVisible: !document.getElementById('toggle-vorticity').classList.contains('is-filtered-out'),
        }));
        expect(r.topoShown && r.topoOpen, 'Topology shown + auto-expanded').toBe(true);
        expect(r.volHidden, 'Volume hidden (no match)').toBe(true);
        expect(r.vorticVisible, 'Vorticity visible').toBe(true);

        // Clearing restores: filter classes gone, default-expanded back.
        await page.evaluate(() => { const s = document.getElementById('s0-overlay-search'); s.value = ''; s.dispatchEvent(new Event('input', { bubbles: true })); });
        await page.waitForTimeout(150);
        r = await page.evaluate(() => ({
            anyFiltered: !!document.querySelector('#viewport-overlay .is-filtered-out'),
            volCollapsed: document.querySelector('#viewport-overlay [data-col="volume"]').classList.contains('is-collapsed'),
        }));
        expect(r.anyFiltered, 'no filter classes after clear').toBe(false);
        expect(r.volCollapsed, 'categories restored back to expanded default').toBe(false);
    });

    test('search clear, ARIA truth, and contextual controls follow their owning layer', async ({ page }) => {
        await gotoAndReady(page);
        await page.waitForTimeout(1500);
        await expand(page);

        const result = await page.evaluate(async () => {
            const display = (selector) => getComputedStyle(document.querySelector(selector)).display;
            const twoFrames = async () => {
                await new Promise((resolve) => requestAnimationFrame(resolve));
                await new Promise((resolve) => requestAnimationFrame(resolve));
            };

            const topology = document.querySelector('[data-col="topology"]');
            if (topology.classList.contains('is-collapsed')) topology.querySelector('.s0-overlay-col-head').click();

            const slice = document.getElementById('toggle-flux-slice');
            const energy = document.getElementById('toggle-em-energy');
            const before = {
                slicePressed: slice.getAttribute('aria-pressed'),
                sliceControls: display('#toggle-flux-slice + .flux-slice-axis-row'),
                energySlider: display('#toggle-em-energy + .s0-sheet-height-row'),
            };

            slice.click();
            energy.click();
            await twoFrames();
            const active = {
                slicePressed: slice.getAttribute('aria-pressed'),
                sliceControls: display('#toggle-flux-slice + .flux-slice-axis-row'),
                energyPressed: energy.getAttribute('aria-pressed'),
                energySlider: display('#toggle-em-energy + .s0-sheet-height-row'),
            };
            energy.click();
            await twoFrames();
            const energySliderAfterOff = display('#toggle-em-energy + .s0-sheet-height-row');

            const search = document.getElementById('s0-overlay-search');
            const clear = document.getElementById('s0-overlay-search-clear');
            search.value = 'vortic';
            search.dispatchEvent(new Event('input', { bubbles: true }));
            await twoFrames();
            const clearVisible = !clear.hidden;
            clear.click();
            await twoFrames();

            return {
                before,
                active,
                energySliderAfterOff,
                search: { value: search.value, clearVisible, clearHidden: clear.hidden },
            };
        });

        expect(result.before).toEqual({
            slicePressed: 'false',
            sliceControls: 'none',
            energySlider: 'none',
        });
        expect(result.active).toEqual({
            slicePressed: 'true',
            sliceControls: 'flex',
            energyPressed: 'true',
            energySlider: 'grid',
        });
        expect(result.energySliderAfterOff).toBe('none');
        expect(result.search).toEqual({ value: '', clearVisible: true, clearHidden: true });
    });

    test('interaction burst sustains the foreground frame budget without resource growth', async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        await gotoAndReady(page);
        await page.waitForTimeout(2500);
        await expand(page);

        const report = await page.evaluate(async () => {
            const {
                startScale0UiAuditProbe,
                measureScale0UiActionToPaint,
                stopScale0UiAuditProbe,
            } = await import('/tests/scale0-ui-audit-probe.js');
            const paint = (label, action) => measureScale0UiActionToPaint(label, action);
            const click = (selector) => document.querySelector(selector).click();
            const gl = window.__ftdCtx?.viewport?.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            const webglRenderer = rendererInfo
                ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                : '';

            // Warm every measured path once so module parsing/JIT and first-time
            // renderer setup stay outside the steady interaction capture.
            click('[data-col="fields"] .s0-overlay-col-head');
            click('[data-col="fields"] .s0-overlay-col-head');
            click('#force-style-row [data-style="glyphs"]');
            click('#force-style-row [data-style="arrows"]');
            click('#toggle-flux-slice');
            click('#toggle-flux-slice');
            const warmSearch = document.getElementById('s0-overlay-search');
            warmSearch.value = 'vortic';
            warmSearch.dispatchEvent(new Event('input', { bubbles: true }));
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            click('#s0-overlay-search-clear');
            click('.s0-overlay-collapse');
            click('.s0-overlay-collapse');
            await new Promise((resolve) => setTimeout(resolve, 500));

            startScale0UiAuditProbe({ rootSelector: '#viewport-overlay' });
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));

            await paint('open fields', () => click('[data-col="fields"] .s0-overlay-col-head'));
            await paint('close fields', () => click('[data-col="fields"] .s0-overlay-col-head'));
            await paint('vector glyphs', () => click('#force-style-row [data-style="glyphs"]'));
            await paint('vector arrows', () => click('#force-style-row [data-style="arrows"]'));
            await paint('slice on', () => click('#toggle-flux-slice'));
            await paint('slice off', () => click('#toggle-flux-slice'));

            const search = document.getElementById('s0-overlay-search');
            await paint('filter', () => {
                search.value = 'vortic';
                search.dispatchEvent(new Event('input', { bubbles: true }));
            });
            await paint('clear filter', () => click('#s0-overlay-search-clear'));
            await paint('panel collapse', () => click('.s0-overlay-collapse'));
            await paint('panel expand', () => click('.s0-overlay-collapse'));

            await new Promise((resolve) => setTimeout(resolve, 12_000));
            const audit = await stopScale0UiAuditProbe();
            return { ...audit, webglRenderer };
        });

        console.log('scale0 overlay interaction budget', JSON.stringify(report));
        await testInfo.attach('scale0-visualization-performance-report.json', {
            body: Buffer.from(JSON.stringify(report, null, 2)),
            contentType: 'application/json',
        });
        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(report.webglRenderer, 'release gate exposes a WebGL renderer').not.toBe('');
            expect(report.webglRenderer, 'release gate does not certify SwiftShader/software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        expect(report.frames.count).toBeGreaterThanOrEqual(600);
        expect(report.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
        expect(report.frames.p95Ms).toBeLessThanOrEqual(17);
        expect(report.frames.p99Ms).toBeLessThanOrEqual(20);
        expect(report.frames.intervalsOver33_4ms).toBe(0);
        expect(report.longTasks).toEqual([]);
        expect(report.actions.p95Ms).toBeLessThanOrEqual(50);
        expect(report.resourceDelta.rafSubscribers).toBe(0);
        expect(report.resourceDelta.domNodes).toBe(0);
        expect(report.resourceDelta.canvases).toBe(0);
        expect(report.errors).toEqual([]);
    });
});
