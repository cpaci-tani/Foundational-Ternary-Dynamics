// Runs inside page.evaluate against real WASM observations and real uPlot draws.
export async function measureLagrangianCadence(durationMs = 2500) {
    const { telemetryHub: hub } = await import('/js/telemetry-hub.js');
    const { getScale0State } = await import('/js/scales/scale0/state/store.js');
    const state = getScale0State();
    const owner = state.useFluxMock ? state.fluxMock : window.__ftdCtx.bridge;
    const panel = window.__ftdCtx.lagrangianPanel;
    const gl = window.__ftdCtx.viewport.renderer.getContext();
    const ext = gl.getExtension('WEBGL_debug_renderer_info');
    const renderer = ext && gl.getParameter(ext.UNMASKED_RENDERER_WEBGL);
    const commits = {}, paints = {}, restores = [];
    for (const entry of panel.cards.values()) {
        if (!entry.onScreen || !entry.chart?.uplot) throw Error(`Chart not visible: ${entry.term.key}`);
        const key = entry.term.key, plot = entry.chart.uplot;
        commits[key] = []; paints[key] = [];
        const original = plot.setData;
        plot.setData = function (data, ...args) {
            const at = performance.now();
            const result = original.call(this, data, ...args);
            commits[key].push({ at, tick: data[0].at(-1),
                sourceTick: hub.getScale0TelemetryMeta('lagrangian')?.sampleTick,
                cost: performance.now() - at });
            return result;
        };
        const draw = () => paints[key].push({ at: performance.now(), tick: plot.data[0].at(-1) });
        (plot.hooks.draw ||= []).push(draw);
        restores.push(() => {
            plot.setData = original;
            const index = plot.hooks.draw.indexOf(draw);
            if (index >= 0) plot.hooks.draw.splice(index, 1);
        });
    }
    const frames = [], source = [], longTasks = [];
    let previous, lastVersion, handle;
    const observer = new PerformanceObserver(list => longTasks.push(...list.getEntries().map(e => e.duration)));
    observer.observe({ type: 'longtask' });
    const start = performance.now(), tickStart = owner.currentTick();
    const frame = time => {
        if (previous != null) frames.push(time - previous);
        previous = time;
        const meta = hub.getScale0TelemetryMeta('lagrangian');
        if (meta?.stateVersion !== lastVersion) {
            lastVersion = meta?.stateVersion;
            source.push({ at: performance.now(), tick: meta?.sampleTick });
        }
        handle = requestAnimationFrame(frame);
    };
    handle = requestAnimationFrame(frame);
    try { await new Promise(resolve => setTimeout(resolve, durationMs)); }
    finally {
        cancelAnimationFrame(handle); observer.disconnect(); restores.forEach(restore => restore());
    }
    return { renderer, start, elapsed: performance.now() - start, tickStart,
        tickEnd: owner.currentTick(), frames, source, commits, paints, longTasks };
}
