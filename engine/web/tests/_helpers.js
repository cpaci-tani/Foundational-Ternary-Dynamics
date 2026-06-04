// @ts-check
/**
 * Shared helpers for Playwright specs.
 *
 * Previously these lived duplicated in 3 spec files (scales.spec.js,
 * animation-clock-freeze.spec.js, wasm-scenario-coverage.spec.js). Extracted
 * per refactoring-analyst ticket RF-8 so all specs share one definition of
 * "what it means for the bridge to be ready" — important because that check
 * is race-sensitive (waitForFunction on the presence of window._ftdBridge).
 */

/**
 * Navigate to the dashboard and wait for the bridge to initialize.
 * All specs should use this instead of raw page.goto() + waitForFunction.
 * @param {import('@playwright/test').Page} page
 * @param {{ path?: string, timeout?: number }} [opts]
 */
export async function gotoAndReady(page, opts = {}) {
    const path = opts.path ?? '/';
    const timeout = opts.timeout ?? 15_000;
    await page.goto(path, { waitUntil: 'domcontentloaded', timeout: Math.max(timeout, 60_000) });
    await page.waitForFunction(() => !!window._ftdBridge, { timeout });
}

/**
 * Set the engine-mode select and fire its change handler.
 * @param {import('@playwright/test').Page} page
 * @param {string} mode - one of 'lattice', 'particles', 'atoms', 'molecules',
 *   'planetary', 'cosmic', 'meta', 'reference frame context', 'hamiltonian-bridge'
 */
export async function switchMode(page, mode) {
    await page.evaluate((m) => {
        const sel = document.getElementById('engine-mode');
        if (!sel) throw new Error('engine-mode select not found');
        sel.value = m;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, mode);
}

/**
 * Collect console errors + page errors into an array.
 * Usage: `const errors = attachConsoleWatcher(page); /* ...run...*\/; expect(errors).toHaveLength(0);`
 * @param {import('@playwright/test').Page} page
 * @returns {string[]} — mutable array populated as errors arrive
 */
export function attachConsoleWatcher(page) {
    /** @type {string[]} */
    const errors = [];
    page.on('console', (msg) => {
        if (msg.type() === 'error') errors.push(msg.text());
    });
    page.on('pageerror', (err) => {
        errors.push(`pageerror: ${err.message}`);
    });
    return errors;
}

/**
 * Collect failed network requests and ≥400 HTTP responses.
 * @param {import('@playwright/test').Page} page
 * @returns {string[]}
 */
export function attachNetworkWatcher(page) {
    /** @type {string[]} */
    const failures = [];
    page.on('requestfailed', (req) => {
        failures.push(`${req.method()} ${req.url()} — ${req.failure()?.errorText}`);
    });
    page.on('response', (resp) => {
        if (resp.status() >= 400) failures.push(`${resp.status()} ${resp.url()}`);
    });
    return failures;
}

/**
 * Known-benign console noise that should be filtered out of assertion arrays.
 * Keep this tight — adding a pattern here hides errors for everyone.
 */
export const KNOWN_NOISE = [
    // WebAssembly abort from ws-bridge exponential backoff on ws://localhost:9100;
    // optional native GPU path, absence is expected in a browser-only test.
    /^Aborted\(\)$/,
    // ws-bridge reconnect logs — benign
    /\[ws-bridge\]/,
    // Chrome font preload warning
    /was preloaded using link preload/,
    // WebSocket connection failure on native port 9100 when offline
    /WebSocket connection to 'ws:\/\/(?:127\.0\.0\.1|localhost):9100\/' failed/,
];

/**
 * Test whether a console/error message matches any KNOWN_NOISE pattern.
 * @param {string} msg
 */
export function isNoise(msg) {
    return KNOWN_NOISE.some((rx) => rx.test(msg));
}

/**
 * Filter an errors array (from attachConsoleWatcher) down to only real errors.
 * @param {string[]} errors
 */
export function realErrors(errors) {
    return errors.filter((e) => !isNoise(e));
}

// ────────────────────────────────────────────────────────────────────────
// Lifecycle / leak-proxy helpers (ticket W7-1 — lifecycle harness).
//
// These read the SAME live introspection hooks the existing specs use:
//   - window.__ftdRAF  (rAF subscriber coordinator, raf-coordinator.js:157;
//                       .size() = subscriber count — the primary leak proxy)
//   - window.__ftdCtx  (Scale-0 publishes the live AppContext on enter,
//                       controller.js:206; ctx.viewport is the Viewport
//                       instance, app.js:541/793 — renderer/camera/controls
//                       are reachable through it)
// They are additive (new exports) and touch no existing helper.
// ────────────────────────────────────────────────────────────────────────

/**
 * Read the current rAF-coordinator subscriber count from the live page.
 * Returns 0 if the coordinator is not yet exposed (defensive — it is
 * installed on window at module load, raf-coordinator.js:156-158).
 * @param {import('@playwright/test').Page} page
 * @returns {Promise<number>}
 */
export async function rafSize(page) {
    return page.evaluate(() => window.__ftdRAF?.size?.() ?? 0);
}

/**
 * Read Three.js `renderer.info.memory` (geometry/texture counts) from the
 * live Viewport, reached via window.__ftdCtx.viewport.renderer.info.memory
 * (Viewport owns `this.renderer = new THREE.WebGLRenderer(...)`,
 * viewport.js:146; ctx.viewport is that Viewport, app.js:793).
 *
 * Returns null when the path is not reachable (e.g. ctx not yet published)
 * so callers can decide to skip rather than assert on garbage.
 * @param {import('@playwright/test').Page} page
 * @returns {Promise<{geometries:number, textures:number}|null>}
 */
export async function getRendererMemory(page) {
    return page.evaluate(() => {
        const info = window.__ftdCtx?.viewport?.renderer?.info;
        const mem = info?.memory;
        if (!mem || typeof mem.geometries !== 'number' || typeof mem.textures !== 'number') {
            return null;
        }
        return { geometries: mem.geometries, textures: mem.textures };
    });
}

/**
 * Read the load-bearing lattice camera/controls fields from the live
 * Viewport: camera.{far, position} + controls.{maxDistance, target}.
 * Path: window.__ftdCtx.viewport.{camera,controls} (viewport.js:142/153).
 * Returns null when unreachable.
 * @param {import('@playwright/test').Page} page
 * @returns {Promise<{far:number, posX:number, posY:number, posZ:number, maxDistance:number, targetX:number, targetY:number, targetZ:number}|null>}
 */
export async function getCameraState(page) {
    return page.evaluate(() => {
        const vp = window.__ftdCtx?.viewport;
        const cam = vp?.camera;
        const ctr = vp?.controls;
        if (!cam || !ctr || !cam.position || !ctr.target) return null;
        return {
            far: cam.far,
            posX: cam.position.x, posY: cam.position.y, posZ: cam.position.z,
            maxDistance: ctr.maxDistance,
            targetX: ctr.target.x, targetY: ctr.target.y, targetZ: ctr.target.z,
        };
    });
}

/**
 * Switch through every mode in `modes` (with a small settle after each so
 * the WASM/Three controllers finish mount/teardown) and return to the first
 * mode (the lattice baseline). Used to exercise the full mount→destroy path
 * of every scale controller in one sweep.
 * @param {import('@playwright/test').Page} page
 * @param {string[]} modes - ordered mode list; modes[0] is the return target
 * @param {{ settleMs?: number }} [opts]
 */
export async function fullModeSweep(page, modes, opts = {}) {
    const settleMs = opts.settleMs ?? 350;
    for (const mode of modes) {
        await switchMode(page, mode);
        await page.waitForTimeout(settleMs);
    }
    // Land back on the baseline (first) mode so the page is left on lattice.
    await switchMode(page, modes[0]);
    await page.waitForTimeout(settleMs);
}
