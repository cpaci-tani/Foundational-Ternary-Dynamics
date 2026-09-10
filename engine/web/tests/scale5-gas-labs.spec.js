// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

/**
 * Scale 5 gas laboratories (Task 5) — three Monaghan-SPH scenarios plus
 * the axis-profile diagnostic channel and panel card.
 *
 * Reaching the live CosmicMockBridge: `window._ftdBridge` (used by other
 * specs' readiness poll) stays pinned to the Scale-0 lattice bridge for the
 * whole app lifetime — app.js's module-level `bridge` variable is assigned
 * once at boot (bootBridge) and never reassigned on a scale switch. The
 * Scale 5 bridge instance recreated on every `loadCosmicScenario` call is
 * instead reachable at `window.__ftdCtx.inspector.bridge`
 * (scales/scale5/controller.js calls `ctx.inspectorRuntime.setBridge(this.bridge)`,
 * which forwards to `inspector.setBridge()`, storing it on `inspector.bridge`;
 * `window.__ftdCtx` is the same AppContext singleton passed to every scale
 * controller, published once by scale0/controller.js at boot). Verified live
 * against the dev server before writing this spec.
 *
 * The headless pane does not run requestAnimationFrame, so physics is
 * driven directly with `bridge.tick()` in-page, and the panel DOM (the
 * profile card's `hidden` flag, its canvases, its telemetry spans) is
 * synced by calling the exported `animateCosmic(ctx)` once afterward —
 * the same function the real rAF loop calls every other frame.
 */

const GAS_LABS = [
    { id: 'cosmic-gas-collapse', axis: 'r' },
    { id: 'cosmic-gas-cloud-collision', axis: 'x' },
    { id: 'cosmic-gas-rotating-disk', axis: 'r' },
];

/** @param {import('@playwright/test').Page} page @param {string} id */
async function selectCosmicScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = /** @type {HTMLSelectElement|null} */ (document.getElementById('cosmic-scenario-select'));
        if (!sel) throw new Error('cosmic-scenario-select not found');
        if (![...sel.options].some((option) => option.value === scenarioId)) {
            throw new Error(`scenario ${scenarioId} missing from #cosmic-scenario-select`);
        }
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await page.waitForFunction(
        (scenarioId) => document.getElementById('cosmic-scenario-select')?.value === scenarioId,
        id,
    );
}

/**
 * Tick the live cosmic bridge `nTicks` times in-page, then force one
 * `animateCosmic` pass so the panel DOM reflects the post-tick diagnostics.
 * Returns a JSON-serializable snapshot (never the bridge object itself).
 * @param {import('@playwright/test').Page} page
 * @param {number} nTicks
 */
async function tickAndSnapshot(page, nTicks) {
    return page.evaluate(async (n) => {
        const bridge = window.__ftdCtx?.inspector?.bridge;
        if (!bridge) return { error: 'no cosmic bridge at window.__ftdCtx.inspector.bridge' };
        const toggle = bridge.getToggle('sph_monaghan');
        for (let i = 0; i < n; i++) bridge.tick();
        const diag = bridge.getDiagnostics();
        const profile = diag.customProfiles;
        const density = profile ? Array.from(profile.density) : null;

        const mod = await import('/js/scales/scale5/controller.js');
        mod.animateCosmic(window.__ftdCtx);
        const card = document.getElementById('cosmic-gas-profile-card');

        return {
            toggle,
            bodyCount: diag.bodyCount,
            totalThermal: diag.totalThermal,
            totalKE: diag.totalKE,
            hasProfile: !!profile,
            axis: profile ? profile.axis : null,
            gasCount: profile ? profile.gasCount : null,
            densitySum: density ? density.reduce((a, b) => a + b, 0) : null,
            // b.density is written ONLY by the SPH pass (initialised to 0
            // otherwise); densitySum/totalThermal above are both positive
            // even if computeSphForces never runs (a mass histogram and the
            // initial internal_energy respectively), so neither actually
            // proves SPH engaged. This does (M7).
            anyBodyDensityPositive: bridge._bodies.some((b) => b.density > 0),
            cardHidden: card ? card.hidden : null,
            cardExists: !!card,
        };
    }, nTicks);
}

test.beforeEach(async ({ page }) => {
    page.setDefaultTimeout(30_000);
});

for (const lab of GAS_LABS) {
    test(`gas lab: ${lab.id} runs Monaghan SPH and populates the axis profile`, async ({ page }) => {
        const errors = attachConsoleWatcher(page);

        await gotoAndReady(page, { path: '/index.html' });
        await switchMode(page, 'cosmic');
        await page.waitForTimeout(500);

        await selectCosmicScenario(page, lab.id);
        await page.waitForTimeout(300);

        const snap = await tickAndSnapshot(page, 60);

        expect(snap.error, snap.error).toBeUndefined();
        expect(snap.toggle, `${lab.id}: sph_monaghan toggle should be on`).toBe(true);
        expect(snap.hasProfile, `${lab.id}: customProfiles should be populated`).toBe(true);
        expect(snap.axis).toBe(lab.axis);
        expect(snap.gasCount, `${lab.id}: gasCount should be positive`).toBeGreaterThan(0);
        expect(snap.densitySum, `${lab.id}: density profile should have positive mass`).toBeGreaterThan(0);
        expect(snap.totalThermal, `${lab.id}: totalThermal should be positive`).toBeGreaterThan(0);
        expect(snap.anyBodyDensityPositive, `${lab.id}: the SPH pass should have run (b.density > 0 for at least one body)`).toBe(true);
        expect(snap.cardExists, `${lab.id}: #cosmic-gas-profile-card should exist`).toBe(true);
        expect(snap.cardHidden, `${lab.id}: profile card should be visible`).toBe(false);

        const relevantErrors = realErrors(errors);
        expect(relevantErrors, `Console errors on ${lab.id}:\n${relevantErrors.join('\n')}`).toHaveLength(0);
    });
}

test('cosmic-galaxy (existing scenario) leaves SPH off and the profile null', async ({ page }) => {
    const errors = attachConsoleWatcher(page);

    await gotoAndReady(page, { path: '/index.html' });
    await switchMode(page, 'cosmic');
    await page.waitForTimeout(500);

    // cosmic-galaxy is the default scenario cosmic mode loads on entry, but
    // select it explicitly so this test does not depend on that default.
    await selectCosmicScenario(page, 'cosmic-galaxy');
    await page.waitForTimeout(300);

    const snap = await tickAndSnapshot(page, 5);

    expect(snap.error, snap.error).toBeUndefined();
    expect(snap.toggle, 'cosmic-galaxy: sph_monaghan should stay off').toBe(false);
    expect(snap.hasProfile, 'cosmic-galaxy: customProfiles should be null').toBe(false);
    expect(snap.cardHidden, 'cosmic-galaxy: profile card should stay hidden').toBe(true);

    const relevantErrors = realErrors(errors);
    expect(relevantErrors, `Console errors on cosmic-galaxy:\n${relevantErrors.join('\n')}`).toHaveLength(0);
});
