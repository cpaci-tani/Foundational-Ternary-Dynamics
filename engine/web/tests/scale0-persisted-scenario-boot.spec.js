// @ts-check

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const WILSON_SCENARIO = 's0-seed-wilson-loop';

test('restored Scale 0 selection is loaded at boot and replayed after reconnect', async ({ page }) => {
    // Chromium/WebView can restore a select value without emitting `change`.
    // Reproduce that exact timing: the toolbar is created dynamically, so set
    // the value as soon as its options exist and deliberately dispatch no event.
    await page.addInitScript((scenarioId) => {
        window.__ftdWasmWorker = false;
        const restoreSelection = () => {
            const select = document.getElementById('scenario-select');
            if (!select || ![...select.options].some(option => option.value === scenarioId)) {
                return false;
            }
            select.value = scenarioId;
            window.__ftdRestoredScale0Scenario = scenarioId;
            return true;
        };
        const observer = new MutationObserver(() => {
            if (restoreSelection()) observer.disconnect();
        });
        const observe = () => {
            observer.observe(document.documentElement, { childList: true, subtree: true });
            if (restoreSelection()) observer.disconnect();
        };
        if (document.documentElement) observe();
        else document.addEventListener('DOMContentLoaded', observe, { once: true });
    }, WILSON_SCENARIO);

    // Port 1 makes the optional native probe fail immediately and keeps this
    // regression deterministic on the in-thread bridge. The controller path is
    // identical; native reconnect invokes the same connection-ready callback.
    await gotoAndReady(page, { path: '/?wsPort=1', timeout: 60_000 });
    await page.waitForFunction((scenarioId) => (
        window.__ftdRestoredScale0Scenario === scenarioId
        && document.getElementById('scenario-select')?.value === scenarioId
    ), WILSON_SCENARIO);
    await page.waitForFunction(async (scenarioId) => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        return getScale0State().currentScenarioId === scenarioId;
    }, WILSON_SCENARIO);

    const initial = await page.evaluate(async (scenarioId) => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const { getScale0ScenarioToggleProfile } = await import('/js/config/toggles.js');
        const state = getScale0State();
        const activeBridge = state.useFluxMock && state.fluxMock
            ? state.fluxMock
            : window.__ftdCtx.bridge;
        return {
            selected: document.getElementById('scenario-select')?.value,
            current: state.currentScenarioId,
            boundary: document.getElementById('flux-boundary-mode')?.value,
            mismatches: getScale0ScenarioToggleProfile(scenarioId)
                .filter(([name, enabled]) => !!activeBridge.getToggle?.(name) !== !!enabled)
                .map(([name]) => name),
        };
    }, WILSON_SCENARIO);

    expect(initial.selected).toBe(WILSON_SCENARIO);
    expect(initial.current).toBe(WILSON_SCENARIO);
    expect(initial.boundary).toBe('2');
    expect(initial.mismatches).toEqual([]);

    const replay = await page.evaluate(async (scenarioId) => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const { getScale0ScenarioToggleProfile } = await import('/js/config/toggles.js');
        const state = getScale0State();
        const ctx = window.__ftdCtx;
        const activeBridge = state.useFluxMock && state.fluxMock ? state.fluxMock : ctx.bridge;

        // Model a replacement native RenderBridge starting from its default
        // packet profile while the DOM and client-side scenario id still say
        // Wilson. No select `change` event can help in this state.
        activeBridge.setupScenario('flux-pulse');
        const mismatchesBefore = getScale0ScenarioToggleProfile(scenarioId)
            .filter(([name, enabled]) => !!activeBridge.getToggle?.(name) !== !!enabled)
            .map(([name]) => name);

        ctx.onBridgeConnectionReady?.({ generation: 2, info: { latticeSize: activeBridge.latticeSize } });

        const mismatchesAfter = getScale0ScenarioToggleProfile(scenarioId)
            .filter(([name, enabled]) => !!activeBridge.getToggle?.(name) !== !!enabled)
            .map(([name]) => name);
        return {
            selected: document.getElementById('scenario-select')?.value,
            current: getScale0State().currentScenarioId,
            mismatchesBefore,
            mismatchesAfter,
            boundary: document.getElementById('flux-boundary-mode')?.value,
        };
    }, WILSON_SCENARIO);

    expect(replay.selected).toBe(WILSON_SCENARIO);
    expect(replay.current).toBe(WILSON_SCENARIO);
    expect(replay.mismatchesBefore).toContain('wave_propagation');
    expect(replay.mismatchesAfter).toEqual([]);
    expect(replay.boundary).toBe('2');
});
