// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 Visualization overlay audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
    });

    test('visual shell uses the canonical dashboard surfaces and interaction states', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(() => {
            const read = (selector) => {
                const style = getComputedStyle(document.querySelector(selector));
                return {
                    background: style.backgroundColor,
                    border: style.borderColor,
                    radius: style.borderRadius,
                    shadow: style.boxShadow,
                    color: style.color,
                };
            };
            const probe = document.createElement('div');
            probe.style.cssText = [
                'position:fixed',
                'inset:auto',
                'background:var(--state-active-bg)',
                'border:1px solid var(--accent)',
                'color:var(--text-primary)',
            ].join(';');
            document.body.append(probe);
            const canonicalActive = read('body > div:last-child');
            probe.remove();

            const categoryColors = [...document.querySelectorAll('.s0-overlay-col')]
                .map((card) => getComputedStyle(card).getPropertyValue('--s0-col').trim());
            return {
                overlay: read('#viewport-overlay'),
                dashboard: read('#panel-area'),
                header: read('.s0-overlay-header'),
                renderDeck: read('.s0-overlay-render-deck'),
                category: read('.s0-overlay-col'),
                categoryHeader: read('.s0-overlay-col-head'),
                search: read('.s0-overlay-search-input'),
                activeLayer: read('#toggle-flux-volume'),
                activeRenderMode: read('#scalar-render-row .style-btn.active'),
                canonicalActive,
                categoryColorCount: new Set(categoryColors).size,
            };
        });

        expect(result.overlay).toEqual(result.dashboard);
        expect(result.header.background).toBe(result.overlay.background);
        expect(result.renderDeck.background).toBe(result.category.background);
        expect(result.categoryHeader.background).toBe(result.category.background);
        expect(result.search.background).not.toBe(result.overlay.background);
        expect(result.activeLayer.background).toBe(result.canonicalActive.background);
        expect(result.activeLayer.border).toBe(result.canonicalActive.border);
        expect(result.activeLayer.color).toBe(result.canonicalActive.color);
        expect(result.activeRenderMode.background).toBe(result.canonicalActive.background);
        expect(result.activeRenderMode.border).toBe(result.canonicalActive.border);
        expect(result.categoryColorCount).toBe(8);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('inventory is complete and false/idempotent renderer paths allocate nothing', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const THREE = await import('three');
            const { COL_TO_TOGGLES } = await import('/js/scales/scale0/ui/overlays/presets.js');
            const { FIELD_TOGGLE_BINDINGS } = await import('/js/scales/scale0/ui/dom.js');
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const { ViewportFieldRenderer } = await import('/js/viewport/field-renderer.js?v=4');
            const { TopologySheetRenderer } = await import('/js/viewport/topology-sheet-renderer.js?v=2');
            const panel = document.getElementById('viewport-overlay');
            const primaryIds = Object.values(COL_TO_TOGGLES).flat();
            const boundIds = FIELD_TOGGLE_BINDINGS.map(([id]) => id);
            const storeKeys = new Set(Object.keys(getScale0State().fieldFlags));
            const scene = new THREE.Scene();
            const field = new ViewportFieldRenderer({
                scene,
                camera: new THREE.PerspectiveCamera(),
                latticeSize: 33,
                halfN: 16.5,
                boundaryShape: 'cube',
                insideBoundary: () => true,
                getBoundaryMode: () => 'lattice',
            });
            const fieldFalseMethods = [
                'toggleEFieldLines', 'toggleBFieldLines', 'togglePoyntingVectors',
                'toggleDivergenceField', 'toggleForceVolume', 'toggleGravityField',
                'toggleStrongForce', 'toggleWeakField', 'toggleDualFluxVolume',
                'toggleChiralityField', 'togglePsiSquaredField', 'togglePhaseField',
                'toggleLagrangianDensityField', 'toggleEntropyDensityField',
                'toggleHorizonField', 'toggleStateField', 'toggleLatencyField',
                'toggleGaussResidualField', 'toggleDarkMatterHalo', 'toggleDampingZones',
                'toggleKnotZones', 'toggleGenesisIsosurface', 'toggleConfinement',
                'toggleFluxSlice', 'toggleFluxStreamlines',
            ];
            for (const method of fieldFalseMethods) field[method]?.(false);
            field.showForceHeatmap(false);
            field.showForceStreamlines_vis(false);
            field.showForceGlyphs(false);
            field.hideAllForceStyles();
            const fieldAllocationsAfterFalse = {
                sceneChildren: scene.children.length,
                objects: [
                    '_eFieldLines', '_bFieldLines', '_poyntingVectors', '_divField',
                    '_forceVolume', '_gravityField', '_strongForce', '_weakField',
                    '_forceHeatmap', '_forceStreamlinePool', '_forceGlyphMeshes',
                    '_dualFluxVolume', '_chiralityField', '_quantumField', '_phaseNeedles',
                    '_horizonField', '_stateField', '_scalarClouds', '_darkMatterHalo',
                    '_dampingZones', '_knotZones', '_genesisIsosurface',
                    '_confinementStrings', '_fluxSliceMesh', '_fluxStreamlines',
                ].filter((key) => field[key] != null),
            };
            field.toggleEFieldLines(true);
            const afterFieldOn = { children: scene.children.length, object: field._eFieldLines };
            field.toggleEFieldLines(true);
            field.toggleEFieldLines(false);
            field.toggleEFieldLines(false);
            const fieldIdempotent = {
                children: scene.children.length,
                sameObject: afterFieldOn.object === field._eFieldLines,
                visible: field._eFieldLines.visible,
            };

            const drawableScene = new THREE.Scene();
            const drawableField = new ViewportFieldRenderer({
                scene: drawableScene,
                camera: new THREE.PerspectiveCamera(),
                latticeSize: 33,
                halfN: 16.5,
                boundaryShape: 'cube',
                insideBoundary: () => true,
                getBoundaryMode: () => 'lattice',
            });
            drawableField.toggleEFieldLines(true);
            drawableField.toggleBFieldLines(true);
            drawableField.togglePoyntingVectors(true);
            const emptyRequestedVisibility = {
                e: drawableField._eFieldLines.visible,
                b: drawableField._bFieldLines.visible,
                poynting: drawableField._poyntingVectors.visible,
            };
            const oneLine = {
                count: 1,
                buffer: new Float32Array([0, 0, 0, 1, 0, 0]),
                offsets: new Uint32Array([0]),
                lengths: new Uint32Array([6]),
            };
            drawableField.updateEFieldLines(oneLine, null);
            drawableField.updateBFieldLines(oneLine, null);
            drawableField.updatePoyntingVectors({
                count: 1,
                positions: new Float32Array([0, 0, 0]),
                vectors: new Float32Array([1, 0, 0]),
            });
            const populatedRequestedVisibility = {
                e: drawableField._eFieldLines.visible,
                b: drawableField._bFieldLines.visible,
                poynting: drawableField._poyntingVectors.visible,
                drawRanges: [
                    drawableField._eFieldLines.geometry.drawRange.count,
                    drawableField._bFieldLines.geometry.drawRange.count,
                    drawableField._poyntingVectors.geometry.drawRange.count,
                ],
            };

            const topology = new TopologySheetRenderer({
                scene,
                getLatticeSize: () => 33,
                getHalfN: () => 16.5,
            });
            topology.toggleGravPotential(false);
            for (const key of ['emEnergy', 'chargeDensity', 'vorticity', 'ePressure', 'bPressure']) {
                topology.toggle(key, false);
            }
            const topologyAfterFalse = {
                children: scene.children.length,
                grav: topology._gravSurface,
                keys: Object.keys(topology._topoSheets),
            };
            topology.toggle('emEnergy', true);
            const sheet = topology._topoSheets.emEnergy;
            const afterSheetOn = scene.children.length;
            topology.toggle('emEnergy', true);
            topology.setHeight('emEnergy', 0.56);
            topology.toggle('emEnergy', false);
            topology.toggle('emEnergy', false);
            const topologyIdempotent = {
                children: scene.children.length,
                expectedChildren: afterSheetOn,
                sameSolid: sheet.solid === topology._topoSheets.emEnergy.solid,
                visible: sheet.solid.visible,
            };

            const result = {
                panel: {
                    columns: panel.querySelectorAll('.s0-overlay-col').length,
                    primaryButtons: primaryIds.length,
                    uniquePrimary: new Set(primaryIds).size,
                    allViewButtons: panel.querySelectorAll('.s0-overlay-body .view-toggle').length,
                    fieldBindings: FIELD_TOGGLE_BINDINGS.length,
                    sheetSliders: panel.querySelectorAll('.s0-sheet-height-slider').length,
                    forceStyles: panel.querySelectorAll('#force-style-row .style-btn').length,
                    scalarStyles: panel.querySelectorAll('#scalar-render-row .style-btn').length,
                    axes: panel.querySelectorAll('[id^="flux-slice-axis-"]').length,
                    fluxStyles: panel.querySelectorAll('#toggle-flux-organic, #toggle-flux-glow').length,
                    clearButtons: panel.querySelectorAll('.s0-overlay-col-clear').length,
                    missingPrimary: primaryIds.filter((id) => document.querySelectorAll(`#${id}`).length !== 1),
                    ungroupedBindings: boundIds.filter((id) => !primaryIds.includes(id)),
                    nonStoreBindings: FIELD_TOGGLE_BINDINGS.filter(([, key]) => !storeKeys.has(key)),
                    standalonePrimary: primaryIds.filter((id) => !boundIds.includes(id)),
                },
                fieldAllocationsAfterFalse,
                fieldIdempotent,
                emptyRequestedVisibility,
                populatedRequestedVisibility,
                topologyAfterFalse,
                topologyIdempotent,
            };
            field.dispose();
            drawableField.dispose();
            topology.dispose();
            return result;
        });

        expect(result.panel).toEqual({
            columns: 8, primaryButtons: 33, uniquePrimary: 33, allViewButtons: 38,
            fieldBindings: 30, sheetSliders: 6, forceStyles: 4, scalarStyles: 2,
            axes: 3, fluxStyles: 2, clearButtons: 8, missingPrimary: [],
            ungroupedBindings: [], nonStoreBindings: [],
            standalonePrimary: ['toggle-sm-reference', 'toggle-flux-volume', 'toggle-flux-slice'],
        });
        expect(result.fieldAllocationsAfterFalse).toEqual({ sceneChildren: 0, objects: [] });
        expect(result.fieldIdempotent).toEqual({ children: 1, sameObject: true, visible: false });
        expect(result.emptyRequestedVisibility).toEqual({ e: false, b: false, poynting: false });
        expect(result.populatedRequestedVisibility).toEqual({
            e: true, b: true, poynting: true, drawRanges: [2, 2, 2],
        });
        expect(result.topologyAfterFalse).toEqual({ children: 1, grav: null, keys: [] });
        expect(result.topologyIdempotent).toEqual({
            children: 3, expectedChildren: 3, sameSolid: true, visible: false,
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('shell reconciliation is incremental and input bursts commit once per frame', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const twoFrames = async () => {
                await new Promise((resolve) => requestAnimationFrame(resolve));
                await new Promise((resolve) => requestAnimationFrame(resolve));
            };
            const panel = document.getElementById('viewport-overlay');
            const strip = document.getElementById('s0-overlay-active');
            for (const id of ['toggle-e-field', 'toggle-b-field', 'toggle-poynting', 'toggle-flux-lines']) {
                const button = document.getElementById(id);
                if (!button.classList.contains('active')) button.click();
            }
            await twoFrames();
            const retainedBefore = new Map(
                [...strip.querySelectorAll('.s0-overlay-chip')]
                    .map((chip) => [chip.dataset.overlayId, chip]),
            );
            const stripMutations = { records: 0, added: 0, removed: 0 };
            const stripObserver = new MutationObserver((records) => {
                for (const record of records) {
                    stripMutations.records++;
                    stripMutations.added += record.addedNodes?.length || 0;
                    stripMutations.removed += record.removedNodes?.length || 0;
                }
            });
            stripObserver.observe(strip, { childList: true, subtree: true });
            document.getElementById('toggle-b-field').click();
            await twoFrames();
            stripObserver.disconnect();
            const retainedAfter = new Map(
                [...strip.querySelectorAll('.s0-overlay-chip')]
                    .map((chip) => [chip.dataset.overlayId, chip]),
            );
            const retainedIdentity = [...retainedAfter.entries()].every(
                ([id, chip]) => id === 'toggle-b-field' || retainedBefore.get(id) === chip,
            );

            const search = document.getElementById('s0-overlay-search');
            const body = panel.querySelector('.s0-overlay-body');
            let filterMutations = 0;
            const filterObserver = new MutationObserver((records) => { filterMutations += records.length; });
            filterObserver.observe(body, { subtree: true, attributes: true });
            for (let i = 0; i < 1000; i++) {
                search.value = i === 999 ? 'vortic' : `no-match-${i}`;
                search.dispatchEvent(new Event('input', { bubbles: true }));
            }
            const filterImmediate = filterMutations;
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const filterAfterFrame = filterMutations;
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const filterAfterSecond = filterMutations;
            filterObserver.disconnect();
            const filterTruth = {
                value: search.value,
                vorticityVisible: !document.getElementById('toggle-vorticity').classList.contains('is-filtered-out'),
                volumeHidden: panel.querySelector('[data-col="volume"]').classList.contains('is-filtered-out'),
            };
            search.value = '';
            search.dispatchEvent(new Event('input', { bubbles: true }));
            await twoFrames();

            const ctx = window.__ftdCtx;
            const slider = document.getElementById('sheet-height-em-energy');
            const display = document.getElementById('sheet-height-em-energy-val');
            let heightCalls = 0;
            const originalHeight = ctx.viewport.setTopologySheetHeight;
            ctx.viewport.setTopologySheetHeight = function (...args) {
                heightCalls++;
                return originalHeight.apply(this, args);
            };
            const displayMutations = { records: 0, characterData: 0, added: 0, removed: 0 };
            const displayObserver = new MutationObserver((records) => {
                for (const record of records) {
                    displayMutations.records++;
                    if (record.type === 'characterData') displayMutations.characterData++;
                    displayMutations.added += record.addedNodes?.length || 0;
                    displayMutations.removed += record.removedNodes?.length || 0;
                }
            });
            displayObserver.observe(display, { subtree: true, childList: true, characterData: true });
            for (let i = 0; i < 600; i++) {
                slider.value = '0.73';
                slider.dispatchEvent(new Event('input', { bubbles: true }));
            }
            const sliderImmediate = heightCalls;
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const sliderAfterFrame = heightCalls;
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const sliderAfterSecond = heightCalls;
            displayObserver.disconnect();
            const displayAfterCommit = display.textContent;

            slider.value = '0.44';
            slider.dispatchEvent(new Event('input', { bubbles: true }));
            ctx._loadGeneration++;
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterStaleGeneration = { calls: heightCalls, display: display.textContent };
            slider.value = '0.31';
            slider.dispatchEvent(new Event('input', { bubbles: true }));
            const engineMode = document.getElementById('engine-mode');
            engineMode.value = 'particles';
            engineMode.dispatchEvent(new Event('change', { bubbles: true }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterInactiveScale = { calls: heightCalls, display: display.textContent };
            engineMode.value = 'lattice';
            engineMode.dispatchEvent(new Event('change', { bubbles: true }));
            ctx.viewport.setTopologySheetHeight = originalHeight;

            return {
                chips: {
                    before: [...retainedBefore.keys()],
                    after: [...retainedAfter.keys()],
                    retainedIdentity,
                    mutations: stripMutations,
                },
                filter: {
                    immediate: filterImmediate,
                    afterFrame: filterAfterFrame,
                    afterSecond: filterAfterSecond,
                    truth: filterTruth,
                },
                slider: {
                    immediate: sliderImmediate,
                    afterFrame: sliderAfterFrame,
                    afterSecond: sliderAfterSecond,
                    display: displayAfterCommit,
                    displayMutations,
                    afterStaleGeneration,
                    afterInactiveScale,
                },
            };
        });

        expect(result.chips.before).toContain('toggle-b-field');
        expect(result.chips.after).not.toContain('toggle-b-field');
        expect(result.chips.retainedIdentity).toBe(true);
        expect(result.chips.mutations).toEqual({ records: 1, added: 0, removed: 1 });
        expect(result.filter.immediate).toBe(0);
        expect(result.filter.afterFrame).toBeGreaterThan(0);
        expect(result.filter.afterSecond).toBe(result.filter.afterFrame);
        expect(result.filter.truth).toEqual({ value: 'vortic', vorticityVisible: true, volumeHidden: true });
        expect(result.slider).toEqual({
            immediate: 0, afterFrame: 1, afterSecond: 1, display: '0.73',
            displayMutations: { records: 1, characterData: 1, added: 0, removed: 0 },
            afterStaleGeneration: { calls: 1, display: '0.73' },
            afterInactiveScale: { calls: 1, display: '0.73' },
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('style, axis, volume-style, and column-clear controls preserve exact truth', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const state = getScale0State();
            const click = (selector) => document.querySelector(selector).click();

            click('#force-style-row [data-style="glyphs"]');
            const forceStyle = {
                state: state.forceStyle,
                active: panelValues('#force-style-row .style-btn', 'style'),
            };
            click('#scalar-render-row [data-scalar-mode="heatmap"]');
            const scalarStyle = {
                state: state.scalarRenderMode,
                active: panelValues('#scalar-render-row .style-btn', 'scalarMode'),
            };

            const axesBefore = { ...ctx.viewport._fieldRenderer._fluxSliceAxes };
            click('#flux-slice-axis-xy');
            click('#flux-slice-axis-yz');
            const axesAfter = { ...ctx.viewport._fieldRenderer._fluxSliceAxes };
            const flux = ctx.viewport._fluxRenderer;
            const organicBefore = flux._fluxOrganic;
            const glowBefore = flux._fluxGlow;
            click('#toggle-flux-organic');
            click('#toggle-flux-glow');
            const fluxStylesAfter = { organic: flux._fluxOrganic, glow: flux._fluxGlow };

            for (const id of ['toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field']) {
                const button = document.getElementById(id);
                if (!button.classList.contains('is-inapplicable') && !button.classList.contains('active')) button.click();
            }
            state.latticeNeedsUpload = false;
            click('.s0-overlay-col-clear[data-clear-col="volume"]');
            const clearImmediate = {
                active: ['toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field', 'toggle-state-field']
                    .filter((id) => !document.getElementById(id).classList.contains('is-inapplicable'))
                    .filter((id) => document.getElementById(id).classList.contains('active')),
                fluxVolume: ctx.viewport._fluxRenderer.showFlux,
                fluxSlice: ctx.viewport._fieldRenderer.showHeatmap,
                showFluxLines: state.fieldFlags.showFluxLines,
                showDivField: state.fieldFlags.showDivField,
                upload: state.latticeNeedsUpload,
            };
            await new Promise((resolve) => requestAnimationFrame(resolve));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const clearShell = {
                badge: document.querySelector('[data-count-for="volume"]').textContent,
                chipIds: [...document.querySelectorAll('#s0-overlay-active .s0-overlay-chip')]
                    .map((chip) => chip.dataset.overlayId),
            };
            state.latticeNeedsUpload = false;
            click('.s0-overlay-col-clear[data-clear-col="volume"]');
            const emptyClearUpload = state.latticeNeedsUpload;

            function panelValues(selector, dataKey) {
                return [...document.querySelectorAll(selector)]
                    .filter((button) => button.classList.contains('active'))
                    .map((button) => button.dataset[dataKey]);
            }
            return {
                forceStyle,
                scalarStyle,
                axesBefore,
                axesAfter,
                fluxStyles: { before: { organic: organicBefore, glow: glowBefore }, after: fluxStylesAfter },
                clearImmediate,
                clearShell,
                emptyClearUpload,
            };
        });

        expect(result.forceStyle).toEqual({ state: 'glyphs', active: ['glyphs'] });
        expect(result.scalarStyle).toEqual({ state: 'heatmap', active: ['heatmap'] });
        expect(result.axesAfter[2]).toBe(!result.axesBefore[2]);
        expect(result.axesAfter[0]).toBe(!result.axesBefore[0]);
        expect(result.axesAfter[1]).toBe(result.axesBefore[1]);
        expect(result.fluxStyles.after).toEqual({
            organic: !result.fluxStyles.before.organic,
            glow: !result.fluxStyles.before.glow,
        });
        expect(result.clearImmediate).toEqual({
            active: [], fluxVolume: false, fluxSlice: false,
            showFluxLines: false, showDivField: false, upload: true,
        });
        expect(result.clearShell.badge).toBe('0');
        for (const id of ['toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field']) {
            expect(result.clearShell.chipIds).not.toContain(id);
        }
        expect(result.emptyClearUpload).toBe(false);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('streamline work is single-frame-budgeted and sampler ownership never oscillates mid-sweep', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const source = await page.evaluate(async () => (
            await (await fetch('/js/scales/scale0/runtime/field-overlays.js')).text()
        ));
        const profiles = await page.evaluate(async () => {
            const { computeStreamlineParams } = await import(
                '/js/scales/scale0/runtime/streamline-integrator.js'
            );
            return {
                worker: computeStreamlineParams(33),
                inThread: computeStreamlineParams(33, { inThreadWasm: true }),
            };
        });
        const budget = Number(source.match(/const OVERLAY_FRAME_BUDGET = (\d+);/)?.[1]);
        const streamlineCost = Number(source.match(/const COST_STREAMLINE = (\d+);/)?.[1]);

        await page.evaluate(() => {
            const worker = window.__ftdCtx.fluxMock?._worker;
            if (!worker) throw new Error('Scale 0 worker is unavailable');
            for (const id of ['toggle-e-field', 'toggle-b-field', 'toggle-poynting']) {
                const button = document.getElementById(id);
                if (button?.classList.contains('active')) button.click();
            }
            window.__gate10SamplerTrace = [];
            window.__gate10OriginalPostMessage = worker.postMessage;
            worker.postMessage = function (...args) {
                const message = args[0];
                if ((message?.type === 'wantSampler' || message?.type === 'unwantSampler')
                    && ['e', 'b', 'poynting'].includes(message.kind)) {
                    window.__gate10SamplerTrace.push({ type: message.type, kind: message.kind, stride: message.stride });
                }
                return window.__gate10OriginalPostMessage.apply(this, args);
            };
        });
        await page.waitForTimeout(500);
        await page.evaluate(() => {
            window.__gate10SamplerTrace.length = 0;
            for (const id of ['toggle-e-field', 'toggle-b-field', 'toggle-poynting']) {
                document.getElementById(id)?.click();
            }
        });
        await expect.poll(
            () => page.evaluate(() => new Set(
                window.__gate10SamplerTrace
                    .filter((message) => message.type === 'wantSampler')
                    .map((message) => message.kind),
            ).size),
            { timeout: 10_000 },
        ).toBe(3);
        await page.waitForTimeout(750);
        const trace = await page.evaluate(() => {
            const worker = window.__ftdCtx.fluxMock?._worker;
            if (worker && window.__gate10OriginalPostMessage) {
                worker.postMessage = window.__gate10OriginalPostMessage;
            }
            const messages = [...window.__gate10SamplerTrace];
            delete window.__gate10SamplerTrace;
            delete window.__gate10OriginalPostMessage;
            return messages;
        });

        expect(budget).toBe(50);
        expect(streamlineCost).toBe(budget);
        expect(profiles.worker).toMatchObject({ maxSeeds: 36, maxLines: 36, maxSteps: 99 });
        expect(profiles.inThread).toMatchObject({ maxSeeds: 16, maxLines: 16, maxSteps: 99 });
        expect(trace.filter((message) => message.type === 'unwantSampler')).toEqual([]);
        expect(trace.filter((message) => message.type === 'wantSampler').map((message) => message.kind).sort())
            .toEqual(['b', 'e', 'poynting']);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
