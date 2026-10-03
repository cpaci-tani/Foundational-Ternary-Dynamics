// @ts-check
/** Integrated line energies at fixed reference wavelengths. Gaussian display
 * responses are approximate; these weights do not reconstruct an RGB spectrum.
 * Beaming is D^4 for integrated specific intensity (I_nu/nu^3 invariant).
 */
export const REFERENCE_WAVELENGTHS_NM = Object.freeze([610, 545, 455]);
const WIDTHS_NM = [35, 30, 25];
/** @param {number[]} color @param {string} [spectral] */
export function referenceLineWeights(color, spectral = 'white') {
    if (spectral === 'red-line') return [1, 0, 0];
    if (spectral === 'green-line') return [0, 1, 0];
    if (spectral === 'blue-line') return [0, 0, 1];
    return [...color];
}
/** CPU reference for the actual shader's approximate display stage.
 * @param {number[]} weights @param {number} factor
 */
export function spectrumToLinearDisplay(weights, factor = 1) {
    if (!Number.isFinite(factor) || factor <= 0) throw new RangeError('A spectral Doppler factor must be finite and positive.');
    return REFERENCE_WAVELENGTHS_NM.map((response, channel) => weights.reduce((sum, weight, line) => sum + weight * Math.exp(-.5 * ((REFERENCE_WAVELENGTHS_NM[line] / factor - response) / WIDTHS_NM[channel]) ** 2), 0));
}
/** @param {number[]} weights @param {number} physicalFactor @param {{optical?:boolean,doppler?:boolean,beaming?:boolean,emission?:number}} [settings] */
export function displayLineSpectrum(weights, physicalFactor, settings = {}) {
    const opticalFactor = settings.optical === false ? 1 : physicalFactor;
    const intensity = (settings.emission ?? 1) * (settings.beaming === false ? 1 : opticalFactor ** 4);
    return spectrumToLinearDisplay(weights, settings.doppler === false ? 1 : opticalFactor).map(value => value * intensity);
}
