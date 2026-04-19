// @ts-check
import { test, expect } from '@playwright/test';

async function bootShell(page) {
    await page.goto('/');
    await page.waitForFunction(() => document.getElementById('app')?.dataset?.shellReady === 'true', { timeout: 10000 });
    await page.waitForFunction(
        () => document.getElementById('loading-overlay')?.classList.contains('hidden'),
        { timeout: 15000 }
    );
}

async function openPanel(page, buttonSelector, openClass) {
    await page.evaluate((sel) => document.querySelector(sel)?.click(), buttonSelector);
    await page.waitForFunction((cls) => document.getElementById('app')?.classList.contains(cls), openClass, { timeout: 3000 });
}

test.describe('Math formatting coverage', () => {
    test.beforeEach(async ({ page }) => {
        await bootShell(page);
    });

    test('KaTeX global is loaded', async ({ page }) => {
        const hasKatex = await page.evaluate(() => typeof window.katex === 'object' && typeof window.katex.renderToString === 'function');
        expect(hasKatex).toBe(true);
    });

    test('FAQ renders .katex spans and leaks no raw delimiters', async ({ page }) => {
        await openPanel(page, '#btn-faq', 'faq-open');
        await page.waitForSelector('#faq-sidebar .faq-reader-section');
        const katexCount = await page.locator('#faq-sidebar .katex').count();
        expect(katexCount).toBeGreaterThan(0);
        const body = await page.locator('#faq-sidebar').innerText();
        expect(body, 'FAQ contains raw \\\\( — LaTeX was not rendered').not.toMatch(/\\\(/);
        expect(body, 'FAQ contains raw \\\\[ — display LaTeX was not rendered').not.toMatch(/\\\[/);
    });

    test('KB renders .katex spans and leaks no raw delimiters', async ({ page }) => {
        await openPanel(page, '#btn-knowledge-base', 'knowledge-base-open');
        await page.waitForSelector('#kb-sidebar .sidelib-entry-chip, #kb-sidebar .sidelib-empty-list', { timeout: 5000 });
        const katexCount = await page.locator('#kb-sidebar .katex').count();
        expect(katexCount).toBeGreaterThan(0);
        const body = await page.locator('#kb-sidebar').innerText();
        expect(body, 'KB contains raw \\\\( — LaTeX was not rendered').not.toMatch(/\\\(/);
        expect(body, 'KB contains raw \\\\[ — display LaTeX was not rendered').not.toMatch(/\\\[/);
    });

    test('Verify panel leaks no raw delimiters', async ({ page }) => {
        await page.evaluate(() => document.querySelector('#tab-bar .tab[data-panel="verification-lab"]')?.click());
        await page.waitForSelector('.verify-header', { timeout: 10000 });
        const body = await page.locator('#panel-verification-lab').innerText();
        expect(body).not.toMatch(/\\\(/);
        expect(body).not.toMatch(/\\\[/);
    });

    test('tooltip system leaks no raw delimiters', async ({ page }) => {
        await page.hover('#btn-knowledge-base');
        await page.waitForTimeout(400);
        const tip = await page.locator('#ui-tooltip').innerText().catch(() => '');
        expect(tip).not.toMatch(/\\\(/);
        expect(tip).not.toMatch(/\\\[/);
    });
});
