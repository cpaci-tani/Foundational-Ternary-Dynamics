/**
 * Catalog → engine-field mapping helpers for Scale-1 particle injection.
 *
 * Extracted from the retired mock-particle-engine.js so the native-WASM
 * adapter (native-particle-engine.js) can resolve catalog identity
 * ([PARAMETRIC] Zoo injection) into the native ParticleEngine's
 * spin / color / spin_axis fields.
 *
 * Color fix (2026-07-29 audit, "strong force structurally unreachable"):
 * the catalog's `color_charge` field holds GROUP descriptors — 'r/g/b' for
 * quarks, 'r̄/ḡ/b̄' for antiquarks, 'none'/'singlet'/'octet' otherwise —
 * never the literal tokens 'r'/'g'/'b' the old mapper compared against, so
 * every particle got color 0 and the strong force could never fire. Colored
 * entries now draw from a deterministic 1→2→3 wheel (injection order), so a
 * baryon-style triplet gets r,g,b and the native strong term is reachable.
 * The engine's color field is an unsigned Z/3 label (same-color pairs repel
 * ×0.5, different-color attract ×1 in the native kernel); quark vs antiquark
 * color-sign is NOT modeled — that limitation is inherited from the engine,
 * not introduced here.
 */

const COLORED_GROUPS = new Set(['r/g/b', 'r̄/ḡ/b̄']);

let _colorWheel = 0;

/** Reset the color wheel (call from scenario/engine reset for determinism). */
export function resetColorWheel() {
    _colorWheel = 0;
}

/**
 * Map a catalog `color_charge` descriptor to the engine's Z/3 color label.
 * Literal 'r'/'g'/'b' map fixed; group descriptors cycle deterministically.
 * 'none', 'singlet' (color-neutral hadrons) and 'octet' (gluon — massless,
 * rejected at injection anyway) map to 0.
 */
export function catalogColorId(colorCharge) {
    if (colorCharge === 'r') return 1;
    if (colorCharge === 'g') return 2;
    if (colorCharge === 'b') return 3;
    if (COLORED_GROUPS.has(colorCharge)) {
        const c = (_colorWheel % 3) + 1;
        _colorWheel += 1;
        return c;
    }
    return 0;
}

/**
 * Spin SIGN from the catalog entry. The catalog stores spin magnitudes
 * (0.5, 1, …) with no orientation, so every spinning particle injects as
 * +1; sign variety comes from scenarios via peSetSpinAxis.
 */
export function catalogSpin(entry) {
    if (!entry || !entry.spin) return 0;
    return entry.spin > 0 ? 1 : -1;
}

/** |S| from catalog spin quantum number (ℏ=1: fermion ½ → |S|=1). */
export function catalogSpinMagnitude(entry) {
    if (!entry || !entry.spin) return 0;
    return Math.abs(entry.spin) * 2.0;
}

/** Initial spin axis: ±z with catalog magnitude (0 vector for spinless). */
export function initSpinAxis(entry, spinSign) {
    if (!spinSign) return [0, 0, 0];
    const mag = catalogSpinMagnitude(entry) || 1.0;
    return [0, 0, spinSign > 0 ? mag : -mag];
}
