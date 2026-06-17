// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

test.describe('Scale 2 scenario registry', () => {
    test('defaults to hydrogen demo without duplicate ae-el-1', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const state = await page.evaluate(async () => {
            const reg = await import('./js/scales/scale2/scenario-registry.js');
            const sel = document.getElementById('ae-scenario-select');
            const options = sel ? Array.from(sel.options).map((o) => o.value) : [];
            return {
                defaultScenario: reg.AE_DEFAULT_SCENARIO,
                selectValue: sel?.value,
                hasHydrogenDemo: options.includes('ae-hydrogen-atom'),
                hasDuplicateEl1: options.includes('ae-el-1'),
                descVisible: getComputedStyle(document.getElementById('ae-scenario-desc')).display !== 'none',
                descText: document.getElementById('ae-scenario-desc-text')?.textContent?.trim() || '',
            };
        });

        expect(state.defaultScenario).toBe('ae-hydrogen-atom');
        expect(state.selectValue).toBe('ae-hydrogen-atom');
        expect(state.hasHydrogenDemo).toBe(true);
        expect(state.hasDuplicateEl1).toBe(false);
        expect(state.descText.length).toBeGreaterThan(20);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
