// @ts-check
/**
 * Phase 2 verification — FTD_PANEL_RENDER_V2 panel render optimizations
 * (SPEC_SCALE0_PERF_TELEMETRY_PANELS §6).
 *
 *  • isPanelLive: the unified visibility predicate — active tab OR a
 *    non-collapsed floated window is "live"; hidden or floated-collapsed is not.
 *    (Fixes floated charts/Lagrangian freezing; adds the collapsed-grid gate.)
 *  • telemetry grid: renders correctly after the per-channel buffer/element
 *    caching restructure, and stays live while visible.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 panel render V2 (FTD_PANEL_RENDER_V2)', () => {
    test('isPanelLive: active OR non-collapsed floated only', async ({ page }) => {
        await gotoAndReady(page);
        const r = await page.evaluate(async () => {
            const { isPanelLive } = await import('/js/ui/panels/panel-visibility.js');
            const mk = (cls, wrapCls) => {
                const el = document.createElement('div');
                el.className = cls;
                if (wrapCls) {
                    const fw = document.createElement('div');
                    fw.className = wrapCls;
                    fw.appendChild(el);
                }
                return el;
            };
            return {
                active:    isPanelLive(mk('panel active')),
                hidden:    isPanelLive(mk('panel')),
                floated:   isPanelLive(mk('panel', 'floating-window')),
                collapsed: isPanelLive(mk('panel', 'floating-window is-collapsed')),
                nullEl:    isPanelLive(null),
            };
        });
        expect(r.active, 'active tab → live').toBe(true);
        expect(r.hidden, 'hidden docked → not live').toBe(false);
        expect(r.floated, 'floated (expanded) → live — fixes the freeze bug').toBe(true);
        expect(r.collapsed, 'floated + collapsed → not live — skip invisible work').toBe(false);
        expect(r.nullEl, 'null → not live').toBe(false);
    });

    test('telemetry grid renders + stays live when visible', async ({ page }) => {
        test.setTimeout(60_000);
        await page.addInitScript(() => { window.__ftdPanelRenderV2 = true; });
        await gotoAndReady(page);
        await expect.poll(
            () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.telemetryGridPanel)),
            { timeout: 20_000 },
        ).toBe(true);

        // Start playback so the hub's primary buffers (flux/energy/…) fill.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });
        await page.waitForTimeout(1200);

        // Drive the grid directly as a "live" (active) panel: confirm the
        // per-channel {u,valueEl,xs,ys} restructure RENDERS and reflects the
        // LIVE hub value (a bug here would blank the grid, throw, or go stale).
        // (Asserting the value *equals* the hub avoids depending on a physics
        // quantity changing — e.g. totalFlux is conserved and stays constant.)
        const r = await page.evaluate(async () => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            const grid = window.__ftdCtx.telemetryGridPanel;
            grid.el.classList.add('active');
            grid.update();
            const cell = grid.el.querySelector('.telemetry-card-value');
            return {
                text: cell ? cell.textContent : null,
                expected: grid.formatValue(telemetryHub.flux.last(), 'J'),
                fluxCount: telemetryHub.flux.count,
            };
        });
        expect(r.fluxCount, 'hub flux buffer has data (sim ran + collected)').toBeGreaterThan(0);
        expect(r.text, 'grid value cell rendered (Map-restructure intact)').not.toBeNull();
        expect(r.text, 'grid value populated (not the init placeholder)').not.toBe('--');
        expect(r.text, 'grid reflects the live hub value').toBe(r.expected);
    });
});
