// @ts-check
import { test, expect } from '@playwright/test';

test('every panel descriptor exposes a unicode icon glyph', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const report = await page.evaluate(async () => {
        const { PANEL_REGISTRY } = await import('/js/ui/scale-registry/panel-registry.js');
        return PANEL_REGISTRY.map((p) => ({ id: p.id, icon: p.icon }));
    });

    for (const entry of report) {
        expect(entry.icon, `panel "${entry.id}" must declare an icon`).toBeTruthy();
        expect(typeof entry.icon).toBe('string');
        expect(entry.icon.length).toBeGreaterThan(0);
    }
});

test('tab bar renders icons alongside labels', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const tabs = await page.$$eval('#tab-bar .tab', (els) => els.map((el) => ({
        panel: el.dataset.panel,
        icon: el.querySelector('.tab-icon')?.textContent || '',
        label: el.querySelector('.tab-label')?.textContent || '',
    })));

    expect(tabs.length).toBeGreaterThan(0);
    for (const tab of tabs) {
        expect(tab.icon, `tab "${tab.panel}" must render an icon node`).not.toBe('');
        expect(tab.label, `tab "${tab.panel}" must render a label node`).not.toBe('');
    }
});
