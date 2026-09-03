/**
 * Scale 2 (Atoms) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns the Atom Engine (AE) frame loop, force decomposition rendering,
 * element legend building, orbital cloud merging, and AE visual state.
 *
 * Delegates to sibling modules:
 *   ./scenarios.js    — ae-* scenario setup (big switch, S2-1)
 *   ./ui-bindings.js  — DOM-coupled toggle/slider sync helpers (S2-2)
 *
 * OWNED STATE (module-private):
 *   - Visual flags: nucleus shells, bond style, orbital shells/lobes,
 *     per-force arrows, orbital clouds, AE field overlay
 *   - Element legend cache (_prevLegendKey)
 *   - AE initial energy reference (drift tracking)
 *   - Force throttle counter (_forceFrame)
 *   - Field grid cache for AE force overlay
 *   - Tick accumulator for sub-1 speed
 *
 * EXPORTS:
 *   animateAE(ctx)                - per-frame update
 *   loadAEScenario(ctx, name)     - atom scenario setup
 *   resetScale2(ctx)              - clear AE-specific state for mode switch
 *   syncAEParams(ctx)             - sync AE physics params from UI sliders
 *   getAEVisualState()            - read visual toggle flags
 *   setAEVisualToggle(key, value) - set a visual toggle flag from outside
 *   bindScale2ControlsUI()        - mount controls card (re-export)
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { getElement } from '../../elements.js';
import {
    expandAEToOrbitalCloud, generateBondingCloud,
    electronConfig, slaterZeff, A0_DISPLAY
} from '../../orbitals.js';
import {
    isotopeEnergy,
    formatEnergy as formatEnergyAE
} from '../../atomic-energy.js';
import { drawBindingEnergyCurve } from './ui/binding-energy-chart.js';
import { formatEnergy, formatTemperature } from '../../units.js';
import { M_E_PHYS } from '../../constants.js';
import { generateGridXZ, sampleAEField } from '../../fields.js';
import { createTickAccumulator, formatSI } from '../scale-utils.js';
import {
    syncAEParamsFromUI, resetAETogglesToDefaults,
    bindScale2ControlsUI, syncAENuclearControlsFromBridge
} from './ui-bindings.js';
import { setupAEScenario, getAEScenarioPreset } from './scenarios.js';
import { AE_PHYSICS_SPECS, getAEScenarioMeta } from './scenario-registry.js';
import {
    advanceAEExperiment, resetAEExperiment, startAEExperiment,
} from './experiment-runtime.js';
import { renderAEScenarioDescription } from './ui/dom.js';
import { telemetryHub } from '../../telemetry-hub.js';

// Re-export for app.js startup wiring
export { bindScale2ControlsUI };


// =====================================================================
// AE-Specific Module State
// =====================================================================

// -- Enhanced atom/molecule visual toggle flags -----------------------
let _showNucleusShells   = true;    // empirical A^(1/3) nuclear-extent envelopes
let _showElementLabels   = true;    // chemical/isotope labels anchored to records
let _bondStyle           = 'cylinders'; // 'cylinders' | 'lines' | 'off'
let _showShellBounds     = false;   // translucent shell boundary spheres
let _showOrbitalLobes    = false;   // p/d/f orbital lobe shapes
let _showAEForceIonic    = false;   // Coulomb force arrows
let _showAEForceVdw      = false;   // van der Waals force arrows
let _showAEForceBond     = false;   // bond spring force arrows
let _showAEForceHBond    = false;   // directional H-bond radial component
let _showAEForceAngle    = false;   // VSEPR angle-strain force
let _showAEForceDipole   = false;   // dipole-dipole force
let _showAEForceNet      = false;   // net force arrows
let _forceFrame          = 0;       // throttle: compute forces every 2nd frame

// -- Orbital cloud and field overlay ---------------------------------
let _showOrbitalClouds   = true;    // orbital electron clouds in AE mode
let _showAEField         = false;   // force field overlay (heatmap + vectors)
let _showBonds           = true;    // bond rendering (shared with Scale 3)

// -- Kinetic/electrostatic structure overlays (Scale 2 deep pass) -----
let _showAEVelocities    = false;   // per-atom velocity vectors
let _showAEDipoles       = false;   // per-atom dipole-moment arrows
let _showAEHBondLines    = false;   // dashed donor-H···acceptor lines
let _showAENuclearEvents = false;   // accepted-collision flashes + reaction planes
let _showAERadiation     = false;   // prompt neutron/gamma transport traces
let _showAEHeat          = false;   // deposited-energy halos
let _showAENuclearBoundary = false; // live neutron transport volume

// -- Element legend cache (avoid DOM rebuilds) -----------------------
let _prevLegendKey       = '';
const _aeLegendZSet      = new Set();  // reusable Set for legend key computation
const _aeLegendZArr      = [];         // reusable sorted array for legend key

// -- Element label buffer (reuse to avoid per-atom alloc every frame)
let _aeLabelBuf          = [];

// -- AE cloud merge buffers (reused to avoid 3x Float32Array alloc per frame)
let _aeMergeCap          = 0;
let _aeMergePos          = null;
let _aeMergeCol          = null;
let _aeMergeSize         = null;
let _aeMergeAtomMap      = null;

// -- Energy drift tracking -------------------------------------------
let _aeInitialEnergy     = null;    // captured at scenario load, before first tick

// -- Field computation cache -----------------------------------------
let _fieldGrid           = null;    // cached grid from generateGridXZ
let _fieldDirty          = true;    // force one paused-state refresh after a state/control change
let _fieldAtomCount      = -1;      // catches paused injections/removals without resampling every frame

// -- Tick accumulator (sub-1 speed fractional ticks, shared helper) --
const _tickAcc = createTickAccumulator();

// -- Paused-state dedup (avoid redundant work when simulation idle) --
let _statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };



// =====================================================================
// Internal Helpers
// =====================================================================

/**
 * Update atomic energy display cards (nuclear binding, B/A, electron
 * binding, FTD mass) for single-element or multi-element views.
 */
let _baChartDrawn = false;

const ISOTOPE_SUPERSCRIPT = Object.freeze({
    0: '⁰', 1: '¹', 2: '²', 3: '³', 4: '⁴',
    5: '⁵', 6: '⁶', 7: '⁷', 8: '⁸', 9: '⁹',
});

function isotopeLabel(Z, N) {
    if (Z === 0 && N === 1) return 'n';
    const el = getElement(Z);
    if (!el) return `Z${Z}`;
    const massNumber = String(Z + N).split('').map(digit => ISOTOPE_SUPERSCRIPT[digit]).join('');
    return `${massNumber}${el.symbol}`;
}

function updateAtomicEnergyDisplay(dom, atomData) {
    if (!dom.aeDiagMass || !atomData || atomData.count === 0) return;

    // The B/A-vs-mass-number curve is a pure function of Z=1..118 (SEMF),
    // not a live simulation quantity — draw it exactly once, not per-frame.
    if (!_baChartDrawn) {
        const chartCanvas = document.getElementById('ae-diag-ba-chart');
        if (chartCanvas) {
            drawBindingEnergyCurve(chartCanvas);
            _baChartDrawn = true;
        }
    }

    if (atomData.count === 1 && atomData.atomicNums) {
        // -- Single element --
        const Z = atomData.atomicNums[0];
        const N = atomData.neutronCounts?.[0] ?? getElement(Z)?.neutrons ?? Z;
        const e = isotopeEnergy(Z, N);
        dom.aeDiagMass.textContent = formatEnergyAE(e.massEnergy);
        dom.aeDiagNbe.textContent = formatEnergyAE(e.bindingEnergy);
        dom.aeDiagBa.textContent = e.bindingPerNucleon.toFixed(4) + ' MeV';
        dom.aeDiagEbe.textContent = (e.electronBinding / 1000).toFixed(2) + ' keV';
        dom.aeDiagMassKb.textContent = formatSI(e.massInKB) + ' k\u0299';
    } else if (atomData.atomicNums) {
        // -- Multiple elements (periodic table or molecule) --
        let totalMass = 0, totalBE = 0, totalNucleons = 0, totalEBE = 0;
        for (let i = 0; i < atomData.count; i++) {
            const Z = atomData.atomicNums[i];
            const N = atomData.neutronCounts?.[i] ?? getElement(Z)?.neutrons ?? Z;
            const e = isotopeEnergy(Z, N);
            totalMass += e.massEnergy;
            totalBE += e.bindingEnergy;
            totalNucleons += e.massNumber;
            totalEBE += e.electronBinding;
        }
        const avgBA = totalNucleons > 0 ? totalBE / totalNucleons : 0;
        dom.aeDiagMass.textContent = formatEnergyAE(totalMass);
        dom.aeDiagNbe.textContent = formatEnergyAE(totalBE);
        dom.aeDiagBa.textContent = avgBA.toFixed(4) + ' MeV';
        dom.aeDiagEbe.textContent = (totalEBE / 1e6).toFixed(2) + ' MeV';
        dom.aeDiagMassKb.textContent = formatSI(totalMass / M_E_PHYS);
    }
}


// =====================================================================
// Exported: resetScale2(ctx)
// =====================================================================

export function resetScale2(ctx) {
    const { bridge, viewport } = ctx;

    // Reset visual flags to defaults
    _showNucleusShells = true;
    _showElementLabels = true;
    _bondStyle         = 'cylinders';
    _showShellBounds   = false;
    _showOrbitalLobes  = false;
    _showAEForceIonic  = false;
    _showAEForceVdw    = false;
    _showAEForceBond   = false;
    _showAEForceHBond  = false;
    _showAEForceAngle  = false;
    _showAEForceDipole = false;
    _showAEForceNet    = false;
    _forceFrame        = 0;
    _showOrbitalClouds = true;
    _showAEField       = false;
    _showBonds         = true;
    _showAEVelocities  = false;
    _showAEDipoles     = false;
    _showAEHBondLines  = false;
    _showAENuclearEvents = false;
    _showAERadiation   = false;
    _showAEHeat        = false;
    _showAENuclearBoundary = false;
    _prevLegendKey     = '';
    _aeLabelBuf        = [];
    _aeMergeCap        = 0;
    _aeMergePos        = null;
    _aeMergeCol        = null;
    _aeMergeSize       = null;
    _aeMergeAtomMap    = null;
    _aeInitialEnergy   = null;
    _fieldGrid         = null;
    _fieldDirty        = true;
    _fieldAtomCount    = -1;
    _tickAcc.reset();
    resetAEExperiment(bridge);

    _statusCache = { tick: '', ptime: '', particles: '', energy: '', state: '' };


    if (viewport) {
        viewport.toggleNucleusShells(true);
        viewport.toggleBondCylinders(true);
        viewport.toggleOrbitalShells(false);
        viewport.toggleOrbitalLobes(false);
        viewport.toggleAEForceIonic(false);
        viewport.toggleAEForceVdw(false);
        viewport.toggleAEForceBond(false);
        viewport.toggleAEForceHBond(false);
        viewport.toggleAEForceAngle(false);
        viewport.toggleAEForceDipole(false);
        viewport.toggleAEForceNet(false);
        viewport.toggleFieldHeatmap(false);
        viewport.toggleFieldVectors(false);
        viewport.toggleBondLines(false);
        viewport.toggleVelocityVectors?.(false);
        viewport.toggleAEDipoles?.(false);
        viewport.toggleHBondLines?.(false);
        viewport.toggleNuclearEvents?.(false);
        viewport.toggleNuclearRadiation?.(false);
        viewport.toggleNuclearHeat?.(false);
        viewport.toggleNuclearBoundary?.(false);
        viewport.toggleElementLabels?.(true);
        viewport.updateElementLabels(null);
    }
}


// =====================================================================
// Exported: getAEVisualState() / setAEVisualToggle()
// =====================================================================

export function getAEVisualState() {
    return {
        showNucleusShells: _showNucleusShells,
        showElementLabels: _showElementLabels,
        bondStyle:         _bondStyle,
        showShellBounds:   _showShellBounds,
        showOrbitalLobes:  _showOrbitalLobes,
        showAEForceIonic:  _showAEForceIonic,
        showAEForceVdw:    _showAEForceVdw,
        showAEForceBond:   _showAEForceBond,
        showAEForceHBond:  _showAEForceHBond,
        showAEForceAngle:  _showAEForceAngle,
        showAEForceDipole: _showAEForceDipole,
        showAEForceNet:    _showAEForceNet,
        showOrbitalClouds: _showOrbitalClouds,
        showAEField:       _showAEField,
        showBonds:         _showBonds,
        showAEVelocities:  _showAEVelocities,
        showAEDipoles:     _showAEDipoles,
        showAEHBondLines:  _showAEHBondLines,
        showAENuclearEvents: _showAENuclearEvents,
        showAERadiation:   _showAERadiation,
        showAEHeat:        _showAEHeat,
        showAENuclearBoundary: _showAENuclearBoundary,
    };
}

export function setAEVisualToggle(key, value) {
    switch (key) {
        case 'showNucleusShells': _showNucleusShells = value; break;
        case 'showElementLabels': _showElementLabels = value; break;
        case 'bondStyle':         _bondStyle         = value; break;
        case 'showShellBounds':   _showShellBounds   = value; break;
        case 'showOrbitalLobes':  _showOrbitalLobes  = value; break;
        case 'showAEForceIonic':  _showAEForceIonic  = value; break;
        case 'showAEForceVdw':    _showAEForceVdw    = value; break;
        case 'showAEForceBond':   _showAEForceBond   = value; break;
        case 'showAEForceHBond':  _showAEForceHBond  = value; break;
        case 'showAEForceAngle':  _showAEForceAngle  = value; break;
        case 'showAEForceDipole': _showAEForceDipole = value; break;
        case 'showAEForceNet':    _showAEForceNet    = value; break;
        case 'showOrbitalClouds': _showOrbitalClouds = value; break;
        case 'showAEField':       _showAEField       = value; _fieldDirty = !!value; break;
        case 'showBonds':         _showBonds         = value; break;
        case 'showAEVelocities':  _showAEVelocities  = value; break;
        case 'showAEDipoles':     _showAEDipoles     = value; break;
        case 'showAEHBondLines':  _showAEHBondLines  = value; break;
        case 'showAENuclearEvents': _showAENuclearEvents = value; break;
        case 'showAERadiation':   _showAERadiation   = value; break;
        case 'showAEHeat':        _showAEHeat        = value; break;
        case 'showAENuclearBoundary': _showAENuclearBoundary = value; break;
        default:
            console.warn(`[Scale2] Unknown visual toggle: ${key}`);
            return;
    }
    if ([
        'showOrbitalClouds', 'showAEForceIonic', 'showAEForceVdw', 'showAEForceBond',
        'showAEForceHBond', 'showAEForceAngle', 'showAEForceDipole', 'showAEForceNet',
        'showAEVelocities', 'showAEDipoles', 'showAEHBondLines', 'showAENuclearEvents',
        'showAERadiation', 'showAEHeat', 'showAENuclearBoundary',
    ].includes(key)) {
        _prevLegendKey = '';
    }
}


// =====================================================================
// Scenario visual presets (mirror of scale1's applyPEOverlayPreset)
// =====================================================================

function _setButtonActive(id, on) {
    const btn = document.getElementById(id);
    if (!btn) return;
    btn.classList.toggle('active', !!on);
    btn.setAttribute('aria-pressed', on ? 'true' : 'false');
}

function _setCheckbox(id, on) {
    const el = document.getElementById(id);
    if (el) el.checked = !!on;
}

function createScenarioRandom(seed) {
    let state = (Number(seed) >>> 0) || 0x6d2b79f5;
    return () => {
        state = (state + 0x6d2b79f5) >>> 0;
        let t = state;
        t = Math.imul(t ^ (t >>> 15), t | 1);
        t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
        return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
    };
}

function applyAEScenarioPhysics(bridge, scenario) {
    if (!scenario?.physics) return;
    for (const spec of AE_PHYSICS_SPECS) {
        const enabled = !!scenario.physics[spec.key];
        bridge[spec.setter]?.(enabled);
        _setCheckbox(spec.elementId, enabled);
    }
    const params = scenario.parameters;
    if (params) {
        bridge.aeSetDt?.(params.dt);
        bridge.aeSetSoftening?.(params.softening);
        bridge.aeSetThermostatTemp?.(params.thermostatTemp);
        const dtSlider = document.getElementById('ae-dt-slider');
        const softSlider = document.getElementById('ae-soft-slider');
        const dtValue = document.getElementById('ae-dt-value');
        const softValue = document.getElementById('ae-soft-value');
        const thermostatSlider = document.getElementById('ae-thermostat-slider');
        const thermostatValue = document.getElementById('ae-thermostat-value');
        if (dtSlider) dtSlider.value = String(params.dt);
        if (softSlider) softSlider.value = String(params.softening);
        if (dtValue) dtValue.textContent = params.dt.toFixed(2);
        if (softValue) softValue.textContent = params.softening.toFixed(2);
        if (thermostatSlider) thermostatSlider.value = String(params.thermostatTemp);
        if (thermostatValue) thermostatValue.textContent = params.thermostatTemp.toFixed(2);
    }
}

/**
 * Apply a scenario's visual preset: module flags, DOM controls (shared
 * scale-2/3 toolbar checkboxes, bond-style select, force/field buttons),
 * and viewport layer visibility. Runs AFTER setupAEScenario so the
 * preset is the last writer over resetScale2's defaults.
 */
function applyAEVisualPreset(viewport, preset) {
    const v = preset?.visuals;
    if (!v) return;

    _showOrbitalClouds = !!v.clouds;
    _showElementLabels = v.labels !== false;
    _showNucleusShells = !!v.shells;
    _bondStyle         = v.bondStyle || 'cylinders';
    _showShellBounds   = !!v.shellBounds;
    _showOrbitalLobes  = !!v.lobes;
    _showAEField       = !!v.field;
    _showAEForceIonic  = !!v.forceIonic;
    _showAEForceVdw    = !!v.forceVdw;
    _showAEForceBond   = !!v.forceBond;
    _showAEForceHBond  = !!v.forceHbond;
    _showAEForceAngle  = !!v.forceAngle;
    _showAEForceDipole = !!v.forceDipole;
    _showAEForceNet    = !!v.forceNet;
    _showAEVelocities  = !!v.velocities;
    _showAEDipoles     = !!v.dipoles;
    _showAEHBondLines  = !!v.hbondLines;
    _showAENuclearEvents = !!v.nuclearEvents;
    _showAERadiation   = !!v.radiation;
    _showAEHeat        = !!v.heat;
    _showAENuclearBoundary = !!v.nuclearBoundary;
    _fieldDirty        = _showAEField;

    _setCheckbox('ae-show-clouds', _showOrbitalClouds);
    _setCheckbox('ae-show-labels', _showElementLabels);
    _setCheckbox('ae-show-shells', _showNucleusShells);
    _setCheckbox('ae-show-shell-bounds', _showShellBounds);
    _setCheckbox('ae-show-lobes', _showOrbitalLobes);
    const bondSelect = document.getElementById('bond-style-select');
    if (bondSelect) bondSelect.value = _bondStyle;
    _setButtonActive('ae-force-ionic', _showAEForceIonic);
    _setButtonActive('ae-force-vdw', _showAEForceVdw);
    _setButtonActive('ae-force-bond', _showAEForceBond);
    _setButtonActive('ae-force-hbond', _showAEForceHBond);
    _setButtonActive('ae-force-angle', _showAEForceAngle);
    _setButtonActive('ae-force-dipole', _showAEForceDipole);
    _setButtonActive('ae-force-net', _showAEForceNet);
    _setButtonActive('toggle-ae-field', _showAEField);
    _setButtonActive('toggle-ae-velocities', _showAEVelocities);
    _setButtonActive('toggle-ae-dipoles', _showAEDipoles);
    _setButtonActive('toggle-ae-hbonds', _showAEHBondLines);
    _setButtonActive('toggle-ae-nuclear-events', _showAENuclearEvents);
    _setButtonActive('toggle-ae-radiation', _showAERadiation);
    _setButtonActive('toggle-ae-heat', _showAEHeat);
    _setButtonActive('toggle-ae-nuclear-boundary', _showAENuclearBoundary);

    if (!viewport) return;
    viewport.toggleNucleusShells(_showNucleusShells);
    viewport.toggleOrbitalShells(_showShellBounds);
    viewport.toggleOrbitalLobes(_showOrbitalLobes);
    viewport.toggleBondCylinders(_bondStyle === 'cylinders');
    viewport.toggleBondLines(_bondStyle === 'lines');
    viewport.toggleAEForceIonic(_showAEForceIonic);
    viewport.toggleAEForceVdw(_showAEForceVdw);
    viewport.toggleAEForceBond(_showAEForceBond);
    viewport.toggleAEForceHBond(_showAEForceHBond);
    viewport.toggleAEForceAngle(_showAEForceAngle);
    viewport.toggleAEForceDipole(_showAEForceDipole);
    viewport.toggleAEForceNet(_showAEForceNet);
    viewport.toggleFieldHeatmap(_showAEField);
    viewport.toggleFieldVectors(_showAEField);
    // Velocity / dipole / H-bond layers are updated per-frame in animateAE
    // when their flags are on (and hidden by their toggles when off).
    viewport.toggleVelocityVectors?.(_showAEVelocities);
    viewport.toggleAEDipoles?.(_showAEDipoles);
    viewport.toggleHBondLines?.(_showAEHBondLines);
    viewport.toggleNuclearEvents?.(_showAENuclearEvents);
    viewport.toggleNuclearRadiation?.(_showAERadiation);
    viewport.toggleNuclearHeat?.(_showAEHeat);
    viewport.toggleNuclearBoundary?.(_showAENuclearBoundary);
    viewport.toggleElementLabels?.(_showElementLabels);
}


// =====================================================================
// Exported: syncAEParams(ctx)
// =====================================================================

export function syncAEParams(ctx) {
    syncAEParamsFromUI(ctx.bridge);
}


// =====================================================================
// Exported: animateAE(ctx)
// =====================================================================

export function animateAE(ctx) {
    const {
        bridge, viewport, running, ticksPerFrame, inspector,
        activeTab, frameCount, dom, now,
        updatePlayButton,
        setRunning, engineMode
    } = ctx;

    // ── 1. Tick AE while running ───────────────────────────────────
    if (running) {
        const wholeTicks = _tickAcc.accumulate(ticksPerFrame);
        for (let i = 0; i < wholeTicks; i++) {
            try {
                bridge.aeTick();
                advanceAEExperiment(bridge);
            } catch (e) {
                console.error('[FTD] aeTick exception:', e);
                if (setRunning) setRunning(false);
                if (updatePlayButton) updatePlayButton();
                return;
            }
        }
    }

    // ── 2. Get atom data and validate positions ────────────────────
    const atomData = bridge.aeGetAtomData();

    if (running && atomData.count > 0) {
        for (let i = 0; i < atomData.count; i++) {
            const x = atomData.positions[i * 3];
            const y = atomData.positions[i * 3 + 1];
            const z = atomData.positions[i * 3 + 2];
            if (!isFinite(x) || !isFinite(y) || !isFinite(z)) {
                console.error(`[FTD] Atom ${i} has non-finite position: (${x}, ${y}, ${z})`);
                if (setRunning) setRunning(false);
                if (updatePlayButton) updatePlayButton();
                break;
            }
            if (Math.abs(x) > 1e4 || Math.abs(y) > 1e4 || Math.abs(z) > 1e4) {
                console.warn(`[FTD] Atom ${i} flew off: (${x.toFixed(1)}, ${y.toFixed(1)}, ${z.toFixed(1)})`);
                if (setRunning) setRunning(false);
                if (updatePlayButton) updatePlayButton();
                break;
            }
        }
    }
    if (atomData.count === 0 && engineMode === 'molecules') {
        console.warn('[FTD] No atoms in molecule data — bridge may have been re-initialized');
    }

    // ── 3. Render as orbital electron clouds or plain atom points ──
    let cloudData = null;
    if (_showOrbitalClouds && atomData.count > 0 && atomData.atomicNums) {
        const t = now * 0.001;
        cloudData = expandAEToOrbitalCloud(atomData, t);

        // ── 4. Merge bonding electron clouds into particle data ────
        if (_bondStyle !== 'off' && atomData.bondCount > 0) {
            const bondCloud = generateBondingCloud(atomData);
            if (bondCloud.count > 0) {
                const mergedCount = cloudData.count + bondCloud.count;
                if (mergedCount > _aeMergeCap) {
                    _aeMergeCap = Math.max(mergedCount, _aeMergeCap * 2);
                    _aeMergePos = new Float32Array(_aeMergeCap * 3);
                    _aeMergeCol = new Float32Array(_aeMergeCap * 3);
                    _aeMergeSize = new Float32Array(_aeMergeCap);
                    _aeMergeAtomMap = new Int32Array(_aeMergeCap);
                }
                _aeMergePos.set(cloudData.positions.subarray(0, cloudData.count * 3));
                _aeMergeCol.set(cloudData.colors.subarray(0, cloudData.count * 3));
                _aeMergeSize.set(cloudData.sizes.subarray(0, cloudData.count));
                _aeMergeAtomMap.set(cloudData.atomMap.subarray(0, cloudData.count));
                _aeMergePos.set(bondCloud.positions.subarray(0, bondCloud.count * 3), cloudData.count * 3);
                _aeMergeCol.set(bondCloud.colors.subarray(0, bondCloud.count * 3), cloudData.count * 3);
                _aeMergeSize.set(bondCloud.sizes.subarray(0, bondCloud.count), cloudData.count);
                _aeMergeAtomMap.fill(-1, cloudData.count, mergedCount);
                cloudData = {
                    positions: _aeMergePos,
                    colors: _aeMergeCol,
                    sizes: _aeMergeSize,
                    count: mergedCount,
                    atomMap: _aeMergeAtomMap,
                };
            }
        }

        viewport.updateParticles(cloudData);
    } else {
        viewport.updateParticles(atomData);
    }

    if (inspector) {
        if (_showOrbitalClouds && cloudData?.atomMap) {
            inspector.setAEContext(atomData, cloudData.atomMap, true);
        } else {
            inspector.setAEContext(atomData, null, false);
        }
    }

    // ── 5. Update bond rendering ───────────────────────────────────
    if (_bondStyle === 'cylinders' && atomData.bondCount > 0) {
        viewport.updateBondCylinders(atomData);
        viewport.toggleBondCylinders(true);
        viewport.toggleBondLines(false);
    } else if (_bondStyle === 'lines' && atomData.bondCount > 0) {
        viewport.updateBondLines(atomData);
        viewport.toggleBondLines(true);
        viewport.toggleBondCylinders(false);
    } else {
        viewport.toggleBondCylinders(false);
        viewport.toggleBondLines(false);
    }

    // ── 6. Update empirical nuclear-extent envelopes ──────────────
    if (_showNucleusShells && atomData.count > 0) {
        viewport.updateNucleusShells(atomData);
    } else if (_showNucleusShells) {
        viewport.updateNucleusShells(null);
    }

    if (_showShellBounds && atomData.count > 0) {
        viewport.updateOrbitalShells(atomData, electronConfig, slaterZeff, A0_DISPLAY);
        viewport.toggleOrbitalShells(true);
    } else if (_showShellBounds) {
        viewport.updateOrbitalShells(null, electronConfig, slaterZeff, A0_DISPLAY);
    }

    if (_showOrbitalLobes && atomData.count > 0) {
        viewport.updateOrbitalLobes(atomData, electronConfig, slaterZeff, A0_DISPLAY);
        viewport.toggleOrbitalLobes(true);
    } else if (_showOrbitalLobes) {
        viewport.updateOrbitalLobes(null, electronConfig, slaterZeff, A0_DISPLAY);
    }

    // ── 7. Update per-atom force arrows (every 2nd frame) ──────────
    // Request only visible output arrays. Net force is nevertheless evaluated
    // from the complete active kernel so its arrow exactly matches the force
    // used by the integrator rather than a presentation-only partial sum.
    const anyForce = _showAEForceIonic || _showAEForceVdw || _showAEForceBond ||
        _showAEForceHBond || _showAEForceAngle || _showAEForceDipole || _showAEForceNet;
    if (anyForce && atomData.count > 0) {
        _forceFrame++;
        if (_forceFrame % 2 === 0) {
            const forceData = bridge.aeGetForceDecomposition({
                ionic: _showAEForceIonic,
                vdw:   _showAEForceVdw,
                bond:  _showAEForceBond,
                hbond: _showAEForceHBond,
                angle: _showAEForceAngle,
                dipole: _showAEForceDipole,
                net:   _showAEForceNet,
            });
            viewport.updateAEForces(atomData.positions, forceData, forceData.count);
        }
    } else if (anyForce) {
        viewport.updateAEForces(null, null, 0);
    }

    // ── 7b. Kinetic/electrostatic structure overlays ────────────────
    // Velocity vectors, dipole arrows, dashed H-bond lines. All three are
    // cheap O(N)/O(N·acceptors) reads; updated only while their flag is on.
    if (_showAEVelocities && atomData.count > 0) {
        const vel = bridge.aeGetVelocities?.();
        if (vel && vel.count > 0) {
            viewport.updateVelocityVectors(atomData.positions, vel.velocities, vel.count);
        }
    } else if (_showAEVelocities) {
        viewport.updateVelocityVectors(null, new Float32Array(0), 0);
    }
    if (_showAEDipoles && atomData.count > 0) {
        const dip = bridge.aeGetDipoles?.();
        if (dip && dip.count > 0) {
            viewport.updateAEDipoles(atomData.positions, dip.dipoles, dip.count);
        }
    } else if (_showAEDipoles) {
        viewport.updateAEDipoles(null, null, 0);
    }
    if (_showAEHBondLines && atomData.count > 0) {
        const hb = bridge.aeGetHBondPairs?.();
        if (hb) viewport.updateHBondLines(hb.segments, hb.count);
    } else if (_showAEHBondLines) {
        viewport.updateHBondLines(null, 0);
    }
    viewport.updateNuclearEffects?.(bridge.aeGetNuclearVisuals?.(), {
        events: _showAENuclearEvents,
        radiation: _showAERadiation,
        heat: _showAEHeat,
        boundary: _showAENuclearBoundary,
    });

    // ── 8. Update element labels ───────────────────────────────────
    if (_showElementLabels && atomData.count > 0 && atomData.atomicNums) {
        const showIsotopes = !!bridge.aeGetNuclearDiagnostics?.();
        const isotopeSeen = new Map();
        let labelCount = 0;
        for (let i = 0; i < atomData.count; i++) {
            const Z = atomData.atomicNums[i];
            const N = atomData.neutronCounts?.[i] ?? getElement(Z)?.neutrons ?? Z;
            const isotopeKey = `${Z}:${N}`;
            const seen = isotopeSeen.get(isotopeKey) || 0;
            isotopeSeen.set(isotopeKey, seen + 1);
            // Large nuclear populations remain legible by sampling repeated
            // isotope labels. The atoms themselves and legend stay complete.
            const labelCap = !showIsotopes || atomData.count <= 12
                ? Infinity
                : Z === 0 ? 5 : Z === 92 ? 3 : 3;
            if (seen >= labelCap) continue;
            const r = Math.round(atomData.colors[i * 3] * 255);
            const g = Math.round(atomData.colors[i * 3 + 1] * 255);
            const b = Math.round(atomData.colors[i * 3 + 2] * 255);
            const lum = 0.299 * r + 0.587 * g + 0.114 * b;
            while (_aeLabelBuf.length <= labelCount) {
                _aeLabelBuf.push({ x: 0, y: 0, z: 0, symbol: '', color: '#ffffff' });
            }
            const lbl = _aeLabelBuf[labelCount++];
            lbl.x = atomData.positions[i * 3];
            lbl.y = atomData.positions[i * 3 + 1];
            lbl.z = atomData.positions[i * 3 + 2];
            lbl.symbol = showIsotopes ? isotopeLabel(Z, N) : (getElement(Z)?.symbol || 'n');
            lbl.color = lum > 200 ? '#aaaaaa' : '#ffffff';
        }
        _aeLabelBuf.length = labelCount;
        viewport.updateElementLabels(_aeLabelBuf);
    } else {
        viewport.updateElementLabels(null);
    }

    // Update element legend (only rebuild when set of elements changes)
    if (dom.aeLegend && atomData.count > 0 && atomData.atomicNums) {
        if (frameCount % 10 === 0) {
            const nuclearDiag = bridge.aeGetNuclearDiagnostics?.();
            _aeLegendZSet.clear();
            for (let i = 0; i < atomData.count; i++) _aeLegendZSet.add(atomData.atomicNums[i]);
            _aeLegendZArr.length = 0;
            for (const z of _aeLegendZSet) _aeLegendZArr.push(z);
            _aeLegendZArr.sort((a, b) => a - b);
            const key = _aeLegendZArr.join(',')
                + (_showOrbitalClouds ? '+c' : '')
                + (_showAEForceIonic ? '+Fi' : '')
                + (_showAEForceVdw ? '+Fv' : '')
                + (_showAEForceBond ? '+Fb' : '')
                + (_showAEForceHBond ? '+Fh' : '')
                + (_showAEForceAngle ? '+Fa' : '')
                + (_showAEForceDipole ? '+Fμ' : '')
                + (_showAEForceNet ? '+Fn' : '')
                + (_showAEVelocities ? '+v' : '')
                + (_showAEDipoles ? '+mu' : '')
                + (_showAEHBondLines ? '+hb' : '')
                + (_showAENuclearEvents ? '+ne' : '')
                + (_showAERadiation ? '+nr' : '')
                + (_showAEHeat ? '+nh' : '')
                + (_showAENuclearBoundary ? '+nb' : '');
            const fullKey = key + (nuclearDiag
                ? `+nr:${nuclearDiag.channel}:${nuclearDiag.phase}:${nuclearDiag.eventCount}:${nuclearDiag.generation}:${nuclearDiag.liveNeutrons}:${nuclearDiag.kEffective.toFixed(3)}`
                : '');
            if (fullKey !== _prevLegendKey) {
                _prevLegendKey = fullKey;
                let html = '<div class="ae-legend-header">Elements</div>';
                for (let k = 0; k < _aeLegendZArr.length; k++) {
                    const Z = _aeLegendZArr[k];
                    const el = getElement(Z) || { symbol: 'n', name: 'Free neutron', color: [0.5, 0.5, 0.5] };
                    const [r, g, b] = el.color;
                    const hex = `#${(r * 255 | 0).toString(16).padStart(2, '0')}${(g * 255 | 0).toString(16).padStart(2, '0')}${(b * 255 | 0).toString(16).padStart(2, '0')}`;
                    html += `<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:${hex}"></span><span class="ae-legend-sym">${el.symbol}</span><span class="ae-legend-name">${el.name}</span></div>`;
                }
                if (_showOrbitalClouds) {
                    html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Substructure (decorative)</div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-proton"></span><span class="ae-legend-name">Protons</span></div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-neutron"></span><span class="ae-legend-name">Neutrons</span></div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-orb-s"></span><span class="ae-legend-name">s orbitals</span></div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-orb-p"></span><span class="ae-legend-name">p orbitals</span></div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-orb-d"></span><span class="ae-legend-name">d orbitals</span></div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-orb-f"></span><span class="ae-legend-name">f orbitals</span></div>';
                }
                const anyForce = _showAEForceIonic || _showAEForceVdw || _showAEForceBond ||
                    _showAEForceHBond || _showAEForceAngle || _showAEForceDipole || _showAEForceNet;
                if (anyForce) {
                    html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Force arrows</div>';
                    if (_showAEForceIonic) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-ionic"></span><span class="ae-legend-name">F<sub>C</sub> Coulomb</span></div>';
                    if (_showAEForceVdw) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-vdw"></span><span class="ae-legend-name">F<sub>vdW</sub> LJ 12-6</span></div>';
                    if (_showAEForceBond) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-bond"></span><span class="ae-legend-name">F<sub>B</sub> bond spring</span></div>';
                    if (_showAEForceHBond) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-hbond"></span><span class="ae-legend-name">F<sub>HB</sub> H-bond</span></div>';
                    if (_showAEForceAngle) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-angle"></span><span class="ae-legend-name">F<sub>θ</sub> angle strain</span></div>';
                    if (_showAEForceDipole) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-dipole"></span><span class="ae-legend-name">F<sub>μμ</sub> dipole</span></div>';
                    if (_showAEForceNet) {
                        html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-force-net"></span><span class="ae-legend-name">F<sub>net</sub> actual integrator force</span></div>';
                        html += '<p class="ae-legend-note">F<sub>net</sub> is the complete post-safety force used by the integrator.</p>';
                    }
                }
                if (_showAEVelocities) {
                    html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Kinetics</div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-velocity-ramp"></span><span class="ae-legend-name">|v|/c — green → white</span></div>';
                }
                if (_showAEDipoles) {
                    html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Dipoles</div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-dipole"></span><span class="ae-legend-name">Bond μ from χ differences</span></div>';
                }
                if (_showAEHBondLines) {
                    html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">H-bonds</div>';
                    html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-hbond"></span><span class="ae-legend-name">Dashed H&#183;&#183;&#183;A pairs</span></div>';
                }
                if (nuclearDiag) {
                    const nuclearPhase = nuclearDiag.phase === 'multiplying' ? 'reaction active'
                        : nuclearDiag.phase === 'transport' ? 'neutron transport active'
                        : nuclearDiag.phase === 'event-limit' ? 'reaction complete · carrier aftermath'
                            : nuclearDiag.phase === 'fuel-depleted' ? 'reaction complete · fuel depleted'
                                : nuclearDiag.phase === 'extinct' ? 'reaction complete · chain extinct'
                                    : nuclearDiag.phase;
                    html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Nuclear reaction</div>';
                    html += `<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#f97316"></span><span class="ae-legend-name">${nuclearDiag.label}</span></div>`;
                    if (_showAENuclearEvents) html += '<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-nuclear-event"></span><span class="ae-legend-name">Accepted collision site / reaction plane</span></div>';
                    if (_showAERadiation) html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#67e8f9"></span><span class="ae-legend-name">Neutron / gamma transport</span></div>';
                    if (_showAEHeat) html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#fb923c"></span><span class="ae-legend-name">Deposited-energy halo</span></div>';
                    if (_showAENuclearBoundary) html += `<div class="ae-legend-item"><span class="ae-legend-swatch ae-legend-swatch-nuclear-boundary-${nuclearDiag.boundaryMode === 'reflect' ? 'reflect' : 'leak'}"></span><span class="ae-legend-name">${nuclearDiag.boundaryMode === 'reflect' ? 'Reflective' : 'Open'} neutron boundary</span></div>`;
                    html += `<p class="ae-legend-note">${nuclearPhase} · ${nuclearDiag.eventCount} rendered event${nuclearDiag.eventCount === 1 ? '' : 's'} · ${nuclearDiag.liveNeutrons} live neutron${nuclearDiag.liveNeutrons === 1 ? '' : 's'} · generation ${nuclearDiag.generation} · observed reproduction ${nuclearDiag.kEffective.toFixed(3)} · mass-channel Q ${nuclearDiag.qMeV.toFixed(3)} MeV/event · total recoverable ${nuclearDiag.releasedMeV.toExponential(3)} MeV (${nuclearDiag.releasedJoule.toExponential(3)} J)</p>`;
                    if (['event-limit', 'fuel-depleted', 'extinct', 'complete'].includes(nuclearDiag.phase)) {
                        html += '<p class="ae-legend-note">Released energy is cumulative and now remains flat; visible packets and halos are the labeled aftermath, not additional reactions.</p>';
                    }
                }
                dom.aeLegend.innerHTML = html;
            }
        }
    } else if (dom.aeLegend && _prevLegendKey !== '') {
        dom.aeLegend.innerHTML = '';
        _prevLegendKey = '';
    }

    // ── 9. Update force field overlay (heatmap + vectors) ──────────
    if (_showAEField && atomData.count > 0 &&
        (running || _fieldDirty || atomData.count !== _fieldAtomCount || frameCount % 15 === 0)) {
        let maxR = 5;
        for (let i = 0; i < atomData.count; i++) {
            const ax = Math.abs(atomData.positions[i * 3]);
            const az = Math.abs(atomData.positions[i * 3 + 2]);
            if (ax > maxR) maxR = ax;
            if (az > maxR) maxR = az;
        }
        const extent = maxR + 5;
        if (!_fieldGrid || Math.abs(_fieldGrid.extent - extent) > 1) {
            _fieldGrid = generateGridXZ(extent, 20);
        }
        const src = bridge.aeGetFieldSources();
        const field = sampleAEField(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateFieldHeatmap(_fieldGrid.positions, field.potentials, _fieldGrid.count, field.maxPotential);
        // Atom positions are already world coordinates. The shared field
        // renderer defaults to the Scale-0 half-voxel shift, so pass an
        // explicit zero offset to keep the AE vectors aligned with both the
        // atom centers and the potential samples.
        viewport.updateFieldVectors(_fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce, 3.0, 0);
        _fieldDirty = false;
        _fieldAtomCount = atomData.count;
    } else if (_showAEField && atomData.count === 0) {
        viewport.updateFieldHeatmap(new Float32Array(0), new Float32Array(0), 0, 0);
        viewport.updateFieldVectors(new Float32Array(0), new Float32Array(0), 0, 0, 3.0, 0);
        _fieldDirty = false;
        _fieldAtomCount = 0;
    }

    // ── 10. Render viewport ────────────────────────────────────────
    viewport.render();

    // ── 11. AE diagnostics (throttled to every 3rd frame) ──────────
    if (frameCount % 3 === 0) {
        const diag = telemetryHub.collectScale2(bridge);

        if (diag) {
            const sTick = formatSI(diag.tick);
            const sParticles = String(diag.atomCount);
            const nuclear = diag.nuclear;
            let sEnergy = formatEnergy(diag.totalEnergy, 2).text;
            if (nuclear) {
                const joule = nuclear.releasedJoule || 0;
                if (nuclear.eventWeight > 1) {
                    sEnergy = joule >= 1e9 ? `${(joule / 1e9).toFixed(3)} GJ`
                        : joule >= 1e6 ? `${(joule / 1e6).toFixed(3)} MJ`
                            : joule >= 1e3 ? `${(joule / 1e3).toFixed(3)} kJ`
                                : `${joule.toFixed(3)} J`;
                } else {
                    sEnergy = `${nuclear.microscopicReleasedMeV.toFixed(3)} MeV`;
                }
            }
            const sState = running ? 'Running' : 'Idle';

            if (_statusCache.tick !== sTick) { dom.statusPtime.textContent = sTick; _statusCache.tick = sTick; }
            if (_statusCache.particles !== sParticles) { dom.statusParticles.textContent = sParticles; _statusCache.particles = sParticles; }
            if (_statusCache.energy !== sEnergy) { dom.statusEnergy.textContent = sEnergy; _statusCache.energy = sEnergy; }
            if (_statusCache.state !== sState) {
                dom.statusState.textContent = sState;
                _statusCache.state = sState;
                if (running) dom.statusDot.classList.remove('idle');
                else dom.statusDot.classList.add('idle');
            }

            if (dom.aeDiagCount) dom.aeDiagCount.textContent = diag.atomCount;
            if (dom.aeDiagBonds) dom.aeDiagBonds.textContent = diag.bondCount;
            if (dom.aeDiagKe) dom.aeDiagKe.textContent = formatEnergy(diag.totalKE, 2).text;
            if (dom.aeDiagEtotal) dom.aeDiagEtotal.textContent = formatEnergy(diag.totalEnergy, 2).text;
            if (dom.aeDiagPeIonic) dom.aeDiagPeIonic.textContent = formatEnergy(diag.totalPEIonic, 2).text;
            if (dom.aeDiagPeVdw) dom.aeDiagPeVdw.textContent = formatEnergy(diag.totalPEVdw, 2).text;
            if (dom.aeDiagPeBond) dom.aeDiagPeBond.textContent = formatEnergy(diag.totalPEBond, 2).text;
            if (dom.aeDiagTemp) dom.aeDiagTemp.textContent = formatTemperature(diag.temperature, 2).text;
            const pMag = Math.sqrt(diag.momentumX ** 2 + diag.momentumY ** 2 + diag.momentumZ ** 2);
            if (dom.aeDiagMomentum) dom.aeDiagMomentum.textContent = pMag.toFixed(6) + ' AMU\u00b7\u00c5/step';
            if (dom.aeDiagTick) dom.aeDiagTick.textContent = sTick;

            if (_aeInitialEnergy === null && diag.totalEnergy !== 0) {
                _aeInitialEnergy = diag.totalEnergy;
            }
            if (_aeInitialEnergy !== null && dom.aeDiagDrift) {
                const drift = ((diag.totalEnergy - _aeInitialEnergy) / Math.abs(_aeInitialEnergy)) * 100;
                dom.aeDiagDrift.textContent = drift.toFixed(4) + '%';
            }

            updateAtomicEnergyDisplay(dom, atomData);

            // NOTE (2026-07-29): the legacy null-canvas FluxEnergyChart /
            // ParticleChart pushes were removed here, same as Scale 1's.
            // They were wired to telemetryHub RingBufferViews (no push
            // method), so every call threw a swallowed page error; the hub's
            // _s2_ae ring is the AE history the charts panel reads.
        }

        switch (activeTab) {
            case 'inspector':
                inspector.update();
                break;
        }
    }
}


// =====================================================================
// Exported: loadAEScenario(ctx, name)
// =====================================================================

export function loadAEScenario(ctx, name) {
    const { bridge, viewport, inspector } = ctx;

    if (!bridge.initAE) return;
    ctx.resetAllVisualState();
    bridge.initAE();

    // Re-baseline hub telemetry (ring buffers + drift reference) so the new
    // scenario doesn't inherit the previous one's energy baseline (A6).
    telemetryHub.resetScale(2);

    // Reset all AE toggles to defaults, then sync sliders from UI
    resetAETogglesToDefaults(bridge);
    syncAEParamsFromUI(bridge);
    const scenario = getAEScenarioMeta(name);
    bridge.aeConfigureNuclearReaction?.(scenario?.nuclear || scenario?.reaction || '');
    syncAENuclearControlsFromBridge(bridge);

    // Clear molecule info (molecules are Scale 3 only)
    if (inspector) inspector.setCurrentMolecule(null);

    // Run scenario-specific setup (big switch delegated to scenarios.js)
    setupAEScenario(name, {
        bridge, viewport, inspector,
        helpers: {
            random: createScenarioRandom(scenario?.seed),
        },
    });

    // The registry is the final physics writer. Scenario setup may temporarily
    // enable auto-bonding to create topology, but the published runtime profile
    // and controls always settle to this contract.
    applyAEScenarioPhysics(bridge, scenario);

    // Start any declared presentation-level intervention schedule only after
    // the canonical scenario profile is established. The protocol runner uses
    // generic bridge controls and never enters a force kernel.
    startAEExperiment(scenario, bridge);

    // Apply the scenario's visual preset (flags + DOM controls + viewport
    // layers). Last writer after resetAllVisualState + setupAEScenario.
    applyAEVisualPreset(viewport, getAEScenarioPreset(name));

    // Capture initial energy reference for drift tracking (before first tick)
    const initDiag = bridge.aeGetDiagnostics();
    if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;

    renderAEScenarioDescription(name);
}

// =====================================================================
// Lifecycle Controller (unified pattern — see scale1/controller.js)
// =====================================================================
//
// Brings Scale 2 into parity with scales 0/1/4/5/6: a singleton
// BaseLifecycleController whose destroy() reclaims any tracked
// listeners/timers/Three.js objects (super.destroy) BEFORE running the
// existing resetScale2() visual reset. Today Scale 2 binds no raw
// listeners/timers/Three.js objects (verified by grep), so super.destroy
// is a no-op and this wrapper is purely defensive — it makes any FUTURE
// `this.bindEvent`/`this.setInterval`/`this.trackThreeObject` here
// automatically reclaimed on mode switch. The exported mount/destroy
// signatures are unchanged so app.js's CONTROLLERS registry is unaffected.
//
// NOTE: resetScale2(ctx) above stays a pure visual reset (NOT routed
// through the lifecycle) because app.js calls it directly mid-session as
// the cross-scale visual reset (_resetAllVisualState); tearing down
// lifecycle resources there would change behavior.

class Scale2LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
    }

    mount(ctx) {
        // Standard setup placeholder
    }

    destroy(ctx) {
        // Reclaim tracked resources first, then run the existing reset.
        super.destroy(ctx);
        resetScale2(ctx);
    }
}

const _lifecycleController = new Scale2LifecycleController();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}
