import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test('dock tabs support keyboard navigation and activation', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    const controls = page.locator('#tab-bar .tab[data-panel="controls"]');
    const jev = page.locator('#tab-bar .tab[data-panel="jev"]');

    await controls.focus();
    await controls.press('ArrowDown');
    await expect(jev).toHaveAttribute('aria-selected', 'false');
    await expect(jev).toBeFocused();

    await jev.press('Home');
    await expect(controls).toHaveAttribute('aria-selected', 'true');
    await expect(controls).toBeFocused();
    await controls.press('End');
    const lastTab = page.locator('#tab-bar .tab:visible').last();
    await expect(lastTab).toBeFocused();
    await lastTab.press('Enter');
    await expect(lastTab).toHaveAttribute('aria-selected', 'true');

    await controls.focus();
    await controls.press('ArrowDown');
    await jev.press(' ');
    await expect(jev).toHaveAttribute('aria-selected', 'true');
    await expect(page.locator('#panel-jev')).toHaveClass(/active/);
});

test('collapsed mobile sheet has an accessible opener and no focusable hidden controls', async ({ page }) => {
    await page.setViewportSize({ width: 390, height: 844 });
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    const sheet = page.locator('#panel-area');
    const openButton = page.getByRole('button', { name: 'Show simulation panels' });
    const selector = page.locator('#tab-select-mobile');

    await page.locator('#gpu-card-close').click();
    await expect(openButton).toBeHidden();
    await page.locator('#btn-panel-hide-mobile').click();
    await expect(sheet).toHaveAttribute('inert', '');
    await expect(sheet).toHaveAttribute('aria-hidden', 'true');
    await expect(openButton).toBeVisible();
    await expect(openButton).toBeFocused();
    await expect(selector).not.toBeFocused();

    await openButton.click();
    await expect(sheet).not.toHaveAttribute('inert');
    await expect(sheet).toHaveAttribute('aria-hidden', 'false');
    await expect(selector).toBeFocused();
    await expect(openButton).toBeHidden();
});

test('JEV is desktop-only and mobile resize selects Controls', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await page.locator('#tab-bar .tab[data-panel="jev"]').click();
    await expect(page.locator('#panel-jev')).toHaveClass(/active/);

    await page.setViewportSize({ width: 390, height: 844 });
    await expect(page.locator('#panel-controls')).toHaveClass(/active/);
    const jevOption = page.locator('#tab-select-mobile option[value="jev"]');
    await expect(jevOption).toHaveAttribute('disabled', '');
    await expect(jevOption).toHaveAttribute('hidden', '');
    await expect(page.locator('#panel-jev')).toBeHidden();

    await page.setViewportSize({ width: 1024, height: 768 });
    await expect(jevOption).not.toHaveAttribute('disabled');
    await page.locator('#tab-bar .tab[data-panel="jev"]').click();
    await expect(page.locator('#panel-jev')).toHaveClass(/active/);
});
