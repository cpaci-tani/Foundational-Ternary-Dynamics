// @ts-check
/**
 * flux-zero-point — the Zero-Point Energy scenario.
 *
 * Pins the two boundary-independent properties:
 *   1. NO manifestation — `manifested` stays 0. The scenario seeds uniform
 *      sub-threshold random flux (~0.3·K_B, ~20× below K_GENESIS = N_c·K_B)
 *      with genesis OFF (config/toggles.js), so no state-particle ever forms —
 *      unlike flux-vacuum-foam, which is near-threshold + genesis ON.
 *   2. Live, non-zero field — totalEnergy is seeded non-zero and the sim tick
 *      keeps advancing (the field is evolving, not frozen).
 *
 * (Whether the floor PERSISTS vs. slowly decays depends on the boundary
 * condition — reflective traps the energy, absorbing bleeds it out — and is
 * asserted separately once the boundary design is settled.)
 *
 * NOTE: flux-* runs on the flux mock, which (with coi-serviceworker) is a
 * worker proxy that self-ticks only while RUNNING and posts diagnostics
 * asynchronously — so this spec ensures running and lets real wall-clock time
 * pass rather than calling tickScale0() inline.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.beforeEach(async ({ page }) => { page.setDefaultTimeout(25_000); });

async function waitForCtx(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
        { timeout: 15_000, message: 'window.__ftdCtx.bridge never became available' },
    ).toBe(true);
}

async function selectScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('scenario-select');
        if (![...sel.options].some((o) => o.value === scenarioId)) {
            sel.add(new Option(scenarioId, scenarioId));
        }
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await page.waitForTimeout(700); // let the (sync) load + any async settle
}

/** Read the worker-posted diagnostics off the active Scale-0 bridge. */
async function readZpe(page) {
    return page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const st = getScale0State();
        const caps = (st.useFluxMock && st.fluxMock)
            ? st.fluxMock.capabilities.scale0
            : window.__ftdCtx.bridge.capabilities.scale0;
        const d = caps.getScale0Diagnostics() || {};
        return { useFluxMock: !!st.useFluxMock, tick: d.tick ?? null, manifested: d.manifested ?? null, totalEnergy: d.totalEnergy ?? null };
    });
}

test.describe('Scale-0 zero-point energy (flux-zero-point)', () => {
    test('holds a persistent fluctuation floor with no manifested particles', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await waitForCtx(page);

        await selectScenario(page, 'flux-zero-point');

        // Ensure the sim is RUNNING so the field evolves (worker self-ticks only
        // while running). btn-play carries data-paused.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });
        await page.waitForTimeout(300);

        const t0 = await readZpe(page);
        expect(t0.totalEnergy, 'energy floor should be seeded non-zero').toBeGreaterThan(0);
        expect(t0.manifested, 'no particles at load (genesis off + sub-threshold)').toBe(0);

        // Sample the floor over ~8s of real time. With reflective boundaries
        // (SCALE0_SCENARIO_BOUNDARY) the energy is trapped, so the floor must
        // hold rather than bleed away — and nothing ever manifests.
        const traj = [t0];
        for (let i = 0; i < 4; i++) { await page.waitForTimeout(2000); traj.push(await readZpe(page)); }
        console.log(`[zero-point] ${traj.map((s) => `t${s.tick}:E=${s.totalEnergy},m=${s.manifested}`).join('  ')}`);
        const tN = traj[traj.length - 1];

        expect(tN.tick, `field should be live / sim advancing (t0=${t0.tick}, tN=${tN.tick})`).toBeGreaterThan(t0.tick);
        for (const s of traj) expect(s.manifested, 'no particles should ever manifest').toBe(0);
        // Persistent floor: still ≥ 70% of the seeded value after running (a
        // "held, did not bleed away" check — reflective boundaries trap it).
        expect(tN.totalEnergy,
            `floor did not persist (t0=${t0.totalEnergy} → tN=${tN.totalEnergy}); reflective boundary should trap the energy`)
            .toBeGreaterThan(t0.totalEnergy * 0.7);
    });
});
