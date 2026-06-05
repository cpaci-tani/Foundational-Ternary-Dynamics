// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

async function activeScale0Tick(page) {
    return page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const active = (state.useFluxMock && state.fluxMock) ? state.fluxMock : window.__ftdCtx?.bridge;
        return active?.capabilities?.scale0?.getScale0Diagnostics?.()?.tick ?? null;
    });
}

test.describe('Playback timeline smoke', () => {
    test.beforeEach(async ({ page }) => {
        await page.setViewportSize({ width: 1280, height: 720 });
        // Clear local storage to ensure fresh, predictable panel state before page loads
        await page.context().addInitScript(() => {
            window.localStorage.removeItem('ftd.panel.mount');
            window.localStorage.removeItem('ftd-panels-collapsed');
        });
    });

    test('single play button renders with compact transport controls', async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=mock', timeout: 30_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true', { timeout: 30_000 });

        const state = await page.evaluate(() => ({
            globalExists: !!document.getElementById('btn-play'),
            localExists:  !!document.getElementById('btn-local-play'),
            globalClass:  document.getElementById('btn-play')?.className ?? '',
            speedNudges:  document.querySelectorAll('[data-speed-nudge]').length,
            speedInput:   !!document.getElementById('ticks-per-frame'),
            speedDisplay: !!document.getElementById('tpf-display'),
            timeBadge:    document.querySelector('.scrub-bar-time')?.textContent ?? '',
            labelCount:   document.querySelectorAll('.scrub-bar .tb-btn-label').length,
        }));
        expect(state.globalExists).toBe(true);
        expect(state.localExists).toBe(false);
        expect(state.globalClass).toContain('tb-btn-global');
        expect(state.speedNudges).toBe(2);
        expect(state.speedInput).toBe(true);
        expect(state.speedDisplay).toBe(true);
        expect(state.timeBadge).toMatch(/^T /);
        expect(state.labelCount).toBe(0);
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

    test('speed nudge buttons drive the existing ticks-per-frame input', async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=mock', timeout: 30_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true', { timeout: 30_000 });

        const before = await page.locator('#ticks-per-frame').inputValue();
        await page.locator('[data-speed-nudge="5"]').click();

        const after = await page.locator('#ticks-per-frame').inputValue();
        const display = await page.locator('#tpf-display').textContent();
        expect(Number(after)).toBeGreaterThan(Number(before));
        expect(display?.trim()).not.toBe('');
    });

    test('tick buttons advance the active Scale 0 source while paused', async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=mock', timeout: 30_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true', { timeout: 30_000 });
        await page.waitForTimeout(800);

        const before = await activeScale0Tick(page);
        await page.locator('#btn-step').click();

        await expect.poll(() => activeScale0Tick(page), { timeout: 5_000 }).toBeGreaterThan(before);
        const afterMainStep = await activeScale0Tick(page);

        await page.locator('.scrub-bar-settings').click();
        await page.locator('[data-step-by="10"]').click();
        await expect.poll(() => activeScale0Tick(page), { timeout: 5_000 }).toBeGreaterThanOrEqual(afterMainStep + 10);

        await expect(page.locator('.scrub-bar-time')).toContainText(/^T /);
    });
});
