// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Playback timeline smoke', () => {
    test.beforeEach(async ({ page }) => {
        await page.setViewportSize({ width: 1280, height: 720 });
        // Clear local storage to ensure fresh, predictable panel state before page loads
        await page.context().addInitScript(() => {
            window.localStorage.removeItem('ftd.panel.mount');
            window.localStorage.removeItem('ftd-panels-collapsed');
        });
    });

    test('single play button renders, captioned, no local button', async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=mock', timeout: 30_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true', { timeout: 30_000 });

        const state = await page.evaluate(() => ({
            globalExists: !!document.getElementById('btn-play'),
            localExists:  !!document.getElementById('btn-local-play'),
            globalClass:  document.getElementById('btn-play')?.className ?? '',
            labelCount:   document.querySelectorAll('.tb-btn-label').length,
        }));
        expect(state.globalExists).toBe(true);
        expect(state.localExists).toBe(false);
        expect(state.globalClass).toContain('tb-btn-global');
        expect(state.labelCount).toBe(3); // play/pause, step, reset
    });

    test('scrub bar mounts as a compact capsule without timeline elements', async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=mock', timeout: 30_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true', { timeout: 30_000 });

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

    test('playback controls remain clickable after switching from side to bottom mount', async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=mock', timeout: 30_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true', { timeout: 30_000 });
        await page.waitForTimeout(600);

        await expect(page.locator('html')).toHaveAttribute('data-panel-mount', 'left');
        await page.locator('[data-panel-mount-toggle] button[data-mount="bottom"]').click();
        await page.waitForTimeout(300);

        const hit = await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            const rect = btn?.getBoundingClientRect();
            if (!rect) return { closestPlay: false, targetId: null, targetClass: null };
            const el = document.elementFromPoint(rect.left + rect.width / 2, rect.top + rect.height / 2);
            return {
                closestPlay: !!el?.closest?.('#btn-play'),
                targetId: el?.id ?? null,
                targetClass: String(el?.className ?? ''),
            };
        });
        expect(hit.closestPlay, `play button center was covered by ${hit.targetId || hit.targetClass}`).toBe(true);

        await page.locator('#btn-play').click();
        await expect(page.locator('#btn-play')).toHaveAttribute('data-paused', 'false');
    });
});
