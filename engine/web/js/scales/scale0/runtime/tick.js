export function advanceSimulation(ctx, state) {
    const latticeSize = ctx.bridge.latticeSize || 32;
    // Global pause kills everything — no tick advance, no upload, no flux mock.
    if (!ctx.running) return latticeSize;

    const wholeTicks = state.tickAccumulator.accumulate(ctx.ticksPerFrame);
    const maxTicksPerFrame = latticeSize > 96 ? 1 : (latticeSize > 48 ? 1 : (latticeSize > 32 ? 2 : wholeTicks));
    const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
    const tickMock = !!(mockScale0 && (state.useFluxMock || state.fieldFlags.showDarkMatterHalo || state.fieldFlags.showGenesisIsosurface));

    // Three-level pause for Scale 0:
    //
    //   `running`         (global)   — gates the visualization layer (overlay
    //                                   re-sampling, streamline re-computation,
    //                                   render). Handled by updateFieldOverlays.
    //   `scenarioRunning` (scenario) — gates the PHYSICS tick. Both the bridge
    //                                   wave-equation step and the flux-mock
    //                                   pattern generator are scenario dynamics
    //                                   (they advance the underlying flux field
    //                                   that everything else is computed from).
    //                                   When scenario is paused but global is
    //                                   on, the flux state is frozen but the
    //                                   B/E/Φ overlays keep re-rendering against
    //                                   that frozen state — so the visualizer
    //                                   still "moves" (importance-sampled seeds
    //                                   re-pick each frame) while the underlying
    //                                   physics is held still.
    const tickScenario = !!ctx.scenarioRunning;
    for (let i = 0; i < ticksToRun; i++) {
        if (!state.useFluxMock && tickScenario) mainScale0.tickScale0();
        if (tickMock && tickScenario) mockScale0.tickScale0();
    }

    // Mark the lattice for re-upload only if a tick actually advanced — when
    // scenario is paused, the lattice contents haven't changed, so skipping the
    // upload saves the per-frame data round-trip.
    if (tickScenario) state.latticeNeedsUpload = true;
    return latticeSize;
}
