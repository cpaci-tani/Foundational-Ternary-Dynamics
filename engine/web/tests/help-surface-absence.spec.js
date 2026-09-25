// @ts-check
import { test, expect } from '@playwright/test';

test.describe('Compact dashboard guidance', () => {
    test.beforeEach(async ({ page }) => {
        await page.goto('/');
        await page.waitForFunction(() => document.getElementById('app')?.dataset?.shellReady === 'true', { timeout: 10000 });
        await page.waitForFunction(
            () => document.getElementById('loading-overlay')?.classList.contains('hidden'),
            { timeout: 15000 }
        );
    });

    test('FAQ and KB are absent while the active panel title remains', async ({ page }) => {
        await expect(page.locator('#btn-faq')).toHaveCount(0);
        await expect(page.locator('#faq-sidebar')).toHaveCount(0);
        await expect(page.locator('#btn-knowledge-base')).toHaveCount(0);
        await expect(page.locator('#kb-sidebar')).toHaveCount(0);
        await expect(page.locator('#app')).not.toHaveClass(/\bfaq-open\b/);
        await expect(page.locator('#app')).not.toHaveClass(/\bknowledge-base-open\b/);
        await expect(page.locator('.panel-dock-kicker')).toHaveCount(0);
        await expect(page.locator('#panel-dock-active-title')).toContainText('Controls');
    });

    test('remaining toolbar controls provide hover guidance', async ({ page }) => {
        const button = page.locator('#btn-settings');
        await expect(button).toBeVisible();
        await button.hover();
        await expect(page.locator('#ui-tooltip')).toBeVisible();
        await expect(page.locator('#ui-tooltip')).toContainText('settings');
    });
});
