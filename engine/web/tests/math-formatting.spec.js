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

test.describe('Math formatting coverage', () => {
    test.beforeEach(async ({ page }) => {
        await bootShell(page);
    });

    test('KaTeX global is loaded', async ({ page }) => {
        const hasKatex = await page.evaluate(() => typeof window.katex === 'object' && typeof window.katex.renderToString === 'function');
        expect(hasKatex).toBe(true);
    });

    test('shared formatter renders inline and display math', async ({ page }) => {
        const html = await page.evaluate(async () => {
            const { renderMathInHtml } = await import('/js/ui/math-format/render.js');
            return renderMathInHtml('Inline \\(x^2\\) and display \\[y^2\\]');
        });
        expect(html.match(/class="katex/g)?.length).toBeGreaterThanOrEqual(2);
        expect(html).not.toMatch(/\\\(|\\\[/);
    });

    test('tooltip system leaks no raw delimiters', async ({ page }) => {
        await page.hover('#btn-settings');
        await page.waitForTimeout(400);
        const tip = await page.locator('#ui-tooltip').innerText().catch(() => '');
        expect(tip).not.toMatch(/\\\(/);
        expect(tip).not.toMatch(/\\\[/);
    });
});
