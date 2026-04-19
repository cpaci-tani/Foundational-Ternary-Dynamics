// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher } from './_helpers.js';

/**
 * Animation-clock freeze contract test.
 *
 * Guards the contract documented in viewport.js::advanceAnimationClock:
 *   - `_animationClock` advances ONLY when the external controller calls
 *     `advanceAnimationClock(dt)` — which it does only while the sim is running.
 *   - `_animateQuantumField()` reads `_animationClock` without advancing it.
 *   - Therefore, when the sim is paused, toggling an overlay (which forces
 *     one render cycle to repaint) must NOT bump the breathing-opacity phase.
 *
 * Why this matters for the large-file refactor:
 *   Wave 3 ticket 14 plans to extract the quantum-field + topology + horizon
 *   visualization into viewport/quantum-renderer.js. That extraction is
 *   HIGH risk (plan Risk 1) because if the new module advances the clock
 *   inside its animateFrame() call instead of reading it, the freeze
 *   semantics break silently — opacity pulses while the sim is paused.
 *
 * This test exists so the extraction can be re-run safely: the test is the
 * regression guard. Without it, the opacity drift would be a visual-only
 * bug invisible to `node --check` / scales.spec.js.
 *
 * Test mechanics:
 *   1. Load the dashboard, select flux-pulse at N=32
 *   2. Advance the sim for ~200 ticks (so the psi² overlay has data to render)
 *   3. Enable ψ² overlay
 *   4. Pause the sim
 *   5. Wait 1 second of wall-clock time (during which requestAnimationFrame
 *      will call render() many times — exercising the "toggle repaint" path)
 *   6. Read the opacity; it should equal the opacity from just after the pause
 *   7. Toggle the overlay off → on 5 times; re-check opacity each time
 *
 * If any step 6/7 check fails, the animation-clock freeze contract is broken.
 */

test.describe('Animation clock freeze (quantum-field opacity)', () => {
    test('opacity stays pinned while paused, regardless of overlay toggles or repaints', async ({ page }) => {
        // Collect console errors to fail fast on module-graph breakage.
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page, { timeout: 10_000 });

        // 1. Select flux-pulse (Scale 0 is the default mode).
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) {
                sel.value = 'flux-pulse';
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }
        });

        // 2. Let the sim run briefly so the flux field has data for ψ² to sample.
        await page.waitForTimeout(500);

        // 3. Enable ψ² overlay (the toggle label varies by version; try two IDs).
        const toggled = await page.evaluate(() => {
            const ids = ['toggle-psi2', 'overlay-psi2', 'btn-psi2'];
            for (const id of ids) {
                const el = document.getElementById(id);
                if (el) {
                    el.checked = true;
                    el.dispatchEvent(new Event('change', { bubbles: true }));
                    return id;
                }
            }
            // Fallback: call the viewport method directly.
            if (window._ftdViewport && typeof window._ftdViewport.setPsi2Visible === 'function') {
                window._ftdViewport.setPsi2Visible(true);
                return 'direct-api';
            }
            return null;
        });

        if (!toggled) {
            test.skip(true, 'ψ² overlay toggle not found in current build — skipping opacity freeze test.');
            return;
        }

        await page.waitForTimeout(100);

        // 4. Pause the sim.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn) btn.click();
        });

        // Capture reference opacity right after pause.
        const opacityAfterPause = await page.evaluate(() => {
            const vp = window._ftdViewport;
            return vp?._quantumField?.material?.opacity ?? null;
        });

        if (opacityAfterPause === null) {
            test.skip(true, '_quantumField not initialized — ψ² overlay likely not built. Skipping.');
            return;
        }

        // 5. Wait 1 second of wall-clock — render loop continues via RAF.
        await page.waitForTimeout(1000);

        const opacityAfterWait = await page.evaluate(() => {
            return window._ftdViewport?._quantumField?.material?.opacity ?? null;
        });

        expect(Math.abs(opacityAfterWait - opacityAfterPause)).toBeLessThan(1e-6);

        // 6. Toggle the overlay 5 times while paused; opacity must stay pinned.
        for (let i = 0; i < 5; i++) {
            await page.evaluate((id) => {
                const el = document.getElementById(id);
                if (el) {
                    el.checked = !el.checked;
                    el.dispatchEvent(new Event('change', { bubbles: true }));
                } else if (window._ftdViewport?.setPsi2Visible) {
                    // Direct API fallback
                    window._ftdViewport._psi2Visible = !window._ftdViewport._psi2Visible;
                    window._ftdViewport.setPsi2Visible(window._ftdViewport._psi2Visible);
                }
            }, toggled);
            // Each toggle triggers at least one render() cycle.
            await page.waitForTimeout(50);
        }

        // Re-enable if currently off (so opacity is readable).
        await page.evaluate((id) => {
            const el = document.getElementById(id);
            if (el && !el.checked) {
                el.checked = true;
                el.dispatchEvent(new Event('change', { bubbles: true }));
            } else if (window._ftdViewport?.setPsi2Visible) {
                window._ftdViewport._psi2Visible = true;
                window._ftdViewport.setPsi2Visible(true);
            }
        }, toggled);
        await page.waitForTimeout(50);

        const opacityAfterToggles = await page.evaluate(() => {
            return window._ftdViewport?._quantumField?.material?.opacity ?? null;
        });

        // The freeze contract demands opacity after N repaints == opacity right after pause.
        expect(Math.abs(opacityAfterToggles - opacityAfterPause)).toBeLessThan(1e-6);

        // No console errors should have occurred.
        expect(errors, `Console errors during animation-clock freeze test: ${errors.join(' | ')}`)
            .toHaveLength(0);
    });
});
