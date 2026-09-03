// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const select = document.getElementById('pe-scenario-select');
        if (!select) throw new Error('Scale 1 scenario selector missing');
        select.value = scenarioId;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

async function appearanceSnapshot(page) {
    return page.evaluate(() => {
        const context = window.__ftdCtx;
        const renderer = context?.viewport?._particleRenderer;
        const geometry = renderer?.particles?.geometry;
        const count = Number(geometry?.drawRange?.count || 0);
        const roles = geometry?.getAttribute('appearanceRole')?.array || [];
        const focus = geometry?.getAttribute('focusWeight')?.array || [];
        const sizes = geometry?.getAttribute('size')?.array || [];
        const positions = geometry?.getAttribute('position')?.array || [];
        let coreCount = 0;
        let supportCount = 0;
        let rimCount = 0;
        let focusedCoreCount = 0;
        let minCoreSize = Number.POSITIVE_INFINITY;
        const stablePrefix = [];
        for (let i = 0; i < count; i++) {
            if (roles[i] > 1.5) rimCount++;
            else if (roles[i] > 0.5) {
                coreCount++;
                if (focus[i] > 0.5) focusedCoreCount++;
                minCoreSize = Math.min(minCoreSize, sizes[i]);
            } else {
                supportCount++;
            }
            if (stablePrefix.length < 36) {
                stablePrefix.push(
                    Math.round(positions[i * 3] * 1e4) / 1e4,
                    Math.round(positions[i * 3 + 1] * 1e4) / 1e4,
                    Math.round(positions[i * 3 + 2] * 1e4) / 1e4,
                    roles[i],
                );
            }
        }
        return {
            logicalCount: Number(context?.bridge?.peGetParticleData?.()?.count || 0),
            count,
            coreCount,
            supportCount,
            rimCount,
            focusedCoreCount,
            minCoreSize: Number.isFinite(minCoreSize) ? minCoreSize : 0,
            hasRoleAttribute: !!geometry?.getAttribute('appearanceRole'),
            hasFocusAttribute: !!geometry?.getAttribute('focusWeight'),
            stablePrefix,
        };
    });
}

test.describe('Scale 1 particle appearance language', () => {
    test('renders a persistent record core, localization support, and support rim', async ({ page }) => {
        page.setDefaultTimeout(20_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectScenario(page, 's1-mass-ladder');

        await expect.poll(() => page.evaluate(() =>
            window.__ftdCtx?.bridge?.peGetParticleData?.()?.count || 0)).toBe(4);
        await expect.poll(() => appearanceSnapshot(page).then(value => value.coreCount)).toBe(4);

        const initial = await appearanceSnapshot(page);
        expect(initial.hasRoleAttribute).toBe(true);
        expect(initial.hasFocusAttribute).toBe(true);
        expect(initial.logicalCount).toBe(4);
        expect(initial.coreCount).toBe(initial.logicalCount);
        expect(initial.supportCount).toBeGreaterThan(initial.logicalCount * 28);
        expect(initial.rimCount).toBeGreaterThanOrEqual(initial.logicalCount * 12);
        expect(initial.minCoreSize).toBeGreaterThanOrEqual(8.4);

        const appearanceKey = page.locator('.pe-appearance-key');
        await expect(appearanceKey).toBeVisible();
        await expect(appearanceKey.locator('.pe-appearance-key-item')).toHaveCount(4);
        await expect(appearanceKey).toContainText('Record core');
        await expect(appearanceKey).toContainText('Localization');
        await expect(appearanceKey).toContainText('Support rim');
        await expect(appearanceKey).toContainText('Activity phase');

        // A static scenario must regenerate the same support geometry instead
        // of a new Math.random cloud on every load.
        await selectScenario(page, 's1-mass-ladder');
        await expect.poll(() => appearanceSnapshot(page).then(value => value.coreCount)).toBe(4);
        const reloaded = await appearanceSnapshot(page);
        expect(reloaded.stablePrefix).toEqual(initial.stablePrefix);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('marks only the selected effective-record core', async ({ page }) => {
        page.setDefaultTimeout(20_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectScenario(page, 's1-mass-ladder');
        await expect.poll(() => page.evaluate(() =>
            window.__ftdCtx?.bridge?.peGetParticleData?.()?.count || 0)).toBe(4);

        const firstId = await page.evaluate(() => Number(
            window.__ftdCtx.bridge.peGetParticleData().ids[0]));
        await page.evaluate((particleId) =>
            window.__ftdCtx.inspector.selectPEParticle(particleId), firstId);

        await expect.poll(() => appearanceSnapshot(page)
            .then(value => value.focusedCoreCount)).toBe(1);
        const focused = await appearanceSnapshot(page);
        expect(focused.coreCount).toBe(4);
        expect(focused.focusedCoreCount).toBe(1);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
