// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, selectScale0Scenario } from './_helpers.js';

test.describe.serial('Scale 0 authoritative scientific-mutation contract', () => {
    /** @type {import('@playwright/test').BrowserContext|undefined} */
    let context;
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
        await selectScale0Scenario(page, 'empty');
        await expect.poll(() => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            return store.getScale0QualificationState().status;
        }), { timeout: 30_000 }).toBe('within-contract');
        await page.waitForTimeout(100);
    });

    test.afterAll(async () => {
        await context?.close();
    });

    test('cache-version mapping preserves one central store singleton', async () => {
        const result = await page.evaluate(async () => {
            const bare = await import('/js/scales/scale0/state/store.js');
            const versioned = await import('/js/scales/scale0/state/store.js?v=2');
            return {
                sameState: bare.getScale0State() === versioned.getScale0State(),
                sameQualification: bare.getScale0QualificationState
                    === versioned.getScale0QualificationState,
            };
        });
        expect(result).toEqual({ sameState: true, sameQualification: true });
    });

    test('visual-only work preserves Empty while an idempotent clear suspends it exactly once', async () => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const before = store.getScale0QualificationState();
            document.getElementById('toggle-flux-glow')?.click();
            const afterVisual = store.getScale0QualificationState();
            document.getElementById('btn-clear-field')?.click();
            const afterClear = store.getScale0QualificationState();
            return {
                before,
                afterVisual,
                afterClear,
                warning: document.getElementById('physics-profile-warning')?.textContent || '',
                metadata: document.getElementById('lat-scenario-desc-text')?.textContent || '',
            };
        });

        expect(result.before.status).toBe('within-contract');
        expect(result.afterVisual.mutationEpoch).toBe(result.before.mutationEpoch);
        expect(result.afterVisual.status).toBe('within-contract');
        expect(result.afterClear.mutationEpoch - result.before.mutationEpoch).toBe(1);
        expect(result.afterClear.status).toBe('suspended');
        expect(result.afterClear.lastMutation).toMatchObject({
            reason: 'clear-field',
            source: 'controls.substrate',
            dispatchStatus: 'unknown',
        });
        expect(result.warning).toContain('qualification suspended');
        expect(result.metadata).toContain('LIVE SCIENTIFIC RECORD MODIFIED — QUALIFICATION SUSPENDED');
        expect(result.metadata).toContain('Reason: clear-field; source: controls.substrate');
        expect(result.metadata).toContain('engine application is not acknowledged');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('only the matching authoritative reload acknowledgement restores qualification', async () => {
        const consoleErrors = attachConsoleWatcher(page);
        const pending = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const oldGeneration = ctx._loadGeneration || 0;
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('scenario-select'));
            select.value = 'empty';
            select.dispatchEvent(new Event('change', { bubbles: true }));
            const afterDispatch = store.getScale0QualificationState();
            const staleAccepted = store.completeScale0AuthoritativeLoad({
                scenarioId: 'empty',
                loadGeneration: oldGeneration,
                source: 'test-stale-ack',
            });
            return {
                oldGeneration,
                newGeneration: ctx._loadGeneration || 0,
                afterDispatch,
                afterStale: store.getScale0QualificationState(),
                staleAccepted,
            };
        });

        expect(pending.newGeneration).toBe(pending.oldGeneration + 1);
        expect(pending.afterDispatch.status).toBe('pending');
        expect(pending.staleAccepted).toBe(false);
        expect(pending.afterStale.status).toBe('pending');

        await expect.poll(() => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            return store.getScale0QualificationState().status;
        }), { timeout: 30_000 }).toBe('within-contract');
        const qualified = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            return {
                qualification: store.getScale0QualificationState(),
                loadGeneration: window.__ftdCtx?._loadGeneration,
            };
        });
        expect(qualified.qualification.status).toBe('within-contract');
        expect(qualified.qualification.anchor?.loadGeneration).toBe(pending.newGeneration);
        expect([
            'worker-configuration-applied',
            'native-profile-ack',
            'in-thread-engine-readback',
        ]).toContain(qualified.qualification.anchor?.source);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('a failed same-scenario reload cannot re-expose the old qualified anchor', async () => {
        const result = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const oldAnchor = store.getScale0QualificationState().anchor;
            ctx._loadGeneration = (ctx._loadGeneration || 0) + 1;
            const failedGeneration = ctx._loadGeneration;
            store.beginScale0AuthoritativeLoad({
                scenarioId: 'empty',
                loadGeneration: failedGeneration,
            });
            const failed = store.failScale0AuthoritativeLoad({
                scenarioId: 'empty',
                loadGeneration: failedGeneration,
                reason: 'test-setup-failure',
            });
            return {
                failed,
                failedGeneration,
                oldAnchor,
                qualification: store.getScale0QualificationState(),
                warning: document.getElementById('physics-profile-warning')?.textContent || '',
                metadata: document.getElementById('lat-scenario-desc-text')?.textContent || '',
            };
        });

        expect(result.failed).toBe(true);
        expect(result.qualification.status).toBe('suspended');
        expect(result.qualification.authoritativeLoad).toMatchObject({
            status: 'failed',
            scenarioId: 'empty',
            loadGeneration: result.failedGeneration,
            failureReason: 'test-setup-failure',
        });
        expect(result.qualification.anchor).toEqual(result.oldAnchor);
        expect(result.warning).toContain('Authoritative scenario load failed');
        expect(result.metadata).toContain('AUTHORITATIVE SCENARIO LOAD FAILED — QUALIFICATION SUSPENDED');
        expect(result.metadata).toContain('Failure: test-setup-failure');
        expect(result.metadata).not.toContain('Reason: clear-field');

        await selectScale0Scenario(page, 'empty', { settleMs: 0 });
        await expect.poll(() => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            return store.getScale0QualificationState().status;
        }), { timeout: 30_000 }).toBe('within-contract');
    });

    test('invalid provenance is inert while term and flux-boundary writes reach one owner once', async () => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const state = store.getScale0State();
            const owner = store.getActiveScale0Bridge(ctx, state);
            const before = store.getScale0QualificationState();
            let invalidCalls = 0;
            const invalid = store.commitScale0ScientificMutation(ctx, {
                reason: 'misspelled-reason',
                source: store.SCALE0_MUTATION_SOURCES.SUBSTRATE_CONTROLS,
                loadGeneration: ctx._loadGeneration || 0,
                owner,
            }, () => { invalidCalls += 1; });

            const main = ctx.bridge;
            const ownerToggleOriginal = owner.setToggle;
            const mainToggleOriginal = main.setToggle;
            const ownerOriginal = owner.setBoundaryShape;
            const mainOriginal = main.setBoundaryShape;
            const ownerFluxOriginal = owner.setFluxBoundaryMode;
            const mainFluxOriginal = main.setFluxBoundaryMode;
            const calls = { ownerToggle: 0, mainToggle: 0, ownerShape: 0, mainShape: 0, ownerFlux: 0, mainFlux: 0 };
            owner.setToggle = () => { calls.ownerToggle += 1; };
            main.setToggle = () => { calls.mainToggle += 1; };
            owner.setBoundaryShape = () => { calls.ownerShape += 1; };
            main.setBoundaryShape = () => { calls.mainShape += 1; };
            owner.setFluxBoundaryMode = () => { calls.ownerFlux += 1; };
            main.setFluxBoundaryMode = () => { calls.mainFlux += 1; };
            let afterToggle;
            let afterShape;
            try {
                const toggle = /** @type {HTMLInputElement} */ (document.getElementById('t-wave'));
                toggle.checked = !toggle.checked;
                toggle.dispatchEvent(new Event('change', { bubbles: true }));
                afterToggle = store.getScale0QualificationState();
                const select = /** @type {HTMLSelectElement} */ (document.getElementById('boundary-select'));
                select.value = 'sphere';
                select.dispatchEvent(new Event('change', { bubbles: true }));
                afterShape = store.getScale0QualificationState();
                const fluxSelect = /** @type {HTMLSelectElement} */ (document.getElementById('flux-boundary-mode'));
                fluxSelect.value = '1';
                fluxSelect.dispatchEvent(new Event('change', { bubbles: true }));
            } finally {
                owner.setToggle = ownerToggleOriginal;
                main.setToggle = mainToggleOriginal;
                owner.setBoundaryShape = ownerOriginal;
                main.setBoundaryShape = mainOriginal;
                owner.setFluxBoundaryMode = ownerFluxOriginal;
                main.setFluxBoundaryMode = mainFluxOriginal;
            }
            return {
                before,
                invalid,
                invalidCalls,
                after: store.getScale0QualificationState(),
                afterToggle,
                afterShape,
                calls,
                splitOwner: owner !== main,
            };
        });

        expect(result.invalid).toEqual({ accepted: false, dispatchStatus: 'rejected' });
        expect(result.invalidCalls).toBe(0);
        expect(result.calls.ownerToggle + result.calls.mainToggle).toBe(1);
        expect(result.calls.ownerShape + result.calls.mainShape).toBe(1);
        expect(result.calls.ownerFlux + result.calls.mainFlux).toBe(1);
        if (result.splitOwner) {
            expect(result.calls.mainToggle).toBe(0);
            expect(result.calls.mainShape).toBe(0);
            expect(result.calls.mainFlux).toBe(0);
        }
        expect(result.afterToggle.mutationEpoch - result.before.mutationEpoch).toBe(1);
        expect(result.afterToggle.lastMutation).toMatchObject({
            reason: 'physics-toggle',
            source: 'controls.physics-toggles',
        });
        expect(result.afterShape.mutationEpoch).toBe(result.afterToggle.mutationEpoch);
        expect(result.afterShape.lastMutation).toMatchObject({
            reason: 'physics-toggle',
            source: 'controls.physics-toggles',
        });
        expect(result.after.mutationEpoch - result.afterShape.mutationEpoch).toBe(1);
        expect(result.after.lastMutation).toMatchObject({
            reason: 'flux-boundary',
            source: 'toolbar.boundary',
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('commit-uncertain native resize remains suspended instead of assuming the old size', async () => {
        const result = await page.evaluate(async () => {
            const { resizeScale0Lattice } = await import(
                '/js/scales/scale0/runtime/scenario-loader.js'
            );
            const store = await import('/js/scales/scale0/state/store.js');
            let resizeCalls = 0;
            let setupTransactions = 0;
            const bridge = {
                isNativeGPU: true,
                latticeSize: 33,
                beginScenarioConfiguration() { setupTransactions += 1; },
                commitScenarioConfiguration() {},
                async resizeScenario() {
                    resizeCalls += 1;
                    const error = new Error('ack lost after dispatch');
                    error.resizeFailurePhase = 'commit-uncertain';
                    throw error;
                },
            };
            const ctx = {
                engineMode: 'lattice',
                bridge,
                _loadGeneration: 5000,
            };
            const state = { currentScenarioId: 'empty' };
            const originalToast = window.showToast;
            window.showToast = () => {};
            try {
                const completed = await resizeScale0Lattice(ctx, state, {}, 49);
                return {
                    completed,
                    resizeCalls,
                    setupTransactions,
                    bridgeSize: bridge.latticeSize,
                    qualification: store.getScale0QualificationState(),
                };
            } finally {
                window.showToast = originalToast;
            }
        });

        expect(result.completed).toBe(false);
        expect(result.resizeCalls).toBe(1);
        expect(result.setupTransactions).toBe(0);
        expect(result.bridgeSize).toBe(33);
        expect(result.qualification).toMatchObject({
            status: 'suspended',
            authoritativeLoad: {
                status: 'failed',
                scenarioId: 'empty',
                failureReason: 'native-resize-commit-uncertain',
            },
        });

        await selectScale0Scenario(page, 'empty', { settleMs: 0 });
        await expect.poll(() => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            return store.getScale0QualificationState().status;
        }), { timeout: 30_000 }).toBe('within-contract');
    });

    test('authoritative native size synchronizes every dependent UI surface', async () => {
        const result = await page.evaluate(async () => {
            const { syncScale0AuthoritativeLatticeSize } = await import(
                '/js/scales/scale0/controller.js'
            );
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('lattice-size'));
            const originalValue = select.value;
            const calls = { injection: [], selection: [], flow: [], viewport: [] };
            const ctx = {
                bridge: { latticeSize: 65 },
                syncScale0InjectionBounds: (size) => calls.injection.push(size),
                syncScale0SelectionBounds: (size) => calls.selection.push(size),
                syncScale0FlowLineControls: (size) => calls.flow.push(size),
                viewport: {
                    latticeSize: 33,
                    setLatticeSize(size) {
                        calls.viewport.push(size);
                        this.latticeSize = size;
                    },
                },
            };
            try {
                const accepted = syncScale0AuthoritativeLatticeSize(ctx, 65);
                return {
                    accepted,
                    selected: Number(select.value),
                    calls,
                    viewportSize: ctx.viewport.latticeSize,
                };
            } finally {
                select.value = originalValue;
            }
        });

        expect(result).toEqual({
            accepted: true,
            selected: 65,
            calls: {
                injection: [65],
                selection: [65],
                flow: [65],
                viewport: [65],
            },
            viewportSize: 65,
        });
    });

    test('rapid native resize intents are serialized and finish with the latest size', async () => {
        const result = await page.evaluate(async () => {
            const { resizeScale0Lattice } = await import(
                '/js/scales/scale0/runtime/scenario-loader.js'
            );
            const calls = [];
            const resolvers = [];
            let inFlight = 0;
            let maxInFlight = 0;
            const bridge = {
                isNativeGPU: true,
                latticeSize: 9,
                resizeScenario(size, scenarioId) {
                    calls.push({ size, scenarioId });
                    inFlight += 1;
                    maxInFlight = Math.max(maxInFlight, inFlight);
                    return new Promise((resolve) => {
                        resolvers.push(() => {
                            inFlight -= 1;
                            this.latticeSize = size;
                            resolve(true);
                        });
                    });
                },
            };
            const ctx = {
                engineMode: 'lattice',
                bridge,
                _loadGeneration: 100,
            };
            const state = { currentScenarioId: 'empty' };
            const first = resizeScale0Lattice(ctx, state, {}, 17);
            while (calls.length < 1) await new Promise((resolve) => setTimeout(resolve, 0));
            const second = resizeScale0Lattice(ctx, state, {}, 33);
            await new Promise((resolve) => setTimeout(resolve, 0));
            const beforeFirstAck = calls.map((call) => call.size);
            resolvers.shift()();
            while (calls.length < 2) await new Promise((resolve) => setTimeout(resolve, 0));
            const beforeSecondAck = calls.map((call) => call.size);
            // Prevent the fake bridge from entering the real canonical loader;
            // this test isolates request serialization and last-intent ordering.
            ctx.engineMode = 'particles';
            resolvers.shift()();
            const settled = await Promise.all([first, second]);
            return {
                beforeFirstAck,
                beforeSecondAck,
                calls,
                maxInFlight,
                finalSize: bridge.latticeSize,
                settled,
            };
        });

        expect(result.beforeFirstAck).toEqual([17]);
        expect(result.beforeSecondAck).toEqual([17, 33]);
        expect(result.calls).toEqual([
            { size: 17, scenarioId: 'empty' },
            { size: 33, scenarioId: 'empty' },
        ]);
        expect(result.maxInFlight).toBe(1);
        expect(result.finalSize).toBe(33);
        expect(result.settled).toEqual([false, false]);
    });
});
