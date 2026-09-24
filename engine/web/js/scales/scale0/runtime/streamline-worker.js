import {
    buildPersistentIndex,
    computeStreamlines,
    sampleFieldMagInto,
} from '../../../fieldlines.js';
import {
    prepareFieldLineKnotCandidate,
    commitFieldLineKnotCandidate,
    discardFieldLineKnotCandidate,
    discardAllFieldLineKnotCandidates,
} from './field-line-knots-worker-state.js';

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

/**
 * Execute the complete worker request protocol. Exporting this pure message
 * boundary lets the Node regression exercise the exact client payload and
 * response shape without a browser Worker shim.
 */
export function executeStreamlineWorkerMessage(data) {
    const { action = 'streamlines', id, kind, fieldData, seeds, opts, knot } = data || {};
    if (action === 'commit-knot') {
        commitFieldLineKnotCandidate(data.candidateId);
        return null;
    }
    if (action === 'discard-knot') {
        discardFieldLineKnotCandidate(data.candidateId);
        return null;
    }
    if (action === 'discard-all-knots') {
        discardAllFieldLineKnotCandidates();
        return null;
    }
    const pooled = computeStreamlines(fieldData, seeds, opts);
    // `provenance.field` is the single canonical field identity. Do not
    // duplicate it in the client request: a duplicate can drift and let the
    // worker attribute a valid sample to the wrong persistent namespace.
    const field = knot?.provenance?.field;
    if (knot && (!['e', 'b', 'flux'].includes(field))) {
        throw new Error(`Invalid field-line knot provenance field: ${field}`);
    }
    // Build only a provisional candidate. The client commits it after its
    // generation/source/tick guard accepts this response, so a late job
    // cannot mutate field identity history.
    const knotResult = knot ? prepareFieldLineKnotCandidate({
        candidateId: id,
        field,
        streamlines: pooled,
        fieldSamples: fieldData,
        provenance: knot.provenance,
        sensitivity: knot.sensitivity,
        perKnotColor: knot.perKnotColor,
        includeLineIds: knot.includeLineIds === true,
    }) : null;
    const lines = copyLiveLines(pooled);
    const response = { id, kind, lines, maxFlux: 0, mags: null, knot: knotResult };
    if (kind === 'flux') {
        const metadata = buildFluxMetadata(fieldData, lines, opts.N, opts.stride);
        response.maxFlux = metadata.maxFlux;
        response.mags = metadata.mags;
    }
    return response;
}

export function workerTransferList(response) {
    if (!response?.lines) return [];
    const transfers = [response.lines.buffer.buffer, response.lines.offsets.buffer, response.lines.lengths.buffer];
    if (response.mags) transfers.push(response.mags.buffer);
    const knotResult = response.knot;
    if (knotResult) {
        const { telemetry, zones, events } = knotResult.snapshot;
        for (const value of [
            telemetry.ids, telemetry.age, telemetry.size, telemetry.peak, telemetry.birth,
            telemetry.fields, telemetry.dirs, telemetry.extents,
            zones.centroids, zones.extents, zones.ids,
            events.tick, events.type, events.nparents, events.nchildren,
            knotResult.lineIds,
        ]) if (ArrayBuffer.isView(value)) transfers.push(value.buffer);
    }
    return transfers;
}

// `self` is absent in Node's direct protocol regression. Keep the browser
// handler thin so test execution and deployed worker behavior share one path.
if (typeof self !== 'undefined') self.onmessage = ({ data }) => {
    const { id, kind } = data || {};
    try {
        const response = executeStreamlineWorkerMessage(data);
        if (response) self.postMessage(response, workerTransferList(response));
    } catch (error) {
        self.postMessage({ id, kind, error: error?.message || String(error) });
    }
};
