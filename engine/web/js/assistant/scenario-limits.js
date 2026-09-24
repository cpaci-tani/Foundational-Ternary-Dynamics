// @ts-check
/** Fixed, once-approved scenario protocols. These are distinct from adaptive
 * AI runs and ordinary single-command step limits in contracts.js. */
export const SCENARIO_LIMITS = Object.freeze({
    ticks: 100000,
    intervals: 200,
    durationMs: 1800000,
    chunkTicks: 128,
});

export const SCENARIO_DEFAULTS = Object.freeze({ ticks: 10000, sampleEvery: 250 });

/** Includes a final partial interval, plus one separately retained baseline.
 * @param {unknown} ticks @param {unknown} sampleEvery
 */
export function validateScenarioProtocol(ticks, sampleEvery) {
    if (typeof ticks !== 'number' || !Number.isSafeInteger(ticks) || ticks < 1 || ticks > SCENARIO_LIMITS.ticks)
        throw new Error(`Ticks must be an exact integer from 1 to ${SCENARIO_LIMITS.ticks}.`);
    if (typeof sampleEvery !== 'number' || !Number.isSafeInteger(sampleEvery) || sampleEvery < 1 || sampleEvery > ticks)
        throw new Error('Sample interval must be an exact integer from 1 to the requested tick count.');
    const intervals = Math.ceil(ticks / sampleEvery);
    if (intervals > SCENARIO_LIMITS.intervals)
        throw new Error(`Sampling exceeds ${SCENARIO_LIMITS.intervals} measured intervals plus the baseline. Choose a sample interval of at least ${Math.ceil(ticks / SCENARIO_LIMITS.intervals)} ticks.`);
    return { ticks, sampleEvery, intervals };
}
