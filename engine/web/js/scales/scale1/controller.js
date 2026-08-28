/**
 * Scale 1 (Particles) Controller — native-engine edition.
 *
 * Scale 1 is a continuous particle system promoted from the discrete
 * lattice: the native C++/WASM ParticleEngine integrates (via the
 * bridge's pe* adapter surface), and particles arrive either from the
 * "⤴ Scale up" promotion pipeline (./promotion.js — one particle per
 * lattice cluster, mass = N·K_B) or from the declarative scenario
 * registry (./scenario-registry.js). The [PARAMETRIC] Zoo can inject
 * catalog particles on top.
 *
 * State lives in ./state/store.js (scale1State) — no module-level lets.
 * Rendering goes through the shared viewport facade; the promotion
 * source voxels render on a separate ghost layer (never multiplexed
 * onto the main particle mesh).
 *
 * app.js contract (duck-typed; do not rename):
 *   mount(ctx) / destroy(ctx)      — mode switch lifecycle
 *   animatePE(ctx)                 — per-frame update (calls viewport.render)
 *   loadPEScenario(ctx, name)      — scenario setup
 *   resetScale1(ctx)               — clear Scale-1 state for cache resets
 *   bindScale1ControlsUI()         — one-time controls panel init
 *   setPE… / setVelocities / setTrails — overlay toggle setters (app.js bindings)
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { formatEnergy } from '../../units.js';
import {
    generateGridXZ, samplePECoulombOnly,
    samplePEGravityField, makePECoulombFieldFn
} from '../../fields.js';
import {
    computeStreamlines, generateEFieldSeeds
} from '../../fieldlines.js';
import { GRAVITY_VIS_GAIN } from '../../constants.js';
import { formatSI } from '../scale-utils.js';
import { Scale1ControlsComponent } from './ui/controls/component.js';
import { refreshPromotionCard } from './ui/controls/pe-controls.js';
import {
    expandPEToCloud, buildPEManifestBlinkRate, updateTrailHistory,
    getCloudParticleMap, getTrailHistory, clearCloudAndTrails, MANIFEST_FILL
} from './pe-cloud-expander.js';
import {
    getScale1Scenario, getScale1ScenarioPreset, DEFAULT_SCALE1_SCENARIO,
} from './scenario-registry.js';
import { scale1State, resetScale1State } from './state/store.js';
import { telemetryHub } from '../../telemetry-hub.js';
import { estimateOrbitPeriod } from './telemetry/orbit-period.js';


// =====================================================================
// Exported: overlay toggle setters + cloud/trail accessors
// =====================================================================

export function setPEEField(on)       { scale1State.overlays.efield = on; }
export function setPEPotential(on)    { scale1State.overlays.potential = on; }
export function setPEGravField(on)    { scale1State.overlays.gravityField = on; }
export function setPEForceCoulomb(on) { scale1State.overlays.forceCoulomb = on; }
export function setPEForceGravity(on) { scale1State.overlays.forceGravity = on; }
export function setPEForceStrong(on)  { scale1State.overlays.forceStrong = on; }
export function setPEForceNet(on)     { scale1State.overlays.forceNet = on; }
export function setPESystem(on)       { scale1State.overlays.system = on; }
export function setVelocities(on)     { scale1State.overlays.velocities = on; }
export function setTrails(on)         { scale1State.overlays.trails = on; }
export function setAdmissibilityRing(on) { scale1State.overlays.admissibilityRing = on; }
export function setProvenanceLabel(on) { scale1State.overlays.provenanceLabel = on; }
export function setMassComparison(on) { scale1State.overlays.massComparison = on; }

/** Promotion-source ghost layer toggle (controls panel). */
export function setVoxelDebug(on, viewport) {
    scale1State.overlays.voxelDebug = !!on;
    if (!viewport) return;
    if (on && scale1State.lastPromotion?.voxelDebug) {
        viewport.updateVoxelDebugLayer(
            scale1State.lastPromotion.voxelDebug,
            scale1State.lastPromotion.latticeSize,
            scale1State.lastPromotion.displayScale);
    }
    viewport.toggleVoxelDebugLayer(!!on && !!scale1State.lastPromotion?.voxelDebug);
}

// Re-export cloud/trail accessors so external consumers have a single
// import surface (controller.js) and need not know about the split file.
export { getCloudParticleMap, getTrailHistory };

function setButtonActive(id, on) {
    const btn = document.getElementById(id);
    if (!btn) return;
    btn.classList.toggle('active', !!on);
    btn.setAttribute('aria-pressed', on ? 'true' : 'false');
}

function setCheckbox(id, on) {
    const el = document.getElementById(id);
    if (el) el.checked = !!on;
}

function setSliderValue(id, value, digits) {
    const slider = document.getElementById(id);
    if (!slider || value === undefined || value === null) return;
    slider.value = String(value);
    const valueEl = document.getElementById(id.replace('-slider', '-value'));
    if (valueEl) valueEl.textContent = Number(value).toFixed(digits);
}

function applyPEPhysicsPreset(bridge, preset) {
    const p = preset.physics || {};
    bridge.peSetCoulomb?.(!!p.coulomb);
    bridge.peSetGravity?.(!!p.gravity);
    bridge.peSetDamping?.(!!p.damping);
    bridge.peSetLorentz?.(!!p.lorentz);
    bridge.peSetExchange?.(!!p.exchange);
    bridge.peSetStrong?.(!!p.strong);
    bridge.peSetMagneticDipole?.(!!p.magnetic_dipole);
    bridge.peSetSpinOrbit?.(!!p.spin_orbit);
    bridge.peSetRadiation?.(!!p.radiation);
    bridge.peSetRelativistic?.(!!p.relativistic);
    bridge.peSetRelativisticVerlet?.(!!p.relativistic_verlet);

    if (p.dt !== undefined) {
        bridge.peSetDt?.(p.dt);
        setSliderValue('pe-dt-slider', p.dt, 1);
    }
    if (p.softening !== undefined) {
        bridge.peSetSoftening?.(p.softening);
        setSliderValue('pe-soft-slider', p.softening, 2);
    }

    setCheckbox('pe-coulomb', p.coulomb);
    setCheckbox('pe-gravity', p.gravity);
    setCheckbox('pe-damping', p.damping);
    setCheckbox('pe-lorentz-p', p.lorentz);
    setCheckbox('pe-exchange', p.exchange);
    setCheckbox('pe-strong', p.strong);
    setCheckbox('pe-magnetic-dipole', p.magnetic_dipole);
    setCheckbox('pe-spin-orbit', p.spin_orbit);
    setCheckbox('pe-radiation', p.radiation);
    setCheckbox('pe-relativistic', p.relativistic);
    setCheckbox('pe-relativistic-verlet', p.relativistic_verlet);
}

function applyPEOverlayPreset(viewport, preset) {
    const o = preset.overlays || {};
    const ov = scale1State.overlays;
    ov.velocities = !!o.velocities;
    ov.trails = !!o.trails;
    ov.efield = !!o.efield;
    ov.potential = !!o.potential;
    ov.gravityField = !!o.gravityField;
    ov.forceCoulomb = !!(o.forceCoulomb ?? o.forces);
    ov.forceGravity = !!o.forceGravity;
    ov.forceStrong = !!o.forceStrong;
    ov.forceNet = !!(o.forceNet ?? o.forces);
    ov.system = !!o.system;
    ov.voxelDebug = !!o.voxelDebug;
    ov.admissibilityRing = !!o.admissibilityRing;
    ov.provenanceLabel = !!o.provenanceLabel;
    ov.massComparison = !!o.massComparison;

    setButtonActive('toggle-velocities', ov.velocities);
    setButtonActive('toggle-trails', ov.trails);
    setButtonActive('toggle-pe-efield', ov.efield);
    setButtonActive('toggle-pe-potential', ov.potential);
    setButtonActive('toggle-pe-gravity-field', ov.gravityField);
    setButtonActive('toggle-pe-force-coulomb', ov.forceCoulomb);
    setButtonActive('toggle-pe-force-gravity', ov.forceGravity);
    setButtonActive('toggle-pe-force-strong', ov.forceStrong);
    setButtonActive('toggle-pe-force-net', ov.forceNet);
    setButtonActive('toggle-pe-system', ov.system);
    setCheckbox('pe-voxel-debug', ov.voxelDebug);
    setButtonActive('toggle-pe-admissibility', ov.admissibilityRing);
    setButtonActive('toggle-pe-provenance', ov.provenanceLabel);
    setButtonActive('toggle-pe-mass-comparison', ov.massComparison);

    if (!viewport) return;
    viewport.toggleVelocityVectors(ov.velocities);
    viewport.toggleTrails(ov.trails);
    viewport.togglePEStreamlines(ov.efield);
    viewport.toggleFieldHeatmap(ov.potential);
    viewport.toggleFieldVectors(ov.potential);
    viewport.toggleGravityVectors(ov.gravityField);
    viewport.togglePEForceCoulomb(ov.forceCoulomb);
    viewport.togglePEForceGravity(ov.forceGravity);
    viewport.togglePEForceStrong(ov.forceStrong);
    viewport.togglePEForceNet(ov.forceNet);
    viewport.togglePESystem(ov.system);
    setVoxelDebug(ov.voxelDebug, viewport);
    viewport.toggleAdmissibilityRings(ov.admissibilityRing);
    viewport.toggleProvenanceLabels(ov.provenanceLabel);
    viewport.toggleMassComparison(ov.massComparison);
}


// =====================================================================
// Lifecycle
// =====================================================================

class Scale1LifecycleController extends BaseLifecycleController {
    destroy(ctx) {
        super.destroy(ctx);
        _resetScale1Internal(ctx);
    }
}

const _lifecycleController = new Scale1LifecycleController();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function resetScale1(ctx) {
    _lifecycleController.destroy(ctx);
}

function _resetScale1Internal(ctx) {
    const { viewport } = ctx;

    clearCloudAndTrails();
    resetScale1State();

    if (viewport) {
        viewport.togglePEStreamlines(false);
        viewport.toggleFieldHeatmap(false);
        viewport.toggleFieldVectors(false);
        viewport.toggleGravityVectors(false);
        viewport.toggleParticleForces(false);
        viewport.togglePEForceCoulomb?.(false);
        viewport.togglePEForceGravity?.(false);
        viewport.togglePEForceStrong?.(false);
        viewport.togglePEForceNet?.(false);
        viewport.togglePESystem(false);
        viewport.toggleVelocityVectors(false);
        viewport.toggleTrails(false);
        viewport.toggleSpinVectors?.(false);
        viewport.toggleVoxelDebugLayer?.(false);
        viewport.toggleAdmissibilityRings?.(false);
        viewport.toggleProvenanceLabels?.(false);
        viewport.toggleMassComparison?.(false);
        if (viewport.setPEManifestation) viewport.setPEManifestation(false, 0);
    }
}


// =====================================================================
// System observables helper
// =====================================================================

/**
 * Mass-weighted center of mass, total momentum p = Σ mᵢvᵢ, and angular
 * momentum about the CoM: L = Σ mᵢ (rᵢ−r_cm) × (vᵢ−v_cm). NOTE: the
 * diagnostics panel's L is about the ORIGIN (native engine convention);
 * this overlay's L is about the CoM and is labeled "L (CoM)".
 */
function computeSystemVectors(peData) {
    const { positions, velocities, masses, count } = peData;
    let M = 0, cx = 0, cy = 0, cz = 0, px = 0, py = 0, pz = 0;
    for (let i = 0; i < count; i++) {
        const m = masses[i];
        M += m;
        cx += m * positions[i * 3];
        cy += m * positions[i * 3 + 1];
        cz += m * positions[i * 3 + 2];
        px += m * velocities[i * 3];
        py += m * velocities[i * 3 + 1];
        pz += m * velocities[i * 3 + 2];
    }
    if (M <= 0) return { com: [0, 0, 0], p: [0, 0, 0], l: [0, 0, 0] };
    cx /= M; cy /= M; cz /= M;
    const vcx = px / M, vcy = py / M, vcz = pz / M;
    let lx = 0, ly = 0, lz = 0;
    for (let i = 0; i < count; i++) {
        const m = masses[i];
        const rx = positions[i * 3]     - cx;
        const ry = positions[i * 3 + 1] - cy;
        const rz = positions[i * 3 + 2] - cz;
        const wx = velocities[i * 3]     - vcx;
        const wy = velocities[i * 3 + 1] - vcy;
        const wz = velocities[i * 3 + 2] - vcz;
        lx += m * (ry * wz - rz * wy);
        ly += m * (rz * wx - rx * wz);
        lz += m * (rx * wy - ry * wx);
    }
    return { com: [cx, cy, cz], p: [px, py, pz], l: [lx, ly, lz] };
}


// =====================================================================
// Exported: animatePE(ctx)
// =====================================================================

export function animatePE(ctx) {
    const {
        bridge, viewport, running, ticksPerFrame, inspector,
        activeTab, frameCount, dom, now, isPanelVisible,
    } = ctx;
    const ov = scale1State.overlays;

    // ── 1. Tick the native engine while running ──────────────────────
    if (running) {
        const wholeTicks = scale1State.tickAcc.accumulate(ticksPerFrame);
        for (let i = 0; i < wholeTicks; i++) {
            bridge.peTick();
        }
    }

    // ── 2. Cloud expansion + manifestation blink ─────────────────────
    const peData  = bridge.peGetParticleData();
    const typeMap = bridge.peGetParticleTypes();
    const forceData = bridge.peGetForces?.() ?? null;
    const frameSec = typeof now === 'number' ? now * 0.001 : performance.now() * 0.001;
    const blinkRate = buildPEManifestBlinkRate(peData, forceData, frameSec);
    const cloud = expandPEToCloud(peData, typeMap, { blinkRate, frameSec });
    viewport.updateParticles(cloud);

    if (viewport.setPEManifestation) {
        viewport.setPEManifestation(true, frameSec, MANIFEST_FILL);
    }

    if (peData.spinAxes && peData.spins && peData.count > 0) {
        viewport.updateSpinVectors?.(
            peData.positions, peData.spinAxes, peData.spins, peData.count);
        viewport.toggleSpinVectors?.(true);
    } else {
        viewport.toggleSpinVectors?.(false);
    }

    if (inspector) {
        inspector.setPEContext(getCloudParticleMap(), cloud.count, typeMap);
    }

    // ── 3. Overlays ──────────────────────────────────────────────────
    if (ov.velocities && peData.count > 0) {
        viewport.updateVelocityVectors(peData.positions, peData.velocities, peData.count);
    }
    telemetryHub.s1._overlayVelocitiesOn = ov.velocities;

    if (running && peData.count > 0) {
        updateTrailHistory(peData);
    }
    if (ov.trails) {
        viewport.updateTrails(getTrailHistory(), typeMap);
    }
    telemetryHub.s1._overlayTrailsOn = ov.trails;
    // Orbit-period estimate (2-body proxy): built from the hub's own
    // tick-gated peSeparation channel (collectScale1Extended), NOT from the
    // visual trail cache (pe-cloud-expander.js getTrailHistory()) — that
    // cache stores per-particle ring buffers of raw positions with no tick
    // stamps, sampled once per rendered frame rather than once per engine
    // tick, so it cannot supply the {tick, separation} series
    // estimateOrbitPeriod expects. telemetryHub.s1.diag.tick / peSeparation
    // are real engine-tick-aligned values already computed elsewhere in the
    // hub; only new-tick samples are appended here (deduped against the
    // last recorded tick) so the history stays real and doesn't repeat a
    // stale sample across the ~3 unthrottled frames between hub collections.
    if (ov.trails && peData.count === 2) {
        // A pair identity key (not just count===2) guards against annihilation
        // + re-injection silently swapping in a different pair while count
        // holds at 2 — without this, a stale sample from the old pairing
        // would anchor estimateOrbitPeriod's "start" reference and produce a
        // numerically plausible but physically meaningless period.
        const id0 = peData.ids[0], id1 = peData.ids[1];
        const pairKey = id0 < id1 ? `${id0}:${id1}` : `${id1}:${id0}`;
        if (telemetryHub._s1SepPairKey !== pairKey) {
            telemetryHub._s1SepHistory.length = 0;
            telemetryHub._s1SepPairKey = pairKey;
        }
        const tick = telemetryHub.s1.diag?.tick ?? null;
        const sep = telemetryHub.peSeparation.last();
        const hist = telemetryHub._s1SepHistory;
        if (tick !== null && sep > 0 && (hist.length === 0 || hist[hist.length - 1].tick !== tick)) {
            hist.push({ tick, separation: sep });
            if (hist.length > 200) hist.shift();
        }
        telemetryHub.s1._orbitPeriod = estimateOrbitPeriod(hist);
    } else {
        telemetryHub.s1._orbitPeriod = null;
        if (peData.count !== 2) {
            telemetryHub._s1SepHistory.length = 0;
            telemetryHub._s1SepPairKey = null;
        }
    }

    if (ov.potential && peData.count > 0) {
        if (!scale1State.fieldGrid) scale1State.fieldGrid = generateGridXZ(25, 20);
        const grid = scale1State.fieldGrid;
        const src   = bridge.peGetFieldSources();
        const field = samplePECoulombOnly(src, grid.positions, grid.count);
        viewport.updateFieldHeatmap(
            grid.positions, field.potentials, grid.count, field.maxPotential);
        viewport.updateFieldVectors(
            grid.positions, field.forces, grid.count, field.maxForce, 8.0);
        // samplePECoulombOnly returns maxPotential (peak |V|) but no min —
        // scan the grid once for the true signed min/max for the telemetry
        // legend (Task 6's tooltip points here).
        let potMin = Infinity, potMax = -Infinity;
        for (let i = 0; i < grid.count; i++) {
            const v = field.potentials[i];
            if (v < potMin) potMin = v;
            if (v > potMax) potMax = v;
        }
        telemetryHub.s1._potentialMin = grid.count > 0 ? potMin : 0;
        telemetryHub.s1._potentialMax = grid.count > 0 ? potMax : 0;
    }
    telemetryHub.s1._overlayPotentialOn = ov.potential;

    // Coulomb E-field streamlines (3D, throttled)
    const refreshStreamlines = running ? frameCount % 5 === 0 : (frameCount % 30 === 0);
    if (ov.efield && peData.count > 0 && refreshStreamlines) {
        const src     = bridge.peGetFieldSources();
        const fieldFn = makePECoulombFieldFn(src, 0.5);
        const buf = scale1State.srcParticlesBuf;
        while (buf.length < src.count) buf.push({ x: 0, y: 0, z: 0 });
        buf.length = src.count;
        for (let i = 0; i < src.count; i++) {
            buf[i].x = src.positions[i * 3];
            buf[i].y = src.positions[i * 3 + 1];
            buf[i].z = src.positions[i * 3 + 2];
        }
        const seeds = generateEFieldSeeds(buf, 3, 100);
        const lines = computeStreamlines({ fieldFn }, seeds, {
            maxSteps: 80, stepSize: 0.5, bounds: 30
        });
        viewport.updatePEStreamlines(lines);
    }
    telemetryHub.s1._overlayEfieldOn = ov.efield;

    if (ov.gravityField && peData.count > 0) {
        if (!scale1State.fieldGrid) scale1State.fieldGrid = generateGridXZ(25, 20);
        const grid = scale1State.fieldGrid;
        const src   = bridge.peGetFieldSources();
        const field = samplePEGravityField(src, grid.positions, grid.count);
        viewport.updateGravityVectors(
            grid.positions, field.forces, grid.count, field.maxForce);
    }
    telemetryHub.s1._overlayGravityFieldOn = ov.gravityField;

    // Per-particle decomposed force arrows (native Float64 decomposition —
    // `net` is the TRUE integrator force incl. every enabled term)
    const anyPEForce = ov.forceCoulomb || ov.forceGravity || ov.forceStrong || ov.forceNet;
    if (anyPEForce && peData.count > 0) {
        const decomp = bridge.peGetForceDecomposition?.() ?? null;
        if (decomp) {
            viewport.updatePEForceDecomposition(decomp, GRAVITY_VIS_GAIN);
        }
    }
    telemetryHub.s1._overlayForceOn = anyPEForce;

    if (ov.system && peData.count > 0) {
        const sys = computeSystemVectors(peData);
        viewport.updatePESystem(sys.com, sys.p, sys.l);
        // hub.peAngMom is the origin-frame L the native engine reports
        // (particle_engine.cpp sums r x mv from raw, non-shifted positions).
        // sys.l above is the true CoM-relative L computed here in JS; surface
        // its magnitude so the "about CoM" telemetry row doesn't silently
        // read the origin-frame channel under a false label.
        telemetryHub.s1._overlaySystemL = Math.hypot(sys.l[0], sys.l[1], sys.l[2]);
    }
    telemetryHub.s1._overlaySystemOn = ov.system;

    if (ov.admissibilityRing && peData.count > 0) {
        viewport.updateAdmissibilityRings(peData, scale1State.promotedSeedById, peData.ids);
    }

    // Provenance labels rebuild a unique CanvasTexture (a GPU upload) per
    // promoted particle every call — clusterId/size never change after
    // capture, only position does, so refreshing at the same cadence as the
    // E-field streamlines above (not every rAF frame) avoids needless
    // per-frame texture churn; a few-frame lag in label position is
    // imperceptible for slow-moving promoted clusters.
    if (ov.provenanceLabel && peData.count > 0 && refreshStreamlines) {
        viewport.updateProvenanceLabels(peData, scale1State.promotedSeedById, peData.ids);
    }
    telemetryHub.s1._overlayProvenanceOn = ov.provenanceLabel;

    // Mass-comparison badges rebuild a unique CanvasTexture per connector
    // every call (same cost profile as the provenance labels above), so
    // this overlay is throttled to the same refreshStreamlines cadence from
    // the start rather than needing a follow-up fix.
    if (ov.massComparison && peData.count > 0 && refreshStreamlines
        && scale1State.lastPromotion?.voxelDebug) {
        viewport.updateMassComparison(
            peData, scale1State.promotedSeedById,
            scale1State.lastPromotion.voxelDebug,
            scale1State.lastPromotion.latticeSize,
            scale1State.lastPromotion.displayScale);
    }
    telemetryHub.s1._overlayMassComparisonOn = ov.massComparison;

    // ── 4. Render ────────────────────────────────────────────────────
    viewport.render();

    // ── 5. Telemetry + panels (throttled to every 3rd frame) ─────────
    if (frameCount % 3 === 0) {
        const diag = telemetryHub.collectScale1(bridge);
        const ext = telemetryHub.collectScale1Extended(bridge);

        if (diag) {
            const cache = scale1State.statusCache;
            const sTick = formatSI(diag.tick);
            const sParticles = String(diag.particleCount);
            const sEnergy = formatEnergy(diag.totalEnergy, 1).text;
            const sState = running ? 'Running' : 'Idle';

            if (cache.tick !== sTick) { dom.statusPtime.textContent = sTick; cache.tick = sTick; }
            if (cache.particles !== sParticles) { dom.statusParticles.textContent = sParticles; cache.particles = sParticles; }
            if (cache.energy !== sEnergy) { dom.statusEnergy.textContent = sEnergy; cache.energy = sEnergy; }
            if (cache.state !== sState) {
                dom.statusState.textContent = sState;
                cache.state = sState;
                if (running) dom.statusDot.classList.remove('idle');
                else dom.statusDot.classList.add('idle');
            }

            // NOTE: Scale 1 no longer pushes into the legacy null-canvas
            // FluxEnergyChart/ParticleChart. Those were wired to hub VIEWS
            // (no push method — every call threw a swallowed page error) and
            // the adapted row fabricated Scale-0 fields with zeros. The hub's
            // _s1_pe ring is the single Scale-1 history; the charts panel
            // reads it via descriptors.
            scale1State.lastPushedTick = diag.tick;
        }

        // Panel redraws honor floated panels too (audit defect: switching on
        // activeTab alone froze floated Scale-1 panels).
        const visible = (id) => (typeof isPanelVisible === 'function'
            ? isPanelVisible(id) : activeTab === id);
        if (visible('inspector')) inspector.update();
    }
}


// =====================================================================
// Exported: loadPEScenario(ctx, name)
// =====================================================================

export function loadPEScenario(ctx, name) {
    const { bridge, viewport } = ctx;

    if (!bridge.initPE) return;
    const scenarioId = getScale1Scenario(name) ? name : DEFAULT_SCALE1_SCENARIO;
    const preset = getScale1ScenarioPreset(scenarioId);

    // Delegate to app.js master reset (clears charts, trails, field cache,
    // and resets all toggle buttons across all scales)
    ctx.resetAllVisualState();

    bridge.initPE();

    // Re-baseline hub telemetry (pe* ring buffers + _peInitialEnergy drift
    // reference) so the new scenario doesn't inherit the previous one's
    // energy baseline.
    telemetryHub.resetScale(1);
    scale1State.lastPushedTick = -1;
    scale1State.currentScenarioId = scenarioId;

    applyPEPhysicsPreset(bridge, preset);

    const scenario = getScale1Scenario(scenarioId);
    scenario?.setup?.({ bridge, viewport });

    // Runtime metadata for the diagnostics panel (hub no longer reads DOM).
    telemetryHub.setScale1Runtime({
        scenario: scenario?.label ?? scenarioId,
        softening: preset.physics.softening ?? 0.1,
    });

    applyPEOverlayPreset(viewport, preset);

    // Epistemic status readout in the toolbar (matches Scale 0's pattern).
    const descEl = document.getElementById('s1-scenario-desc-text');
    if (descEl) descEl.textContent = preset.description;
    const descWrap = document.getElementById('s1-scenario-desc');
    if (descWrap) descWrap.style.display = '';

    // Promotion provenance card reflects any capture the scenario consumed.
    refreshPromotionCard();

    // Soft circles + shader manifestation (void slots stay as faint ghosts)
    if (viewport?.setParticleShape) viewport.setParticleShape(0);
    if (viewport?.setParticleGlow) viewport.setParticleGlow(0.28);
    if (viewport?.setPEManifestation) viewport.setPEManifestation(true, 0, MANIFEST_FILL);
}


// =====================================================================
// Exported: bindScale1ControlsUI()
// =====================================================================

export function bindScale1ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale1ControlsComponent(controlsPanel).init();
}
