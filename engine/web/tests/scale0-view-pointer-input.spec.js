// @ts-check
/**
 * Pointer input over the Scale 0 view must go to something the user can see.
 *
 * An element at computed opacity 0 that still takes pointer events sits
 * above the 3D canvas as an unmarked dead zone: orbit drags, wheel zoom and
 * picks that start inside it never reach the view. Any such element is
 * reported with its box so the failure names what to fix.
 */
import { test, expect } from '@playwright/test';
import { bootDashboard } from './_helpers.js';

/** Fully transparent elements that are the pointer target somewhere over the view canvas. */
async function invisibleInputBlockers(page) {
    return page.evaluate(() => {
        const canvas = window.__ftdCtx?.viewport?.renderer?.domElement;
        if (!canvas) throw new Error('Scale 0 view canvas not found');
        const view = canvas.getBoundingClientRect();
        // Something opaque may cover a transparent element (an open dock panel
        // does); it only blocks the view where it is itself the pointer target.
        const pointerTargetIn = (root, part) => {
            const rect = part.getBoundingClientRect();
            const left = Math.max(rect.left, view.left);
            const right = Math.min(rect.right, view.right);
            const top = Math.max(rect.top, view.top);
            const bottom = Math.min(rect.bottom, view.bottom);
            if (right - left < 1 || bottom - top < 1) return null;
            for (const fy of [1 / 6, 1 / 2, 5 / 6]) {
                for (const fx of [1 / 6, 1 / 2, 5 / 6]) {
                    const x = left + (right - left) * fx;
                    const y = top + (bottom - top) * fy;
                    const target = document.elementFromPoint(x, y);
                    if (target && root.contains(target)) return { x: Math.round(x), y: Math.round(y) };
                }
            }
            return null;
        };
        const blockers = [];
        const reported = [];
        for (const root of document.querySelectorAll('body *')) {
            if (root.contains(canvas) || reported.some((seen) => seen.contains(root))) continue;
            if (parseFloat(getComputedStyle(root).opacity) !== 0) continue;
            // pointer-events is inherited, but a descendant can turn it back
            // on, so every part of the transparent subtree is probed.
            for (const part of [root, ...root.querySelectorAll('*')]) {
                if (getComputedStyle(part).pointerEvents === 'none') continue;
                const hit = pointerTargetIn(root, part);
                if (!hit) continue;
                const rect = root.getBoundingClientRect();
                blockers.push({
                    element: root.id ? `#${root.id}`
                        : `${root.tagName.toLowerCase()}.${[...root.classList].join('.')}`,
                    box: [rect.left, rect.top, rect.width, rect.height].map(Math.round),
                    takesInputAt: hit,
                });
                reported.push(root);
                break;
            }
        }
        return blockers;
    });
}

async function collapsePanels(page) {
    const collapsed = () => page.evaluate(
        () => document.getElementById('app')?.classList.contains('panels-collapsed') === true);
    if (!(await collapsed())) await page.locator('#btn-panel-toggle').click();
    await expect.poll(collapsed).toBe(true);
}

test('no transparent element takes pointer input over the Scale 0 view with panels collapsed', async ({ page }) => {
    test.setTimeout(90_000);
    await bootDashboard(page, { engine: 'wasm', requiredCapabilities: ['scale0'] });
    await expect(page.locator('#app')).toHaveAttribute('data-active-scale', '0');
    await collapsePanels(page);

    // Polled so an element caught mid fade-out is not mistaken for a
    // standing blocker; a standing one still fails after the window.
    await expect.poll(() => invisibleInputBlockers(page), { timeout: 5_000 }).toEqual([]);
});
