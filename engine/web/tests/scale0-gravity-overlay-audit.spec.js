// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

const LATTICE_SIZES = [9, 17, 25, 33, 49, 65, 97, 113, 145, 181];

test.describe('Scale 0 discrete gravity overlay audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
    });

    test('potential labels and values follow the selected finite operators', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const { computeGravPotentialFrame } = await import(
                '/js/scales/scale0/runtime/overlay-frames.js'
            );
            const positions = new Float32Array([4.5, 4.5, 4.5]);
            const local = computeGravPotentialFrame(
                { bridge: { getToggle: () => false } },
                {
                    poissonLatency: {
                        positions, values: new Float32Array([2]), count: 1,
                        effectiveStride: 2, origin: 0,
                    },
                    fluxVector: {
                        positions, vectors: new Float32Array([3, 4, 0]), count: 1,
                        effectiveStride: 1, origin: 0,
                    },
                },
                { useFluxMock: false },
            );
            const localSnapshot = {
                value: Number(local.values[0].toFixed(6)), normalizer: local.normalizer,
                source: local.source, operator: local.operator,
                effectiveStride: local.effectiveStride, origin: local.origin,
            };
            const poisson = computeGravPotentialFrame(
                { bridge: { getToggle: () => true } },
                {
                    poissonLatency: {
                        positions, values: new Float32Array([2]), count: 1,
                        effectiveStride: 2, origin: 1,
                    },
                    fluxVector: {
                        positions, vectors: new Float32Array([30, 40, 0]), count: 1,
                    },
                },
                { useFluxMock: false },
            );
            return {
                local: localSnapshot,
                poisson: {
                    value: poisson.values[0], normalizer: poisson.normalizer,
                    source: poisson.source, operator: poisson.operator,
                    effectiveStride: poisson.effectiveStride, origin: poisson.origin,
                },
            };
        });

        expect(result.local).toEqual({
            value: -0.05,
            normalizer: 0.05,
            source: 'local-density-gradient',
            operator: 'phi=-G_N|J|',
            effectiveStride: 1,
            origin: 0,
        });
        expect(result.poisson).toEqual({
            value: -4,
            normalizer: 4,
            source: 'poisson-latency',
            operator: 'phi=-L^2',
            effectiveStride: 2,
            origin: 1,
        });
    });

    test('gravity renderers are drawable-aware, per-force, batched, and disposable', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const THREE = await import('three');
            const { ViewportFieldRenderer } = await import('/js/viewport/field-renderer.js?v=gravity-audit');
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
            const vectors = {
                count: 2,
                positions: new Float32Array([8, 8, 8, 10, 8, 8]),
                vectors: new Float32Array([1, 0, 0, 0.5, 0, 0]),
            };
            const lines = {
                count: 1,
                buffer: new Float32Array([8, 8, 8, 9, 8, 8, 10, 8, 8]),
                offsets: new Uint32Array([0]),
                lengths: new Uint32Array([9]),
            };

            field.showGravityForce(true);
            const emptyArrow = {
                visible: field._gravityField.visible,
                draw: field._gravityField.geometry.drawRange.count,
            };
            field.updateGravityField(vectors);
            const fullArrow = {
                visible: field._gravityField.visible,
                draw: field._gravityField.geometry.drawRange.count,
            };
            field.updateGravityField(null);
            const clearedArrow = {
                visible: field._gravityField.visible,
                draw: field._gravityField.geometry.drawRange.count,
            };

            field.showForceHeatmap({ em: true, gravity: true, strong: false, weak: false });
            field.updateForceHeatmap(vectors, 'gravity');
            field.updateForceHeatmap(vectors, 'em');
            const heatmaps = {
                distinct: field._forceHeatmaps.gravity !== field._forceHeatmaps.em,
                typed: Object.keys(field._forceHeatmaps).sort(),
                visible: [field._forceHeatmaps.gravity.visible, field._forceHeatmaps.em.visible],
                draws: [
                    field._forceHeatmaps.gravity.geometry.drawRange.count,
                    field._forceHeatmaps.em.geometry.drawRange.count,
                ],
            };

            field.showForceStreamlines_vis({ em: true, gravity: true, strong: false, weak: false });
            field.updateForceStreamlines(lines, 'gravity');
            field.updateForceStreamlines(lines, 'em');
            const flows = {
                distinct: field._forceStreamlineMeshes.gravity.mesh
                    !== field._forceStreamlineMeshes.em.mesh,
                typed: Object.keys(field._forceStreamlineMeshes).sort(),
                lineSegments: Object.values(field._forceStreamlineMeshes)
                    .every((entry) => entry.mesh.isLineSegments),
                drawObjects: Object.values(field._forceStreamlineMeshes).length,
                visible: [
                    field._forceStreamlineMeshes.gravity.mesh.visible,
                    field._forceStreamlineMeshes.em.mesh.visible,
                ],
                draws: [
                    field._forceStreamlineMeshes.gravity.mesh.geometry.drawRange.count,
                    field._forceStreamlineMeshes.em.mesh.geometry.drawRange.count,
                ],
                capacity: field._forceStreamlineMeshes.gravity.mesh.geometry
                    .getAttribute('position').count,
            };

            field.clearForceVisualization('gravity', 'heatmap');
            field.clearForceVisualization('gravity', 'flow');
            const isolatedClear = {
                gravityHeat: field._forceHeatmaps.gravity.geometry.drawRange.count,
                emHeat: field._forceHeatmaps.em.geometry.drawRange.count,
                gravityFlow: field._forceStreamlineMeshes.gravity.mesh.geometry.drawRange.count,
                emFlow: field._forceStreamlineMeshes.em.mesh.geometry.drawRange.count,
            };
            field.dispose();
            return { emptyArrow, fullArrow, clearedArrow, heatmaps, flows, isolatedClear, remaining: scene.children.length };
        });

        expect(result.emptyArrow).toEqual({ visible: false, draw: 0 });
        expect(result.fullArrow).toEqual({ visible: true, draw: 4 });
        expect(result.clearedArrow).toEqual({ visible: false, draw: 0 });
        expect(result.heatmaps).toEqual({
            distinct: true, typed: ['em', 'gravity'], visible: [true, true], draws: [2, 2],
        });
        expect(result.flows).toEqual({
            distinct: true, typed: ['em', 'gravity'], lineSegments: true,
            drawObjects: 2, visible: [true, true], draws: [4, 4], capacity: 16000,
        });
        expect(result.isolatedClear).toEqual({
            gravityHeat: 0, emHeat: 2, gravityFlow: 0, emFlow: 4,
        });
        expect(result.remaining).toBe(0);
    });

    test('every lattice size keeps gravity sampling and flow work bounded', async ({ page }) => {
        const result = await page.evaluate(async (sizes) => {
            const { buildForceOverlayData } = await import(
                '/js/scales/scale0/runtime/field-overlays.js?v=15'
            );
            const { computeStreamlineParams } = await import(
                '/js/scales/scale0/runtime/streamline-integrator.js'
            );
            return sizes.map((N) => {
                const params = computeStreamlineParams(N);
                let forceStride = 0;
                buildForceOverlayData({
                    fieldFlags: {
                        showForceEM: false, showForceGravity: true,
                        showForceStrong: false, showForceWeak: false,
                    },
                    forceStyle: 'arrows',
                }, {
                    getScale0ForceField(_type, stride) {
                        forceStride = stride;
                        return { count: 0 };
                    },
                }, {}, N, params.stride, params.stepsScale, params.seedSpacing, params);
                const flowSteps = Math.max(20, Math.ceil(params.maxSteps * 0.4));
                return {
                    N,
                    forceStride,
                    candidates: Math.ceil(N / forceStride) ** 3,
                    flowWork: params.maxSeeds * flowSteps,
                };
            });
        }, LATTICE_SIZES);

        expect(result.map((row) => row.forceStride)).toEqual([1, 1, 1, 1, 1, 2, 3, 3, 4, 4]);
        for (const row of result) {
            expect(row.candidates, `L=${row.N} gravity sample candidate budget`).toBeLessThan(120_000);
            expect(row.flowWork, `L=${row.N} gravity RK4 work budget`).toBeLessThanOrEqual(2_880);
        }
    });

    test('gravity flow is worker-backed and a style switch cannot repaint stale lines', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await selectScale0Scenario(page, 's0-seed-massive-body', { settleMs: 250 });
        await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const state = store.getScale0State();
            const active = store.getActiveScale0Bridge(window.__ftdCtx, state);
            active.setToggle('forces', true);
            active.setToggle('gravity', true);
            active.setToggle('latency_field', true);
            active.setToggle('geometric_gravity', true);
            active.tickOnce();
            store.setFieldToggle('showForceGravity', true);
            store.setForceStyle('flow');
            window.__ftdCtx.viewport.hideAllForceStyles();
            window.__ftdCtx.viewport.showForceStreamlines_vis({
                em: false, gravity: true, strong: false, weak: false,
            });
            state.fieldNeedsUpdate = true;
        });

        await expect.poll(() => page.evaluate(() => (
            window.__ftdCtx?.viewport?._fieldRenderer
                ?._forceStreamlineMeshes?.gravity?.mesh?.geometry?.drawRange?.count || 0
        )), { timeout: 15_000 }).toBeGreaterThan(0);

        const workerState = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const sched = getScale0State().overlaySched;
            return {
                hasClient: !!sched?.streamlineWorkerClient,
                kinds: sched?.jobs?.slice(0, sched.jobCount).map((job) => job.kind),
                flowDrawObjects: Object.keys(
                    window.__ftdCtx.viewport._fieldRenderer._forceStreamlineMeshes || {},
                ).length,
            };
        });
        expect(workerState.hasClient).toBe(true);
        expect(workerState.kinds).toContain(4);
        expect(workerState.kinds).toContain(5);
        expect(workerState.kinds.indexOf(4)).toBeLessThan(workerState.kinds.indexOf(5));
        expect(workerState.flowDrawObjects).toBe(1);

        const switched = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            store.setForceStyle('arrows');
            const state = store.getScale0State();
            const viewport = window.__ftdCtx.viewport;
            viewport.hideAllForceStyles();
            viewport.showArrowForces(state.fieldFlags);
            state.fieldNeedsUpdate = true;
            await new Promise((resolve) => requestAnimationFrame(resolve));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const flow = viewport._fieldRenderer._forceStreamlineMeshes.gravity;
            return { visible: flow.mesh.visible, draw: flow.mesh.geometry.drawRange.count };
        });
        expect(switched).toEqual({ visible: false, draw: 0 });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('warmed gravity surface plus every force style sustains the 60 FPS frame budget', async ({ page }, testInfo) => {
        testInfo.setTimeout(300_000);
        const consoleErrors = attachConsoleWatcher(page);
        await selectScale0Scenario(page, 's0-seed-massive-body', { settleMs: 250 });
        const requestedSizes = (process.env.FTD_GRAVITY_PERF_SIZES || '9,17,25,33,49,65,97')
            .split(',').map(Number).filter(Number.isFinite);
        const requestedStyles = (process.env.FTD_GRAVITY_PERF_STYLES || 'arrows,heatmap,flow,glyphs')
            .split(',').map((value) => value.trim()).filter(Boolean);
        const reports = await page.evaluate(async ({ sizes, styles }) => {
            const controller = await import('/js/scales/scale0/controller.js');
            const store = await import('/js/scales/scale0/state/store.js');
            const { createScale0ViewportAdapter } = await import('/js/scales/scale0/viewport-adapter.js');
            const probe = await import('/tests/scale0-ui-audit-probe.js?gravity=1');
            const ctx = window.__ftdCtx;
            const out = [];
            const waitFor = async (predicate, label, timeout = 20_000) => {
                const deadline = performance.now() + timeout;
                while (!predicate()) {
                    if (performance.now() >= deadline) {
                        const state = store.getScale0State();
                        const field = ctx?.viewport?._fieldRenderer;
                        throw new Error(`Timed out waiting for ${label}: ${JSON.stringify({
                            activeSize: store.getActiveScale0Bridge(ctx, state)?.latticeSize,
                            flags: state.fieldFlags,
                            style: state.forceStyle,
                            dirty: state.fieldNeedsUpdate,
                            sched: state.overlaySched && {
                                active: state.overlaySched.active,
                                cursor: state.overlaySched.cursor,
                                jobCount: state.overlaySched.jobCount,
                                kinds: state.overlaySched.jobs.slice(0, state.overlaySched.jobCount)
                                    .map((job) => ({ kind: job.kind, phase: job.phase, flowType: job.flowType })),
                                forceItems: state.overlaySched.forceFrame?.items
                                    ?.map((item) => ({ type: item.type, count: item.data?.count })),
                            },
                            gravity: field?._gravityField && {
                                requested: field._gravityFieldRequested,
                                visible: field._gravityField.visible,
                                draw: field._gravityField.geometry.drawRange.count,
                            },
                        })}`);
                    }
                    await new Promise((resolve) => setTimeout(resolve, 25));
                }
            };
            const drawable = (field, style) => {
                if (style === 'arrows') return (field._gravityField?.geometry.drawRange.count || 0) > 0;
                if (style === 'heatmap') return (field._forceHeatmaps?.gravity?.geometry.drawRange.count || 0) > 0;
                if (style === 'flow') return (field._forceStreamlineMeshes?.gravity?.mesh.geometry.drawRange.count || 0) > 0;
                return (field._forceGlyphMeshes?.gravity?.count || 0) > 0;
            };

            for (const N of sizes) {
                if (Number(document.getElementById('lattice-size')?.value) !== N) {
                    await controller.resize(ctx, N);
                    await waitFor(() => {
                        const active = store.getActiveScale0Bridge(ctx, store.getScale0State());
                        return active?.ready === true && Number(active?.latticeSize) === N;
                    }, `L=${N} worker resize`, 45_000);
                }
                const state = store.getScale0State();
                const active = store.getActiveScale0Bridge(ctx, state);
                active.setToggle('forces', true);
                active.setToggle('gravity', true);
                active.setToggle('latency_field', true);
                active.setToggle('geometric_gravity', true);
                active.tickOnce();
                await waitFor(
                    () => ((active.getDiagnostics?.()
                        ?? active.capabilities?.scale0?.getScale0Diagnostics?.())?.tick || 0) >= 1,
                    `L=${N} geometric-gravity prime tick`,
                );
                store.resetFieldFlags();
                store.setFieldToggle('showForceGravity', true);
                store.setFieldToggle('showGravPotential', true);
                const adapter = createScale0ViewportAdapter(ctx.viewport);
                adapter.setFluxVolumeVisible(false);
                adapter.setFluxSliceVisible(false);
                adapter.setOverlayVisible('showGravPotential', true);

                for (const style of styles) {
                    store.setForceStyle(style);
                    adapter.syncForceStyle(style, store.getFieldStateSnapshot());
                    state.fieldNeedsUpdate = true;
                    await waitFor(
                        () => ctx.viewport._topoRenderer?._gravPotDrawable
                            && ctx.viewport._topoRenderer?._gravPotData?.source === 'poisson-latency'
                            && drawable(ctx.viewport._fieldRenderer, style),
                        `L=${N} ${style} geometry`,
                    );
                    // Drawable becomes true as soon as its job uploads. Wait for
                    // the rest of the one-shot overlay transaction to commit so
                    // transition work is not charged to the warmed frame probe.
                    await waitFor(
                        () => !state.fieldNeedsUpdate && !state.overlaySched?.active,
                        `L=${N} ${style} overlay transaction`,
                    );
                    await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
                    const renderSamples = [];
                    const originalRender = ctx.viewport.render;
                    ctx.viewport.render = function (...args) {
                        const startedAt = performance.now();
                        try {
                            return originalRender.apply(this, args);
                        } finally {
                            renderSamples.push(performance.now() - startedAt);
                        }
                    };
                    probe.startScale0UiAuditProbe();
                    // A two-second window gives p99 enough samples to measure
                    // sustained cadence instead of promoting one host-scheduler
                    // rAF wobble into a false overlay regression.
                    await new Promise((resolve) => setTimeout(resolve, 2_000));
                    const report = await probe.stopScale0UiAuditProbe();
                    ctx.viewport.render = originalRender;
                    const renderTotal = renderSamples.reduce((sum, value) => sum + value, 0);
                    const rendererInfo = ctx.viewport.renderer?.info?.render;
                    const field = ctx.viewport._fieldRenderer;
                    out.push({
                        N,
                        style,
                        frames: report.frames,
                        longTasks: report.longTasks,
                        errors: report.errors,
                        debug: {
                            drawCount: style === 'arrows'
                                ? field._gravityField?.geometry.drawRange.count
                                : style === 'heatmap'
                                    ? field._forceHeatmaps?.gravity?.geometry.drawRange.count
                                    : style === 'flow'
                                        ? field._forceStreamlineMeshes?.gravity?.mesh.geometry.drawRange.count
                                        : field._forceGlyphMeshes?.gravity?.count,
                            renderMeanMs: renderSamples.length ? renderTotal / renderSamples.length : 0,
                            renderMaxMs: renderSamples.length ? Math.max(...renderSamples) : 0,
                            renderCalls: rendererInfo?.calls,
                            triangles: rendererInfo?.triangles,
                            points: rendererInfo?.points,
                            lines: rendererInfo?.lines,
                        },
                    });
                }
            }
            return out;
        }, { sizes: requestedSizes, styles: requestedStyles });

        await testInfo.attach('gravity-performance-report.json', {
            body: Buffer.from(JSON.stringify(reports, null, 2)),
            contentType: 'application/json',
        });
        const slowest = reports.reduce((current, report) => (
            !current || report.frames.effectiveFps < current.frames.effectiveFps ? report : current
        ), null);
        const worstP99 = reports.reduce((current, report) => (
            !current || report.frames.p99Ms > current.frames.p99Ms ? report : current
        ), null);
        console.log(
            `[gravity-overlay] ${reports.length} warmed combinations; `
            + `minimum ${slowest?.frames.effectiveFps.toFixed(2)} FPS at L=${slowest?.N} ${slowest?.style}; `
            + `worst p99 ${worstP99?.frames.p99Ms.toFixed(2)} ms at L=${worstP99?.N} ${worstP99?.style}`,
        );
        expect(reports).toHaveLength(requestedSizes.length * requestedStyles.length);
        for (const report of reports) {
            const label = `L=${report.N} ${report.style} ${JSON.stringify({ frames: report.frames, debug: report.debug })}`;
            expect(report.frames.count, `${label} sample adequacy`).toBeGreaterThanOrEqual(100);
            expect(report.frames.effectiveFps, `${label} effective FPS`).toBeGreaterThanOrEqual(58);
            expect(report.frames.p99Ms, `${label} p99 frame interval`).toBeLessThanOrEqual(20);
            expect(report.frames.intervalsOver33_4ms, `${label} missed two-frame slots`).toBe(0);
            expect(report.longTasks, `${label} long tasks`).toEqual([]);
            expect(report.errors, `${label} page errors`).toEqual([]);
        }
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
