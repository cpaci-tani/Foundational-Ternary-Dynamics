/* global window */
import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

test('simulation dropdown exposes scale buttons and a separate Observer launch action', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    const trigger = page.locator('#simulation-menu-trigger');
    await expect(trigger).toBeVisible();
    await expect(page.locator('#engine-mode')).toBeHidden();
    await expect(page.locator('#simulation-menu')).toBeHidden();
    await expect(page.locator('#engine-mode option[value="observer"]')).toHaveCount(0);
    await trigger.click();
    await expect(page.locator('#simulation-menu')).toBeVisible();
    await expect(page.locator('#simulation-menu [data-simulation-mode]')).toHaveCount(6);
    await expect(page.locator('#simulation-menu #btn-observer-workspace')).toHaveRole('button');
    await page.locator('[data-simulation-mode="particles"]').click();
    await page.waitForFunction(() => window.__ftdCtx.engineMode === 'particles');
    await expect(trigger).toContainText('Scale 1 (Particles)');
    await expect(page.locator('#simulation-menu')).toBeHidden();
    await trigger.click();
    await expect(page.locator('[data-simulation-mode="particles"]')).toHaveAttribute('aria-pressed', 'true');
    await page.locator('[data-simulation-mode="lattice"]').click();
    await page.waitForFunction(() => window.__ftdCtx.engineMode === 'lattice');
    await expect(trigger).toContainText('Scale 0 (Lattice)');
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await expect(trigger).toHaveAttribute('aria-expanded', 'false');
    expect(await page.locator('#simulation-menu').evaluate(panel => panel.matches(':popover-open'))).toBe(false);
    await expect(page.locator('#engine-mode')).toHaveValue('lattice');
    await page.getByRole('button', { name: '← Lattice Sim', exact: true }).click();
    await expect(page.locator('#observer-workspace')).toBeHidden();
    await expect(trigger).toBeFocused();
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.getByRole('button', { name: '← Lattice Sim', exact: true }).click();
    await expect(trigger).toBeFocused();
});

test('simulation dropdown supports keyboard traversal, dismissal, and opening the Observer', async ({ page }, testInfo) => {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    const trigger = page.locator('#simulation-menu-trigger');
    await trigger.focus();
    await page.keyboard.press('ArrowDown');
    await expect(page.locator('[data-simulation-mode="lattice"]')).toBeFocused();
    await page.keyboard.press('End');
    await expect(page.locator('#btn-observer-workspace')).toBeFocused();
    await page.keyboard.press('Home');
    await expect(page.locator('[data-simulation-mode="lattice"]')).toBeFocused();
    await page.keyboard.press('Escape');
    await expect(page.locator('#simulation-menu')).toBeHidden();
    await expect(trigger).toBeFocused();
    await page.keyboard.press('ArrowUp');
    await expect(page.locator('#btn-observer-workspace')).toBeFocused();
    await page.screenshot({ path: testInfo.outputPath('simulation-dropdown.png') });
    await page.keyboard.press('Enter');
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.getByRole('button', { name: '← Lattice Sim', exact: true }).click();
    await expect(trigger).toBeFocused();
});
