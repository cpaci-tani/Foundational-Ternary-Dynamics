// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
    switchMode,
} from './_helpers.js';

test.describe('Scale 0 Particle Display controls-card audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
    });

    test('inventory matches renderer truth and sign sizes survive live and empty frames', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const ids = [
                'particle-shape-select',
                'particle-pos-size', 'particle-pos-size-val',
                'particle-neg-size', 'particle-neg-size-val',
                'particle-opacity', 'particle-opacity-val',
                'particle-glow', 'particle-glow-val',
            ];
            const ctx = window.__ftdCtx;
            const viewport = ctx.viewport;
            const renderer = viewport._particleRenderer;
            const card = document.getElementById('particle-shape-select')?.closest('.card');
            const uniforms = renderer.particles.material.uniforms;
            const values = {
                shape: Number(document.getElementById('particle-shape-select')?.value),
                positive: Number(document.getElementById('particle-pos-size')?.value),
                negative: Number(document.getElementById('particle-neg-size')?.value),
                opacity: Number(document.getElementById('particle-opacity')?.value),
                glow: Number(document.getElementById('particle-glow')?.value),
            };
            const displays = {
                positive: document.getElementById('particle-pos-size-val')?.textContent,
                negative: document.getElementById('particle-neg-size-val')?.textContent,
                opacity: document.getElementById('particle-opacity-val')?.textContent,
                glow: document.getElementById('particle-glow-val')?.textContent,
            };
            const data = {
                positions: new Float32Array([10, 10, 10, 12, 12, 12]),
                colors: new Float32Array([0.29, 0.87, 0.50, 0.97, 0.44, 0.44]),
                sizes: new Float32Array([6, 6]),
                colorCharge: new Float32Array([1, 2]),
                count: 2,
            };
            const empty = {
                positions: new Float32Array(0),
                colors: new Float32Array(0),
                sizes: new Float32Array(0),
                colorCharge: new Float32Array(0),
                count: 0,
            };
            let directRenders = 0;
            const originalRender = viewport.render;
            viewport.render = () => { directRenders++; };
            let evidence;
            try {
                viewport.toggleColorChargeRender(true);
                viewport.setPositiveSize(20);
                viewport.setNegativeSize(8);
                viewport.updateParticles(data);
                const sizeAttr = renderer.particles.geometry.getAttribute('size');
                const firstFrame = Array.from(sizeAttr.array.slice(0, 2));
                viewport.updateParticles(data);
                const secondFrame = Array.from(sizeAttr.array.slice(0, 2));
                viewport.updateParticles(empty);
                const emptyVersion = sizeAttr.version;
                viewport.setPositiveSize(22);
                const emptySetVersion = sizeAttr.version;
                viewport.updateParticles(data);
                const restoredFrame = Array.from(sizeAttr.array.slice(0, 2));
                const liveVersion = sizeAttr.version;
                viewport.setPositiveSize(22);
                const idempotentVersion = sizeAttr.version;
                evidence = {
                    firstFrame,
                    secondFrame,
                    restoredFrame,
                    emptyVersion,
                    emptySetVersion,
                    liveVersion,
                    idempotentVersion,
                };
            } finally {
                viewport.render = originalRender;
                viewport.toggleColorChargeRender(false);
                const { syncScale0ParticleDisplay } = await import(
                    '/js/scales/scale0/ui/controls/wire.js?v=11'
                );
                syncScale0ParticleDisplay(ctx);
            }
            return {
                missing: ids.filter((id) => !document.getElementById(id)),
                duplicates: ids.filter((id) => document.querySelectorAll(`#${id}`).length !== 1),
                title: card?.querySelector('.card-title')?.textContent,
                nodes: card?.querySelectorAll('*').length,
                inputs: card?.querySelectorAll('input,select').length,
                values,
                displays,
                renderer: {
                    shape: uniforms.shapeType.value,
                    positive: viewport.visualSettings.positiveSize,
                    negative: viewport.visualSettings.negativeSize,
                    opacity: uniforms.uOpacity.value,
                    glow: uniforms.uGlow.value,
                },
                evidence,
                directRenders,
            };
        });

        expect(result.missing).toEqual([]);
        expect(result.duplicates).toEqual([]);
        expect(result.title).toBe('Particle Display');
        expect(result.nodes).toBeGreaterThan(15);
        expect(result.inputs).toBe(5);
        expect(result.displays).toEqual({
            positive: result.values.positive.toFixed(1),
            negative: result.values.negative.toFixed(1),
            opacity: result.values.opacity.toFixed(2),
            glow: result.values.glow.toFixed(2),
        });
        expect(result.renderer).toEqual(result.values);
        expect(result.evidence.firstFrame).toEqual([20, 8]);
        expect(result.evidence.secondFrame).toEqual([20, 8]);
        expect(result.evidence.restoredFrame).toEqual([22, 8]);
        expect(result.evidence.emptySetVersion).toBe(result.evidence.emptyVersion);
        expect(result.evidence.idempotentVersion).toBe(result.evidence.liveVersion);
        expect(result.directRenders).toBe(0);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('four hundred input events collapse to one stable latest-value transaction', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.goto('/js/scales/scale0/ui/controls/flux-volume.js', { waitUntil: 'domcontentloaded' });
        const result = await page.evaluate(async () => {
            const { createParticleDisplayCard } = await import(
                '/js/scales/scale0/ui/controls/flux-volume.js?gate8-card=1'
            );
            const { syncScale0ParticleDisplay, wireScale0Controls } = await import(
                '/js/scales/scale0/ui/controls/wire.js?gate8-wire=1'
            );
            const host = document.createElement('div');
            host.id = 'panel-controls';
            const grid = document.createElement('div');
            grid.id = 'panel-controls-grid';
            grid.append(createParticleDisplayCard());
            host.append(grid);
            document.body.append(host);
            const calls = [];
            const viewport = {
                setParticleShape: (v) => calls.push(['shape', v]),
                setParticleSizes: (p, n) => calls.push(['sizes', p, n]),
                setParticleOpacity: (v) => calls.push(['opacity', v]),
                setParticleGlow: (v) => calls.push(['glow', v]),
            };
            const ctx = { viewport, engineMode: 'lattice', _loadGeneration: 1 };
            wireScale0Controls(ctx, { setLatticeNeedsUpload: () => {} });
            calls.length = 0; // discard the intentional initial truth sync
            const card = document.getElementById('particle-shape-select').closest('.card');
            const before = {
                nodes: card.querySelectorAll('*').length,
                inputs: card.querySelectorAll('input,select').length,
            };
            const mutations = { records: 0, characterData: 0, added: 0, removed: 0 };
            const observer = new MutationObserver((list) => {
                for (const mutation of list) {
                    mutations.records++;
                    if (mutation.type === 'characterData') mutations.characterData++;
                    mutations.added += mutation.addedNodes?.length || 0;
                    mutations.removed += mutation.removedNodes?.length || 0;
                }
            });
            observer.observe(card, {
                subtree: true, childList: true, attributes: true, characterData: true,
            });
            const shape = document.getElementById('particle-shape-select');
            shape.value = '4';
            shape.dispatchEvent(new Event('change', { bubbles: true }));
            const finalValues = {
                'particle-pos-size': 20,
                'particle-neg-size': 8,
                'particle-opacity': 0.7,
                'particle-glow': 0.3,
            };
            let inputEvents = 0;
            for (let i = 0; i < 100; i++) {
                for (const [id, value] of Object.entries(finalValues)) {
                    const input = document.getElementById(id);
                    input.value = String(value);
                    input.dispatchEvent(new Event('input', { bubbles: true }));
                    inputEvents++;
                }
            }
            const immediate = [...calls];
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterFrame = [...calls];
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterSecondFrame = [...calls];
            observer.disconnect();

            calls.length = 0;
            const positive = document.getElementById('particle-pos-size');
            positive.value = '22';
            positive.dispatchEvent(new Event('input', { bubbles: true }));
            ctx.engineMode = 'particles';
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const staleScaleCalls = [...calls];
            const staleScaleDisplay = document.getElementById('particle-pos-size-val').textContent;
            ctx.engineMode = 'lattice';
            syncScale0ParticleDisplay(ctx);
            const reentryCalls = [...calls];
            return {
                before,
                after: {
                    nodes: card.querySelectorAll('*').length,
                    inputs: card.querySelectorAll('input,select').length,
                },
                inputEvents,
                immediate,
                afterFrame,
                afterSecondFrame,
                mutations,
                staleScaleCalls,
                staleScaleDisplay,
                reentryCalls,
                reentryDisplay: document.getElementById('particle-pos-size-val').textContent,
            };
        });

        expect(result.inputEvents).toBe(400);
        expect(result.immediate).toEqual([['shape', 4]]);
        expect(result.afterFrame).toEqual([
            ['shape', 4],
            ['opacity', 0.7], ['glow', 0.3], ['sizes', 20, 8],
        ]);
        expect(result.afterSecondFrame).toEqual(result.afterFrame);
        expect(result.mutations).toEqual({
            records: 4, characterData: 4, added: 0, removed: 0,
        });
        expect(result.staleScaleCalls).toEqual([]);
        expect(result.staleScaleDisplay).toBe('20.0');
        expect(result.reentryCalls).toEqual([
            ['shape', 4],
            ['opacity', 0.7], ['glow', 0.3], ['sizes', 22, 8],
        ]);
        expect(result.reentryDisplay).toBe('22.0');
        expect(result.after).toEqual(result.before);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('Scale 0 presentation survives a scenario change and Scale 1 round trip', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(async () => {
            const set = (id, value, type = 'input') => {
                const input = document.getElementById(id);
                input.value = String(value);
                input.dispatchEvent(new Event(type, { bubbles: true }));
            };
            set('particle-shape-select', 3, 'change');
            set('particle-pos-size', 18);
            set('particle-neg-size', 9);
            set('particle-opacity', 0.71);
            set('particle-glow', 0.22);
            await new Promise((resolve) => requestAnimationFrame(resolve));
        });
        const readState = () => page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const uniforms = viewport._particleRenderer.particles.material.uniforms;
            return {
                mode: window.__ftdCtx.engineMode,
                dom: {
                    shape: Number(document.getElementById('particle-shape-select').value),
                    positive: Number(document.getElementById('particle-pos-size').value),
                    negative: Number(document.getElementById('particle-neg-size').value),
                    opacity: Number(document.getElementById('particle-opacity').value),
                    glow: Number(document.getElementById('particle-glow').value),
                },
                renderer: {
                    shape: uniforms.shapeType.value,
                    positive: viewport.visualSettings.positiveSize,
                    negative: viewport.visualSettings.negativeSize,
                    opacity: uniforms.uOpacity.value,
                    glow: uniforms.uGlow.value,
                },
            };
        });
        const before = await readState();
        await selectScale0Scenario(page, 's0-vacuum-electron');
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
        const afterScenario = await readState();
        await switchMode(page, 'particles');
        await page.waitForTimeout(500);
        const inScale1 = await readState();
        await switchMode(page, 'lattice');
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
        const afterReentry = await readState();

        const expected = {
            shape: 3, positive: 18, negative: 9, opacity: 0.71, glow: 0.22,
        };
        expect(before).toEqual({ mode: 'lattice', dom: expected, renderer: expected });
        expect(afterScenario).toEqual({ mode: 'lattice', dom: expected, renderer: expected });
        expect(inScale1.mode).toBe('particles');
        expect(inScale1.renderer.shape).toBe(0);
        expect(inScale1.renderer.glow).toBeCloseTo(0.28, 12);
        expect(afterReentry).toEqual({ mode: 'lattice', dom: expected, renderer: expected });
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
