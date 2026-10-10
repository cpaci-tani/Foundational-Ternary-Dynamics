/**
 * @file engine/web/js/ui/components/live-rulers/scale-landmarks.js
 * @purpose Reference lengths between the Scale 0 lattice and the scale of the
 *          elementary particles, for the zoom-out view and the Scale Context
 *          ruler. Pure data and arithmetic.
 * @consumers ./landmark-rings.js, ../../../viewport.js,
 *            ../../../scales/scale0/ui/overlays/scale-context-panel.js
 *
 * EVERY length here is an external reference, not an FTD result. Nothing is
 * simulated between the lattice (about 10^-33 m) and these lengths. A particle
 * is placed at its reduced Compton wavelength, hbar / (m c), computed from the
 * measured masses in constants.js: the length below which confining it costs
 * more than its own rest energy. That is a standard definition, used here
 * only to say where on a length axis each particle belongs.
 *
 * Left out on purpose: the up, down and strange quarks (confined, and their
 * masses are scheme-dependent, so a free-particle length would mislead) and
 * the neutrinos (masses not measured).
 */

import {
    HBAR_C_MEV_M,
    M_B_PHYS, M_C_PHYS, M_E_PHYS, M_HIGGS_PHYS, M_MU_PHYS, M_T_PHYS, M_TAU_PHYS, M_W_PHYS, M_Z_PHYS,
} from '../../../constants.js';
import { VOXEL_LENGTH_M } from './measure.js';

/** Collision energy of the LHC in MeV (13.6 TeV), the shortest length probed so far. */
export const LHC_COLLISION_ENERGY_MEV = 13.6e6;
/** Order-of-magnitude size of a nucleon. A round reference, not a measured radius. */
export const NUCLEAR_SCALE_M = 1e-15;

/** Reduced Compton wavelength hbar / (m c) in metres, for a mass in MeV. */
export function reducedComptonWavelength(massMeV) {
    return HBAR_C_MEV_M / massMeV;
}

const particle = (id, label, massMeV, note = '') => ({
    id,
    label,
    metres: reducedComptonWavelength(massMeV),
    kind: 'particle',
    basis: `reduced Compton wavelength, mass ${massMeV} MeV${note ? `, ${note}` : ''}`,
});

/**
 * Smallest first. `label` is what the view shows; `basis` says where the
 * number comes from and is carried into tooltips and accessible names.
 */
export const SCALE_LANDMARKS = Object.freeze([
    {
        id: 'lhc',
        label: 'LHC resolution',
        metres: HBAR_C_MEV_M / LHC_COLLISION_ENERGY_MEV,
        kind: 'experiment',
        basis: 'hbar c / 13.6 TeV; nothing shorter has been probed by experiment',
    },
    particle('top', 'top quark', M_T_PHYS, 'reference value without a specified scheme'),
    particle('higgs', 'Higgs boson', M_HIGGS_PHYS),
    particle('z', 'Z boson', M_Z_PHYS),
    particle('w', 'W boson', M_W_PHYS),
    particle('bottom', 'bottom quark', M_B_PHYS, 'running mass'),
    particle('tau', 'tau', M_TAU_PHYS),
    particle('charm', 'charm quark', M_C_PHYS, 'running mass'),
    {
        id: 'nuclear',
        label: 'nuclear scale',
        metres: NUCLEAR_SCALE_M,
        kind: 'reference',
        basis: 'order of magnitude of a proton or neutron',
    },
    particle('muon', 'muon', M_MU_PHYS),
    particle('electron', 'electron', M_E_PHYS),
].map(Object.freeze));

export function getScaleLandmark(id) {
    return SCALE_LANDMARKS.find((landmark) => landmark.id === id) || null;
}

/** A length in metres as a count of lattice voxels. */
export function metresToVoxels(metres) {
    return metres / VOXEL_LENGTH_M;
}

/** The largest landmark: the view needs to reach a little past it. */
export const OUTERMOST_LANDMARK_M = SCALE_LANDMARKS[SCALE_LANDMARKS.length - 1].metres;

/**
 * Zoom-menu stops beyond the quasi-domain. `across` is the length the view
 * frames: a few times the landmark, so its ring sits inside the view with the
 * neighbouring ones around it.
 */
export const LANDMARK_FRAMED_VIEWS = Object.freeze({
    lhc: Object.freeze({ landmark: 'lhc', across: 4, menu: 'LHC resolution' }),
    electroweak: Object.freeze({ landmark: 'w', across: 3, menu: 'W, Z, Higgs, top' }),
    nuclear: Object.freeze({ landmark: 'nuclear', across: 5, menu: 'Nuclear scale' }),
    electron: Object.freeze({ landmark: 'electron', across: 2.4, menu: 'Electron' }),
});

/** Length in lattice voxels that a framed landmark view spans. */
export function framedLandmarkVoxels(viewId) {
    const view = LANDMARK_FRAMED_VIEWS[viewId];
    const landmark = view && getScaleLandmark(view.landmark);
    return landmark ? metresToVoxels(landmark.metres) * view.across : null;
}

// A ring is drawn while its diameter is between these many pixels and this
// multiple of the longer view side. Smaller, it is a dot on the lattice;
// larger, it is off screen.
export const RING_MIN_PX = 10;
export const RING_MAX_VIEW_SPANS = 2.4;

/**
 * Which landmarks are on screen, and how large, for a view in which one
 * lattice voxel covers `pixelsPerVoxel` pixels.
 * @returns {{id:string,label:string,metres:number,diameterPx:number,basis:string,kind:string}[]}
 */
export function visibleLandmarks(pixelsPerVoxel, viewWidth, viewHeight) {
    if (!(pixelsPerVoxel > 0)) return [];
    const maxPx = Math.max(viewWidth, viewHeight) * RING_MAX_VIEW_SPANS;
    const shown = [];
    for (const landmark of SCALE_LANDMARKS) {
        const diameterPx = metresToVoxels(landmark.metres) * pixelsPerVoxel;
        if (diameterPx < RING_MIN_PX || diameterPx > maxPx) continue;
        shown.push({ ...landmark, diameterPx });
    }
    return shown;
}
