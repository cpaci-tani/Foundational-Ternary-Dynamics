// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const VIEWPORTS = [
    { width: 320, height: 568 },
    { width: 390, height: 844 },
    { width: 667, height: 375 },
    { width: 768, height: 1024 },
    { width: 1024, height: 768 },
    { width: 1280, height: 800 },
    { width: 1440, height: 900 },
    { width: 1920, height: 1080 },
    { width: 2512, height: 1188 },
];

async function toolbarGeometry(page) {
    return page.locator('#toolbar').evaluate((toolbar) => {
        const box = toolbar.getBoundingClientRect();
        const slots = [...toolbar.querySelectorAll(':scope > [data-topbar-slot]')]
            .filter((slot) => {
                const style = getComputedStyle(slot);
                const rect = slot.getBoundingClientRect();
                return style.display !== 'none' && style.visibility !== 'hidden'
                    && rect.width > 0 && rect.height > 0;
            })
            .map((slot) => {
                const rect = slot.getBoundingClientRect();
                return { name: slot.dataset.topbarSlot, centerY: rect.top + rect.height / 2, height: rect.height };
            });
        return { height: box.height, slots };
    });
}

async function scenarioMenuGeometry(page) {
    return page.locator('#scenario-picker .scenario-picker-menu').evaluate((menu) => {
        const rect = menu.getBoundingClientRect();
        return {
            left: rect.left, right: rect.right, top: rect.top, bottom: rect.bottom,
            pageOverflow: document.documentElement.scrollWidth - document.documentElement.clientWidth,
            viewportWidth: innerWidth, viewportHeight: innerHeight,
        };
    });
}

test('topbar controls stay in one row at phone through wide desktop widths', async ({ page }, testInfo) => {
    test.setTimeout(90_000);
    await page.addInitScript(() => {
        localStorage.setItem('ftd-theme', 'abyss');
        localStorage.setItem('ftd-gpu-card-dismissed', '1');
    });
    await page.setViewportSize(VIEWPORTS[VIEWPORTS.length - 1]);
    await gotoAndReady(page, { timeout: 45_000 });

    for (const viewport of VIEWPORTS) {
        await page.setViewportSize(viewport);
        await page.evaluate(() => new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve))));

        const geometry = await toolbarGeometry(page);
        const centers = geometry.slots.map((slot) => slot.centerY);
        const tallest = Math.max(...geometry.slots.map((slot) => slot.height));
        const label = `${viewport.width}x${viewport.height}: ${JSON.stringify(geometry)}`;
        expect(geometry.slots.length, label).toBeGreaterThan(1);
        expect(Math.max(...centers) - Math.min(...centers), label).toBeLessThanOrEqual(4);
        expect(geometry.height - tallest, label).toBeLessThanOrEqual(20);
        if (process.env.FTD_LAYOUT_SCREENSHOTS === '1' && [390, 1280, 2512].includes(viewport.width)) {
            await page.screenshot({ path: testInfo.outputPath(`topbar-${viewport.width}.png`), fullPage: true });
        }
    }

    await page.setViewportSize({ width: 1280, height: 800 });
    await page.locator('#scenario-picker > summary').click();
    const desktopMenu = await scenarioMenuGeometry(page);
    expect(desktopMenu.left).toBeGreaterThanOrEqual(-2);
    expect(desktopMenu.right).toBeLessThanOrEqual(desktopMenu.viewportWidth + 2);
    expect(desktopMenu.pageOverflow).toBeLessThanOrEqual(2);
    await page.locator('#scenario-picker input[type="search"]').fill('wave');
    await page.locator('#scenario-picker > summary').click();

    await page.setViewportSize({ width: 390, height: 844 });
    const menu = page.locator('#btn-toolbar-menu');
    await expect(menu).toBeVisible();
    const closedHeight = (await toolbarGeometry(page)).height;
    await menu.click();
    await expect(menu).toHaveAttribute('aria-expanded', 'true');
    const openedHeight = (await toolbarGeometry(page)).height;
    expect(openedHeight).toBeLessThanOrEqual(closedHeight + 2);
    if (process.env.FTD_LAYOUT_SCREENSHOTS === '1') {
        await page.screenshot({ path: testInfo.outputPath('topbar-menu-390.png'), fullPage: true });
    }
    await page.locator('#scenario-picker > summary').click();
    const mobileMenu = await scenarioMenuGeometry(page);
    expect(mobileMenu.left).toBeGreaterThanOrEqual(-2);
    expect(mobileMenu.right).toBeLessThanOrEqual(mobileMenu.viewportWidth + 2);
    expect(mobileMenu.pageOverflow).toBeLessThanOrEqual(2);
    await page.locator('#scenario-picker input[type="search"]').fill('wave');
});
