/** Bounded presentation settings. Camera feedback never enters world state or optical history. */
/** @param {unknown} value @param {number} fallback @param {number} low @param {number} high */
function bounded(value, fallback, low, high) { return typeof value === 'number' && Number.isFinite(value) ? Math.max(low, Math.min(high, value)) : fallback; }
/** @param {{feedbackEnabled?:boolean,feedbackLayers?:number,feedbackDepth?:number,feedbackStrength?:number,feedbackScale?:number}} settings */
export function feedbackConfiguration(settings = {}) {
    return {
        enabled: settings.feedbackEnabled === true,
        layers: Math.round(bounded(settings.feedbackLayers, 2, 1, 3)),
        depth: Math.round(bounded(settings.feedbackDepth, 3, 3, 5)),
        strength: bounded(settings.feedbackStrength, 0.45, 0, 1),
        scale: bounded(settings.feedbackScale, 0.6, 0.25, 1),
    };
}
