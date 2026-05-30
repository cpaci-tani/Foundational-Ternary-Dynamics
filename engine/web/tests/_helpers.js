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
    await page.goto(path);
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
