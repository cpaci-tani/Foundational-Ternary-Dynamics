// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 substrate-controls card audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page, { path: '/?engine=wasm' });
        await page.waitForFunction(() => {
            const stateReady = !!window.__ftdCtx?.fluxMock?.ready;
            return document.getElementById('app')?.dataset.shellReady === 'true'
                && stateReady
                && typeof window.__ftdCtx?.syncScale0InjectionBounds === 'function';
        });
    });

    test('inventory is unique and engine constants are truthful read-only values', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const { K_B, G_N, DAMPING } = await import('/js/constants.js');
            const { getScale0State, getActiveScale0Bridge } = await import(
                '/js/scales/scale0/state/store.js'
            );
            const ids = [
                'btn-inject', 'btn-inject-wave', 'btn-inject-flux', 'btn-inject-pair',
                'inj-x', 'inj-y', 'inj-z', 'inj-state-pos', 'inj-state-neg',
                'btn-center', 'btn-random', 'combo-kb', 'combo-gn', 'combo-damp',
                'btn-clear-field', 'btn-random-flux',
            ];
            const sliderIds = ['combo-kb', 'combo-gn', 'combo-damp'];
            const displayIds = ['combo-kb-val', 'combo-gn-val', 'combo-damp-val'];
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const owner = getActiveScale0Bridge(ctx, state);
            const card = document.getElementById('btn-inject')?.closest('.card');
            return {
                missing: ids.filter((id) => !document.getElementById(id)),
                duplicates: ids.filter((id) => document.querySelectorAll(`#${id}`).length !== 1),
                title: card?.querySelector('.card-title')?.textContent,
                fixedBadges: card?.querySelectorAll('.ctrl-native-fixed').length,
                sliders: sliderIds.map((id) => {
                    const el = /** @type {HTMLInputElement} */ (document.getElementById(id));
                    return {
                        disabled: el.disabled,
                        readOnly: el.getAttribute('aria-readonly'),
                        ariaDisabled: el.getAttribute('aria-disabled'),
                        step: el.step,
                        tooltip: el.dataset.uiTooltip || el.title,
                        value: Number(el.value),
                    };
                }),
                displays: displayIds.map((id) => document.getElementById(id)?.textContent),
                expected: [K_B, G_N, DAMPING],
                backendSetters: {
                    main: typeof ctx.bridge?.setParam,
                    owner: typeof owner?.setParam,
                },
                bounds: ['inj-x', 'inj-y', 'inj-z'].map((id) => ({
                    min: document.getElementById(id)?.getAttribute('min'),
                    max: document.getElementById(id)?.getAttribute('max'),
                })),
            };
        });

        expect(result.missing).toEqual([]);
        expect(result.duplicates).toEqual([]);
        expect(result.title).toBe('Substrate Controls');
        expect(result.fixedBadges).toBe(3);
        expect(result.sliders.every((s) => s.disabled)).toBe(true);
        expect(result.sliders.map((s) => s.readOnly)).toEqual(['true', 'true', 'true']);
        expect(result.sliders.map((s) => s.ariaDisabled)).toEqual(['true', 'true', 'true']);
        expect(result.sliders.map((s) => s.step)).toEqual(['any', 'any', 'any']);
        expect(result.sliders.every((s) => /read-only/.test(s.tooltip))).toBe(true);
        for (let i = 0; i < result.expected.length; i++) {
            expect(result.sliders[i].value).toBeCloseTo(result.expected[i], 14);
        }
        expect(result.displays).toEqual([
            result.expected[0].toFixed(3),
            result.expected[1].toFixed(3),
            result.expected[2].toFixed(4),
        ]);
        expect(result.backendSetters).toEqual({ main: 'undefined', owner: 'undefined' });
        expect(result.bounds).toEqual([
            { min: '0', max: '32' },
            { min: '0', max: '32' },
            { min: '0', max: '32' },
        ]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('all inject and field actions dispatch once to the active owner with bounded coordinates', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const {
                getScale0State,
                getActiveScale0Bridge,
                getScale0QualificationState,
                setLatticeNeedsUpload,
                subscribeScale0Qualification,
            } = await import(
                '/js/scales/scale0/state/store.js'
            );
            const { rafCoordinator } = await import('/js/lib/raf-coordinator.js');
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const main = ctx.bridge;
            const owner = getActiveScale0Bridge(ctx, state);
            const methods = [
                'injectParticle', 'injectWavepacket', 'injectFlux',
                'createEntangledPair', 'clearField', 'seedRandomFlux',
            ];
            const mainCalls = [];
            const ownerCalls = [];
            const originals = { main: {}, owner: {} };
            const card = document.getElementById('btn-inject').closest('.card');
            const before = {
                nodes: card.querySelectorAll('*').length,
                inputs: card.querySelectorAll('input').length,
                subscribers: rafCoordinator.size(),
                resources: performance.getEntriesByType('resource').length,
            };
            for (const method of methods) {
                originals.main[method] = main[method];
                originals.owner[method] = owner[method];
                main[method] = (...args) => mainCalls.push([method, ...args]);
                owner[method] = (...args) => ownerCalls.push([method, ...args]);
            }
            const originalClearCharts = ctx.clearCharts;
            const originalViewportResize = ctx.viewport.setLatticeSize;
            let chartClears = 0;
            let viewportResizes = 0;
            ctx.clearCharts = () => { chartClears++; };
            ctx.viewport.setLatticeSize = () => { viewportResizes++; };
            const mutations = { records: 0, byAttribute: {} };
            const mutationObserver = new MutationObserver((list) => {
                for (const mutation of list) {
                    mutations.records++;
                    if (mutation.type === 'attributes') {
                        const key = `${mutation.target.id}:${mutation.attributeName}`;
                        mutations.byAttribute[key] = (mutations.byAttribute[key] || 0) + 1;
                    }
                }
            });
            mutationObserver.observe(card, { subtree: true, attributes: true, childList: true });
            const startEpoch = getScale0QualificationState().mutationEpoch;
            const scientificMutations = [];
            let lastObservedEpoch = startEpoch;
            const unsubscribeQualification = subscribeScale0Qualification((snapshot) => {
                const epoch = snapshot.lastMutation?.mutationEpoch;
                if (!Number.isInteger(epoch) || epoch <= lastObservedEpoch) return;
                lastObservedEpoch = epoch;
                scientificMutations.push({ ...snapshot.lastMutation });
            });
            setLatticeNeedsUpload(false);
            try {
                const x = /** @type {HTMLInputElement} */ (document.getElementById('inj-x'));
                const y = /** @type {HTMLInputElement} */ (document.getElementById('inj-y'));
                const z = /** @type {HTMLInputElement} */ (document.getElementById('inj-z'));
                // No blur/change event: the click handler itself must clamp and
                // reflect the values before crossing the bridge boundary.
                x.value = '999';
                y.value = '-8';
                z.value = '15.6';
                document.getElementById('btn-inject').click();
                document.getElementById('inj-state-neg').click();
                document.getElementById('btn-inject-wave').click();
                document.getElementById('btn-inject-flux').click();
                document.getElementById('btn-inject-pair').click();
                document.getElementById('btn-random').click();
                document.getElementById('btn-clear-field').click();
                document.getElementById('btn-random-flux').click();
                const uploadDirtyImmediate = state.latticeNeedsUpload;
                // Exercise the burst path without mutating engine state. It
                // must add calls only—never listeners, nodes, or resources.
                for (let i = 0; i < 10; i++) {
                    document.getElementById('btn-center').click();
                    document.getElementById('btn-inject-wave').click();
                    document.getElementById('btn-random-flux').click();
                }
                await new Promise((resolve) => requestAnimationFrame(() => resolve()));
                return {
                    activeIsWorker: owner === state.fluxMock && owner !== main,
                    mainCalls,
                    ownerCalls,
                    clampedAfterFirst: ownerCalls[0]?.slice(1, 5),
                    reflected: [x.value, y.value, z.value],
                    chartClears,
                    viewportResizes,
                    mutations,
                    uploadDirtyImmediate,
                    mutationEpochDelta: getScale0QualificationState().mutationEpoch - startEpoch,
                    scientificMutations,
                    before,
                    after: {
                        nodes: card.querySelectorAll('*').length,
                        inputs: card.querySelectorAll('input').length,
                        subscribers: rafCoordinator.size(),
                        resources: performance.getEntriesByType('resource').length,
                    },
                };
            } finally {
                for (const method of methods) {
                    main[method] = originals.main[method];
                    owner[method] = originals.owner[method];
                }
                ctx.clearCharts = originalClearCharts;
                ctx.viewport.setLatticeSize = originalViewportResize;
                unsubscribeQualification();
                mutationObserver.disconnect();
            }
        });

        expect(result.activeIsWorker).toBe(true);
        expect(result.mainCalls).toEqual([]);
        expect(result.ownerCalls.filter(([name]) => name === 'injectParticle')).toHaveLength(1);
        expect(result.ownerCalls.filter(([name]) => name === 'injectWavepacket')).toHaveLength(12);
        expect(result.ownerCalls.filter(([name]) => name === 'injectFlux')).toHaveLength(1);
        expect(result.ownerCalls.filter(([name]) => name === 'createEntangledPair')).toHaveLength(1);
        expect(result.ownerCalls.filter(([name]) => name === 'clearField')).toHaveLength(1);
        expect(result.ownerCalls.filter(([name]) => name === 'seedRandomFlux')).toHaveLength(11);
        expect(result.clampedAfterFirst).toEqual([32, 0, 16, 1]);
        expect(result.reflected.map(Number).every((v) => v >= 0 && v <= 32)).toBe(true);
        expect(result.chartClears).toBe(1);
        expect(result.viewportResizes).toBe(0);
        expect(result.mutations).toEqual({
            records: 2,
            byAttribute: {
                'inj-state-neg:class': 1,
                'inj-state-pos:class': 1,
            },
        });
        expect(result.uploadDirtyImmediate).toBe(true);
        expect(result.mutationEpochDelta).toBe(27);
        expect(result.scientificMutations).toHaveLength(27);
        expect(result.scientificMutations.every((mutation) => (
            mutation.source === 'controls.substrate'
            && mutation.dispatchStatus === 'unknown'
        ))).toBe(true);
        const reasonCounts = Object.fromEntries(
            ['inject-particle', 'inject-wavepacket', 'inject-flux', 'inject-pair', 'clear-field', 'random-flux']
                .map((reason) => [
                    reason,
                    result.scientificMutations.filter((mutation) => mutation.reason === reason).length,
                ]),
        );
        expect(reasonCounts).toEqual({
            'inject-particle': 1,
            'inject-wavepacket': 12,
            'inject-flux': 1,
            'inject-pair': 1,
            'clear-field': 1,
            'random-flux': 11,
        });
        expect(result.after).toEqual(result.before);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('successful lattice resize synchronizes coordinate limits before the next action', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(async () => {
            const controller = await import('/js/scales/scale0/controller.js?v=11');
            await controller.resize(window.__ftdCtx, 9);
        });
        await page.waitForFunction(() => {
            const inputs = ['inj-x', 'inj-y', 'inj-z'].map((id) => document.getElementById(id));
            return window.__ftdCtx?.fluxMock?.ready === true
                && window.__ftdCtx.fluxMock.latticeSize === 9
                && inputs.every((el) => el?.getAttribute('max') === '8');
        });
        const result = await page.evaluate(async () => {
            const { getScale0State, getActiveScale0Bridge } = await import(
                '/js/scales/scale0/state/store.js'
            );
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const owner = getActiveScale0Bridge(ctx, state);
            const original = owner.injectParticle;
            const calls = [];
            owner.injectParticle = (...args) => calls.push(args);
            try {
                for (const id of ['inj-x', 'inj-y', 'inj-z']) {
                    document.getElementById(id).value = '1000';
                }
                document.getElementById('btn-inject').click();
                return {
                    ownerN: owner.latticeSize,
                    maxes: ['inj-x', 'inj-y', 'inj-z']
                        .map((id) => document.getElementById(id).getAttribute('max')),
                    values: ['inj-x', 'inj-y', 'inj-z']
                        .map((id) => document.getElementById(id).value),
                    calls,
                };
            } finally {
                owner.injectParticle = original;
            }
        });

        expect(result.ownerN).toBe(9);
        expect(result.maxes).toEqual(['8', '8', '8']);
        expect(result.values).toEqual(['8', '8', '8']);
        expect(result.calls).toEqual([[8, 8, 8, 1, 0, 0, 0]]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
