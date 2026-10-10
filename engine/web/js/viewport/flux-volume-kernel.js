/**
 * @file engine/web/js/viewport/flux-volume-kernel.js
 * @purpose One complete flux-volume presentation frame as a pure function:
 *          activation proxy, drawable-only peak, peak-hold normaliser, and
 *          per-source colour / size / visibility. No DOM and no three.js, so
 *          the same code runs in flux-volume-worker.js and under node:test.
 * @consumers ./flux-volume-worker.js, tests/flux-volume-worker.node.test.mjs
 * @related ./flux-renderer.js — its synchronous path (updateFluxVolume →
 *          _writeFluxAttributesUntil) is the reference this kernel must match
 *          bit for bit; the node test pins the two together.
 *
 * Constants that belong to the renderer (point-size floor and ceiling, the
 * threshold-zero colour floor, the peak-hold decay) arrive in `job`, so the
 * renderer stays their single owner.
 */

import { computeFluxActivation } from './flux-activation.js';
import { fluxToColorInto } from '../fields.js';

/**
 * @param {object} job
 * @param {Float32Array|Float64Array} job.density  x-fastest |J| on the source grid
 * @param {number} job.sourceN                     samples per axis
 * @param {Uint8Array} job.stateMask               1 where a site is manifested
 * @param {Uint8Array|null} job.insideMask         1 where a site is drawable; null = all
 * @param {number} job.thresholdFraction           relative activation-energy cutoff
 * @param {number} job.peakHold                    normaliser held from the previous frame
 * @param {number} job.peakHoldDecay
 * @param {number} job.pointFloor
 * @param {number} job.pointCeiling
 * @param {number[]} job.colorFloor                RGB floor applied at threshold zero
 * @param {object} buffers  scratchA, scratchB, activation (Float64, N^3);
 *                          colors (Float32, 3 N^3); sizes, visibilities (Float32, N^3)
 * @returns {{instantMaxActivation:number, maxActivation:number, visibleCount:number, manifestedCount:number}}
 */
export function computeFluxVolumeFrame(job, buffers) {
    const { density, sourceN, stateMask, insideMask, thresholdFraction } = job;
    const { scratchA, scratchB, activation, colors, sizes, visibilities } = buffers;
    const sourceCount = sourceN * sourceN * sourceN;

    const { instantMax, manifestedCount } = computeFluxActivation(
        density,
        sourceN,
        stateMask,
        scratchA,
        scratchB,
        activation,
    );

    // Normalise only against drawable cells. Energy outside a shaped
    // presentation boundary must not raise the cutoff and suppress a weaker
    // in-bound voxel.
    let instantMaxActivation = instantMax;
    if (insideMask && instantMax > 0) {
        instantMaxActivation = 0;
        for (let sourceIndex = 0; sourceIndex < sourceCount; sourceIndex++) {
            if (insideMask[sourceIndex] !== 1) continue;
            if (activation[sourceIndex] > instantMaxActivation) {
                instantMaxActivation = activation[sourceIndex];
            }
        }
    }

    // Peak-hold-with-decay: fast attack, slow release. An empty frame keeps
    // the prior normalisation.
    const peakHold = job.peakHold || 0;
    const maxActivation = instantMaxActivation > 1e-20
        ? Math.max(instantMaxActivation, peakHold * job.peakHoldDecay)
        : peakHold;

    const pointFloor = job.pointFloor;
    const pointSpan = job.pointCeiling - pointFloor;
    const floorR = job.colorFloor[0];
    const floorG = job.colorFloor[1];
    const floorB = job.colorFloor[2];
    const inspectAll = thresholdFraction === 0;
    let visibleCount = 0;

    for (let sourceIndex = 0; sourceIndex < sourceCount; sourceIndex++) {
        const c3 = sourceIndex * 3;
        const magnitude = Number(density[sourceIndex]);
        const value = activation[sourceIndex];
        const relativeInstantEnergy = instantMaxActivation > 1e-20
            ? (value / instantMaxActivation) ** 2
            : 0;
        const finiteSource = magnitude >= 0 && magnitude < Infinity;
        const inside = insideMask ? insideMask[sourceIndex] === 1 : true;
        const meetsThreshold = inspectAll || relativeInstantEnergy >= thresholdFraction;
        const visible = finiteSource && inside && meetsThreshold;
        visibilities[sourceIndex] = visible ? 1 : 0;
        if (visible) visibleCount++;

        const energyPhase = maxActivation > 1e-20
            ? Math.min(1, (value / maxActivation) ** 2)
            : 0;
        fluxToColorInto(colors, c3, energyPhase, 1);
        if (inspectAll) {
            colors[c3] = Math.max(colors[c3], floorR);
            colors[c3 + 1] = Math.max(colors[c3 + 1], floorG);
            colors[c3 + 2] = Math.max(colors[c3 + 2], floorB);
        }
        sizes[sourceIndex] = pointFloor + pointSpan * energyPhase;
    }

    return { instantMaxActivation, maxActivation, visibleCount, manifestedCount };
}
