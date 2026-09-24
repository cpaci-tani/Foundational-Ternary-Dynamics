/** Validate and interpret one completed Gravity instrument observation. */
import { aggregateMetrics, forceMagnitudes } from '../../analysis/gravity-analysis.js?v=4';

const isTick = value => Number.isSafeInteger(value) && value >= 0;
const isFiniteNumber = value => typeof value === 'number' && Number.isFinite(value);

function validSlab(slab, axis, latticeSize, observation) {
    const mid = latticeSize >> 1;
    const startPlane = Math.max(0, mid - 1);
    const planeCount = Math.min(latticeSize - 1, mid + 1) - startPlane + 1;
    const meta = slab?.metadata;
    return slab?.axis === axis && slab.index === mid && slab.N === latticeSize
        && slab.startPlane === startPlane && slab.planeCount === planeCount
        && slab.data instanceof Float64Array && slab.data.length === planeCount * latticeSize * latticeSize
        && slab.maxRho === observation.maxRho
        && meta?.sampleTick === observation.sampleTick
        && meta?.source === observation.source
        && meta?.sourceEpoch === observation.sourceEpoch
        && meta?.configurationToken === observation.configurationToken
        && meta?.loadGeneration === observation.loadGeneration
        && meta?.dataVersion === observation.dataVersion;
}

function validAggregate(aggregate) {
    return !!aggregate && typeof aggregate === 'object'
        && typeof aggregate.active === 'boolean'
        && typeof aggregate.requested === 'boolean'
        && ['latencyMax', 'latencyMean', 'fMin', 'gammaMax', 'dilationMaxPct']
            .every(key => isFiniteNumber(aggregate[key]))
        && isTick(aggregate.voxelCount);
}

export function readGravityObservation(observation, latticeSize, stride) {
    if (!Number.isSafeInteger(latticeSize) || latticeSize < 1
        || !Number.isSafeInteger(stride) || stride < 1
        || !observation || observation.source !== 'wasm-worker'
        || !isTick(observation.sampleTick)
        || observation.N !== latticeSize || observation.stride !== stride
        || !isTick(observation.sourceEpoch)
        || observation.configurationToken !== observation.sourceEpoch
        || !Number.isSafeInteger(observation.loadGeneration) || observation.loadGeneration < 1
        || !isTick(observation.dataVersion)
        || !isFiniteNumber(observation.maxRho) || observation.maxRho < 1e-30
        || observation.slabs?.length !== 3) return null;
    for (let axis = 0; axis < 3; axis++) {
        if (!validSlab(observation.slabs[axis], axis, latticeSize, observation)) return null;
    }
    const { latency, kretschmann, gravity, gravityMetricAgg } = observation;
    for (const [sample, key, components] of [[latency, 'values', 1], [kretschmann, 'values', 1], [gravity, 'vectors', 3]]) {
        if (!sample || !Number.isSafeInteger(sample.count) || sample.count < 0
            || !ArrayBuffer.isView(sample[key]) || sample[key].length < sample.count * components) return null;
        for (let i = 0; i < sample.count * components; i++) if (!Number.isFinite(sample[key][i])) return null;
    }
    if (!validAggregate(gravityMetricAgg)) return null;
    return {
        observation,
        tick: observation.sampleTick,
        stamp: `${observation.sourceEpoch}:${observation.dataVersion}:${observation.sampleTick}`,
        metrics: aggregateMetrics({ latencyVals: latency.values, latencyCount: latency.count,
            kretVals: kretschmann.values, kretCount: kretschmann.count,
            forceMags: forceMagnitudes(gravity.vectors, gravity.count), forceCount: gravity.count }),
        aggregate: gravityMetricAgg,
    };
}
