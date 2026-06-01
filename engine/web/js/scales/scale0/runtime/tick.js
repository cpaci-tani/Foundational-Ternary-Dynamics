import { getScale0MemoryRecorder } from '../controller.js';

export function advanceSimulation(ctx, state) {
    const latticeSize = ctx.bridge.latticeSize || 32;
    // Global pause kills everything — no tick advance, no upload, no flux mock.
    if (!ctx.running) return latticeSize;
    // User is scrubbing — freeze physics so the snapshot loaded by
    // hydrateToTick stays put until scrubEnd / resumeLive.
    if (state.scrubbing) return latticeSize;
    // A render is fast-forwarding snapshots into the clip buffer — it owns
    // bridge.tick() during that time. Letting the animate loop also tick
    // would double-advance and corrupt both the clip and the live sim.
    if (state.rendering) return latticeSize;

    const wholeTicks = state.tickAccumulator.accumulate(ctx.ticksPerFrame);
    const maxTicksPerFrame = latticeSize > 96 ? 1 : (latticeSize > 48 ? 1 : (latticeSize > 32 ? 2 : wholeTicks));
    const ticksToRun = Math.min(wholeTicks, maxTicksPerFrame);

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
    // Only tick the mock bridge when it IS the physics source. Merely
    // enabling a derived-overlay (darkMatterHalo, genesisIsosurface) used to
    // start the mock's tick loop as a side effect — that made the scene
    // "tick on toggle". Overlays now sample whatever state the mock is in.
    const tickMock = !!(mockScale0 && state.useFluxMock);

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
    //
    // `latticeNeedsUpload` is a one-shot flag consumed (and cleared) by
    // frame-sync.js on its own throttle cadence, which is generally finer than
    // the overlay throttle — so the overlay scheduler cannot reliably use it as
    // a "did the field change since my last sweep?" signal. Maintain instead a
    // monotonic `fieldDataVersion` that ticks once per actual field advance and
    // is never cleared; the overlay scheduler latches its value at sweep start
    // and only re-sweeps when it has moved (skip-unchanged). Owned entirely
    // within the scale0 runtime (set here, read in field-overlays.js).
    if (tickScenario && ticksToRun > 0) {
        state.latticeNeedsUpload = true;
        state.fieldDataVersion = (state.fieldDataVersion || 0) + 1;
    } else if (tickScenario) {
        state.latticeNeedsUpload = true;
    }

    // Feed the playback timeline (no-op if the recorder hasn't been created
    // yet or if the scenario didn't tick this frame).
    if (tickScenario && ticksToRun > 0) {
        const rec = getScale0MemoryRecorder();
        const activeScale0 = (state.useFluxMock && mockScale0) ? mockScale0 : mainScale0;
        rec?.onTick(activeScale0);
    }

    return latticeSize;
}
