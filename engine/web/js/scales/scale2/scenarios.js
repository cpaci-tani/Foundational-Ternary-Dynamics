/**
 * Scale 2 — AE Scenario Loader
 * ────────────────────────────────────────────────────────────────────
 *
 * Extracted verbatim from scales/scale2/controller.js (ticket S2-1).
 * Houses the big `switch (name)` that builds atom scenarios:
 *   - Full 118-element periodic table
 *   - Noble-gas clusters (vdW only)
 *   - Ionic formation (Coulomb-driven)
 *   - Covalent bond formation (auto-bonding)
 *   - H-bonding (Phase 3)
 *   - VSEPR geometry relaxation
 *   - Thermal dynamics (thermostat + gas kinetics)
 *   - Metallic clusters
 *   - Individual element scenarios (ae-el-1 through ae-el-118)
 *
 * Pure move — no scenario body was changed. The controller handles
 * bridge init, toggle resets, slider sync, and energy-reference capture
 * before and after this function runs.
 *
 * CTX:
 *   { bridge, viewport, inspector,
 *     helpers: { setPhase3 }  // shared Phase-3 toggle helper
 *   }
 */

import { allElements, tablePosition, getElement } from '../../elements.js';
import { NEUTRON_PROTON_MASS_RATIO } from '../../constants.js';


// ═════════════════════════════════════════════════════════════════════
// Per-scenario VISUAL presets (mirror of scale1/scenarios.js overlays)
// ═════════════════════════════════════════════════════════════════════
//
// Physics toggles are set imperatively inside each scenario body below;
// these presets cover the VISUAL layer only (what overlays/toggles light
// up when the scenario loads). Applied by the controller's
// applyAEVisualPreset AFTER setupAEScenario, so the preset is the last
// writer over resetScale2's defaults.
//
// Note: the force-arrow `net` channel = ionic + vdW + bond only —
// angle-strain / H-bond / dipole forces are NOT in the decomposition
// (mock-atom-engine aeGetForceDecomposition), so VSEPR presets light
// forceNet without claiming to show the angle force itself.

const BASE_VISUALS = Object.freeze({
    clouds: true,            // orbital electron clouds
    shells: true,            // nucleus strong-force glow
    bondStyle: 'cylinders',  // 'cylinders' | 'lines' | 'off'
    shellBounds: false,      // translucent shell boundary spheres
    lobes: false,            // p/d/f orbital lobes
    field: false,            // force-field heatmap + vectors
    forceIonic: false,
    forceVdw: false,
    forceBond: false,
    forceNet: false,
    velocities: false,       // per-atom velocity vectors
    dipoles: false,          // per-atom dipole-moment arrows
    hbondLines: false,       // dashed donor-H···acceptor lines
});

const IONIC_VISUALS    = Object.freeze({ forceIonic: true, field: true });
const VDW_VISUALS      = Object.freeze({ forceVdw: true });
const COVALENT_VISUALS = Object.freeze({ forceBond: true });
const VSEPR_VISUALS    = Object.freeze({ forceNet: true });
const WATER_VISUALS    = Object.freeze({ hbondLines: true });

const AE_PRESET_OVERRIDES = Object.freeze({
    'ae-periodic':              { clouds: false, shells: false },  // 118 atoms — perf
    'ae-hydrogen-atom':         { shellBounds: true },
    'ae-rutherford-scattering': { forceNet: true, velocities: true },
    'ae-he-cluster':            VDW_VISUALS,
    'ae-ar-cluster':            VDW_VISUALS,
    'ae-noble-mix':             VDW_VISUALS,
    'ae-collision':             { forceVdw: true, velocities: true },
    'ae-nacl-form':             IONIC_VISUALS,
    'ae-nacl-lattice':          IONIC_VISUALS,
    'ae-mgf2':                  IONIC_VISUALS,
    'ae-h2-form':               COVALENT_VISUALS,
    'ae-o2-form':               COVALENT_VISUALS,
    'ae-ch4-form':              COVALENT_VISUALS,
    'ae-fe-bcc':                COVALENT_VISUALS,
    'ae-cu-fcc':                COVALENT_VISUALS,
    'ae-water-dimer':           WATER_VISUALS,
    'ae-water-cluster':         WATER_VISUALS,
    'ae-vsepr-linear':          VSEPR_VISUALS,
    'ae-vsepr-tetrahedral':     VSEPR_VISUALS,
    'ae-vsepr-bent':            VSEPR_VISUALS,
    'ae-thermal-gas':           { shells: false, velocities: true },
    'ae-custom':                {},
});

/**
 * Visual preset for a scenario. ae-el-* (single elements) get shell
 * boundary spheres like ae-hydrogen-atom; everything else resolves via
 * the override table on top of BASE_VISUALS.
 * @param {string} name - scenario identifier (ae-*)
 * @returns {{ visuals: object }}
 */
export function getAEScenarioPreset(name) {
    const override = name.startsWith('ae-el-')
        ? { shellBounds: true }
        : (AE_PRESET_OVERRIDES[name] || {});
    return { visuals: { ...BASE_VISUALS, ...override } };
}


/**
 * Execute the scenario body for the given name.
 * @param {string} name - scenario identifier (ae-*)
 * @param {object} ctx  - shared context
 */
export function setupAEScenario(name, ctx) {
    const { bridge, viewport, inspector, helpers } = ctx;
    const aeSetPhase3 = helpers.setPhase3;

    // Typical spacing (in Bohr radii) used by the periodic-table scenario
    const S = 5;

    switch (name) {
        case 'ae-periodic': {
            // Full 118-element periodic table in standard 18-column layout
            const gap = S * 1.2;
            const elements = allElements();
            for (const el of elements) {
                const pos = tablePosition(el.Z);
                if (!pos) continue;
                let rowY = pos.row;
                if (pos.row >= 8) rowY = pos.row + 0.5; // extra gap before f-block
                const x = (pos.col - 9.5) * gap;
                const y = (1 - rowY) * gap;
                bridge.aeAddLockedAtom(el.Z, x, y, 0);
            }
            if (inspector) inspector.setScenarioInfo({
                title: 'Periodic Table',
                desc: 'All 118 elements in standard layout \u2014 atoms locked, no dynamics',
                fields: {
                    'Elements': '118',
                    'Layout': '18-column standard',
                    'State': 'All locked (static display)',
                }
            });
            if (viewport) {
                const centerY = -gap * 4;
                viewport.controls.target.set(0, centerY, 0);
                viewport.camera.position.set(0, centerY, 100);
                viewport.controls.update();
            }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // SINGLE-ATOM PHYSICS -- hydrogen atom + scattering demos
        // ══════════════════════════════════════════════════════════════
        case 'ae-hydrogen-atom': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(true);    document.getElementById('ae-ionic').checked = true;
            bridge.aeAddLockedAtom(1, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({
                title: 'Hydrogen Atom',
                desc: 'Single H atom \u2014 proton nucleus plus 1s electron cloud. Enable orbitals to see the probability shell.',
                fields: {
                    'Atoms': '1 \u00d7 H (locked)',
                    'Shell': 'n=1, \u2113=0 (1s)',
                    'Force': 'Nucleus-electron Coulomb',
                }
            });
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 15); viewport.controls.update(); }
            break;
        }
        case 'ae-rutherford-scattering': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(true);    document.getElementById('ae-ionic').checked = true;
            bridge.aeAddLockedAtom(79, 0, 0, 0);
            const b = 3.5;
            const startX = -25, vx = 0.8;
            bridge.aeAddAtom(2, startX, b, 0, vx, 0, 0, 2);
            if (inspector) inspector.setScenarioInfo({
                title: 'Rutherford Scattering',
                desc: '\u03b1 particle (He\u00b2\u207a) deflected by locked Au nucleus \u2014 Coulomb scattering.',
                fields: {
                    'Target': 'Au (Z=79, locked)',
                    'Projectile': '\u03b1 / He\u00b2\u207a',
                    'Impact param b': b.toFixed(1) + ' a\u2080',
                    'Force': 'Coulomb',
                }
            });
            if (viewport) { viewport.controls.target.set(-5, 0, 0); viewport.camera.position.set(0, 0, 50); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // NOBLE GAS CLUSTERS -- vdW only (no bonding, no ionic)
        // ══════════════════════════════════════════════════════════════
        case 'ae-he-cluster': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            const S = 5.5;
            const hex = [[0,0,0],[S,0,0],[S*0.5,S*0.866,0],
                         [0,0,S],[S,0,S],[S*0.5,S*0.866,S]];
            for (const [x, y, z] of hex)
                bridge.aeAddAtom(2, x - S*0.5, y - S*0.3, z - S*0.5,
                    (Math.random()-0.5)*0.2, (Math.random()-0.5)*0.2, (Math.random()-0.5)*0.2, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Helium Cluster',
                desc: 'Six He atoms \u2014 van der Waals (LJ 12-6) only. Watch them settle.',
                fields: { 'Atoms': '6 \u00d7 He', 'Force': 'vdW only', 'Bonding': 'None (noble gas)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 35); viewport.controls.update(); }
            break;
        }
        case 'ae-ar-cluster': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            const S = 6.0;
            for (let ix = 0; ix < 2; ix++) for (let iy = 0; iy < 2; iy++) for (let iz = 0; iz < 2; iz++)
                bridge.aeAddAtom(18, (ix-0.5)*S, (iy-0.5)*S, (iz-0.5)*S,
                    (Math.random()-0.5)*0.15, (Math.random()-0.5)*0.15, (Math.random()-0.5)*0.15, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Argon Cluster',
                desc: 'Eight Ar atoms in a cube \u2014 vdW condensation dynamics.',
                fields: { 'Atoms': '8 \u00d7 Ar', 'Force': 'vdW only', 'Layout': '2\u00d72\u00d72 cube' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 35); viewport.controls.update(); }
            break;
        }
        case 'ae-noble-mix': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            bridge.aeAddAtom(2, -12, 0, 0, 0.1, 0, 0, 0);
            bridge.aeAddAtom(2, -8, 0, 0, -0.1, 0, 0, 0);
            bridge.aeAddAtom(10, -2, 0, 0, 0.1, 0, 0, 0);
            bridge.aeAddAtom(10, 2, 0, 0, -0.1, 0, 0, 0);
            bridge.aeAddAtom(18, 7, 0, 0, 0.1, 0, 0, 0);
            bridge.aeAddAtom(18, 12, 0, 0, -0.1, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Noble Gas Mix',
                desc: 'He + Ne + Ar \u2014 different sizes interact via vdW only.',
                fields: { 'Atoms': '2 He + 2 Ne + 2 Ar', 'Force': 'vdW only' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 45); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // IONIC FORMATION -- Coulomb-driven, no covalent bonding
        // ══════════════════════════════════════════════════════════════
        case 'ae-nacl-form': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeAddAtom(11, -12, 0, 0, 0.15, 0, 0, 1);   // Na+
            bridge.aeAddAtom(17, 12, 0, 0, -0.15, 0, 0, -1);  // Cl-
            if (inspector) inspector.setScenarioInfo({ title: 'NaCl Formation',
                desc: 'Na\u207a and Cl\u207b attract via Coulomb force \u2014 ionic bond formation.',
                fields: { 'Atoms': 'Na\u207a + Cl\u207b', 'Force': 'Ionic (Coulomb)', 'Bonding': 'None (ionic)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 40); viewport.controls.update(); }
            break;
        }
        case 'ae-nacl-lattice': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetBondsForce(false); document.getElementById('ae-bonds-force').checked = false;
            const sp = 7.5;
            for (let ix = 0; ix < 3; ix++) for (let iy = 0; iy < 3; iy++) {
                const charge = ((ix + iy) % 2 === 0) ? 1 : -1;
                const Z = charge === 1 ? 11 : 17;
                bridge.aeAddAtom(Z, (ix-1)*sp, (iy-1)*sp, 0, 0, 0, 0, charge);
            }
            if (inspector) inspector.setScenarioInfo({ title: 'NaCl 3\u00d73 Lattice',
                desc: 'Ionic crystal lattice \u2014 alternating Na\u207a/Cl\u207b held by Coulomb.',
                fields: { 'Atoms': '9 (Na\u207a/Cl\u207b alternating)', 'Layout': '3\u00d73 grid', 'Force': 'Ionic + vdW' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 45); viewport.controls.update(); }
            break;
        }
        case 'ae-mgf2': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeAddAtom(12, 0, 0, 0, 0, 0, 0, 2);     // Mg2+
            bridge.aeAddAtom(9, -15, 0, 0, 0.2, 0, 0, -1);  // F-
            bridge.aeAddAtom(9, 15, 0, 0, -0.2, 0, 0, -1);  // F-
            if (inspector) inspector.setScenarioInfo({ title: 'MgF\u2082 Formation',
                desc: 'Mg\u00b2\u207a attracts two F\u207b ions \u2014 ionic bond formation.',
                fields: { 'Atoms': 'Mg\u00b2\u207a + 2 F\u207b', 'Force': 'Ionic (Coulomb)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 45); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // COVALENT FORMATION -- watch bonds form via auto-bonding
        // ══════════════════════════════════════════════════════════════
        case 'ae-h2-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            bridge.aeAddAtom(1, -7, 0, 0, 0.08, 0, 0, 0);
            bridge.aeAddAtom(1, 7, 0, 0, -0.08, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'H\u2082 Formation',
                desc: 'Two hydrogen atoms approach \u2014 vdW attracts, bond forms at r < 4.8.',
                fields: { 'Atoms': '2 \u00d7 H', 'Force': 'vdW + auto-bond', 'Threshold': '1.2 \u00d7 \u03c3_avg \u2248 4.8' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 25); viewport.controls.update(); }
            break;
        }
        case 'ae-o2-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            bridge.aeAddAtom(8, -5, 0, 0, 0.06, 0, 0, 0);
            bridge.aeAddAtom(8, 5, 0, 0, -0.06, 0, 0, 0);
            aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'O\u2082 Formation',
                desc: 'Two oxygen atoms approach and bond \u2014 double bond forms.',
                fields: { 'Atoms': '2 \u00d7 O', 'Force': 'vdW + auto-bond + angle strain' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 25); viewport.controls.update(); }
            break;
        }
        case 'ae-ch4-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            aeSetPhase3(bridge, { angle: true });
            const d = 9, t = 1 / Math.sqrt(3);
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, d*t, d*t, d*t, -0.05, -0.05, -0.05, 0);
            bridge.aeAddAtom(1, d*t, -d*t, -d*t, -0.05, 0.05, 0.05, 0);
            bridge.aeAddAtom(1, -d*t, d*t, -d*t, 0.05, -0.05, 0.05, 0);
            bridge.aeAddAtom(1, -d*t, -d*t, d*t, 0.05, 0.05, -0.05, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'CH\u2084 Assembly',
                desc: 'Carbon + 4 hydrogens approach \u2014 bonds form, angle strain drives tetrahedral.',
                fields: { 'Atoms': 'C + 4H', 'Target': '109.47\u00b0 tetrahedral', 'Force': 'vdW + bond + angle' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 30); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // H-BONDING -- pre-formed water molecules with hydrogen bonds
        // ══════════════════════════════════════════════════════════════
        case 'ae-water-dimer': {
            const ang = 104.5 * Math.PI / 180;
            const rOH = 3.4;
            bridge.aeAddAtom(8, -7, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -7 + rOH, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -7 + rOH*Math.cos(ang), rOH*Math.sin(ang), 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 7, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 7 - rOH, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 7 - rOH*Math.cos(ang), -rOH*Math.sin(ang), 0, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            aeSetPhase3(bridge, { hbonds: true, angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'Water Dimer',
                desc: 'Two H\u2082O molecules \u2014 H-bond attracts them. First Phase 3 demo!',
                fields: { 'Atoms': '6 (2 \u00d7 H\u2082O)', 'Force': 'Bond + H-bond + angle strain', 'H-bond': 'LJ 10-12 + angular' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 35); viewport.controls.update(); }
            break;
        }
        case 'ae-water-cluster': {
            const ang = 104.5 * Math.PI / 180;
            const rOH = 3.4;
            const N_mol = 5, R_ring = 16;
            for (let m = 0; m < N_mol; m++) {
                const theta = (2 * Math.PI * m) / N_mol;
                const ox = R_ring * Math.cos(theta), oy = R_ring * Math.sin(theta);
                bridge.aeAddAtom(8, ox, oy, 0, 0, 0, 0, 0);
                const tn = (2 * Math.PI * (m + 1)) / N_mol;
                const dnx = Math.cos(tn) - Math.cos(theta), dny = Math.sin(tn) - Math.sin(theta);
                const dn = Math.sqrt(dnx*dnx + dny*dny);
                bridge.aeAddAtom(1, ox + rOH*dnx/dn, oy + rOH*dny/dn, 0, 0, 0, 0, 0);
                const px = -dny/dn, py = dnx/dn;
                const h2x = Math.cos(ang)*dnx/dn + Math.sin(ang)*px;
                const h2y = Math.cos(ang)*dny/dn + Math.sin(ang)*py;
                bridge.aeAddAtom(1, ox + rOH*h2x, oy + rOH*h2y, 0, 0, 0, 0, 0);
            }
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            aeSetPhase3(bridge, { hbonds: true, angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'Water Pentamer',
                desc: 'Five H\u2082O molecules in a ring \u2014 H-bond network demonstration.',
                fields: { 'Atoms': '15 (5 \u00d7 H\u2082O)', 'Force': 'Bond + H-bond + angle', 'Pattern': 'Cyclic H-bond ring' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 55); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // VSEPR GEOMETRY -- start at wrong angle, watch relaxation
        // ══════════════════════════════════════════════════════════════
        case 'ae-vsepr-linear': {
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 2.0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 0, 2.0, 0, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'CO\u2082 VSEPR',
                desc: 'CO\u2082 starts bent (90\u00b0) \u2014 angle strain drives it to linear (180\u00b0).',
                fields: { 'Atoms': 'C + 2O', 'Start': '90\u00b0', 'Target': '180\u00b0 (linear)', 'Steric #': '2' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }
        case 'ae-vsepr-tetrahedral': {
            const d = 3.5;
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, d, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -d, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 0, d, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 0, 0, d, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'CH\u2084 VSEPR',
                desc: 'CH\u2084 starts at 90\u00b0 \u2014 angle strain relaxes to 109.47\u00b0 tetrahedral.',
                fields: { 'Atoms': 'C + 4H', 'Start': '90\u00b0', 'Target': '109.47\u00b0', 'Steric #': '4' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }
        case 'ae-vsepr-bent': {
            const r = 3.4;
            const theta0 = 150 * Math.PI / 180;
            bridge.aeAddAtom(8, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, r, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, r*Math.cos(theta0), r*Math.sin(theta0), 0, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'H\u2082O VSEPR',
                desc: 'H\u2082O starts at 150\u00b0 \u2014 lone pairs drive H-O-H toward 104.5\u00b0 bent.',
                fields: { 'Atoms': 'O + 2H', 'Start': '150\u00b0', 'Target': '104.5\u00b0', 'Lone pairs': '2' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // THERMAL DYNAMICS -- thermostat + gas kinetics
        // ══════════════════════════════════════════════════════════════
        case 'ae-thermal-gas': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeSetIonic(false);   document.getElementById('ae-ionic').checked = false;
            aeSetPhase3(bridge, { thermostat: true, temp: 1.0 });
            const L = 15;
            for (let n = 0; n < 12; n++) {
                const x = (Math.random()-0.5)*2*L, y = (Math.random()-0.5)*2*L, z = (Math.random()-0.5)*2*L;
                const speed = 0.3 + Math.random()*0.5;
                const phi = Math.random()*2*Math.PI, th = Math.acos(2*Math.random()-1);
                bridge.aeAddAtom(18, x, y, z,
                    speed*Math.sin(th)*Math.cos(phi), speed*Math.sin(th)*Math.sin(phi), speed*Math.cos(th), 0);
            }
            if (inspector) inspector.setScenarioInfo({ title: 'Thermal Gas',
                desc: '12 Ar atoms with Berendsen thermostat \u2014 temperature stabilizes at T=1.',
                fields: { 'Atoms': '12 \u00d7 Ar', 'Force': 'vdW only', 'Thermostat': 'ON (T=1.0)' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 55); viewport.controls.update(); }
            break;
        }
        case 'ae-collision': {
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            bridge.aeAddAtom(18, -20, 0, 0, 0.4, 0, 0, 0);
            bridge.aeAddAtom(18, 20, 0, 0, -0.4, 0, 0, 0);
            if (inspector) inspector.setScenarioInfo({ title: 'Head-On Collision',
                desc: 'Two Ar atoms approach at speed \u2014 LJ repulsion at short range.',
                fields: { 'Atoms': '2 \u00d7 Ar', 'Force': 'vdW (LJ 12-6)', 'Speed': '0.4 each' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 50); viewport.controls.update(); }
            break;
        }

        // ══════════════════════════════════════════════════════════════
        // METALLIC CLUSTERS -- multi-atom bonding
        // ══════════════════════════════════════════════════════════════
        case 'ae-fe-bcc': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            const a = 0.9;
            for (let ix = -1; ix <= 1; ix += 2)
                for (let iy = -1; iy <= 1; iy += 2)
                    for (let iz = -1; iz <= 1; iz += 2)
                        bridge.aeAddAtom(26, ix*a, iy*a, iz*a, 0, 0, 0, 0);
            bridge.aeAddAtom(26, 0, 0, 0, 0, 0, 0, 0);
            bridge.aePreBond();
            if (inspector) inspector.setScenarioInfo({ title: 'Fe BCC Cluster',
                desc: 'Iron atoms in body-centered cubic arrangement \u2014 metallic bonding.',
                fields: { 'Atoms': '9 \u00d7 Fe', 'Layout': 'BCC (8 corners + center)', 'Force': 'vdW + bond' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 15); viewport.controls.update(); }
            break;
        }
        case 'ae-cu-fcc': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            const a = 1.5;
            bridge.aeAddAtom(29, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(29, a, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(29, -a, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(29, 0, a, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(29, 0, -a, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(29, 0, 0, a, 0, 0, 0, 0);
            bridge.aeAddAtom(29, 0, 0, -a, 0, 0, 0, 0);
            bridge.aePreBond();
            if (inspector) inspector.setScenarioInfo({ title: 'Cu FCC Seed',
                desc: 'Copper atoms in face-centered cubic seed \u2014 nearest-neighbor bonding.',
                fields: { 'Atoms': '7 \u00d7 Cu', 'Layout': 'FCC (center + 6 face)', 'Force': 'vdW + bond' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 15); viewport.controls.update(); }
            break;
        }

        case 'ae-custom':
            if (inspector) inspector.setScenarioInfo(null);
            break;

        default: {
            // Handle individual element scenarios: ae-el-1 through ae-el-118
            const isElement = name.startsWith('ae-el-');
            if (isElement) {
                const Z = parseInt(name.slice(6));
                bridge.aeAddLockedAtom(Z, 0, 0, 0);
                const el = getElement(Z);
                if (inspector && el) {
                    const N = el.neutrons || 0;
                    const mass = (Z + N * NEUTRON_PROTON_MASS_RATIO).toFixed(2);
                    const period = el.row <= 7 ? el.row : (el.row === 8 ? '6 (Ln)' : '7 (An)');
                    inspector.setScenarioInfo({
                        title: el.name,
                        desc: `Isolated ${el.name} atom (Z = ${Z})`,
                        fields: {
                            'Symbol': el.symbol,
                            'Z': Z,
                            'Period': period,
                            'Group': el.col,
                            'Mass': mass + ' AMU',
                            'Max Bonds': el.maxBonds,
                        }
                    });
                }
                if (viewport) {
                    const dist = Z > 54 ? 50 : Z > 36 ? 40 : Z > 18 ? 30 : 20;
                    viewport.controls.target.set(0, 0, 0);
                    viewport.camera.position.set(0, 0, dist);
                    viewport.controls.update();
                }
            }
            break;
        }
    }

}
