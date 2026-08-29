/** Shared logarithmic playback-speed scale used by the slider and presets. */

const SLIDER_MIN = 0;
const SLIDER_MAX = 100;
const UNITY_VALUE = 50;
const STEPS_PER_DECADE = 25;

export function sliderValueToSpeed(value) {
    const raw = Math.max(SLIDER_MIN, Math.min(SLIDER_MAX, Number(value)));
    return Math.pow(10, (raw - UNITY_VALUE) / STEPS_PER_DECADE);
}

export function speedToSliderValue(speed) {
    const value = Number(speed);
    if (!Number.isFinite(value) || value <= 0) return UNITY_VALUE;
    return Math.max(
        SLIDER_MIN,
        Math.min(SLIDER_MAX, UNITY_VALUE + STEPS_PER_DECADE * Math.log10(value)),
    );
}

export function speedLabel(speed) {
    if (speed < 0.1) return speed.toFixed(2);
    return speed.toFixed(1);
}
