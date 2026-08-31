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

    test('enable all applies one compatible 24-term engine profile', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const button = page.getByRole('button', { name: 'Enable all physics' });
        await expect(button).toHaveCount(1);
        await expect(button).toBeEnabled();

        const prepared = await page.evaluate(async () => {
            const { getActiveScale0Bridge, getScale0State } =
                await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const owner = getActiveScale0Bridge(ctx, state);
            const idle = ctx.bridge;
            const audit = {
                ownerBatchCalls: [],
                ownerSingleCalls: [],
                idleBatchCalls: [],
                idleSingleCalls: [],
                owner,
                idle,
                ownerSetToggles: owner.setToggles,
                ownerSetToggle: owner.setToggle,
                idleSetToggles: idle?.setToggles,
                idleSetToggle: idle?.setToggle,
            };
            owner.setToggles = function (entries) {
                audit.ownerBatchCalls.push(entries.map(([key, value]) => [key, value]));
                return audit.ownerSetToggles.call(this, entries);
            };
            owner.setToggle = function (key, value) {
                audit.ownerSingleCalls.push([key, value]);
                return audit.ownerSetToggle.call(this, key, value);
            };
            if (idle && idle !== owner) {
                idle.setToggles = function (entries) {
                    audit.idleBatchCalls.push(entries.map(([key, value]) => [key, value]));
                    return audit.idleSetToggles.call(this, entries);
                };
                idle.setToggle = function (key, value) {
                    audit.idleSingleCalls.push([key, value]);
                    return audit.idleSetToggle.call(this, key, value);
                };
            }
            window.__enableAllPhysicsAudit = audit;
            return { activeIsWorker: owner === state.fluxMock, idleIsDistinct: idle !== owner };
        });

        let result;
        try {
            // Ten same-turn activations must collapse to the first accepted action.
            const immediate = await page.evaluate(() => {
                const action = document.getElementById('btn-enable-all-physics');
                for (let i = 0; i < 10; i++) action.click();
                return { disabled: action.disabled };
            });
            expect(immediate.disabled).toBe(true);

            await page.waitForFunction(async () => {
                const {
                    SCALE0_ENABLE_ALL_PHYSICS_KEYS,
                    SCALE0_ENABLE_ALL_PHYSICS_EXCLUDED_KEYS,
                } = await import('/js/config/toggles.js');
                const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                const owner = getScale0State().fluxMock;
                return document.getElementById('btn-enable-all-physics')?.disabled === false
                    && SCALE0_ENABLE_ALL_PHYSICS_KEYS.every((key) => owner.getToggle(key) === true)
                    && SCALE0_ENABLE_ALL_PHYSICS_EXCLUDED_KEYS
                        .every((key) => owner.getToggle(key) === false);
            });

            result = await page.evaluate(async () => {
                const {
                    SCALE0_TOGGLES,
                    SCALE0_ADVANCED_TOGGLES,
                    SCALE0_ENABLE_ALL_PHYSICS_KEYS,
                    SCALE0_ENABLE_ALL_PHYSICS_EXCLUDED_KEYS,
                } = await import('/js/config/toggles.js');
                const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                const audit = window.__enableAllPhysicsAudit;
                const owner = getScale0State().fluxMock;
                const ids = new Map(
                    [...SCALE0_TOGGLES, ...SCALE0_ADVANCED_TOGGLES]
                        .map(([key, , id]) => [key, id]),
                );
                const expectedBatch = [
                    ...SCALE0_ENABLE_ALL_PHYSICS_EXCLUDED_KEYS.map((key) => [key, false]),
                    ...SCALE0_ENABLE_ALL_PHYSICS_KEYS.map((key) => [key, true]),
                ];
                const requirements = {
                    lorentz_force: 'forces',
                    selective_damping: 'damping',
                    larmor_radiation: 'damping',
                    weak_transmutation: 'dual_substrate',
                    triad_binding: 'color_forces',
                    exchange_force: 'poisson_coulomb',
                    latency_field: 'gravity',
                    symmetric_movement_order: 'movement',
                    confinement: 'color_forces',
                };
                return {
                    expectedBatch,
                    ownerBatchCalls: audit.ownerBatchCalls,
                    ownerSingleCalls: audit.ownerSingleCalls,
                    idleBatchCalls: audit.idleBatchCalls,
                    idleSingleCalls: audit.idleSingleCalls,
                    enabledCount: SCALE0_ENABLE_ALL_PHYSICS_KEYS.length,
                    excludedCount: SCALE0_ENABLE_ALL_PHYSICS_EXCLUDED_KEYS.length,
                    engineEnabled: SCALE0_ENABLE_ALL_PHYSICS_KEYS
                        .map((key) => [key, owner.getToggle(key)]),
                    engineExcluded: SCALE0_ENABLE_ALL_PHYSICS_EXCLUDED_KEYS
                        .map((key) => [key, owner.getToggle(key)]),
                    uiParity: expectedBatch.flatMap(([key, expected]) => {
                        const input = document.getElementById(ids.get(key));
                        return input instanceof HTMLInputElement
                            ? [[key, expected, input.checked]]
                            : [];
                    }),
                    requirementsSatisfied: Object.entries(requirements)
                        .every(([term, prerequisite]) => (
                            !owner.getToggle(term) || owner.getToggle(prerequisite)
                        )),
                    warning: document.getElementById('physics-profile-warning')?.textContent,
                    warningHidden: document.getElementById('physics-profile-warning')?.hidden,
                    buttonText: document.getElementById('btn-enable-all-physics')?.textContent
                        ?.replace(/\s+/g, ' ').trim(),
                };
            });
        } finally {
            await page.evaluate(() => {
                const audit = window.__enableAllPhysicsAudit;
                if (!audit) return;
                audit.owner.setToggles = audit.ownerSetToggles;
                audit.owner.setToggle = audit.ownerSetToggle;
                if (audit.idle && audit.idle !== audit.owner) {
                    audit.idle.setToggles = audit.idleSetToggles;
                    audit.idle.setToggle = audit.idleSetToggle;
                }
                delete window.__enableAllPhysicsAudit;
            });
        }

        expect(prepared.activeIsWorker).toBe(true);
        expect(prepared.idleIsDistinct).toBe(true);
        expect(result.enabledCount).toBe(24);
        expect(result.excludedCount).toBe(4);
        expect(result.ownerBatchCalls).toEqual([result.expectedBatch]);
        expect(result.ownerSingleCalls).toEqual([]);
        expect(result.idleBatchCalls).toEqual([]);
        expect(result.idleSingleCalls).toEqual([]);
        expect(result.engineEnabled.every(([, value]) => value === true)).toBe(true);
        expect(result.engineExcluded.every(([, value]) => value === false)).toBe(true);
        expect(result.uiParity.every(([, expected, actual]) => expected === actual)).toBe(true);
        expect(result.requirementsSatisfied).toBe(true);
        expect(result.warningHidden).toBe(false);
        expect(result.warning).toContain('modified');
        expect(result.buttonText).toContain('Enable all physics');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('runtime dual-substrate transitions preserve the live field', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const dual = page.getByRole('checkbox', { name: 'Dual Substrate' });
        const step = page.getByRole('button', { name: 'Step', exact: true });
        const readState = () => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            return {
                engine: owner.getToggle('dual_substrate'),
                ui: document.getElementById('t-dual')?.checked,
                magnitude: Array.from(owner.getFluxVolume())
                    .reduce((sum, component) => sum + Math.abs(component), 0),
                tick: owner.currentTick(),
            };
        });

        if (await dual.isChecked()) await dual.click();
        await expect(dual).not.toBeChecked();
        await page.waitForFunction(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.getToggle('dual_substrate') === false;
        });
        // Seed through the production worker API so this test never depends on
        // when the first scenario volume frame happens to arrive.
        await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            const midpoint = Math.floor(owner.N / 2);
            owner.injectFlux(midpoint, midpoint, midpoint, 3, 0, 0);
        });
        await expect.poll(async () => (await readState()).magnitude).toBeGreaterThan(0);
        const before = await readState();

        await dual.click();
        await expect(dual).toBeChecked();
        await page.waitForFunction(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.getToggle('dual_substrate') === true;
        });
        const enabled = await readState();
        await step.click();
        await page.waitForFunction(async (priorTick) => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.currentTick() > priorTick;
        }, enabled.tick);
        const afterEnable = await readState();

        await dual.click();
        await expect(dual).not.toBeChecked();
        await page.waitForFunction(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.getToggle('dual_substrate') === false;
        });
        const disabled = await readState();
        await step.click();
        await page.waitForFunction(async (priorTick) => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.currentTick() > priorTick;
        }, disabled.tick);
        const afterDisable = await readState();

        const evidence = JSON.stringify({ before, enabled, afterEnable, disabled, afterDisable });
        expect(before.magnitude, evidence).toBeGreaterThan(0);
        expect(afterEnable.magnitude).toBeGreaterThan(before.magnitude * 1e-6);
        expect(afterDisable.magnitude).toBeGreaterThan(before.magnitude * 1e-6);
        expect(enabled).toMatchObject({ engine: true, ui: true });
        expect(disabled).toMatchObject({ engine: false, ui: false });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('twenty standard and research toggle cycles dispatch only to the active owner', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        // Pin a fresh, explicit scenario generation. The worker can legitimately
        // re-enter authoritative-readback pending after shell readiness; clicks
        // during that interval are disabled and must not dispatch.
        const priorGeneration = await page.evaluate(() => window.__ftdCtx?._loadGeneration || 0);
        await selectScale0Scenario(page, 'flux-pair-production', { settleMs: 0 });
        await page.evaluate(async ({ generation, scenarioId }) => {
            const { getScale0QualificationState, getScale0State } =
                await import('/js/scales/scale0/state/store.js');
            const deadline = performance.now() + 30_000;
            let stableSince = 0;
            let stableGeneration = -1;
            let lastSnapshot = null;
            while (performance.now() < deadline) {
                const worker = window.__ftdCtx?.fluxMock;
                const lifecycle = worker?.lifecycleDebug;
                const currentGeneration = window.__ftdCtx?._loadGeneration || 0;
                const qualification = getScale0QualificationState();
                const busy = document.getElementById('physics-profile-warning')
                    ?.closest('.card')?.getAttribute('aria-busy');
                lastSnapshot = {
                    currentGeneration,
                    currentScenarioId: getScale0State().currentScenarioId,
                    workerReady: worker?.ready,
                    hasEngineToggles: worker?.hasEngineToggles,
                    configurationToken: lifecycle?.configurationToken,
                    appliedConfigurationToken: lifecycle?.appliedConfigurationToken,
                    qualificationStatus: qualification.status,
                    authoritativeLoad: qualification.authoritativeLoad,
                    busy,
                };
                const ready = currentGeneration > generation
                    && getScale0State().currentScenarioId === scenarioId
                    && worker?.ready === true
                    && worker?.hasEngineToggles === true
                    && lifecycle?.appliedConfigurationToken === lifecycle?.configurationToken
                    && qualification.status === 'within-contract'
                    && busy === 'false';
                if (!ready) {
                    stableGeneration = currentGeneration;
                    stableSince = 0;
                } else if (currentGeneration !== stableGeneration || !stableSince) {
                    stableGeneration = currentGeneration;
                    stableSince = performance.now();
                } else if (stableSince && performance.now() - stableSince >= 1_500) {
                    return;
                }
                await new Promise((resolve) => setTimeout(resolve, 50));
            }
            throw new Error(`Scale-0 physics controls never stabilized: ${JSON.stringify(lastSnapshot)}`);
        }, { generation: priorGeneration, scenarioId: 'flux-pair-production' });
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
                const initial = { gauss: gauss.checked, knot: knot.checked };
                const controlState = {
                    gaussDisabled: gauss.disabled,
                    knotDisabled: knot.disabled,
                    gaussPendingMarker: gauss.dataset.scale0PendingDisabled ?? null,
                    knotPendingMarker: knot.dataset.scale0PendingDisabled ?? null,
                    busy: card.getAttribute('aria-busy'),
                };
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
                    initial,
                    controlState,
                    final: { gauss: gauss.checked, knot: knot.checked },
                    warningHidden: document.getElementById('physics-profile-warning').hidden,
                    warningText: document.getElementById('physics-profile-warning').textContent,
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
        expect(result.ownerCalls, JSON.stringify(result.controlState)).toHaveLength(40);
        expect(result.ownerCalls.filter(([key]) => key === 'gauss_projection')).toHaveLength(20);
        expect(result.ownerCalls.filter(([key]) => key === 'knot_tracking')).toHaveLength(20);
        expect(result.final).toEqual(result.initial);
        expect(result.warningHidden).toBe(false);
        expect(result.warningText).toContain('modified');
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
