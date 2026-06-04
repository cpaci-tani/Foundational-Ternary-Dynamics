// @ts-check
import { test, expect } from '@playwright/test';

test.describe('Playback timeline smoke', () => {
    test.beforeEach(async ({ page }) => {
        await page.setViewportSize({ width: 1280, height: 720 });
        // Clear local storage to ensure fresh, predictable panel state before page loads
        await page.context().addInitScript(() => {
            window.localStorage.removeItem('ftd.panel.mount');
            window.localStorage.removeItem('ftd-panels-collapsed');
        });
    });

    test('single play button renders (no local button or labels)', async ({ page }) => {
        await page.goto('/?engine=mock');
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');

        const state = await page.evaluate(() => ({
            globalExists: !!document.getElementById('btn-play'),
            localExists:  !!document.getElementById('btn-local-play'),
            globalClass:  document.getElementById('btn-play')?.className ?? '',
            labelCount:   document.querySelectorAll('.tb-btn-label').length,
        }));
        expect(state.globalExists).toBe(true);
        expect(state.localExists).toBe(false);
        expect(state.globalClass).toContain('tb-btn-global');
        expect(state.labelCount).toBe(0);
    });

    test('scrub bar mounts as a compact capsule without timeline elements', async ({ page }) => {
        await page.goto('/?engine=mock');
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');

        // Ensure panels aren't collapsed — the scrub bar hides in that state.
        await page.evaluate(() => {
            const app = document.getElementById('app');
            if (app && app.classList.contains('panels-collapsed')) {
                document.getElementById('btn-panel-toggle')?.click();
            }
        });

        const report = await page.evaluate(() => {
            const bar    = document.getElementById('scrub-bar');
            const strip  = bar?.querySelector('.scrub-bar-strip');
            const zones  = bar?.querySelector('.scrub-bar-zones');
            const render = bar?.querySelector('.scrub-bar-render-btn');
            const ph     = bar?.querySelector('.scrub-bar-playhead');
            return {
                barMounted: !!bar,
                hasStrip: !!strip,
                hasRenderBtn: !!render,
                hasZones: !!zones,
                hasPlayhead: !!ph,
            };
        });
        expect(report.barMounted).toBe(true);
        expect(report.hasStrip).toBe(false);
        expect(report.hasRenderBtn).toBe(false);
        expect(report.hasZones).toBe(false);
        expect(report.hasPlayhead).toBe(false);
    });
});
