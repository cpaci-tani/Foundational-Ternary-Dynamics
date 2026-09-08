// @ts-check
/**
 * B3 regression — the `langevin` research toggle must not leak across scenario
 * switches.
 *
 * The quark-gluon-plasma C++ scenario body enables the Langevin thermostat.
 * `langevin` is intentionally NOT in `SCALE0_TOGGLES`
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
            return null;
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
    test('langevin does not leak from a thermal scenario into the next', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtx(page);

        // 1) The qualified thermal-transport scenario enables the thermostat.
        await selectScenario(page, 's0-seed-quark-gluon-plasma');
        const thermal = await readLangevin(page);
        const activeLangevin = thermal.useFluxMock ? thermal.langevinMock : thermal.langevinMain;
        expect(activeLangevin,
            `quark-gluon-plasma should run with langevin ON on its active bridge ` +
            `(useFluxMock=${thermal.useFluxMock}, main=${thermal.langevinMain}, mock=${thermal.langevinMock})`)
            .toBe(true);

        // 2) Switch to a scenario that does NOT use langevin — it must be OFF everywhere.
        await selectScenario(page, 'flux-pulse');
        const after = await readLangevin(page);
        expect(after.langevinMain,
            `langevin leaked onto the MAIN bridge after switching to flux-pulse ` +
            `(was set by quark-gluon-plasma; useFluxMock-then=${thermal.useFluxMock})`).toBe(false);
        expect(after.langevinMock ?? false,
            `langevin leaked onto the FLUX MOCK after switching to flux-pulse`).toBe(false);
    });

    test('langevin does not leak from a thermal scenario into a main-bridge scenario', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtx(page);

        await selectScenario(page, 's0-seed-quark-gluon-plasma');
        const thermal = await readLangevin(page);

        // quantum-tunnel runs on the main/WASM bridge — exactly where an inactive
        // bridge pin would resurface, so this is the sequence in which a leak bites.
        await selectScenario(page, 'quantum-tunnel');
        const after = await readLangevin(page);
        expect(after.langevinMain,
            `langevin leaked onto the main bridge into quantum-tunnel ` +
            `(thermal: useFluxMock=${thermal.useFluxMock} main=${thermal.langevinMain} mock=${thermal.langevinMock}; ` +
            `quantum: useFluxMock=${after.useFluxMock})`).toBe(false);
    });
});
