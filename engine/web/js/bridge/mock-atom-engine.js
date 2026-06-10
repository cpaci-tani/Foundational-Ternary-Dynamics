/**
 * Scale-2 Atom Engine (AE) — MockBridge side only.
 *
 * Atomic molecular dynamics with ionic (Coulomb), vdW (LJ 12-6), covalent
 * bond spring, H-bond, dipole-dipole, and VSEPR angle-strain forces.
 * Includes Berendsen thermostat, electronegativity-sensitive auto-bonding,
 * 1-2 / 1-3 exclusion, and a Velocity Verlet integrator.
 *
 * Extracted from `bridge-init.js` as Wave 2 ticket 6 of the large-file
 * refactor (see docs/SPEC_REFACTOR_LARGE_FILES.md §4). This is a move, not
 * a rewrite — method bodies preserved verbatim; the only structural change
 * is that `this.*` field accesses go through the live `state` reference.
 *
 * STATE CONTRACT — `state` must be the MockBridge instance (not a
 * destructured copy), exposing:
 *   Read:
 *     _boundaryShape: string
 *     _reflectIntoBoundary(p, cx, cy, cz, R): void
 *   Read + write (created/managed here):
 *     _ae                : { atoms, bonds, nextId, tick, dt, soft,
 *                            damping, bonding, ionic, vdw, bonds_force,
 *                            speed_limit, h_bonds, angle_strain,
 *                            dipole_dipole, thermostat, thermostat_temp,
 *                            electronegativity }
 *     _aeBondSet         : Set<number>            (bond-pair lookup, rebuilt per tick)
 *     _aeIdToIdx         : Map<number, number>    (atom.id -> array index)
 *     _aeNeighborSets    : Array<Set<number>>     (per-atom bonded partner IDs)
 *
 * Bond-pair numeric key: `lo * 100000 + hi`. Safe as long as atom IDs stay
 * below 100000 (typical simulations have <1000 atoms).
 */

import { N_BASE } from '../constants.js';
import {
    AE_EPS_BASE, AE_K_COULOMB, AE_K_BOND, AE_SPEED_MAX,
    AE_H_BOND_EPS, AE_K_ANGLE, AE_THERMOSTAT_TAU, AE_CHI_TABLE,
} from '../atomic-props.js';
import { cpkColor, defaultNeutronCount as elemNeutrons, maxBonds as elemMaxBonds } from '../elements.js';
import { debugLog } from '../core/log.js';

/**
 * Main-group valence electron count (for VSEPR lone-pair geometry).
 * Transition metals fall back to elemMaxBonds(Z).
 */
function _valenceElectrons(Z) {
    const mainGroup = [
        0,
        1, 2,
        1, 2, 3, 4, 5, 6, 7, 8,
        1, 2, 3, 4, 5, 6, 7, 8,
    ];
    if (Z <= 18) return mainGroup[Z] || 0;
    const col = [
        /*K*/ 1, /*Ca*/ 2,
        /*Sc-Zn (3d transition)*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*Ga*/ 3, /*Ge*/ 4, /*As*/ 5, /*Se*/ 6, /*Br*/ 7, /*Kr*/ 8
    ];
    const offset = (Z - 19) % 18;
    if (offset >= 0 && offset < col.length) {
        const v = col[offset];
        return v > 0 ? v : elemMaxBonds(Z);
    }
    return elemMaxBonds(Z);
}

/**
 * Standard covalent valence (target total bond order) by atomic number.
 *
 * Used by bond-order inference (audit P0-12/P0-13): an atom "wants" its
 * summed bond order to equal this value, so the residual valence above its
 * bond *count* drives promotion of single bonds to double/triple/aromatic.
 * Values are the common neutral covalent valences for the elements that
 * appear in the molecule library; ionic / noble species return 0 so they
 * never promote a covalent multiple bond. Anything unlisted falls back to
 * the element's max-bond count.
 */
const COVALENT_VALENCE = {
    1: 1,   // H
    2: 0,   // He (noble)
    5: 3,   // B
    6: 4,   // C
    7: 3,   // N
    8: 2,   // O
    9: 1,   // F
    10: 0,  // Ne (noble)
    11: 0,  // Na (ionic — bonds via electrostatics, not covalent order)
    15: 3,  // P
    16: 2,  // S
    17: 1,  // Cl
    18: 0,  // Ar (noble)
    35: 1,  // Br
};

function _covalentValence(Z) {
    if (Z in COVALENT_VALENCE) return COVALENT_VALENCE[Z];
    return elemMaxBonds(Z);
}

// Bond-order inference tuning (audit P0-12/P0-13). AROMATIC_ORDER is the
// sentinel order written for delocalised ring bonds; the renderer treats
// orders ≥ 1.5 and < 2 as aromatic. A carbon is an aromatic-ring candidate
// when its residual valence (valence − bond count) is exactly 1 and it sits
// in a closed cycle of like candidates (benzene: 6 carbons, each degree 3).
const AROMATIC_ORDER = 1.5;
const MAX_BOND_ORDER = 3;

/**
 * Atomic properties from atomic number Z + neutron count N.
 */
function computeAtomicProps(Z, N = 0) {
    const mass = Z + N * 1.001;
    const z_cbrt = Math.cbrt(Z);
    const radius = z_cbrt > 0 ? 1.0 / z_cbrt : 1.0;
    const vdw_epsilon = AE_EPS_BASE * Math.pow(Z, 2.0 / 3.0);
    const vdw_sigma = radius * N_BASE;
    const max_bonds = elemMaxBonds(Z);
    const electronegativity = (Z >= 1 && Z <= 18) ? AE_CHI_TABLE[Z]
                            : (Z > 18 ? 1.5 + 0.3 * Math.log(Z) : 0);
    return { mass, radius, vdw_epsilon, vdw_sigma, max_bonds, electronegativity };
}

/**
 * Build the atom-engine provider bound to the given bridge-like state.
 *
 * @param {object} state - MockBridge instance (live reference).
 */
export function createAtomEngine(state) {

    function initAE() {
        state._ae = {
            atoms: [], bonds: [], nextId: 0, tick: 0,
            dt: 0.1,       // Larger dt for visible dynamics in sim units
            soft: 0.3,     // Softening in Bohr radii
            damping: false, bonding: true,
            ionic: true,
            vdw: true,
            bonds_force: true,
            speed_limit: true,
            // Phase 3 forces (all off by default)
            h_bonds: false,
            angle_strain: false,
            dipole_dipole: false,
            thermostat: false,
            thermostat_temp: 1.0,
            electronegativity: false,
        };
    }

    function resetAE() {
        if (state._ae) {
            state._ae.atoms = [];
            state._ae.bonds = [];
            state._ae.nextId = 0;
            state._ae.tick = 0;
        }
    }

    function aeAddAtom(Z, x, y, z, vx = 0, vy = 0, vz = 0, charge = 0, N = -1) {
        if (!state._ae) initAE();
        const neutrons = N >= 0 ? N : elemNeutrons(Z);
        const props = computeAtomicProps(Z, neutrons);
        const id = state._ae.nextId++;
        state._ae.atoms.push({
            id, Z, N: neutrons, charge, mass: props.mass, radius: props.radius,
            vdw_epsilon: props.vdw_epsilon, vdw_sigma: props.vdw_sigma,
            max_bonds: props.max_bonds, bonds: [],
            electronegativity: props.electronegativity,
            valence_electrons: _valenceElectrons(Z),
            dipole_x: 0, dipole_y: 0, dipole_z: 0,
            x, y, z, vx, vy, vz, ax: 0, ay: 0, az: 0, locked: false
        });
        return id;
    }

    function aeAddLockedAtom(Z, x, y, z, charge = 0, N = -1) {
        const id = aeAddAtom(Z, x, y, z, 0, 0, 0, charge, N);
        if (state._ae && id >= 0) {
            state._ae.atoms[state._ae.atoms.length - 1].locked = true;
        }
        return id;
    }

    function aeCreateBond(idA, idB, order = 1) {
        if (!state._ae) return;
        const a = state._ae.atoms.find(at => at.id === idA);
        const b = state._ae.atoms.find(at => at.id === idB);
        if (!a || !b) return;
        const sig_avg = (a.vdw_sigma + b.vdw_sigma) / 2;
        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0) / order;
        const eps_mix = Math.sqrt(a.vdw_epsilon * b.vdw_epsilon);
        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
        a.bonds.push({ partner_id: idB, r_eq, k_bond, order });
        b.bonds.push({ partner_id: idA, r_eq, k_bond, order });
    }

    /**
     * Build bond lookup structures for O(1) bond checks and partner lookups.
     * Called once per force evaluation to avoid O(bonds) scans in the inner loop.
     *
     * CAUTION: The bond key formula `lo * 100000 + hi` assumes atom IDs < 100000.
     * Since _ae.nextId increments monotonically and typical simulations have
     * < 1000 atoms, this is safe. If atom IDs ever exceed 100000, collisions
     * would cause false bond-pair matches. Use string keys as fallback if needed.
     */
    function _aeBuildBondLookup() {
        const atoms = state._ae.atoms;
        const bondSet = new Set();
        const idToIdx = new Map();
        const neighborSets = new Array(atoms.length);

        for (let i = 0; i < atoms.length; i++) {
            idToIdx.set(atoms[i].id, i);
            const ns = new Set();
            for (const b of atoms[i].bonds) {
                const lo = Math.min(atoms[i].id, b.partner_id);
                const hi = Math.max(atoms[i].id, b.partner_id);
                bondSet.add(lo * 100000 + hi);
                ns.add(b.partner_id);
            }
            neighborSets[i] = ns;
        }
        state._aeBondSet = bondSet;
        state._aeIdToIdx = idToIdx;
        state._aeNeighborSets = neighborSets;
    }

    function _aeIsBonded(id_a, id_b) {
        const lo = Math.min(id_a, id_b), hi = Math.max(id_a, id_b);
        return state._aeBondSet.has(lo * 100000 + hi);
    }

    function _aeIs13(i, j) {
        const nsI = state._aeNeighborSets[i];
        const nsJ = state._aeNeighborSets[j];
        for (const pid of nsI) {
            if (nsJ.has(pid)) return true;
        }
        return false;
    }

    function _aeComputeDipoleMoments() {
        const atoms = state._ae.atoms;
        for (const a of atoms) {
            a.dipole_x = 0; a.dipole_y = 0; a.dipole_z = 0;
            for (const bond of a.bonds) {
                const jIdx = state._aeIdToIdx.get(bond.partner_id);
                if (jIdx === undefined) continue;
                const aj = atoms[jIdx];
                const chi_diff = aj.electronegativity - a.electronegativity;
                if (Math.abs(chi_diff) < 1e-10) continue;
                a.dipole_x += (aj.x - a.x) * chi_diff;
                a.dipole_y += (aj.y - a.y) * chi_diff;
                a.dipole_z += (aj.z - a.z) * chi_diff;
            }
        }
    }

    function _aeComputeForce(i) {
        const atoms = state._ae.atoms;
        const ai = atoms[i];
        let fx = 0, fy = 0, fz = 0;
        const soft2 = state._ae.soft * state._ae.soft;

        for (let j = 0; j < atoms.length; j++) {
            if (j === i) continue;
            const aj = atoms[j];
            const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            if (r < 1e-20) continue;
            const rx = dx / r, ry = dy / r, rz = dz / r;

            // 1-2 exclusion: bonded pairs use spring instead of LJ (O(1) lookup)
            const isBonded = _aeIsBonded(ai.id, aj.id);

            // 1-3 exclusion: atoms sharing a bonded partner (O(bonds) via Set)
            const is13 = !isBonded && _aeIs13(i, j);

            // Ionic (Coulomb) — skip for bonded and 1-3 pairs
            if (state._ae.ionic && !isBonded && !is13 && ai.charge !== 0 && aj.charge !== 0) {
                const f_ionic = -AE_K_COULOMB * ai.charge * aj.charge / r2;
                fx += f_ionic * rx; fy += f_ionic * ry; fz += f_ionic * rz;
            }

            // Van der Waals (LJ 12-6) — skip for bonded and 1-3 pairs
            if (state._ae.vdw && !isBonded && !is13) {
                const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                const sr = sig_mix / r;
                const sr6 = sr * sr * sr * sr * sr * sr;
                const sr12 = sr6 * sr6;
                const f_vdw = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
                fx += f_vdw * rx; fy += f_vdw * ry; fz += f_vdw * rz;
            }

            // H-bonds: LJ 10-12 + cos²(θ_DHA) angular factor
            if (state._ae.h_bonds) {
                const isElecNeg = (Z) => Z === 7 || Z === 8 || Z === 9;
                const hbondForce = (hAtom, acceptor, _hIdx, aIdx) => {
                    let donorIdx = -1;
                    for (const b of hAtom.bonds) {
                        const didx = state._aeIdToIdx.get(b.partner_id);
                        if (didx !== undefined && isElecNeg(atoms[didx].Z)) { donorIdx = didx; break; }
                    }
                    if (donorIdx < 0 || donorIdx === aIdx) return;
                    const sig_hb = (hAtom.vdw_sigma + acceptor.vdw_sigma) / 2;
                    if (sig_hb <= 0 || r < 1e-10) return;
                    const shr = sig_hb / r;
                    const shr10 = Math.pow(shr, 10);
                    const shr12 = shr10 * shr * shr;
                    const f_rad = AE_H_BOND_EPS * 60.0 * (shr12 - shr10) / r;
                    const donor = atoms[donorIdx];
                    const dhx = hAtom.x - donor.x, dhy = hAtom.y - donor.y, dhz = hAtom.z - donor.z;
                    const hax = acceptor.x - hAtom.x, hay = acceptor.y - hAtom.y, haz = acceptor.z - hAtom.z;
                    const dh_mag = Math.sqrt(dhx*dhx + dhy*dhy + dhz*dhz);
                    const ha_mag = Math.sqrt(hax*hax + hay*hay + haz*haz);
                    let cos_theta = 1.0;
                    if (dh_mag > 1e-30 && ha_mag > 1e-30)
                        cos_theta = (dhx*hax + dhy*hay + dhz*haz) / (dh_mag * ha_mag);
                    const ang = cos_theta * cos_theta;
                    fx += f_rad * ang * rx; fy += f_rad * ang * ry; fz += f_rad * ang * rz;
                };
                if (ai.Z === 1 && isElecNeg(aj.Z)) hbondForce(ai, aj, i, j);
                if (aj.Z === 1 && isElecNeg(ai.Z)) hbondForce(aj, ai, j, i);
            }

            // Dipole-dipole: 1/r^5 interaction between pre-computed molecular dipoles
            if (state._ae.dipole_dipole) {
                const mi_x = ai.dipole_x, mi_y = ai.dipole_y, mi_z = ai.dipole_z;
                const mj_x = aj.dipole_x, mj_y = aj.dipole_y, mj_z = aj.dipole_z;
                const mi_mag2 = mi_x*mi_x + mi_y*mi_y + mi_z*mi_z;
                const mj_mag2 = mj_x*mj_x + mj_y*mj_y + mj_z*mj_z;
                if (mi_mag2 > 1e-60 && mj_mag2 > 1e-60 && r > 1e-10) {
                    const mi_dot_r = mi_x*rx + mi_y*ry + mi_z*rz;
                    const mj_dot_r = mj_x*rx + mj_y*ry + mj_z*rz;
                    const mi_dot_mj = mi_x*mj_x + mi_y*mj_y + mi_z*mj_z;
                    const coeff = 3.0 * AE_K_COULOMB / (r2 * r2 * r);
                    const t1 = 5.0 * mi_dot_r * mj_dot_r / r2;
                    fx += coeff * (t1*rx - mj_x*mi_dot_r - mi_x*mj_dot_r - rx*mi_dot_mj);
                    fy += coeff * (t1*ry - mj_y*mi_dot_r - mi_y*mj_dot_r - ry*mi_dot_mj);
                    fz += coeff * (t1*rz - mj_z*mi_dot_r - mi_z*mj_dot_r - rz*mi_dot_mj);
                }
            }
        }

        // Bond spring forces (O(1) partner lookup via Map)
        if (state._ae.bonds_force) {
            for (const bond of ai.bonds) {
                const jIdx = state._aeIdToIdx.get(bond.partner_id);
                const aj = jIdx !== undefined ? atoms[jIdx] : null;
                if (!aj) continue;
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (r < 1e-20) continue;
                const rx = dx / r, ry = dy / r, rz = dz / r;
                const dr = r - bond.r_eq;
                const f_bond = bond.k_bond * dr;
                fx += f_bond * rx; fy += f_bond * ry; fz += f_bond * rz;
            }
        }

        // Angle strain / VSEPR (3-body): restoring force toward equilibrium angles
        // Force on central atom i only; terminal atoms get Newton's 3rd in _aeComputeAllForces
        if (state._ae.angle_strain && ai.bonds.length >= 2) {
            for (let b1 = 0; b1 < ai.bonds.length; b1++) {
                for (let b2 = b1 + 1; b2 < ai.bonds.length; b2++) {
                    const j1 = state._aeIdToIdx.get(ai.bonds[b1].partner_id);
                    const j2 = state._aeIdToIdx.get(ai.bonds[b2].partner_id);
                    if (j1 === undefined || j2 === undefined) continue;
                    const a1 = atoms[j1], a2 = atoms[j2];
                    const r1x = a1.x - ai.x, r1y = a1.y - ai.y, r1z = a1.z - ai.z;
                    const r2x = a2.x - ai.x, r2y = a2.y - ai.y, r2z = a2.z - ai.z;
                    const m1 = Math.sqrt(r1x*r1x + r1y*r1y + r1z*r1z);
                    const m2 = Math.sqrt(r2x*r2x + r2y*r2y + r2z*r2z);
                    if (m1 < 1e-30 || m2 < 1e-30) continue;

                    let cos_t = (r1x*r2x + r1y*r2y + r1z*r2z) / (m1 * m2);
                    cos_t = Math.max(-1, Math.min(1, cos_t));
                    const theta = Math.acos(cos_t);

                    const nbonds = ai.bonds.length;
                    const lone_pairs = Math.max(0, Math.floor((ai.valence_electrons - nbonds) / 2));
                    const steric = nbonds + lone_pairs;
                    let theta_eq;
                    switch (steric) {
                        case 2: theta_eq = Math.PI; break;
                        case 3: theta_eq = 2 * Math.PI / 3; break;
                        case 4:
                            if (lone_pairs === 0) theta_eq = Math.acos(-1/3);
                            else if (lone_pairs === 1) theta_eq = 107 * Math.PI / 180;
                            else theta_eq = 104.5 * Math.PI / 180;
                            break;
                        default: theta_eq = Math.acos(-1/3); break;
                    }

                    const sin_t = Math.sin(theta);
                    if (Math.abs(sin_t) < 1e-15) continue;
                    const dV = AE_K_ANGLE * (theta - theta_eq);

                    const r1hx = r1x/m1, r1hy = r1y/m1, r1hz = r1z/m1;
                    const r2hx = r2x/m2, r2hy = r2y/m2, r2hz = r2z/m2;
                    let p1x = r2hx - cos_t*r1hx, p1y = r2hy - cos_t*r1hy, p1z = r2hz - cos_t*r1hz;
                    const pm1 = Math.sqrt(p1x*p1x + p1y*p1y + p1z*p1z);
                    if (pm1 < 1e-30) continue;
                    p1x /= pm1; p1y /= pm1; p1z /= pm1;
                    let p2x = r1hx - cos_t*r2hx, p2y = r1hy - cos_t*r2hy, p2z = r1hz - cos_t*r2hz;
                    const pm2 = Math.sqrt(p2x*p2x + p2y*p2y + p2z*p2z);
                    if (pm2 < 1e-30) continue;
                    p2x /= pm2; p2y /= pm2; p2z /= pm2;

                    const fj1 = dV / (m1 * sin_t);
                    const fj2 = dV / (m2 * sin_t);
                    fx -= fj1 * p1x + fj2 * p2x;
                    fy -= fj1 * p1y + fj2 * p2y;
                    fz -= fj1 * p1z + fj2 * p2z;
                }
            }
        }

        // Safety clamp: cap force magnitude to prevent residual explosions
        const fmag2 = fx * fx + fy * fy + fz * fz;
        const F_MAX = 50.0;
        if (fmag2 > F_MAX * F_MAX) {
            const scale = F_MAX / Math.sqrt(fmag2);
            fx *= scale; fy *= scale; fz *= scale;
        }

        return { fx, fy, fz };
    }

    /**
     * Infer covalent bond orders (single / double / triple / aromatic) from
     * geometry + valence saturation. Audit P0-12/P0-13: the auto-bonder only
     * knows connectivity, so without this pass every bond renders as order 1
     * and the multi-order molecules (O₂, N₂, CO₂, ethylene, acetylene,
     * benzene, carbonyls) look identical to single-bonded ones.
     *
     * Rule (two signals, run after all bonds exist):
     *   1. Valence saturation (primary): each atom targets a total bond order
     *      equal to its covalent valence v(Z). Its residual = v − degree is the
     *      number of extra order-units it still needs. A bond between two atoms
     *      that both still have residual capacity is promoted; this is what
     *      turns the single O–O into O=O, the single N–N into N≡N, each C–O in
     *      CO₂ into C=O, the C–C in ethylene into C=C, and in acetylene into C≡C.
     *   2. Distance ordering (tie-breaker): bonds are promoted shortest-first
     *      (smallest r/r_eq), so when an atom has more candidate partners than
     *      residual capacity the geometrically tighter (genuinely multiple)
     *      bond wins. Multiply-bonded atoms are placed closer in molecules.js.
     *
     * Aromatic rings (e.g. benzene) are detected first: a maximal set of
     * carbons each with residual exactly 1 that forms a closed cycle (every
     * member has ≥2 ring neighbours in the set) has all its intra-set bonds
     * marked aromatic (order AROMATIC_ORDER) and its residual cleared, so the
     * ring renders as a uniform delocalised ring rather than a Kekulé
     * single/double alternation.
     *
     * Orders are written to BOTH directed half-edges (ai→aj and aj→ai).
     * Idempotent: resets every order to 1 before re-inferring, so it is safe
     * to call after each auto-bonding pass.
     *
     * KNOWN LIMITATION: molecules whose hand-built geometry is an incomplete
     * fragment (the 8-atom 'diamond' cell, parts of 'caffeine') leave some
     * carbons under-coordinated — they are missing single-bond neighbours that
     * a full crystal/ring would supply. Valence saturation then reads that
     * missing connectivity as unsaturation and may promote a normal single
     * bond to double/triple. This is a geometry-completeness artifact in those
     * two non-canonical molecules, not an inference error; every advertised
     * multiple bond in the diatomics, CO₂, the alkenes/alkynes, the carbonyls,
     * and benzene is inferred correctly. (See AUDIT_WEB_ENGINE_2026-05-27 H-11.)
     */
    function _aeInferBondOrders() {
        if (!state._ae) return;
        const atoms = state._ae.atoms;
        if (atoms.length === 0) return;

        const idToIdx = new Map();
        for (let i = 0; i < atoms.length; i++) idToIdx.set(atoms[i].id, i);

        // Reset all directed half-edges to single before inferring.
        for (const a of atoms) {
            for (const b of a.bonds) b.order = 1;
        }

        // Build the undirected bond list (one entry per pair, i < j by index).
        const bonds = [];
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            for (const b of a.bonds) {
                const j = idToIdx.get(b.partner_id);
                if (j === undefined || j <= i) continue;
                const aj = atoms[j];
                const dx = aj.x - a.x, dy = aj.y - a.y, dz = aj.z - a.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const ratio = b.r_eq > 0 ? r / b.r_eq : 1;
                bonds.push({ i, j, ratio });
            }
        }
        if (bonds.length === 0) return;

        // Per-atom residual valence = covalent valence − bond degree.
        const degree = new Array(atoms.length).fill(0);
        for (const e of bonds) { degree[e.i]++; degree[e.j]++; }
        const residual = new Array(atoms.length).fill(0);
        for (let i = 0; i < atoms.length; i++) {
            residual[i] = Math.max(0, _covalentValence(atoms[i].Z) - degree[i]);
        }

        // ── Aromatic-ring detection ────────────────────────────────────────
        // Candidates: carbons with residual exactly 1. A candidate is in a ring
        // iff it has ≥2 bonds to other candidates. Iteratively drop candidates
        // with <2 candidate-neighbours (peel chains/leaves); what survives is
        // the set of closed-cycle aromatic carbons.
        const isCandidate = new Array(atoms.length).fill(false);
        for (let i = 0; i < atoms.length; i++) {
            if (atoms[i].Z === 6 && residual[i] === 1) isCandidate[i] = true;
        }
        // Adjacency among candidates.
        const candAdj = new Map(); // idx -> Set of candidate neighbour idxs
        const ensure = (k) => { if (!candAdj.has(k)) candAdj.set(k, new Set()); return candAdj.get(k); };
        for (const e of bonds) {
            if (isCandidate[e.i] && isCandidate[e.j]) {
                ensure(e.i).add(e.j);
                ensure(e.j).add(e.i);
            }
        }
        let changed = true;
        while (changed) {
            changed = false;
            for (let i = 0; i < atoms.length; i++) {
                if (!isCandidate[i]) continue;
                const nbrs = candAdj.get(i);
                const live = nbrs ? [...nbrs].filter(k => isCandidate[k]).length : 0;
                if (live < 2) { isCandidate[i] = false; changed = true; }
            }
        }
        // Mark intra-ring bonds aromatic; clear residual of ring atoms.
        const aromaticBond = new Array(bonds.length).fill(false);
        for (let bi = 0; bi < bonds.length; bi++) {
            const e = bonds[bi];
            if (isCandidate[e.i] && isCandidate[e.j]) {
                aromaticBond[bi] = true;
            }
        }
        for (let i = 0; i < atoms.length; i++) {
            if (isCandidate[i]) residual[i] = 0;
        }

        // ── Greedy valence-saturation promotion (shortest bond first) ──────
        const order = new Array(bonds.length).fill(1);
        const idxOrder = bonds.map((_, k) => k).sort((p, q) => bonds[p].ratio - bonds[q].ratio);
        for (const bi of idxOrder) {
            if (aromaticBond[bi]) continue;
            const e = bonds[bi];
            while (residual[e.i] > 0 && residual[e.j] > 0 && order[bi] < MAX_BOND_ORDER) {
                order[bi]++;
                residual[e.i]--;
                residual[e.j]--;
            }
        }

        // ── Write orders back to both directed half-edges ──────────────────
        const finalOrder = (bi) => aromaticBond[bi] ? AROMATIC_ORDER : order[bi];
        for (let bi = 0; bi < bonds.length; bi++) {
            const e = bonds[bi];
            const ai = atoms[e.i], aj = atoms[e.j];
            const o = finalOrder(bi);
            const hAB = ai.bonds.find(b => b.partner_id === aj.id);
            const hBA = aj.bonds.find(b => b.partner_id === ai.id);
            if (hAB) hAB.order = o;
            if (hBA) hBA.order = o;
        }
    }

    /**
     * Run auto-bonding logic without physics integration.
     * Call after loading a molecule to establish bonds before the first tick.
     */
    function aePreBond() {
        if (!state._ae || !state._ae.bonding) {
            debugLog('[FTD aePreBond] skipped — ae:', !!state._ae, 'bonding:', state._ae?.bonding);
            return;
        }
        const atoms = state._ae.atoms;
        let bondsCreated = 0;
        for (let i = 0; i < atoms.length; i++) {
            for (let j = i + 1; j < atoms.length; j++) {
                const ai = atoms[i], aj = atoms[j];
                if (ai.bonds.some(b => b.partner_id === aj.id)) continue;
                if (ai.bonds.length >= ai.max_bonds || aj.bonds.length >= aj.max_bonds) continue;
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const sig_avg = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                if (r < 1.2 * sig_avg) {
                    const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                    const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                    const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                    ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                    aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    bondsCreated++;
                }
            }
        }
        // Infer double/triple/aromatic orders now that connectivity is set
        // (audit P0-12/P0-13). Must run after every bond exists so valence
        // saturation sees the full degree of each atom.
        _aeInferBondOrders();
        debugLog(`[FTD aePreBond] ${atoms.length} atoms, ${bondsCreated} bonds created`);
        for (const a of atoms) {
            debugLog(`  atom ${a.id} Z=${a.Z} pos=(${a.x.toFixed(2)},${a.y.toFixed(2)},${a.z.toFixed(2)}) bonds=${a.bonds.length}/${a.max_bonds} sigma=${a.vdw_sigma.toFixed(2)}`);
        }
    }

    function _aeComputeAllForces() {
        const atoms = state._ae.atoms;
        _aeBuildBondLookup();

        if (state._ae.dipole_dipole) _aeComputeDipoleMoments();

        const forces = new Array(atoms.length);
        for (let i = 0; i < atoms.length; i++) {
            forces[i] = _aeComputeForce(i);
        }

        // Angle strain: distribute Newton's-3rd-law forces to terminal atoms
        if (state._ae.angle_strain) {
            for (let i = 0; i < atoms.length; i++) {
                const ai = atoms[i];
                if (ai.bonds.length < 2) continue;
                for (let b1 = 0; b1 < ai.bonds.length; b1++) {
                    for (let b2 = b1 + 1; b2 < ai.bonds.length; b2++) {
                        const j1 = state._aeIdToIdx.get(ai.bonds[b1].partner_id);
                        const j2 = state._aeIdToIdx.get(ai.bonds[b2].partner_id);
                        if (j1 === undefined || j2 === undefined) continue;
                        const a1 = atoms[j1], a2 = atoms[j2];
                        const r1x = a1.x-ai.x, r1y = a1.y-ai.y, r1z = a1.z-ai.z;
                        const r2x = a2.x-ai.x, r2y = a2.y-ai.y, r2z = a2.z-ai.z;
                        const m1 = Math.sqrt(r1x*r1x+r1y*r1y+r1z*r1z);
                        const m2 = Math.sqrt(r2x*r2x+r2y*r2y+r2z*r2z);
                        if (m1 < 1e-30 || m2 < 1e-30) continue;
                        let cos_t = (r1x*r2x+r1y*r2y+r1z*r2z)/(m1*m2);
                        cos_t = Math.max(-1, Math.min(1, cos_t));
                        const theta = Math.acos(cos_t);
                        const nbonds = ai.bonds.length;
                        const lone_pairs = Math.max(0, Math.floor((ai.valence_electrons - nbonds) / 2));
                        const steric = nbonds + lone_pairs;
                        let theta_eq;
                        switch (steric) {
                            case 2: theta_eq = Math.PI; break;
                            case 3: theta_eq = 2*Math.PI/3; break;
                            case 4:
                                if (lone_pairs===0) theta_eq = Math.acos(-1/3);
                                else if (lone_pairs===1) theta_eq = 107*Math.PI/180;
                                else theta_eq = 104.5*Math.PI/180;
                                break;
                            default: theta_eq = Math.acos(-1/3); break;
                        }
                        const sin_t = Math.sin(theta);
                        if (Math.abs(sin_t) < 1e-15) continue;
                        const dV = AE_K_ANGLE * (theta - theta_eq);
                        const r1hx=r1x/m1, r1hy=r1y/m1, r1hz=r1z/m1;
                        const r2hx=r2x/m2, r2hy=r2y/m2, r2hz=r2z/m2;
                        let p1x=r2hx-cos_t*r1hx, p1y=r2hy-cos_t*r1hy, p1z=r2hz-cos_t*r1hz;
                        const pm1=Math.sqrt(p1x*p1x+p1y*p1y+p1z*p1z);
                        if (pm1<1e-30) continue;
                        p1x/=pm1; p1y/=pm1; p1z/=pm1;
                        let p2x=r1hx-cos_t*r2hx, p2y=r1hy-cos_t*r2hy, p2z=r1hz-cos_t*r2hz;
                        const pm2=Math.sqrt(p2x*p2x+p2y*p2y+p2z*p2z);
                        if (pm2<1e-30) continue;
                        p2x/=pm2; p2y/=pm2; p2z/=pm2;
                        const fj1 = dV/(m1*sin_t), fj2 = dV/(m2*sin_t);
                        forces[j1].fx += fj1*p1x; forces[j1].fy += fj1*p1y; forces[j1].fz += fj1*p1z;
                        forces[j2].fx += fj2*p2x; forces[j2].fy += fj2*p2y; forces[j2].fz += fj2*p2z;
                    }
                }
            }
        }

        return forces;
    }

    function aeTick() {
        if (!state._ae) return;
        const atoms = state._ae.atoms;
        const dt = state._ae.dt;
        const tickNum = state._ae.tick;

        let forces = _aeComputeAllForces();

        // Debug: log first 3 ticks
        if (tickNum < 3) {
            debugLog(`[FTD aeTick #${tickNum}] dt=${dt} atoms=${atoms.length}`);
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i], f = forces[i];
                debugLog(`  atom ${a.id}: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)}) force=(${f.fx.toFixed(4)},${f.fy.toFixed(4)},${f.fz.toFixed(4)}) bonds=${a.bonds.length}`);
            }
        }

        // Half-kick
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            if (a.locked) continue;
            const hdt = dt * 0.5 / a.mass;
            a.vx += forces[i].fx * hdt;
            a.vy += forces[i].fy * hdt;
            a.vz += forces[i].fz * hdt;
        }

        // Drift
        for (const a of atoms) {
            if (a.locked) continue;
            a.x += a.vx * dt;
            a.y += a.vy * dt;
            a.z += a.vz * dt;
        }

        // Boundary containment (AE mode: origin-centered, radius 35)
        if (state._boundaryShape !== 'cube' && state._boundaryShape !== 'none') {
            for (const a of atoms) {
                if (a.locked) continue;
                state._reflectIntoBoundary(a, 0, 0, 0, 35);
            }
        }

        forces = _aeComputeAllForces();

        // Half-kick again
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            if (a.locked) continue;
            const hdt = dt * 0.5 / a.mass;
            a.vx += forces[i].fx * hdt;
            a.vy += forces[i].fy * hdt;
            a.vz += forces[i].fz * hdt;
        }

        if (tickNum < 3) {
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i];
                debugLog(`  atom ${a.id} after tick: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)})`);
            }
        }

        // Speed limit
        if (state._ae.speed_limit) {
            for (const a of atoms) {
                if (a.locked) continue;
                const speed = Math.sqrt(a.vx * a.vx + a.vy * a.vy + a.vz * a.vz);
                if (speed > AE_SPEED_MAX) {
                    const s = AE_SPEED_MAX / speed;
                    a.vx *= s; a.vy *= s; a.vz *= s;
                }
            }
        }

        // Damping
        if (state._ae.damping) {
            const d = Math.max(0, 1 - 0.02 * dt);
            for (const a of atoms) {
                if (a.locked) continue;
                a.vx *= d; a.vy *= d; a.vz *= d;
            }
        }

        // Berendsen thermostat
        if (state._ae.thermostat && state._ae.thermostat_temp > 0) {
            let ke = 0, n_free = 0;
            for (const a of atoms) {
                if (!a.locked) {
                    ke += 0.5 * a.mass * (a.vx*a.vx + a.vy*a.vy + a.vz*a.vz);
                    n_free++;
                }
            }
            if (n_free > 0) {
                const T_current = 2.0 * ke / (3.0 * n_free);
                if (T_current > 1e-30) {
                    const lam = Math.sqrt(1.0 + dt / AE_THERMOSTAT_TAU
                        * (state._ae.thermostat_temp / T_current - 1.0));
                    for (const a of atoms) {
                        if (!a.locked) { a.vx *= lam; a.vy *= lam; a.vz *= lam; }
                    }
                }
            }
        }

        // Auto-bonding + bond breaking
        if (state._ae.bonding) {
            for (let i = 0; i < atoms.length; i++) {
                for (let j = i + 1; j < atoms.length; j++) {
                    const ai = atoms[i], aj = atoms[j];
                    if (ai.bonds.some(b => b.partner_id === aj.id)) continue;
                    if (ai.bonds.length >= ai.max_bonds || aj.bonds.length >= aj.max_bonds) continue;
                    const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    const sig_avg = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                    let bond_threshold = 1.2 * sig_avg;
                    if (state._ae.electronegativity) {
                        const chi_diff = Math.abs(ai.electronegativity - aj.electronegativity);
                        bond_threshold *= (1.0 + 0.2 * chi_diff);
                    }
                    if (r < bond_threshold) {
                        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                        const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                        ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                        aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    }
                }
            }
            // Bond breaking — break only when stretched far beyond equilibrium
            for (const a of atoms) {
                a.bonds = a.bonds.filter(b => {
                    const jIdx = state._aeIdToIdx.get(b.partner_id);
                    if (jIdx === undefined) return false;
                    const partner = atoms[jIdx];
                    const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    return r <= 3.5 * b.r_eq;
                });
            }
            // Re-infer bond orders after connectivity may have changed this
            // tick (audit P0-12/P0-13). Idempotent — resets to single first.
            _aeInferBondOrders();
        }

        state._ae.tick++;
    }

    function aeGetAtomData() {
        if (!state._ae) return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), atomicNums: new Int32Array(0), charges: new Int32Array(0), ids: new Int32Array(0), bonds: new Int32Array(0), bondOrders: new Float32Array(0), bondCount: 0, count: 0 };
        const atoms = state._ae.atoms;
        const count = atoms.length;
        const positions = new Float32Array(count * 3);
        const colors = new Float32Array(count * 3);
        const sizes = new Float32Array(count);
        const atomicNums = new Int32Array(count);
        const charges = new Int32Array(count);
        const ids = new Int32Array(count);

        let bondCount = 0;
        for (const a of atoms) {
            for (const b of a.bonds) {
                if (b.partner_id > a.id) bondCount++;
            }
        }
        const bonds = new Int32Array(bondCount * 2);
        // Float (not Int) so the aromatic sentinel order 1.5 survives — the
        // renderer distinguishes 1.5 ≤ order < 2 as aromatic (audit P0-13).
        const bondOrders = new Float32Array(bondCount);

        for (let i = 0; i < count; i++) {
            const a = atoms[i];
            positions[i * 3] = a.x;
            positions[i * 3 + 1] = a.y;
            positions[i * 3 + 2] = a.z;
            const [cr, cg, cb] = cpkColor(a.Z);
            colors[i * 3] = cr; colors[i * 3 + 1] = cg; colors[i * 3 + 2] = cb;
            sizes[i] = 6.0 + a.radius * 10.0;
            if (sizes[i] > 60) sizes[i] = 60;
            atomicNums[i] = a.Z;
            charges[i] = a.charge;
            ids[i] = a.id;
        }

        let bi = 0;
        for (const a of atoms) {
            for (const b of a.bonds) {
                if (b.partner_id > a.id) {
                    bonds[bi * 2] = a.id;
                    bonds[bi * 2 + 1] = b.partner_id;
                    bondOrders[bi] = b.order || 1;
                    bi++;
                }
            }
        }

        return { positions, colors, sizes, atomicNums, charges, ids, bonds, bondOrders, bondCount, count };
    }

    function aeGetFieldSources() {
        if (!state._ae) return { positions: new Float32Array(0), charges: new Float32Array(0), count: 0 };
        const atoms = state._ae.atoms;
        const n = atoms.length;
        const positions = new Float32Array(n * 3);
        const charges = new Float32Array(n);
        for (let i = 0; i < n; i++) {
            positions[i * 3] = atoms[i].x;
            positions[i * 3 + 1] = atoms[i].y;
            positions[i * 3 + 2] = atoms[i].z;
            charges[i] = atoms[i].charge;
        }
        return { positions, charges, count: n };
    }

    function aeGetDiagnostics() {
        if (!state._ae) return { tick: 0, atomCount: 0, bondCount: 0, totalKE: 0, totalPEIonic: 0, totalPEVdw: 0, totalPEBond: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, temperature: 0 };
        const atoms = state._ae.atoms;
        let ke = 0, pe_ionic = 0, pe_vdw = 0, pe_bond = 0;
        let px = 0, py = 0, pz = 0;
        const soft2 = state._ae.soft * state._ae.soft;

        for (const a of atoms) {
            const v2 = a.vx * a.vx + a.vy * a.vy + a.vz * a.vz;
            ke += 0.5 * a.mass * v2;
            px += a.mass * a.vx; py += a.mass * a.vy; pz += a.mass * a.vz;
        }

        for (let i = 0; i < atoms.length; i++) {
            for (let j = i + 1; j < atoms.length; j++) {
                const ai = atoms[i], aj = atoms[j];
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (ai.charge !== 0 && aj.charge !== 0) {
                    pe_ionic += AE_K_COULOMB * ai.charge * aj.charge / r;
                }
                const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                const sr = sig_mix / r; const sr6 = sr ** 6; const sr12 = sr6 * sr6;
                pe_vdw += 4.0 * eps_mix * (sr12 - sr6);
            }
        }

        // Bond PE — build id→atom map ONCE (F-10) so the partner lookup in the
        // bond loop is O(1) instead of an O(N) Array.find per bond (was O(N²)).
        const idToAtom = new Map();
        for (let i = 0; i < atoms.length; i++) idToAtom.set(atoms[i].id, atoms[i]);
        const counted = new Set();
        for (const a of atoms) {
            for (const b of a.bonds) {
                const key = Math.min(a.id, b.partner_id) + ',' + Math.max(a.id, b.partner_id);
                if (counted.has(key)) continue;
                counted.add(key);
                const partner = idToAtom.get(b.partner_id);
                if (!partner) continue;
                const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const dr = r - b.r_eq;
                pe_bond += 0.5 * b.k_bond * dr * dr;
            }
        }

        let bondCount = 0;
        for (const a of atoms) {
            for (const b of a.bonds) { if (b.partner_id > a.id) bondCount++; }
        }

        // Equipartition proxy in SIM UNITS (implicit k_B = 1), NOT kelvin.
        // No Boltzmann conversion is applied — this is the bare 2⟨KE⟩/(3N)
        // statistic. The UI relabels it "(sim)" (audit P0-10); do not append
        // a "K" suffix or treat this as an SI temperature downstream.
        const T = atoms.length > 0 ? 2.0 * ke / (3.0 * atoms.length) : 0;

        return {
            tick: state._ae.tick, atomCount: atoms.length, bondCount,
            totalKE: ke, totalPEIonic: pe_ionic, totalPEVdw: pe_vdw, totalPEBond: pe_bond,
            totalEnergy: ke + pe_ionic + pe_vdw + pe_bond,
            momentumX: px, momentumY: py, momentumZ: pz, temperature: T
        };
    }

    /**
     * Decomposed forces on each atom: ionic (Coulomb), vdW (LJ), bond (spring), net.
     *
     * F-8 (visibility-gated, EXACT): the O(N²) ionic+vdW pair loop is the only
     * expensive part. The caller passes `want` flags for the channels whose
     * force arrows are actually visible; `net` requires all three channels.
     * Channels that are neither requested nor needed by `net` are returned as
     * the zeroed Float32Array they were allocated to — the renderer leaves those
     * (invisible) arrow meshes hidden, so the displayed output is identical.
     *
     * The long-range pair loop is skipped ENTIRELY when no long-range channel
     * (ionic, vdW, or net) is requested — e.g. bond-only arrows on ae-periodic.
     * No cutoff is introduced: when the loop does run it is the same exact
     * full-N² sum as before, so every displayed channel value is unchanged.
     *
     * @param {{ionic?:boolean, vdw?:boolean, bond?:boolean, net?:boolean}} [want]
     *        Which channels to compute. Omitted ⇒ all (backward-compatible).
     */
    function aeGetForceDecomposition(want) {
        if (!state._ae) return { ionic: new Float32Array(0), vdw: new Float32Array(0), bond: new Float32Array(0), net: new Float32Array(0), count: 0 };
        const atoms = state._ae.atoms;
        const n = atoms.length;
        const ionic = new Float32Array(n * 3);
        const vdw   = new Float32Array(n * 3);
        const bond  = new Float32Array(n * 3);
        const net   = new Float32Array(n * 3);
        const soft2 = state._ae.soft * state._ae.soft;

        // Default to all-channels when no selection is given (callers that pass
        // a `want` object get per-channel gating). `net` pulls in every channel.
        const wantNet   = !want || want.net;
        const wantIonic = wantNet || (want && want.ionic);
        const wantVdw   = wantNet || (want && want.vdw);
        const wantBond  = wantNet || !want || want.bond;
        // The pair loop produces ONLY the long-range channels (ionic, vdW).
        const wantPair  = wantIonic || wantVdw;

        // Net accumulator kept in f64 so the final net = f32((fi+fv)+fb) is
        // bit-identical to the original single-expression sum (no intermediate
        // float32 rounding between the ionic+vdW and bond passes). Only when net
        // is actually requested.
        const netAcc = wantNet ? new Float64Array(n * 3) : null;

        _aeBuildBondLookup();

        for (let i = 0; wantPair && i < n; i++) {
            const ai = atoms[i];
            let fi_x = 0, fi_y = 0, fi_z = 0;
            let fv_x = 0, fv_y = 0, fv_z = 0;

            for (let j = 0; j < n; j++) {
                if (j === i) continue;
                const aj = atoms[j];
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r2 = dx * dx + dy * dy + dz * dz + soft2;
                const r = Math.sqrt(r2);
                if (r < 1e-20) continue;
                const rx = dx / r, ry = dy / r, rz = dz / r;

                const isBonded = _aeIsBonded(ai.id, aj.id);
                const is13 = !isBonded && _aeIs13(i, j);

                if (wantIonic && state._ae.ionic && !isBonded && !is13 && ai.charge !== 0 && aj.charge !== 0) {
                    const f = -AE_K_COULOMB * ai.charge * aj.charge / r2;
                    fi_x += f * rx; fi_y += f * ry; fi_z += f * rz;
                }

                if (wantVdw && state._ae.vdw && !isBonded && !is13) {
                    const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                    const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                    const sr = sig_mix / r;
                    const sr6 = sr * sr * sr * sr * sr * sr;
                    const sr12 = sr6 * sr6;
                    const f = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
                    fv_x += f * rx; fv_y += f * ry; fv_z += f * rz;
                }
            }

            ionic[i * 3] = fi_x; ionic[i * 3 + 1] = fi_y; ionic[i * 3 + 2] = fi_z;
            vdw[i * 3]   = fv_x; vdw[i * 3 + 1]   = fv_y; vdw[i * 3 + 2]   = fv_z;
            // Seed the f64 net accumulator with the ionic+vdW partials (bond
            // added in the bond pass). Kept in f64 → no intermediate rounding.
            if (netAcc) {
                netAcc[i * 3] = fi_x + fv_x; netAcc[i * 3 + 1] = fi_y + fv_y; netAcc[i * 3 + 2] = fi_z + fv_z;
            }
        }

        // Bond spring channel (O(bonds), cheap) — computed independently of the
        // long-range pair loop so bond-only arrows skip the N² work entirely.
        if (wantBond && state._ae.bonds_force) {
            for (let i = 0; i < n; i++) {
                const ai = atoms[i];
                let fb_x = 0, fb_y = 0, fb_z = 0;
                for (const b of ai.bonds) {
                    const jIdx = state._aeIdToIdx.get(b.partner_id);
                    const aj = jIdx !== undefined ? atoms[jIdx] : null;
                    if (!aj) continue;
                    const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                    if (r < 1e-20) continue;
                    const rx = dx / r, ry = dy / r, rz = dz / r;
                    const dr = r - b.r_eq;
                    const f = b.k_bond * dr;
                    fb_x += f * rx; fb_y += f * ry; fb_z += f * rz;
                }
                bond[i * 3] = fb_x; bond[i * 3 + 1] = fb_y; bond[i * 3 + 2] = fb_z;
                // Add the f64 bond partial onto the f64 net accumulator.
                if (netAcc) {
                    netAcc[i * 3] += fb_x; netAcc[i * 3 + 1] += fb_y; netAcc[i * 3 + 2] += fb_z;
                }
            }
        }

        // Commit the f64 net accumulator to the float32 net channel in one pass.
        // net[k] = f32((fi+fv)+fb), matching the original fi+fv+fb expression.
        if (netAcc) net.set(netAcc);

        return { ionic, vdw, bond, net, count: n };
    }

    function aeSetDt(dt)              { if (state._ae) state._ae.dt = dt; }
    function aeGetDt()                 { return state._ae ? state._ae.dt : 0.01; }
    function aeSetSoftening(s)        { if (state._ae) state._ae.soft = s; }
    function aeSetDamping(e)          { if (state._ae) state._ae.damping = e; }
    function aeSetBonding(e)          { if (state._ae) state._ae.bonding = e; }
    function aeSetIonic(e)            { if (state._ae) state._ae.ionic = e; }
    function aeSetVdw(e)              { if (state._ae) state._ae.vdw = e; }
    function aeSetBondsForce(e)       { if (state._ae) state._ae.bonds_force = e; }
    function aeSetSpeedLimit(e)       { if (state._ae) state._ae.speed_limit = e; }
    function aeSetHBonds(e)           { if (state._ae) state._ae.h_bonds = e; }
    function aeSetAngleStrain(e)      { if (state._ae) state._ae.angle_strain = e; }
    function aeSetDipoleDipole(e)     { if (state._ae) state._ae.dipole_dipole = e; }
    function aeSetThermostat(e)       { if (state._ae) state._ae.thermostat = e; }
    function aeSetThermostatTemp(t)   { if (state._ae) state._ae.thermostat_temp = t; }
    function aeSetElectronegativity(e){ if (state._ae) state._ae.electronegativity = e; }
    function aeAtomCount()            { return state._ae ? state._ae.atoms.length : 0; }
    function aeClear()                { resetAE(); }

    /**
     * Snapshot of the live AE runtime parameters + physics toggle states.
     * Read by telemetryHub.collectScale2 so the diagnostics panel reports
     * engine truth (not DOM checkbox state). Mirrors peGetToggle's role on
     * Scale 1, returned as one object because AE toggles are only ever
     * consumed together.
     */
    function aeGetRuntimeState() {
        if (!state._ae) return null;
        const ae = state._ae;
        return {
            dt: ae.dt,
            softening: ae.soft,
            thermostatTemp: ae.thermostat_temp,
            toggles: {
                ionic: !!ae.ionic,
                vdw: !!ae.vdw,
                bonds_force: !!ae.bonds_force,
                bonding: !!ae.bonding,
                damping: !!ae.damping,
                speed_limit: !!ae.speed_limit,
                h_bonds: !!ae.h_bonds,
                angle_strain: !!ae.angle_strain,
                dipole_dipole: !!ae.dipole_dipole,
                thermostat: !!ae.thermostat,
                electronegativity: !!ae.electronegativity,
            },
        };
    }

    function aeInspectAtom(id) {
        if (!state._ae) return null;
        const a = state._ae.atoms.find(at => at.id === id);
        if (!a) return null;
        const mass = a.Z + a.N * 1.001;
        const speed = Math.sqrt(a.vx * a.vx + a.vy * a.vy + a.vz * a.vz);
        const ke = 0.5 * mass * speed * speed;

        const bondInfo = a.bonds.map(b => {
            const p = state._ae.atoms.find(at => at.id === b.partner_id);
            if (!p) return null;
            const dx = p.x - a.x, dy = p.y - a.y, dz = p.z - a.z;
            const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
            return { partnerId: b.partner_id, partnerZ: p.Z, dist, r_eq: b.r_eq, order: b.order };
        }).filter(Boolean);

        let nearestId = -1, nearestDist = Infinity, nearestZ = 0;
        const bondSet = new Set(a.bonds.map(b => b.partner_id));
        for (const other of state._ae.atoms) {
            if (other.id === id || bondSet.has(other.id)) continue;
            const dx = other.x - a.x, dy = other.y - a.y, dz = other.z - a.z;
            const d = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (d < nearestDist) { nearestDist = d; nearestId = other.id; nearestZ = other.Z; }
        }

        // Net force magnitude — must rebuild bond lookups first.
        _aeBuildBondLookup();
        const idx = state._ae.atoms.indexOf(a);
        const f = _aeComputeForce(idx);
        const fNetMag = Math.sqrt(f.fx * f.fx + f.fy * f.fy + f.fz * f.fz);

        return {
            id, Z: a.Z, N: a.N, charge: a.charge, mass, radius: a.radius,
            locked: a.locked, sigma: a.vdw_sigma, epsilon: a.vdw_epsilon,
            maxBonds: a.max_bonds,
            x: a.x, y: a.y, z: a.z,
            vx: a.vx, vy: a.vy, vz: a.vz,
            speed, ke, bonds: bondInfo,
            nearestId, nearestDist, nearestZ, fNetMag,
        };
    }

    return {
        initAE, resetAE,
        aeAddAtom, aeAddLockedAtom, aeCreateBond,
        _aeBuildBondLookup, _aeIsBonded, _aeIs13,
        _aeComputeDipoleMoments, _aeComputeForce, _aeComputeAllForces,
        aePreBond, aeTick,
        aeGetAtomData, aeGetFieldSources, aeGetDiagnostics, aeGetForceDecomposition,
        aeSetDt, aeGetDt, aeSetSoftening, aeSetDamping, aeSetBonding,
        aeSetIonic, aeSetVdw, aeSetBondsForce, aeSetSpeedLimit,
        aeSetHBonds, aeSetAngleStrain, aeSetDipoleDipole,
        aeSetThermostat, aeSetThermostatTemp, aeSetElectronegativity,
        aeAtomCount, aeClear, aeInspectAtom, aeGetRuntimeState,
    };
}
