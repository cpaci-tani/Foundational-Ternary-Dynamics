import { getActiveLatticeSize } from '../state/store.js';

/** Advance Scale-0 physics by `tickCount` ticks on the active owner only. */
export function runScale0PhysicsTicks(ctx, state, tickCount = 1) {
    if (tickCount <= 0) return;

    // Worker path (WasmBridgeProxy): tickScale0 capability is a no-op because the
    // worker self-ticks. For discrete steps (step button / stepScale0) call
    // tickOnce(), which sends a single-tick command to the worker, which runs the
    // tick and fires postFrame() so diagnostics update on the main thread.
    const fm = state.fluxMock;
    if (fm && fm.isWorker && state.useFluxMock) {
        for (let i = 0; i < tickCount; i++) fm.tickOnce();
        state.fieldDataVersion = (state.fieldDataVersion || 0) + tickCount;
        return;
    }

    const mainScale0 = ctx.bridge.capabilities.scale0;
    for (let i = 0; i < tickCount; i++) {
        mainScale0.tickScale0();
    }

    // WebSocketBridge is asynchronous and may coalesce high-frequency playback
    // demand. Its tick_complete/run_complete callback owns the version/upload
    // bump so UI state advances exactly once per physics tick the server really
    // completed, not once per attempted request.
    if (ctx.bridge?.isNativeGPU) return;

    state.latticeNeedsUpload = true;
    state.fieldDataVersion = (state.fieldDataVersion || 0) + tickCount;
}

export function advanceSimulation(ctx, state) {
    const latticeSize = getActiveLatticeSize(ctx, state);

    // Worker-backed physics (WasmBridgeProxy): the worker self-ticks on its own
    // ~60Hz loop when CTRL.RUNNING=1. Forward the desired run state (deduped in
    // the proxy) and drive overlay/render refresh from the worker's physics-data
    // version (sampler-only readbacks must not impersonate a physics update),
    // then return — the in-thread tick path below is for non-worker scenarios only.
    const fm = state.fluxMock;
    if (fm && fm.isWorker && state.useFluxMock) {
        if (typeof fm.setRunning === 'function') fm.setRunning(ctx.running);
        if (typeof fm.setTicksPerFrame === 'function') fm.setTicksPerFrame(ctx.ticksPerFrame);
        const dataVersion = fm.dataVersion || 0;
        if (dataVersion !== state._lastWorkerDataVersion) {
            state._lastWorkerDataVersion = dataVersion;
            state.latticeNeedsUpload = true;
            state.fieldDataVersion = (state.fieldDataVersion || 0) + 1;
        }
        return latticeSize;
    }

    // Global pause kills everything — no tick advance, no upload.
    if (!ctx.running) return latticeSize;
    const wholeTicks = state.tickAccumulator.accumulate(ctx.ticksPerFrame);
    const maxTicksPerFrame = latticeSize > 96 ? 1 : (latticeSize > 48 ? 1 : (latticeSize > 32 ? 2 : wholeTicks));
    const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

    if (ticksToRun > 0) {
        runScale0PhysicsTicks(ctx, state, ticksToRun);
    }

    return latticeSize;
}
