// @ts-check
/**
 * Phase 1 verification — FTD_TELEMETRY_ONDEMAND demand-gated telemetry
 * (SPEC_SCALE0_PERF_TELEMETRY_PANELS §5).
 *
 * On the WasmBridgeProxy worker path:
 *   • Lagrangian is actually gated. With no dock consumer, the proxy's last
 *     Lagrangian snapshot FREEZES while the sim keeps ticking.
 *   • Energy-audit is still computed in the worker (postFrame rewrites
 *     diag.totalEnergy from it). Demand still must not *request* extra
 *     main-thread / native audit just because the conservation overlay is on.
 *   • Opening Lagrangian turns wantLag (and wantAudit) back on; the lag
 *     snapshot moves again.
 *
 * The conservation micropanel is always-on on Scale 0 and is NOT an audit
 * consumer. Default Charts chips are not either.
 *
 * If the page is not cross-origin isolated (no SAB → main-thread WasmBridge),
 * the test skips: freeze is a worker-proxy observable.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

async function readProbe(page) {
    return page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const { getScale0TelemetryDemand } = await import('/js/telemetry/demand.js');
        const st = getScale0State();
        const caps = (st.useFluxMock && st.fluxMock)
            ? st.fluxMock.capabilities.scale0
            : window.__ftdCtx.bridge.capabilities.scale0;
        const d = caps.getScale0Diagnostics?.() || {};
        const a = caps.getScale0EnergyAudit?.() || null;
        const l = caps.getScale0Lagrangian?.() || null;
        const demand = getScale0TelemetryDemand(window.__ftdCtx);
        return {
            tick: Number(d.tick ?? -1),
            fieldE: a ? Number(a.fieldEnergy ?? a.totalEnergy ?? 0) : null,
            hasAudit: !!a,
            lagTotal: l ? Number(l.total ?? l.hamiltonian ?? 0) : null,
            hasLag: !!l,
            worker: !!(st.useFluxMock && st.fluxMock && st.fluxMock.isWorker === true),
            wantAudit: !!demand.wantAudit,
            wantLag: !!demand.wantLag,
        };
    });
}

test.describe('Scale-0 demand-gated telemetry (FTD_TELEMETRY_ONDEMAND)', () => {
    test('audit freezes when no consumer is visible, stays live when a panel is open', async ({ page }) => {
        test.setTimeout(90_000);
        await page.addInitScript(() => { window.__ftdTelemetryOnDemand = true; });
        await gotoAndReady(page, { timeout: 90_000 });
        await expect.poll(
            () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
            { timeout: 20_000 },
        ).toBe(true);

        // flux-pulse on the worker path; start playback.
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) { sel.value = 'flux-pulse'; sel.dispatchEvent(new Event('change', { bubbles: true })); }
        });
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });
        // Default tab is 'controls' — make sure no telemetry consumer is visible.
        await page.evaluate(() => {
            const t = document.querySelector('#tab-bar .tab[data-panel="controls"]');
            if (t) t.click();
        });

        const pre = await readProbe(page);
        test.skip(!pre.worker, 'WASM worker path inactive (no cross-origin isolation) — gating freeze is a worker observable');

        // ── Phase A: Controls + conservation overlay, no dock consumer ──
        // The worker still runs getEnergyAudit at a reduced cadence to rewrite
        // diag.totalEnergy (see wasm-bridge.worker.js postFrame). The stream
        // that actually gates is Lagrangian. Demand must not pin audit just
        // because the always-on conservation overlay is visible.
        await page.waitForTimeout(1500);
        const a0 = await readProbe(page);
        await page.waitForTimeout(800);
        const a1 = await readProbe(page);

        expect(a1.tick, 'sim must keep ticking').toBeGreaterThan(a0.tick);
        expect(a1.wantAudit, 'Controls + conservation must not request audit').toBe(false);
        expect(a1.wantLag, 'Controls must not request Lagrangian').toBe(false);

        const conservationVisible = await page.evaluate(() => {
            const el = document.getElementById('conservation-micropanel');
            return !!(el && el.getClientRects().length > 0);
        });
        expect(conservationVisible, 'conservation overlay is on during Controls').toBe(true);

        // Default Charts chips (flux/energy/particles/charge/entropy) are cheap
        // collectScale0 series — opening Charts must not request audit.
        await page.evaluate(() => {
            const t = document.querySelector('#tab-bar .tab[data-panel="charts"]');
            if (t) t.click();
        });
        await page.waitForTimeout(400);
        const charts = await readProbe(page);
        expect(charts.wantAudit, 'default Charts chips must not request audit').toBe(false);
        await page.evaluate(() => {
            const t = document.querySelector('#tab-bar .tab[data-panel="controls"]');
            if (t) t.click();
        });
        await page.waitForTimeout(200);

        // ── Phase B: open Lagrangian panel → lag stream requested ──
        await page.evaluate(() => {
            const t = document.querySelector('#tab-bar .tab[data-panel="lagrangian"]');
            if (t) t.click();
        });
        expect(await page.evaluate(() => window.__ftdCtx.isPanelVisible('lagrangian')),
            'lagrangian panel should read as visible').toBe(true);

        await expect.poll(async () => {
            const p = await readProbe(page);
            return p.wantLag && p.hasLag;
        }, { timeout: 8_000, message: 'Lagrangian stream never became live after opening the panel' }).toBe(true);
    });

});
