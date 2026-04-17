// @ts-check
/**
 * Full integration smoke test for the panel-mount feature.
 * Exercises click cycling, keyboard shortcuts, reload persistence, and
 * overlay side-swap across all three mounts.
 */
import { test, expect } from '@playwright/test';

const MOUNTS = ['left', 'bottom', 'right'];

test.afterEach(async ({ page }) => {
    await page.evaluate(() => {
        localStorage.removeItem('ftd.panel.mount');
    });
});

test('cycling all three mounts via click updates attribute, storage, and aria-pressed', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    for (const mount of MOUNTS) {
        await page.evaluate((m) =>
            document.querySelector(`[data-panel-mount-toggle] button[data-mount="${m}"]`).click()
        , mount);
        await page.waitForTimeout(50);

        const state = await page.evaluate(() => ({
            attr:   document.documentElement.dataset.panelMount,
            stored: localStorage.getItem('ftd.panel.mount'),
        }));
        expect(state.attr).toBe(mount);
        expect(state.stored).toBe(mount);

        const pressedBtn = await page.evaluate((m) => {
            const btn = document.querySelector(`[data-panel-mount-toggle] button[data-mount="${m}"]`);
            return btn?.getAttribute('aria-pressed');
        }, mount);
        expect(pressedBtn).toBe('true');

        const otherPressed = await page.evaluate((m) => {
            return Array.from(document.querySelectorAll('[data-panel-mount-toggle] button'))
                .filter((b) => b.dataset.mount !== m)
                .every((b) => b.getAttribute('aria-pressed') === 'false');
        }, mount);
        expect(otherPressed).toBe(true);
    }
});

test('keyboard shortcuts cycle all three mounts', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const shortcutMap = [
        { key: 'ArrowLeft',  mount: 'left' },
        { key: 'ArrowRight', mount: 'right' },
        { key: 'ArrowDown',  mount: 'bottom' },
    ];

    for (const { key, mount } of shortcutMap) {
        await page.keyboard.press(`Control+Shift+${key}`);
        const attr = await page.evaluate(() => document.documentElement.dataset.panelMount);
        expect(attr).toBe(mount);
    }
});

test('reload restores persisted mount', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('right');
    });

    await page.reload();
    await page.waitForTimeout(400);

    const attr = await page.evaluate(() => document.documentElement.dataset.panelMount);
    expect(attr).toBe('right');
});

test('right mount moves overlay panels to the left edge', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const leftPx = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('right');
        await new Promise((r) => requestAnimationFrame(r));
        const el = document.getElementById('viewport-overlay') || document.querySelector('.viewport-overlay-panel');
        if (!el) return null;
        return el.getBoundingClientRect().left;
    });

    // Overlay must be near the left edge (within first 30% of screen)
    if (leftPx !== null) {
        expect(leftPx).toBeLessThan(window.screen.availWidth * 0.3 + 100);
    }
});

test('switching mounts does not break existing panel tab activation', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    for (const mount of MOUNTS) {
        await page.evaluate((m) =>
            document.querySelector(`[data-panel-mount-toggle] button[data-mount="${m}"]`).click()
        , mount);
        await page.waitForTimeout(50);

        // Controls panel must still be reachable
        const controlsTab = await page.evaluate(() => {
            const tab = document.querySelector('#tab-bar .tab[data-panel="controls"]');
            return tab ? { display: tab.style.display, ariaSelected: tab.getAttribute('aria-selected') } : null;
        });
        expect(controlsTab).not.toBeNull();
        expect(controlsTab.display).not.toBe('none');
    }
});

test('panel area is visible and not zero-sized in all three mounts', async ({ page }) => {
    await page.setViewportSize({ width: 1440, height: 900 });
    await page.goto('/');
    await page.waitForTimeout(800);

    for (const mount of MOUNTS) {
        await page.evaluate((m) =>
            document.querySelector(`[data-panel-mount-toggle] button[data-mount="${m}"]`).click()
        , mount);
        await page.waitForTimeout(80);

        const rect = await page.evaluate(() => {
            const el = document.getElementById('panel-area');
            const r = el.getBoundingClientRect();
            return { width: r.width, height: r.height };
        });

        expect(rect.width, `panel-area width in ${mount} mount`).toBeGreaterThan(0);
        expect(rect.height, `panel-area height in ${mount} mount`).toBeGreaterThan(0);
    }
});
