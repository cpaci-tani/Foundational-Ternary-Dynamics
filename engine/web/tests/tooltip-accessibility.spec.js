import { test, expect } from '@playwright/test';
import { bootDashboard } from './_helpers.js';

test('tooltips follow keyboard focus without discarding other descriptions', async ({ page }) => {
    await bootDashboard(page, { engine: 'wasm' });

    const play = page.locator('#btn-play');
    const step = page.locator('#btn-step');
    const tooltip = page.locator('#ui-tooltip');
    await play.evaluate((button) => button.setAttribute('aria-describedby', 'existing-help'));

    await play.focus();
    await expect(tooltip).toBeVisible();
    await expect(tooltip).toContainText('Play/pause');
    await expect(play).toHaveAttribute('aria-describedby', 'existing-help ui-tooltip');

    await page.keyboard.press('Tab');
    await expect(step).toBeFocused();
    await expect(tooltip).toContainText('Advance the simulation');
    await expect(play).toHaveAttribute('aria-describedby', 'existing-help');
    await expect(step).toHaveAttribute('aria-describedby', 'ui-tooltip');

    await page.keyboard.press('Escape');
    await expect(tooltip).toBeHidden();
    await expect(step).not.toHaveAttribute('aria-describedby', /ui-tooltip/);

    await page.locator('#btn-settings').hover();
    await expect(tooltip).toBeVisible();
    await expect(tooltip).toContainText('settings');

    await page.locator('#btn-settings').click();
    await page.locator('[data-setting="tooltips"][data-value="off"]').click();
    await expect(page.locator('html')).toHaveAttribute('data-tooltips', 'off');
    await page.locator('#settings-close').click();
    await step.focus();
    await expect(tooltip).toBeHidden();
    await expect(step).not.toHaveAttribute('aria-describedby', /ui-tooltip/);
});

test.describe('touch viewport', () => {
    test.use({ viewport: { width: 390, height: 844 }, isMobile: true, hasTouch: true });

    test('a tap shows help, activates the control, and an outside tap dismisses it', async ({ page }) => {
        await bootDashboard(page, { engine: 'wasm' });
        if (await page.locator('#gpu-server-card').isVisible()) {
            await page.locator('#gpu-card-close').tap();
        }

        const play = page.locator('#btn-play');
        const tooltip = page.locator('#ui-tooltip');
        const before = await play.getAttribute('data-paused');
        await play.tap();
        await expect(tooltip).toBeVisible();
        await expect(tooltip).toContainText('Play/pause');
        await expect(play).not.toHaveAttribute('data-paused', before);

        await page.touchscreen.tap(30, 30);
        await expect(tooltip).toBeHidden();
        await expect(play).not.toHaveAttribute('aria-describedby', /ui-tooltip/);
    });
});
