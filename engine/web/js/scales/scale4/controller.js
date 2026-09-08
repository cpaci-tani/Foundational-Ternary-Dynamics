/**
 * Scale 4 (Solar System) Controller
 *
 * Owns the Scale 4 N-Body physics loop, scenario loading, and UI list mapping.
 * Extracted from app.js to isolate planetary logic into a self-contained module.
 */

import { BaseLifecycleController } from '../../lifecycle.js';
import { PlanetaryMockBridge } from '../../bridge/mock-scale4.js?v=10';
import { PlanetaryRenderer } from '../../planetary-renderer.js?v=8';
import { rafCoordinator } from '../../lib/raf-coordinator.js';
import { hideScale0Overlays, saveScaleCameraState, restoreScaleCameraState } from '../scale-utils.js';
import { telemetryHub } from '../../telemetry-hub.js';
import { DEFAULT_SOLAR_PHYSICS } from '../../config/solar-system-physics.js?v=4';

// F-6: drive the planetary loop from the shared rAF coordinator instead of
// setInterval(…, 16). 60 Hz matches the old ~16 ms cadence (1000/16 ≈ 62.5),
// and because 60 ≥ the coordinator's VISIBILITY_PAUSE_THRESHOLD_HZ (30) the
// loop keeps advancing when the tab is backgrounded — exactly the opposite of
// setInterval, which throttles to ~1 Hz and lets the sim drift. Integration is
// frame-counted (accumulate ticksPerFrame → bridge.run(f)), NOT
// wall-clock-integrated. The bridge owns stable solver subdivision for the
// selected physical duration of each public tick.
const PLANETARY_LOOP_HZ = 60;
const PLANETARY_LOOP_ID = 'scale4-planetary-loop';

function bodyBreakdown(diag, prefix = 'modeled bodies') {
    const parts = [`${diag.bodyCount} ${prefix}`];
    if (diag.starCount) parts.push(`${diag.starCount} ${diag.starCount === 1 ? 'star' : 'stars'}`);
    if (diag.planetCount) parts.push(`${diag.planetCount} ${diag.planetCount === 1 ? 'planet' : 'planets'}`);
    if (diag.dwarfPlanetCount) parts.push(`${diag.dwarfPlanetCount} ${diag.dwarfPlanetCount === 1 ? 'dwarf' : 'dwarfs'}`);
    if (diag.moonCount) parts.push(`${diag.moonCount} ${diag.moonCount === 1 ? 'moon' : 'moons'}`);
    if (diag.smallBodyCount) parts.push(`${diag.smallBodyCount} small ${diag.smallBodyCount === 1 ? 'body' : 'bodies'}`);
    if (diag.debrisCount) parts.push(`${diag.debrisCount} debris`);
    return parts.join(' · ');
}

function formatElapsedDays(days) {
    const totalMinutes = Math.max(0, Math.round((Number(days) || 0) * 24 * 60));
    const wholeDays = Math.floor(totalMinutes / (24 * 60));
    const hours = Math.floor((totalMinutes % (24 * 60)) / 60);
    const minutes = totalMinutes % 60;
    return `${wholeDays}d ${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}`;
}

function formatIntegrationSlice(seconds) {
    if (!Number.isFinite(seconds)) return 'natural units';
    if (seconds >= 60 - 1e-6) {
        const minutes = seconds / 60;
        const label = Math.abs(minutes - Math.round(minutes)) < 1e-9
            ? String(Math.round(minutes))
            : String(Number(minutes.toPrecision(3)));
        return `${label} min`;
    }
    if (seconds >= 1) return `${seconds.toPrecision(3)} s`;
    return `${seconds.toExponential(2)} s`;
}

class Scale4LifecycleController extends BaseLifecycleController {
    constructor() {
        super();
        this.bridge = null;
        this.renderer = null;
        // AU/M_sun/yr dynamics are the default. The internal mode name remains
        // `planetary` to avoid breaking saved workspace and routing contracts.
        this._gravityMode = 'physical';
        this._timeStepKey = 'minute';
        this._physicsState = { ...DEFAULT_SOLAR_PHYSICS };
        this._viewState = {
            orbits: true, axes: false, labels: true, moons: true,
            ecliptic: true, belts: true, habitable: true,
            gravityField: true, accelerationVectors: false, velocityVectors: false,
            hillSpheres: false, rocheLimits: false, collisionShells: false,
        };
        this._lastUiUpdate = 0;
        this._lastBodyRevision = -1;
    }

    mount(ctx) {
        // Standard setup placeholder
    }

    loadScenario(ctx, name = 'planetary-solar') {
        const { viewport, inspector } = ctx;

        // Isolate visualization overlays
        hideScale0Overlays(viewport);

        // Body ids are scoped to a scenario load. Clear the old selection
        // before recreating the bridge so an id reused by the next scenario
        // cannot silently reinterpret (for example) Sun #0 as Saturn #0.
        inspector?.clearSelection?.();

        this.bridge = new PlanetaryMockBridge();
        // Persist the user's gravity-mode choice across the bridge recreate that
        // happens on every (re)load. A fresh bridge defaults to 'physical';
        // re-apply the remembered mode BEFORE setupScenario so initial
        // velocities are generated with the correct G (P0-1).
        if (this._gravityMode) this.bridge.setGravityMode(this._gravityMode);
        this.bridge.setTimeStep(this._timeStepKey);
        this.bridge.setPhysicsToggles(this._physicsState);
        this.bridge.setupScenario(name);
        telemetryHub.resetScale(4);

        this._disposePlanetaryRenderer();
        this.renderer = new PlanetaryRenderer(viewport.scene, viewport.camera, viewport.renderer, viewport.controls);
        this.trackThreeObject(this.renderer);

        if (inspector) inspector.setPlanetaryContext(this.bridge, this.renderer);

        // Capture the pre-Scale-4 camera/controls state so destroy() can restore
        // it for other scales (audit P1-8a). The `!this._saved*` guard is
        // load-bearing: loadScenario() re-runs on every in-place reload
        // (scenario change, gravity-mode change) WITHOUT an intervening
        // destroy(), and by then near/far/min/max have already been narrowed
        // below. Capturing only on the first load (when _saved* is null) keeps
        // the originals pristine; destroy() nulls them so a later re-entry
        // re-captures a fresh baseline. Vectors are cloned so later camera
        // motion doesn't mutate the saved snapshot.
        saveScaleCameraState(this, viewport);

        // Camera contract: astronomical state is expressed in AU and the full
        // Solar System needs a much deeper clip range than the former toy model.
        viewport.camera.near = 0.001;
        viewport.camera.far = 500;
        viewport.camera.updateProjectionMatrix();
        viewport.controls.minDistance = 0.01;
        viewport.controls.maxDistance = 500;

        const data = this.bridge.getPlanetaryData();
        this.renderer.update(data);
        this._lastBodyRevision = data.bodyRevision;
        this._applyViewState();
        this._setSystemCamera(viewport);

        this._populateLayerList(ctx);
        this._bindToggles(ctx);

        this._startPlanetaryLoop(ctx);
    }

    _startPlanetaryLoop(ctx) {
        // loadScenario() runs again on every in-place reload (scenario change,
        // gravity-mode change) WITHOUT an intervening destroy(), so tear down
        // any prior loop first — otherwise subscriptions stack and the sim runs
        // N× too fast after N reloads. (Wave-1 invariant, preserved for the rAF
        // path: rafCoordinator.subscribe with a duplicate id only replaces the
        // entry, but unsubscribing first keeps the contract explicit and lets
        // destroy() share one teardown helper.)
        this._stopPlanetaryLoop();

        // Per-frame fractional-tick accumulator (was a closure local under
        // setInterval; promoted to an instance field so a scenario reload that
        // re-subscribes does not silently inherit a stale partial tick).
        this._planetAcc = 0;

        // NOTE (P0-5): ctx is the live getter object from app.js _makeCtx(), so
        // ctx.running / ctx.engineMode / ctx.ticksPerFrame are read fresh each
        // frame — NOT captured snapshots. This is what makes pause work after
        // load. Do not destructure these into locals here.
        //
        // F-6: subscribe to the shared rAF coordinator at 60 Hz in place of
        // setInterval(…, 16). The body is unchanged, so the integration is
        // identical at the playback layer: same accumulation of ticksPerFrame
        // and same bridge.run(f). The selected minute/hour/day interval and its
        // stable internal subdivisions remain bridge-owned.
        this._planetaryLoopSub = rafCoordinator.subscribe(PLANETARY_LOOP_ID, {
            hz: PLANETARY_LOOP_HZ,
            cb: () => {
                // Guard: if we've switched away from planetary, idle.
                // (_stopPlanetaryLoop() in destroy() also drops this
                // subscription; this is belt-and-suspenders for the
                // in-place-reload window.)
                if (ctx.engineMode !== 'planetary') {
                    return;
                }

                if (ctx.running) {
                    this._planetAcc += ctx.ticksPerFrame || 1;
                    const f = Math.floor(this._planetAcc);
                    this._planetAcc -= f;
                    if (f > 0 && this.bridge) this.bridge.run(f);
                }

                if (this.bridge) {
                    telemetryHub.collectScale4(this.bridge);
                    this._updateLiveTelemetry();
                }

                if (this.bridge && this.renderer) {
                    const currentData = this.bridge.getPlanetaryData();
                    this.renderer.update(currentData);
                    if (currentData.bodyRevision !== this._lastBodyRevision) {
                        this._lastBodyRevision = currentData.bodyRevision;
                        this._populateLayerList(ctx);
                    }
                }

                // Update DOM diagnostics and inspector state
                if (ctx.inspector) ctx.inspector.update();
                if (ctx.viewport) ctx.viewport.render();
            },
        });
    }

    /**
     * Tear down the planetary frame loop. Idempotent; safe to call when no
     * loop is active. (F-6: replaces the Wave-1 clearInterval teardown.)
     */
    _stopPlanetaryLoop() {
        if (this._planetaryLoopSub) {
            this._planetaryLoopSub.unsubscribe();
            this._planetaryLoopSub = null;
        }
    }

    /**
     * Dispose and untrack the current renderer during an in-place scenario
     * reload. Without the untrack step, every retired renderer stayed strongly
     * referenced until Scale 4 was exited and was disposed a second time by
     * BaseLifecycleController.destroy().
     */
    _disposePlanetaryRenderer() {
        if (!this.renderer) return;
        const trackedIndex = this._threeObjects.indexOf(this.renderer);
        if (trackedIndex >= 0) this._threeObjects.splice(trackedIndex, 1);
        this.renderer.dispose();
        this.renderer = null;
    }

    _populateLayerList(ctx) {
        const layerList = document.getElementById('planetary-layer-list');
        const bodies = this.bridge?.getBodies?.() || this.bridge?._bodies || [];
        if (layerList && bodies.length) {
            layerList.innerHTML = '';
            const byParent = new Map();
            for (const body of bodies) {
                const parentId = body.parentId ?? -1;
                if (!byParent.has(parentId)) byParent.set(parentId, []);
                byParent.get(parentId).push(body);
            }

            const selectBody = (body) => {
                if (ctx.inspector) {
                    ctx.inspector.setEngineMode('planetary');
                    ctx.inspector.selectPlanetaryBody?.(body.id);
                    const btn = document.querySelector('.tab[data-panel="inspector"]');
                    if (btn) btn.click();
                } else {
                    this.renderer?.setSelectedBody?.(body.id);
                }
            };

            const makeRow = (b, nested = false, tagName = 'li') => {
                const row = document.createElement(tagName);
                row.className = `solar-system-body-row${nested ? ' is-moon' : ''}`;
                row.dataset.bodyId = String(b.id);
                row.setAttribute('role', 'option');
                row.setAttribute('aria-selected', 'false');
                if (tagName !== 'summary') row.tabIndex = 0;
                row.title = `Focus and inspect ${b.name || `body ${b.id}`}. Position and mean-radius geometry use the same physical AU scale.${b.colorBasis ? ` Appearance: ${b.colorBasis}.` : ''}`;
                const dot = document.createElement('span');
                dot.className = 'solar-system-body-dot';
                dot.style.background = b.color || '#8ca4ba';
                const copy = document.createElement('span');
                copy.className = 'solar-system-body-copy';
                const name = document.createElement('strong');
                name.textContent = b.name || `Body ${b.id}`;
                const meta = document.createElement('small');
                meta.textContent = `${b.className || 'Celestial body'} · ${b.type === 0 ? b.mass.toPrecision(4) : b.mass.toExponential(3)} M☉`;
                copy.append(name, meta);
                row.append(dot, copy);
                row.addEventListener('click', () => selectBody(b));
                if (tagName !== 'summary') {
                    row.addEventListener('keydown', (event) => {
                        if (event.key !== 'Enter' && event.key !== ' ') return;
                        event.preventDefault();
                        selectBody(b);
                    });
                }
                return row;
            };

            const bodyIds = new Set(bodies.map((body) => body.id));
            const roots = bodies.filter((body) => body.parentId == null || body.parentId < 0 || !bodyIds.has(body.parentId));
            const appendBranch = (body, container, depth = 0) => {
                const children = byParent.get(body.id) || [];
                if (!children.length) {
                    container.appendChild(makeRow(body, depth > 0));
                    return;
                }
                const wrapper = document.createElement('li');
                wrapper.className = 'solar-system-branch';
                const details = document.createElement('details');
                details.open = depth === 0 || ['earth', 'jupiter', 'saturn'].includes(body.key);
                const summary = makeRow(body, depth > 0, 'summary');
                const descendants = document.createElement('ul');
                descendants.className = 'solar-system-moon-list';
                children.forEach((child) => appendBranch(child, descendants, depth + 1));
                details.append(summary, descendants);
                wrapper.appendChild(details);
                container.appendChild(wrapper);
            };
            (roots.length ? roots : bodies).forEach((body) => appendBranch(body, layerList));
            this._syncLayerSelection(ctx.inspector?._selectedPlanetaryId ?? -1);
        }
    }

    _syncLayerSelection(bodyId) {
        const selectedId = Number(bodyId);
        document.querySelectorAll('#planetary-layer-list [data-body-id]').forEach((element) => {
            const selected = Number(element.dataset.bodyId) === selectedId;
            element.classList.toggle('is-selected', selected);
            element.setAttribute('aria-selected', selected ? 'true' : 'false');
        });
    }

    _applyViewState() {
        if (!this.renderer) return;
        this.renderer.setRenderOrbits(this._viewState.orbits);
        this.renderer.setRenderAxes(this._viewState.axes);
        this.renderer.setRenderLabels(this._viewState.labels);
        this.renderer.setRenderMoons(this._viewState.moons);
        this.renderer.setEclipticVisible(this._viewState.ecliptic);
        this.renderer.setRenderBelts(this._viewState.belts);
        this.renderer.setRenderHabitableZone(this._viewState.habitable);
        this.renderer.setRenderGravityField(this._viewState.gravityField);
        this.renderer.setRenderAccelerationVectors(this._viewState.accelerationVectors);
        this.renderer.setRenderVelocityVectors(this._viewState.velocityVectors);
        this.renderer.setRenderHillSpheres(this._viewState.hillSpheres);
        this.renderer.setRenderRocheLimits(this._viewState.rocheLimits);
        this.renderer.setRenderCollisionShells(this._viewState.collisionShells);
    }

    _setSystemCamera(viewport) {
        if (!viewport || !this.bridge) return;
        const bodies = this.bridge.getBodies?.() || [];
        const radius = Math.max(0.4, ...bodies.map((body) => Math.hypot(body.x, body.y, body.z)));
        const star = bodies.find((body) => body.type === 0);
        const target = star ? { x: star.x, y: star.z, z: star.y } : { x: 0, y: 0, z: 0 };
        const distance = Math.min(95, Math.max(1.6, radius * 1.42));
        viewport.camera.near = 0.001;
        viewport.camera.far = 500;
        viewport.camera.updateProjectionMatrix();
        viewport.controls.minDistance = 0.01;
        viewport.controls.maxDistance = 500;
        viewport.controls.target.set(target.x, target.y, target.z);
        viewport.camera.position.set(target.x, target.y + distance * 0.57, target.z + distance);
        viewport.camera.lookAt(target.x, target.y, target.z);
        viewport.controls.update?.();
    }

    _updateLiveTelemetry(force = false) {
        const now = performance.now();
        if (!force && now - this._lastUiUpdate < 100) return;
        this._lastUiUpdate = now;
        const diag = this.bridge?.getDiagnostics?.();
        if (!diag) return;
        const time = document.getElementById('planetary-live-time');
        const count = document.getElementById('planetary-live-bodies');
        const summary = document.getElementById('planetary-system-summary');
        const timeLabel = document.getElementById('status-time-label');
        const bodyLabel = document.getElementById('status-particles-label');
        const energyLabel = document.getElementById('status-energy-label');
        const statusTime = document.getElementById('status-ptime');
        const statusBodies = document.getElementById('status-particles');
        const statusEnergy = document.getElementById('status-energy');
        const overlayStatus = document.getElementById('planetary-overlay-status');
        const panelTitle = document.getElementById('planetary-system-title');
        const gravityReadout = document.getElementById('planetary-ctrl-gravity');
        const integratorReadout = document.getElementById('planetary-ctrl-integrator');
        const unitsReadout = document.getElementById('planetary-ctrl-units');
        const epochReadout = document.getElementById('planetary-ctrl-epoch');
        const appearanceReadout = document.getElementById('planetary-ctrl-appearance');
        const overlayFootnote = document.querySelector('#cs-viewport-overlay .scale-overlay-footnote');
        const scaleGauge = document.getElementById('planetary-scale-gauge');
        const geometryContract = document.getElementById('planetary-geometry-contract');
        const physicsOverlayReadout = document.getElementById('planetary-physics-overlay-readout');
        const totalEnergy = telemetryHub.plTotal?.last?.();
        if (time) time.textContent = diag.scenario === 'planetary-solar'
            ? `tick ${diag.tick} · J2000 + ${formatElapsedDays(diag.timeDays)}`
            : (diag.scenario === 'planetary-threebody'
                ? `tick ${diag.tick} · ${diag.timeYears.toFixed(5)} natural time`
                : `tick ${diag.tick} · ${formatElapsedDays(diag.timeDays)} elapsed`);
        if (count) count.textContent = bodyBreakdown(diag);
        if (summary) summary.textContent = bodyBreakdown(diag, 'active bodies');
        const selectedScenario = document.querySelector('#planetary-scenario-select option:checked');
        if (panelTitle) panelTitle.textContent = selectedScenario?.textContent?.trim() || 'Solar System';
        if (timeLabel) timeLabel.textContent = diag.scenario === 'planetary-threebody' ? 'Time' : 'Day';
        if (bodyLabel) bodyLabel.textContent = 'Bodies';
        if (energyLabel) energyLabel.textContent = 'Mech E';
        if (statusTime) {
            statusTime.textContent = diag.scenario === 'planetary-threebody'
                ? diag.timeYears.toFixed(4)
                : diag.timeDays.toFixed(diag.timeStep?.key === 'minute' ? 5 : (diag.timeStep?.key === 'hour' ? 4 : 2));
            statusTime.title = diag.scenario === 'planetary-threebody'
                ? 'Elapsed figure-eight natural-unit time.'
                : (diag.scenario === 'planetary-solar'
                    ? `Elapsed physical model time in days since J2000. Tick ${diag.tick}; ${diag.timeStep?.label || 'selected interval'} per tick.`
                    : `Elapsed physical model time in days since this experiment or catalog seed was loaded. Tick ${diag.tick}; ${diag.timeStep?.label || 'selected interval'} per tick.`);
        }
        if (statusBodies) {
            statusBodies.textContent = String(diag.bodyCount);
            statusBodies.title = `${diag.bodyCount} gravitational bodies are active in the current system.`;
        }
        if (statusEnergy) {
            statusEnergy.textContent = Number.isFinite(totalEnergy) ? totalEnergy.toExponential(3) : '—';
            statusEnergy.title = diag.scenario === 'planetary-threebody'
                ? 'Total kinetic plus gravitational potential energy in figure-eight natural units.'
                : 'Newtonian kinetic plus pair-potential energy in M☉·AU²/yr². Perturbation potentials and dissipative ledgers are reported separately.';
        }
        if (overlayStatus) {
            overlayStatus.textContent = diag.scenario === 'planetary-threebody'
                ? 'Orbital mechanics · Figure-8 · G=1 natural units'
                : (this._gravityMode === 'physical'
                    ? `${diag.activePhysicsCount}/${diag.applicablePhysicsCount} effective kernels · physical AU/M☉/yr · ${diag.timeStep.label}/tick`
                    : 'Slow comparison gauge · lattice G · not astronomy-time faithful');
        }
        if (gravityReadout) {
            gravityReadout.textContent = diag.scenario === 'planetary-threebody'
                ? 'G = 1 · natural units'
                : `${this.bridge.G.toFixed(this._gravityMode === 'physical' ? 3 : 2)} AU³ M☉⁻¹ yr⁻²`;
        }
        if (integratorReadout) {
            integratorReadout.textContent = diag.scenario === 'planetary-threebody'
                ? `${diag.integrator} · 0.01 natural time/tick · ${diag.timeStep.integrationSubsteps}×${diag.dt.toExponential(1)}`
                : `${diag.integrator} · ${diag.timeStep.label}/tick · ${diag.timeStep.integrationSubsteps}×${formatIntegrationSlice(diag.timeStep.integrationDtSeconds)}`;
        }
        if (unitsReadout) unitsReadout.textContent = diag.units;
        if (epochReadout) {
            epochReadout.textContent = diag.scenario === 'planetary-solar' || diag.scenario === 'planetary-mercury-relativity'
                ? 'J2000 approximate elements'
                : (diag.scenario.startsWith('exo-')
                    ? `NASA PSCompPars · ${diag.provenance?.retrieved || 'catalog snapshot'}`
                    : 'Designed experiment initial state');
        }
        if (appearanceReadout) {
            appearanceReadout.textContent = diag.scenario.startsWith('exo-')
                ? 'Modeled class/temperature palette · not observed true color'
                : (diag.scenario === 'planetary-solar'
                    ? 'Planet-specific procedural reference palette'
                    : 'Scenario-specific procedural palette');
        }
        if (overlayFootnote) {
            overlayFootnote.textContent = diag.scenario.startsWith('exo-')
                ? '[PARAMETRIC] NASA Exoplanet Archive scale data; planet colors are modeled presentation cues, not observations or FTD predictions'
                : '[PARAMETRIC / IMPOSED] NASA/JPL data + standard effective celestial mechanics; not an FTD derivation';
        }
        const naturalUnits = diag.scenario === 'planetary-threebody';
        if (scaleGauge) {
            scaleGauge.textContent = naturalUnits ? '1:1 natural units' : '1:1 AU · physical radii';
            scaleGauge.title = naturalUnits
                ? 'The figure-eight experiment uses one undistorted natural-unit coordinate gauge; it is not an AU-scale astronomical system.'
                : 'Exact rendering conversion: 1 Three.js world unit equals 1 astronomical unit. Body surface radii and every separation use that same coordinate gauge.';
        }
        if (geometryContract) {
            geometryContract.textContent = naturalUnits
                ? 'Uncompressed 1:1 natural-unit geometry for the dimensionless figure-eight experiment; no astronomical-unit claim is made.'
                : (diag.scenario === 'planetary-solar'
                    ? 'Physical 1:1 AU geometry: JPL mean radii, body separations, and orbit paths share one uncompressed scale. Labels and selection marks are UI overlays.'
                    : 'Uncompressed 1:1 AU geometry: modeled body radii, separations, and orbit paths share one scale. Labels and selection marks are UI overlays.');
        }
        if (physicsOverlayReadout) {
            const overlay = this.renderer?.getPhysicsOverlayStatus?.();
            const field = overlay?.gravityField;
            if (field?.visible) {
                const scope = overlay.selectedOnly ? 'selected body' : `${field.sourceCount} bodies`;
                physicsOverlayReadout.textContent = `Gravity field · ${field.samples.toLocaleString()} samples · ${scope} · |g| ${field.min.toExponential(2)}–${field.max.toExponential(2)}`;
            } else {
                const counts = [
                    ['a', overlay?.accelerationVectors], ['v', overlay?.velocityVectors],
                    ['Hill', overlay?.hillSpheres], ['Roche', overlay?.rocheLimits],
                    ['contact', overlay?.collisionShells],
                ].filter(([, value]) => value > 0).map(([label, value]) => `${label} ${value}`);
                physicsOverlayReadout.textContent = counts.length ? counts.join(' · ') : 'Physics overlays off';
            }
        }
        const auditValues = {
            'planetary-audit-newtonian': diag.forceAudit?.newtonianAcceleration,
            'planetary-audit-gr': diag.forceAudit?.relativityAcceleration,
            'planetary-audit-j2': diag.forceAudit?.j2Acceleration,
            'planetary-audit-radiation': diag.forceAudit?.radiationAcceleration,
            'planetary-audit-pressure': diag.forceAudit?.radiationPressureAcceleration,
            'planetary-audit-pr-drag': diag.forceAudit?.poyntingRobertsonDragAcceleration,
            'planetary-audit-solar-wind': diag.forceAudit?.solarWindDragAcceleration,
            'planetary-audit-tide': diag.forceAudit?.tideAcceleration,
            'planetary-audit-atmosphere': diag.forceAudit?.atmosphereAcceleration,
        };
        for (const [id, value] of Object.entries(auditValues)) {
            const element = document.getElementById(id);
            if (element) element.textContent = Number.isFinite(value) ? Number(value).toExponential(3) : '—';
        }
        const collisions = document.getElementById('planetary-audit-collisions');
        const roche = document.getElementById('planetary-audit-roche');
        const dissipated = document.getElementById('planetary-audit-dissipated');
        const latestEvent = document.getElementById('planetary-audit-latest-event');
        if (collisions) collisions.textContent = String(diag.collisionCount || 0);
        if (roche) roche.textContent = String(diag.rocheDisruptionCount || 0);
        if (dissipated) dissipated.textContent = Number(diag.dissipatedEnergy || 0).toExponential(3);
        if (latestEvent) latestEvent.textContent = diag.latestEvent?.message || '—';
        this._updatePhysicsControls();
    }

    _bindToggles(ctx) {
        // loadScenario() can run repeatedly without an intervening destroy()
        // (the scenario <select> reloads in place), so guard each binding with
        // a dataset flag to avoid stacking duplicate listeners.
        //
        // NOTE (audit §E / P1-8 dead-UI): #planetary-opt-orbits and
        // #planetary-opt-axes are NOT dead. The audit checked index.html only;
        // these checkboxes are rendered at runtime by getPlanetaryPanelTemplate()
        // in js/ui/components/panel-resources/template.js (the "Visualization
        // Overlays" card of #panel-planetary), alongside #planetary-layer-list
        // which _populateLayerList() fills. Both bindings drive real renderer
        // behaviour — setRenderOrbits() toggles orbit-line visibility,
        // setRenderAxes() lazily builds + toggles a per-mesh AxesHelper
        // (planetary-renderer.js). Keep them. Do NOT also render these IDs in the
        // Scale-4 toolbar template: duplicate element IDs would split the toggle
        // state across two checkboxes and getElementById would bind only the
        // first.
        const toggles = [
            ['planetary-opt-orbits', 'orbits', 'setRenderOrbits'],
            ['planetary-opt-axes', 'axes', 'setRenderAxes'],
            ['planetary-opt-labels', 'labels', 'setRenderLabels'],
            ['planetary-opt-moons', 'moons', 'setRenderMoons'],
            ['planetary-opt-ecliptic', 'ecliptic', 'setEclipticVisible'],
            ['planetary-opt-belts', 'belts', 'setRenderBelts'],
            ['planetary-opt-habitable', 'habitable', 'setRenderHabitableZone'],
            ['planetary-opt-gravity-field', 'gravityField', 'setRenderGravityField'],
            ['planetary-opt-acceleration', 'accelerationVectors', 'setRenderAccelerationVectors'],
            ['planetary-opt-velocity', 'velocityVectors', 'setRenderVelocityVectors'],
            ['planetary-opt-hill', 'hillSpheres', 'setRenderHillSpheres'],
            ['planetary-opt-roche', 'rocheLimits', 'setRenderRocheLimits'],
            ['planetary-opt-collision', 'collisionShells', 'setRenderCollisionShells'],
        ];
        if (!this._selectionSyncBound) {
            this._selectionSyncBound = true;
            this.bindEvent(document, 'ftd:scale4-selection-change', (event) => {
                this._syncLayerSelection(event.detail?.bodyId ?? -1);
            });
        }
        for (const [id, key, method] of toggles) {
            const el = document.getElementById(id);
            if (!el) continue;
            el.checked = this._viewState[key];
            if (!el.dataset.s4Bound) {
                el.dataset.s4Bound = '1';
                this.bindEvent(el, 'change', (e) => {
                    this._viewState[key] = e.target.checked;
                    this.renderer?.[method]?.(e.target.checked);
                });
            }
        }

        const home = document.getElementById('planetary-home-view');
        if (home && !home.dataset.s4Bound) {
            home.dataset.s4Bound = '1';
            this.bindEvent(home, 'click', () => {
                ctx.inspector?.clearSelection?.();
                this.renderer?.stopFollowingBody?.();
                this._setSystemCamera(ctx.viewport);
            });
        }

        // Gravity-constant mode toggle (P0-1). this._gravityMode is the
        // controller-level source of truth (the bridge is recreated on every
        // reload, so the choice must survive at controller scope). On change,
        // remember the mode and reload the current scenario so initial
        // velocities are regenerated for the new G — orbits run ~63× faster in
        // 'physical' (Keplerian) vs 'presentation' (slow lattice-G comparison).
        const gravSel = document.getElementById('planetary-gravity-mode');
        if (gravSel && !gravSel.dataset.s4Bound) {
            gravSel.dataset.s4Bound = '1';
            this.bindEvent(gravSel, 'change', (e) => {
                this._gravityMode = e.target.value;
                const scenario = document.getElementById('planetary-scenario-select')?.value
                    || this.bridge?._scenarioName
                    || 'planetary-solar';
                this.loadScenario(ctx, scenario);
            });
        }
        // Keep the visible selection in sync with the active mode across both
        // first mount and in-place scenario reloads.
        if (gravSel) gravSel.value = this._gravityMode;

        const timeStepSelect = document.getElementById('planetary-time-step');
        if (timeStepSelect && !timeStepSelect.dataset.s4Bound) {
            timeStepSelect.dataset.s4Bound = '1';
            this.bindEvent(timeStepSelect, 'change', (event) => {
                if (!this.bridge?.setTimeStep?.(event.target.value)) return;
                this._timeStepKey = event.target.value;
                this._planetAcc = 0;
                this._syncTimeStepControl();
                this._updateLiveTelemetry(true);
            });
        }
        this._syncTimeStepControl();

        document.querySelectorAll('[data-solar-physics]').forEach((el) => {
            const key = el.dataset.solarPhysics;
            el.checked = !!this._physicsState[key];
            if (!el.dataset.s4PhysicsBound) {
                el.dataset.s4PhysicsBound = '1';
                this.bindEvent(el, 'change', (event) => {
                    this._physicsState[key] = event.target.checked;
                    this.bridge?.setPhysicsToggle?.(key, event.target.checked);
                    this._updatePhysicsControls();
                    this._updateOverlayStatus();
                });
            }
        });

        const enableAll = document.getElementById('planetary-physics-enable-all');
        if (enableAll && !enableAll.dataset.s4PhysicsBound) {
            enableAll.dataset.s4PhysicsBound = '1';
            this.bindEvent(enableAll, 'click', () => {
                this._physicsState = { ...DEFAULT_SOLAR_PHYSICS };
                this.bridge?.setPhysicsToggles?.(this._physicsState);
                document.querySelectorAll('[data-solar-physics]').forEach((el) => {
                    el.checked = true;
                });
                this._updatePhysicsControls();
                this._updateOverlayStatus();
            });
        }

        this._updatePhysicsControls();
        this._updateOverlayStatus();
    }

    _updatePhysicsControls() {
        const status = this.bridge?.getPhysicsStatus?.() || [];
        const active = status.filter((item) => item.active).length;
        const applicable = status.filter((item) => item.applicable).length;
        const summary = document.getElementById('planetary-physics-summary');
        if (summary) summary.textContent = `${active} active · ${applicable} applicable · ${status.length} available`;
        for (const item of status) {
            const input = document.getElementById(`planetary-physics-${item.key}`);
            const row = input?.closest?.('.solar-physics-row');
            const state = document.getElementById(`planetary-physics-state-${item.key}`);
            if (input) input.checked = item.requested;
            if (row) {
                row.classList.toggle('is-active', item.active);
                row.classList.toggle('is-standby', item.requested && !item.applicable);
                row.classList.toggle('is-off', !item.requested);
                row.title = `${item.description} ${item.applicable ? 'Applicable to the current scenario.' : `Standby: ${item.reason}`} [${item.status}]`;
            }
            if (state) state.textContent = item.active ? 'active' : (item.requested ? 'standby' : 'off');
        }
    }

    _syncTimeStepControl() {
        const select = document.getElementById('planetary-time-step');
        if (!select || !this.bridge) return;
        const step = this.bridge.getTimeStepStatus?.();
        select.disabled = !!step?.naturalUnits;
        select.value = step?.naturalUnits ? 'natural' : this._timeStepKey;
        select.title = step?.naturalUnits
            ? 'This dimensionless Figure-8 experiment uses 0.01 natural-time units per tick; minutes, hours, and days have no defined mapping here.'
            : `${step.label} advances per Scale 4 tick. The solver uses ${step.integrationSubsteps} stable integration ${step.integrationSubsteps === 1 ? 'step' : 'substeps'}; playback speed separately multiplies ticks per frame.`;
    }

    /**
     * Update the viewport overlay status line so the UI does not assert AU/yr
     * timing fidelity while in 'presentation' mode (P0-1). In 'physical' mode the
     * Keplerian AU/M☉/yr timing is faithful, so the label says so.
     */
    _updateOverlayStatus() {
        const statusEl = document.getElementById('planetary-overlay-status');
        if (statusEl) {
            if (this.bridge?._scenarioName === 'planetary-threebody') {
                statusEl.textContent = 'Orbital mechanics — Figure-8 three-body (G=1 natural units; not AU/yr)';
            } else {
                const diag = this.bridge?.getDiagnostics?.();
                statusEl.textContent = this._gravityMode === 'physical'
                    ? `${diag?.activePhysicsCount ?? 0}/${diag?.applicablePhysicsCount ?? 0} effective kernels · physical AU/M☉/yr · ${diag?.timeStep?.label || '1 minute'}/tick`
                    : 'Slow comparison gauge · lattice G · not astronomy-time faithful';
            }
        }
        const gravEl = document.getElementById('planetary-ctrl-gravity');
        if (gravEl && this.bridge) {
            const diag = this.bridge.getDiagnostics?.();
            gravEl.textContent = diag?.scenario === 'planetary-threebody'
                ? 'G = 1 · natural units'
                : `${this.bridge.G.toFixed(this._gravityMode === 'physical' ? 3 : 2)} AU³ M☉⁻¹ yr⁻²`;
        }
        this._updateLiveTelemetry(true);
    }

    step() {
        if (this.bridge) {
            this.bridge.run(1);
            if (this.renderer) {
                const currentData = this.bridge.getPlanetaryData();
                this.renderer.update(currentData);
            }
        }
    }

    destroy(ctx) {
        // F-6: the frame loop is now an rAF-coordinator subscription, not a
        // tracked setInterval, so super.destroy() no longer clears it. Drop the
        // subscription explicitly before the base teardown so a later re-mount
        // starts a fresh one and the coordinator stops its rAF when no other
        // subscribers remain.
        this._stopPlanetaryLoop();
        this._disposePlanetaryRenderer();
        super.destroy(ctx);
        this._selectionSyncBound = false;
        // The toolbar toggle elements persist in the DOM across scale switches
        // (hidden via .scale4-only). super.destroy() removed their listeners, so
        // clear the bind-guard flags too; otherwise re-entering Scale 4 would
        // see s4Bound and skip re-binding, leaving the toggles dead.
        ['planetary-opt-orbits', 'planetary-opt-axes', 'planetary-opt-labels', 'planetary-opt-moons',
            'planetary-opt-ecliptic', 'planetary-opt-belts', 'planetary-opt-habitable',
            'planetary-opt-gravity-field', 'planetary-opt-acceleration', 'planetary-opt-velocity',
            'planetary-opt-hill', 'planetary-opt-roche', 'planetary-opt-collision',
            'planetary-home-view', 'planetary-gravity-mode', 'planetary-time-step',
            'planetary-physics-enable-all'].forEach((id) => {
            const el = document.getElementById(id);
            if (el) {
                delete el.dataset.s4Bound;
                delete el.dataset.s4PhysicsBound;
            }
        });
        document.querySelectorAll('[data-solar-physics]').forEach((el) => {
            delete el.dataset.s4PhysicsBound;
        });
        this.bridge = null;
        const timeLabel = document.getElementById('status-time-label');
        const particleLabel = document.getElementById('status-particles-label');
        const energyLabel = document.getElementById('status-energy-label');
        if (timeLabel) timeLabel.textContent = 'T';
        if (particleLabel) particleLabel.textContent = 'Particles';
        if (energyLabel) energyLabel.textContent = 'Energy';
        // Restore lattice particles visibility for other scales
        if (ctx && ctx.viewport && ctx.viewport.particles) {
            ctx.viewport.particles.visible = true;
        }
        // Restore camera/controls clip planes + zoom limits to their
        // pre-Scale-4 values (audit P1-8a; originals captured in loadScenario).
        //
        // Division of labour with viewport.setEngineMode() (called downstream
        // via inspectorRuntime.syncMode AFTER this destroy()): that path already
        // re-derives camera.near, controls.minDistance, and re-centres
        // position/target for the destination scale — but it NEVER touches
        // camera.far or controls.maxDistance. Scale 4 narrows both
        // (far 2000→1000, maxDistance 500→100), so without this restore a
        // Scale 4→0 switch leaves the lattice with a 1000-unit far plane and a
        // 100-unit zoom cap → far geometry culled / z-fighting. far + maxDistance
        // are therefore the load-bearing restores here; near/position/min/target
        // are restored too (harmless — setEngineMode overwrites them) so this
        // teardown stays self-contained.
        if (ctx && ctx.viewport) {
            restoreScaleCameraState(this, ctx.viewport);
        }
    }
}

const _lifecycleController = new Scale4LifecycleController();

export function mount(ctx) {
    _lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    _lifecycleController.destroy(ctx);
}

export function loadScenario(ctx, name = 'planetary-solar') {
    _lifecycleController.loadScenario(ctx, name);
}

export function step() {
    _lifecycleController.step();
}

/** Read-only lifecycle evidence used by the Scale 4 regression audit. */
export function getScale4RuntimeAudit() {
    return {
        hasBridge: !!_lifecycleController.bridge,
        hasRenderer: !!_lifecycleController.renderer,
        trackedThreeObjects: _lifecycleController._threeObjects.length,
        loopActive: !!_lifecycleController._planetaryLoopSub,
        scenario: _lifecycleController.bridge?._scenarioName || null,
    };
}

export function dispose(ctx) {
    _lifecycleController.destroy(ctx);
}
