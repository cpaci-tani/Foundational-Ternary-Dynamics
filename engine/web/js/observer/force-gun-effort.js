// @ts-check
/** Presentation of measured effort only; this never changes the applied force. */
/** @typedef {{fraction:number,percent:number,overloaded:boolean,color:string}} TetherEffort */
const STOPS = [
    { at: 0, rgb: [109, 228, 255] },
    { at: 0.5, rgb: [255, 227, 109] },
    { at: 0.75, rgb: [255, 157, 85] },
    { at: 1, rgb: [255, 82, 107] },
];

/** Actual effort saturates at 100%; a separate pre-clamp measurement identifies
 * demand the gun cannot supply. Missing/stale telemetry must not imply failure.
 * @param {{forceMagnitude:number,maxForce:number,requestedForceMagnitude?:number}|null} telemetry
 * @returns {TetherEffort|null}
 */
export function forceGunEffort(telemetry) {
    if (!telemetry || !Number.isFinite(telemetry.forceMagnitude) || telemetry.forceMagnitude < 0
        || !Number.isFinite(telemetry.maxForce) || telemetry.maxForce <= 0) return null;
    const fraction = Math.min(1, telemetry.forceMagnitude / telemetry.maxForce);
    const requested = telemetry.requestedForceMagnitude;
    const overloaded = typeof requested === 'number' && Number.isFinite(requested)
        && requested > telemetry.maxForce * (1 + 1e-6);
    const upper = STOPS.findIndex(stop => stop.at >= fraction);
    const to = STOPS[Math.max(1, upper)], from = STOPS[Math.max(0, upper - 1)];
    const mix = (fraction - from.at) / (to.at - from.at);
    const rgb = from.rgb.map((value, axis) => Math.round(value + (to.rgb[axis] - value) * mix));
    const color = overloaded ? '#f178ff' : `#${rgb.map(value => value.toString(16).padStart(2, '0')).join('')}`;
    return { fraction, percent: Math.round(fraction * 100), overloaded, color };
}
