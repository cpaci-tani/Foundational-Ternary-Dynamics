// @ts-check
/**
 * B3 regression — the `langevin` research toggle must not leak across scenario
 * switches.
 *
 * The emergent-ic* / quark-gluon-plasma scenarios enable the langevin thermostat
 * in their custom `load()`. `langevin` is intentionally NOT in `SCALE0_TOGGLES`
 * (config/toggles.js documents it as a user-owned research control), so the
 * loader's whitelist reset never clears it — which RAISED the concern (audit B3)
 * that it could persist into the next scenario.
 *
 * Runtime verification (2026-06-05) shows it does NOT leak: every scenario load
 * resets the bridge (via `setupScenario`), which clears langevin, so switching
 * away from an emergent scenario leaves it OFF on both the main bridge and the
 * flux mock. This spec pins that (no-leak) behaviour as a permanent guard;
 * audit finding B3 is closed as not-reproduced.
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

/** Read the langevin toggle on both the main bridge and the flux mock (if any). */
async function readLangevin(page) {
    return page.evaluate(async () => {
        const ctx = window.__ftdCtx;
        const store = await import('/js/scales/scale0/state/store.js');
        const st = store.getScale0State();
        const rd = (b) => {
            if (!b) return null;
            const caps = b.capabilities && b.capabilities.scale0;
            if (caps && typeof caps.getToggle === 'function') return !!caps.getToggle('langevin');
            if (typeof b.getToggle === 'function') return !!b.getToggle('langevin');
            const t = b._toggles || b.toggles;
            return t ? !!t.langevin : null;
        };
        return {
            scenario: st.currentScenarioId,
            useFluxMock: !!st.useFluxMock,
            langevinMain: rd(ctx.bridge),
            langevinMock: rd(st.fluxMock),
        };
    });
}

test.describe('Scale-0 langevin toggle-leak (B3)', () => {
    test('langevin does not leak from an emergent scenario into the next', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtx(page);

        // 1) An emergent scenario enables the langevin thermostat in its load().
        await selectScenario(page, 's0-seed-emergent-ic1');
        const emergent = await readLangevin(page);
        const activeLangevin = emergent.useFluxMock ? emergent.langevinMock : emergent.langevinMain;
        expect(activeLangevin,
            `emergent-ic1 should run with langevin ON on its active bridge ` +
            `(useFluxMock=${emergent.useFluxMock}, main=${emergent.langevinMain}, mock=${emergent.langevinMock})`)
            .toBe(true);

        // 2) Switch to a scenario that does NOT use langevin — it must be OFF everywhere.
        await selectScenario(page, 'flux-pulse');
        const after = await readLangevin(page);
        expect(after.langevinMain,
            `langevin leaked onto the MAIN bridge after switching to flux-pulse ` +
            `(was set by emergent-ic1; useFluxMock-then=${emergent.useFluxMock})`).toBe(false);
        expect(after.langevinMock ?? false,
            `langevin leaked onto the FLUX MOCK after switching to flux-pulse`).toBe(false);
    });

    test('langevin does not leak from an emergent scenario into a main-bridge scenario', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtx(page);

        await selectScenario(page, 's0-seed-emergent-ic1');
        const emergent = await readLangevin(page);

        // quantum-tunnel runs on the main/WASM bridge — exactly where the emergent
        // custom-load sets langevin, so this is the sequence in which a leak bites.
        await selectScenario(page, 'quantum-tunnel');
        const after = await readLangevin(page);
        expect(after.langevinMain,
            `langevin leaked onto the main bridge into quantum-tunnel ` +
            `(emergent: useFluxMock=${emergent.useFluxMock} main=${emergent.langevinMain} mock=${emergent.langevinMock}; ` +
            `quantum: useFluxMock=${after.useFluxMock})`).toBe(false);
    });
});
