export function advanceSimulation(ctx, state) {
    const latticeSize = ctx.bridge.latticeSize || 32;
    if (!ctx.running) return latticeSize;

    const wholeTicks = state.tickAccumulator.accumulate(ctx.ticksPerFrame);
    const maxTicksPerFrame = latticeSize > 96 ? 1 : (latticeSize > 48 ? 1 : (latticeSize > 32 ? 2 : wholeTicks));
    const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
    const tickMock = !!(mockScale0 && (state.useFluxMock || state.fieldFlags.showDarkMatterHalo || state.fieldFlags.showGenesisIsosurface));

    for (let i = 0; i < ticksToRun; i++) {
        if (!state.useFluxMock) mainScale0.tickScale0();
        if (tickMock) mockScale0.tickScale0();
    }

    state.latticeNeedsUpload = true;
    return latticeSize;
}
