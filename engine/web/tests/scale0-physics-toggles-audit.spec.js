// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

test.describe('Scale 0 physics-toggles controls-card audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(async () => {
            const { getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const worker = window.__ftdCtx?.fluxMock;
            const lifecycle = worker?.lifecycleDebug;
            return document.getElementById('app')?.dataset.shellReady === 'true'
                && worker?.ready === true
                && worker?.hasEngineToggles === true
                && !!lifecycle?.workerRuntimeId
                && lifecycle.appliedConfigurationToken === lifecycle.configurationToken
                && getScale0QualificationState().status === 'within-contract'
                && document.getElementById('physics-profile-warning')
                    ?.closest('.card')?.getAttribute('aria-busy') === 'false';
        });
    });

    test('every visible configured term has one control and matches engine truth', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const initial = await page.evaluate(async () => {
            const { SCALE0_TOGGLES, SCALE0_ADVANCED_TOGGLES } =
                await import('/js/config/toggles.js');
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            const all = [...SCALE0_TOGGLES, ...SCALE0_ADVANCED_TOGGLES];
            return {
                missing: all.filter(([, , id]) => !document.getElementById(id))
                    .map(([, , id]) => id),
                duplicates: all.filter(([, , id]) => document.querySelectorAll(`#${id}`).length > 1)
                    .map(([, , id]) => id),
                parity: all.flatMap(([key, , id]) => {
                    const input = document.getElementById(id);
                    return input instanceof HTMLInputElement
                        ? [[key, owner.getToggle(key), input.checked]]
                        : [];
                }),
                cardCount: document.querySelectorAll('#physics-profile-warning').length,
                checkboxCount: document.querySelectorAll(
                    '#physics-profile-warning ~ .combo-section-label, '
                    + '#physics-profile-warning ~ .toggle-row input[type="checkbox"], '
                    + '#physics-profile-warning ~ details input[type="checkbox"]',
                ).length,
                evaporationLabel: document.querySelector('label[for="t-evaporation"]')?.textContent,
                genesisLabel: document.querySelector('label[for="t-genesis"]')?.textContent,
            };
        });

        // de_broglie_clock is scenario-owned and intentionally has no dashboard input.
        expect(initial.missing).toEqual(['t-de-broglie']);
        expect(initial.duplicates).toEqual([]);
        expect(initial.cardCount).toBe(1);
        expect(initial.parity.every(([, engine, ui]) => engine === ui)).toBe(true);
        expect(initial.evaporationLabel).toBe('Evaporation');
        expect(initial.genesisLabel).toBe('Genesis');

        await selectScale0Scenario(page, 'flux-pair-production');
        await page.waitForFunction(() => {
            return window.__ftdCtx?.fluxMock?.hasEngineToggles === true
                && document.getElementById('physics-profile-warning')
                    ?.closest('.card')?.getAttribute('aria-busy') === 'false';
        });
        const pair = await page.evaluate(async () => {
            const { SCALE0_TOGGLES, SCALE0_ADVANCED_TOGGLES } =
                await import('/js/config/toggles.js');
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            const parity = [...SCALE0_TOGGLES, ...SCALE0_ADVANCED_TOGGLES]
                .flatMap(([key, , id]) => {
                    const input = document.getElementById(id);
                    return input instanceof HTMLInputElement
                        ? [[key, owner.getToggle(key), input.checked]]
                        : [];
                });
            return {
                parity,
                enginePair: owner.getToggle('pair_production'),
                uiPair: document.getElementById('t-pair-production')?.checked,
            };
        });
        expect(pair.parity.every(([, engine, ui]) => engine === ui)).toBe(true);
        expect(pair.enginePair).toBe(true);
        expect(pair.uiPair).toBe(true);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('twenty standard and research toggle cycles dispatch only to the active owner', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const { rafCoordinator } = await import('/js/lib/raf-coordinator.js');
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const main = ctx.bridge;
            const owner = state.fluxMock;
            const mainOriginal = main.setToggle;
            const ownerOriginal = owner.setToggle;
            const mainCalls = [];
            const ownerCalls = [];
            const card = document.getElementById('physics-profile-warning').closest('.card');
            const before = {
                nodes: card.querySelectorAll('*').length,
                inputs: card.querySelectorAll('input[type="checkbox"]').length,
                subscribers: rafCoordinator.size(),
            };
            const activeIsWorker = (await import('/js/scales/scale0/state/store.js'))
                .getActiveScale0Bridge(ctx, state) === owner;
            main.setToggle = function (key, value) {
                mainCalls.push([key, value]);
                return mainOriginal.call(this, key, value);
            };
            owner.setToggle = function (key, value) {
                ownerCalls.push([key, value]);
                return ownerOriginal.call(this, key, value);
            };
            try {
                const gauss = document.getElementById('t-gauss');
                const knot = document.getElementById('t-knot-tracking');
                for (let i = 0; i < 10; i++) {
                    gauss.click();
                    gauss.click();
                    knot.click();
                    knot.click();
                }
                await new Promise((resolve) => setTimeout(resolve, 250));
                return {
                    before,
                    activeIsWorker,
                    mainCalls,
                    ownerCalls,
                    final: { gauss: gauss.checked, knot: knot.checked },
                    warningHidden: document.getElementById('physics-profile-warning').hidden,
                    after: {
                        nodes: card.querySelectorAll('*').length,
                        inputs: card.querySelectorAll('input[type="checkbox"]').length,
                        subscribers: rafCoordinator.size(),
                    },
                };
            } finally {
                main.setToggle = mainOriginal;
                owner.setToggle = ownerOriginal;
            }
        });

        expect(result.mainCalls).toEqual([]);
        expect(result.activeIsWorker).toBe(true);
        expect(result.ownerCalls).toHaveLength(40);
        expect(result.ownerCalls.filter(([key]) => key === 'gauss_projection')).toHaveLength(20);
        expect(result.ownerCalls.filter(([key]) => key === 'knot_tracking')).toHaveLength(20);
        expect(result.final).toEqual({ gauss: false, knot: false });
        expect(result.warningHidden).toBe(true);
        expect(result.after).toEqual(result.before);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('scenario handoff blocks edits until readback and restores research terms', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const immediate = await page.evaluate(() => {
            const select = document.getElementById('scenario-select');
            const pair = document.getElementById('t-pair-production');
            const card = document.getElementById('physics-profile-warning').closest('.card');
            select.value = 'flux-pair-production';
            select.dispatchEvent(new Event('change', { bubbles: true }));
            const pending = {
                busy: card.getAttribute('aria-busy'),
                pairDisabled: pair.disabled,
                pairChecked: pair.checked,
            };
            pair.click();
            return { pending, afterBlockedClick: pair.checked };
        });
        expect(immediate.pending.busy).toBe('true');
        expect(immediate.pending.pairDisabled).toBe(true);
        expect(immediate.afterBlockedClick).toBe(immediate.pending.pairChecked);

        await page.waitForFunction(() => {
            return window.__ftdCtx?.fluxMock?.hasEngineToggles === true
                && document.getElementById('physics-profile-warning')
                    ?.closest('.card')?.getAttribute('aria-busy') === 'false';
        });
        const loaded = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            return {
                scenario: state.currentScenarioId,
                enginePair: state.fluxMock.getToggle('pair_production'),
                uiPair: document.getElementById('t-pair-production').checked,
                pairDisabled: document.getElementById('t-pair-production').disabled,
                pendingMarker: document.getElementById('t-pair-production')
                    .dataset.scale0PendingDisabled ?? null,
                profileValue: document.getElementById('t-pair-production')
                    .dataset.scale0ProfileValue ?? null,
                busy: document.getElementById('physics-profile-warning')
                    .closest('.card').getAttribute('aria-busy'),
                warningHidden: document.getElementById('physics-profile-warning').hidden,
            };
        });
        expect(loaded).toEqual({
            scenario: 'flux-pair-production',
            enginePair: true,
            uiPair: true,
            pairDisabled: false,
            pendingMarker: null,
            profileValue: '1',
            busy: 'false',
            warningHidden: true,
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('ten immediate restore clicks collapse to one scenario reload', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await selectScale0Scenario(page, 'flux-pair-production');
        await page.waitForFunction(() => {
            return window.__ftdCtx?.fluxMock?.hasEngineToggles === true;
        });
        const before = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const pair = document.getElementById('t-pair-production');
            pair.click();
            return {
                generation: window.__ftdCtx._loadGeneration,
                workers: window.__ftdWasmWorkers(),
                lifecycle: getScale0State().fluxMock?.lifecycleDebug ?? null,
                warningVisible: !document.getElementById('physics-profile-warning').hidden,
            };
        });
        expect(before.warningVisible).toBe(true);

        const immediate = await page.evaluate(() => {
            const button = document.getElementById('btn-reset-physics-toggles');
            for (let i = 0; i < 10; i++) button.click();
            return { disabled: button.disabled, generation: window.__ftdCtx._loadGeneration };
        });
        expect(immediate.disabled).toBe(true);
        expect(immediate.generation - before.generation).toBe(1);

        await page.waitForFunction(async () => {
            const { getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const owner = window.__ftdCtx?.fluxMock;
            const lifecycle = owner?.lifecycleDebug;
            const qualification = getScale0QualificationState();
            return owner?.ready === true
                && owner?.hasEngineToggles === true
                && lifecycle?.appliedConfigurationToken === lifecycle?.configurationToken
                && qualification.status === 'within-contract'
                && qualification.anchor?.scenarioId === 'flux-pair-production'
                && qualification.anchor?.loadGeneration === window.__ftdCtx?._loadGeneration
                && document.getElementById('btn-reset-physics-toggles')?.disabled === false;
        });
        const after = await page.evaluate(async () => {
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            return {
                generation: window.__ftdCtx._loadGeneration,
                workers: window.__ftdWasmWorkers(),
                lifecycle: state.fluxMock?.lifecycleDebug ?? null,
                qualification: getScale0QualificationState(),
                enginePair: state.fluxMock.getToggle('pair_production'),
                uiPair: document.getElementById('t-pair-production').checked,
                warningHidden: document.getElementById('physics-profile-warning').hidden,
            };
        });
        expect(after.generation - before.generation).toBe(1);
        expect(after.workers.created - before.workers.created).toBe(0);
        expect(after.workers.terminated - before.workers.terminated).toBe(0);
        expect(after.workers.live).toBe(1);
        expect(after.lifecycle.workerRuntimeId).toBe(before.lifecycle.workerRuntimeId);
        expect(after.lifecycle.moduleInitCount).toBe(1);
        expect(after.lifecycle.renderBridgeGeneration
            - before.lifecycle.renderBridgeGeneration).toBe(1);
        expect(after.lifecycle.appliedConfigurationToken).toBe(after.lifecycle.configurationToken);
        expect(after.qualification.status).toBe('within-contract');
        expect(after.qualification.anchor.scenarioId).toBe('flux-pair-production');
        expect(after.qualification.anchor.loadGeneration).toBe(after.generation);
        expect(after.qualification.anchor.source).toBe('worker-configuration-applied');
        expect(after.enginePair).toBe(true);
        expect(after.uiPair).toBe(true);
        expect(after.warningHidden).toBe(true);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
