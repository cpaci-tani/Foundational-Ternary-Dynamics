// @ts-check
/**
 * Panel-wiring audit — every Scale-0 sidepanel reads from one of three bridge
 * telemetry sources: `getScale0Diagnostics` (diagnostics table + charts),
 * `getScale0EnergyAudit` (the diagnostics audit-rows + conservation panel +
 * the Lagrangian "Action & Constraints" table), and `getScale0Lagrangian`
 * (the Lagrangian chart + table). This spec loads EVERY scenario on its active
 * bridge and asserts all three are wired — i.e. when a scenario is live (has a
 * flux field or particles), the audit and Lagrangian return live values, not
 * zeros.
 *
 * The gap this guards: on the **worker proxy** path (the default for flux-*),
 * `getScale0EnergyAudit`/`getScale0Lagrangian` used to fall through to the
 * never-ticked shadow and return all-zero, so the audit-rows and the whole
 * Lagrangian panel were dead while the diagnostics table was live. The worker
 * now posts audit + Lagrangian each frame and the proxy serves them.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const KNOWN_EMPTY = new Set([
    'empty',
    's0-seed-emergent-ic4-subthreshold',
    's0-seed-emergent-ic2-thermal-runaway',
]);

async function waitForCtx(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
        { timeout: 20_000, message: 'window.__ftdCtx.bridge never became available' },
    ).toBe(true);
}

test.describe('Scale-0 panel wiring (diagnostics + audit + Lagrangian)', () => {
    test('every scenario wires diagnostics, energy-audit and Lagrangian telemetry', async ({ page }) => {
        test.setTimeout(360_000);
        await gotoAndReady(page);
        await waitForCtx(page);

        const scenarios = await page.evaluate(async () => {
            const m = await import('/js/scales/scale0/scenario-registry.js');
            return m.SCALE0_SCENARIOS.map((s) => s.id);
        });

        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        /** @type {Array<{id:string,owner:string,live:boolean,auditWired:boolean,lagWired:boolean,e:any,a:any,l:any}>} */
        const rows = [];

        for (const id of scenarios) {
            await page.evaluate((scenarioId) => {
                const sel = document.getElementById('scenario-select');
                if (![...sel.options].some((o) => o.value === scenarioId)) sel.add(new Option(scenarioId, scenarioId));
                sel.value = scenarioId;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }, id);
            await page.waitForTimeout(650); // let the worker tick + post audit/lag a few frames

            const snap = await page.evaluate(async () => {
                const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                const st = getScale0State();
                const caps = (st.useFluxMock && st.fluxMock)
                    ? st.fluxMock.capabilities.scale0
                    : window.__ftdCtx.bridge.capabilities.scale0;
                const d = caps.getScale0Diagnostics?.() || {};
                const a = caps.getScale0EnergyAudit?.() || null;
                const l = caps.getScale0Lagrangian?.() || null;
                return {
                    owner: st.useFluxMock ? 'mock' : 'wasm',
                    totalFlux: d.totalFlux ?? null, manifested: d.manifested ?? null,
                    aFieldE: a ? (a.fieldEnergy ?? null) : null,
                    aWaveE:  a ? (a.waveEnergy ?? null) : null,
                    aHasObj: !!a,
                    lTotal:  l ? (l.total ?? null) : null,
                    lHam:    l ? (l.hamiltonian ?? null) : null,
                    lFK:     l ? (l.fieldKinetic ?? null) : null,
                    lHasObj: !!l,
                };
            }, id);

            const live = (Number(snap.totalFlux) > 0.01) || (Number(snap.manifested) > 0);
            const hasFlux = Number(snap.totalFlux) > 0.01;
            // The energy audit + Lagrangian are field-energy driven. The wiring gap
            // we guard is a FLUX scenario whose audit/Lagrangian read zero (the
            // worker-shadow bug). A particle-only scenario legitimately has zero
            // field/wave energy and zero field Lagrangian — so only require non-zero
            // values when there is actually a flux field; otherwise just require the
            // telemetry object to be present (not null).
            const auditWired = snap.aHasObj && (!hasFlux ||
                Math.abs(Number(snap.aFieldE) || 0) > 0 || Math.abs(Number(snap.aWaveE) || 0) > 0);
            const lagWired = snap.lHasObj && (!hasFlux ||
                Math.abs(Number(snap.lTotal) || 0) > 1e-9 ||
                Math.abs(Number(snap.lHam) || 0) > 1e-9 ||
                Math.abs(Number(snap.lFK) || 0) > 1e-9);

            rows.push({ id, owner: snap.owner, live,
                auditWired: auditWired || KNOWN_EMPTY.has(id),
                lagWired: lagWired || KNOWN_EMPTY.has(id),
                e: { f: snap.totalFlux, m: snap.manifested }, a: { fE: snap.aFieldE, wE: snap.aWaveE }, l: { t: snap.lTotal, h: snap.lHam } });
        }

        const fmt = (r) => `${r.auditWired ? 'A' : '·'}${r.lagWired ? 'L' : '·'} ${r.owner.padEnd(4)} ` +
            `${String(r.id).padEnd(34)} flux=${r.e.f} aud(fE=${r.a.fE},wE=${r.a.wE}) lag(t=${r.l.t},H=${r.l.h})`;
        console.log('\n=== Scale-0 panel wiring (A=audit L=lagrangian) ===\n' + rows.map(fmt).join('\n'));

        const auditDead = rows.filter((r) => r.live && !r.auditWired).map((r) => `${r.id} (${r.owner})`);
        const lagDead = rows.filter((r) => r.live && !r.lagWired).map((r) => `${r.id} (${r.owner})`);
        expect(auditDead, 'live scenarios with a dead energy-audit telemetry').toEqual([]);
        expect(lagDead, 'live scenarios with a dead Lagrangian telemetry').toEqual([]);
    });
});
