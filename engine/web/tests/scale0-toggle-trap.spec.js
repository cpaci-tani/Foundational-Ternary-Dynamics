// @ts-check
/**
 * Scale-0 toggle-trap regression: "selective_damping requires damping".
 *
 * The C++ TermToggles::validate() requires selective_damping ⇒ damping. Every
 * tick with selective_damping=true while damping=false emits
 *   [TermToggles] Invalid combination: selective_damping requires damping
 * to stderr (→ console.error in the WASM build). Several Scale-0 scenario presets
 * turn damping OFF; if any leaves selective_damping ON (its default), the engine
 * bursts that error on every tick of that scenario.
 *
 * applyToggleDefaults (scenario-loader.js) clamps the requires-invariant on the
 * merged final toggle state, so no preset — present or future — can leave a
 * dependent ON while its prerequisite is OFF. This spec pins that: it loads each
 * damping-OFF scenario through the real controller path (dropdown change) and
 * asserts (a) no scenario leaves an invalid selective_damping/damping combo and
 * (b) zero TermToggles console errors. It also asserts the flux-pulse default is
 * physics-unchanged (damping + selective_damping both ON).
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

// Scale-0 scenario presets that set damping=false (config/toggles.js). Each is a
// regression case: the loader must clamp selective_damping OFF for all of them.
const DAMPING_OFF_SCENARIOS = [
    'light-rainbow', 'light-dipole', 'light-two-slit', 'light-photon-race',
    'quantum-well', 'flux-zero-point',
    's0-field-rf-lattice-wave', 's0-field-light-lattice-wave', 's0-field-sound-lattice-wave',
    's0-field-thomson-scattering', 's0-field-thomson-unlocked-recoil',
    's0-field-spacetime-forcing-boundary',
];

test.describe('Scale-0 toggle-trap: selective_damping requires damping', () => {
    // Boot (WASM) plus a sweep of scenario loads runs long on a cold headless
    // start; give each test generous headroom over the 60s default.
    test.slow();

    // Pin the IN-THREAD WASM bridge (window.__ftdWasmWorker=false). This is the
    // bridge window._ftdBridge points at and the one this spec ticks, so it is
    // the bridge whose validate() would emit the error. It also makes
    // applyToggleDefaults fully synchronous (no off-thread worker re-init race
    // from rapid scenario switching), so the assertion is deterministic. The
    // clamp under test runs identically for the worker path (it sets the same
    // toggle values the worker is built from).
    test.beforeEach(async ({ page }) => {
        await page.addInitScript(() => { window.__ftdWasmWorker = false; });
    });

    test('no damping-off scenario leaves an invalid combo or emits a TermToggles error', async ({ page }) => {
        /** @type {string[]} */
        const ttErrors = [];
        page.on('console', (msg) => {
            const t = msg.text();
            if (t.includes('TermToggles') || t.includes('Invalid combination')) ttErrors.push(t);
        });

        await gotoAndReady(page, { timeout: 90_000 });

        const offenders = await page.evaluate(async (ids) => {
            const sel = /** @type {HTMLSelectElement} */ (document.getElementById('scenario-select'));
            const b = window._ftdBridge;
            const bad = [];
            for (const id of ids) {
                sel.value = id;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
                await new Promise((r) => setTimeout(r, 250));
                // Tick the in-thread bridge to force C++ TermToggles::validate().
                b.tick();
                b.tick();
                const damping = b.getToggle('damping');
                const selective_damping = b.getToggle('selective_damping');
                if (selective_damping === true && damping === false) {
                    bad.push({ id, damping, selective_damping });
                }
            }
            return bad;
        }, DAMPING_OFF_SCENARIOS);

        expect(offenders, `scenarios left with selective_damping=true while damping=false: ${JSON.stringify(offenders)}`).toHaveLength(0);
        expect(ttErrors, `TermToggles errors emitted: ${JSON.stringify(ttErrors)}`).toHaveLength(0);
    });

    test('flux-pulse default keeps damping + selective_damping ON (physics unchanged)', async ({ page }) => {
        await gotoAndReady(page, { timeout: 90_000 });
        const state = await page.evaluate(() => {
            const b = window._ftdBridge;
            return { damping: b.getToggle('damping'), selective_damping: b.getToggle('selective_damping') };
        });
        expect(state.damping).toBe(true);
        expect(state.selective_damping).toBe(true);
    });
});
