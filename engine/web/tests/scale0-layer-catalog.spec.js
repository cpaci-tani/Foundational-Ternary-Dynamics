// @ts-check
/**
 * The Scale 0 Visualization panel as built from layer-catalog.js.
 *
 * Every layer is always listed, grouped by the quantity it draws. A layer's
 * tooltip leads with its equation, typeset, and fits the tooltip. A renamed
 * layer is still found by its earlier name. The Rendering rows follow the
 * layers that are on, and group choices are stored under one key prefix.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 Visualization panel layer table', () => {
    test.beforeEach(async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
        await page.evaluate(() => {
            document.getElementById('gpu-server-card')?.remove();
            const panel = document.getElementById('viewport-overlay');
            if (panel.classList.contains('is-collapsed')) panel.querySelector('.s0-overlay-collapse').click();
        });
    });

    test('the panel lists the table: eight groups, thirty-seven layers, in order', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const { LAYER_GROUPS } = await import('/js/scales/scale0/ui/overlays/layer-catalog.js');
            const panel = document.getElementById('viewport-overlay');
            const groups = [...panel.querySelectorAll('.s0-overlay-col')].map((col) => ({
                id: col.dataset.col,
                label: col.querySelector('.s0-overlay-col-label').textContent,
                layers: [...col.querySelectorAll('.view-toggle.field-toggle')].map((button) => [button.id, button.textContent.trim()]),
                shown: getComputedStyle(col).display !== 'none',
            }));
            const header = panel.querySelector('.s0-overlay-header');
            return {
                expected: LAYER_GROUPS.map((group) => ({
                    id: group.id, label: group.label, layers: group.layers.map((layer) => [layer.id, layer.label]), shown: true,
                })),
                groups,
                headerText: header.innerText.replace(/\s+/g, ' ').trim(),
                headerHeight: header.getBoundingClientRect().height,
                searchRow: panel.querySelector('.s0-overlay-command').getBoundingClientRect().height,
                buttonsWithoutType: panel.querySelectorAll('button:not([type="button"])').length,
                inlineStyles: panel.querySelectorAll('[style]').length,
                statusTags: [...panel.querySelectorAll('[data-ui-tooltip], [title]')]
                    .map((el) => el.dataset.uiTooltip || el.title)
                    .filter((text) => /\[[A-Z][A-Z —-]*\]|LEDGER|FTD-\d{4}/.test(text)).length,
            };
        });
        expect(result.groups).toEqual(result.expected);
        expect(result.headerText).toMatch(/^VISUALIZATION \d+ active$/i);
        expect(result.headerHeight, 'one-line header').toBeLessThanOrEqual(44);
        expect(result.searchRow, 'one 40 px search row').toBeLessThanOrEqual(42);
        expect(result.buttonsWithoutType).toBe(0);
        expect(result.inlineStyles, 'no inline styles in the markup').toBe(0);
        expect(result.statusTags, 'no status tags or ledger references in the panel').toBe(0);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('every equation typesets and fits the tooltip', async ({ page }) => {
        await page.waitForFunction(() => !!window.katex, undefined, { timeout: 30_000 });
        const rendered = await page.evaluate(async () => {
            const { LAYERS } = await import('/js/scales/scale0/ui/overlays/layer-catalog.js');
            // The tooltip is 320 px wide with 12 px padding on each side.
            const box = document.createElement('div');
            box.className = 'ui-tooltip';
            box.style.cssText = 'position:fixed;left:0;top:0;width:320px;visibility:hidden';
            document.body.append(box);
            const out = [];
            for (const layer of LAYERS.filter((entry) => entry.equation)) {
                box.innerHTML = window.katex.renderToString(layer.equation, { throwOnError: false, displayMode: true, output: 'html' });
                const formula = box.querySelector('.katex-html');
                out.push({
                    id: layer.id,
                    error: !!box.querySelector('.katex-error'),
                    width: Math.ceil(formula.getBoundingClientRect().width),
                    room: box.clientWidth - 24,
                });
            }
            box.remove();
            return out;
        });
        expect(rendered.length).toBe(36);
        for (const entry of rendered) {
            expect(entry.error, `${entry.id}: KaTeX accepts the equation`).toBe(false);
            expect(entry.width, `${entry.id}: ${entry.width} px in ${entry.room} px`).toBeLessThanOrEqual(entry.room);
        }

        // And the live tooltip shows it: math first, then the sentence.
        await page.locator('#toggle-flux-slice').hover();
        const tooltip = page.locator('.ui-tooltip');
        await expect(tooltip).toBeVisible();
        await expect(tooltip.locator('.katex')).toHaveCount(1);
        await expect(tooltip).toContainText('Flux magnitude on the three mid-planes');
    });

    test('a renamed layer is still found by its earlier name, dimmed or not', async ({ page }) => {
        const found = await page.evaluate(async () => {
            const search = document.getElementById('s0-overlay-search');
            const twoFrames = async () => {
                await new Promise((resolve) => requestAnimationFrame(resolve));
                await new Promise((resolve) => requestAnimationFrame(resolve));
            };
            const out = {};
            for (const query of ['horizon', 'halo', 'psi', 'confinement', 'strong', 'entropy', 'lagrangian', 'color charge', 'dual j']) {
                search.value = query;
                search.dispatchEvent(new Event('input', { bubbles: true }));
                await twoFrames();
                out[query] = [...document.querySelectorAll('#viewport-overlay .view-toggle.field-toggle')]
                    .filter((button) => !button.classList.contains('is-filtered-out') && button.getClientRects().length > 0)
                    .map((button) => button.id);
            }
            search.value = '';
            search.dispatchEvent(new Event('input', { bubbles: true }));
            await twoFrames();
            return out;
        });
        expect(found.horizon).toEqual(['toggle-horizon']);
        expect(found.halo).toEqual(['toggle-dark-halo']);
        expect(found.psi).toEqual(['toggle-psi-squared']);
        expect(found.confinement).toEqual(['toggle-confinement']);
        expect(found.strong).toContain('toggle-force-strong');
        expect(found.entropy).toEqual(['toggle-entropy-density']);
        expect(found.lagrangian).toEqual(['toggle-lagrangian-density']);
        expect(found['color charge']).toEqual(['toggle-color-charge']);
        expect(found['dual j']).toEqual(['toggle-dual-substrate']);
    });

    test('group choices are stored under one prefix and the old keys are dropped', async ({ page }) => {
        await page.evaluate(() => {
            localStorage.setItem('ftd.s0overlay.inspector.v1.cat.volume.collapsed', '1');
            localStorage.setItem('ftd.s0overlay.inspector.v1.cat.topology.collapsed', '0');
        });
        await page.reload();
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true'
            && !!document.querySelector('#viewport-overlay .s0-overlay-col'), undefined, { timeout: 90_000 });
        const state = await page.evaluate(() => {
            const open = () => [...document.querySelectorAll('#viewport-overlay .s0-overlay-col')]
                .filter((col) => !col.classList.contains('is-collapsed')).map((col) => col.dataset.col);
            const before = open();
            document.querySelector('[data-col="energy"] .s0-overlay-col-head').click();
            return {
                before,
                after: open(),
                retired: Object.keys(localStorage).filter((key) => key.startsWith('ftd.s0overlay.')),
                stored: localStorage.getItem('ftd.overlay.scale0.group.energy.collapsed'),
            };
        });
        expect(state.before, 'Flux J holds the one layer that is on').toEqual(['flux']);
        expect(state.after).toEqual(['flux', 'energy']);
        expect(state.retired).toEqual([]);
        expect(state.stored).toBe('0');
    });
});
