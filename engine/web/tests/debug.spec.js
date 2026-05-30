import { test, expect } from '@playwright/test';

test('debug coordinates', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 15_000 }).toBe(true);

    const coords = await page.evaluate(() => {
        const btnPlay = document.getElementById('btn-play');
        const tabBar = document.getElementById('tab-bar');
        const diagnosticsTab = Array.from(document.querySelectorAll('#tab-bar .tab')).find(el => el.textContent.includes('Diagnostics'));

        const getRect = el => {
            if (!el) return null;
            const r = el.getBoundingClientRect();
            return { left: r.left, top: r.top, width: r.width, height: r.height };
        };

        return {
            viewport: { width: window.innerWidth, height: window.innerHeight },
            btnPlay: getRect(btnPlay),
            tabBar: getRect(tabBar),
            diagnosticsTab: getRect(diagnosticsTab),
            bodyScroll: { top: window.scrollY, left: window.scrollX },
            htmlMount: document.documentElement.getAttribute('data-panel-mount') || 'none',
        };
    });

    console.log('COORDINATES:', JSON.stringify(coords, null, 2));
});
