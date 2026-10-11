// @ts-check
/** The top bar's brand is the startup splash's lattice cube, drawn at bar size. */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

/** What the brand canvas holds right now. */
async function readLogo(page) {
    return page.evaluate(() => {
        const brand = document.querySelector('#toolbar .brand');
        const canvas = brand.querySelector('canvas.brand-lattice');
        const bar = document.getElementById('toolbar').getBoundingClientRect();
        const box = canvas.getBoundingClientRect();
        const data = canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height).data;
        let inked = 0;
        let border = 0;
        let hash = 0;
        for (let y = 0; y < canvas.height; y++) {
            for (let x = 0; x < canvas.width; x++) {
                const alpha = data[(y * canvas.width + x) * 4 + 3];
                hash = (hash * 31 + alpha) >>> 0;
                if (alpha <= 24) continue;
                inked += 1;
                if (x === 0 || y === 0 || x === canvas.width - 1 || y === canvas.height - 1) border += 1;
            }
        }
        return {
            text: brand.textContent.trim(),
            css: [Math.round(box.width), Math.round(box.height)],
            pixels: [canvas.width, canvas.height],
            ratio: window.devicePixelRatio,
            inside: box.top >= bar.top && box.bottom <= bar.bottom && box.left >= bar.left,
            inked: inked / (canvas.width * canvas.height),
            border,
            hash,
        };
    });
}

test('the top bar shows the turning lattice cube in place of the product name', async ({ page }) => {
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    await expect(page.getByRole('heading', { name: 'FTD Engine', level: 1 })).toBeVisible();

    const logo = await readLogo(page);
    expect(logo.text, 'no lettering in the bar').toBe('');
    expect(logo.css[0]).toBe(logo.css[1]);
    expect(logo.css[0]).toBeGreaterThanOrEqual(40);
    expect(logo.pixels[0], 'drawn at the screen\'s pixel density').toBe(Math.round(logo.css[0] * Math.min(3, logo.ratio)));
    expect(logo.inside, 'inside the bar').toBe(true);
    expect(logo.inked, 'a cube is drawn').toBeGreaterThan(0.2);
    expect(logo.border, 'and none of it is cut off').toBe(0);

    // It turns, as on the splash.
    await expect.poll(async () => (await readLogo(page)).hash, { timeout: 5_000 }).not.toBe(logo.hash);
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('with reduced motion the cube is drawn once and stays still', async ({ page }) => {
    await page.emulateMedia({ reducedMotion: 'reduce' });
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    const first = await readLogo(page);
    expect(first.inked).toBeGreaterThan(0.2);
    await page.waitForTimeout(900);
    expect((await readLogo(page)).hash).toBe(first.hash);
});
