// @ts-check
import { test, expect } from '@playwright/test';

test.describe('FAQ sidebar', () => {
    test.beforeEach(async ({ page }) => {
        await page.goto('/');
        await page.waitForFunction(() => document.getElementById('app')?.dataset?.shellReady === 'true', { timeout: 10000 });
        await page.waitForFunction(
            () => document.getElementById('loading-overlay')?.classList.contains('hidden'),
            { timeout: 15000 }
        );
    });

    test('FAQ button is present in the topbar', async ({ page }) => {
        await expect(page.locator('#btn-faq')).toBeVisible();
    });

    test('clicking FAQ opens the sidebar; Escape closes it', async ({ page }) => {
        await page.evaluate(() => document.querySelector('#btn-faq')?.click());
        await page.waitForFunction(() => document.getElementById('app')?.classList.contains('faq-open'), { timeout: 3000 });
        await expect(page.locator('#faq-sidebar')).toHaveAttribute('aria-hidden', 'false');
        await page.keyboard.press('Escape');
        await page.waitForFunction(() => !document.getElementById('app')?.classList.contains('faq-open'), { timeout: 3000 });
        await expect(page.locator('#faq-sidebar')).toHaveAttribute('aria-hidden', 'true');
    });

    test('opening FAQ closes KB (mutex)', async ({ page }) => {
        await page.evaluate(() => document.querySelector('#btn-knowledge-base')?.click());
        await page.waitForFunction(() => document.getElementById('app')?.classList.contains('knowledge-base-open'), { timeout: 3000 });
        await page.evaluate(() => document.querySelector('#btn-faq')?.click());
        await page.waitForFunction(
            () => document.getElementById('app')?.classList.contains('faq-open')
                && !document.getElementById('app')?.classList.contains('knowledge-base-open'),
            { timeout: 3000 }
        );
    });

    test('renders both sections plus an All pill', async ({ page }) => {
        await page.evaluate(() => document.querySelector('#btn-faq')?.click());
        await page.waitForSelector('#faq-sidebar-sections .sidelib-section-pill');
        const labels = (await page.locator('#faq-sidebar-sections .sidelib-section-pill').allTextContents()).map((l) => l.trim());
        expect(labels.some((l) => /all/i.test(l))).toBe(true);
        expect(labels).toContain('Physics');
        expect(labels).toContain('Foundations');
    });

    test('each rendered entry shows all four reader sections', async ({ page }) => {
        await page.evaluate(() => document.querySelector('#btn-faq')?.click());
        await page.waitForSelector('#faq-sidebar-list .sidelib-entry-chip');
        const entryIds = await page.locator('#faq-sidebar-list .sidelib-entry-chip')
            .evaluateAll((els) => els.map((e) => e.dataset.sidelibEntry));
        expect(entryIds.length).toBeGreaterThan(0);
        for (const id of entryIds) {
            await page.evaluate((eid) => document.querySelector(`#faq-sidebar-list [data-sidelib-entry="${eid}"]`)?.click(), id);
            await page.waitForFunction(
                () => document.querySelectorAll('#faq-sidebar-reader .faq-reader-section').length === 4,
                { timeout: 2000 }
            );
            const labels = await page.locator('#faq-sidebar-reader .faq-reader-section .faq-section-label').allTextContents();
            expect(labels.length).toBe(4);
            expect(labels[0]).toMatch(/problem/i);
            expect(labels[1]).toMatch(/mainstream/i);
            expect(labels[2]).toMatch(/angle/i);
            expect(labels[3]).toMatch(/open/i);
            const openBullets = await page.locator('#faq-sidebar-reader .faq-open-bullet').count();
            expect(openBullets).toBeGreaterThan(0);
        }
    });

    test('no claim-making verbs leak into the rendered panel', async ({ page }) => {
        await page.evaluate(() => document.querySelector('#btn-faq')?.click());
        await page.waitForSelector('#faq-sidebar .sidelib-body');
        const body = await page.locator('#faq-sidebar').innerText();
        expect(body).not.toMatch(/\bPASS\b/);
        expect(body).not.toMatch(/\bFAIL\b/);
        expect(body).not.toMatch(/\bSOLVES\b/i);
        expect(body).not.toMatch(/\bproves\b/i);
    });
});
