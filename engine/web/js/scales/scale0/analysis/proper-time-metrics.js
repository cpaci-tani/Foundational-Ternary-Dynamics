/**
 * Pure reduction for one coherent proper-time sampler observation.
 * Acquisition owns cadence and sampler demand; this module owns only the
 * numeric reduction and preserves source provenance for every consumer.
 */

function exactCounter(value) {
    if (value === null || value === undefined || value === '') return null;
    const n = Number(value);
    return Number.isSafeInteger(n) && n >= 0 ? n : null;
}

function sampleMeta(sample) {
    const provenance = sample?.provenance || {};
    return {
        sampleTick: exactCounter(sample?.sampleTick ?? provenance.sampleTick ?? provenance.tick),
        sourceEpoch: exactCounter(sample?.sourceEpoch ?? provenance.sourceEpoch),
        epoch: exactCounter(sample?.epoch ?? provenance.epoch),
        source: sample?.source ?? provenance.source ?? null,
    };
}

function sharedCounter(rows, key) {
    const values = rows.map(row => row[key]);
    const present = values.filter(value => value !== null);
    if (!present.length) return { coherent: true, value: null };
    // A populated cache row without provenance cannot borrow another row's
    // tick/epoch. All requested channels must either carry the counter or all
    // omit it (the direct synchronous owner supplies the common boundary).
    if (present.length !== values.length) return { coherent: false, value: null };
    return { coherent: present.every(value => value === present[0]), value: present[0] };
}

function sharedSource(rows) {
    const values = rows.map(row => row.source);
    const present = values.filter(Boolean);
    if (!present.length) return { coherent: true, value: null };
    if (present.length !== values.length) return { coherent: false, value: null };
    return { coherent: present.every(value => value === present[0]), value: present[0] };
}

function sampleCount(sample) {
    return Math.max(0, Math.min(sample?.count | 0, sample?.values?.length || 0));
}

function scalarStats(sample) {
    const count = sampleCount(sample);
    if (!count) return { count: 0, mean: Number.NaN, min: Number.NaN, max: Number.NaN };
    let sum = 0, min = Infinity, max = -Infinity;
    for (let i = 0; i < count; i++) {
        const value = Number(sample.values[i]);
        if (!Number.isFinite(value)) {
            return { count, mean: Number.NaN, min: Number.NaN, max: Number.NaN };
        }
        sum += value;
        if (value < min) min = value;
        if (value > max) max = value;
    }
    return { count, mean: sum / count, min, max };
}

export function reduceProperTimeSamples({ tau, lapse, phase } = {}) {
    const rows = [tau, lapse, phase].map(sampleMeta);
    const tick = sharedCounter(rows, 'sampleTick');
    const sourceEpoch = sharedCounter(rows, 'sourceEpoch');
    const epoch = sharedCounter(rows, 'epoch');
    const source = sharedSource(rows);
    const coherent = tick.coherent && sourceEpoch.coherent && epoch.coherent && source.coherent;

    // Reject mixed frames before touching their arrays. Besides avoiding wasted
    // work, this prevents a scratch-backed older row from being scanned after
    // the owner has already identified the observation as incoherent.
    const tauStats = coherent ? scalarStats(tau)
        : { count: sampleCount(tau), mean: Number.NaN, min: Number.NaN, max: Number.NaN };
    const lapseStats = coherent ? scalarStats(lapse)
        : { count: sampleCount(lapse), mean: Number.NaN, min: Number.NaN, max: Number.NaN };
    const phaseCount = sampleCount(phase);
    let dbPhaseMean = Number.NaN;
    let dbPhaseCircVar = Number.NaN;
    if (coherent && phaseCount > 0) {
        let sumCos = 0, sumSin = 0, valid = true;
        for (let i = 0; i < phaseCount; i++) {
            const value = Number(phase.values[i]);
            if (!Number.isFinite(value)) { valid = false; break; }
            sumCos += Math.cos(value);
            sumSin += Math.sin(value);
        }
        if (valid) {
            const meanCos = sumCos / phaseCount, meanSin = sumSin / phaseCount;
            const resultant = Math.sqrt(meanCos * meanCos + meanSin * meanSin);
            if (resultant > 1e-6) {
                const twoPi = 2 * Math.PI;
                dbPhaseMean = (Math.atan2(meanSin, meanCos) + twoPi) % twoPi;
            }
            dbPhaseCircVar = Math.max(0, Math.min(1, 1 - resultant));
        }
    }

    return {
        coherent,
        sampleTick: coherent ? tick.value : null,
        sourceEpoch: coherent ? sourceEpoch.value : null,
        epoch: coherent ? epoch.value : null,
        source: coherent ? source.value : null,
        hasField: tauStats.count > 0 || lapseStats.count > 0 || phaseCount > 0,
        tauCount: tauStats.count,
        lapseCount: lapseStats.count,
        phaseCount,
        tauStride: tau?.effectiveStride ?? null,
        lapseStride: lapse?.effectiveStride ?? null,
        phaseStride: phase?.effectiveStride ?? null,
        properTimeMean: coherent ? tauStats.mean : Number.NaN,
        properTimeMin: coherent ? tauStats.min : Number.NaN,
        properTimeMax: coherent ? tauStats.max : Number.NaN,
        lapseMean: coherent ? lapseStats.mean : Number.NaN,
        dbPhaseMean: coherent ? dbPhaseMean : Number.NaN,
        dbPhaseCircVar: coherent ? dbPhaseCircVar : Number.NaN,
    };
}
