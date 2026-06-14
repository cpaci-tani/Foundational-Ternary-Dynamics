import { getActiveLatticeSize } from '../state/store.js';

/** Advance Scale-0 physics by `tickCount` ticks on the active owner only. */
export function runScale0PhysicsTicks(ctx, state, tickCount = 1) {
    if (tickCount <= 0) return;

    const fm = state.fluxMock;
    if (fm && fm.isWorker && state.useFluxMock) {
        const mockScale0 = fm.capabilities?.scale0;
        for (let i = 0; i < tickCount; i++) {
            mockScale0?.tickScale0?.();
        }
        state.latticeNeedsUpload = true;
        state.fieldDataVersion = (state.fieldDataVersion || 0) + tickCount;
        return;
    }

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
    const tickMock = !!(mockScale0 && state.useFluxMock);

    for (let i = 0; i < tickCount; i++) {
        if (!state.useFluxMock) mainScale0.tickScale0();
        if (tickMock) mockScale0.tickScale0();
    }

    state.latticeNeedsUpload = true;
    state.fieldDataVersion = (state.fieldDataVersion || 0) + tickCount;
}

export function advanceSimulation(ctx, state) {
    const latticeSize = getActiveLatticeSize(ctx, state);

    // Worker-backed physics (Phase 2): the MockBridgeProxy's worker self-ticks on
    // its own clock. Forward the desired run state (deduped in the proxy) and
    // drive overlay/render refresh from the worker's frame counter, then return —
    // the in-thread tick path below is for non-worker scenarios only.
    const fm = state.fluxMock;
    if (fm && fm.isWorker && state.useFluxMock) {
        if (typeof fm.setRunning === 'function') fm.setRunning(ctx.running);
        const fc = fm.frameCounter || 0;
        if (fc !== state._lastWorkerFrame) {
            state._lastWorkerFrame = fc;
            state.latticeNeedsUpload = true;
            state.fieldDataVersion = (state.fieldDataVersion || 0) + 1;
        }
        return latticeSize;
    }

    // Global pause kills everything — no tick advance, no upload, no flux mock.
    if (!ctx.running) return latticeSize;
    const wholeTicks = state.tickAccumulator.accumulate(ctx.ticksPerFrame);
    const maxTicksPerFrame = latticeSize > 96 ? 1 : (latticeSize > 48 ? 1 : (latticeSize > 32 ? 2 : wholeTicks));
    const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

    if (ticksToRun > 0) {
        runScale0PhysicsTicks(ctx, state, ticksToRun);
    }

    return latticeSize;
}
