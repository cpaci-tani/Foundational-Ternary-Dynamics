// @ts-check
/**
 * The view ruler fits the open stretch of its own row.
 *
 * Whatever sits in that row (the icon rail of a collapsed dock, an open side
 * panel, an overlay panel open or collapsed, the play bar when the dock is at
 * the bottom) the ruler stops short of it with the same gap on both sides, in
 * every dock position and on every scale. Overlay panels collapse to a
 * one-line pill so the ruler gets the row back, and the default framing keeps
 * the clock above the lattice below the ruler.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const GAP = 8;
const BAND = 70;

/** The ruler's rectangle and every visible element that reaches into its row. */
async function readRow(page) {
    return page.evaluate(({ band }) => {
        const view = document.getElementById('viewport');
        const viewRect = view.getBoundingClientRect();
        const ruler = document.querySelector('.live-ruler-view');
        const shown = (el) => {
            if (!el || el.hidden || el.getClientRects().length === 0) return false;
            const style = getComputedStyle(el);
            return style.visibility !== 'hidden' && Number(style.opacity) >= 0.05;
        };
        const candidates = [
            ...document.querySelectorAll('#tab-bar, #panel-rail-resizer, #panel-area, #panel-side-resizer, #play-bar'),
            ...[...view.children].filter((el) => !el.matches('.live-rulers, #viewport-frame-chrome, canvas')),
        ];
        const inRow = [];
        for (const el of candidates) {
            if (!shown(el)) continue;
            const rect = el.getBoundingClientRect();
            if (rect.width < 8 || rect.height < 8 || rect.width > viewRect.width * 0.85) continue;
            if (!(rect.top < viewRect.top + band && rect.bottom > viewRect.top)) continue;
            inRow.push({ name: el.id || el.className, left: rect.left, right: rect.right });
        }
        const rect = ruler.getBoundingClientRect();
        return {
            hidden: ruler.hidden || ruler.getClientRects().length === 0,
            left: rect.left, right: rect.right, width: rect.width,
            viewLeft: viewRect.left, viewRight: viewRect.right,
            inRow,
        };
    }, { band: BAND });
}

function expectClear(row, label) {
    if (row.hidden) return;
    for (const obstacle of row.inRow) {
        const clear = obstacle.right <= row.left - GAP + 0.5 || obstacle.left >= row.right + GAP - 0.5;
        expect(clear, `${label}: ruler ${Math.round(row.left)}..${Math.round(row.right)} clears ${obstacle.name} `
            + `${Math.round(obstacle.left)}..${Math.round(obstacle.right)}`).toBe(true);
    }
    expect(row.left).toBeGreaterThanOrEqual(row.viewLeft + GAP - 0.5);
    expect(row.right).toBeLessThanOrEqual(row.viewRight - GAP + 0.5);
}

async function setDock(page, mount, collapsed) {
    await page.evaluate(({ mount: next, collapsed: want }) => {
        document.documentElement.dataset.panelMount = next;
        const app = document.getElementById('app');
        if (app.classList.contains('panels-collapsed') !== want) document.getElementById('btn-panel-toggle').click();
    }, { mount, collapsed });
    // The dock animates and the ruler re-reads layout at most every 250 ms.
    await page.waitForTimeout(900);
}

async function setOverlayCollapsed(page, collapsed) {
    await page.evaluate((want) => {
        const button = [...document.querySelectorAll('.s0-overlay-collapse, .viewport-overlay-collapse')]
            .find((el) => el.getClientRects().length > 0);
        const panel = button?.closest('#viewport-overlay, .viewport-overlay-panel');
        if (panel && panel.classList.contains('is-collapsed') !== want) button.click();
    }, collapsed);
    await page.waitForTimeout(500);
}

test.describe('view ruler fits between the UI', () => {
    test.use({ viewport: { width: 1800, height: 1000 } });

    test.beforeEach(async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
        await page.evaluate(() => document.getElementById('gpu-server-card')?.remove());
    });

    test('every dock position, with the dock and the Visualization panel open or collapsed', async ({ page }) => {
        for (const mount of ['left', 'right', 'bottom']) {
            for (const dockCollapsed of [false, true]) {
                for (const overlayCollapsed of [false, true]) {
                    await setDock(page, mount, dockCollapsed);
                    await setOverlayCollapsed(page, overlayCollapsed);
                    const row = await readRow(page);
                    const label = `${mount}, dock ${dockCollapsed ? 'collapsed' : 'open'}, panel ${overlayCollapsed ? 'collapsed' : 'open'}`;
                    expect(row.inRow.length, `${label}: something is in the ruler's row`).toBeGreaterThan(0);
                    expectClear(row, label);
                    if (mount !== 'bottom') expect(row.hidden, `${label}: there is room for the ruler`).toBe(false);
                }
            }
        }
    });

    test('a collapsed dock leaves its icon rail, and the ruler starts beside it', async ({ page }) => {
        await setDock(page, 'left', true);
        await setOverlayCollapsed(page, true);
        const row = await readRow(page);
        const rail = row.inRow.find((entry) => entry.name === 'tab-bar');
        expect(rail, 'the rail is in the ruler row').toBeTruthy();
        expect(row.left).toBeGreaterThanOrEqual(rail.right + GAP - 0.5);
        expect(row.left - rail.right, 'no more than the rail, its grip and the gap').toBeLessThan(24);
    });

    test('the Visualization panel collapses to a one-line pill that is its own button', async ({ page }) => {
        await setDock(page, 'left', true);
        await setOverlayCollapsed(page, false);
        const open = await readRow(page);
        await setOverlayCollapsed(page, true);
        const pill = await page.evaluate(() => {
            const panel = document.getElementById('viewport-overlay');
            const rect = panel.getBoundingClientRect();
            const button = panel.querySelector('.s0-overlay-collapse').getBoundingClientRect();
            const shown = (selector) => panel.querySelector(selector).getClientRects().length > 0;
            return {
                width: rect.width, height: rect.height,
                buttonCovers: button.width >= rect.width - 4 && button.height >= rect.height - 4,
                text: panel.querySelector('.s0-overlay-header').innerText.replace(/\s+/g, ' ').trim(),
                summary: document.getElementById('s0-overlay-summary').textContent,
                subtitle: shown('.s0-overlay-subtitle'), body: shown('.s0-overlay-body'),
                expanded: panel.querySelector('.s0-overlay-collapse').getAttribute('aria-expanded'),
            };
        });
        expect(pill.height).toBeLessThanOrEqual(44);
        expect(pill.width).toBeLessThanOrEqual(240);
        expect(pill.text).toMatch(/^VISUALIZATION \d+$/i);
        expect(pill.summary, 'the full count is still in the document').toMatch(/^\d+ active$/);
        expect(pill.subtitle).toBe(false);
        expect(pill.body).toBe(false);
        expect(pill.buttonCovers, 'the whole pill is the control').toBe(true);
        expect(pill.expanded).toBe('false');

        const collapsed = await readRow(page);
        expect(collapsed.width, 'the ruler gets the row back').toBeGreaterThan(open.width + 100);

        // A click anywhere on the pill opens the panel again.
        const box = await page.locator('#viewport-overlay').boundingBox();
        await page.mouse.click(box.x + 40, box.y + box.height / 2);
        await expect(page.locator('#viewport-overlay')).not.toHaveClass(/is-collapsed/);
        await expect(page.locator('#viewport-overlay .s0-overlay-body')).toBeVisible();
    });

    test('other scales: the ruler avoids their overlay panel, which collapses to a pill', async ({ page }) => {
        for (const mode of ['particles', 'atoms', 'planetary', 'cosmic']) {
            await page.evaluate((next) => {
                const select = /** @type {HTMLSelectElement} */ (document.getElementById('engine-mode'));
                select.value = next;
                select.dispatchEvent(new Event('change', { bubbles: true }));
            }, mode);
            await page.waitForFunction((next) => document.getElementById('app').classList.contains(`mode-${next}`), mode, { timeout: 30_000 });
            await page.waitForTimeout(2500);
            await setOverlayCollapsed(page, false);
            const open = await readRow(page);
            expect(open.inRow.some((entry) => /viewport-overlay/.test(entry.name)), `${mode}: its overlay panel is in the row`).toBe(true);
            expectClear(open, `${mode}, panel open`);

            await setOverlayCollapsed(page, true);
            const size = await page.evaluate(() => {
                const panel = [...document.querySelectorAll('.viewport-overlay-panel')].find((el) => el.getClientRects().length > 0);
                const rect = panel.getBoundingClientRect();
                const button = panel.querySelector('.viewport-overlay-collapse').getBoundingClientRect();
                return { width: rect.width, height: rect.height, covers: button.width >= rect.width - 4 && button.height >= rect.height - 4 };
            });
            expect(size.height, `${mode}: one line`).toBeLessThanOrEqual(44);
            expect(size.covers, `${mode}: the whole pill is the control`).toBe(true);
            const collapsed = await readRow(page);
            expectClear(collapsed, `${mode}, panel collapsed`);
            expect(collapsed.width).toBeGreaterThanOrEqual(open.width);
            await setOverlayCollapsed(page, false);
        }
    });

    test('tick labels never print over each other', async ({ page }) => {
        for (const [mount, dockCollapsed] of [['left', true], ['left', false]]) {
            await setDock(page, /** @type {string} */ (mount), /** @type {boolean} */ (dockCollapsed));
            for (const view of ['lattice', 'quasi', 'detail']) {
                await page.evaluate((id) => window.__ftdCtx.viewport.setFramedView(id), view);
                await page.waitForTimeout(350);
                const overlaps = await page.evaluate(() => {
                    const labels = [...document.querySelectorAll('.live-ruler-view .live-ruler-tick span')]
                        .map((el) => el.getBoundingClientRect()).filter((rect) => rect.width > 0)
                        .sort((a, b) => a.left - b.left);
                    let worst = Infinity;
                    for (let i = 1; i < labels.length; i++) worst = Math.min(worst, labels[i].left - labels[i - 1].right);
                    return { count: labels.length, worst };
                });
                expect(overlaps.count).toBeGreaterThanOrEqual(2);
                expect(overlaps.worst, `${mount} ${view}: labels keep clear of each other`).toBeGreaterThanOrEqual(6);
            }
        }
    });

    test('the default and whole-lattice views keep the clock below the ruler', async ({ page }) => {
        const clockTop = () => page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const core = viewport._sceneCore;
            const clock = core.globalClock;
            const height = viewport.renderer.domElement.getBoundingClientRect().height;
            viewport.camera.updateMatrixWorld();
            clock.updateWorldMatrix(true, false);
            const eye = clock.getWorldPosition(clock.position.clone()).applyMatrix4(viewport.camera.matrixWorldInverse);
            const radius = clock.userData.radius * clock.getWorldScale(clock.position.clone()).y;
            const tan = Math.tan((viewport.camera.fov * Math.PI) / 360);
            const ndcTop = (eye.y + radius) / (-eye.z * tan);
            return { topPx: ((1 - ndcTop) / 2) * height, visible: clock.visible, extra: core.clockClearance() };
        });
        const boot = await clockTop();
        expect(boot.visible).toBe(true);
        expect(boot.topPx, 'default view: clock top is below the ruler row').toBeGreaterThanOrEqual(BAND);
        // Framed at boot, before the shell's final height settles: within a
        // lattice unit of where it would be framed now.
        expect(boot.extra).toBeLessThan(1);

        await page.evaluate(() => window.__ftdCtx.viewport.setFramedView('lattice'));
        const framed = await clockTop();
        expect(framed.topPx, 'whole-lattice view: clock top is below the ruler row').toBeGreaterThanOrEqual(BAND);

        // With the clock switched off there is nothing to keep clear, and the
        // default view goes back to its plain distance.
        const plain = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            viewport.toggleGlobalClock(false);
            viewport._sceneCore.setCameraPreset('front');
            const n = viewport.latticeSize;
            const z = viewport.camera.position.z;
            viewport.toggleGlobalClock(true);
            return { n, z };
        });
        expect(plain.z).toBeCloseTo(plain.n / 2 + plain.n * 2.2, 9);
    });
});
