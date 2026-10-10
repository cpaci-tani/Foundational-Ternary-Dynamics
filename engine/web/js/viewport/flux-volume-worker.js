/**
 * @file engine/web/js/viewport/flux-volume-worker.js
 * @purpose Module worker that computes one flux-volume presentation frame
 *          off the main thread. Owns the two Float64 scratch grids; every
 *          other buffer arrives by transfer and is transferred back.
 * @consumers ./flux-volume-worker-client.js
 *
 * Protocol (one request in flight at a time):
 *   in   { type:'frame', id, sourceN, density, stateMask|null, insideMask|null,
 *          thresholdFraction, peakHold, peakHoldDecay, pointFloor, pointCeiling,
 *          colorFloor, activation|null, colors|null, sizes|null, visibilities|null }
 *   out  { type:'frame', id, instantMaxActivation, maxActivation, visibleCount,
 *          manifestedCount, computeMs, density, stateMask|null,
 *          activation, colors, sizes, visibilities }
 *   out  { type:'error', id, message }
 * Recycled output arrays of the wrong length are replaced, never resized.
 */

import { computeFluxVolumeFrame } from './flux-volume-kernel.js';

let scratchA = new Float64Array(0);
let scratchB = new Float64Array(0);
let emptyStateMask = new Uint8Array(0);

function sized(array, Type, length) {
    return array && array.length === length ? array : new Type(length);
}

self.onmessage = ({ data }) => {
    if (data?.type !== 'frame') return;
    try {
        const sourceCount = data.sourceN * data.sourceN * data.sourceN;
        if (scratchA.length !== sourceCount) {
            scratchA = new Float64Array(sourceCount);
            scratchB = new Float64Array(sourceCount);
        }
        let stateMask = data.stateMask;
        if (!stateMask) {
            if (emptyStateMask.length !== sourceCount) emptyStateMask = new Uint8Array(sourceCount);
            stateMask = emptyStateMask;
        }
        const activation = sized(data.activation, Float64Array, sourceCount);
        const colors = sized(data.colors, Float32Array, sourceCount * 3);
        const sizes = sized(data.sizes, Float32Array, sourceCount);
        const visibilities = sized(data.visibilities, Float32Array, sourceCount);

        const started = performance.now();
        const result = computeFluxVolumeFrame(
            { ...data, stateMask },
            { scratchA, scratchB, activation, colors, sizes, visibilities },
        );
        // Every array goes back: outputs to be shown, inputs to be reused.
        const transfer = [
            data.density.buffer,
            activation.buffer,
            colors.buffer,
            sizes.buffer,
            visibilities.buffer,
        ];
        if (data.stateMask) transfer.push(data.stateMask.buffer);
        self.postMessage({
            type: 'frame',
            id: data.id,
            ...result,
            computeMs: performance.now() - started,
            density: data.density,
            stateMask: data.stateMask || null,
            activation,
            colors,
            sizes,
            visibilities,
        }, transfer);
    } catch (error) {
        self.postMessage({ type: 'error', id: data.id, message: String(error?.message || error) });
    }
};
