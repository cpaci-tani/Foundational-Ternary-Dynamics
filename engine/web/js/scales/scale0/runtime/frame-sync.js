import { getActiveScale0Bridge, getActiveScale0Capability, getActiveLatticeSize } from '../state/store.js';
import { computeManifestationBlink } from './manifestation-flash.js';

const _fluxSlicePlanes = [];
import { MANIFEST_FILL } from '../../../viewport/constants.js';

export function syncRenderableData(ctx, state, viewportAdapter) {
    const latticeSize = getActiveLatticeSize(ctx, state);
    const volUpdateInterval = latticeSize > 96 ? 6 : (latticeSize > 64 ? 4 : (latticeSize > 48 ? 3 : 1));
    if (!state.latticeNeedsUpload || ctx.frameCount % volUpdateInterval !== 0) return latticeSize;

    const activeBridge = getActiveScale0Bridge(ctx, state);
    const activeScale0 = getActiveScale0Capability(ctx, state) ?? ctx.bridge.capabilities.scale0;

    let particleData = activeScale0.getScale0ParticleFrame();

    // Manifestation spawn-flash (added 2026-07-14): a real "this voxel just
    // manifested" visual, reusing the shader blink pipeline Scale-1 already
    // drives. See manifestation-flash.js for the position-diffing approach
    // and its stated imprecision (option (b) of the engine-visualization
    // checklist — a JS-side diff, not yet a WASM-exposed per-voxel age).
    const frameSec = performance.now() / 1000;
    if (particleData && particleData.count > 0) {
        const { phases, rates } = computeManifestationBlink(particleData, frameSec);
        particleData = { ...particleData, phases, rates };
        viewportAdapter.raw?.setPEManifestation?.(true, frameSec, MANIFEST_FILL);
    }

    viewportAdapter.applyParticleFrame(particleData);

    if (state.fieldFlags.showConfinement) {
        viewportAdapter.applyConfinementStrings(activeBridge);
    }

    if (viewportAdapter.isFluxVolumeVisible()) {
        let volume = activeScale0.getScale0FluxVolume();
        // Native FTV2 returns a compact regular-grid descriptor instead of an
        // N^3 buffer. Mock/WASM and legacy FTV1 remain plain typed arrays.
        const volumeLength = volume?.data?.length ?? volume?.length ?? 0;
        if (volumeLength > 0) viewportAdapter.applyFluxVolume(volume, latticeSize);
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
