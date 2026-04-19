// @ts-check
import { test, expect } from '@playwright/test';

test.describe('Verify panel', () => {
    test.beforeEach(async ({ page }) => {
        await page.goto('/');
        await page.waitForFunction(() => document.getElementById('app')?.dataset?.shellReady === 'true', { timeout: 10000 });
        // Wait for the loading overlay to be dismissed — this happens after wireTabs()
        // binds the panel dock, so tab clicks are functional after this point.
        await page.waitForFunction(
            () => document.getElementById('loading-overlay')?.classList.contains('hidden'),
            { timeout: 15000 }
        );
        await page.evaluate(() => document.querySelector('#tab-bar .tab[data-panel="verification-lab"]')?.click());
        await page.waitForSelector('#panel-verification-lab.active');
        // Give the async manifest fetch a moment to resolve and render.
        await page.waitForSelector('.verify-header', { timeout: 10000 });
    });

    test('renders header with build stamp and counts', async ({ page }) => {
        const stamp = await page.textContent('.verify-header-stamp');
        expect(stamp).toMatch(/FTD v/);
        expect(stamp).toMatch(/build /);

        const counts = await page.locator('.verify-counts li').allTextContents();
        expect(counts.length).toBe(3);
        expect(counts.some((t) => /hard-prediction/.test(t))).toBe(true);
    });

    test('renders exactly three tiers', async ({ page }) => {
        const tiers = await page.locator('.verify-tier').count();
        expect(tiers).toBe(3);
        await expect(page.locator('.verify-tier--hard')).toBeVisible();
        await expect(page.locator('.verify-tier--parametric')).toBeVisible();
        await expect(page.locator('.verify-tier--unpredicted')).toBeVisible();
    });

    test('hard-tier rows show a pull strip; parametric rows do not', async ({ page }) => {
        const hardStrips = await page.locator('.verify-tier--hard .verify-pull-strip').count();
        expect(hardStrips).toBeGreaterThan(0);
        const paramStrips = await page.locator('.verify-tier--parametric .verify-pull-strip').count();
        expect(paramStrips).toBe(0);
    });

    test('filter pills hide non-matching tiers', async ({ page }) => {
        await page.click('.verify-filter[data-filter="hard"]');
        await expect(page.locator('.verify-tier--hard')).toBeVisible();
        await expect(page.locator('.verify-tier--parametric')).not.toBeVisible();
        await expect(page.locator('.verify-tier--unpredicted')).not.toBeVisible();

        await page.click('.verify-filter[data-filter="all"]');
        await expect(page.locator('.verify-tier--parametric')).toBeVisible();
    });

    test('no PASS/FAIL badge strings appear anywhere in the panel', async ({ page }) => {
        const body = await page.locator('#panel-verification-lab').innerText();
        expect(body).not.toMatch(/\bPASS\b/);
        expect(body).not.toMatch(/\bFAIL\b/);
        expect(body).not.toMatch(/\bCLOSE\b/);
    });

    test('unpredicted rows show "no prediction" rather than an FTD value', async ({ page }) => {
        const unpredTier = page.locator('.verify-tier--unpredicted');
        await expect(unpredTier).toContainText(/no prediction/i);
    });
});
