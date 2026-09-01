/**
 * Flux-volume activation proxy.
 *
 * [PROXY — visualization] A rendered voxel combines two engine-accounted
 * channels in raw simulation units:
 *
 *   epsilon_i = 1/2 |J_i|^2
 *             + mean_{j in Moore(i), j != i}(1/2 |J_j|^2)
 *             + |s_i| E_REST
 *
 * The Moore mean makes energy surrounding a voxel visible without changing
 * the engine state or the set of voxels the engine computes.  The returned
 * activation amplitude sqrt(2 epsilon_i) is used only for thresholding,
 * point size, and colour phase.  It is not a new dynamical law.
 */

import { E_REST } from '../constants.js';

export const STATE_REST_AMPLITUDE = Math.sqrt(2 * E_REST);

/**
 * Incremental form of computeFluxActivation for dense browser lattices.
 * Each step yields at a caller-supplied deadline, keeping the O(N^3)
 * visualization proxy out of a single animation frame. The result is
 * byte-for-byte equivalent to computeFluxActivation for the same inputs.
 */
export function createFluxActivationStepper(
    density,
    axisCount,
    stateMask,
    scratchA,
    scratchB,
    activation,
) {
    const N = Math.max(1, Math.trunc(Number(axisCount) || 1));
    const count = N * N * N;
    const plane = N * N;
    let phase = 'scan';
    let index = 0;
    let sourceMax = 0;
    let manifestedCount = 0;
    let amplitudeScale = 0;
    let stateEnergyRatio = 0;
    let instantMax = 0;

    const invalid = !density || density.length !== count
        || scratchA.length < count || scratchB.length < count
        || activation.length < count;

    const advancePhase = () => {
        if (phase === 'scan') {
            amplitudeScale = Math.max(
                sourceMax,
                manifestedCount > 0 ? STATE_REST_AMPLITUDE : 0,
            );
            if (!(amplitudeScale > 0) || !Number.isFinite(amplitudeScale)) {
                phase = 'clear';
            } else {
                const stateRatio = STATE_REST_AMPLITUDE / amplitudeScale;
                stateEnergyRatio = stateRatio * stateRatio;
                phase = 'ratios';
            }
        } else if (phase === 'clear') phase = 'done';
        else if (phase === 'ratios') phase = 'x';
        else if (phase === 'x') phase = 'y';
        else if (phase === 'y') phase = 'z';
        else if (phase === 'z') phase = 'done';
        index = 0;
    };

    return {
        step(deadline = Infinity) {
            if (invalid) phase = 'done';
            while (phase !== 'done') {
                const end = Math.min(count, index + 2048);
                for (; index < end; index++) {
                    if (phase === 'scan') {
                        const magnitude = Number(density[index]);
                        if (Number.isFinite(magnitude) && magnitude >= 0 && magnitude > sourceMax) {
                            sourceMax = magnitude;
                        }
                        if (stateMask?.[index]) manifestedCount++;
                    } else if (phase === 'clear') {
                        activation[index] = 0;
                    } else if (phase === 'ratios') {
                        const magnitude = Number(density[index]);
                        const ratio = Number.isFinite(magnitude) && magnitude >= 0
                            ? magnitude / amplitudeScale
                            : 0;
                        scratchA[index] = ratio * ratio;
                    } else if (phase === 'x') {
                        const x = index % N;
                        let sum = scratchA[index];
                        if (x > 0) sum += scratchA[index - 1];
                        if (x + 1 < N) sum += scratchA[index + 1];
                        scratchB[index] = sum;
                    } else if (phase === 'y') {
                        const y = Math.floor(index / N) % N;
                        let sum = scratchB[index];
                        if (y > 0) sum += scratchB[index - N];
                        if (y + 1 < N) sum += scratchB[index + N];
                        scratchA[index] = sum;
                    } else {
                        const z = Math.floor(index / plane);
                        const y = Math.floor(index / N) % N;
                        const x = index % N;
                        let sum = scratchA[index];
                        let zNeighbours = 1;
                        if (z > 0) { sum += scratchA[index - plane]; zNeighbours++; }
                        if (z + 1 < N) { sum += scratchA[index + plane]; zNeighbours++; }
                        const mooreCount = (1 + (x > 0 ? 1 : 0) + (x + 1 < N ? 1 : 0))
                            * (1 + (y > 0 ? 1 : 0) + (y + 1 < N ? 1 : 0))
                            * zNeighbours;
                        const magnitude = Number(density[index]);
                        const ownAmplitudeRatio = Number.isFinite(magnitude) && magnitude >= 0
                            ? magnitude / amplitudeScale
                            : 0;
                        const ownRatio = ownAmplitudeRatio * ownAmplitudeRatio;
                        const surroundingRatio = mooreCount > 1
                            ? Math.max(0, sum - ownRatio) / (mooreCount - 1)
                            : 0;
                        const totalEnergyRatio = ownRatio + surroundingRatio
                            + (stateMask?.[index] ? stateEnergyRatio : 0);
                        const value = amplitudeScale * Math.sqrt(Math.max(0, totalEnergyRatio));
                        activation[index] = value;
                        if (value > instantMax) instantMax = value;
                    }
                }
                if (index >= count) advancePhase();
                if (performance.now() >= deadline) break;
            }
            return {
                done: phase === 'done',
                phase,
                instantMax,
                sourceMax,
                manifestedCount,
            };
        },
    };
}

/**
 * Fill `activation` with sqrt(2 epsilon_i) for every source voxel.
 * `scratchA` and `scratchB` are reused Float64 buffers.  A separable
 * three-cell box filter computes the complete 3x3x3 Moore sum in O(N^3)
 * rather than visiting 27 neighbours for every voxel.
 */
export function computeFluxActivation(
    density,
    axisCount,
    stateMask,
    scratchA,
    scratchB,
    activation,
) {
    const N = Math.max(1, Math.trunc(Number(axisCount) || 1));
    const count = N * N * N;
    if (!density || density.length !== count
        || scratchA.length < count || scratchB.length < count
        || activation.length < count) {
        return { instantMax: 0, sourceMax: 0, manifestedCount: 0 };
    }

    let sourceMax = 0;
    let manifestedCount = 0;
    for (let i = 0; i < count; i++) {
        const magnitude = Number(density[i]);
        if (Number.isFinite(magnitude) && magnitude >= 0 && magnitude > sourceMax) {
            sourceMax = magnitude;
        }
        if (stateMask?.[i]) manifestedCount++;
    }

    // Work in a frame-local amplitude scale so squaring very large but finite
    // fields cannot overflow. Multiplying the final root-energy ratio by the
    // same scale restores the physical relative amplitude for peak hold.
    const amplitudeScale = Math.max(
        sourceMax,
        manifestedCount > 0 ? STATE_REST_AMPLITUDE : 0,
    );
    if (!(amplitudeScale > 0) || !Number.isFinite(amplitudeScale)) {
        activation.fill(0, 0, count);
        return { instantMax: 0, sourceMax, manifestedCount };
    }

    for (let i = 0; i < count; i++) {
        const magnitude = Number(density[i]);
        const ratio = Number.isFinite(magnitude) && magnitude >= 0
            ? magnitude / amplitudeScale
            : 0;
        scratchA[i] = ratio * ratio;
    }

    const plane = N * N;

    // X sum.
    for (let z = 0; z < N; z++) {
        for (let y = 0; y < N; y++) {
            const base = z * plane + y * N;
            for (let x = 0; x < N; x++) {
                let sum = scratchA[base + x];
                if (x > 0) sum += scratchA[base + x - 1];
                if (x + 1 < N) sum += scratchA[base + x + 1];
                scratchB[base + x] = sum;
            }
        }
    }

    // Y sum.
    for (let z = 0; z < N; z++) {
        for (let y = 0; y < N; y++) {
            const base = z * plane + y * N;
            for (let x = 0; x < N; x++) {
                let sum = scratchB[base + x];
                if (y > 0) sum += scratchB[base - N + x];
                if (y + 1 < N) sum += scratchB[base + N + x];
                scratchA[base + x] = sum;
            }
        }
    }

    // Z sum, then split own energy from the mean of the available surrounding
    // Moore sites. This keeps an energetic source stronger than the halo it
    // activates while still making that surrounding energy visible.
    const stateRatio = STATE_REST_AMPLITUDE / amplitudeScale;
    const stateEnergyRatio = stateRatio * stateRatio;
    let instantMax = 0;
    for (let z = 0; z < N; z++) {
        for (let y = 0; y < N; y++) {
            const base = z * plane + y * N;
            for (let x = 0; x < N; x++) {
                const index = base + x;
                let sum = scratchA[index];
                let neighbours = 1;
                if (z > 0) { sum += scratchA[index - plane]; neighbours++; }
                if (z + 1 < N) { sum += scratchA[index + plane]; neighbours++; }
                const xNeighbours = 1 + (x > 0 ? 1 : 0) + (x + 1 < N ? 1 : 0);
                const yNeighbours = 1 + (y > 0 ? 1 : 0) + (y + 1 < N ? 1 : 0);
                const zNeighbours = neighbours;
                const mooreCount = xNeighbours * yNeighbours * zNeighbours;
                const magnitude = Number(density[index]);
                const ownAmplitudeRatio = Number.isFinite(magnitude) && magnitude >= 0
                    ? magnitude / amplitudeScale
                    : 0;
                const ownRatio = ownAmplitudeRatio * ownAmplitudeRatio;
                const surroundingRatio = mooreCount > 1
                    ? Math.max(0, sum - ownRatio) / (mooreCount - 1)
                    : 0;
                const totalEnergyRatio = ownRatio + surroundingRatio
                    + (stateMask?.[index] ? stateEnergyRatio : 0);
                const value = amplitudeScale * Math.sqrt(Math.max(0, totalEnergyRatio));
                activation[index] = value;
                if (value > instantMax) instantMax = value;
            }
        }
    }

    return { instantMax, sourceMax, manifestedCount };
}
