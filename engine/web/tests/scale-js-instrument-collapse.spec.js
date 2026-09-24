import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode } from './_helpers.js';

async function chooseCoveredTransport(page, panelSelector) {
    return page.evaluate((selector) => {
        document.querySelector('[data-collapse-test-target]')?.removeAttribute('data-collapse-test-target');
        const panelRect = document.querySelector(selector).getBoundingClientRect();
        const controls = [...document.querySelectorAll('#play-bar button')]
            .filter((button) => button.checkVisibility())
            .map((button) => ({ button, rect: button.getBoundingClientRect() }));
        const overlap = ({ rect }) => Math.max(0, Math.min(rect.right, panelRect.right) - Math.max(rect.left, panelRect.left))
            * Math.max(0, Math.min(rect.bottom, panelRect.bottom) - Math.max(rect.top, panelRect.top));
        const target = controls.sort((a, b) => overlap(b) - overlap(a))[0]?.button;
        target?.setAttribute('data-collapse-test-target', 'true');
        const rect = target?.getBoundingClientRect();
        const hit = rect ? document.elementFromPoint(rect.left + rect.width / 2, rect.top + rect.height / 2) : null;
        return { covered: !!target && !(hit === target || target.contains(hit)) };
    }, panelSelector);
}

async function expectCollapseReleasesTransport(page, panelSelector) {
    const button = page.locator(`${panelSelector} .instrument-panel-collapse`);
    const box = await button.boundingBox();
    expect(box?.width).toBeGreaterThanOrEqual(44);
    expect(box?.height).toBeGreaterThanOrEqual(44);
    await button.click();
    await expect(page.locator(panelSelector)).toHaveClass(/instrument-panel-collapsed/);
    await expect(button).toHaveAttribute('aria-expanded', 'false');
    const target = page.locator('[data-collapse-test-target]');
    await expect(target).toBeVisible();
    await target.click({ trial: true });
    const hit = await page.evaluate(() => {
        const target = document.querySelector('[data-collapse-test-target]');
        const rect = target.getBoundingClientRect();
        const node = document.elementFromPoint(rect.left + rect.width / 2, rect.top + rect.height / 2);
        return node === target || target.contains(node);
    });
    expect(hit).toBe(true);
    await button.click();
    await expect(page.locator(panelSelector)).not.toHaveClass(/instrument-panel-collapsed/);
    await expect(button).toHaveAttribute('aria-expanded', 'true');
    await expect(page.locator(`${panelSelector} > :not(.instrument-panel-chrome)`).first()).toBeVisible();
}

test.describe.serial('shared instrument panel collapse chrome', () => {
    let context;
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        context = await browser.newContext({ baseURL, deviceScaleFactor: 2 });
        page = await context.newPage();
        await gotoAndReady(page, { path: '/?engine=wasm' });
        await page.evaluate(() => document.getElementById('gpu-card-close')?.click());
    });

    test.afterAll(async () => context?.close());

    test('Genesis collapse exposes covered transport and keeps plot labels separated', async () => {
        await switchMode(page, 'lattice');
        await page.evaluate(async () => {
            const controller = await import('/js/scales/scale0/controller.js?v=44');
            controller.loadScenario(window.__ftdCtx, 's0-seed-cluster-law');
        });
        await page.waitForSelector('#genesis-burst-panel .genesis-burst-plot', { state: 'visible' });
        for (const [width, height] of [[1440, 900], [390, 844], [667, 375]]) {
            await page.setViewportSize({ width, height });
            await page.waitForTimeout(100);
            const labelBounds = await page.evaluate(async () => {
                const canvas = document.querySelector('#genesis-burst-panel .genesis-burst-plot');
                const rect = canvas.getBoundingClientRect();
                const ctx = canvas.getContext('2d');
                ctx.font = '16px sans-serif';
                const { computeGenesisPlotLayout } = await import('/js/scales/scale0/ui/overlays/genesis-burst-panel.js');
                const layout = computeGenesisPlotLayout(rect.width, rect.height, (text) => ctx.measureText(text).width);
                const tick90 = layout.ticks.at(-1);
                return {
                    inBounds: tick90.left >= 0 && tick90.right <= rect.width
                        && layout.axisLabel.left >= 0 && layout.axisLabel.right <= rect.width,
                    gap: layout.axisLabel.left - tick90.right,
                    requiredGap: layout.labelGap,
                };
            });
            expect(labelBounds.inBounds).toBe(true);
            expect(labelBounds.gap).toBeGreaterThanOrEqual(labelBounds.requiredGap);
            const target = await chooseCoveredTransport(page, '#genesis-burst-panel');
            if (width < 1000) expect(target.covered).toBe(true);
            await expectCollapseReleasesTransport(page, '#genesis-burst-panel');
        }
    });

    test('Meta collapse exposes covered transport and expansion restores content', async () => {
        await switchMode(page, 'meta');
        for (const [width, height] of [[1440, 900], [390, 844], [667, 375]]) {
            await page.setViewportSize({ width, height });
            await page.waitForTimeout(100);
            const target = await chooseCoveredTransport(page, '#scale6-meta-panel');
            if (width < 1000) expect(target.covered).toBe(true);
            await expectCollapseReleasesTransport(page, '#scale6-meta-panel');
        }
    });
});
