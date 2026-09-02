// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const select = document.getElementById('pe-scenario-select');
        if (!select) throw new Error('Scale 1 scenario selector missing');
        select.value = scenarioId;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

test('Scale 1 trajectory controls drive breadcrumb, line, and energy-heatmap rendering', async ({ page }) => {
    page.setDefaultTimeout(20_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page);
    await switchMode(page, 'particles');
    await selectScenario(page, 's1-coulomb-orbit');
    await page.locator('#tab-bar .tab[data-panel="controls"]').click();

    await expect(page.locator('#pe-trail-history')).toBeVisible();
    await expect(page.locator('[data-pe-trail-mode]')).toHaveCount(3);
    const values = [
        ['#pe-trail-history', '600', '#pe-trail-history-value', '600 ticks'],
        ['#pe-trail-stride', '3', '#pe-trail-stride-value', '3 ticks'],
        ['#pe-trail-despawn', '300', '#pe-trail-despawn-value', '300 ticks'],
        ['#pe-trail-opacity', '0.82', '#pe-trail-opacity-value', '0.82'],
        ['#pe-trail-size', '0.48', '#pe-trail-size-value', '0.48 lu'],
    ];
    for (const [input, value, output, label] of values) {
        await page.locator(input).fill(value);
        await expect(page.locator(output)).toHaveText(label);
    }

    await page.locator('#btn-play').click();
    await expect.poll(() => page.evaluate(() => {
        const trails = window.__ftdCtx?.viewport?._particleRenderer?.trails;
        return {
            points: !!trails?.isPoints,
            opacity: trails?.material?.opacity ?? null,
            size: trails?.material?.size ?? null,
            drawn: trails?.geometry?.drawRange?.count || 0,
        };
    }), { timeout: 20_000 }).toMatchObject({
        points: true,
        opacity: 0.82,
        size: 0.48,
    });
    await expect.poll(() => page.evaluate(() =>
        window.__ftdCtx?.viewport?._particleRenderer?.trails?.geometry?.drawRange?.count || 0,
    ), { timeout: 20_000 }).toBeGreaterThan(2);

    await page.locator('[data-pe-trail-mode="lines"]').click();
    await expect(page.locator('[data-pe-trail-mode="lines"]')).toHaveAttribute('aria-pressed', 'true');
    await expect.poll(() => page.evaluate(() => {
        const trails = window.__ftdCtx?.viewport?._particleRenderer?.trails;
        return { lines: !!trails?.isLineSegments, mode: trails?.userData?.trailMode };
    })).toEqual({ lines: true, mode: 'lines' });
    await expect(page.locator('#pe-trail-energy-legend')).toBeHidden();

    await page.locator('[data-pe-trail-mode="energy"]').click();
    await expect(page.locator('#pe-trail-energy-legend')).toBeVisible();
    await expect.poll(() => page.evaluate(() => {
        const trails = window.__ftdCtx?.viewport?._particleRenderer?.trails;
        const count = trails?.geometry?.drawRange?.count || 0;
        const colors = trails?.geometry?.getAttribute('color')?.array || [];
        const unique = new Set();
        for (let index = 0; index < Math.min(count * 3, colors.length); index += 3) {
            unique.add(`${colors[index].toFixed(3)}:${colors[index + 1].toFixed(3)}:${colors[index + 2].toFixed(3)}`);
        }
        return {
            lines: !!trails?.isLineSegments,
            mode: trails?.userData?.trailMode,
            colors: unique.size,
        };
    })).toMatchObject({ lines: true, mode: 'energy' });
    await expect.poll(() => page.evaluate(() => {
        const trails = window.__ftdCtx?.viewport?._particleRenderer?.trails;
        const count = trails?.geometry?.drawRange?.count || 0;
        const colors = trails?.geometry?.getAttribute('color')?.array || [];
        const unique = new Set();
        for (let index = 0; index < Math.min(count * 3, colors.length); index += 3) {
            unique.add(`${colors[index].toFixed(3)}:${colors[index + 1].toFixed(3)}:${colors[index + 2].toFixed(3)}`);
        }
        return unique.size;
    })).toBeGreaterThan(1);
    await expect(page.locator('#pe-trail-energy-max')).not.toHaveText('0');

    await page.locator('#btn-pe-trail-reset').click();
    await expect(page.locator('[data-pe-trail-mode="breadcrumbs"]')).toHaveAttribute('aria-pressed', 'true');
    await expect(page.locator('#pe-trail-history-value')).toHaveText('240 ticks');
    await expect(page.locator('#pe-trail-stride-value')).toHaveText('1 tick');
    await expect(page.locator('#pe-trail-despawn-value')).toHaveText('120 ticks');
    await expect(page.locator('#pe-trail-opacity-value')).toHaveText('0.72');
    await expect(page.locator('#pe-trail-size-value')).toHaveText('0.34 lu');
    expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
});
