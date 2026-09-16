// @ts-check
/**
 * Native transport overlay, end to end (spec 2026-09-15 native transport
 * overlays, section 7). Switches the overlay on in the live Scale-0 page and
 * checks that the reference engine's exact link energy current is drawn on
 * lattice links only: every vertex is a site centre or a half-step point on a
 * cell or box face, every segment has a site-centre end, and no segment spans
 * more than half a lattice step on any axis. The legend must name the reference
 * engine and, for the default pure-wave scenario (flux-pulse), say the balance
 * closes.
 *
 * Clicks go through DOM el.click() inside page.evaluate, as in
 * overlay-scheduler.spec.js (the panel header overlaps the toolbar headless).
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher } from './_helpers.js';

// The optional native GPU server socket is absent in a browser-only run.
const NATIVE_SERVER_SOCKET = /ws:\/\/(?:127\.0\.0\.1|localhost):9100/;

async function waitForScale0Ctx(page) {
    await expect.poll(
        () => page.evaluate(() => !!window.__ftdCtx),
        { timeout: 15_000, message: 'window.__ftdCtx (Scale-0 ctx) never became available' },
    ).toBe(true);
}

test('native transport overlay draws the exact link current on lattice links', async ({ page }) => {
    test.setTimeout(120_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page);
    await waitForScale0Ctx(page);

    await page.evaluate(() => { window.__ftdCtx.running = true; });
    await page.evaluate(() => {
        const button = document.getElementById('toggle-native-transport');
        if (!button) throw new Error('toggle-native-transport not found');
        button.click();
    });

    await expect.poll(
        () => page.evaluate(() => window.__ftdCtx?.viewport?._nativeTransportRenderer?._mesh?.geometry.drawRange.count ?? 0),
        { timeout: 45_000, message: 'the native transport renderer never drew a link' },
    ).toBeGreaterThan(0);

    const legend = await page.locator('#native-transport-legend').textContent();
    expect(legend).toContain('[REFERENCE ENGINE]');
    expect(legend).toContain('closes at every site');

    const geometry = await page.evaluate(() => {
        const geo = window.__ftdCtx.viewport._nativeTransportRenderer._mesh.geometry;
        const count = geo.drawRange.count;
        const p = geo.getAttribute('position').array;
        const EPS = 1e-4;
        const frac = (c) => c - Math.floor(c);
        const isHalf = (c) => Math.abs(frac(c) - 0.5) < EPS;
        const isInteger = (c) => frac(c) < EPS || frac(c) > 1 - EPS;
        const vertex = (v) => [p[3 * v], p[3 * v + 1], p[3 * v + 2]];
        const isCentre = (q) => q.every(isHalf);
        const isFacePoint = (q) => q.every((c) => isHalf(c) || isInteger(c)) && q.some(isInteger);
        let badVertex = null, longSegment = null, centrelessSegment = null;
        for (let v = 0; v < count; v++) {
            const q = vertex(v);
            if (!isCentre(q) && !isFacePoint(q) && !badVertex) badVertex = q;
        }
        for (let v = 0; v + 1 < count; v += 2) {
            const a = vertex(v), b = vertex(v + 1);
            if (a.some((c, i) => Math.abs(b[i] - c) > 0.5 + EPS) && !longSegment) longSegment = [a, b];
            if (!isCentre(a) && !isCentre(b) && !centrelessSegment) centrelessSegment = [a, b];
        }
        return { count, badVertex, longSegment, centrelessSegment };
    });
    expect(geometry.count % 4, 'each link is drawn as two half-segments (4 vertices)').toBe(0);
    expect(geometry.badVertex, 'a vertex is neither a site centre nor a face point').toBeNull();
    expect(geometry.longSegment, 'a segment spans more than half a lattice step').toBeNull();
    expect(geometry.centrelessSegment, 'a segment has no site-centre end').toBeNull();

    expect(errors.filter((e) => !NATIVE_SERVER_SOCKET.test(e))).toEqual([]);
});
