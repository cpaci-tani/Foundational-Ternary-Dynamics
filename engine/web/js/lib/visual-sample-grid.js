/**
 * JS twin of engine/include/ftd/visual_sample_grid.h.
 * Used to place compact FTV2 samples at the same centre-anchored voxel
 * coordinates the native sampler emits, including when an older 16-byte
 * FTV2 header omitted origin.
 */

const K_MAX_DENSE_VISUAL_SAMPLES = 262144;

export function visualSampleGrid(N, requestedStride, interior = false) {
    let stride = Math.max(1, Math.trunc(Number(requestedStride) || 1));
    const extent = Math.max(0, N - (interior ? 2 : 0));
    const samplesFor = (s) => {
        const per = Math.floor((extent + s - 1) / s);
        return per * per * per;
    };
    while (samplesFor(stride) > K_MAX_DENSE_VISUAL_SAMPLES) stride += 1;
    const lo = interior ? 1 : 0;
    const hi = interior ? N - 2 : N - 1;
    if (hi < lo) return { stride, origin: 0, count: 0 };
    const center = Math.floor((N - 1) / 2);
    const origin = center - Math.floor((center - lo) / stride) * stride;
    const count = Math.floor((hi - origin) / stride) + 1;
    return { stride, origin, count };
}
