// @ts-check
import { test, expect } from '@playwright/test';

/**
 * Scale-switching smoke suite for the FTD web dashboard.
 *
 * What this suite catches:
 *   - Import graph breakage (missing modules → 404s during module load)
 *   - Console errors during page load and during scale switches
 *   - Mode-specific controllers failing to initialize (no bridge, null ctx)
 *   - Scale 5 cosmic physics cadence regressions (Phase B.1)
 *   - Scale 11 consciousness listener leak regressions (Phase B.2)
 *
 * What it does NOT do:
 *   - Visual regression (GPU nondeterminism makes screenshot diffing unreliable)
 *   - Cross-browser (Chromium only; we use Three.js + importmaps)
 *   - Physics correctness (covered by C++ CTests and Python pytest)
 */

/** Helper: set the engine-mode select and fire its change handler. */
async function switchMode(page, mode) {
    await page.evaluate((m) => {
        const sel = document.getElementById('engine-mode');
        if (!sel) throw new Error('engine-mode select not found');
        sel.value = m;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, mode);
}

/** Helper: collect console errors into an array for later assertion. */
function attachConsoleWatcher(page) {
    const errors = [];
    page.on('console', (msg) => {
        if (msg.type() === 'error') errors.push(msg.text());
    });
    page.on('pageerror', (err) => {
        errors.push(`pageerror: ${err.message}`);
    });
    return errors;
}

/** Helper: collect failed network requests. */
function attachNetworkWatcher(page) {
    const failures = [];
    page.on('requestfailed', (req) => {
        failures.push(`${req.method()} ${req.url()} — ${req.failure()?.errorText}`);
    });
    page.on('response', (resp) => {
        if (resp.status() >= 400) failures.push(`${resp.status()} ${resp.url()}`);
    });
    return failures;
}

const KNOWN_NOISE = [
    // WebAssembly abort from ws-bridge exponential backoff on ws://localhost:9100;
    // optional native GPU path, absence is expected in a browser-only test.
    /^Aborted\(\)$/,
    // ws-bridge reconnect logs — benign
    /\[ws-bridge\]/,
    // Chrome font preload warning
    /was preloaded using link preload/,
];

function isNoise(msg) {
    return KNOWN_NOISE.some((rx) => rx.test(msg));
}

test.beforeEach(async ({ page }) => {
    // Grant a bit of extra time for initial WASM compile + module graph load
    page.setDefaultTimeout(20_000);
});

test('index.html loads, bridge initializes, zero 404s', async ({ page }) => {
    const errors = attachConsoleWatcher(page);
    const failures = attachNetworkWatcher(page);

    await page.goto('/index.html');

    // Wait for the main app to wire up its debug bridge accessor
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000, message: 'window._ftdBridge never became non-null' })
        .toBe(true);

    // Give WASM + scale controllers a moment to settle
    await page.waitForTimeout(1500);

    const relevantErrors = errors.filter((e) => !isNoise(e));
    expect(relevantErrors, `Console errors: ${relevantErrors.join('\n')}`).toHaveLength(0);
    expect(failures, `Failed requests: ${failures.join('\n')}`).toHaveLength(0);
});

const MODES = ['lattice', 'particles', 'atoms', 'molecules', 'planetary', 'cosmic', 'meta', 'consciousness'];

for (const mode of MODES) {
    test(`scale switch: ${mode} loads without errors`, async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        const failures = attachNetworkWatcher(page);

        await page.goto('/index.html');
        await expect.poll(() => page.evaluate(() => typeof window._ftdBridge !== 'undefined'),
            { timeout: 15_000 }).toBe(true);
        await page.waitForTimeout(800);

        await switchMode(page, mode);
        await page.waitForTimeout(1500);

        // Bridge should still be alive after the mode switch
        const bridgeAlive = await page.evaluate(() => !!window._ftdBridge);
        expect(bridgeAlive, `bridge lost after switching to ${mode}`).toBe(true);

        const relevantErrors = errors.filter((e) => !isNoise(e));
        expect(relevantErrors, `Errors switching to ${mode}:\n${relevantErrors.join('\n')}`).toHaveLength(0);

        const bad = failures.filter((f) => !/favicon|\/ws/.test(f));
        expect(bad, `Failed requests switching to ${mode}:\n${bad.join('\n')}`).toHaveLength(0);
    });
}

test('Scale 11 consciousness: listener count stable across 5 re-entries (Phase B.2)', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000 }).toBe(true);
    // Warm up: enter/leave consciousness once BEFORE installing the patch so
    // one-time first-load initialization lands on the untracked baseline.
    await page.evaluate(async () => {
        const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
        const eng = document.getElementById('engine-mode');
        eng.value = 'consciousness';
        eng.dispatchEvent(new Event('change', { bubbles: true }));
        await sleep(800);
        eng.value = 'lattice';
        eng.dispatchEvent(new Event('change', { bubbles: true }));
        await sleep(400);
    });

    const samples = await page.evaluate(async () => {
        let adds = 0, rems = 0;
        const origAdd = EventTarget.prototype.addEventListener;
        const origRem = EventTarget.prototype.removeEventListener;
        EventTarget.prototype.addEventListener = function (...a) { adds++; return origAdd.apply(this, a); };
        EventTarget.prototype.removeEventListener = function (...a) { rems++; return origRem.apply(this, a); };

        const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
        const eng = document.getElementById('engine-mode');

        const cycleAdds = [];
        const cycleRems = [];
        for (let i = 0; i < 5; i++) {
            eng.value = 'consciousness';
            eng.dispatchEvent(new Event('change', { bubbles: true }));
            await sleep(600);
            eng.value = 'lattice';
            eng.dispatchEvent(new Event('change', { bubbles: true }));
            await sleep(400);
            cycleAdds.push(adds);
            cycleRems.push(rems);
        }

        EventTarget.prototype.addEventListener = origAdd;
        EventTarget.prototype.removeEventListener = origRem;
        return { cycleAdds, cycleRems };
    });

    // After warm-up, consciousness re-entries must be pure no-ops for the
    // event-listener count. Phase B.2 keeps _csPedagogy alive, so wireSubTabs
    // and ConsciousnessPedagogy() both run zero times on re-entry.
    // Net = adds - rems should be 0 for every cycle (some internal churn
    // from other scales may still add+remove symmetrically).
    const nets = samples.cycleAdds.map((a, i) => a - samples.cycleRems[i]);
    const firstNet = nets[0];
    for (let i = 1; i < nets.length; i++) {
        expect(nets[i],
            `cycle ${i}: net listener count drifted from ${firstNet} to ${nets[i]} ` +
            `(adds=${JSON.stringify(samples.cycleAdds)}, rems=${JSON.stringify(samples.cycleRems)})`)
            .toBe(firstNet);
    }
});

test('Scale 5 cosmic: no _cosmicInterval leak after Phase B.1', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000 }).toBe(true);

    await switchMode(page, 'cosmic');
    await page.waitForTimeout(1500);

    // After Phase B.1, cosmic physics runs inside animateCosmic (rAF-driven),
    // not via a module-level setInterval. window._cosmicInterval must NEVER
    // be set.
    const hasInterval = await page.evaluate(() => !!window._cosmicInterval);
    expect(hasInterval, 'window._cosmicInterval was set — Phase B.1 regression').toBe(false);

    // Leaving cosmic should still be clean.
    await switchMode(page, 'lattice');
    await page.waitForTimeout(500);
    const stillNoInterval = await page.evaluate(() => !!window._cosmicInterval);
    expect(stillNoInterval).toBe(false);
});

test('Constants: K_B matches 0.511 and is a named export', async ({ page }) => {
    await page.goto('/index.html');
    const k = await page.evaluate(async () => {
        const mod = await import('./js/constants.js');
        return { K_B: mod.K_B, hasAlpha: typeof mod.ALPHA === 'number', hasGStar: typeof mod.G_STAR === 'number' };
    });
    expect(k.K_B).toBe(0.511);
    expect(k.hasAlpha).toBe(true);
    expect(k.hasGStar).toBe(true);
});

test('Scale 0 module contract and scenario registry are wired', async ({ page }) => {
    await page.goto('/index.html');
    await expect.poll(() => page.evaluate(() => !!window._ftdBridge),
        { timeout: 15_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const controller = await import('./js/scales/scale0/controller.js');
        const registry = await import('./js/scales/scale0/scenario-registry.js');
        const requiredFns = ['bindUI', 'enter', 'exit', 'loadScenario', 'animate', 'step', 'reset', 'resize'];
        const moduleShapeOk = requiredFns.every((name) => typeof controller[name] === 'function');
        const validation = registry.validateScale0ScenarioRegistry();
        const select = document.getElementById('scenario-select');
        return {
            moduleShapeOk,
            validation,
            optionCount: select?.options.length || 0,
            scenarioCount: registry.SCALE0_SCENARIOS.length,
            firstScenario: registry.SCALE0_SCENARIOS[0]?.id,
            firstOption: select?.options[0]?.value || null,
        };
    });

    expect(result.moduleShapeOk).toBe(true);
    expect(result.validation.ok, `Registry errors: ${result.validation.errors.join(', ')}`).toBe(true);
    expect(result.optionCount).toBe(result.scenarioCount);
    expect(result.firstOption).toBe(result.firstScenario);
});
