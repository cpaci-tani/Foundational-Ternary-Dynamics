// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 lattice-size and boundary toolbar audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    });

    test('size and boundary menus expose the exact supported values', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const limits = await import('/js/scales/scale0/ui/toolbar/limits.js');
            const snapshotSizes = () => [...document.querySelectorAll('#lattice-size option')]
                .map((option) => ({
                    value: Number(option.value),
                    label: option.textContent?.trim(),
                    disabled: option.disabled,
                }));
            const wasmSizes = snapshotSizes();
            limits.syncScale0LatticeSizeAvailability(true);
            const nativeSizes = snapshotSizes();
            limits.syncScale0LatticeSizeAvailability(false);
            return {
                wasmSizes,
                nativeSizes,
                boundaries: [...document.querySelectorAll('#flux-boundary-mode option')].map((option) => ({
                    value: Number(option.value),
                    label: option.textContent?.trim(),
                })),
                groups: document.querySelectorAll('#lattice-size-group').length,
                sizeSelects: document.querySelectorAll('#lattice-size').length,
                boundarySelects: document.querySelectorAll('#flux-boundary-mode').length,
            };
        });

        expect(result.wasmSizes.map((option) => option.value))
            .toEqual([9, 17, 25, 33, 49, 65, 97, 113, 145, 181]);
        expect(result.wasmSizes.every((option) => option.value > 0 && option.value % 2 === 1)).toBe(true);
        expect(result.wasmSizes.filter((option) => option.value <= 97).every((option) => !option.disabled)).toBe(true);
        expect(result.wasmSizes.filter((option) => option.value > 97)).toEqual([
            { value: 113, label: '113 · Native GPU', disabled: true },
            { value: 145, label: '145 · Native GPU', disabled: true },
            { value: 181, label: '181 · Native GPU', disabled: true },
        ]);
        expect(result.nativeSizes.filter((option) => option.value > 97)).toEqual([
            { value: 113, label: '113', disabled: false },
            { value: 145, label: '145', disabled: false },
            { value: 181, label: '181', disabled: false },
        ]);
        expect(result.boundaries).toEqual([
            { value: 2, label: 'Dispersal' },
            { value: 1, label: 'Reflective' },
            { value: 0, label: 'Periodic' },
        ]);
        expect(result.groups).toBe(1);
        expect(result.sizeSelects).toBe(1);
        expect(result.boundarySelects).toBe(1);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('worker-init recovery clamps to direct L33, disables larger sizes, reloads, and refuses a later L49 resize', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        test.skip(!(await page.evaluate(() => globalThis.crossOriginIsolated === true)),
            'requires the COOP/COEP Scale 0 worker path');
        const result = await page.evaluate(async () => {
            const controller = await import('/js/scales/scale0/controller.js');
            const scenarioLoader = await import('/js/scales/scale0/runtime/scenario-loader.js');
            const store = await import('/js/scales/scale0/state/store.js');
            const { createScale0ViewportAdapter } = await import('/js/scales/scale0/viewport-adapter.js');
            const ctx = window.__ftdCtx;
            const state = store.getScale0State();
            await controller.resize(ctx, 49);
            const deadline = performance.now() + 30_000;
            while (performance.now() < deadline) {
                const owner = store.getActiveScale0Bridge(ctx, state);
                if (owner?.isWorker === true && owner.ready === true
                    && owner.latticeSize === 49
                    && store.isScale0AuthoritativeGenerationReady(state)) break;
                await new Promise((resolve) => setTimeout(resolve, 25));
            }
            const workerBefore = store.getActiveScale0Bridge(ctx, state);
            const viewportAdapter = createScale0ViewportAdapter(ctx.viewport);
            await scenarioLoader.fallbackToInThreadEngine(
                ctx,
                state,
                viewportAdapter,
                state.currentScenarioId,
                { id: state.currentScenarioId },
            );
            const select = document.getElementById('lattice-size');
            const afterFallback = {
                workerBefore: workerBefore?.isWorker === true,
                workerDisabled: ctx._wasmWorkerDisabled === true,
                useFluxMock: state.useFluxMock,
                selected: Number(select.value),
                bridgeSize: Number(ctx.bridge?.latticeSize),
                qualification: store.getScale0QualificationState().status,
                options: [...select.options].map((option) => ({
                    value: Number(option.value),
                    disabled: option.disabled,
                    label: option.textContent?.trim(),
                })),
            };
            const laterResizeAccepted = await scenarioLoader.resizeScale0Lattice(
                ctx, state, viewportAdapter, 49,
            );
            return {
                afterFallback,
                laterResizeAccepted,
                afterRefusal: {
                    selected: Number(select.value),
                    bridgeSize: Number(ctx.bridge?.latticeSize),
                    useFluxMock: state.useFluxMock,
                },
            };
        });

        expect(result.afterFallback.workerBefore).toBe(true);
        expect(result.afterFallback.workerDisabled).toBe(true);
        expect(result.afterFallback.useFluxMock).toBe(false);
        expect(result.afterFallback.selected).toBe(33);
        expect(result.afterFallback.bridgeSize).toBe(33);
        expect(result.afterFallback.qualification).toBe('within-contract');
        expect(result.afterFallback.options.filter((option) => option.value > 33)
            .every((option) => option.disabled)).toBe(true);
        expect(result.afterFallback.options.find((option) => option.value === 49)?.label)
            .toContain('WASM worker / Native GPU');
        expect(result.laterResizeAccepted).toBe(false);
        expect(result.afterRefusal).toEqual({
            selected: 33,
            bridgeSize: 33,
            useFluxMock: false,
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('each boundary input updates the active owner and viewport exactly once', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.waitForFunction(async () => {
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            const lifecycle = owner?.lifecycleDebug;
            const qualification = getScale0QualificationState();
            return owner?.ready === true
                && !!lifecycle?.workerRuntimeId
                && lifecycle.appliedConfigurationToken === lifecycle.configurationToken
                && qualification.status === 'within-contract';
        });

        const result = await page.evaluate(async () => {
            const controller = await import('/js/scales/scale0/controller.js');
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const active = controller.getActivePhysicsOwner(ctx);
            const main = ctx.bridge;
            const viewport = ctx.viewport;
            const originalActive = active.setFluxBoundaryMode;
            const originalMain = main?.setFluxBoundaryMode;
            const originalViewport = viewport?.setReflectiveBoundary;
            const activeCalls = [];
            const mainCalls = [];
            const viewportCalls = [];
            const uploadFlags = [];

            active.setFluxBoundaryMode = function (mode) {
                activeCalls.push(mode);
                return originalActive.call(this, mode);
            };
            if (main !== active && typeof originalMain === 'function') {
                main.setFluxBoundaryMode = function (mode) {
                    mainCalls.push(mode);
                    return originalMain.call(this, mode);
                };
            }
            if (typeof originalViewport === 'function') {
                viewport.setReflectiveBoundary = function (on) {
                    viewportCalls.push(!!on);
                    return originalViewport.call(this, on);
                };
            }

            try {
                getScale0State().latticeNeedsUpload = false;
                const select = document.getElementById('flux-boundary-mode');
                for (const mode of [0, 1, 2]) {
                    getScale0State().latticeNeedsUpload = false;
                    select.value = String(mode);
                    select.dispatchEvent(new Event('change', { bubbles: true }));
                    // Read synchronously: the next render frame legitimately
                    // consumes this one-shot invalidation flag.
                    uploadFlags.push(getScale0State().latticeNeedsUpload);
                    await new Promise((resolve) => requestAnimationFrame(resolve));
                }
                return {
                    activeCalls,
                    mainCalls,
                    viewportCalls,
                    uploadFlags,
                    distinctOwners: main !== active,
                    selected: select.value,
                };
            } finally {
                active.setFluxBoundaryMode = originalActive;
                if (main !== active && typeof originalMain === 'function') {
                    main.setFluxBoundaryMode = originalMain;
                }
                if (typeof originalViewport === 'function') {
                    viewport.setReflectiveBoundary = originalViewport;
                }
            }
        });

        expect(result.activeCalls).toEqual([0, 1, 2]);
        if (result.distinctOwners) expect(result.mainCalls).toEqual([]);
        expect(result.viewportCalls).toEqual([false, true, false]);
        expect(result.uploadFlags).toEqual([true, true, true]);
        expect(result.selected).toBe('2');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('ten rapid small resizes conserve workers and commit only the final lattice', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        test.skip(!(await page.evaluate(() => globalThis.crossOriginIsolated === true)),
            'requires the COOP/COEP Scale 0 worker path');
        await page.waitForFunction(async () => {
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const owner = getScale0State().fluxMock;
            const lifecycle = owner?.lifecycleDebug;
            const qualification = getScale0QualificationState();
            return owner?.ready === true
                && !!lifecycle?.workerRuntimeId
                && lifecycle.appliedConfigurationToken === lifecycle.configurationToken
                && qualification.status === 'within-contract';
        });

        const result = await page.evaluate(async () => {
            const { getScale0State, getScale0QualificationState } =
                await import('/js/scales/scale0/state/store.js');
            const { rafCoordinator } = await import('/js/lib/raf-coordinator.js');
            const ctx = window.__ftdCtx;
            const baselineDeadline = performance.now() + 20_000;
            let state;
            while (performance.now() < baselineDeadline) {
                state = getScale0State();
                const lifecycle = state.fluxMock?.lifecycleDebug;
                const qualification = getScale0QualificationState();
                if (state.fluxMock?.ready === true
                    && !!lifecycle?.workerRuntimeId
                    && lifecycle.appliedConfigurationToken === lifecycle.configurationToken
                    && qualification.status === 'within-contract') break;
                await new Promise((resolve) => setTimeout(resolve, 50));
            }
            const select = document.getElementById('lattice-size');
            const sizes = [17, 25, 33, 17, 25, 33, 17, 25, 17, 33];
            const before = {
                workers: window.__ftdWasmWorkers(),
                lifecycle: state.fluxMock?.lifecycleDebug ?? null,
                generation: ctx._loadGeneration || 0,
                subscribers: rafCoordinator.size(),
                scenario: state.currentScenarioId,
            };

            for (const size of sizes) {
                select.value = String(size);
                select.dispatchEvent(new Event('change', { bubbles: true }));
            }

            const finalSize = sizes.at(-1);
            const deadline = performance.now() + 20_000;
            while (performance.now() < deadline) {
                state = getScale0State();
                const lifecycle = state.fluxMock?.lifecycleDebug;
                const qualification = getScale0QualificationState();
                if (state.fluxMock?.ready === true
                    && state.fluxMock?.latticeSize === finalSize
                    && ctx.viewport?.latticeSize === finalSize
                    && lifecycle?.appliedConfigurationToken === lifecycle?.configurationToken
                    && qualification.status === 'within-contract'
                    && qualification.anchor?.scenarioId === state.currentScenarioId
                    && qualification.anchor?.loadGeneration === ctx._loadGeneration) break;
                await new Promise((resolve) => setTimeout(resolve, 50));
            }
            state = getScale0State();
            const qualification = getScale0QualificationState();

            return {
                inputCount: sizes.length,
                finalSize,
                selected: Number(select.value),
                mainSize: ctx.bridge?.latticeSize,
                ownerSize: state.fluxMock?.latticeSize,
                ownerReady: state.fluxMock?.ready === true,
                viewportSize: ctx.viewport?.latticeSize,
                scenario: state.currentScenarioId,
                before,
                afterWorkers: window.__ftdWasmWorkers(),
                afterLifecycle: state.fluxMock?.lifecycleDebug ?? null,
                qualification,
                afterGeneration: ctx._loadGeneration || 0,
                afterSubscribers: rafCoordinator.size(),
            };
        });

        expect(result.afterGeneration - result.before.generation).toBe(result.inputCount);
        expect(result.afterWorkers.created - result.before.workers.created).toBe(0);
        expect(result.afterWorkers.terminated - result.before.workers.terminated).toBe(0);
        expect(result.afterWorkers.created).toBe(
            result.afterWorkers.terminated + result.afterWorkers.live,
        );
        expect(result.afterWorkers.live).toBe(1);
        expect(result.before.lifecycle.workerRuntimeId).toBeTruthy();
        expect(result.afterLifecycle.workerRuntimeId).toBe(result.before.lifecycle.workerRuntimeId);
        expect(result.afterLifecycle.moduleInitCount).toBe(1);
        expect(result.afterLifecycle.renderBridgeGeneration
            - result.before.lifecycle.renderBridgeGeneration).toBe(1);
        expect(result.afterLifecycle.appliedConfigurationToken)
            .toBe(result.afterLifecycle.configurationToken);
        expect(result.qualification.status).toBe('within-contract');
        expect(result.qualification.anchor.scenarioId).toBe(result.scenario);
        expect(result.qualification.anchor.loadGeneration).toBe(result.afterGeneration);
        expect(result.qualification.anchor.source).toBe('worker-configuration-applied');
        expect(result.afterSubscribers).toBe(result.before.subscribers);
        expect(result.selected).toBe(result.finalSize);
        expect(result.mainSize).toBe(result.finalSize);
        expect(result.ownerSize).toBe(result.finalSize);
        expect(result.viewportSize).toBe(result.finalSize);
        expect(result.ownerReady).toBe(true);
        expect(result.scenario).toBe(result.before.scenario);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('browser WASM refuses a native-only size before replacing its active worker', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.waitForFunction(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.ready === true;
        });

        const result = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const select = document.getElementById('lattice-size');
            const before = {
                selected: Number(select.value),
                main: ctx.bridge?.latticeSize,
                owner: state.fluxMock?.latticeSize,
                viewport: ctx.viewport?.latticeSize,
                workers: window.__ftdWasmWorkers(),
            };
            select.value = '113';
            select.dispatchEvent(new Event('change', { bubbles: true }));
            await new Promise((resolve) => setTimeout(resolve, 100));
            return {
                before,
                after: {
                    selected: Number(select.value),
                    main: ctx.bridge?.latticeSize,
                    owner: state.fluxMock?.latticeSize,
                    viewport: ctx.viewport?.latticeSize,
                    workers: window.__ftdWasmWorkers(),
                },
                optionDisabled: select.querySelector('option[value="113"]')?.disabled,
            };
        });

        expect(result.optionDisabled).toBe(true);
        expect(result.after).toEqual(result.before);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('a superseded asynchronous native resize cannot reload stale Scale 0 state', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const { resizeScale0Lattice } = await import('/js/scales/scale0/runtime/scenario-loader.js');
            let releaseResize;
            const resizeGate = new Promise((resolve) => { releaseResize = resolve; });
            let resizeCalls = 0;
            const bridge = {
                isNativeGPU: true,
                isWasm64: true,
                latticeSize: 33,
                async resizeScenario() {
                    resizeCalls += 1;
                    await resizeGate;
                },
            };
            const ctx = {
                bridge,
                engineMode: 'lattice',
                _loadGeneration: 41,
            };
            const state = { currentScenarioId: 'flux-pulse' };
            const originalToast = window.showToast;
            window.showToast = () => {};
            try {
                const resizePromise = resizeScale0Lattice(ctx, state, {}, 49);
                while (resizeCalls === 0) {
                    await new Promise((resolve) => setTimeout(resolve, 0));
                }
                // A newer scenario/resize request wins while the native server
                // is still preparing the old allocation.
                ctx._loadGeneration += 1;
                releaseResize();
                await resizePromise;
                return {
                    resizeCalls,
                    bridgeSize: bridge.latticeSize,
                    generation: ctx._loadGeneration,
                };
            } finally {
                window.showToast = originalToast;
            }
        });

        expect(result.resizeCalls).toBe(1);
        expect(result.bridgeSize).toBe(33);
        expect(result.generation).toBe(43);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
