/**
 * Scale 2 (Atoms) Controller
 * ────────────────────────────────────────────────────────────────────
 *
 * Owns the Atom Engine (AE) frame loop, scenario loading, force
 * decomposition rendering, element legend building, orbital cloud
 * merging, and all AE-specific visual state.  Extracted from app.js
 * to keep each scale's logic isolated and independently testable.
 *
 * WHY THIS EXISTS:
 *   app.js grew to 4500+ lines with six scale-specific animation loops
 *   and scenario loaders tangled together.  This module encapsulates
 *   everything Scale 2 needs so the main app becomes a thin dispatcher.
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
 * CONTEXT OBJECT (ctx):
 *   The caller passes a ctx bag containing shared application state.
 *   This avoids coupling to app.js module-level variables.
 *
 *   Required ctx properties:
 *     bridge            - WASMBridge (with AE methods)
 *     viewport          - Viewport (Three.js) renderer
 *     running           - boolean, true when simulation is playing
 *     ticksPerFrame     - number, simulation ticks per render frame
 *     inspector         - Inspector panel instance
 *     fluxEnergyChart   - FluxEnergyChart instance
 *     particleChart     - ParticleChart instance
 *     activeTab         - string, currently selected right-panel tab
 *     frameCount        - number, global frame counter
 *     dom               - object with cached DOM references
 *     now               - DOMHighResTimeStamp (for animateAE)
 *     updatePlayButton  - function, syncs play/pause button state
 *     updateOnticPanel  - function, refreshes ontic panel
 *     updateHierarchyPanel - function, refreshes hierarchy panel
 *     resetAllVisualState  - function, master cross-scale visual reset
 *     setRunning        - function(bool), sets app-level running flag
 *     engineMode        - string, current engine mode ('atoms'/'molecules')
 *
 * EXPORTS:
 *   animateAE(ctx)                - per-frame update
 *   loadAEScenario(ctx, name)     - atom scenario setup
 *   resetScale2(ctx)              - clear AE-specific state for mode switch
 *   syncAEParams(ctx)             - sync AE physics params from UI sliders
 *   getAEVisualState()            - read visual toggle flags (for app.js reset)
 *   setAEVisualToggle(key, value) - set a visual toggle flag from outside
 *
 * ---------------------------------------------------------------
 * DELEGATION STUBS: after wiring into app.js, the app.js functions
 * become thin wrappers:
 *
 *   function animateAE(now) {
 *       return scale2.animateAE({ ...ctx, now });
 *   }
 *   function loadAEScenario(name) {
 *       return scale2.loadAEScenario(ctx, name);
 *   }
 * ---------------------------------------------------------------
 */

import { allElements, tablePosition, elementSymbol, getElement } from '../../elements.js?v=20260304q';
import {
    expandAEToOrbitalCloud, generateBondingCloud,
    electronConfig, slaterZeff, A0_DISPLAY, nuclearShellRadius
} from '../../orbitals.js?v=20260309c';
import {
    atomicEnergy, periodicTableTotalEnergy,
    formatEnergy as formatEnergyAE
} from '../../atomic-energy.js?v=20260304q';
import { formatEnergy, formatTemperature } from '../../units.js';
import { generateGridXZ, sampleAEField } from '../../fields.js?v=20260304q';
import { SCALE2_TOGGLES } from '../../config/toggles.js';


// =====================================================================
// AE-Specific Module State
// =====================================================================

// -- Enhanced atom/molecule visual toggle flags -----------------------
let _showNucleusShells   = true;    // strong force glow shells around nuclei
let _bondStyle           = 'cylinders'; // 'cylinders' | 'lines' | 'off'
let _showShellBounds     = false;   // translucent shell boundary spheres
let _showOrbitalLobes    = false;   // p/d/f orbital lobe shapes
let _showAEForceIonic    = false;   // Coulomb force arrows
let _showAEForceVdw      = false;   // van der Waals force arrows
let _showAEForceBond     = false;   // bond spring force arrows
let _showAEForceNet      = false;   // net force arrows
let _forceFrame          = 0;       // throttle: compute forces every 2nd frame

// -- Orbital cloud and field overlay ---------------------------------
let _showOrbitalClouds   = true;    // orbital electron clouds in AE mode
let _showAEField         = false;   // force field overlay (heatmap + vectors)
let _showBonds           = true;    // bond rendering (shared with Scale 3)

// -- Element legend cache (avoid DOM rebuilds) -----------------------
let _prevLegendKey       = '';

// -- Energy drift tracking -------------------------------------------
let _aeInitialEnergy     = null;    // captured at scenario load, before first tick

// -- Field computation cache -----------------------------------------
let _fieldGrid           = null;    // cached grid from generateGridXZ

// -- Tick accumulator (sub-1 speed fractional ticks) -----------------
let _tickAccumulator     = 0;

// -- AE toggle defaults (from config/toggles.js) ---------------------
const AE_DEFAULT_TOGGLES = SCALE2_TOGGLES;


// =====================================================================
// Internal Helpers
// =====================================================================

/**
 * Format large numbers with K/M suffixes for the status bar.
 * Duplicated here to avoid importing a private app.js helper.
 */
function formatNumber(n) {
    if (n >= 1000000) return (n / 1000000).toFixed(1) + 'M';
    if (n >= 1000)    return (n / 1000).toFixed(1) + 'K';
    return n.toString();
}

/** Format a number with SI suffix (T/G/M/K). */
function formatSI(n) {
    if (Math.abs(n) >= 1e12) return (n / 1e12).toFixed(2) + 'T';
    if (Math.abs(n) >= 1e9)  return (n / 1e9).toFixed(2) + 'G';
    if (Math.abs(n) >= 1e6)  return (n / 1e6).toFixed(2) + 'M';
    if (Math.abs(n) >= 1e3)  return (n / 1e3).toFixed(2) + 'K';
    return n.toFixed(2);
}

/**
 * Update atomic energy display cards (nuclear binding, B/A, electron
 * binding, FTD mass) for single-element or multi-element views.
 *
 * Originally app.js lines ~1492-1521.
 */
function updateAtomicEnergyDisplay(dom, atomData) {
    if (!dom.aeDiagMass || !atomData || atomData.count === 0) return;

    if (atomData.count === 1 && atomData.atomicNums) {
        // -- Single element --
        const Z = atomData.atomicNums[0];
        const e = atomicEnergy(Z);
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
            const e = atomicEnergy(Z);
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
        dom.aeDiagMassKb.textContent = formatSI(totalMass / 0.51099895);
    }
}

/**
 * Sync all AE toggle checkboxes to the bridge.
 * Called after initAE() and after _resetAETogglesToDefaults().
 */
function _syncAEParamsFromUIInternal(bridge) {
    const dtEl = document.getElementById('ae-dt-slider');
    if (dtEl) bridge.aeSetDt(parseFloat(dtEl.value));
    const softEl = document.getElementById('ae-soft-slider');
    if (softEl) bridge.aeSetSoftening(parseFloat(softEl.value));
    // Sync all AE toggles from checkboxes
    for (const [elId, , setter] of AE_DEFAULT_TOGGLES) {
        const el = document.getElementById(elId);
        if (el && bridge[setter]) bridge[setter](el.checked);
    }
}

/**
 * Reset all AE toggle checkboxes to their default values and push
 * defaults into the bridge.
 */
function _resetAETogglesToDefaults(bridge) {
    for (const [elId, defaultVal, setter] of AE_DEFAULT_TOGGLES) {
        const el = document.getElementById(elId);
        if (el) el.checked = defaultVal;
        if (bridge[setter]) bridge[setter](defaultVal);
    }
}

/**
 * Enable Phase 3 forces for specific scenarios and sync UI checkboxes.
 * flags: { hbonds, angle, dipole, thermostat, elec, temp }
 */
function _aeSetPhase3(bridge, flags) {
    const map = {
        hbonds:     ['ae-hbonds',             'aeSetHBonds'],
        angle:      ['ae-angle',              'aeSetAngleStrain'],
        dipole:     ['ae-dipole',             'aeSetDipoleDipole'],
        thermostat: ['ae-thermostat',         'aeSetThermostat'],
        elec:       ['ae-electronegativity',  'aeSetElectronegativity'],
    };
    for (const [key, [elId, setter]] of Object.entries(map)) {
        if (flags[key] !== undefined && bridge[setter]) {
            bridge[setter](flags[key]);
            const el = document.getElementById(elId);
            if (el) el.checked = flags[key];
        }
    }
    if (flags.temp !== undefined && bridge.aeSetThermostatTemp) {
        bridge.aeSetThermostatTemp(flags.temp);
    }
}


// =====================================================================
// Exported: resetScale2(ctx)
// =====================================================================
/**
 * Clear all Scale 2 module state for a clean mode switch.
 *
 * NOTE: Cross-scale visual resets (shared viewport overlays, DOM button
 * deactivation) remain in app.js _resetAllVisualState() because those
 * elements are shared across scales and managed by the central reset.
 */
export function resetScale2(ctx) {
    const { viewport } = ctx;

    // Reset visual flags to defaults
    _showNucleusShells = true;
    _bondStyle         = 'cylinders';
    _showShellBounds   = false;
    _showOrbitalLobes  = false;
    _showAEForceIonic  = false;
    _showAEForceVdw    = false;
    _showAEForceBond   = false;
    _showAEForceNet    = false;
    _forceFrame        = 0;
    _showOrbitalClouds = true;
    _showAEField       = false;
    _showBonds         = true;
    _prevLegendKey     = '';
    _aeInitialEnergy   = null;
    _fieldGrid         = null;
    _tickAccumulator   = 0;

    // Clear viewport AE-specific overlays if available
    if (viewport) {
        viewport.toggleNucleusShells(true);
        viewport.toggleBondCylinders(true);
        viewport.toggleOrbitalShells(false);
        viewport.toggleOrbitalLobes(false);
        viewport.toggleAEForceIonic(false);
        viewport.toggleAEForceVdw(false);
        viewport.toggleAEForceBond(false);
        viewport.toggleAEForceNet(false);
        viewport.toggleFieldHeatmap(false);
        viewport.toggleFieldVectors(false);
        viewport.toggleBondLines(false);
        viewport.updateElementLabels(null);
    }
}


// =====================================================================
// Exported: getAEVisualState() / setAEVisualToggle()
// =====================================================================
// Allow app.js to read/write visual toggle flags for cross-scale resets
// and UI event handlers without directly accessing module state.

export function getAEVisualState() {
    return {
        showNucleusShells: _showNucleusShells,
        bondStyle:         _bondStyle,
        showShellBounds:   _showShellBounds,
        showOrbitalLobes:  _showOrbitalLobes,
        showAEForceIonic:  _showAEForceIonic,
        showAEForceVdw:    _showAEForceVdw,
        showAEForceBond:   _showAEForceBond,
        showAEForceNet:    _showAEForceNet,
        showOrbitalClouds: _showOrbitalClouds,
        showAEField:       _showAEField,
        showBonds:         _showBonds,
    };
}

export function setAEVisualToggle(key, value) {
    switch (key) {
        case 'showNucleusShells': _showNucleusShells = value; break;
        case 'bondStyle':         _bondStyle         = value; break;
        case 'showShellBounds':   _showShellBounds   = value; break;
        case 'showOrbitalLobes':  _showOrbitalLobes  = value; break;
        case 'showAEForceIonic':  _showAEForceIonic  = value; break;
        case 'showAEForceVdw':    _showAEForceVdw    = value; break;
        case 'showAEForceBond':   _showAEForceBond   = value; break;
        case 'showAEForceNet':    _showAEForceNet    = value; break;
        case 'showOrbitalClouds': _showOrbitalClouds = value; break;
        case 'showAEField':       _showAEField       = value; break;
        case 'showBonds':         _showBonds         = value; break;
        default:
            console.warn(`[Scale2] Unknown visual toggle: ${key}`);
    }
}


// =====================================================================
// Exported: syncAEParams(ctx)
// =====================================================================
/**
 * Sync AE physics parameters from UI sliders/checkboxes into the bridge.
 * Called by app.js after scenario load and when switching to Scale 2/3.
 */
export function syncAEParams(ctx) {
    _syncAEParamsFromUIInternal(ctx.bridge);
}


// =====================================================================
// Exported: animateAE(ctx)
// =====================================================================
// Per-frame update for Scale 2 (Atoms) and Scale 3 (Molecules).
// Both scales share the same AtomEngine and render loop; Scale 3 just
// loads molecule scenarios instead of individual atom scenarios.
//
// Originally app.js lines ~1209-1485.
//
// Responsibilities:
//   1. Tick the AE simulation (accumulator handles sub-1 speeds)
//   2. Validate atom positions (auto-pause on NaN or flyaway)
//   3. Expand atom positions into orbital electron clouds
//   4. Merge bonding electron clouds into the point cloud
//   5. Update bond rendering (cylinders or lines)
//   6. Update nucleus shells, orbital shells, orbital lobes
//   7. Compute per-atom force arrows (throttled every 2nd frame)
//   8. Render element symbol labels and color legend
//   9. Update AE force field overlay (heatmap + vectors)
//  10. Render the viewport
//  11. Update diagnostics, charts, and panels (throttled to every 3rd frame)

export function animateAE(ctx) {
    const {
        bridge, viewport, running, ticksPerFrame, inspector,
        fluxEnergyChart, particleChart,
        activeTab, frameCount, dom, now,
        updatePlayButton, updateOnticPanel, updateHierarchyPanel,
        setRunning, engineMode
    } = ctx;

    // ── 1. Tick AE simulation if running ───────────────────────────
    if (running) {
        _tickAccumulator += ticksPerFrame;
        const wholeTicks = Math.floor(_tickAccumulator);
        _tickAccumulator -= wholeTicks;
        for (let i = 0; i < wholeTicks; i++) {
            try {
                bridge.aeTick();
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
        const t = now * 0.001; // seconds for breathing animation
        cloudData = expandAEToOrbitalCloud(atomData, t);

        // ── 4. Merge bonding electron clouds into particle data ────
        if (_bondStyle !== 'off' && atomData.bondCount > 0) {
            const bondCloud = generateBondingCloud(atomData);
            if (bondCloud.count > 0) {
                const mergedCount = cloudData.count + bondCloud.count;
                const mp = new Float32Array(mergedCount * 3);
                const mc = new Float32Array(mergedCount * 3);
                const ms = new Float32Array(mergedCount);
                mp.set(cloudData.positions.subarray(0, cloudData.count * 3));
                mc.set(cloudData.colors.subarray(0, cloudData.count * 3));
                ms.set(cloudData.sizes.subarray(0, cloudData.count));
                mp.set(bondCloud.positions.subarray(0, bondCloud.count * 3), cloudData.count * 3);
                mc.set(bondCloud.colors.subarray(0, bondCloud.count * 3), cloudData.count * 3);
                ms.set(bondCloud.sizes.subarray(0, bondCloud.count), cloudData.count);
                cloudData = { positions: mp, colors: mc, sizes: ms, count: mergedCount };
            }
        }

        viewport.updateParticles(cloudData);
    } else {
        viewport.updateParticles(atomData);
    }

    // Pass AE context to inspector for click-to-inspect
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

    // ── 6. Update nucleus shells (strong force glow) ───────────────
    if (_showNucleusShells && atomData.count > 0) {
        viewport.updateNucleusShells(atomData);
    }

    // Update orbital shell boundaries
    if (_showShellBounds && atomData.count > 0) {
        viewport.updateOrbitalShells(atomData, electronConfig, slaterZeff, A0_DISPLAY);
        viewport.toggleOrbitalShells(true);
    }

    // Update orbital lobes
    if (_showOrbitalLobes && atomData.count > 0) {
        viewport.updateOrbitalLobes(atomData, electronConfig, slaterZeff, A0_DISPLAY);
        viewport.toggleOrbitalLobes(true);
    }

    // ── 7. Update per-atom force arrows (every 2nd frame) ──────────
    const anyForce = _showAEForceIonic || _showAEForceVdw || _showAEForceBond || _showAEForceNet;
    if (anyForce && atomData.count > 0) {
        _forceFrame++;
        if (_forceFrame % 2 === 0) {
            const forceData = bridge.aeGetForceDecomposition();
            viewport.updateAEForces(atomData.positions, forceData, forceData.count);
        }
    }

    // ── 8. Update element labels ───────────────────────────────────
    if (atomData.count > 0 && atomData.atomicNums) {
        const labels = [];
        for (let i = 0; i < atomData.count; i++) {
            const Z = atomData.atomicNums[i];
            const sym = elementSymbol(Z);
            // Convert CPK color to CSS hex for canvas rendering
            const r = Math.round(atomData.colors[i * 3] * 255);
            const g = Math.round(atomData.colors[i * 3 + 1] * 255);
            const b = Math.round(atomData.colors[i * 3 + 2] * 255);
            // Use white text unless atom is very light-colored
            const lum = 0.299 * r + 0.587 * g + 0.114 * b;
            const hexColor = lum > 200 ? '#aaaaaa' : '#ffffff';
            labels.push({
                x: atomData.positions[i * 3],
                y: atomData.positions[i * 3 + 1],
                z: atomData.positions[i * 3 + 2],
                symbol: sym,
                color: hexColor,
            });
        }
        viewport.updateElementLabels(labels);
    } else {
        viewport.updateElementLabels(null);
    }

    // Update element legend (only rebuild when set of elements changes)
    if (dom.aeLegend && atomData.count > 0 && atomData.atomicNums) {
        const zSet = new Set();
        for (let i = 0; i < atomData.count; i++) zSet.add(atomData.atomicNums[i]);
        const key = [...zSet].sort((a, b) => a - b).join(',') + (_showOrbitalClouds ? '+c' : '');
        if (key !== _prevLegendKey) {
            _prevLegendKey = key;
            let html = '<div class="ae-legend-header">Elements</div>';
            for (const Z of [...zSet].sort((a, b) => a - b)) {
                const el = getElement(Z);
                const [r, g, b] = el.color;
                const hex = `#${(r * 255 | 0).toString(16).padStart(2, '0')}${(g * 255 | 0).toString(16).padStart(2, '0')}${(b * 255 | 0).toString(16).padStart(2, '0')}`;
                html += `<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:${hex}"></span><span class="ae-legend-sym">${el.symbol}</span><span class="ae-legend-name">${el.name}</span></div>`;
            }
            if (_showOrbitalClouds) {
                html += '<div class="ae-legend-sep"></div><div class="ae-legend-header">Substructure</div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#ff4d33"></span><span class="ae-legend-name">Protons</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#4d80e6"></span><span class="ae-legend-name">Neutrons</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#66bfff"></span><span class="ae-legend-name">s orbitals</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#4de673"></span><span class="ae-legend-name">p orbitals</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#ffb333"></span><span class="ae-legend-name">d orbitals</span></div>';
                html += '<div class="ae-legend-item"><span class="ae-legend-swatch" style="background:#d94db3"></span><span class="ae-legend-name">f orbitals</span></div>';
            }
            dom.aeLegend.innerHTML = html;
        }
    } else if (dom.aeLegend) {
        if (_prevLegendKey !== '') { dom.aeLegend.innerHTML = ''; _prevLegendKey = ''; }
    }

    // ── 9. Update force field overlay (heatmap + vectors) ──────────
    if (_showAEField && atomData.count > 0) {
        // Auto-compute grid extent from atom bounding box
        let maxR = 5;
        for (let i = 0; i < atomData.count; i++) {
            const ax = Math.abs(atomData.positions[i * 3]);
            const az = Math.abs(atomData.positions[i * 3 + 2]);
            if (ax > maxR) maxR = ax;
            if (az > maxR) maxR = az;
        }
        const extent = maxR + 5; // padding around atoms
        if (!_fieldGrid || Math.abs(_fieldGrid.extent - extent) > 1) {
            _fieldGrid = generateGridXZ(extent, 20);
        }
        const src = bridge.aeGetFieldSources();
        const field = sampleAEField(src, _fieldGrid.positions, _fieldGrid.count);
        viewport.updateFieldHeatmap(_fieldGrid.positions, field.potentials, _fieldGrid.count, field.maxPotential);
        viewport.updateFieldVectors(_fieldGrid.positions, field.forces, _fieldGrid.count, field.maxForce, 3.0);
    }

    // ── 10. Render viewport ────────────────────────────────────────
    viewport.render();

    // ── 11. AE diagnostics (throttled to every 3rd frame) ──────────
    if (frameCount % 3 === 0) {
        const diag = bridge.aeGetDiagnostics();

        // Update status bar
        dom.statusTick.textContent = formatNumber(diag.tick);
        dom.statusPtime.textContent = formatNumber(diag.tick);
        dom.statusParticles.textContent = diag.atomCount;
        dom.statusEnergy.textContent = formatEnergy(diag.totalEnergy, 2).text;

        // Update status dot
        if (running) {
            dom.statusDot.classList.remove('idle');
            dom.statusState.textContent = 'Running';
        } else {
            dom.statusDot.classList.add('idle');
            dom.statusState.textContent = 'Idle';
        }

        // Update AE diagnostic cards
        dom.aeDiagCount.textContent = diag.atomCount;
        dom.aeDiagBonds.textContent = diag.bondCount;
        dom.aeDiagKe.textContent = formatEnergy(diag.totalKE, 2).text;
        dom.aeDiagEtotal.textContent = formatEnergy(diag.totalEnergy, 2).text;
        dom.aeDiagPeIonic.textContent = formatEnergy(diag.totalPEIonic, 2).text;
        dom.aeDiagPeVdw.textContent = formatEnergy(diag.totalPEVdw, 2).text;
        dom.aeDiagPeBond.textContent = formatEnergy(diag.totalPEBond, 2).text;
        dom.aeDiagTemp.textContent = formatTemperature(diag.temperature, 2).text;
        const pMag = Math.sqrt(diag.momentumX ** 2 + diag.momentumY ** 2 + diag.momentumZ ** 2);
        dom.aeDiagMomentum.textContent = pMag.toFixed(6) + ' AMU\u00b7\u00c5/step';
        dom.aeDiagTick.textContent = formatNumber(diag.tick);

        // Energy drift tracking (reference captured at load time; fallback here)
        if (_aeInitialEnergy === null && diag.totalEnergy !== 0) {
            _aeInitialEnergy = diag.totalEnergy;
        }
        if (_aeInitialEnergy !== null) {
            const drift = ((diag.totalEnergy - _aeInitialEnergy) / Math.abs(_aeInitialEnergy)) * 100;
            dom.aeDiagDrift.textContent = drift.toFixed(4) + '%';
        }

        // Atomic energy physics display
        updateAtomicEnergyDisplay(dom, atomData);

        // Feed charts with adapted data
        const diagAdapted = {
            tick: diag.tick,
            manifested: diag.atomCount,
            positive: 0, negative: 0,
            totalFlux: 0, totalEnergy: diag.totalEnergy,
            fieldEnergy: diag.totalPEIonic + diag.totalPEVdw + diag.totalPEBond,
            kineticEnergy: diag.totalKE,
            peFlux: diag.totalPEIonic,
        };
        fluxEnergyChart.push(diagAdapted);
        particleChart.push(diagAdapted);

        // Update active panel visuals
        switch (activeTab) {
            case 'charts':
                fluxEnergyChart.draw();
                particleChart.draw();
                break;
            case 'inspector':
                inspector.update();
                break;
            case 'ontic':
                if (updateOnticPanel) updateOnticPanel();
                break;
            case 'hierarchy':
                if (updateHierarchyPanel) updateHierarchyPanel();
                break;
        }
    }
}


// =====================================================================
// Exported: loadAEScenario(ctx, name)
// =====================================================================
// Set up an atom scenario in the AE engine.
// Originally app.js lines ~3426-3827.
//
// Handles:
//   - Full 118-element periodic table layout
//   - Noble gas clusters (vdW only)
//   - Ionic formation (Coulomb-driven)
//   - Covalent bond formation (auto-bonding)
//   - Hydrogen bonding (Phase 3)
//   - VSEPR geometry relaxation
//   - Thermal dynamics (thermostat + gas kinetics)
//   - Metallic clusters (multi-atom bonding)
//   - Individual element scenarios (ae-el-1 through ae-el-118)
//
// NOTE: ctx.resetAllVisualState() is called first to clear cross-scale
// visual state. That function lives in app.js because it touches DOM
// elements and viewport toggles shared across all scales.

export function loadAEScenario(ctx, name) {
    const { bridge, viewport, inspector } = ctx;

    if (!bridge.initAE) return;
    ctx.resetAllVisualState();
    bridge.initAE();

    // Reset all AE toggles to defaults, then sync sliders from UI
    _resetAETogglesToDefaults(bridge);
    _syncAEParamsFromUIInternal(bridge);
    // Scale 2 override: no auto-bonding for individual atoms
    if (bridge.aeSetBonding) bridge.aeSetBonding(false);
    const bondEl = document.getElementById('ae-bonding');
    if (bondEl) bondEl.checked = false;

    // Clear molecule info (molecules are Scale 3 only)
    if (inspector) inspector.setCurrentMolecule(null);

    // ── Procedural scenarios (periodic table, elements, custom) ────
    const S = 5;   // typical spacing (in Bohr radii)

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
            _aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'O\u2082 Formation',
                desc: 'Two oxygen atoms approach and bond \u2014 double bond forms.',
                fields: { 'Atoms': '2 \u00d7 O', 'Force': 'vdW + auto-bond + angle strain' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 25); viewport.controls.update(); }
            break;
        }
        case 'ae-ch4-form': {
            bridge.aeSetBonding(true); document.getElementById('ae-bonding').checked = true;
            _aeSetPhase3(bridge, { angle: true });
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
            // Molecule 1 (left)
            bridge.aeAddAtom(8, -7, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -7 + rOH, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -7 + rOH*Math.cos(ang), rOH*Math.sin(ang), 0, 0, 0, 0, 0);
            // Molecule 2 (right, rotated so O faces mol1's H)
            bridge.aeAddAtom(8, 7, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 7 - rOH, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 7 - rOH*Math.cos(ang), -rOH*Math.sin(ang), 0, 0, 0, 0, 0);
            // Pre-bond to establish O-H covalent bonds
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3(bridge, { hbonds: true, angle: true });
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
            _aeSetPhase3(bridge, { hbonds: true, angle: true });
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
            // CO2: start bent at 90 deg, should relax to 180 deg (linear)
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 2.0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(8, 0, 2.0, 0, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'CO\u2082 VSEPR',
                desc: 'CO\u2082 starts bent (90\u00b0) \u2014 angle strain drives it to linear (180\u00b0).',
                fields: { 'Atoms': 'C + 2O', 'Start': '90\u00b0', 'Target': '180\u00b0 (linear)', 'Steric #': '2' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }
        case 'ae-vsepr-tetrahedral': {
            // CH4: start at 90 deg (cubic), should relax to 109.47 deg
            const d = 3.5;
            bridge.aeAddAtom(6, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, d, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, -d, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 0, d, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, 0, 0, d, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3(bridge, { angle: true });
            if (inspector) inspector.setScenarioInfo({ title: 'CH\u2084 VSEPR',
                desc: 'CH\u2084 starts at 90\u00b0 \u2014 angle strain relaxes to 109.47\u00b0 tetrahedral.',
                fields: { 'Atoms': 'C + 4H', 'Start': '90\u00b0', 'Target': '109.47\u00b0', 'Steric #': '4' }});
            if (viewport) { viewport.controls.target.set(0, 0, 0); viewport.camera.position.set(0, 0, 20); viewport.controls.update(); }
            break;
        }
        case 'ae-vsepr-bent': {
            // H2O: start at 150 deg (too wide), should relax to 104.5 deg
            const r = 3.4;
            const theta0 = 150 * Math.PI / 180;
            bridge.aeAddAtom(8, 0, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, r, 0, 0, 0, 0, 0, 0);
            bridge.aeAddAtom(1, r*Math.cos(theta0), r*Math.sin(theta0), 0, 0, 0, 0, 0);
            bridge.aeSetBonding(true); bridge.aePreBond();
            bridge.aeSetBonding(false); document.getElementById('ae-bonding').checked = false;
            _aeSetPhase3(bridge, { angle: true });
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
            _aeSetPhase3(bridge, { thermostat: true, temp: 1.0 });
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
                    const mass = (Z + N * 1.001).toFixed(2);
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
                // Camera distance scaled to atom size
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

    // Capture initial energy reference for drift tracking (before first tick)
    const initDiag = bridge.aeGetDiagnostics();
    if (initDiag.totalEnergy !== 0) _aeInitialEnergy = initDiag.totalEnergy;
}
