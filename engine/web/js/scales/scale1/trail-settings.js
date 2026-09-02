/**
 * Presentation-only trajectory-history settings for Scale 1.
 *
 * All durations are expressed in native particle-engine ticks. This keeps a
 * trail's visible history stable when display FPS changes and avoids treating
 * rendered frames as physical time.
 */

export const TRAIL_HISTORY_CAPACITY = 1200;
export const TRAIL_GLOBAL_SAMPLE_BUDGET = 96000;

export function trailCapacityForPopulation(population) {
    const count = Math.max(1, Math.floor(Number(population) || 1));
    return Math.max(24, Math.min(
        TRAIL_HISTORY_CAPACITY,
        Math.floor(TRAIL_GLOBAL_SAMPLE_BUDGET / count),
    ));
}

export const DEFAULT_TRAIL_SETTINGS = Object.freeze({
    renderMode: 'breadcrumbs',
    historyTicks: 240,
    sampleEveryTicks: 1,
    disappearDelayTicks: 120,
    opacity: 0.72,
    pointSize: 0.34,
    fadeExponent: 1.35,
});

export const TRAIL_RENDER_MODES = Object.freeze([
    'breadcrumbs',
    'lines',
    'energy',
]);

const LIMITS = Object.freeze({
    historyTicks: [10, TRAIL_HISTORY_CAPACITY],
    sampleEveryTicks: [1, 24],
    disappearDelayTicks: [0, 1200],
    opacity: [0.05, 1],
    pointSize: [0.08, 1.2],
    fadeExponent: [0.35, 4],
});

function finite(value, fallback) {
    const number = Number(value);
    return Number.isFinite(number) ? number : fallback;
}

function clamp(value, [minimum, maximum]) {
    return Math.min(maximum, Math.max(minimum, value));
}

export function normalizeTrailSettings(candidate = {}, base = DEFAULT_TRAIL_SETTINGS) {
    const requestedMode = String(candidate?.renderMode ?? base?.renderMode
        ?? DEFAULT_TRAIL_SETTINGS.renderMode);
    const normalized = {
        renderMode: TRAIL_RENDER_MODES.includes(requestedMode)
            ? requestedMode : DEFAULT_TRAIL_SETTINGS.renderMode,
    };
    for (const key of Object.keys(LIMITS)) {
        const fallback = finite(base?.[key], DEFAULT_TRAIL_SETTINGS[key]);
        normalized[key] = clamp(finite(candidate?.[key], fallback), LIMITS[key]);
    }
    normalized.historyTicks = Math.round(normalized.historyTicks);
    normalized.sampleEveryTicks = Math.round(normalized.sampleEveryTicks);
    normalized.disappearDelayTicks = Math.round(normalized.disappearDelayTicks);
    return normalized;
}

/** Fade a removed particle's complete history over its configured retention. */
export function trailRetentionAlpha(trail, currentTick, settings = DEFAULT_TRAIL_SETTINGS) {
    if (!Number.isFinite(trail?.inactiveSinceTick)) return 1;
    const delay = Math.max(0, Number(settings.disappearDelayTicks) || 0);
    if (delay === 0) return 0;
    const age = Math.max(0, Number(currentTick) - trail.inactiveSinceTick);
    return Math.max(0, 1 - age / delay);
}
