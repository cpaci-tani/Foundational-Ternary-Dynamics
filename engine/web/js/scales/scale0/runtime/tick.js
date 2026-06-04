import { getScale0MemoryRecorder } from '../controller.js';

export function advanceSimulation(ctx, state) {
    const latticeSize = ctx.bridge.latticeSize || 32;

    // Worker-backed physics (Phase 2): the MockBridgeProxy's worker self-ticks on
    // its own clock. Forward the desired run state (deduped in the proxy) and
    // drive overlay/render refresh from the worker's frame counter, then return —
    // the in-thread tick path below is for non-worker scenarios only.
    const fm = state.fluxMock;
    if (fm && fm.isWorker && state.useFluxMock) {
        const run = ctx.running && !state.scrubbing && !state.rendering;
        if (typeof fm.setRunning === 'function') fm.setRunning(run);
        const fc = fm.frameCounter || 0;
        if (fc !== state._lastWorkerFrame) {
            state._lastWorkerFrame = fc;
            state.latticeNeedsUpload = true;
            state.fieldDataVersion = (state.fieldDataVersion || 0) + 1;
            // Feed the playback timeline so scrub works for worker scenarios. The
            // recorder throttles internally; getScale0Snapshot reads the shadow's
            // SAB views and copies out (same call the in-thread path makes below).
            if (run) {
                const rec = getScale0MemoryRecorder();
                rec?.onTick(fm.capabilities.scale0);
            }
        }
        return latticeSize;
    }

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

    // Past the global-pause / scrubbing / rendering guards above, `running` is
    // true, so physics advances this frame. Tick the WASM bridge unless a JS
    // flux mock owns the physics; tick the mock only when it IS the source.
    for (let i = 0; i < ticksToRun; i++) {
        if (!state.useFluxMock) mainScale0.tickScale0();
        if (tickMock) mockScale0.tickScale0();
    }

    // Mark the lattice for re-upload only if a tick actually advanced — when no
    // tick ran this frame, the lattice contents haven't changed, so skipping the
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
    state.latticeNeedsUpload = true;
    if (ticksToRun > 0) {
        state.fieldDataVersion = (state.fieldDataVersion || 0) + 1;
    }

    // Feed the playback timeline (no-op if the recorder hasn't been created
    // yet or if the scenario didn't tick this frame).
    if (ticksToRun > 0) {
        const rec = getScale0MemoryRecorder();
        const activeScale0 = (state.useFluxMock && mockScale0) ? mockScale0 : mainScale0;
        rec?.onTick(activeScale0);
    }

    return latticeSize;
}
