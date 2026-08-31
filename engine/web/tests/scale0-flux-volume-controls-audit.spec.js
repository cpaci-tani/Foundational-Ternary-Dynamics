// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 Flux Volume controls-card audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
    });

    test('inventory and displayed values match both volume and slice renderer truth', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(() => {
            const ids = [
                'flux-shape-select', 'flux-opacity', 'flux-opacity-val',
                'flux-point-scale', 'flux-point-scale-val',
                'flux-scenario-scale', 'flux-scenario-scale-val',
                'flux-threshold', 'flux-threshold-val',
                'flux-lattice-spacing', 'flux-lattice-spacing-val',
                'wireframe-brightness', 'wireframe-brightness-val',
            ];
            const ctx = window.__ftdCtx;
            const volume = ctx.viewport._fluxRenderer;
            const slice = ctx.viewport._fieldRenderer;
            const card = document.getElementById('flux-shape-select')?.closest('.card');
            const value = (id) => Number(document.getElementById(id)?.value);
            return {
                missing: ids.filter((id) => !document.getElementById(id)),
                duplicates: ids.filter((id) => document.querySelectorAll(`#${id}`).length !== 1),
                title: card?.querySelector('.card-title')?.textContent,
                values: {
                    shape: value('flux-shape-select'),
                    opacity: value('flux-opacity'),
                    pointScale: value('flux-point-scale'),
                    scenarioScale: value('flux-scenario-scale'),
                    threshold: value('flux-threshold'),
                    spacing: value('flux-lattice-spacing'),
                    brightness: value('wireframe-brightness'),
                },
                thresholdRange: {
                    min: Number(document.getElementById('flux-threshold')?.min),
                    max: Number(document.getElementById('flux-threshold')?.max),
                    step: Number(document.getElementById('flux-threshold')?.step),
                    ariaLabel: document.getElementById('flux-threshold')?.getAttribute('aria-label'),
                },
                displays: {
                    opacity: document.getElementById('flux-opacity-val')?.textContent,
                    pointScale: document.getElementById('flux-point-scale-val')?.textContent,
                    scenarioScale: document.getElementById('flux-scenario-scale-val')?.textContent,
                    threshold: document.getElementById('flux-threshold-val')?.textContent,
                    spacing: document.getElementById('flux-lattice-spacing-val')?.textContent,
                    brightness: document.getElementById('wireframe-brightness-val')?.textContent,
                },
                renderer: {
                    volumeShape: volume._fluxShape,
                    sliceShape: slice._fluxSliceShape,
                    volumeOpacity: volume._fluxOpacity,
                    sliceOpacity: slice._fluxSliceOpacity,
                    pointScale: volume._fluxPointScale,
                    slicePointScale: slice._fluxSlicePointScale,
                    threshold: volume._fluxThreshold,
                    sliceThreshold: slice._fluxSliceThreshold,
                    scenarioScale: volume._scenarioScale,
                    sceneScale: ctx.viewport.scene.scale.x,
                    spacing: volume._fluxLatticeSpacing,
                    volumeSpacing: volume._fluxVolume?.scale.x,
                    brightness: ctx.viewport._sceneCore._wireframeBrightness,
                },
                nodes: card?.querySelectorAll('*').length,
                inputs: card?.querySelectorAll('input,select').length,
            };
        });

        expect(result.missing).toEqual([]);
        expect(result.duplicates).toEqual([]);
        expect(result.title).toBe('Flux Volume');
        expect(result.nodes).toBeGreaterThan(20);
        expect(result.inputs).toBe(7);
        expect(result.thresholdRange).toEqual({
            min: 0,
            max: 0.5,
            step: 0.0001,
            ariaLabel: 'Relative flux threshold',
        });
        expect(result.renderer).toEqual({
            volumeShape: result.values.shape,
            sliceShape: result.values.shape,
            volumeOpacity: result.values.opacity,
            sliceOpacity: result.values.opacity,
            pointScale: result.values.pointScale,
            slicePointScale: result.values.pointScale,
            threshold: result.values.threshold,
            sliceThreshold: result.values.threshold,
            scenarioScale: result.values.scenarioScale,
            sceneScale: result.values.scenarioScale,
            spacing: result.values.spacing,
            volumeSpacing: result.values.spacing,
            brightness: result.values.brightness,
        });
        expect(result.displays).toEqual({
            opacity: result.values.opacity.toFixed(2),
            pointScale: result.values.pointScale.toFixed(1),
            scenarioScale: result.values.scenarioScale.toFixed(1),
            threshold: result.values.threshold < 0.001
                ? result.values.threshold.toFixed(4)
                : result.values.threshold.toFixed(3),
            spacing: result.values.spacing.toFixed(2),
            brightness: result.values.brightness.toFixed(2),
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('six hundred input events collapse to one latest-value frame transaction', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.goto('/js/scales/scale0/ui/controls/flux-volume.js', { waitUntil: 'domcontentloaded' });
        const result = await page.evaluate(async () => {
            const { createFluxVolumeCard } = await import(
                '/js/scales/scale0/ui/controls/flux-volume.js?gate7-batch-card=1'
            );
            const { wireScale0Controls } = await import(
                '/js/scales/scale0/ui/controls/wire.js?gate7-batch-wire=1'
            );
            const host = document.createElement('div');
            host.id = 'panel-controls';
            const grid = document.createElement('div');
            grid.id = 'panel-controls-grid';
            grid.append(createFluxVolumeCard());
            host.append(grid);
            document.body.append(host);
            const calls = [];
            let uploads = 0;
            const viewport = {
                setFluxShape: (v) => calls.push(['shape', v]),
                setFluxSliceShape: (v) => calls.push(['sliceShape', v]),
                setFluxOpacity: (v) => calls.push(['opacity', v]),
                setFluxSliceOpacity: (v) => calls.push(['sliceOpacity', v]),
                setFluxPointScale: (v) => calls.push(['point', v]),
                setFluxSlicePointScale: (v) => calls.push(['slicePoint', v]),
                setFluxThreshold: (v) => calls.push(['threshold', v]),
                setFluxSliceThreshold: (v) => calls.push(['sliceThreshold', v]),
                setScenarioScale: (v) => calls.push(['scenarioScale', v]),
                setFluxLatticeSpacing: (v) => calls.push(['spacing', v]),
                setWireframeBrightness: (v) => calls.push(['brightness', v]),
            };
            const ctx = { bridge: { latticeSize: 33 }, viewport, _loadGeneration: 1 };
            wireScale0Controls(ctx, { setLatticeNeedsUpload: () => { uploads++; } });
            const card = document.getElementById('flux-shape-select').closest('.card');
            const before = { nodes: card.querySelectorAll('*').length, inputs: card.querySelectorAll('input,select').length };
            const mutations = { records: 0, characterData: 0, added: 0, removed: 0 };
            const mutationObserver = new MutationObserver((list) => {
                for (const mutation of list) {
                    mutations.records++;
                    if (mutation.type === 'characterData') mutations.characterData++;
                    mutations.added += mutation.addedNodes?.length || 0;
                    mutations.removed += mutation.removedNodes?.length || 0;
                }
            });
            mutationObserver.observe(card, {
                subtree: true, childList: true, attributes: true, characterData: true,
            });
            const shape = document.getElementById('flux-shape-select');
            shape.value = '4';
            shape.dispatchEvent(new Event('change', { bubbles: true }));
            const finalValues = {
                'flux-opacity': 0.42,
                'flux-point-scale': 2.3,
                'flux-threshold': 0.0077,
                'flux-scenario-scale': 1.7,
                'flux-lattice-spacing': 1.55,
                'wireframe-brightness': 0.33,
            };
            for (let i = 0; i < 100; i++) {
                for (const [id, value] of Object.entries(finalValues)) {
                    const input = document.getElementById(id);
                    input.value = String(value);
                    input.dispatchEvent(new Event('input', { bubbles: true }));
                }
            }
            const immediate = { calls: [...calls], uploads };
            await new Promise((resolve) => requestAnimationFrame(() => resolve()));
            const afterFrame = { calls: [...calls], uploads };
            await new Promise((resolve) => requestAnimationFrame(() => resolve()));
            const afterSecondFrame = { calls: [...calls], uploads };

            // A load that wins before the next frame invalidates the pending
            // user transaction; loader-owned values must not be overwritten.
            calls.length = 0;
            const point = document.getElementById('flux-point-scale');
            point.value = '1.9';
            point.dispatchEvent(new Event('input', { bubbles: true }));
            ctx._loadGeneration = 2;
            point.value = '2.6';
            await new Promise((resolve) => requestAnimationFrame(() => resolve()));
            const staleCalls = [...calls];
            mutationObserver.disconnect();
            return {
                before,
                after: { nodes: card.querySelectorAll('*').length, inputs: card.querySelectorAll('input,select').length },
                immediate,
                afterFrame,
                afterSecondFrame,
                staleCalls,
                mutations,
                uploads,
                displays: {
                    opacity: document.getElementById('flux-opacity-val').textContent,
                    point: document.getElementById('flux-point-scale-val').textContent,
                    threshold: document.getElementById('flux-threshold-val').textContent,
                    scenario: document.getElementById('flux-scenario-scale-val').textContent,
                    spacing: document.getElementById('flux-lattice-spacing-val').textContent,
                    brightness: document.getElementById('wireframe-brightness-val').textContent,
                },
            };
        });

        expect(result.immediate).toEqual({
            calls: [['shape', 4], ['sliceShape', 4]],
            uploads: 0,
        });
        expect(result.afterFrame.calls).toEqual([
            ['shape', 4], ['sliceShape', 4],
            ['opacity', 0.42], ['sliceOpacity', 0.42],
            ['point', 2.3], ['slicePoint', 2.3],
            ['threshold', 0.0077], ['sliceThreshold', 0.0077],
            ['scenarioScale', 1.7], ['spacing', 1.55], ['brightness', 0.33],
        ]);
        expect(result.afterFrame.uploads).toBe(1);
        expect(result.afterSecondFrame).toEqual(result.afterFrame);
        expect(result.staleCalls).toEqual([]);
        expect(result.mutations).toEqual({
            records: 6,
            characterData: 6,
            added: 0,
            removed: 0,
        });
        expect(result.uploads).toBe(1);
        expect(result.displays).toEqual({
            opacity: '0.42', point: '2.3', threshold: '0.008',
            scenario: '1.7', spacing: '1.55', brightness: '0.33',
        });
        expect(result.after).toEqual(result.before);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('slice opacity and shape survive a lattice resize mesh rebuild', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(async () => {
            const sliceButton = document.getElementById('toggle-flux-slice');
            if (!sliceButton.classList.contains('active')) sliceButton.click();
            const opacity = document.getElementById('flux-opacity');
            opacity.value = '0.42';
            opacity.dispatchEvent(new Event('input', { bubbles: true }));
            const shape = document.getElementById('flux-shape-select');
            shape.value = '4';
            shape.dispatchEvent(new Event('change', { bubbles: true }));
            await new Promise((resolve) => requestAnimationFrame(() => resolve()));
            window.__gate7OldSliceMesh = window.__ftdCtx.viewport._fieldRenderer._fluxSliceMesh;
            const controller = await import('/js/scales/scale0/controller.js?v=13');
            await controller.resize(window.__ftdCtx, 9);
        });
        await page.waitForFunction(() => {
            const ctx = window.__ftdCtx;
            return ctx?.fluxMock?.ready === true
                && ctx.fluxMock.latticeSize === 9
                && ctx.viewport?._fieldRenderer?._fluxSliceMeshSize === 9;
        });
        const result = await page.evaluate(() => {
            const ctx = window.__ftdCtx;
            const renderer = ctx.viewport._fieldRenderer;
            const mesh = renderer._fluxSliceMesh;
            return {
                rebuilt: mesh !== window.__gate7OldSliceMesh,
                opacityState: renderer._fluxSliceOpacity,
                opacityUniform: mesh.material.uniforms.uOpacity.value,
                shapeState: renderer._fluxSliceShape,
                shapeUniform: mesh.material.uniforms.shapeType.value,
                uiOpacity: Number(document.getElementById('flux-opacity').value),
                uiShape: Number(document.getElementById('flux-shape-select').value),
            };
        });

        expect(result).toEqual({
            rebuilt: true,
            opacityState: 0.42,
            opacityUniform: 0.42,
            shapeState: 4,
            shapeUniform: 4,
            uiOpacity: 0.42,
            uiShape: 4,
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
