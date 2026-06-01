// @ts-check
/**
 * Reconciliation spec (ticket W0-2).
 *
 * Several engine items were "fixed" during the 2026-04/05 refactor + audit
 * sweeps. A claim that something is fixed is only worth as much as a test that
 * re-asserts it at HEAD, so a future regression is *caught* rather than
 * silently re-introduced. This suite reconciles four such prior fixes against
 * the LIVE dashboard (real Chromium, real bridge, real scale controllers).
 *
 * Each assertion is its own independent `test(...)` so a single failure points
 * at one specific contract. All four boot via `gotoAndReady(page)` and reach
 * into the running app through the same live-bridge / live-ctx introspection
 * hooks the existing specs use (`window._ftdBridge` per scales.spec.js,
 * `getScale0State()` import per audit-regression.spec.js, `window.__ftdCtx`
 * per the scale-0 controller, `window.__ftdRAF` per the rAF coordinator).
 *
 * Robustness to GPU/timing noise: behavioural checks poll with `expect.poll`
 * and allow a few frames; console assertions go through the shared
 * `realErrors()` / `KNOWN_NOISE` filters.
 *
 * Source hooks each test pins (file:line at authoring time, 2026-06-01):
 *   1. capabilities getter — engine/web/js/bridge-init.js:42-44 installs it on
 *      MockBridge / WasmBridge / WebSocketBridge prototypes; getter shape
 *      {scale0,scale1,scale2} in engine/web/js/bridge/capabilities/install.js:22-36.
 *   2. inspector setBridge — engine/web/js/app.js:186 exposes ctx.inspectorRuntime;
 *      engine/web/js/inspector/app-runtime.js:41-50 defines setBridge;
 *      window.__ftdCtx published in engine/web/js/scales/scale0/controller.js:206.
 *   3. scale-4 rAF — engine/web/js/scales/scale4/controller.js:129 subscribes
 *      'scale4-planetary-loop' to rafCoordinator (window.__ftdRAF, raf-coordinator.js:157);
 *      no legacy window._planetaryInterval anywhere; tick from
 *      PlanetaryMockBridge.getDiagnostics().tick (bridge/mock-scale4.js:284).
 *   4. overlay preempt — toggling #toggle-e-field → setFieldToggle('showEField')
 *      sets state.fieldNeedsUpdate=true (scales/scale0/state/store.js:105-122);
 *      the scheduler preempts the in-flight sweep and consumes the dirty
 *      (scales/scale0/runtime/field-overlays.js:926-929 + :962-985).
 */

import { test, expect } from '@playwright/test';
import {
    gotoAndReady,
    switchMode,
    attachConsoleWatcher,
    realErrors,
} from './_helpers.js';

test.beforeEach(async ({ page }) => {
    // WASM compile + Three.js + module graph need headroom on slower machines.
    page.setDefaultTimeout(20_000);
});

test.describe('reconcile prior fixes', () => {

    // ────────────────────────────────────────────────────────────────────
    // 1. Capabilities getter is installed on ALL THREE bridge facades.
    //
    // bridge-init.js applies installCapabilityGetter to MockBridge,
    // WasmBridge AND WebSocketBridge prototypes (audit P0-4: WS previously
    // lacked it, so `bridge.capabilities.scale0` threw on the native-GPU
    // path). We reconcile this two ways:
    //   (a) STRUCTURAL — import the three classes and assert the
    //       `capabilities` accessor is reachable on each prototype chain
    //       (full coverage incl. WebSocketBridge, which is not the active
    //       bridge in a browser-only test).
    //   (b) LIVE — assert the *active* bridge's `.capabilities` is a
    //       non-null object exposing scale0/scale1/scale2 (the shape from
    //       install.js).
    // ────────────────────────────────────────────────────────────────────
    test('capabilities getter present on Mock/Wasm/WebSocket bridge prototypes + live bridge shape', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        const result = await page.evaluate(async () => {
            // (a) Structural: the barrel installs the getter on all three
            // prototypes at module load. Walk the prototype chain so a
            // subclass that inherits the accessor still counts.
            const mod = await import('./js/bridge-init.js');
            const hasCapabilitiesAccessor = (Ctor) => {
                if (typeof Ctor !== 'function' || !Ctor.prototype) return false;
                let proto = Ctor.prototype;
                while (proto && proto !== Object.prototype) {
                    const d = Object.getOwnPropertyDescriptor(proto, 'capabilities');
                    if (d && typeof d.get === 'function') return true;
                    proto = Object.getPrototypeOf(proto);
                }
                return false;
            };
            const structural = {
                mock: hasCapabilitiesAccessor(mod.MockBridge),
                wasm: hasCapabilitiesAccessor(mod.WasmBridge),
                ws: hasCapabilitiesAccessor(mod.WebSocketBridge),
            };

            // (b) Live: the active bridge (Wasm or Mock in a browser run)
            // must expose the symmetric scale0/1/2 surface.
            const b = window._ftdBridge;
            const caps = b ? b.capabilities : null;
            const live = {
                bridgeCtor: (b && b.constructor && b.constructor.name) || null,
                capsIsObject: !!caps && typeof caps === 'object',
                hasScale0: !!(caps && caps.scale0),
                hasScale1: !!(caps && caps.scale1),
                hasScale2: !!(caps && caps.scale2),
            };
            return { structural, live };
        });

        // Structural: all three facades carry the accessor.
        expect(result.structural.mock, 'MockBridge.prototype.capabilities getter').toBe(true);
        expect(result.structural.wasm, 'WasmBridge.prototype.capabilities getter').toBe(true);
        expect(result.structural.ws, 'WebSocketBridge.prototype.capabilities getter (audit P0-4)').toBe(true);

        // Live: active bridge exposes a real {scale0,scale1,scale2} object.
        expect(result.live.capsIsObject, `live bridge (${result.live.bridgeCtor}) .capabilities is an object`).toBe(true);
        expect(result.live.hasScale0).toBe(true);
        expect(result.live.hasScale1).toBe(true);
        expect(result.live.hasScale2).toBe(true);

        expect(realErrors(errors), `console errors: ${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    // ────────────────────────────────────────────────────────────────────
    // 2. Inspector setBridge wiring is callable.
    //
    // Scales that own their own bridge (Scale 4 planetary, Scale 5 cosmic)
    // re-point the inspector via ctx.inspectorRuntime.setBridge(...) on
    // every scale switch (audit P1-1). If that handle is not a function the
    // inspector silently queries a stale backend. Reconcile that
    // inspectorRuntime.setBridge (and the underlying inspector.setBridge)
    // are functions.
    // ────────────────────────────────────────────────────────────────────
    test('ctx.inspectorRuntime.setBridge (and inspector.setBridge) are callable', async ({ page }) => {
        await gotoAndReady(page);

        // window.__ftdCtx is published by the Scale-0 controller on enter();
        // poll because that runs slightly after the bridge is ready.
        await expect.poll(
            () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.inspectorRuntime)),
            { timeout: 15_000, message: 'window.__ftdCtx.inspectorRuntime never became available' },
        ).toBe(true);

        const wiring = await page.evaluate(() => {
            const ctx = window.__ftdCtx;
            const runtime = ctx?.inspectorRuntime || null;
            const inspector = runtime?.inspector || ctx?.inspector || null;
            return {
                runtimeSetBridgeIsFn: typeof runtime?.setBridge === 'function',
                inspectorSetBridgeIsFn: typeof inspector?.setBridge === 'function',
                inspectorPresent: !!inspector,
            };
        });

        expect(wiring.runtimeSetBridgeIsFn, 'inspectorRuntime.setBridge must be a function (scale-switch re-pointing)').toBe(true);
        expect(wiring.inspectorPresent, 'a live inspector instance must exist').toBe(true);
        expect(wiring.inspectorSetBridgeIsFn, 'inspector.setBridge must be a function (runtime delegates to it)').toBe(true);
    });

    // ────────────────────────────────────────────────────────────────────
    // 3. Scale-4 (planetary) runs on the rAF coordinator, not a stale
    //    setInterval.
    //
    // F-6 moved the planetary loop from setInterval(…, 16) to a shared
    // rafCoordinator subscription ('scale4-planetary-loop'). Reconcile that:
    //   (a) no leaked window._planetaryInterval / _planetaryIntervalId
    //       global exists (the old timer handle is gone),
    //   (b) the rAF coordinator has at least one live subscriber while in
    //       planetary mode (the loop is actually wired to rAF), and
    //   (c) the loop ADVANCES — the planetary bridge tick increases once the
    //       sim is running — proving the subscription's callback is firing.
    //
    // The planetary bridge is reachable as ctx.inspector.bridge
    // (Scale4 controller calls inspector.setPlanetaryContext(bridge,...)).
    // The monotonic step counter is PlanetaryMockBridge.getDiagnostics().tick
    // (bridge/mock-scale4.js:284-289) — NOT getPlanetaryData(), which returns
    // only { count, buffer } (the render positions), with no tick field.
    // ────────────────────────────────────────────────────────────────────
    test('planetary loop uses rAF (no setInterval global) and advances', async ({ page }) => {
        await gotoAndReady(page);

        const rafBefore = await page.evaluate(() => window.__ftdRAF?.size?.() ?? 0);

        await switchMode(page, 'planetary');
        // Let loadScenario() build the bridge + subscribe the loop.
        await expect.poll(
            () => page.evaluate(() => {
                const b = window.__ftdCtx?.inspector?.bridge;
                return typeof b?.getDiagnostics === 'function';
            }),
            { timeout: 15_000, message: 'planetary bridge never became reachable via inspector' },
        ).toBe(true);

        // (a) The legacy interval handle must NOT exist on window.
        const noStaleInterval = await page.evaluate(() => {
            return typeof window._planetaryInterval === 'undefined'
                && typeof window._planetaryIntervalId === 'undefined';
        });
        expect(noStaleInterval, 'window._planetaryInterval / _planetaryIntervalId leaked — F-6 regression').toBe(true);

        // (b) The rAF coordinator must have gained a subscriber for planetary.
        const rafAfter = await page.evaluate(() => window.__ftdRAF?.size?.() ?? 0);
        expect(rafAfter, `rAF coordinator should have ≥1 subscriber in planetary (before=${rafBefore}, after=${rafAfter})`).toBeGreaterThan(0);

        // (c) Drive the sim and confirm the loop advances the tick. The loop
        // body only integrates while ctx.running is true and engineMode is
        // 'planetary' — set running via the ctx setter (same path the play
        // button uses). Poll for a tick increase; rAF cadence + the tiny
        // decorative dt mean it may take a handful of frames.
        const tick0 = await page.evaluate(() => {
            window.__ftdCtx.running = true;
            return window.__ftdCtx.inspector.bridge.getDiagnostics().tick;
        });

        await expect.poll(
            () => page.evaluate(() => window.__ftdCtx?.inspector?.bridge?.getDiagnostics?.().tick ?? -1),
            { timeout: 10_000, message: `planetary tick never advanced past ${tick0}` },
        ).toBeGreaterThan(tick0);

        // Stop the sim so we leave the page in a benign state for serial runs.
        await page.evaluate(() => { window.__ftdCtx.running = false; });
    });

    // ────────────────────────────────────────────────────────────────────
    // 4. Overlay preempt-on-toggle: flipping a field overlay dirties the
    //    scheduler and the dirty is consumed within a couple of frames (no
    //    stale frame).
    //
    // The amortized overlay scheduler (field-overlays.js) preempts any
    // in-flight sweep when state.fieldNeedsUpdate is set, and the trigger
    // gate honours fieldNeedsUpdate even under global pause — so a toggle is
    // reflected on the very next overlay-update frame instead of waiting for
    // a throttle boundary against a stale snapshot.
    //
    // Introspection surface (differs slightly from the ticket's suggested
    // hook — there is no window field-flag store; the canonical store is the
    // importable getScale0State(), exactly as audit-regression.spec.js uses):
    //   - state.fieldFlags.showEField  ← the E-field overlay flag
    //   - state.fieldNeedsUpdate       ← the one-shot preempt/dirty flag
    // Clicking #toggle-e-field runs setFieldToggle('showEField', …) which
    // both flips the flag and sets fieldNeedsUpdate=true. We assert:
    //   (i)  the click flips the flag AND raises fieldNeedsUpdate (preempt
    //        signalled), then
    //   (ii) after letting the Scale-0 animate loop run a few frames the
    //        dirty is CONSUMED (fieldNeedsUpdate back to false) — i.e. the
    //        scheduler actually processed the preempt rather than stranding
    //        a stale frame.
    // ────────────────────────────────────────────────────────────────────
    test('toggling #toggle-e-field dirties the overlay scheduler and the preempt is consumed', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        // Ensure we are on Scale 0 (default) and the controller is mounted.
        await expect.poll(
            () => page.evaluate(() => typeof window.__ftdCtx !== 'undefined'),
            { timeout: 15_000, message: 'window.__ftdCtx (Scale-0 ctx) never became available' },
        ).toBe(true);

        // Make sure the E-field toggle button exists and start from a known
        // (off) state so the click is a genuine off→on transition.
        const toggleReady = await page.evaluate(async () => {
            const { getScale0State, setFieldToggle } = await import('./js/scales/scale0/state/store.js');
            const btn = document.getElementById('toggle-e-field');
            // Normalise to OFF without going through the button (no dirty side
            // effect we care about yet); the click below is the event under test.
            setFieldToggle('showEField', false);
            const st = getScale0State();
            st.fieldNeedsUpdate = false; // clear any residual dirty from boot
            return {
                hasButton: !!btn,
                startFlag: !!st.fieldFlags.showEField,
                startDirty: !!st.fieldNeedsUpdate,
            };
        });
        expect(toggleReady.hasButton, '#toggle-e-field button must exist on Scale 0').toBe(true);
        expect(toggleReady.startFlag).toBe(false);
        expect(toggleReady.startDirty).toBe(false);

        // (i) Click the real button via direct DOM dispatch — the idiom every
        // passing spec uses (faq/panel-mount/math-formatting). A Playwright
        // page.click() here times out: the panel-scale-header overlaps the
        // toolbar button in the headless layout and "intercepts pointer events".
        // Read the dirty flag SYNCHRONOUSLY in the same evaluate: setFieldToggle
        // raises fieldNeedsUpdate, and the Scale-0 rAF loop consumes it within a
        // single frame — a second async round-trip would race the consume and
        // miss the raised signal. No rAF callback can fire mid-synchronous-block.
        const afterClick = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            const btn = document.getElementById('toggle-e-field');
            btn.click();                  // off→on; setFieldToggle sets dirty
            const st = getScale0State();  // read before any rAF frame runs
            return {
                flag: !!st.fieldFlags.showEField,
                dirty: !!st.fieldNeedsUpdate,
                anyActive: !!st.anyFieldActive,
            };
        });
        expect(afterClick.flag, 'clicking #toggle-e-field should enable showEField').toBe(true);
        expect(afterClick.anyActive, 'enabling an overlay should mark anyFieldActive').toBe(true);
        expect(afterClick.dirty, 'enabling an overlay must set fieldNeedsUpdate (scheduler preempt signal)').toBe(true);

        // (ii) Let the Scale-0 animate loop run a few frames. updateFieldOverlays
        // honours fieldNeedsUpdate immediately (even under pause) and clears it
        // when it opens the fresh sweep — so a live scheduler consumes the dirty
        // within a couple of frames. Poll rather than fixed-wait to stay robust
        // to frame-rate noise.
        await expect.poll(
            () => page.evaluate(async () => {
                const { getScale0State } = await import('./js/scales/scale0/state/store.js');
                return !!getScale0State().fieldNeedsUpdate;
            }),
            { timeout: 8_000, message: 'fieldNeedsUpdate was never consumed — overlay preempt path is not live (stale-frame regression)' },
        ).toBe(false);

        // The overlay flag itself must remain ON after the preempt (we toggled
        // it on; consuming the dirty must not silently flip it back).
        const flagStillOn = await page.evaluate(async () => {
            const { getScale0State } = await import('./js/scales/scale0/state/store.js');
            return !!getScale0State().fieldFlags.showEField;
        });
        expect(flagStillOn, 'showEField should stay enabled after the preempt is consumed').toBe(true);

        expect(realErrors(errors), `console errors: ${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
