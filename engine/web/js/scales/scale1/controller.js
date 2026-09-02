/**
 * Scale 1 (Particles) Controller — native-engine edition.
 *
 * Scale 1 separates a read-only registered Native Matter observer from the
 * continuous Effective Particle Lab. Catalog Reference keeps [PARAMETRIC]
 * identities apart. Runtime scale handoff has been retired.
 *
 * State lives in ./state/store.js (scale1State) — no module-level lets.
 * Rendering goes through the shared viewport facade.
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
import { Scale1ControlsComponent } from './ui/controls/component.js?v=13';
import {
    refreshScale1ScenarioContractCard, hydratePePhysicsControls,
    markPePhysicsProfileModified, setPePhysicsProfileState, syncPeTrailControls,
    updatePeTrailEnergyLegend,
} from './ui/controls/pe-controls.js?v=16';
import {
    expandPEToCloud, buildPEManifestBlinkRate, updateTrailHistory,
    getCloudParticleMap, getTrailHistory, clearCloudAndTrails, MANIFEST_FILL
} from './pe-cloud-expander.js?v=3';
import {
    getScale1Scenario, getScale1ScenarioPreset, DEFAULT_SCALE1_SCENARIO,
    installScale1ScenarioManifest, populateScale1ScenarioSelect,
    syncScale1ScenarioBehaviorUI,
} from './scenario-registry.js?v=15';
import { scale1State, resetScale1State } from './state/store.js?v=7';
import {
    DEFAULT_TRAIL_SETTINGS,
    normalizeTrailSettings,
} from './trail-settings.js?v=2';
import { telemetryHub } from '../../telemetry-hub.js';
import { estimateOrbitPeriod } from './telemetry/orbit-period.js';
import { scale1ParticleLedger } from './telemetry/particle-ledger.js?v=2';
import {
    focusedFieldSources,
    focusedSystemObservables,
} from './inspection-focus.js?v=1';
import { shouldRefreshScale1Observation } from './observation-cadence.js?v=1';
import { scale1ParticleWorkerExecutor } from './particle-worker-executor.js?v=2';


// =====================================================================
// Exported: overlay toggle setters + cloud/trail accessors
// =====================================================================

function updatePEOverlaySummary() {
    const summary = document.getElementById('pe-overlay-summary');
    if (!summary) return;
    const active = Object.values(scale1State.overlays).filter(Boolean).length;
    summary.textContent = `${active} active`;
}

function setOverlay(key, on) {
    const next = !!on;
    if (scale1State.overlays[key] !== next) scale1State.observationDirty = true;
    scale1State.overlays[key] = next;
    updatePEOverlaySummary();
}

export function setPEEField(on)       { setOverlay('efield', on); }
export function setPEPotential(on)    { setOverlay('potential', on); }
export function setPEFieldBattery(on) { setOverlay('fieldBattery', on); }
export function isPEFieldSurfaceActive() {
    return !!(scale1State.overlays.potential || scale1State.overlays.fieldBattery);
}
export function setPEGravField(on)    { setOverlay('gravityField', on); }
export function setPEForceCoulomb(on) { setOverlay('forceCoulomb', on); }
export function setPEForceGravity(on) { setOverlay('forceGravity', on); }
export function setPEForceLorentz(on) { setOverlay('forceLorentz', on); }
export function setPEForceExchange(on) { setOverlay('forceExchange', on); }
export function setPEForceStrong(on)  { setOverlay('forceStrong', on); }
export function setPEForceRadiation(on) { setOverlay('forceRadiation', on); }
export function setPEForceMagneticDipole(on) { setOverlay('forceMagneticDipole', on); }
export function setPEForceSpinOrbit(on) { setOverlay('forceSpinOrbit', on); }
export function setPEForceNet(on)     { setOverlay('forceNet', on); }
export function setPESystem(on)       { setOverlay('system', on); }
export function setVelocities(on)     { setOverlay('velocities', on); }
export function setTrails(on)         { setOverlay('trails', on); }
export function setAdmissibilityRing(on) { setOverlay('admissibilityRing', on); }
export function setProvenanceLabel(on) { setOverlay('provenanceLabel', on); }
export function markObservationDirty() { scale1State.observationDirty = true; }
export function markPhysicsProfileModified() {
    markPePhysicsProfileModified();
    markObservationDirty();
}

export function setTrailSettings(patch = {}) {
    scale1State.trailSettings = normalizeTrailSettings(patch, scale1State.trailSettings);
    syncPeTrailControls(scale1State.trailSettings);
    return { ...scale1State.trailSettings };
}

export function resetTrailSettings() {
    scale1State.trailSettings = { ...DEFAULT_TRAIL_SETTINGS };
    syncPeTrailControls(scale1State.trailSettings);
    return { ...scale1State.trailSettings };
}

export function applyPhysicsProfile(bridge, profile) {
    if (!bridge || scale1State.mode === 'native_matter') return false;
    const specs = Array.from(scale1State.registry?.physics || []);
    if (!specs.length) return false;

    const scenarioPreset = profile === 'scenario'
        ? getScale1ScenarioPreset(scale1State.currentScenarioId, scale1State.registry)
        : null;
    let accepted = true;
    for (const spec of specs) {
        let enabled = false;
        if (profile === 'applicable') enabled = !!spec.available;
        else if (profile === 'verified') enabled = !!spec.verifiedProfile;
        else if (profile === 'scenario') enabled = !!scenarioPreset?.physics?.[spec.toggle];
        else return false;

        const result = bridge.peSetToggle?.(spec.toggle, enabled);
        if (spec.available && result === false) accepted = false;
    }

    hydratePePhysicsControls(scale1State.registry, bridge);
    const labels = {
        scenario: 'Scenario profile',
        verified: 'Verified profile',
        applicable: 'All applicable',
    };
    setPePhysicsProfileState(labels[profile], profile !== 'scenario');
    markObservationDirty();
    return accepted;
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
    for (const spec of Array.from(scale1State.registry?.physics || [])) {
        bridge.peSetToggle?.(spec.toggle, !!p[spec.toggle]);
    }

    if (p.dt !== undefined) {
        bridge.peSetDt?.(p.dt);
        setSliderValue('pe-dt-slider', p.dt, 1);
    }
    if (p.softening !== undefined) {
        bridge.peSetSoftening?.(p.softening);
        setSliderValue('pe-soft-slider', p.softening, 2);
    }
    markObservationDirty();

    setCheckbox('pe-coulomb', p.coulomb);
    setCheckbox('pe-gravity', p.gravity);
    setCheckbox('pe-damping', p.damping);
    setCheckbox('pe-lorentz-p', p.lorentz);
    setCheckbox('pe-exchange', p.exchange);
    setCheckbox('pe-strong', p.strong);
    setCheckbox('pe-magnetic-dipole', p.magnetic_dipole);
    setCheckbox('pe-spin-orbit', p.spin_orbit);
    setCheckbox('pe-radiation', p.radiation);
    setCheckbox('pe-relativistic-verlet', p.relativistic_verlet);
    setCheckbox('pe-contact-events', p.contact_events);
}

function inspectionFocusKey(focus) {
    if (!focus) return '';
    if (focus.kind === 'particle') return `particle:${focus.particleId}`;
    if (focus.kind === 'cluster') return `cluster:${focus.key}`;
    return String(focus.kind || 'focus');
}

function applyPEOverlayPreset(viewport, preset) {
    const o = preset.overlays || {};
    const ov = scale1State.overlays;
    ov.velocities = !!o.velocities;
    ov.trails = !!o.trails;
    ov.efield = !!o.efield;
    ov.potential = !!o.potential;
    ov.fieldBattery = !!o.fieldBattery;
    ov.gravityField = !!o.gravityField;
    ov.forceCoulomb = !!(o.forceCoulomb ?? o.forces);
    ov.forceGravity = !!o.forceGravity;
    ov.forceLorentz = !!o.forceLorentz;
    ov.forceExchange = !!o.forceExchange;
    ov.forceStrong = !!o.forceStrong;
    ov.forceRadiation = !!o.forceRadiation;
    ov.forceMagneticDipole = !!o.forceMagneticDipole;
    ov.forceSpinOrbit = !!o.forceSpinOrbit;
    ov.forceNet = !!(o.forceNet ?? o.forces);
    ov.system = !!o.system;
    ov.admissibilityRing = !!o.admissibilityRing;
    ov.provenanceLabel = !!o.provenanceLabel;

    setButtonActive('toggle-velocities', ov.velocities);
    setButtonActive('toggle-trails', ov.trails);
    setButtonActive('toggle-pe-efield', ov.efield);
    setButtonActive('toggle-pe-potential', ov.potential);
    setButtonActive('toggle-pe-field-battery', ov.fieldBattery);
    setButtonActive('toggle-pe-gravity-field', ov.gravityField);
    setButtonActive('toggle-pe-force-coulomb', ov.forceCoulomb);
    setButtonActive('toggle-pe-force-gravity', ov.forceGravity);
    setButtonActive('toggle-pe-force-lorentz', ov.forceLorentz);
    setButtonActive('toggle-pe-force-exchange', ov.forceExchange);
    setButtonActive('toggle-pe-force-strong', ov.forceStrong);
    setButtonActive('toggle-pe-force-radiation', ov.forceRadiation);
    setButtonActive('toggle-pe-force-magnetic-dipole', ov.forceMagneticDipole);
    setButtonActive('toggle-pe-force-spin-orbit', ov.forceSpinOrbit);
    setButtonActive('toggle-pe-force-net', ov.forceNet);
    setButtonActive('toggle-pe-system', ov.system);
    setButtonActive('toggle-pe-admissibility', ov.admissibilityRing);
    setButtonActive('toggle-pe-provenance', ov.provenanceLabel);
    updatePEOverlaySummary();

    if (!viewport) return;
    viewport.toggleVelocityVectors(ov.velocities);
    viewport.toggleTrails(ov.trails);
    viewport.togglePEStreamlines(ov.efield);
    viewport.toggleFieldHeatmap(ov.potential || ov.fieldBattery);
    viewport.toggleFieldVectors(ov.potential || ov.fieldBattery);
    viewport.toggleGravityVectors(ov.gravityField);
    viewport.togglePEForceCoulomb(ov.forceCoulomb);
    viewport.togglePEForceGravity(ov.forceGravity);
    viewport.togglePEForceLorentz(ov.forceLorentz);
    viewport.togglePEForceExchange(ov.forceExchange);
    viewport.togglePEForceStrong(ov.forceStrong);
    viewport.togglePEForceRadiation(ov.forceRadiation);
    viewport.togglePEForceMagneticDipole(ov.forceMagneticDipole);
    viewport.togglePEForceSpinOrbit(ov.forceSpinOrbit);
    viewport.togglePEForceNet(ov.forceNet);
    viewport.togglePESystem(ov.system);
    viewport.toggleAdmissibilityRings(ov.admissibilityRing);
    viewport.toggleProvenanceLabels(ov.provenanceLabel);
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
    scale1ParticleWorkerExecutor.ensure();
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function resetScale1(ctx) {
    _lifecycleController.destroy(ctx);
}

function _resetScale1Internal(ctx) {
    const { viewport, inspector } = ctx;

    inspector?.clearPEInspection?.();

    clearCloudAndTrails();
    resetScale1State();
    scale1ParticleWorkerExecutor.dispose();

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
        viewport.toggleAdmissibilityRings?.(false);
        viewport.toggleProvenanceLabels?.(false);
        if (viewport.setPEManifestation) viewport.setPEManifestation(false, 0);
    }
}


// =====================================================================
// System observables helper
// =====================================================================

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
        const workerHandled = scale1State.mode !== 'native_matter'
            && scale1ParticleWorkerExecutor.request(bridge, wholeTicks, () => {
                scale1State.observationDirty = true;
            });
        if (!workerHandled) {
            for (let i = 0; i < wholeTicks; i++) bridge.peTick();
        }
    } else if (scale1ParticleWorkerExecutor.status().busy) {
        // Pause is an immediate ownership boundary. Any in-flight batch may
        // finish inside the worker, but its generation is invalidated so no
        // post-pause tick can be committed to the dashboard engine.
        scale1ParticleWorkerExecutor.invalidate();
    }

    // ── 2. Cloud expansion + shared observation snapshot ─────────────
    // Positions remain frame-fresh. The expensive exact ledger, hierarchy,
    // event, and force serialization work runs on one load-aware cadence and
    // is completely idle when a paused state has not changed.
    const peData = bridge.peGetParticleData();
    const peTick = Number(bridge.peGetTick?.()
        ?? scale1State.lastSnapshot?.core?.tick
        ?? telemetryHub.s1.diag?.tick
        ?? 0);
    const observationRevision = Number(bridge.peGetObservationRevision?.() ?? peTick);
    const nowMs = Number.isFinite(Number(now)) ? Number(now) : performance.now();
    const observationDue = shouldRefreshScale1Observation({
        dirty: scale1State.observationDirty,
        hasSnapshot: !!scale1State.lastSnapshot,
        tick: peTick,
        count: peData.count,
        lastTick: scale1State.lastObservationTick,
        revision: observationRevision,
        lastRevision: scale1State.lastObservationRevision,
        lastCount: scale1State.lastObservationCount,
        nowMs,
        lastObservationMs: scale1State.lastObservationMs,
    });

    if (observationDue) {
        if (scale1State.currentScenarioId === 's1-finite-port-field-battery') {
            let battery = bridge.peGetFinitePortBatterySnapshot?.() ?? null;
            const desiredLayers = Math.min(
                Number(battery?.capacity || 0),
                1 + Math.floor(peTick / 20),
            );
            while (battery && Number(battery.acceptedLayers || 0) < desiredLayers) {
                if (!bridge.peStepFinitePortBattery?.()) break;
                battery = bridge.peGetFinitePortBatterySnapshot?.() ?? battery;
            }
            scale1State.finitePortBatterySnapshot = battery;
            const status = document.getElementById('pe-field-battery-status');
            if (status && battery) {
                status.textContent = `Layer ${battery.acceptedLayers}/${battery.capacity} · `
                    + `E ${Number(battery.totalBookedEnergy).toPrecision(6)}`;
            }
        } else {
            scale1State.finitePortBatterySnapshot = null;
        }
        scale1State.lastSnapshot = bridge.peGetSnapshot?.(
            scale1State.currentScenarioId || '') ?? scale1State.lastSnapshot;
        scale1State.lastForceData = bridge.peGetForces?.() ?? null;
        scale1ParticleLedger.observe({
            peData,
            snapshot: scale1State.lastSnapshot,
            forceData: scale1State.lastForceData,
            scenarioId: scale1State.currentScenarioId || '',
            scenarioLabel: getScale1Scenario(scale1State.currentScenarioId)?.label || '',
            softening: scale1State.softening,
        });
        const ledgerView = scale1ParticleLedger.getView();
        inspector?.reconcilePEInspection?.(ledgerView.hierarchy);
        scale1State.trailEnergyDensityById.clear();
        for (const particle of Array.from(ledgerView.hierarchy?.particles || [])) {
            const radius = Math.max(0.001, Number(particle.effectiveRadius) || 0.4);
            const effectiveVolume = (4 / 3) * Math.PI * radius * radius * radius;
            scale1State.trailEnergyDensityById.set(
                Number(particle.id),
                Math.max(0, Number(particle.kineticEnergy) || 0) / effectiveVolume,
            );
        }
        scale1State.visualRecordById.clear();
        if (scale1State.mode === 'native_matter') {
            for (const object of Array.from(scale1State.lastSnapshot?.objects || [])) {
                scale1State.visualRecordById.set(object.id, {
                    clusterId: object.provenance?.sourceObjectId ?? object.id,
                    size: object.manifestationSupportCount || object.constituentCount || 0,
                    admissible: object.provenance?.qualification === 'qualified_selected',
                    label: `native #${object.provenance?.sourceObjectId ?? object.id} · c${object.id}`,
                });
            }
        }
        scale1State.lastObservationMs = nowMs;
        scale1State.lastObservationTick = peTick;
        scale1State.lastObservationRevision = observationRevision;
        scale1State.lastObservationCount = peData.count;
        scale1State.observationDirty = false;
    }
    const typeMap = bridge.peGetParticleTypes();
    const forceData = scale1State.lastForceData;
    const inspectionFocus = inspector?.getPEInspectionFocus?.() || null;
    const focusKey = inspectionFocusKey(inspectionFocus);
    const focusChanged = focusKey !== scale1State.lastInspectionFocusKey;
    if (focusChanged) {
        scale1State.lastInspectionFocusKey = focusKey;
        scale1State.lastFieldSources = null;
        scale1State.lastForceDecomposition = null;
    }
    const overlayRefreshDue = observationDue || focusChanged;
    const fieldOverlayActive = ov.potential || ov.fieldBattery
        || ov.efield || ov.gravityField;
    const fieldRefreshDue = overlayRefreshDue
        || (fieldOverlayActive && !scale1State.lastFieldSources);
    const frameSec = typeof now === 'number' ? now * 0.001 : performance.now() * 0.001;
    const blinkRate = buildPEManifestBlinkRate(peData, forceData, frameSec);
    const cloud = expandPEToCloud(peData, typeMap, { blinkRate, frameSec });
    viewport.updateParticles(cloud);

    if (viewport.setPEManifestation) {
        viewport.setPEManifestation(true, frameSec, MANIFEST_FILL);
    }

    if (peData.spinAxes && peData.spins && peData.count > 0) {
        viewport.updateSpinVectors?.(
            peData.positions, peData.spinAxes, peData.spins, peData.count, peData.ids);
        viewport.toggleSpinVectors?.(true);
    } else {
        viewport.toggleSpinVectors?.(false);
    }

    if (inspector) {
        inspector.setPEContext(getCloudParticleMap(), cloud.count, typeMap);
    }

    // ── 3. Overlays ──────────────────────────────────────────────────
    if (ov.velocities && peData.count > 0) {
        viewport.updateVelocityVectors(peData.positions, peData.velocities, peData.count, peData.ids);
    }
    telemetryHub.s1._overlayVelocitiesOn = ov.velocities;

    if (running) {
        updateTrailHistory(
            peData,
            peTick,
            scale1State.trailSettings,
            scale1State.trailEnergyDensityById,
        );
    }
    let trailStats = null;
    if (ov.trails) {
        trailStats = viewport.updateTrails(
            getTrailHistory(), typeMap, scale1State.trailSettings, peTick);
    }
    updatePeTrailEnergyLegend(trailStats, scale1State.trailSettings.renderMode);
    telemetryHub.s1._overlayTrailsOn = ov.trails;
    // Orbit-period estimate (2-body proxy): built from the hub's own
    // tick-gated peSeparation channel (collectScale1Extended), NOT from the
    // visual trail cache (pe-cloud-expander.js getTrailHistory()). Although
    // that cache is now tick-stamped, it remains a presentation cache with a
    // user-adjustable stride and history window, so it must not supply the
    // invariant orbit-period diagnostic. The hub's separation channel is the
    // registered tick-aligned telemetry source estimateOrbitPeriod expects;
    // only new-tick separation samples are appended below.
    if (ov.trails && peData.count === 2) {
        // A pair identity key (not just count===2) guards against selected removal
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

    let peFieldSources = scale1State.lastFieldSources;
    if ((ov.potential || ov.efield || ov.gravityField) && peData.count > 0
        && (fieldRefreshDue || !peFieldSources)) {
        peFieldSources = focusedFieldSources(
            {
                positions: peData.positions,
                charges: peData.charges,
                masses: peData.masses,
                count: peData.count,
            },
            peData.ids,
            inspectionFocus,
            scale1State.inspectionFieldSources,
        );
        scale1State.lastFieldSources = peFieldSources;
    }

    const finitePortField = scale1State.finitePortBatterySnapshot;
    if (ov.fieldBattery && finitePortField?.count > 0 && fieldRefreshDue) {
        viewport.updateFieldHeatmap(
            finitePortField.positions, finitePortField.magnitudes,
            finitePortField.count, finitePortField.maxMagnitude);
        viewport.updateFieldVectors(
            finitePortField.positions, finitePortField.vectors,
            finitePortField.count, finitePortField.maxMagnitude, 5.0);
        telemetryHub.s1._potentialMin = 0;
        telemetryHub.s1._potentialMax = Number(finitePortField.maxMagnitude) || 0;
    } else if (ov.potential && peFieldSources?.count > 0 && fieldRefreshDue) {
        if (!scale1State.fieldGrid) scale1State.fieldGrid = generateGridXZ(25, 20);
        const grid = scale1State.fieldGrid;
        const field = samplePECoulombOnly(peFieldSources, grid.positions, grid.count);
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
    const refreshStreamlines = fieldRefreshDue;
    if (ov.efield && peFieldSources?.count > 0 && refreshStreamlines) {
        const fieldFn = makePECoulombFieldFn(peFieldSources, 0.5);
        const buf = scale1State.srcParticlesBuf;
        while (buf.length < peFieldSources.count) buf.push({ x: 0, y: 0, z: 0 });
        buf.length = peFieldSources.count;
        for (let i = 0; i < peFieldSources.count; i++) {
            buf[i].x = peFieldSources.positions[i * 3];
            buf[i].y = peFieldSources.positions[i * 3 + 1];
            buf[i].z = peFieldSources.positions[i * 3 + 2];
        }
        const seeds = generateEFieldSeeds(buf, 3, 100);
        const lines = computeStreamlines({ fieldFn }, seeds, {
            maxSteps: 80, stepSize: 0.5, bounds: 30
        });
        viewport.updatePEStreamlines(lines);
    }
    telemetryHub.s1._overlayEfieldOn = ov.efield;

    if (ov.gravityField && peFieldSources?.count > 0 && fieldRefreshDue) {
        if (!scale1State.fieldGrid) scale1State.fieldGrid = generateGridXZ(25, 20);
        const grid = scale1State.fieldGrid;
        const field = samplePEGravityField(peFieldSources, grid.positions, grid.count);
        viewport.updateGravityVectors(
            grid.positions, field.forces, grid.count, field.maxForce);
    }
    telemetryHub.s1._overlayGravityFieldOn = ov.gravityField;

    // Per-particle decomposed force arrows (native Float64 decomposition —
    // `net` is the TRUE integrator force incl. every enabled term)
    const anyPEForce = ov.forceCoulomb || ov.forceGravity || ov.forceLorentz
        || ov.forceExchange || ov.forceStrong || ov.forceRadiation
        || ov.forceMagneticDipole || ov.forceSpinOrbit || ov.forceNet;
    if (anyPEForce && peData.count > 0) {
        if (overlayRefreshDue || !scale1State.lastForceDecomposition) {
            scale1State.lastForceDecomposition =
                bridge.peGetForceDecomposition?.() ?? null;
        }
        const decomp = scale1State.lastForceDecomposition;
        if (decomp) {
            viewport.updatePEForceDecomposition(decomp, GRAVITY_VIS_GAIN, peData.ids);
        }
    }
    telemetryHub.s1._overlayForceOn = anyPEForce;

    if (ov.system && peData.count > 0) {
        const conservation = scale1State.lastSnapshot?.conservation;
        const focusedSystem = focusedSystemObservables(peData, inspectionFocus);
        if (focusedSystem) {
            viewport.updatePESystem(
                focusedSystem.center,
                focusedSystem.momentum,
                focusedSystem.angularMomentum,
            );
            telemetryHub.s1._overlaySystemL = Math.hypot(...focusedSystem.angularMomentum);
        } else if (conservation) {
            const c = conservation.centerOfMass;
            const p = conservation.totalMomentum;
            const l = conservation.totalAngularMomentum;
            viewport.updatePESystem([c.x, c.y, c.z], [p.x, p.y, p.z], [l.x, l.y, l.z]);
            telemetryHub.s1._overlaySystemL = Math.hypot(l.x, l.y, l.z);
        }
    }
    telemetryHub.s1._overlaySystemOn = ov.system;

    if (ov.admissibilityRing && peData.count > 0) {
        viewport.updateAdmissibilityRings(peData, scale1State.visualRecordById, peData.ids);
    }

    // Provenance labels rebuild a unique CanvasTexture per record, so refresh
    // them at the same cadence as field streamlines rather than every frame.
    if (ov.provenanceLabel && peData.count > 0 && refreshStreamlines) {
        viewport.updateProvenanceLabels(peData, scale1State.visualRecordById, peData.ids);
    }
    telemetryHub.s1._overlayProvenanceOn = ov.provenanceLabel;

    // ── 4. Render ────────────────────────────────────────────────────
    viewport.render();

    // Tick and state remain frame-fresh without forcing an exact energy pass.
    const liveCache = scale1State.statusCache;
    const liveTick = formatSI(peTick);
    const liveCount = String(peData.count);
    const liveState = scale1State.mode === 'native_matter'
        ? 'Read-only' : (running ? 'Running' : 'Idle');
    if (liveCache.tick !== liveTick) {
        dom.statusPtime.textContent = liveTick;
        liveCache.tick = liveTick;
    }
    if (liveCache.particles !== liveCount) {
        dom.statusParticles.textContent = liveCount;
        liveCache.particles = liveCount;
    }
    if (liveCache.state !== liveState) {
        dom.statusState.textContent = liveState;
        liveCache.state = liveState;
        dom.statusDot.classList.toggle('idle', !running);
    }

    // ── 5. Exact telemetry + panels (shared observation cadence) ─────
    if (observationDue) {
        const diag = telemetryHub.collectScale1(bridge);
        telemetryHub.collectScale1Extended(bridge);

        if (diag) {
            const cache = scale1State.statusCache;
            const sTick = formatSI(diag.tick);
            const sParticles = String(diag.particleCount);
            const formattedEnergy = formatEnergy(diag.totalEnergy, 1).text;
            const sEnergy = diag.stateEnergyComplete ? formattedEnergy
                : `partial ${formattedEnergy}`;
            const sState = scale1State.mode === 'native_matter'
                ? 'Read-only' : (running ? 'Running' : 'Idle');

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
    const { bridge, viewport, inspector } = ctx;

    if (!bridge.initPE) return;
    scale1ParticleWorkerExecutor.invalidate();
    scale1ParticleWorkerExecutor.ensure();
    scale1State.registry = bridge.peGetPhysicsRegistry?.() ?? null;
    installScale1ScenarioManifest(scale1State.registry);
    const requested = getScale1Scenario(name);
    const scenarioId = requested?.available ? name : DEFAULT_SCALE1_SCENARIO;
    const scenario = getScale1Scenario(scenarioId);
    populateScale1ScenarioSelect(
        document.getElementById('pe-scenario-select'), scenarioId);
    const preset = getScale1ScenarioPreset(scenarioId, scale1State.registry);

    inspector?.clearPEInspection?.();

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
    scale1State.softening = preset.physics.softening ?? 0.1;

    scale1State.mode = scenario?.mode || 'effective_lab';
    bridge.peSetMode?.(scale1State.mode);

    applyPEPhysicsPreset(bridge, preset);

    viewport?.clearPEScenarioVisual?.();
    scenario?.setup?.({ bridge, viewport });
    const fieldBatteryControls = document.getElementById('pe-field-battery-controls');
    if (fieldBatteryControls) {
        fieldBatteryControls.hidden = scenarioId !== 's1-finite-port-field-battery';
    }
    scale1State.finitePortBatterySnapshot =
        bridge.peGetFinitePortBatterySnapshot?.() ?? null;
    scale1State.lastSnapshot = bridge.peGetSnapshot?.(scenarioId) ?? null;
    scale1ParticleLedger.beginScenario({
        scenarioId,
        label: scenario?.label ?? scenarioId,
        tick: scale1State.lastSnapshot?.core?.tick ?? 0,
    });
    hydratePePhysicsControls(scale1State.registry, bridge);
    syncPeTrailControls(scale1State.trailSettings);

    // Runtime metadata for the diagnostics panel (hub no longer reads DOM).
    telemetryHub.setScale1Runtime({
        scenario: scenario?.label ?? scenarioId,
        softening: preset.physics.softening ?? 0.1,
        mode: scale1State.mode,
    });

    applyPEOverlayPreset(viewport, preset);

    // Epistemic status readout in the toolbar (matches Scale 0's pattern).
    const descEl = document.getElementById('s1-scenario-desc-text');
    if (descEl) descEl.textContent = preset.description;
    const descWrap = document.getElementById('s1-scenario-desc');
    if (descWrap) descWrap.style.display = '';

    syncScale1ScenarioBehaviorUI(scenario, scale1State.m3ViewId);
    refreshScale1ScenarioContractCard();

    // syncScale1ScenarioBehaviorUI renders the declared observable as
    // pedagogical metadata. Scenario changes, resets, and other UI-driven
    // reloads intentionally do not activate its recommended panel: the user's
    // current panel is retained, matching Scale 0.

    // Soft circles + shader manifestation (void slots stay as faint ghosts)
    if (viewport?.setParticleShape) viewport.setParticleShape(0);
    if (viewport?.setParticleGlow) viewport.setParticleGlow(0.28);
    if (viewport?.setPEManifestation) viewport.setPEManifestation(true, 0, MANIFEST_FILL);
    viewport?.setCameraPreset?.('front');
    // The registered M3 record is localized within one lattice unit, while
    // effective-lab scenarios span tens of units. Give the evidence artifact
    // an honest presentation zoom without changing its coordinates or radius.
    viewport?.setZoomMagnitude?.(scenarioId === 's1-native-m3-replay' ? 2.6 : 1.0);
}


// =====================================================================
// Exported: bindScale1ControlsUI()
// =====================================================================

export function bindScale1ControlsUI() {
    const controlsPanel = document.getElementById('panel-controls');
    if (controlsPanel) new Scale1ControlsComponent(controlsPanel).init();
}
