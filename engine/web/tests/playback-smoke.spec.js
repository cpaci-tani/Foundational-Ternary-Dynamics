// @ts-check
import { test, expect } from '@playwright/test';

test.describe('Playback timeline smoke', () => {
    test.beforeEach(async ({ page }) => {
        await page.setViewportSize({ width: 1280, height: 720 });
    });

    test('play buttons render with labels and distinct classes', async ({ page }) => {
        await page.goto('/');
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');

        const state = await page.evaluate(() => {
            const g = document.getElementById('btn-play');
            const l = document.getElementById('btn-local-play');
            const labels = [...document.querySelectorAll('.tb-btn-labeled .tb-btn-label')]
                .map((s) => s.textContent?.trim());
            return {
                globalExists: !!g,
                localExists: !!l,
                globalClass: g?.className ?? '',
                localClass:  l?.className ?? '',
                labels,
            };
        });
        expect(state.globalExists).toBe(true);
        expect(state.localExists).toBe(true);
        expect(state.globalClass).toContain('tb-btn-global');
        expect(state.localClass).toContain('tb-btn-local');
        expect(state.labels).toContain('global');
        expect(state.labels).toContain('local');
    });

    test('scrub bar mounts with memory zones after sim runs briefly', async ({ page }) => {
        await page.goto('/');
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');

        // Ensure panels aren't collapsed — the scrub bar hides in that state.
        await page.evaluate(() => document.getElementById('app')?.classList.remove('panels-collapsed'));

        // Make sure the sim is running.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn?.dataset.paused === 'true') btn.click();
        });
        await page.waitForTimeout(3000);

        const report = await page.evaluate(() => {
            const bar    = document.getElementById('scrub-bar');
            const strip  = bar?.querySelector('.scrub-bar-strip');
            const zones  = bar?.querySelectorAll('.scrub-bar-zone');
            const render = bar?.querySelector('.scrub-bar-render-btn');
            const ph     = bar?.querySelector('.scrub-bar-playhead');
            return {
                barMounted: !!bar,
                hasStrip: !!strip,
                hasRenderBtn: !!render,
                zoneCount: zones?.length ?? 0,
                playheadAt: ph?.style.left ?? null,
            };
        });
        expect(report.barMounted).toBe(true);
        expect(report.hasStrip).toBe(true);
        expect(report.hasRenderBtn).toBe(true);
        expect(report.zoneCount).toBeGreaterThan(0);
    });

    test('render chip appears, progresses, and can be cancelled', async ({ page }) => {
        await page.goto('/');
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');

        await page.waitForFunction(() => typeof window.__ftdStartRender === 'function');
        await page.evaluate(() => window.__ftdStartRender(3));

        await page.waitForFunction(() => {
            const chip = document.getElementById('render-chip');
            return chip && !chip.hidden;
        }, { timeout: 5000 });

        await page.evaluate(() => document.querySelector('.render-chip-cancel')?.click());
        await page.waitForTimeout(500);

        const afterCancel = await page.evaluate(() => document.getElementById('render-chip')?.hidden);
        expect(afterCancel).toBe(true);
    });
});
