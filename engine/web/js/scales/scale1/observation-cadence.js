/**
 * Scale-1 presentation observation scheduler.
 *
 * Physics ticks and O(N) position reads remain independent. This scheduler
 * governs only expensive exact diagnostics and hierarchy/event serialization.
 */
export function scale1ObservationIntervalMs(particleCount) {
    const count = Math.max(0, Number(particleCount) || 0);
    if (count >= 256) return 200;
    if (count >= 96) return 100;
    return 50;
}

export function shouldRefreshScale1Observation({
    dirty = false,
    hasSnapshot = false,
    tick = 0,
    count = 0,
    lastTick = -1,
    revision = 0,
    lastRevision = -1,
    lastCount = -1,
    nowMs = 0,
    lastObservationMs = Number.NEGATIVE_INFINITY,
} = {}) {
    if (dirty || !hasSnapshot) return true;
    if (tick === lastTick && count === lastCount && revision === lastRevision) return false;
    return nowMs - lastObservationMs >= scale1ObservationIntervalMs(count);
}
