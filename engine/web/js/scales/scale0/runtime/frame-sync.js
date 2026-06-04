const _fluxSlicePlanes = [];

export function syncRenderableData(ctx, state, viewportAdapter) {
    const latticeSize = ctx.bridge.latticeSize || 32;
    const volUpdateInterval = latticeSize > 96 ? 6 : (latticeSize > 64 ? 4 : (latticeSize > 48 ? 3 : 1));
    if (!state.latticeNeedsUpload || ctx.frameCount % volUpdateInterval !== 0) return latticeSize;

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;
    // Active physics owner — when state.useFluxMock is true the mock is
    // the source being ticked (see runtime/tick.js::advanceSimulation), so
    // every read here must prefer the mock. Sampling ctx.bridge in mock
    // mode silently shows stale/frozen data — same bug class as the
    // flux-slice panel had before its 2026-04-26 fix.
    const activeBridge = (state.useFluxMock && state.fluxMock) ? state.fluxMock : ctx.bridge;
    const activeScale0 = (state.useFluxMock && mockScale0) ? mockScale0 : mainScale0;

    let particleData = activeScale0.getScale0ParticleFrame();
    viewportAdapter.applyParticleFrame(particleData);

    if (state.fieldFlags.showConfinement) {
        viewportAdapter.applyConfinementStrings(activeBridge);
    }

    if (viewportAdapter.isFluxVolumeVisible()) {
        let volume = activeScale0.getScale0FluxVolume();
        if (volume && volume.length > 0) viewportAdapter.applyFluxVolume(volume, latticeSize);
    }

    if (viewportAdapter.isFluxSliceVisible()) {
        const sliceIdx = Math.floor(latticeSize / 2);
        // Gather every enabled axis (0=yz, 1=xz, 2=xy) and pack the mid-planes
        // into the dedicated slice mesh in one update. Default is all three.
        const axes = viewportAdapter.getEnabledFluxSliceAxes();
        _fluxSlicePlanes.length = 0;
        for (const axis of axes) {
            const slice = activeScale0.getScale0FluxSlice(axis, sliceIdx);
            if (slice && slice.length > 0) _fluxSlicePlanes.push({ axis, data: slice });
        }
        if (_fluxSlicePlanes.length) viewportAdapter.applyFluxSlices(_fluxSlicePlanes, latticeSize, sliceIdx);
    }

    state.latticeNeedsUpload = false;
    return latticeSize;
}
