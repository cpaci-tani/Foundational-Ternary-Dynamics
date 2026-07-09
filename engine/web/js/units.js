/**
 * FTD Unit Conversion Layer
 *
 * Central module for converting raw simulation values to human-readable
 * strings with proper physical unit labels.
 *
 * Three unit regimes coexist:
 *   Scale 0 (Lattice):   Planck units  (1 voxel = l_P, 1 tick = t_P)
 *   Scale 1 (Particle):  Planck units  (continuous positions, mass in MeV)
 *   Scale 2 (Atom):      Bohr-scaled   (positions ~ Bohr radii, mass in AMU)
 *
 * Every formatter returns { text, value, unit } where:
 *   text  = "0.511 MeV"  (ready for DOM insertion)
 *   value = 0.511         (the converted numeric value)
 *   unit  = "MeV"         (the unit string alone)
 */

// Wave 2F (2026-04-26): every constant in this module is now sourced
// from constants.js (single source of truth). The exports below keep
// their previous names so the 6 importers of units.js (app.js,
// scale1/2 controllers, scale0/runtime/diagnostics, pe-telemetry,
// diagnostics) need no edits — they continue to import from units.js
// for the formatter helpers below, and any constant they need.
import {
    C_SPEED,
    K_B,
    M_PLANCK_GEV,
    BOHR_RADIUS_M as _BOHR_RADIUS_M,
    BOHR_RADIUS_ANGSTROM as _BOHR_RADIUS_ANGSTROM,
    PLANCK_LENGTH_M as _PLANCK_LENGTH_M,
    PLANCK_TIME_S as _PLANCK_TIME_S,
    PLANCK_TEMP_K as _PLANCK_TEMP_K,
    PLANCK_MASS_KG as _PLANCK_MASS_KG,
    PLANCK_FORCE_N as _PLANCK_FORCE_N,
    J_PER_EV as _J_PER_EV,
    C_MS as _C_MS,
    FTD_TICK_S as _FTD_TICK_S,
    BOHR_LATTICE_TO_M as _BOHR_LATTICE_TO_M,
    AMU_MEV as _AMU_MEV,
    K_PER_EV as _K_PER_EV,
    K_PER_MEV as _K_PER_MEV,
} from './constants.js';

// ── Conversion Constants (re-exports from constants.js) ─────────────

// Planck units -> SI
//
// Note: PLANCK_TIME_S (≈ 5.391247e-44 s) is the SI Planck time t_P, used
// for naive lattice-tick→seconds conversion when one tick is taken as
// one Planck time. FTD_TICK_S (= t_P/√3) is the project-canonical tick
// duration: physical c = c_lattice·(a_phys/t_phys) with the CFL Courant
// number c_lattice = 1/√3 and a_phys ≡ ℓ_P forces t_phys = ℓ_P/(√3·c).
// Use FTD_TICK_S for any FTD-internal time conversion;
// PLANCK_TIME_S is exposed only for SI-Planck reporting.
export const PLANCK_LENGTH_M   = _PLANCK_LENGTH_M;
export const PLANCK_TIME_S     = _PLANCK_TIME_S;
export const FTD_TICK_S        = _FTD_TICK_S;
export const PLANCK_ENERGY_GEV = M_PLANCK_GEV;        // single source vs old 1.22089e19 dup
export const PLANCK_ENERGY_MEV = M_PLANCK_GEV * 1e3;  // GeV → MeV
export const PLANCK_MASS_KG    = _PLANCK_MASS_KG;
export const PLANCK_TEMP_K     = _PLANCK_TEMP_K;
export const PLANCK_FORCE_N    = _PLANCK_FORCE_N;

// Length conversions
export const FM_PER_PLANCK       = PLANCK_LENGTH_M * 1e15;
export const ANGSTROM_PER_PLANCK = PLANCK_LENGTH_M * 1e10;
export const BOHR_RADIUS_M       = _BOHR_RADIUS_M;
export const BOHR_RADIUS_ANGSTROM = _BOHR_RADIUS_ANGSTROM;
// FTD-natural Bohr radius lives in lattice units (R_BOHR = 4π_FTD/(K_B·α)).
// BOHR_LATTICE_TO_M is the SI-Bohr-radius / FTD-Bohr-radius scale factor —
// multiply a lattice length by this to obtain meters when interpreting
// the FTD Bohr length as the physical Bohr radius (5.29177210903e-11 m).
export const BOHR_LATTICE_TO_M  = _BOHR_LATTICE_TO_M;

// Energy conversions
export const EV_PER_MEV = 1e6;
export const J_PER_EV   = _J_PER_EV;
export const KB_MEV     = K_B;                               // 0.511 MeV (FTD anchor)

// Mass
export const AMU_MEV = _AMU_MEV;

// Speed
export const C_MS      = _C_MS;
export const C_LATTICE = C_SPEED;

// Temperature
export const K_PER_EV  = _K_PER_EV;
export const K_PER_MEV = _K_PER_MEV;


// ── Internal Helpers ────────────────────────────────────────────────

/**
 * Adaptive-precision number formatter.
 * Returns a string with appropriate significant figures.
 */
function _fmtNum(v, digits = 4) {
    if (typeof v !== 'number' || !isFinite(v)) return '--';
    if (v === 0) return '0';
    const abs = Math.abs(v);
    if (abs >= 1e6)  return v.toExponential(2);
    if (abs >= 1e4)  return v.toFixed(0);
    if (abs >= 100)  return v.toFixed(1);
    if (abs >= 1)    return v.toFixed(Math.min(digits, 3));
    if (abs >= 0.001) return v.toFixed(digits);
    return v.toExponential(2);
}

/**
 * Build a { text, value, unit } result object.
 */
function _fmt(value, unit, digits = 4) {
    return { text: _fmtNum(value, digits) + ' ' + unit, value, unit };
}


// ── Energy ──────────────────────────────────────────────────────────

function _autoScaleEnergy_MeV(mev) {
    const abs = Math.abs(mev);
    if (abs === 0)       return _fmt(0, 'MeV');
    if (abs >= 1e6)      return _fmt(mev / 1e6, 'TeV', 3);
    if (abs >= 1e3)      return _fmt(mev / 1e3, 'GeV', 3);
    if (abs >= 1)        return _fmt(mev, 'MeV');
    if (abs >= 1e-3)     return _fmt(mev * 1e3, 'keV');
    if (abs >= 1e-6)     return _fmt(mev * 1e6, 'eV');
    if (abs >= 1e-9)     return _fmt(mev * 1e9, 'meV', 3);
    return { text: mev.toExponential(2) + ' MeV', value: mev, unit: 'MeV' };
}

function _autoScaleEnergy_eV(ev) {
    const abs = Math.abs(ev);
    if (abs === 0)       return _fmt(0, 'eV');
    if (abs >= 1e9)      return _fmt(ev / 1e9, 'TeV', 3);
    if (abs >= 1e6)      return _fmt(ev / 1e6, 'GeV', 3);
    if (abs >= 1e3)      return _fmt(ev / 1e3, 'MeV', 3);
    if (abs >= 1)        return _fmt(ev, 'eV');
    if (abs >= 1e-3)     return _fmt(ev * 1e3, 'meV', 3);
    return { text: ev.toExponential(2) + ' eV', value: ev, unit: 'eV' };
}

/**
 * Format an energy value with auto-scaling unit.
 *   Scale 0/1: value is in MeV (or MeV-equivalent Planck units)
 *   Scale 2:   value is in eV (simulation-unit ~ eV scale)
 */
export function formatEnergy(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    if (scale === 2) return _autoScaleEnergy_eV(value);
    return _autoScaleEnergy_MeV(value);
}


// ── Mass ────────────────────────────────────────────────────────────

/**
 * Format a mass value.
 *   Scale 0/1: value in MeV/c^2 — auto-scale eV..TeV
 *   Scale 2:   value in AMU
 */
export function formatMass(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    if (scale === 2) return _fmt(value, 'AMU', 3);
    // MeV/c^2 auto-scale (reuse energy scaling)
    const r = _autoScaleEnergy_MeV(value);
    r.unit += '/c\u00B2';
    r.text = r.text.replace(r.unit.replace('/c\u00B2', ''), r.unit.replace('/c\u00B2', '')) ;
    // Simpler: just show the energy unit (MeV is conventional for particle masses)
    return _autoScaleEnergy_MeV(value);
}


// ── Length ───────────────────────────────────────────────────────────

/**
 * Format a length/distance value.
 *   Scale 0: lattice units (voxels)
 *   Scale 1: lattice units (lu)
 *   Scale 2: simulation units -> Angstroms (multiply by Bohr radius 0.529 A)
 */
export function formatLength(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    if (scale === 0) return _fmt(value, 'vox');
    if (scale === 1) return _fmt(value, 'lu');
    // Scale 2: convert to Angstroms
    const angstroms = value * BOHR_RADIUS_ANGSTROM;
    return _fmt(angstroms, '\u00C5');   // A with ring (Angstrom symbol)
}


// ── Velocity ────────────────────────────────────────────────────────

/**
 * Format a velocity value.
 *   Scale 0/1: voxels/tick -> fraction of c  (divide by C_LATTICE)
 *   Scale 2:   simulation units/step
 */
export function formatVelocity(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    if (scale === 2) return _fmt(value, 'su/step');
    // Scale 0/1: express as fraction of c
    const frac = value / C_LATTICE;
    return _fmt(frac, 'c');
}


// ── Force ───────────────────────────────────────────────────────────

function _fmtForceVal(value, unit) {
    if (value === 0) return { text: '0 ' + unit, value: 0, unit };
    const abs = Math.abs(value);
    if (abs < 1e-4) return { text: value.toExponential(2) + ' ' + unit, value, unit };
    if (abs >= 1e4) return { text: value.toExponential(2) + ' ' + unit, value, unit };
    return { text: value.toFixed(6) + ' ' + unit, value, unit };
}

/**
 * Format a force magnitude.
 *   Scale 0/1: Planck units ("Pl")
 *   Scale 2:   eV/Angstrom
 */
export function formatForce(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    if (scale === 2) return _fmtForceVal(value, 'eV/\u00C5');
    return _fmtForceVal(value, 'Pl');
}


// ── Temperature ─────────────────────────────────────────────────────

/**
 * Format a temperature value.
 *   Scale 0/1: natural units (MeV, kB=1) — show MeV + Kelvin equivalent
 *   Scale 2:   sim-unit equipartition proxy 2·KE/(3N), NOT kelvin
 *
 * Audit P0-10 (2026-05-27): the Scale-2 AtomEngine "temperature" is the
 * bare equipartition proxy T_sim = 2⟨KE⟩/(3N) with an implicit k_B = 1.
 * No Boltzmann conversion to kelvin is performed, so the value is in the
 * same sim units as the AE energy cards. The previous "K" suffix made an
 * unbacked kelvin claim; it is relabelled "(sim)" so the readout no longer
 * asserts a unit it does not compute. (Tooltip fixed in
 * tooltips/definitions.js; card unit-hint in diagnostics-template.js.)
 */
export function formatTemperature(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    if (scale === 2) return _fmt(value, '(sim)');
    // Scale 0/1: value is in MeV
    const kelvin = value * K_PER_MEV;
    return {
        text: _fmtNum(value) + ' MeV (' + _fmtNum(kelvin, 2) + ' K)',
        value, unit: 'MeV'
    };
}


// ── Field Quantities (Scale 0 only) ─────────────────────────────────

/** Format flux vector magnitude. */
export function formatFlux(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    return _fmt(value, 'Pl');
}

/** Format density |J|. */
export function formatDensity(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    return _fmt(value, 'Pl');
}

/** Format entropy (dimensionless, kB = 1). */
export function formatEntropy(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    return _fmt(value, 'k\u0299');   // subscript B for kB
}

/** Format divergence (1/voxel). */
export function formatDivergence(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    return _fmt(value, '/vox');
}

/** Format E-field magnitude. */
export function formatField_E(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    return _fmtForceVal(value, 'Pl');
}

/** Format B-field magnitude. */
export function formatField_B(value, scale = 0) {
    if (typeof value !== 'number' || !isFinite(value)) return { text: '--', value: 0, unit: '' };
    return _fmtForceVal(value, 'Pl');
}


// ── Composite Formatters ────────────────────────────────────────────

/**
 * Format a 3-vector with units.
 * @param {string} quantity - one of 'flux','velocity','force','field_E','field_B','curl'
 */
export function formatVec3(x, y, z, quantity, scale = 0) {
    const fmtMap = {
        flux:    formatFlux,
        velocity: formatVelocity,
        force:   formatForce,
        field_E: formatField_E,
        field_B: formatField_B,
        curl:    formatDivergence,
    };
    const formatter = fmtMap[quantity] || formatFlux;
    // Get unit from one component
    const ref = formatter(x, scale);
    const unit = ref.unit;
    return `(${_fmtNum(formatter(x, scale).value)}, ${_fmtNum(formatter(y, scale).value)}, ${_fmtNum(formatter(z, scale).value)}) ${unit}`;
}

/**
 * Format a position vector.
 *   Scale 0: integer voxel coords
 *   Scale 1: continuous lattice units (2 dp)
 *   Scale 2: Angstroms (3 dp)
 */
export function formatPosition(x, y, z, scale = 0) {
    if (scale === 0) {
        return `(${x}, ${y}, ${z}) vox`;
    }
    if (scale === 1) {
        return `(${_fmtNum(x, 2)}, ${_fmtNum(y, 2)}, ${_fmtNum(z, 2)}) lu`;
    }
    // Scale 2: simulation units -> Angstroms
    const a = BOHR_RADIUS_ANGSTROM;
    const ax = x * a, ay = y * a, az = z * a;
    return `(${ax.toFixed(3)}, ${ay.toFixed(3)}, ${az.toFixed(3)}) \u00C5`;
}


// ── Backward Compatibility Wrappers ─────────────────────────────────

/**
 * Drop-in replacement for particle-catalog.js formatMass(mass_mev).
 * Returns a string (not an object).
 */
export function formatMassCompat(mass_mev) {
    if (mass_mev === 0) return 'massless';
    return formatEnergy(mass_mev, 1).text;
}

/**
 * Drop-in replacement for atomic-energy.js formatEnergy(mev).
 * Returns a string (not an object).
 */
export function formatEnergyCompat(mev) {
    return formatEnergy(mev, 1).text;
}


// ── Unified Entry Point ─────────────────────────────────────────────

const FORMATTERS = {
    energy:      formatEnergy,
    mass:        formatMass,
    length:      formatLength,
    velocity:    formatVelocity,
    force:       formatForce,
    temperature: formatTemperature,
    flux:        formatFlux,
    density:     formatDensity,
    entropy:     formatEntropy,
    divergence:  formatDivergence,
    field_E:     formatField_E,
    field_B:     formatField_B,
};

/**
 * Universal formatter dispatcher.
 * @param {number} value   - raw numeric value
 * @param {string} quantity - 'energy','mass','length','velocity','force','temperature',...
 * @param {number} scale   - 0, 1, or 2
 * @returns {{ text: string, value: number, unit: string }}
 */
export function formatWithUnit(value, quantity, scale = 0) {
    const fn = FORMATTERS[quantity];
    if (!fn) return _fmt(value, '?');
    return fn(value, scale);
}
