// @ts-check
/**
 * Phase 1 verification — FTD_TELEMETRY_ONDEMAND demand-gated telemetry
 * (SPEC_SCALE0_PERF_TELEMETRY_PANELS §5). On the worker path, the audit /
 * Lagrangian O(N³) passes are computed only when a visible consumer needs them:
 *
 *   • no consumer visible  → the worker stops computing the audit, so the
 *     proxy's served value FREEZES — *while the sim keeps ticking* (the choppy-
 *     playback fix: the worker frame budget is no longer spent on telemetry
 *     nobody is looking at).
 *   • a panel open          → the audit is LIVE again (catch-up on open) and
 *     non-zero (functionality preserved — every panel still gets its numbers).
 *
 * This is a worker-path feature (the freeze is observable because the proxy
 * serves the worker's last-posted scalar). If the page is not cross-origin
 * isolated (no SAB → in-thread MockBridge), the test skips: the main-thread
 * gate still applies, but `getScale0EnergyAudit()` recomputes on each direct
 * read there, so "freeze" is not the right observable.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

async function readProbe(page) {
    return page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const st = getScale0State();
        const caps = (st.useFluxMock && st.fluxMock)
            ? st.fluxMock.capabilities.scale0
            : window.__ftdCtx.bridge.capabilities.scale0;
        const d = caps.getScale0Diagnostics?.() || {};
        const a = caps.getScale0EnergyAudit?.() || null;
        return {
            tick: Number(d.tick ?? -1),
            fieldE: a ? Number(a.fieldEnergy ?? a.totalEnergy ?? 0) : null,
            hasAudit: !!a,
            worker: !!(st.useFluxMock && st.fluxMock && st.fluxMock.isWorker === true),
        };
    });
}

test.describe('Scale-0 demand-gated telemetry (FTD_TELEMETRY_ONDEMAND)', () => {
    test('audit freezes when no consumer is visible, stays live when a panel is open', async ({ page }) => {
        test.setTimeout(60_000);
        await page.addInitScript(() => { window.__ftdTelemetryOnDemand = true; });
        await gotoAndReady(page);
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
        test.skip(!pre.worker, 'worker path inactive (no cross-origin isolation) — gating freeze is a worker observable');

        // ── Phase A: no consumer → audit FREEZES while the sim keeps ticking ──
        // 1500ms (not 900): under heavy load the diagnostics loop sends the
        // want-mask later, so the worker takes longer to stop the audit pass.
        await page.waitForTimeout(1500);           // want-mask propagates, worker stops the audit pass
        const a0 = await readProbe(page);
        await page.waitForTimeout(800);
        const a1 = await readProbe(page);

        expect(a1.tick, 'sim must keep ticking (so a frozen audit is the gate, not a pause)').toBeGreaterThan(a0.tick);
        expect(a0.hasAudit && a1.hasAudit, 'audit object still served (last value)').toBe(true);
        expect(a1.fieldE, 'audit must be FROZEN when no consumer is visible').toBe(a0.fieldE);

        // ── Phase B: open Lagrangian panel → audit resumes (catch-up + live) ──
        await page.evaluate(() => {
            const t = document.querySelector('#tab-bar .tab[data-panel="lagrangian"]');
            if (t) t.click();
        });
        expect(await page.evaluate(() => window.__ftdCtx.isPanelVisible('lagrangian')),
            'lagrangian panel should read as visible').toBe(true);

        await page.waitForTimeout(500);            // mask on + catch-up + a few computes
        const b0 = await readProbe(page);
        await page.waitForTimeout(800);
        const b1 = await readProbe(page);

        expect(b1.tick, 'sim still ticking').toBeGreaterThan(b0.tick);
        expect(b1.fieldE !== b0.fieldE, 'audit must be LIVE again when the panel is open').toBe(true);
        expect(Math.abs(Number(b1.fieldE) || 0) > 0, 'audit must be non-zero (wired)').toBe(true);
    });

    test('quantify the per-tick audit O(N³) cost the gate reclaims (L=97)', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);

        // Build a throwaway in-thread MockBridge at L=97 and time the audit's
        // derived-field pass (ensureFieldDerivedCache: curl/Poynting/divJ) — the
        // exact O(N³) work the worker now SKIPS every tick when no panel is open.
        // Measured after getDiagnostics() so the shared energy cache is warm and
        // the number is purely the audit-only cost the gate removes.
        const cost = await page.evaluate(async () => {
            const { MockBridge } = await import('/js/bridge/mock-bridge.js');
            const b = new MockBridge(97);
            b.setupScenario('flux-pulse');
            const s0 = b.capabilities.scale0;
            for (let i = 0; i < 20; i++) s0.tickScale0();        // evolve past the seed
            b.getDiagnostics(); b.getEnergyAudit();               // warm caches once
            const N = 15;
            let tDiag = 0, tAudit = 0;
            for (let i = 0; i < N; i++) {
                s0.tickScale0();                                  // invalidate per-tick caches
                let t = performance.now(); b.getDiagnostics();    let dDiag = performance.now() - t;
                t = performance.now(); b.getEnergyAudit();        let dAudit = performance.now() - t;
                tDiag += dDiag; tAudit += dAudit;
            }
            return { L: b.latticeSize, diagMs: tDiag / N, auditMs: tAudit / N };
        });

        console.log(`\n[Phase 1 gain] L=${cost.L}: audit-only ensureFieldDerivedCache ≈ ${cost.auditMs.toFixed(2)} ms/tick ` +
            `(diagnostics ≈ ${cost.diagMs.toFixed(2)} ms/tick). The worker pays this EVERY tick in the legacy ` +
            `path; demand-gating skips it whenever no audit/Lagrangian consumer is visible.\n`);

        // Sanity: the audit pass is a real, non-trivial per-tick cost at L=97
        // (this is *why* gating restores worker throughput — not a microscopic win).
        expect(cost.auditMs, 'audit derived-field pass should be a measurable per-tick cost at L=97').toBeGreaterThan(0.5);
    });
});
