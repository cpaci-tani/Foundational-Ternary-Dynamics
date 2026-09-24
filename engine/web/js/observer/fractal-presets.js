// @ts-check
/** Stable shader styles shared by the catalog and renderer. All are distant radiance. */
export const FRACTAL_PRESETS = Object.freeze([
    { id: 'fractal-julia', label: 'Living Julia', description: 'An evolving quadratic fractal fills the distant sky.' },
    { id: 'fractal-menger', label: 'Menger folds', description: 'A decorative sponge shell with five levels of repeating voids.' },
    { id: 'fractal-kaleidoscope', label: 'Fractal kaleidoscope', description: 'Folded geometric light evolves around your viewpoint.' },
    { id: 'fractal-mandelbulb', label: 'Mandelbulb nebula', description: 'Sculpted bulbs with branching ridges, orbit colors and illuminated recesses.' },
    { id: 'fractal-mandelbox', label: 'Mandelbox citadel', description: 'Box and sphere folds build intricate architectural cavities.' },
    { id: 'fractal-sierpinski', label: 'Sierpiński crystal', description: 'Nested tetrahedral forms repeat into a luminous geometric structure.' },
    { id: 'fractal-apollonian', label: 'Apollonian vaults', description: 'Sphere-inversion geometry forms a dense network of rounded chambers.' },
    { id: 'fractal-julia-quaternion', label: 'Quaternion Julia reef', description: 'A three-dimensional slice through a four-dimensional quaternion Julia set.' },
    { id: 'fractal-kleinian', label: 'Kleinian chambers', description: 'Kleinian-inspired inversion folds form recursive corridors and delicate openings.' },
].map((preset, style) => Object.freeze({ ...preset, style })));

/** @param {string} preset */
export function fractalStyle(preset) {
    return FRACTAL_PRESETS.find(entry => entry.id === preset)?.style ?? -1;
}

/** Presentation-only budget. Invalid external values never reach GLSL. @param {number|undefined} value */
export function boundedFractalDetail(value) {
    return Number.isFinite(value) ? Math.max(0, Math.min(1, /** @type {number} */ (value))) : 0.65;
}
