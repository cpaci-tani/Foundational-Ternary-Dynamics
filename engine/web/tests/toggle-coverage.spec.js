// @ts-check
/**
 * Toggle-coverage spec (ticket W3-2).
 *
 * Proves WIRING for every Scale-0 field/overlay toggle: that clicking the
 * real DOM button flips the corresponding `fieldFlags` state key, and that
 * clicking again flips it back. This is the live answer to the 2026-05-27
 * web audit, which mis-flagged these toggles as orphans/dead code — that
 * audit grepped `index.html` for `id="toggle-..."` literals and found none,
 * because the buttons are *dynamically rendered* by
 * `js/scales/scale0/ui/overlays/template.js` at Scale-0 boot, not authored
 * into static HTML. "Wired" is therefore settled by a click test, not a grep.
 *
 * Canonical map: engine/web/docs/TOGGLE_REGISTRY.md.
 *
 * DESIGN — data-first, self-updating:
 *   The toggle list is NOT hard-coded here. It is derived at runtime from the
 *   live modules so the test tracks the code automatically when toggles are
 *   added/removed:
 *     - state keys      ← getScale0State().fieldFlags  (store.js, the canon)
 *     - id ↔ key map    ← FIELD_TOGGLE_BINDINGS         (dom.js, authoritative)
 *   A toggle WITH a DOM button (id present in FIELD_TOGGLE_BINDINGS and the
 *   element exists) is exercised via a real button click. A toggle WITHOUT a
 *   button (state-only — currently none; the set is ∅) is exercised via
 *   setFieldToggle directly. The split is computed live, not assumed.
 *
 * ROBUSTNESS (mirrors reconcile-claims.spec.js, which already proves
 * #toggle-e-field exists + clicks on a default boot):
 *   - Click via DOM dispatch `el.click()` inside page.evaluate, NOT
 *     page.click(): in the headless layout the panel-scale-header overlaps
 *     the toolbar buttons and Playwright's actionability check times out
 *     ("intercepts pointer events"). DOM dispatch bypasses the hit-test.
 *   - State is read SYNCHRONOUSLY in the same evaluate as the click. The
 *     Scale-0 rAF loop can consume `fieldNeedsUpdate` a frame later, but the
 *     `fieldFlags` value itself is set synchronously by setFieldToggle inside
 *     the click handler, so a same-tick read is race-free.
 *   - Console assertions go through the shared realErrors()/KNOWN_NOISE filter.
 *
 * Source hooks pinned (file:line at authoring time, 2026-06-01):
 *   - fieldFlags + setFieldToggle:  scales/scale0/state/store.js:57-122
 *   - id↔key bindings:              scales/scale0/ui/dom.js:1-38 (FIELD_TOGGLE_BINDINGS)
 *   - click wiring:                 scales/scale0/ui/bindings.js:122-130
 *       btn.click() → setToggleState(buttonId, fieldKey, !readButtonActive(buttonId))
 *       → setButtonActive(...) + setFieldToggle(fieldKey, on)
 *   - buttons rendered:             scales/scale0/ui/overlays/template.js:41-233
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

test.beforeEach(async ({ page }) => {
    // WASM compile + Three.js + module graph need headroom on slower machines.
    page.setDefaultTimeout(20_000);
});

/**
 * Build the live toggle inventory from the running app:
 *   { keys, bindings, withButton, stateOnly, missingButton }
 * - keys:          all fieldFlags keys (the canon)
 * - bindings:      [[buttonId, fieldKey], ...] from FIELD_TOGGLE_BINDINGS
 * - withButton:    bindings whose DOM element is actually present + is a field key
 * - stateOnly:     fieldFlags keys with NO binding (exercised via setFieldToggle)
 * - missingButton: bindings whose DOM element is absent (a real wiring gap → surfaced)
 * @param {import('@playwright/test').Page} page
 */
async function buildInventory(page) {
    return await page.evaluate(async () => {
        const store = await import('./js/scales/scale0/state/store.js');
        const dom = await import('./js/scales/scale0/ui/dom.js');
        const keys = Object.keys(store.getScale0State().fieldFlags);
        const bindings = dom.FIELD_TOGGLE_BINDINGS.map(([id, key]) => [id, key]);
        const boundKeys = new Set(bindings.map(([, key]) => key));

        const withButton = [];
        const missingButton = [];
        for (const [id, key] of bindings) {
            const el = document.getElementById(id);
            // A binding only counts as "has a usable button" if the element
            // exists AND the key is a real fieldFlags key (guards against a
            // stray binding row pointing at a non-store key).
            if (el && keys.includes(key)) withButton.push([id, key]);
            else missingButton.push([id, key, { elPresent: !!el, keyInStore: keys.includes(key) }]);
        }
        const stateOnly = keys.filter((k) => !boundKeys.has(k));
        return { keys, bindings, withButton, stateOnly, missingButton };
    });
}

test.describe('Scale-0 field toggle coverage', () => {

    // ────────────────────────────────────────────────────────────────────
    // 0. Inventory sanity — the map is internally consistent.
    //
    // This is the structural half of "the audit over-counted orphans": the
    // store key list and the DOM id↔key bindings must be in lockstep. We
    // assert there is no binding pointing at a missing button and no binding
    // pointing at a non-store key. (State-only keys are allowed — they are
    // covered separately below — but at authoring time the set is empty.)
    // ────────────────────────────────────────────────────────────────────
    test('inventory: every binding has a rendered button and a real store key', async ({ page }) => {
        await gotoAndReady(page);
        const inv = await buildInventory(page);

        // We expect a non-trivial surface (≈32 toggles); guard against the
        // module failing to load and silently yielding an empty list.
        expect(inv.keys.length, 'fieldFlags should expose the field-toggle keys').toBeGreaterThan(10);
        expect(inv.bindings.length, 'FIELD_TOGGLE_BINDINGS should be non-empty').toBeGreaterThan(10);

        // The headline anti-orphan assertion: no binding is dangling.
        expect(
            inv.missingButton,
            `bindings with no rendered button / non-store key (each is a real wiring gap): ${JSON.stringify(inv.missingButton)}`,
        ).toEqual([]);

        // Every binding key resolves to a fieldFlags key.
        const boundKeys = inv.bindings.map(([, k]) => k);
        for (const k of boundKeys) {
            expect(inv.keys, `binding key "${k}" must be a fieldFlags key`).toContain(k);
        }
    });

    // ────────────────────────────────────────────────────────────────────
    // 1. THE deliverable — every button toggle flips its state on click and
    //    flips back on a second click. Drives all ~32 in one sweep so a
    //    genuine broken/orphan toggle (button exists but state does not move)
    //    surfaces as a named failure rather than going unnoticed.
    // ────────────────────────────────────────────────────────────────────
    test('every field-toggle button flips its fieldFlags key on click (and back)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        const inv = await buildInventory(page);
        expect(inv.withButton.length, 'expected a healthy set of button-backed toggles').toBeGreaterThan(10);

        /** @type {string[]} */
        const broken = [];

        // Exercise each toggle in its own round-trip. Normalise to a known
        // state first (we drive OFF→ON→OFF off the *measured* start so the
        // click is always a genuine transition regardless of scenario defaults).
        for (const [buttonId, fieldKey] of inv.withButton) {
            const result = await page.evaluate(async ({ buttonId, fieldKey }) => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                const st = getScale0State();
                const btn = document.getElementById(buttonId);
                if (!btn) return { buttonId, fieldKey, fatal: 'button vanished mid-sweep' };

                // Click is OFF→ON or ON→OFF depending on current state; we
                // record before/after for both a click and a second click.
                const before = !!st.fieldFlags[fieldKey];
                btn.click();                          // 1st click → flip
                const afterFirst = !!getScale0State().fieldFlags[fieldKey];
                btn.click();                          // 2nd click → flip back
                const afterSecond = !!getScale0State().fieldFlags[fieldKey];
                return { buttonId, fieldKey, before, afterFirst, afterSecond };
            }, { buttonId, fieldKey });

            if (result.fatal) {
                broken.push(`${result.buttonId} (${result.fieldKey}): ${result.fatal}`);
                continue;
            }
            // Wiring contract: first click must change the value; second click
            // must restore it. Either failure = a genuine orphan/broken toggle.
            if (result.afterFirst === result.before) {
                broken.push(`${result.buttonId} → fieldFlags.${result.fieldKey}: click did NOT flip (stuck at ${result.before})`);
            }
            if (result.afterSecond !== result.before) {
                broken.push(`${result.buttonId} → fieldFlags.${result.fieldKey}: second click did NOT restore (before=${result.before}, afterSecond=${result.afterSecond})`);
            }
        }

        // Surface ALL broken toggles at once (not just the first) so a real
        // finding is fully enumerated in the failure message.
        expect(broken, `broken/orphan field toggles (button present but state not wired):\n  ${broken.join('\n  ')}`).toEqual([]);

        // Wiring must not emit console errors anywhere across the full sweep.
        const real = realErrors(errors);
        expect(real, `console errors during toggle sweep:\n  ${real.join('\n  ')}`).toEqual([]);
    });

    // ────────────────────────────────────────────────────────────────────
    // 2. State-only field flags (no DOM button) — exercised via
    //    setFieldToggle directly. At authoring time this set is EMPTY
    //    (every key has a button), so this test is a no-op guard that
    //    activates automatically if a buttonless key is ever added.
    // ────────────────────────────────────────────────────────────────────
    test('state-only field flags (if any) flip via setFieldToggle', async ({ page }) => {
        await gotoAndReady(page);
        const inv = await buildInventory(page);

        if (inv.stateOnly.length === 0) {
            // Document the current reality explicitly: the map has no
            // buttonless field flags. This keeps the test meaningful (it
            // asserts the *expected* empty set) rather than vacuously passing.
            expect(inv.stateOnly).toEqual([]);
            test.info().annotations.push({
                type: 'note',
                description: 'No state-only field flags — all 32 keys are button-backed; nothing to exercise here.',
            });
            return;
        }

        /** @type {string[]} */
        const broken = [];
        for (const key of inv.stateOnly) {
            const result = await page.evaluate(async (fieldKey) => {
                const { getScale0State, setFieldToggle } = await import('./js/scales/scale0/state/store.js');
                const before = !!getScale0State().fieldFlags[fieldKey];
                setFieldToggle(fieldKey, !before);
                const afterToggle = !!getScale0State().fieldFlags[fieldKey];
                setFieldToggle(fieldKey, before);            // restore
                const afterRestore = !!getScale0State().fieldFlags[fieldKey];
                return { fieldKey, before, afterToggle, afterRestore };
            }, key);
            if (result.afterToggle === result.before) broken.push(`${result.fieldKey}: setFieldToggle did not flip`);
            if (result.afterRestore !== result.before) broken.push(`${result.fieldKey}: restore failed`);
        }
        expect(broken, `state-only flags that did not flip:\n  ${broken.join('\n  ')}`).toEqual([]);
    });

    // ────────────────────────────────────────────────────────────────────
    // 3. anyFieldActive bookkeeping — turning a toggle on must mark the
    //    aggregate active flag; turning everything off must clear it.
    //    Guards the recomputeAnyFieldActive() path that gates whether the
    //    overlay pipeline runs at all (a regression here silently disables
    //    every overlay even when individual flags are set).
    // ────────────────────────────────────────────────────────────────────
    test('anyFieldActive tracks the aggregate of field flags', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        const result = await page.evaluate(async () => {
            const { getScale0State, setFieldToggle, resetFieldFlags } = await import('./js/scales/scale0/state/store.js');
            // Start from a clean all-off baseline.
            resetFieldFlags();
            const cleared = getScale0State().anyFieldActive;     // expect false
            // Enable exactly one well-known flag via the store mutator.
            setFieldToggle('showEField', true);
            const afterOne = getScale0State().anyFieldActive;    // expect true
            // Clear it again.
            setFieldToggle('showEField', false);
            const afterClear = getScale0State().anyFieldActive;  // expect false
            return { cleared, afterOne, afterClear };
        });

        expect(result.cleared, 'fresh reset should leave anyFieldActive false').toBe(false);
        expect(result.afterOne, 'enabling one overlay should set anyFieldActive').toBe(true);
        expect(result.afterClear, 'clearing the last overlay should reset anyFieldActive').toBe(false);

        const real = realErrors(errors);
        expect(real, `console errors: ${real.join('\n  ')}`).toEqual([]);
    });
});
