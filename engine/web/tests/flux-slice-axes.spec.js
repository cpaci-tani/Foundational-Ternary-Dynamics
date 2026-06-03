// @ts-check
/**
 * Flux-slice ALL-AXIS overlay + shared Flux-Volume controls regression spec
 * (2026-06-02).
 *
 * Two features pinned here:
 *
 *  (A) ALL THREE PLANES. The 3D flux slice overlay (#toggle-flux-slice) renders
 *      the three lattice mid-planes — xy (z=L/2), xz (y=L/2), yz (x=L/2) — into a
 *      dedicated _fluxSliceMesh, with per-axis toggle buttons
 *      (#flux-slice-axis-{xy,xz,yz}) that default all-on. Previously only the
 *      single hardcoded xz plane (axis 1) rendered (runtime/frame-sync.js).
 *
 *  (B) SHARED CONTROLS. The Flux Volume card's Opacity / Shape / Point Size /
 *      Threshold controls drive the slice mesh in parallel with the volume point
 *      cloud (ui/controls/wire.js::wireFluxVolume fans out to setFluxSlice*).
 *
 * ── Introspection surface PINNED (authoring time) ──
 *   - viewport.getEnabledFluxSliceAxes()          ← viewport.js → FieldRenderer
 *   - viewport.updateFluxSlices(planes, N, index) ← packs enabled planes
 *   - viewport.setFluxSlice{Opacity,Shape,PointScale,Threshold,AxisEnabled}
 *   - fieldRenderer._fluxSliceMesh                ← viewport/field-renderer.js
 *        .geometry.drawRange.count   packed points (= planes·N² at threshold 0)
 *        .material.uniforms.{uOpacity, shapeType}
 *        ._fluxSliceThreshold / ._fluxSlicePointScale  (applied per pack)
 *   - axis-button wiring   ← scales/scale0/ui/bindings.js
 *   - slider fan-out       ← scales/scale0/ui/controls/wire.js
 *   - per-frame gather     ← scales/scale0/runtime/frame-sync.js (enabled axes)
 *
 * DETERMINISM: rather than race the live animate / dissipation loop, the tests
 * drive the EXACT path frame-sync uses — gather getFluxSlice() per enabled axis,
 * call viewport.updateFluxSlices(...) — and read the mesh's own counters in the
 * same synchronous page.evaluate (no rAF can fire mid-block). Threshold 0 makes
 * the packed count exactly planes·N² on the default cube boundary (no clipping).
 * Clicks go via el.click() in page.evaluate (the headless panel layout overlaps
 * the toolbar, so page.click() is unreliable — mirrors the sibling specs).
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

test.beforeEach(async ({ page }) => {
    page.setDefaultTimeout(20_000);
});

async function waitForCtx(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.viewport)),
        { timeout: 15_000, message: 'window.__ftdCtx.viewport never became available' },
    ).toBe(true);
}

/**
 * Pack the slice for every currently-enabled axis exactly the way
 * runtime/frame-sync.js does, synchronously, and return the mesh draw count
 * plus the enabled-axis set. Injects a central flux blob first so the shared
 * max is > 0 (updateFluxSlices early-returns on an all-zero field).
 */
async function packSlice(page) {
    return page.evaluate(() => {
        const ctx = window.__ftdCtx, v = ctx.viewport, b = ctx.bridge;
        const N = b.latticeSize | 0, mid = N >> 1;
        if (typeof b.injectFlux === 'function') b.injectFlux(mid, mid, mid, 0.5, 0, 0);
        const axes = v.getEnabledFluxSliceAxes();
        const planes = [];
        for (const a of axes) {
            const data = b.getFluxSlice(a, mid);
            if (data && data.length) planes.push({ axis: a, data });
        }
        v.updateFluxSlices(planes, N, mid);
        const m = v._fieldRenderer._fluxSliceMesh;
        return { N, axes: axes.slice().sort(), drawCount: m?.geometry?.drawRange?.count ?? -1 };
    });
}

test.describe('Scale-0 flux slice — all axes + shared volume controls', () => {

    // ── (A) UI surface: the three per-axis buttons + the viewport API. ──
    test('axis buttons exist, default all-on, and expose the viewport API', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtx(page);

        const ui = await page.evaluate(() => {
            const ids = ['flux-slice-axis-xy', 'flux-slice-axis-xz', 'flux-slice-axis-yz'];
            const v = window.__ftdCtx.viewport;
            return {
                present: ids.map(id => !!document.getElementById(id)),
                active: ids.map(id => !!document.getElementById(id)?.classList.contains('active')),
                sliceToggle: !!document.getElementById('toggle-flux-slice'),
                enabled: v.getEnabledFluxSliceAxes(),
                setters: ['setFluxSliceOpacity', 'setFluxSliceShape', 'setFluxSlicePointScale',
                    'setFluxSliceThreshold', 'setFluxSliceAxisEnabled', 'updateFluxSlices']
                    .map(fn => typeof v[fn]),
            };
        });

        expect(ui.present, 'all three xy/xz/yz buttons must exist').toEqual([true, true, true]);
        expect(ui.active, 'all three axis buttons default active (all-on)').toEqual([true, true, true]);
        expect(ui.sliceToggle, '#toggle-flux-slice must exist').toBe(true);
        expect(ui.enabled.slice().sort(), 'default enabled axes = [0,1,2]').toEqual([0, 1, 2]);
        expect(ui.setters, 'every flux-slice viewport method must be a function')
            .toEqual(['function', 'function', 'function', 'function', 'function', 'function']);

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });

    // ── (A) Rendering: all three planes pack; per-axis toggle drops one. ──
    test('renders all three mid-planes; per-axis toggle drops a plane', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtx(page);

        // Threshold 0 ⇒ every cell of every enabled plane packs ⇒ count = planes·N².
        await page.evaluate(() => window.__ftdCtx.viewport.setFluxSliceThreshold(0));

        const all = await packSlice(page);
        expect(all.axes, 'all three axes enabled by default').toEqual([0, 1, 2]);
        expect(all.drawCount, 'three planes ⇒ 3·N² packed points').toBe(3 * all.N * all.N);

        // Turn xy (axis 2) off via its real button handler.
        await page.evaluate(() => document.getElementById('flux-slice-axis-xy').click());
        const two = await packSlice(page);
        expect(two.axes, 'xy removed ⇒ [0,1] (yz, xz)').toEqual([0, 1]);
        expect(two.drawCount, 'two planes ⇒ 2·N² packed points').toBe(2 * two.N * two.N);

        // Restore xy.
        await page.evaluate(() => document.getElementById('flux-slice-axis-xy').click());
        const restored = await packSlice(page);
        expect(restored.axes, 'xy restored ⇒ [0,1,2]').toEqual([0, 1, 2]);
        expect(restored.drawCount, 'three planes again ⇒ 3·N²').toBe(3 * restored.N * restored.N);

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });

    // ── (B) Shared controls: the Flux Volume sliders drive the slice mesh. ──
    test('Flux Volume Opacity / Shape / Threshold / Point-Size drive the slice', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtx(page);

        // Make sure the slice mesh is instantiated (toggle it on if needed).
        await page.evaluate(() => {
            const b = document.getElementById('toggle-flux-slice');
            if (b && !b.classList.contains('active')) b.click();
        });

        const res = await page.evaluate(() => {
            const setSlider = (id, val) => {
                const s = document.getElementById(id);
                s.value = String(val);
                s.dispatchEvent(new Event('input', { bubbles: true }));
            };
            const setSelect = (id, val) => {
                const s = document.getElementById(id);
                s.value = String(val);
                s.dispatchEvent(new Event('change', { bubbles: true }));
            };
            setSlider('flux-opacity', 0.42);
            setSelect('flux-shape-select', 4);   // Triangle
            setSlider('flux-threshold', 0.077);
            setSlider('flux-point-scale', 2.3);
            const fr = window.__ftdCtx.viewport._fieldRenderer;
            const m = fr._fluxSliceMesh;
            return {
                opacity: m?.material?.uniforms?.uOpacity?.value ?? null,
                shape: m?.material?.uniforms?.shapeType?.value ?? null,
                threshold: fr._fluxSliceThreshold,
                pointScale: fr._fluxSlicePointScale,
            };
        });

        expect(res.opacity, 'flux-opacity slider must drive the slice uOpacity uniform').toBeCloseTo(0.42, 5);
        expect(res.shape, 'flux-shape-select must drive the slice shapeType uniform').toBe(4);
        expect(res.threshold, 'flux-threshold slider must drive the slice threshold').toBeCloseTo(0.077, 5);
        expect(res.pointScale, 'flux-point-scale slider must drive the slice point scale').toBeCloseTo(2.3, 5);

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });
});
