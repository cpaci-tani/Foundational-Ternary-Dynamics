// @ts-check
/**
 * Responsive overflow regression guard.
 *
 * This is the standing test behind the 2026-06 responsive UI audit. It pins the
 * two properties that the audit established and that are easy to silently break
 * with a single careless `width:100%`-on-a-wide-child or a fixed px size:
 *
 *   1. NO HORIZONTAL OVERFLOW — across a phone→wide width matrix, across the
 *      representative engine scales, and in the two overlay states that broke
 *      historically (the compact "Menu" expansion and the bottom-sheet panel).
 *      Two complementary signals are checked because `body { overflow:hidden }`
 *      masks page-level scrollWidth: (a) documentElement.scrollWidth, and
 *      (b) curated structural containers must each stay within the viewport
 *      (these are the boxes that must FIT, not horizontally scroll).
 *
 *   2. PLAY-BAR ↔ BOTTOM-SHEET — on a phone the floating transport capsule must
 *      not overlap the open bottom-sheet panel or the status bar (the capsule is
 *      lifted above the sheet (--m-sheet-h) when open, and rests above the status
 *      bar when collapsed — play-bar.css).
 *
 *   3. FULL-BLEED CANVAS — on a phone the 3D canvas fills the whole viewport cell
 *      (full width, full height below the toolbar) and runs edge-to-edge behind the
 *      floating bottom sheet; `touch-action:none` lets OrbitControls own orbit/pan/
 *      zoom. Collapsing the sheet must NOT change the canvas size (the sheet floats,
 *      it is not a grid row — so there is no dead zone and no WebGL resize thrash).
 *      This is the property the 2026-06-14 "make the mobile canvas full VH/VW" pass
 *      established (app-shell.css grid + viewport-overlays.css zero padding).
 *
 * A touch-tablet block (hasTouch + isMobile ⇒ pointer:coarse at 768px) guards the
 * coarse-pointer tap-target floors in responsive.css, which are pointer-based and
 * therefore not exercised by the width-only resize matrix above.
 *
 * Width-based CSS (max-width media queries) is what the matrix below exercises;
 * that is exactly why the audit fixes were written width-based where possible.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode } from './_helpers.js';

// width × height matrix: small phone → wide desktop, plus both tablet orientations.
const VIEWPORTS = [
    { label: 'small-phone',       w: 360,  h: 780 },
    { label: 'iphone',            w: 390,  h: 844 },
    { label: 'large-phone',       w: 414,  h: 896 },
    { label: 'tablet-portrait',   w: 768,  h: 1024 },
    { label: 'tablet-landscape',  w: 1024, h: 768 },
    { label: 'desktop',           w: 1280, h: 800 },
    { label: 'wide',              w: 1440, h: 900 },
];

// Representative scales — different controllers mount different overlays/toolbars.
const SCALES = ['lattice', 'particles', 'atoms', 'cosmic'];

/**
 * In-page overflow audit. Returns the two complementary overflow signals plus a
 * curated-container check. Runs entirely in the page so it sees live layout.
 *
 * @param {import('@playwright/test').Page} page
 */
async function auditOverflow(page) {
    return page.evaluate(() => {
        const d = document.documentElement;
        const vw = window.innerWidth;
        const vh = window.innerHeight;
        const TOL = 2;

        const sel = (el) => {
            const cls = (el.className && el.className.toString)
                ? '.' + el.className.toString().trim().split(/\s+/).slice(0, 2).join('.')
                : '';
            return (el.tagName.toLowerCase() + (el.id ? '#' + el.id : '') + cls).slice(0, 48);
        };
        const visible = (el) => {
            const r = el.getBoundingClientRect();
            const s = getComputedStyle(el);
            return r.width > 0 && r.height > 0 && s.visibility !== 'hidden'
                && s.display !== 'none' && parseFloat(s.opacity) > 0.05;
        };
        const onscreen = (el) => {
            const r = el.getBoundingClientRect();
            return r.left < vw - 1 && r.right > 1 && r.bottom > 1 && r.top < vh - 1;
        };
        // An element is allowed to extend past the edge if an ancestor scrolls/clips
        // it horizontally (intentional, e.g. the scrollable compact toolbar).
        const inScrollContainer = (el) => {
            let n = el.parentElement;
            while (n && n !== document.body) {
                const ox = getComputedStyle(n).overflowX;
                if (ox === 'auto' || ox === 'scroll' || ox === 'hidden') return true;
                n = n.parentElement;
            }
            return false;
        };

        // (a) page-level horizontal scroll
        const pageOverflow = d.scrollWidth - d.clientWidth;

        // (b) curated structural containers that must FIT inside the viewport
        const curatedSelectors = [
            '#app', '#toolbar', '#viewport', '#status-bar', '#panel-area',
            '#tab-bar', '.play-bar', '.topbar-slot-context',
        ];
        const curated = [];
        for (const cs of curatedSelectors) {
            for (const el of document.querySelectorAll(cs)) {
                if (!visible(el)) continue;
                const r = el.getBoundingClientRect();
                if (r.right > vw + TOL || r.left < -TOL || r.width > vw + TOL) {
                    curated.push({ sel: sel(el), left: Math.round(r.left), right: Math.round(r.right), w: Math.round(r.width) });
                }
            }
        }

        // (c) gross overflow sweep (diagnostic) — any visible on-screen element that
        //     pokes well past the right edge and is NOT inside a scroll container.
        const gross = [];
        const seen = new Set();
        for (const el of document.querySelectorAll('body *')) {
            if (!visible(el) || !onscreen(el)) continue;
            const r = el.getBoundingClientRect();
            if (r.right > vw + 24 && r.width > 40 && !inScrollContainer(el)) {
                const k = sel(el);
                if (seen.has(k)) continue;
                seen.add(k);
                gross.push({ sel: k, right: Math.round(r.right), w: Math.round(r.width) });
            }
        }

        return {
            vw, vh, layoutMode: d.dataset.layoutMode, panelMount: d.dataset.panelMount,
            scrollW: d.scrollWidth, clientW: d.clientWidth, pageOverflow,
            curated, gross: gross.slice(0, 12),
        };
    });
}

function assertNoOverflow(report, context) {
    expect(report.pageOverflow, `page horizontal scroll @ ${context} (scrollW ${report.scrollW} vs clientW ${report.clientW})`).toBeLessThanOrEqual(2);
    expect(report.curated, `structural container overflow @ ${context}: ${JSON.stringify(report.curated)}`).toEqual([]);
    expect(report.gross, `gross element overflow @ ${context}: ${JSON.stringify(report.gross)}`).toEqual([]);
}

test.describe('responsive overflow', () => {
    test.beforeEach(async ({ page }) => {
        await page.addInitScript(() => window.localStorage.clear());
    });

    test('no horizontal overflow across the width matrix (lattice)', async ({ page }) => {
        await page.setViewportSize({ width: VIEWPORTS[VIEWPORTS.length - 1].w, height: VIEWPORTS[VIEWPORTS.length - 1].h });
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForTimeout(500);

        for (const vp of VIEWPORTS) {
            await page.setViewportSize({ width: vp.w, height: vp.h });
            // let the breakpoint service + mount toggle re-evaluate and reflow
            await page.evaluate(() => window.dispatchEvent(new Event('resize')));
            await page.waitForTimeout(350);
            const report = await auditOverflow(page);
            assertNoOverflow(report, `${vp.label} ${vp.w}×${vp.h}`);
        }
    });

    test('no horizontal overflow across scales at a phone width (390)', async ({ page }) => {
        await page.setViewportSize({ width: 390, height: 844 });
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForTimeout(500);

        for (const mode of SCALES) {
            await switchMode(page, mode);
            await page.waitForTimeout(600);
            const report = await auditOverflow(page);
            assertNoOverflow(report, `phone 390 · scale=${mode}`);
        }
    });

    test('compact Menu expansion stays within the viewport (390)', async ({ page }) => {
        await page.setViewportSize({ width: 390, height: 844 });
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForTimeout(500);

        // Expand the secondary toolbar controls (historically overflowed to ~895px).
        const opened = await page.evaluate(() => {
            const tb = document.getElementById('toolbar');
            if (!tb) return false;
            tb.dataset.compactMenu = 'open';
            return true;
        });
        expect(opened, 'toolbar present').toBe(true);
        await page.waitForTimeout(250);

        const report = await auditOverflow(page);
        assertNoOverflow(report, 'phone 390 · Menu open');

        // Direct guard on the context panel that broke before the topbar.css fix.
        const ctx = await page.evaluate(() => {
            const el = document.querySelector('.topbar-slot-context');
            if (!el) return null;
            const r = el.getBoundingClientRect();
            return { right: Math.round(r.right), width: Math.round(r.width), vw: window.innerWidth };
        });
        if (ctx) {
            expect(ctx.width, `context panel width ${ctx.width} must fit ${ctx.vw}`).toBeLessThanOrEqual(ctx.vw + 2);
            expect(ctx.right, `context panel right ${ctx.right} must not exceed ${ctx.vw}`).toBeLessThanOrEqual(ctx.vw + 2);
        }
    });

    test('play-bar does not overlap the open bottom sheet or status bar (390)', async ({ page }) => {
        await page.setViewportSize({ width: 390, height: 844 });
        await gotoAndReady(page, { timeout: 45_000 });
        // Wait for the transport capsule to mount (it appears after bridge init).
        await page.waitForSelector('#play-bar', { timeout: 15_000 });
        await page.waitForTimeout(500);

        const geom = await page.evaluate(() => {
            const app = document.getElementById('app');
            const r = (id) => {
                const el = document.getElementById(id);
                if (!el) return null;
                const b = el.getBoundingClientRect();
                return { top: b.top, bottom: b.bottom, h: b.height };
            };
            const overlap = (a, b) => (a && b) ? Math.max(0, Math.min(a.bottom, b.bottom) - Math.max(a.top, b.top)) : null;
            const pb = r('play-bar'), panel = r('panel-area'), status = r('status-bar');
            return {
                collapsed: app.classList.contains('panels-collapsed'),
                pb, panel, status,
                overlapPanel: overlap(pb, panel),
                overlapStatus: overlap(pb, status),
            };
        });

        expect(geom.pb, 'play-bar mounted').not.toBeNull();
        // Default mobile state is the open bottom sheet.
        expect(geom.collapsed, 'bottom sheet open by default on phone').toBe(false);
        expect(geom.overlapPanel, `play-bar overlaps panel by ${geom.overlapPanel}px`).toBeLessThanOrEqual(1);
        expect(geom.overlapStatus, `play-bar overlaps status bar by ${geom.overlapStatus}px`).toBeLessThanOrEqual(1);
    });

    test('mobile canvas is full-bleed (full width + height below the toolbar) with touch enabled (390)', async ({ page }) => {
        await page.setViewportSize({ width: 390, height: 844 });
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForSelector('#viewport canvas', { timeout: 15_000 });
        // Renderer sizing is reactive to resize; nudge + settle before measuring.
        await page.evaluate(() => window.dispatchEvent(new Event('resize')));
        await page.waitForTimeout(400);

        const geom = await page.evaluate(() => {
            const r = (sel) => {
                const el = document.querySelector(sel);
                if (!el) return null;
                const b = el.getBoundingClientRect();
                return { left: b.left, right: b.right, top: b.top, bottom: b.bottom, w: b.width, h: b.height };
            };
            const canvas = document.querySelector('#viewport canvas');
            return {
                vw: window.innerWidth,
                vh: window.innerHeight,
                toolbar: r('#toolbar'),
                viewport: r('#viewport'),
                canvas: r('#viewport canvas'),
                viewportPadding: getComputedStyle(document.querySelector('#viewport')).padding,
                canvasTouchAction: canvas ? getComputedStyle(canvas).touchAction : null,
                viewportTouchAction: getComputedStyle(document.querySelector('#viewport')).touchAction,
            };
        });

        expect(geom.viewport, '#viewport present').not.toBeNull();
        expect(geom.canvas, '3D canvas present').not.toBeNull();

        // (a) #viewport fills the full width and runs from the toolbar bottom to the
        //     bottom of the screen — the grid gives it the whole 1fr cell.
        expect(geom.viewport.left, 'viewport flush left').toBeLessThanOrEqual(1);
        expect(Math.abs(geom.viewport.right - geom.vw), 'viewport flush right').toBeLessThanOrEqual(2);
        expect(Math.abs(geom.viewport.top - (geom.toolbar ? geom.toolbar.bottom : 0)), 'viewport starts at toolbar bottom').toBeLessThanOrEqual(2);
        expect(Math.abs(geom.viewport.bottom - geom.vh), 'viewport reaches screen bottom').toBeLessThanOrEqual(2);

        // (b) zero padding so the WebGL canvas is not inset/shrunk.
        expect(geom.viewportPadding, 'viewport has no padding on mobile').toBe('0px');

        // (c) the canvas itself fills the viewport cell (full VW + full height below toolbar).
        expect(Math.abs(geom.canvas.w - geom.viewport.w), `canvas width ${geom.canvas.w} ≈ viewport ${geom.viewport.w}`).toBeLessThanOrEqual(2);
        expect(Math.abs(geom.canvas.h - geom.viewport.h), `canvas height ${geom.canvas.h} ≈ viewport ${geom.viewport.h}`).toBeLessThanOrEqual(2);

        // (d) touch-action:none on the canvas (and its container) hands all gestures to
        //     OrbitControls instead of the browser (no page scroll / native pinch).
        expect(geom.canvasTouchAction, 'canvas touch-action:none').toBe('none');
        expect(geom.viewportTouchAction, 'viewport touch-action:none').toBe('none');
    });

    test('collapsing the bottom sheet keeps the canvas full (no dead zone, no resize) (390)', async ({ page }) => {
        await page.setViewportSize({ width: 390, height: 844 });
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForSelector('#viewport canvas', { timeout: 15_000 });
        await page.evaluate(() => window.dispatchEvent(new Event('resize')));
        await page.waitForTimeout(400);

        const before = await page.evaluate(() => {
            const b = document.querySelector('#viewport canvas').getBoundingClientRect();
            return { w: Math.round(b.width), h: Math.round(b.height), top: Math.round(b.top) };
        });

        // Collapse via the real mobile control; fall back to the generic dock toggle.
        const collapsed = await page.evaluate(() => {
            const btn = document.querySelector('.panel-dock-hide-btn')
                || document.querySelector('#btn-panel-toggle');
            if (!btn) return { clicked: false };
            btn.click();
            return { clicked: true };
        });
        expect(collapsed.clicked, 'a panel-collapse control exists on mobile').toBe(true);
        await page.waitForTimeout(500); // sheet slide transition (0.35s) + settle

        const after = await page.evaluate(() => {
            const app = document.getElementById('app');
            const cb = document.querySelector('#viewport canvas').getBoundingClientRect();
            const pb = document.querySelector('#panel-area').getBoundingClientRect();
            return {
                isCollapsed: app.classList.contains('panels-collapsed'),
                canvas: { w: Math.round(cb.width), h: Math.round(cb.height), top: Math.round(cb.top) },
                panelTop: Math.round(pb.top),
                vh: window.innerHeight,
            };
        });

        expect(after.isCollapsed, 'sheet collapsed after toggle').toBe(true);
        // The sheet slides off the bottom edge — it must not leave a reserved gap.
        expect(after.panelTop, `collapsed sheet slid off-screen (top ${after.panelTop} ≥ ${after.vh})`).toBeGreaterThanOrEqual(after.vh - 2);
        // The canvas is governed by the grid, not the sheet, so its box is unchanged.
        expect(Math.abs(after.canvas.w - before.w), `canvas width unchanged (${before.w}→${after.canvas.w})`).toBeLessThanOrEqual(1);
        expect(Math.abs(after.canvas.h - before.h), `canvas height unchanged (${before.h}→${after.canvas.h})`).toBeLessThanOrEqual(1);
        expect(Math.abs(after.canvas.top - before.top), `canvas top unchanged (${before.top}→${after.canvas.top})`).toBeLessThanOrEqual(1);
    });
});

// ── Touch tablet (coarse pointer) ─────────────────────────────────────────────
// isMobile+hasTouch makes Chromium report `pointer: coarse`, which the width
// matrix above cannot emulate. Guards the ≥768px coarse-pointer tap-target floors.
test.describe('touch tablet (coarse pointer)', () => {
    test.use({ viewport: { width: 768, height: 1024 }, hasTouch: true, isMobile: true });

    test.beforeEach(async ({ page }) => {
        await page.addInitScript(() => window.localStorage.clear());
    });

    test('no horizontal overflow at tablet width with a coarse pointer', async ({ page }) => {
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForTimeout(600);
        const report = await auditOverflow(page);
        assertNoOverflow(report, 'tablet 768 · coarse pointer');
    });

    test('coarse-pointer tap targets are boosted to finger size', async ({ page }) => {
        await gotoAndReady(page, { timeout: 45_000 });
        await page.waitForTimeout(600);

        const coarse = await page.evaluate(() => matchMedia('(pointer: coarse)').matches);
        // If the runner cannot emulate a coarse pointer, the rule under test can't
        // fire; skip rather than assert on a desktop-pointer layout.
        test.skip(!coarse, 'environment does not report pointer: coarse');

        const sizes = await page.evaluate(() => {
            const out = {};
            const cb = document.querySelector('#app .toggle-row input[type="checkbox"]');
            if (cb) out.checkbox = Math.round(parseFloat(getComputedStyle(cb).width));
            const tbSelect = document.querySelector('#app .tb-select');
            if (tbSelect) out.tbSelectMinH = Math.round(parseFloat(getComputedStyle(tbSelect).minHeight));
            return out;
        });

        if (sizes.checkbox !== undefined) {
            expect(sizes.checkbox, 'coarse-pointer checkbox boosted to 22px').toBeGreaterThanOrEqual(20);
        }
        if (sizes.tbSelectMinH !== undefined) {
            expect(sizes.tbSelectMinH, 'coarse-pointer .tb-select min-height ≥ 44px').toBeGreaterThanOrEqual(44);
        }
    });
});
