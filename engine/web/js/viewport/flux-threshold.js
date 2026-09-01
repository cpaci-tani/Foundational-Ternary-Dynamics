/**
 * Shared Flux Volume threshold response.
 *
 * The control uses a quadratic position curve so the visually important
 * near-zero range receives usable slider travel while the renderer still
 * receives the exact dimensionless activation-energy threshold in [0, 0.5].
 * Threshold never changes spatial sampling: every available voxel is tested
 * and only voxels below the selected activation cutoff are omitted.
 */

export const FLUX_THRESHOLD_MAX = 0.5;

export function clampFluxThreshold(value) {
    const numeric = Number(value);
    if (!Number.isFinite(numeric)) return 0;
    return Math.max(0, Math.min(FLUX_THRESHOLD_MAX, numeric));
}

/** Map the range input's linear position to the displayed physical fraction. */
export function sliderPositionToFluxThreshold(position) {
    const normalized = clampFluxThreshold(position) / FLUX_THRESHOLD_MAX;
    return FLUX_THRESHOLD_MAX * normalized * normalized;
}

/** Inverse of sliderPositionToFluxThreshold for scenario/profile restores. */
export function fluxThresholdToSliderPosition(threshold) {
    const normalized = clampFluxThreshold(threshold) / FLUX_THRESHOLD_MAX;
    return FLUX_THRESHOLD_MAX * Math.sqrt(normalized);
}

export function formatFluxThreshold(value) {
    const threshold = clampFluxThreshold(value);
    if (threshold < 0.0001) return threshold.toFixed(6);
    if (threshold < 0.001) return threshold.toFixed(5);
    return threshold.toFixed(3);
}
