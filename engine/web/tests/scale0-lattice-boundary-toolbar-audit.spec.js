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
                periodicAxes: [...document.querySelectorAll('#flux-periodic-axis option')].map((option) => ({
                    value: Number(option.value),
                    label: option.textContent?.trim(),
                })),
                groups: document.querySelectorAll('#lattice-size-group').length,
                sizeSelects: document.querySelectorAll('#lattice-size').length,
                boundarySelects: document.querySelectorAll('#flux-boundary-mode').length,
                periodicAxisSelects: document.querySelectorAll('#flux-periodic-axis').length,
                orientationLabel: document.querySelector('label[for="flux-periodic-axis"]')?.textContent?.trim(),
                globalClockReadouts: document.querySelectorAll('#global-clock-readout').length,
                viewToggles: [...document.querySelectorAll(
                    '#status-scene-controls .status-menu:first-of-type .view-toggle',
                )].map((button) => ({
                    id: button.id,
                    label: button.textContent?.trim(),
                    pressed: button.getAttribute('aria-pressed'),
                })),
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
        expect(result.periodicAxes).toEqual([
            { value: 2, label: 'Z · forward/aft' },
            { value: 0, label: 'X · lateral' },
            { value: 1, label: 'Y · vertical' },
            { value: 3, label: 'XYZ · all axes' },
        ]);
        expect(result.groups).toBe(1);
        expect(result.sizeSelects).toBe(1);
        expect(result.boundarySelects).toBe(1);
        expect(result.periodicAxisSelects).toBe(1);
        expect(result.orientationLabel).toBe('Orientation');
        expect(result.globalClockReadouts).toBe(1);
        expect(result.viewToggles).toEqual([
            { id: 'toggle-axes', label: 'Axes', pressed: 'true' },
            { id: 'toggle-grid', label: 'Grid', pressed: 'true' },
            { id: 'toggle-boundary-orientation', label: 'Arrows', pressed: 'true' },
            { id: 'toggle-global-clock', label: 'Clock', pressed: 'true' },
            { id: 'scene-force-flow', label: '∿ Flow', pressed: 'false' },
        ]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('orientation arrows and global ordinal clock follow engine truth', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.waitForFunction(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return getScale0State().fluxMock?.ready === true;
        });

        const result = await page.evaluate(async () => {
            const controller = await import('/js/scales/scale0/controller.js');
            const ctx = window.__ftdCtx;
            const owner = controller.getActivePhysicsOwner(ctx);
            const axisCalls = [];
            const originalAxis = owner.setFluxPeriodicAxis;
            owner.setFluxPeriodicAxis = function (axis) {
                axisCalls.push(axis);
                return originalAxis.call(this, axis);
            };
            try {
                const mode = document.getElementById('flux-boundary-mode');
                const axis = document.getElementById('flux-periodic-axis');
                mode.value = '2';
                mode.dispatchEvent(new Event('change', { bubbles: true }));
                await new Promise((resolve) => requestAnimationFrame(resolve));
                const disabledDispersal = axis.disabled;
                axis.value = '2';
                mode.value = '0';
                mode.dispatchEvent(new Event('change', { bubbles: true }));
                await new Promise((resolve) => requestAnimationFrame(resolve));
                const disabledPeriodic = axis.disabled;
                for (const value of [0, 1, 2, 3, 2]) {
                    axis.value = String(value);
                    axis.dispatchEvent(new Event('change', { bubbles: true }));
                }
                mode.value = '2';
                mode.dispatchEvent(new Event('change', { bubbles: true }));
                await new Promise((resolve) => requestAnimationFrame(resolve));

                const arrows = ctx.viewport?._sceneCore?._orientationArrows ?? [];
                const activeAxes = arrows
                    .filter((arrow) => arrow.line.material.opacity === 1)
                    .map((arrow) => arrow.userData.boundaryAxis);
                const core = ctx.viewport?._sceneCore;
                ctx.viewport?.setGlobalClockState?.({
                    tick: 17,
                    running: true,
                    maxCausalBudget: 0.36,
                    causalProjectionEvents: 1,
                });
                core?._animateGlobalClock?.(
                    core._globalClockPulseStartedAt + core._globalClockPulseDurationMs * 0.25,
                );
                const clockReadout = document.getElementById('global-clock-readout');
                const c4Reference = core?.globalClock
                    ?.getObjectByName('scale0-clock-c4-theory-reference');
                return {
                    disabledDispersal,
                    disabledPeriodic,
                    axisCalls,
                    selectedAxis: axis.value,
                    arrowCount: arrows.length,
                    activeAxes,
                    orientationName: ctx.viewport?._sceneCore?.boundaryOrientation?.name,
                    clockName: core?.globalClock?.name,
                    latticeSize: core?._latticeSize,
                    clockPosition: core?.globalClock?.position?.toArray(),
                    clockNdc: core?.globalClock?.position?.clone()
                        ?.project(core?._camera)?.toArray(),
                    cameraPosition: core?._camera?.position?.toArray(),
                    cameraTarget: core?._controls?.target?.toArray(),
                    clockModel: core?.globalClock?.userData?.clockModel,
                    phaseOrder: core?.globalClock?.userData?.phaseOrder,
                    phaseSegmentCount: core?._globalClockPhaseSegments?.length,
                    phaseTwoOpacity: core?._globalClockPhaseSegments?.[2]?.material?.opacity,
                    cursorRotation: core?._globalClockPhaseCursor?.rotation?.z,
                    forwardArrowName: core?._globalClockForwardArrow?.name,
                    c4Status: c4Reference?.userData,
                    clockReadout: clockReadout?.textContent,
                    clockRateData: clockReadout?.dataset?.clockRate,
                    causalBudgetData: clockReadout?.dataset?.causalBudget,
                    projectionData: clockReadout?.dataset?.causalProjection,
                    handRotation: core?._globalClockHand?.rotation?.z,
                    mappedClockRate: core?._globalClockRate,
                    colorMovedFromFree: core?._globalClockRateColor?.getHexString?.() !== '38bdf8',
                };
            } finally {
                owner.setFluxPeriodicAxis = originalAxis;
            }
        });

        expect(result.disabledDispersal).toBe(false);
        expect(result.disabledPeriodic).toBe(false);
        expect(result.axisCalls).toEqual([0, 1, 2, 3, 2]);
        expect(result.selectedAxis).toBe('2');
        expect(result.arrowCount).toBe(6);
        expect(result.activeAxes).toEqual([2, 2]);
        expect(result.orientationName).toBe('scale0-boundary-orientation');
        expect(result.clockName).toBe('scale0-global-ordinal-clock');
        expect(result.clockPosition).toEqual([
            result.latticeSize * 0.82,
            result.latticeSize + Math.max(3.2, result.latticeSize * 0.16),
            result.latticeSize * 0.82,
        ]);
        const cameraCenter = result.latticeSize / 2;
        for (const component of result.cameraTarget) {
            expect(component).toBeCloseTo(cameraCenter, 12);
        }
        expect(result.cameraPosition[0]).toBeCloseTo(cameraCenter, 12);
        expect(result.cameraPosition[1]).toBeCloseTo(cameraCenter, 12);
        expect(result.cameraPosition[2])
            .toBeCloseTo(cameraCenter + result.latticeSize * 2.2, 12);
        expect(Math.abs(result.clockNdc[0])).toBeLessThan(1);
        expect(Math.abs(result.clockNdc[1])).toBeLessThan(1);
        expect(result.clockNdc[2]).toBeGreaterThan(-1);
        expect(result.clockNdc[2]).toBeLessThan(1);
        expect(result.clockModel).toBe('global-ordinal-plus-selected-causal-budget');
        expect(result.phaseOrder).toEqual([
            'read', 'write', 'pair', 'Gauss', 'latency',
            'forces', 'movement', 'boundary', 'weak/triad', 'proper time',
        ]);
        expect(result.phaseSegmentCount).toBe(10);
        expect(result.phaseTwoOpacity).toBe(1);
        expect(result.cursorRotation).toBeCloseTo(-Math.PI / 2, 12);
        expect(result.forwardArrowName).toBe('scale0-clock-forward-update-arrow');
        expect(result.c4Status).toMatchObject({
            productionTelemetry: false,
            status: 'conditional-open',
        });
        expect(result.clockReadout).toBe('tick 17 · τ′min 0.800');
        expect(result.clockRateData).toBe('0.800000');
        expect(result.causalBudgetData).toBe('0.360000');
        expect(result.projectionData).toBe('true');
        expect(result.mappedClockRate).toBeCloseTo(0.8, 12);
        expect(result.colorMovedFromFree).toBe(true);
        expect(result.handRotation).toBeCloseTo(-7 * Math.PI * 2 / 10, 12);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('every clock visual has a semantic hover overlay', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const hoverModel = await page.evaluate(() => {
            const core = window.__ftdCtx?.viewport?._sceneCore;
            if (!core?.globalClock) throw new Error('global clock unavailable');
            core.setGlobalClockState({
                tick: 24,
                running: true,
                maxCausalBudget: 0.36,
                causalProjectionEvents: 0,
            });
            core.render(core._scene, core._camera);
            core.globalClock.updateMatrixWorld(true);

            const canvasRect = core._renderer.domElement.getBoundingClientRect();
            const project = (world) => {
                const ndc = world.project(core._camera);
                return {
                    x: canvasRect.left + (ndc.x + 1) * canvasRect.width / 2,
                    y: canvasRect.top + (1 - ndc.y) * canvasRect.height / 2,
                };
            };
            const ringPoint = (mesh, angle) => {
                const params = mesh.geometry.parameters;
                const radius = (params.innerRadius + params.outerRadius) / 2;
                const local = mesh.position.clone().set(
                    Math.cos(angle) * radius,
                    Math.sin(angle) * radius,
                    0,
                );
                return project(mesh.localToWorld(local));
            };

            const phase = core._globalClockPhaseSegments[0];
            const phaseParams = phase.geometry.parameters;
            const phasePoint = ringPoint(
                phase,
                phaseParams.thetaStart + phaseParams.thetaLength / 2,
            );
            const ratePoint = ringPoint(core._globalClockRateRing, 0.17);
            const c4 = core.globalClock.getObjectByName('scale0-clock-c4-theory-reference');
            const c4Node = c4.children[1];
            const c4Point = project(c4Node.getWorldPosition(c4Node.position.clone()));

            const hoverKeys = new Set();
            core.globalClock.traverse((object) => {
                if (object.userData?.clockHoverKey) hoverKeys.add(object.userData.clockHoverKey);
            });
            return {
                phasePoint,
                ratePoint,
                c4Point,
                hoverKeys: [...hoverKeys].sort(),
            };
        });

        expect(hoverModel.hoverKeys).toEqual(expect.arrayContaining([
            'clock', 'dial', 'hand', 'rate', 'cursor', 'arrow', 'c4',
            ...Array.from({ length: 10 }, (_, index) => `phase-${index}`),
        ]));

        const hover = page.locator('.scale0-clock-hover');
        await page.mouse.move(hoverModel.phasePoint.x, hoverModel.phasePoint.y);
        await expect(hover).toBeVisible();
        await expect(hover).toHaveAttribute('data-clock-hover-key', 'phase-0');
        await expect(hover.locator('.scale0-clock-hover-title')).toHaveText('Stage 1 · Read');
        await expect(hover.locator('.scale0-clock-hover-status')).toHaveText('[IMPLEMENTED ORDER]');

        await page.mouse.move(hoverModel.ratePoint.x, hoverModel.ratePoint.y);
        await expect(hover).toHaveAttribute('data-clock-hover-key', 'rate');
        await expect(hover.locator('.scale0-clock-hover-title')).toHaveText('Mapped local-rate band');
        await page.evaluate(() => {
            const core = window.__ftdCtx.viewport._sceneCore;
            core.setGlobalClockState({
                tick: 24,
                running: true,
                maxCausalBudget: 0.36,
                causalProjectionEvents: 0,
            });
            core._globalClockHover.show('rate', 800, 300);
        });
        await expect(hover.locator('.scale0-clock-hover-live'))
            .toContainText('Bmax 0.3600 · τ′min 0.8000');

        await page.mouse.move(hoverModel.c4Point.x, hoverModel.c4Point.y);
        await expect(hover).toHaveAttribute('data-clock-hover-key', 'c4');
        await expect(hover.locator('.scale0-clock-hover-title'))
            .toHaveText('C4 / quartic-clock reference');
        await expect(hover.locator('.scale0-clock-hover-status'))
            .toHaveText('[CONDITIONAL · OPEN]');

        await page.mouse.move(2, 2);
        await expect(hover).toBeHidden();
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('View menu independently hides orientation arrows and the 3D clock', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const viewMenu = page.locator('#status-scene-controls details.status-menu').first();
        await viewMenu.locator('summary').click();

        const orientation = page.locator('#toggle-boundary-orientation');
        const clock = page.locator('#toggle-global-clock');
        await expect(orientation).toBeVisible();
        await expect(clock).toBeVisible();

        const snapshot = () => page.evaluate(() => {
            const core = window.__ftdCtx?.viewport?._sceneCore;
            const orientationButton = document.getElementById('toggle-boundary-orientation');
            const clockButton = document.getElementById('toggle-global-clock');
            return {
                orientationVisible: core?.boundaryOrientation?.visible,
                clockVisible: core?.globalClock?.visible,
                showOrientation: core?._showBoundaryOrientation,
                showClock: core?._showGlobalClock,
                orientationPressed: orientationButton?.getAttribute('aria-pressed'),
                clockPressed: clockButton?.getAttribute('aria-pressed'),
                clockReadout: document.getElementById('global-clock-readout')?.textContent,
            };
        });

        const before = await snapshot();
        await orientation.click();
        const orientationOff = await snapshot();
        await clock.click();
        const bothOff = await snapshot();
        const rebuilt = await page.evaluate(() => {
            const core = window.__ftdCtx?.viewport?._sceneCore;
            core?.onLatticeSizeChanged(core._latticeSize, core._halfN);
            return {
                orientationVisible: core?.boundaryOrientation?.visible,
                clockVisible: core?.globalClock?.visible,
                showOrientation: core?._showBoundaryOrientation,
                showClock: core?._showGlobalClock,
            };
        });
        await orientation.click();
        await clock.click();
        const restored = await snapshot();

        expect(before).toMatchObject({
            orientationVisible: true,
            clockVisible: true,
            showOrientation: true,
            showClock: true,
            orientationPressed: 'true',
            clockPressed: 'true',
        });
        expect(orientationOff).toMatchObject({
            orientationVisible: false,
            clockVisible: true,
            showOrientation: false,
            showClock: true,
            orientationPressed: 'false',
            clockPressed: 'true',
        });
        expect(bothOff).toMatchObject({
            orientationVisible: false,
            clockVisible: false,
            showOrientation: false,
            showClock: false,
            orientationPressed: 'false',
            clockPressed: 'false',
        });
        expect(bothOff.clockReadout).toMatch(/^tick \d+( · τ′min \d\.\d{3})?$/);
        expect(rebuilt).toEqual({
            orientationVisible: false,
            clockVisible: false,
            showOrientation: false,
            showClock: false,
        });
        expect(restored).toMatchObject({
            orientationVisible: true,
            clockVisible: true,
            showOrientation: true,
            showClock: true,
            orientationPressed: 'true',
            clockPressed: 'true',
        });
        expect(restored.clockReadout).toMatch(/^tick \d+( · τ′min \d\.\d{3})?$/);
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
