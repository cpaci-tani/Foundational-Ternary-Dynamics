export function syncRenderableData(ctx, state, viewportAdapter) {
    const latticeSize = ctx.bridge.latticeSize || 32;
    const volUpdateInterval = latticeSize > 96 ? 6 : (latticeSize > 64 ? 4 : (latticeSize > 48 ? 3 : 1));
    if (!state.latticeNeedsUpload || ctx.frameCount % volUpdateInterval !== 0) return latticeSize;

    const mainScale0 = ctx.bridge.capabilities.scale0;
    const mockScale0 = state.fluxMock?.capabilities?.scale0 || null;

    let particleData = mainScale0.getScale0ParticleFrame();
    if (state.useFluxMock && (!particleData || particleData.count === 0) && mockScale0) {
        const mockData = mockScale0.getScale0ParticleFrame();
        if (mockData && mockData.count > 0) particleData = mockData;
    }
    viewportAdapter.applyParticleFrame(particleData);

    if (state.fieldFlags.showConfinement) {
        viewportAdapter.applyConfinementStrings(ctx.bridge);
    }

    if (viewportAdapter.isFluxVolumeVisible()) {
        let volume;
        if (state.useFluxMock && mockScale0) {
            volume = mockScale0.getScale0FluxVolume();
            if (!volume || volume.length === 0) volume = mainScale0.getScale0FluxVolume();
        } else {
            volume = mainScale0.getScale0FluxVolume();
            if ((!volume || volume.length === 0) && mockScale0) volume = mockScale0.getScale0FluxVolume();
        }
        if (volume && volume.length > 0) viewportAdapter.applyFluxVolume(volume, latticeSize);
    }

    if (viewportAdapter.isFluxSliceVisible()) {
        const sliceIdx = Math.floor(latticeSize / 2);
        let slice;
        if (state.useFluxMock && mockScale0) {
            slice = mockScale0.getScale0FluxSlice(1, sliceIdx);
            if (!slice || slice.length === 0) slice = mainScale0.getScale0FluxSlice(1, sliceIdx);
        } else {
            slice = mainScale0.getScale0FluxSlice(1, sliceIdx);
            if ((!slice || slice.length === 0) && mockScale0) slice = mockScale0.getScale0FluxSlice(1, sliceIdx);
        }
        if (slice && slice.length > 0) viewportAdapter.applyFluxSlice(slice, latticeSize, 1, sliceIdx);
    }

    state.latticeNeedsUpload = false;
    return latticeSize;
}
