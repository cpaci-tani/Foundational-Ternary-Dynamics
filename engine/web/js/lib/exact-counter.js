/** Lossless uint64 transport counters. JSON uses safe numbers or decimal strings. */
const MAX_U64 = (1n << 64n) - 1n;
const MAX_SAFE = BigInt(Number.MAX_SAFE_INTEGER);

export function exactCounter(value) {
    let integer;
    if (typeof value === 'number') {
        if (!Number.isSafeInteger(value) || value < 0) return null;
        return value;
    }
    if (typeof value === 'bigint') integer = value;
    else if (typeof value === 'string' && /^(0|[1-9][0-9]*)$/.test(value)) {
        if (value.length > 20) return null;
        integer = BigInt(value);
    } else return null;
    if (integer < 0n || integer > MAX_U64) return null;
    return integer <= MAX_SAFE ? Number(integer) : integer.toString();
}

/** Unknown values are unordered; callers must decide whether absence is allowed. */
export function compareExactCounters(left, right) {
    const a = exactCounter(left), b = exactCounter(right);
    if (a === null || b === null) return null;
    const x = BigInt(a), y = BigInt(b);
    return x < y ? -1 : x > y ? 1 : 0;
}

export function safeCounterNumber(value) {
    const normalized = exactCounter(value);
    return typeof normalized === 'number' ? normalized : null;
}

const COUNTER_KEYS = new Set([
    'epoch', 'sourceEpoch', 'telemetryEpoch', 'telemetrySourceEpoch',
    'stateVersion', 'state_version', 'backendStateVersion', 'snapshotVersion',
    'telemetrySnapshotVersion', 'tick', 'sampleTick',
    '_requestId', 'requestId',
]);

/** Reject rounded/ambiguous counters before they can enter a cache or match an RPC. */
export function normalizeNativeCounters(value) {
    if (!value || typeof value !== 'object' || Array.isArray(value)) return value;
    for (const [key, child] of Object.entries(value)) {
        if (COUNTER_KEYS.has(key) && child !== null && child !== undefined) {
            const normalized = exactCounter(child);
            if (normalized === null) throw new TypeError(`Invalid exact counter: ${key}`);
            value[key] = normalized;
        } else if (child && typeof child === 'object' && !Array.isArray(child)) {
            normalizeNativeCounters(child);
        }
    }
    return value;
}

/** Samples must declare matching source/epoch/tick metadata before fields are combined. */
export function commonSampleProvenance(samples) {
    if (!samples.length) return null;
    const identity = sample => {
        const source = sample?.provenance?.source ?? sample?.source ?? sample?.backend;
        const nativeInstanceId = sample?.provenance?.nativeInstanceId ?? sample?.nativeInstanceId ?? null;
        const sourceEpoch = exactCounter(sample?.provenance?.sourceEpoch ?? sample?.sourceEpoch);
        const epoch = exactCounter(sample?.provenance?.epoch ?? sample?.epoch);
        const tick = exactCounter(sample?.provenance?.sampleTick ?? sample?.sampleTick);
        return typeof source === 'string' && source && sourceEpoch !== null
            && epoch !== null && tick !== null ? { source, nativeInstanceId, sourceEpoch, epoch, sampleTick: tick } : null;
    };
    const first = identity(samples[0]);
    if (!first) return null;
    return samples.every(sample => {
        const other = identity(sample);
        return other && other.source === first.source
            && other.nativeInstanceId === first.nativeInstanceId
            && compareExactCounters(other.sourceEpoch, first.sourceEpoch) === 0
            && compareExactCounters(other.epoch, first.epoch) === 0
            && compareExactCounters(other.sampleTick, first.sampleTick) === 0;
    }) ? Object.freeze(first) : null;
}
