// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

const LATTICE_SIZES = [9, 17, 25, 33, 49, 65, 97, 113, 145, 181];

// Playwright tracing continuously captures WebGL screencast JPEGs. That
// compositor readback changes the cadence being measured, so this dedicated
// performance/audit file runs untraced; failures still retain screenshots.
test.use({ trace: 'off' });

test.describe('Scale 0 discrete gravity overlay audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        // The focused scientific tests contain worker-specific lifecycle
        // assertions. Only the dedicated release matrix is backend-selectable.
        const requestedBackend = testInfo.title.includes('warmed gravity surface')
            ? process.env.FTD_GRAVITY_PERF_BACKEND || 'auto'
            : 'wasm';
        if (requestedBackend === 'direct-wasm') {
            await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        }
        const path = requestedBackend === 'native'
            ? '/?engine=native'
            : requestedBackend === 'wasm' || requestedBackend === 'direct-wasm'
                ? '/?engine=wasm'
                : '/';
        await gotoAndReady(page, { path, timeout: 90_000 });
        await page.waitForFunction(async (backendContract) => {
            const store = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const state = store.getScale0State();
            const active = store.getActiveScale0Bridge(ctx, state);
            if (!active || !store.isScale0AuthoritativeGenerationReady(state)) return false;
            if (backendContract === 'native') return active.isNativeGPU === true;
            if (backendContract === 'wasm') return active.isWorker === true && active.ready === true;
            if (backendContract === 'direct-wasm') {
                return active.isWasm === true && active.isWorker !== true;
            }
            return active.isNativeGPU === true
                || (active.isWorker === true && active.ready === true)
                || (active.isWasm === true && active.isWorker !== true);
        }, requestedBackend, { timeout: 90_000 });
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

    test('compact native gravity slices honor the FTV2 origin at the lattice mid-plane', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const [{ gravitySliceMidIndex }, { gravitySlice }] = await Promise.all([
                import('/js/scales/scale0/ui/overlays/gravity-panel.js'),
                import('/js/scales/scale0/analysis/gravity-analysis.js'),
            ]);
            const latticeSize = 113;
            const axisCount = 37;
            const spacing = 3;
            const origin = 2;
            const mid = gravitySliceMidIndex(latticeSize, axisCount, spacing, origin);
            const wrongMid = Math.round((latticeSize >> 1) / spacing);
            const volume = new Float32Array(axisCount ** 3);
            const index = (x, y, z) => x + axisCount * (y + axisCount * z);
            volume[index(mid, mid, mid)] = 9;
            volume[index(wrongMid, wrongMid, wrongMid)] = 4;
            const maxima = [0, 1, 2].map((axis) => {
                const plane = gravitySlice(volume, axisCount, axis, mid, 'latency', 9, spacing);
                let max = 0;
                for (const value of plane) if (value > max) max = value;
                return max;
            });
            return { mid, wrongMid, maxima };
        });
        expect(result.mid).toBe(18);
        expect(result.wrongMid).toBe(19);
        for (const maximum of result.maxima) expect(maximum).toBeGreaterThan(0.99);
    });

    test('direct-WASM fused proxy reduction mirrors the centered visual grid and stencil', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const {
                gravityProxySamplesFromVolume,
                gravityVisualSampleGrid,
            } = await import('/js/scales/scale0/analysis/gravity-analysis.js?v=3');
            const N = 9;
            const volume = new Float64Array(N ** 3);
            const center = (N - 1) >> 1;
            volume[(center * N + center) * N + center] = 3;
            const samples = gravityProxySamplesFromVolume(volume, N, 2);
            const empty = gravityProxySamplesFromVolume(new Float64Array(N ** 3), N, 2);
            return {
                fullGrid: gravityVisualSampleGrid(N, 2, false),
                interiorGrid: gravityVisualSampleGrid(N, 2, true),
                maxRho: samples.maxRho,
                latencyCount: samples.latencyCount,
                latency: samples.latencyVals[0],
                kretCount: samples.kretCount,
                kret: samples.kretVals[0],
                emptyCounts: [empty.latencyCount, empty.kretCount],
            };
        });

        expect(result.fullGrid).toEqual({ stride: 2, origin: 0, count: 5, end: 10 });
        expect(result.interiorGrid).toEqual({ stride: 2, origin: 2, count: 3, end: 8 });
        expect(result.maxRho).toBe(9);
        expect(result.latencyCount).toBe(1);
        expect(result.latency).toBeCloseTo(Math.sqrt(0.998), 12);
        expect(result.kretCount).toBe(1);
        expect(result.kret).toBeCloseTo(16 * 0.998, 10);
        expect(result.emptyCounts).toEqual([0, 0]);
    });

    test('mounted direct-WASM consumes its zero-copy volume before any later engine call', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const { mountGravityPanel } = await import(
                '/js/scales/scale0/ui/overlays/gravity-panel.js?direct-zero-copy=1'
            );
            const store = await import('/js/scales/scale0/state/store.js');
            const scenarioId = 's0-seed-massive-body';
            const select = document.getElementById('scenario-select');
            if (![...select.options].some((option) => option.value === scenarioId)) {
                select.add(new Option(scenarioId, scenarioId));
            }
            select.value = scenarioId;
            store.setCurrentScenarioId(scenarioId);
            const loadGeneration = 900_001;
            store.beginScale0AuthoritativeLoad({ scenarioId, loadGeneration });
            if (!store.completeScale0AuthoritativeLoad({
                scenarioId, loadGeneration, tick: 0, source: 'mounted-zero-copy-test',
            })) throw new Error('failed to establish controlled Gravity test generation');
            const host = document.createElement('section');
            host.className = 'active';
            document.body.appendChild(host);
            const N = 9;
            const volume = new Float32Array(N ** 3);
            volume[((4 * N) + 4) * N + 4] = 2;
            const order = [];
            let volumeIssued = false;
            let poisoned = false;
            let readoutBeforePoison = null;
            const caps = {
                latticeSize: N,
                getScale0ForceField() {
                    order.push('force');
                    return { vectors: new Float32Array([0.25, 0, 0]), count: 1 };
                },
                hasScale0SamplerSnapshot: () => true,
                getScale0SamplerSnapshotVersion: () => null,
                getScale0GravityMetricAgg() {
                    order.push('aggregate');
                    return {
                        active: false, requested: false, latencyMax: 0,
                        latencyMean: 0, fMin: 1, gammaMax: 1,
                        dilationMaxPct: 0, voxelCount: 0,
                    };
                },
                getScale0FluxVolume() {
                    order.push('volume');
                    volumeIssued = true;
                    return volume;
                },
            };
            const bridge = {
                isWasm: true,
                isWorker: false,
                capabilities: { scale0: caps },
                replaceSamplerWants() {},
                getToggle(key) {
                    order.push(`toggle:${key}`);
                    if (volumeIssued && !poisoned) {
                        readoutBeforePoison = host.querySelector('.grav-tile-readout')?.textContent || null;
                        // A subsequent embind call may invalidate a zero-copy
                        // heap view. Poison it here: metrics/slices remain valid
                        // only if the panel consumed the view synchronously.
                        volume.fill(Number.NaN);
                        poisoned = true;
                    }
                    if (key === 'forces' || key === 'gravity') return true;
                    if (key === 'geometric_gravity') return false;
                    if (key === 'latency_field' || key === 'field_energy_gravity') return false;
                    return false;
                },
            };
            const api = mountGravityPanel(host, () => bridge);
            const snapshot = {
                order: [...order],
                readoutBeforePoison,
                latencyMax: api.lastMetrics?.L?.max ?? null,
                forceMax: api.lastMetrics?.F?.max ?? null,
                telemetryState: api.telemetryState,
                applicability: api.applicability,
                coordinatorActive: api.coordinatorActive,
                authoritativeReady: api.authoritativeGenerationReady,
                scenarioId: store.getScale0State().currentScenarioId,
            };
            api.dispose();
            host.remove();
            return snapshot;
        });

        const forceIndex = result.order.indexOf('force');
        const aggregateIndex = result.order.indexOf('aggregate');
        const volumeIndex = result.order.indexOf('volume');
        const nextEngineIndex = result.order.findIndex((value, index) => (
            index > volumeIndex && value.startsWith('toggle:')
        ));
        expect(forceIndex, JSON.stringify(result)).toBeGreaterThanOrEqual(0);
        expect(aggregateIndex).toBeGreaterThan(forceIndex);
        expect(volumeIndex).toBeGreaterThan(aggregateIndex);
        expect(nextEngineIndex).toBeGreaterThan(volumeIndex);
        expect(result.readoutBeforePoison).toMatch(/^max /);
        expect(result.latencyMax).toBeCloseTo(Math.sqrt(0.998), 6);
        expect(result.forceMax).toBeCloseTo(0.25, 6);
        expect(result.telemetryState).toBe('ready');
    });

    test('worker sampler readiness follows received message provenance, not a later atomic tick', async ({ page }) => {
        const ready = await page.evaluate(async () => {
            const { WasmBridgeProxy } = await import('/js/bridge/wasm-bridge-proxy.js');
            const { samplerVersionsAdvanced } = await import(
                '/js/scales/scale0/ui/overlays/gravity-panel.js?version-coherence=1'
            );
            const proxy = Object.create(WasmBridgeProxy.prototype);
            proxy._samplerCache = { 'latency@2': { values: new Float32Array([1]), count: 1 } };
            proxy._samplerCacheVersion = { 'latency@2': 7 };
            // No live worker/control block is needed: readiness belongs to the
            // received message. A concurrently newer atomic cannot invalidate it.
            return {
                ready: proxy.hasSamplerSnapshot('latency', 2),
                version: proxy.getSamplerSnapshotVersion('latency', 2),
                allAdvanced: samplerVersionsAdvanced([3, 4, 5], [4, 5, 6]),
                mixedRejected: samplerVersionsAdvanced([3, 4, 5], [4, 4, 6]),
            };
        });
        expect(ready).toEqual({
            ready: true,
            version: 7,
            allAdvanced: true,
            mixedRejected: false,
        });
    });

    test('paused native replies advance telemetry without another field version', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const { mountGravityPanel } = await import(
                '/js/scales/scale0/ui/overlays/gravity-panel.js?native-freshness=1'
            );
            const store = await import('/js/scales/scale0/state/store.js');
            const scenarioId = 's0-seed-massive-body';
            const select = document.getElementById('scenario-select');
            if (![...select.options].some((option) => option.value === scenarioId)) {
                select.add(new Option(scenarioId, scenarioId));
            }
            select.value = scenarioId;
            store.setCurrentScenarioId(scenarioId);
            const loadGeneration = 900_002;
            store.beginScale0AuthoritativeLoad({ scenarioId, loadGeneration });
            if (!store.completeScale0AuthoritativeLoad({
                scenarioId, loadGeneration, tick: 0, source: 'paused-native-test',
            })) throw new Error('failed to establish controlled Gravity test generation');
            const host = document.createElement('section');
            host.className = 'active';
            document.body.appendChild(host);
            const volume = new Float32Array(33 ** 3);
            let revision = 41;
            let value = 0.25;
            let geometric = false;
            let forces = true;
            let latencyRequested = false;
            const scalar = () => ({ values: new Float32Array([value]), count: 1 });
            const vector = () => ({ vectors: new Float32Array([value, 0, 0]), count: 1 });
            const caps = {
                latticeSize: 33,
                getScale0FieldSamples: scalar,
                getScale0ForceField: vector,
                getScale0FluxVolume: () => volume,
                hasScale0SamplerSnapshot: () => true,
                getScale0SamplerSnapshotVersion: (kind) => (
                    kind === 'gravityMetricAgg' ? null : revision
                ),
                // Omit `requested` to exercise compatibility with the current
                // deployed WASM artifact; the panel must recover the exact
                // request state from the authoritative toggle mirror.
                getScale0GravityMetricAgg: () => ({
                    active: false,
                    latencyMax: 0,
                    latencyMean: 0,
                    fMin: 1,
                    gammaMax: 1,
                    dilationMaxPct: 0,
                    voxelCount: 0,
                }),
            };
            const bridge = {
                capabilities: { scale0: caps },
                replaceSamplerWants() {},
                getToggle(key) {
                    if (key === 'forces') return forces;
                    if (key === 'gravity') return true;
                    if (key === 'geometric_gravity') return geometric;
                    if (key === 'latency_field') return latencyRequested;
                    if (key === 'field_energy_gravity') return false;
                    return false;
                },
            };
            const api = mountGravityPanel(host, () => bridge);
            api.update();
            const first = {
                max: api.lastMetrics?.L?.max,
                history: api.historyLength,
            };

            // This models the native reply arriving after the first getter
            // enqueued it, while the paused engine's fieldDataVersion is stable.
            revision = 42;
            value = 0.75;
            api.update();
            const second = {
                max: api.lastMetrics?.L?.max,
                history: api.historyLength,
                telemetry: host.querySelector('#gravity-panel-telemetry')?.textContent || '',
                forceSliceTitle: host.querySelector('.grav-qbtn[data-kind="force"]')?.title || '',
                forceLawTitle: host.querySelector('[data-grav-force-label-wrap]')?.title || '',
                cppHeadingTitle: host.querySelector('[data-grav-cpp-heading]')?.title || '',
                cppLatencyTitle: host.querySelector('[data-grav-value="cpp-latency"]')
                    ?.previousElementSibling?.title || '',
                cppGammaTitle: host.querySelector('[data-grav-value="cpp-gamma"]')
                    ?.previousElementSibling?.title || '',
            };
            geometric = true;
            revision = 43;
            api.update();
            const geometricTelemetry = host.querySelector(
                '#gravity-panel-telemetry',
            )?.textContent || '';
            forces = false;
            api.update();
            const inactiveTelemetry = host.querySelector(
                '#gravity-panel-telemetry',
            )?.textContent || '';
            const inactiveForceTitle = host.querySelector(
                '[data-grav-force-label-wrap]',
            )?.title || '';
            latencyRequested = true;
            revision = 44;
            api.update();
            const requestedLatencyStatus = host.querySelector(
                '[data-grav-cpp-status]',
            )?.textContent || '';
            latencyRequested = false;
            revision = 45;
            api.update();
            const inactiveLatencyStatus = host.querySelector(
                '[data-grav-cpp-status]',
            )?.textContent || '';
            api.dispose();
            host.remove();
            return {
                first,
                second,
                geometricTelemetry,
                inactiveTelemetry,
                inactiveForceTitle,
                requestedLatencyStatus,
                inactiveLatencyStatus,
            };
        });

        expect(result.first.max).toBeCloseTo(0.25, 6);
        expect(result.second.max).toBeCloseTo(0.75, 6);
        // A newer transport snapshot is not a new physics observation.
        expect(result.second.history).toBe(result.first.history);
        expect(result.second.telemetry).toContain('Engine force branch active · G_N·∇₂|J|');
        expect(result.second.forceLawTitle).toContain('radius-2 central-difference stencil');
        expect(result.second.forceLawTitle).toContain('only at manifested sites');
        expect(result.second.telemetry).toContain('G_N lattice coupling');
        expect(result.second.telemetry).not.toContain('gravity PE');
        expect(result.geometricTelemetry).toContain('Engine force branch active · Mᵢc²L·∇₂L');
        expect(result.inactiveTelemetry).toContain('Engine force sampler · not applied');
        expect(result.inactiveForceTitle).toContain('forces umbrella toggle is OFF');
        expect(result.requestedLatencyStatus)
            .toBe('requested — no nonzero latency cells in this engine observation');
        expect(result.inactiveLatencyStatus)
            .toBe('inactive — Poisson-latency operator not requested');
        expect(result.second.cppHeadingTitle).toContain('∇²φ_latency = 4πG_N(ρ−ρ̄)');
        expect(result.second.cppHeadingTitle).toContain('M_GRAVITATIONAL|s|');
        expect(result.second.cppHeadingTitle).toContain('½(|J|²+|wave_vel|²)');
        expect(result.second.cppHeadingTitle).toContain('not a derivation of spacetime geometry');
        expect(result.second.cppLatencyTitle).toContain('it is not φ_latency itself');
        expect(result.second.cppGammaTitle)
            .toContain('1/√(1−L²−|v|²/C_SPEED²)');
        expect(result.second.forceSliceTitle).toContain('Presentation-only slice proxy');
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
            const { runScale0PhysicsTicks } = await import('/js/scales/scale0/runtime/tick.js');
            const state = store.getScale0State();
            const active = store.getActiveScale0Bridge(window.__ftdCtx, state);
            active.setToggle('forces', true);
            active.setToggle('gravity', true);
            active.setToggle('latency_field', true);
            active.setToggle('geometric_gravity', true);
            const tickBefore = Number((active.getDiagnostics?.()
                ?? active.capabilities?.scale0?.getScale0Diagnostics?.())?.tick || 0);
            runScale0PhysicsTicks(window.__ftdCtx, state, 1);
            const deadline = performance.now() + 15_000;
            while (Number((active.getDiagnostics?.()
                ?? active.capabilities?.scale0?.getScale0Diagnostics?.())?.tick || 0) <= tickBefore) {
                if (performance.now() >= deadline) {
                    throw new Error('Timed out waiting for the gravity-prime worker tick');
                }
                await new Promise((resolve) => setTimeout(resolve, 25));
            }
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
        testInfo.setTimeout(900_000);
        const consoleErrors = attachConsoleWatcher(page);
        const requestedBackend = process.env.FTD_GRAVITY_PERF_BACKEND || 'auto';
        const requireHardwareWebgl = process.env.FTD_HARDWARE_WEBGL === '1';
        const backend = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const active = store.getActiveScale0Bridge(ctx, store.getScale0State());
            const gl = ctx?.viewport?.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            return {
                isWorker: active?.isWorker === true,
                isNativeGPU: active?.isNativeGPU === true,
                isDirectWasm: active?.isWasm === true && active?.isWorker !== true,
                webglRenderer: rendererInfo
                    ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                    : '',
            };
        });
        if (requestedBackend === 'native') expect(backend.isNativeGPU).toBe(true);
        if (requestedBackend === 'wasm') expect(backend.isWorker).toBe(true);
        if (requestedBackend === 'direct-wasm') expect(backend.isDirectWasm).toBe(true);
        if (requireHardwareWebgl) {
            expect(backend.webglRenderer, 'release matrix exposes a WebGL renderer').not.toBe('');
            expect(backend.webglRenderer, 'release matrix does not certify SwiftShader/software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        await selectScale0Scenario(page, 's0-seed-massive-body', { settleMs: 250 });
        const backendDefaultSizes = requestedBackend === 'native' || backend.isNativeGPU
            ? LATTICE_SIZES
            : requestedBackend === 'direct-wasm' || backend.isDirectWasm
                ? LATTICE_SIZES.filter((size) => size <= 33)
                : LATTICE_SIZES.filter((size) => size <= 97);
        const requestedSizes = (process.env.FTD_GRAVITY_PERF_SIZES || backendDefaultSizes.join(','))
            .split(',').map(Number).filter(Number.isFinite);
        const requestedStyles = (process.env.FTD_GRAVITY_PERF_STYLES || 'arrows,heatmap,flow,glyphs')
            .split(',').map((value) => value.trim()).filter(Boolean);
        const reports = await page.evaluate(async ({ sizes, styles, webglRenderer }) => {
            const controller = await import('/js/scales/scale0/controller.js');
            const store = await import('/js/scales/scale0/state/store.js');
            const { runScale0PhysicsTicks } = await import('/js/scales/scale0/runtime/tick.js');
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
                            qualification: store.getScale0QualificationState(),
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
                        const current = store.getScale0State();
                        const owner = store.getActiveScale0Bridge(ctx, current);
                        const transportReady = current.useFluxMock
                            ? current.fluxMock?.ready === true
                            : owner != null;
                        return transportReady
                            && Number(owner?.latticeSize) === N
                            && store.isScale0AuthoritativeGenerationReady(current);
                    }, `L=${N} authoritative resize`, 45_000);
                }
                const state = store.getScale0State();
                const active = store.getActiveScale0Bridge(ctx, state);
                if (N > 97 && active?.isNativeGPU !== true) {
                    throw new Error(`L=${N} requires an explicit native-GPU owner`);
                }
                if (N > 33 && active?.isNativeGPU !== true && active?.isWorker !== true) {
                    throw new Error(`L=${N} requires the WASM worker or native-GPU owner`);
                }
                active.setToggle('forces', true);
                active.setToggle('gravity', true);
                active.setToggle('latency_field', true);
                active.setToggle('geometric_gravity', true);
                const tickBefore = Number((active.getDiagnostics?.()
                    ?? active.capabilities?.scale0?.getScale0Diagnostics?.())?.tick || 0);
                runScale0PhysicsTicks(ctx, state, 1);
                await waitFor(
                    () => Number((active.getDiagnostics?.()
                        ?? active.capabilities?.scale0?.getScale0Diagnostics?.())?.tick || 0) > tickBefore,
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
                    // Match the release-gate protocol: allow transition
                    // allocations and deferred driver work to settle before a
                    // long enough window that one host-scheduler wobble cannot
                    // masquerade as sustained panel cadence.
                    await new Promise((resolve) => setTimeout(resolve, 3_000));
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
                    await new Promise((resolve) => setTimeout(resolve, 12_000));
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
                            backend: active?.constructor?.name || 'unknown',
                            isWorker: active?.isWorker === true,
                            isNativeGPU: active?.isNativeGPU === true,
                            webglRenderer,
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
        }, { sizes: requestedSizes, styles: requestedStyles, webglRenderer: backend.webglRenderer });
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
            if (requestedBackend === 'native') {
                expect(report.debug.isNativeGPU, `${label} explicit native owner`).toBe(true);
            } else if (requestedBackend === 'wasm') {
                expect(report.debug.isWorker, `${label} explicit WASM-worker owner`).toBe(true);
                expect(report.debug.isNativeGPU, `${label} is not native`).toBe(false);
            } else if (requestedBackend === 'direct-wasm') {
                expect(report.debug.isWorker, `${label} is not a worker`).toBe(false);
                expect(report.debug.isNativeGPU, `${label} is not native`).toBe(false);
            }
            if (requireHardwareWebgl) {
                expect(report.debug.webglRenderer, `${label} hardware WebGL owner`)
                    .not.toMatch(/swiftshader|software/i);
            }
            if (report.N > 97) {
                expect(report.debug.isNativeGPU, `${label} native-only size owner`).toBe(true);
            } else if (report.N > 33 && !report.debug.isNativeGPU) {
                expect(report.debug.isWorker, `${label} worker/native size owner`).toBe(true);
            }
            expect(report.frames.count, `${label} sample adequacy`).toBeGreaterThanOrEqual(600);
            // 59.5 admits one boundary-quantization interval in a nominal
            // 59.94/60 Hz rAF stream; p99 and missed-slot gates below still
            // reject sustained cadence below the 60 Hz frame budget.
            expect(report.frames.effectiveFps, `${label} effective FPS`).toBeGreaterThanOrEqual(59.5);
            expect(report.frames.p95Ms, `${label} p95 frame interval`).toBeLessThanOrEqual(17);
            expect(report.frames.p99Ms, `${label} p99 frame interval`).toBeLessThanOrEqual(20);
            expect(report.frames.intervalsOver33_4ms, `${label} missed two-frame slots`).toBe(0);
            expect(report.longTasks, `${label} long tasks`).toEqual([]);
            expect(report.errors, `${label} page errors`).toEqual([]);
        }
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
