import {
    buildPersistentIndex,
    computeStreamlines,
    sampleFieldMagInto,
} from '../../../fieldlines.js';

function liveFloatCount(lines) {
    if (!lines.count) return 0;
    const last = lines.count - 1;
    return lines.offsets[last] + lines.lengths[last];
}

function copyLiveLines(lines) {
    const floats = liveFloatCount(lines);
    return {
        count: lines.count,
        buffer: lines.buffer.slice(0, floats),
        offsets: lines.offsets.slice(0, lines.count),
        lengths: lines.lengths.slice(0, lines.count),
    };
}

function buildFluxMetadata(fieldData, lines, N, stride) {
    let maxFlux = 0;
    for (let i = 0; i < fieldData.count; i++) {
        const x = fieldData.vectors[i * 3];
        const y = fieldData.vectors[i * 3 + 1];
        const z = fieldData.vectors[i * 3 + 2];
        maxFlux = Math.max(maxFlux, Math.sqrt(x * x + y * y + z * z));
    }
    const mags = new Float32Array(lines.buffer.length / 3);
    if (lines.count) {
        const index = buildPersistentIndex(
            fieldData.positions, fieldData.vectors, fieldData.count, N, stride,
        );
        for (let li = 0; li < lines.count; li++) {
            const base = lines.offsets[li];
            const points = lines.lengths[li] / 3;
            const magBase = base / 3;
            for (let i = 0; i < points; i++) {
                mags[magBase + i] = sampleFieldMagInto(
                    index,
                    lines.buffer[base + i * 3],
                    lines.buffer[base + i * 3 + 1],
                    lines.buffer[base + i * 3 + 2],
                );
            }
        }
    }
    return { maxFlux, mags };
}

self.onmessage = ({ data }) => {
    const { id, kind, fieldData, seeds, opts } = data || {};
    try {
        const pooled = computeStreamlines(fieldData, seeds, opts);
        const lines = copyLiveLines(pooled);
        const response = { id, kind, lines, maxFlux: 0, mags: null };
        if (kind === 'flux') {
            const metadata = buildFluxMetadata(fieldData, lines, opts.N, opts.stride);
            response.maxFlux = metadata.maxFlux;
            response.mags = metadata.mags;
        }
        const transfers = [lines.buffer.buffer, lines.offsets.buffer, lines.lengths.buffer];
        if (response.mags) transfers.push(response.mags.buffer);
        self.postMessage(response, transfers);
    } catch (error) {
        self.postMessage({ id, kind, error: error?.message || String(error) });
    }
};
