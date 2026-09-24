// Runs inside page.evaluate against real worker-WASM Gravity observations.
// It records actual panel-canvas draws and retained SVG path commits; a fast
// viewport with a slow panel therefore cannot satisfy the cadence evidence.
export async function measureGravityCadence(durationMs = 2500) {
    const store = await import('/js/scales/scale0/state/store.js');
    const state = store.getScale0State();
    const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
    const panelApi = window.__ftdGravityPanel;
    const stride = panelApi?.telemetryStride;
    const capabilities = owner?.capabilities?.scale0;
    const currentObservationTick = () => {
        const observationTick = capabilities?.getScale0GravityObservation?.(stride)?.sampleTick;
        if (Number.isSafeInteger(observationTick) && observationTick >= 0) return observationTick;
        // Direct browser WASM performs the sampler reads synchronously inside
        // this same JS task, so its current tick is the exact source clock.
        // Other asynchronous/native owners must never receive this fallback.
        if (owner?.isWasm === true && owner?.isWorker !== true) {
            const tick = owner.currentTick?.();
            return Number.isSafeInteger(tick) && tick >= 0 ? tick : null;
        }
        return null;
    };

    const gl = window.__ftdCtx.viewport.renderer.getContext();
    const ext = gl.getExtension('WEBGL_debug_renderer_info');
    const renderer = ext && gl.getParameter(ext.UNMASKED_RENDERER_WEBGL);
    const heatmaps = { 0: [], 1: [], 2: [] };
    const sparks = { Lmax: [], Kmax: [], Fmean: [], dil: [] };
    const frames = [], source = [], surfaces = [], longTasks = [];
    const canvasPrototype = CanvasRenderingContext2D.prototype;
    const pathPrototype = SVGPathElement.prototype;
    const originalDrawImage = canvasPrototype.drawImage;
    const originalSetAttribute = pathPrototype.setAttribute;

    canvasPrototype.drawImage = function monitoredGravityHeatmapDraw(...args) {
        const match = /^gravity-panel-tile-(\d)$/.exec(this.canvas?.id || '');
        const result = originalDrawImage.apply(this, args);
        if (match) {
            const axis = Number(match[1]);
            heatmaps[axis].push({
                at: performance.now(),
                tick: Number(this.canvas.dataset.sampleTick),
                sourceTick: currentObservationTick(),
            });
        }
        return result;
    };

    pathPrototype.setAttribute = function monitoredGravitySparkCommit(name, value) {
        const row = name === 'd' ? this.closest?.('[data-grav-series]') : null;
        const key = row?.closest?.('#gravity-panel-delta') ? row.dataset.gravSeries : null;
        const result = originalSetAttribute.call(this, name, value);
        if (key && sparks[key]) {
            const tick = panelApi?.sampleTick ?? null;
            const sourceTick = currentObservationTick();
            sparks[key].push({ at: performance.now(), tick, sourceTick, length: String(value).length });
        }
        return result;
    };

    let previousFrame = null;
    let previousSourceTick = null;
    let previousPanelTick = null;
    let frameHandle = null;
    const readDatasetTick = element => {
        const value = element?.dataset?.sampleTick;
        return value === undefined ? null : Number(value);
    };
    const observeFrame = time => {
        if (previousFrame !== null) frames.push(time - previousFrame);
        previousFrame = time;
        const sourceTick = currentObservationTick();
        if (Number.isSafeInteger(sourceTick) && sourceTick !== previousSourceTick) {
            previousSourceTick = sourceTick;
            source.push({ at: performance.now(), tick: sourceTick });
        }
        const apiTick = panelApi?.sampleTick ?? null;
        if (Number.isSafeInteger(apiTick) && apiTick !== previousPanelTick) {
            previousPanelTick = apiTick;
            const root = document.getElementById('gravity-panel');
            surfaces.push({
                at: performance.now(),
                apiTick,
                sourceTick,
                panelTick: readDatasetTick(root),
                telemetryTick: readDatasetTick(document.getElementById('gravity-panel-telemetry')),
                deltaTick: readDatasetTick(document.getElementById('gravity-panel-delta')),
                tileTicks: [...root.querySelectorAll('.grav-tile canvas')].map(readDatasetTick),
                // SVG provenance advances even when a flat trace retains an
                // identical `d` string and therefore has no DOM path commit.
                sparkTicks: [...root.querySelectorAll('#gravity-panel-delta .grav-spark')]
                    .map(readDatasetTick),
            });
        }
        frameHandle = requestAnimationFrame(observeFrame);
    };

    const observer = typeof PerformanceObserver === 'function'
        ? new PerformanceObserver(list => longTasks.push(...list.getEntries().map(entry => entry.duration)))
        : null;
    try { observer?.observe({ type: 'longtask' }); } catch { /* unsupported entry type */ }
    const start = performance.now();
    const tickStart = owner?.currentTick?.() ?? null;
    frameHandle = requestAnimationFrame(observeFrame);
    try {
        await new Promise(resolve => setTimeout(resolve, durationMs));
    } finally {
        if (frameHandle !== null) cancelAnimationFrame(frameHandle);
        observer?.disconnect();
        canvasPrototype.drawImage = originalDrawImage;
        pathPrototype.setAttribute = originalSetAttribute;
    }
    return {
        renderer,
        start,
        elapsed: performance.now() - start,
        tickStart,
        tickEnd: owner?.currentTick?.() ?? null,
        frames,
        source,
        surfaces,
        heatmaps,
        sparks,
        longTasks,
    };
}
