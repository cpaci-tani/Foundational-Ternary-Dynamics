// @ts-check
/**
 * Scale-0 WASM worker-teardown regression (Phase 1 lifecycle).
 *
 * `scale0-worker.spec.js` proves a WasmBridgeProxy spins up for flux-*
 * scenarios and that switching to a direct-WASM scenario flips `useFluxMock`
 * off. It does NOT prove the underlying Worker THREAD is actually terminated,
 * nor cover the lattice-resize and scale-switch teardown paths — the surfaces
 * most likely to leak workers since the WASM worker became the default deployed
 * path.
 *
 * This spec asserts conservation of live WasmBridgeProxy workers across all
 * three teardown paths (scenario churn, resize, scale switch) via the
 * `window.__ftdWasmWorkers()` debug counter (wasm-bridge-proxy.js):
 *   created === terminated + live   at every step, and   live ≤ 1   always.
 *
 * Requires COOP/COEP (SharedArrayBuffer) — SKIPs on a non-isolated server.
 * Run:  python serve.py 8081 --cache   (the harness server already sends COOP/COEP).
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode } from './_helpers.js';

const coiReady = (page) => page.evaluate(() =>
    globalThis.crossOriginIsolated === true && typeof SharedArrayBuffer !== 'undefined');

const workers = (page) => page.evaluate(() =>
    (typeof window.__ftdWasmWorkers === 'function') ? window.__ftdWasmWorkers() : null);

const fmReady = (page) => page.evaluate(async () => {
    const st = (await import('/js/scales/scale0/state/store.js')).getScale0State?.();
    const fm = st?.fluxMock;
    return { isWorker: !!fm?.isWorker, ready: !!fm?.ready, useFluxMock: !!st?.useFluxMock };
});

// Resize to a DIFFERENT existing dropdown option (whatever sizes the build offers)
// and return the chosen value — robust to the actual option set, so the change
// handler always fires a real rebuild.
const resizeToOther = (page) => page.evaluate(() => {
    const sel = document.getElementById('lattice-size');
    if (!sel) return null;
    const cur = sel.value;
    const other = [...sel.options].map((o) => o.value).find((v) => v !== cur);
    if (!other) return null;
    sel.value = other;
    sel.dispatchEvent(new Event('change', { bubbles: true }));
    return other;
});

test.beforeEach(async ({ page }) => { page.setDefaultTimeout(30_000); });

test.describe('Scale-0 WASM worker teardown — no orphaned workers', () => {
    test('WASM workers are conserved across scenario churn, resize, and scale switch', async ({ page }) => {
        test.setTimeout(120_000);
        await gotoAndReady(page);
        test.skip(!(await coiReady(page)), 'requires cross-origin isolation (serve.py --cache COOP/COEP)');

        // Debug counter must be installed (wasm-bridge-proxy.js installs it at module load).
        await expect.poll(async () => (await workers(page)) !== null,
            { timeout: 10_000, message: '__ftdWasmWorkers debug counter not installed' }).toBe(true);

        // Wait for the initial flux-pulse WASM worker to come up.
        await expect.poll(async () => (await fmReady(page)).ready,
            { timeout: 20_000, message: 'initial WASM worker never became ready' }).toBe(true);

        const invariant = async (label) => {
            const w = await workers(page);
            expect(w, `${label}: counter present`).not.toBeNull();
            // Conservation: every proxy ever created is either still live or terminated.
            expect(w.created, `${label}: created === terminated + live`).toBe(w.terminated + w.live);
            // At most one Scale-0 worker proxy alive at a time (no accumulation).
            expect(w.live, `${label}: ≤1 live worker (no accumulation)`).toBeLessThanOrEqual(1);
            return w;
        };

        const w0 = await invariant('baseline');
        expect(w0.live, 'one live WASM worker after flux-pulse boot').toBe(1);

        // ── (1) Lattice resize — rebuilds the WasmBridgeProxy; old worker must terminate ──
        const targetSize = await resizeToOther(page);
        expect(targetSize, 'a second lattice-size option exists to resize to').not.toBeNull();
        await expect.poll(async () => (await fmReady(page)).ready,
            { timeout: 20_000, message: 'WASM worker did not re-ready after resize' }).toBe(true);
        const wResize = await invariant('after resize');
        expect(wResize.terminated, 'resize terminated the prior WASM worker').toBeGreaterThanOrEqual(w0.terminated + 1);
        expect(wResize.live, 'still exactly one live WASM worker after resize').toBe(1);

        // ── (2) Scenario churn — switch to another worker scenario if available ──
        const altScenario = await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (!sel) return null;
            const cur = sel.value;
            // Prefer another flux-* / quantum-* / s0-* scenario (worker-eligible).
            const opt = [...sel.options].map((o) => o.value).find((v) =>
                v !== cur && /^(flux-|quantum-|s0-seed-|s0-field-)/.test(v));
            return opt ?? null;
        });
        if (altScenario) {
            const before = await workers(page);
            await page.evaluate((v) => {
                const sel = document.getElementById('scenario-select');
                sel.value = v; sel.dispatchEvent(new Event('change', { bubbles: true }));
            }, altScenario);
            await expect.poll(async () => (await fmReady(page)).ready, { timeout: 20_000 }).toBe(true);
            const wChurn = await invariant(`after churn → ${altScenario}`);
            expect(wChurn.terminated, 'scenario churn terminated the prior WASM worker')
                .toBeGreaterThanOrEqual(before.terminated + 1);
            expect(wChurn.live, 'still exactly one live WASM worker after churn').toBe(1);
        }

        // ── (3) Scale switch away from lattice — proxy cleared, worker terminated ──
        await switchMode(page, 'particles');
        await page.waitForTimeout(500);
        const wAway = await invariant('after leaving Scale 0');
        expect(wAway.live, 'no live WASM worker once Scale 0 is exited').toBe(0);

        // ── Return to lattice — a fresh WASM worker comes up, still conserved ──
        await switchMode(page, 'lattice');
        await expect.poll(async () => (await fmReady(page)).ready,
            { timeout: 20_000, message: 'WASM worker did not come back on Scale-0 re-entry' }).toBe(true);
        const wBack = await invariant('after Scale-0 re-entry');
        expect(wBack.live, 'one live WASM worker after re-entry').toBe(1);
    });
});
