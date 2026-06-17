import { getActiveLatticeSize } from '../state/store.js';

/** Advance Scale-0 physics by `tickCount` ticks on the active owner only. */
export function runScale0PhysicsTicks(ctx, state, tickCount = 1) {
    if (tickCount <= 0) return;

    // Scale-0 off-thread path: WasmBridgeProxy is stored as state.fluxMock (NOT
    // ctx.bridge, which is always the main-thread WasmBridge). The proxy's
    // tickScale0 capability is a no-op because the worker self-ticks when RUNNING.
    // For discrete steps use tickOnce(), which sends a single-tick command to the
    // worker, which runs the tick and calls postFrame() so diagnostics update.
    if (state.useFluxMock && state.fluxMock && typeof state.fluxMock.tickOnce === 'function') {
        for (let i = 0; i < tickCount; i++) state.fluxMock.tickOnce();
        state.fieldDataVersion = (state.fieldDataVersion || 0) + tickCount;
        return;
    }

    const mainScale0 = ctx.bridge.capabilities.scale0;
    for (let i = 0; i < tickCount; i++) {
        mainScale0.tickScale0();
    }

    state.latticeNeedsUpload = true;
    state.fieldDataVersion = (state.fieldDataVersion || 0) + tickCount;
}

export function advanceSimulation(ctx, state) {
    const latticeSize = getActiveLatticeSize(ctx, state);

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
