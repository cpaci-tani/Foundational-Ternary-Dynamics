/** Pure gravity-slice preparation shared by slab and dense acquisition paths. */
import { gravitySlice, gravitySliceFromSlab } from '../../analysis/gravity-analysis.js?v=4';
import { transposeAndFlipNN } from './slice-render.js';

function normalize(raw, size) {
    if (!raw) return null;
    const data = transposeAndFlipNN(raw, size);
    let max = 0;
    for (let index = 0; index < data.length; index++) max = Math.max(max, data[index]);
    return { data, max, norm: max > 1e-30 ? 1 / max : 1 };
}

export function prepareGravitySlabSlice(slab, kind, size) {
    return normalize(gravitySliceFromSlab(slab, kind), size);
}

export function prepareGravityVolumeSlice(magnitude, size, axis, mid, kind, rho, spacing) {
    return normalize(gravitySlice(magnitude, size, axis, mid, kind, rho, spacing), size);
}
