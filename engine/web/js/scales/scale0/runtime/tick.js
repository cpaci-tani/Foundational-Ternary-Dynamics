import { getActiveLatticeSize } from '../state/store.js';

/** Advance Scale-0 physics by `tickCount` ticks on the active owner only. */
export function runScale0PhysicsTicks(ctx, state, tickCount = 1) {
    if (tickCount <= 0) return;

    // Worker-backed WasmBridgeProxy: tickScale0 on the capability is a no-op
    // because the worker self-ticks when RUNNING. For discrete steps (including
    // the single-tick button) use tickOnce(), which sends a command to the
    // worker, runs one tick there, then calls postFrame() so the frame counter
    // increments and the main-thread render picks up the new state.
    if (typeof ctx.bridge.tickOnce === 'function') {
        for (let i = 0; i < tickCount; i++) ctx.bridge.tickOnce();
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
