/**
 * Scale 0 Controls Panel Wiring
 *
 * Binds event listeners for every control card mounted by Scale0ControlsComponent:
 *   - Physics toggles card (all visible standard and research toggles)
 *   - Substrate controls card (injection, parameter sliders, field actions)
 *   - Flux volume card (shape, opacity, point size, threshold, scenario scale)
 *
 * This replaces the legacy wiring blocks in app.js's wireControls().
 *
 * IMPORTANT: `ctx` uses live accessors — closures read ctx.bridge / ctx.viewport
 * at event-handler invocation time so scale-switching (which reassigns bridge)
 * doesn't leave stale references behind.
 */

import {
    SCALE0_TOGGLES,
    SCALE0_ADVANCED_TOGGLES,
    getScale0ScenarioToggleProfile,
} from '../../../../config/toggles.js';
import { K_B } from '../../../../constants.js';
import { getPhysicsHarness } from '../../../../physics/index.js';
import { getEl, markScenarioOverrideRows } from '../dom.js?v=3';
import { updateScenarioMetadata } from '../bindings.js?v=3';
import {
    DEFAULT_FLOW_LINE_SETTINGS,
    getScale0State,
    getActiveLatticeSize,
    getActiveScale0Bridge,
    getFlowLineSettings,
    resetFlowLineSettings,
    setFlowLineSetting,
} from '../../state/store.js';
import { computeStreamlineParams } from '../../runtime/streamline-integrator.js';

let _wired = false;

function latticeN(ctx) {
    return getActiveLatticeSize(ctx, getScale0State());
}

/** Execute a substrate command on the one bridge that owns live Scale-0 state. */
function activeHarness(ctx, fn) {
    const owner = getActiveScale0Bridge(ctx, getScale0State());
    if (!owner) {
        console.error('[Scale0] substrate control has no active physics owner');
        return false;
    }
    fn(getPhysicsHarness(owner));
    return true;
}

function setDisplayText(display, text) {
    if (!display || display.textContent === text) return;
    const textNode = display.firstChild;
    if (display.childNodes.length === 1 && textNode?.nodeType === 3) {
        textNode.nodeValue = text;
    } else {
        display.textContent = text;
    }
}

/**
 * Collapse high-polling range input bursts to one latest-value transaction per
 * animation frame. Callers decide whether load generations invalidate queued
 * jobs and may perform one card-wide action after the committed batch.
 */
function createLatestInputFrame(ctx, {
    generationAware = true,
    canCommit = null,
    afterFlush = null,
} = {}) {
    const pendingInputs = new Map();
    let inputFrame = null;
    return (key, slider, display, format, apply, metadata = null) => {
        pendingInputs.set(key, {
            generation: generationAware ? (ctx._loadGeneration || 0) : null,
            slider,
            display,
            format,
            apply,
            metadata,
        });
        if (inputFrame !== null) return;
        inputFrame = requestAnimationFrame(() => {
            inputFrame = null;
            const jobs = [...pendingInputs.values()];
            pendingInputs.clear();
            if (canCommit && !canCommit()) return;
            const generation = ctx._loadGeneration || 0;
            const committed = [];
            for (const job of jobs) {
                if (generationAware && job.generation !== generation) continue;
                const value = Number.parseFloat(job.slider.value);
                if (!Number.isFinite(value)) continue;
                setDisplayText(job.display, job.format(value));
                job.apply(value);
                committed.push(job);
            }
            if (committed.length) afterFlush?.(committed);
        });
    };
}

function wirePhysicsToggles(ctx, api) {
    const currentScenarioId = () => getScale0State().currentScenarioId || 'flux-pulse';
    const physicsUiToggles = [...SCALE0_TOGGLES, ...SCALE0_ADVANCED_TOGGLES];
    const profileIsModified = () => {
        const fallback = new Map([
            ...getScale0ScenarioToggleProfile(currentScenarioId())
                .map(([key, expected]) => [key, !!expected]),
            ...SCALE0_ADVANCED_TOGGLES.map(([key, expected]) => [key, !!expected]),
        ]);
        return physicsUiToggles.some(([key, , elId]) => {
            const el = getEl(elId);
            if (!el) return false;
            const baseline = el.dataset.scale0ProfileValue;
            const expected = baseline === '1' || (baseline !== '0' && fallback.get(key));
            return el.checked !== !!expected;
        });
    };
    const renderProfileStatus = (modified = profileIsModified()) => {
        const warning = getEl('physics-profile-warning');
        if (warning && warning.hidden === modified) warning.hidden = !modified;
        updateScenarioMetadata(currentScenarioId(), {
            profileModified: modified,
            preserveDisclosure: true,
        });
    };
    const setActiveToggle = (toggleKey, value) => {
        const owner = getActiveScale0Bridge(ctx, getScale0State());
        if (typeof owner?.setToggle !== 'function') {
            console.error(`[Scale0] physics toggle owner missing setToggle (${toggleKey})`);
            return false;
        }
        owner.setToggle(toggleKey, value);
        return true;
    };
    const markUserEdit = () => {
        ctx._scale0ToggleUserEditGeneration = ctx._loadGeneration || 0;
    };

    // Engine readback calls this after repainting the checkboxes. Keeping the
    // status computation here avoids a second ownership model in the loader.
    ctx.onScale0ToggleProfileSynced = () => renderProfileStatus();

    for (const [toggleKey, , elId] of SCALE0_TOGGLES) {
        const el = getEl(elId);
        if (!el) continue;
        if (ctx.bridge?.isNativeGPU && toggleKey === 'confinement') {
            // TermToggles carries this as an intent flag for serialization,
            // but no native C++ phase consumes it. Do not present a writable
            // checkbox that implies string-tension physics is executing. The
            // separate viewport `toggle-confinement` remains a visual proxy.
            el.checked = false;
            el.disabled = true;
            el.setAttribute('aria-disabled', 'true');
            const label = el.closest('.toggle-row')?.querySelector('label');
            if (label) {
                label.textContent = 'Confinement (visual proxy only)';
                label.title = 'Native engine term is not implemented. Use the Confinement viewport overlay for visualization only.';
            }
            continue;
        }
        el.addEventListener('change', () => {
            markUserEdit();
            setActiveToggle(toggleKey, el.checked);
            const row = el.closest('.toggle-row');
            if (row?.classList.contains('scenario-override')) {
                row.classList.remove('scenario-override');
            }
            renderProfileStatus();
        });
    }

    // Advanced / research toggles (SCALE0_ADVANCED_TOGGLES): same setToggle
    // wiring. They are not reset by the JS whitelist, but an authoritative C++
    // scenario isolation profile may still clear them. Engine readback repaints
    // these controls, and any user change suspends scenario qualification just
    // like a standard term change.
    //
    // They remain absent from SCALE0_TOGGLES, so scenario-loader.js's whitelist
    // reset does not invent values for them. Requirement gating is documented
    // in each label's tooltip (e.g. Triad Binding requires Color Forces ON).
    for (const [toggleKey, , elId] of SCALE0_ADVANCED_TOGGLES) {
        const el = getEl(elId);
        if (!el) continue;
        el.addEventListener('change', () => {
            markUserEdit();
            setActiveToggle(toggleKey, el.checked);
            renderProfileStatus();
        });
    }

    // Re-run the canonical scenario load so C++ configure_* isolation wins.
    // Toggle-only restore from partial tables previously re-armed disabled terms.
    const resetBtn = getEl('btn-reset-physics-toggles');
    let resetFlashTimer = 0;
    if (resetBtn) {
        resetBtn.addEventListener('click', () => {
            if (resetBtn.disabled) return;
            resetBtn.disabled = true;
            const id = currentScenarioId();
            if (typeof api?.loadScenario === 'function') {
                api.loadScenario(ctx, id);
            } else {
                console.error('[Scale0] restore profile: api.loadScenario missing');
            }
            markScenarioOverrideRows(SCALE0_TOGGLES);
            renderProfileStatus(false);
            resetBtn.classList.add('ctrl-reset-flash');
            clearTimeout(resetFlashTimer);
            resetFlashTimer = setTimeout(() => {
                resetBtn.classList.remove('ctrl-reset-flash');
                resetBtn.disabled = false;
                resetFlashTimer = 0;
            }, 320);
        });
    }
}

function wireInjection(ctx, api) {
    let injState = 1;
    const injPos = getEl('inj-state-pos');
    const injNeg = getEl('inj-state-neg');
    if (injPos && injNeg) {
        injPos.addEventListener('click', () => {
            if (injState === 1) return;
            injState = 1;
            injPos.classList.add('active');
            injNeg.classList.remove('active');
        });
        injNeg.addEventListener('click', () => {
            if (injState === -1) return;
            injState = -1;
            injNeg.classList.add('active');
            injPos.classList.remove('active');
        });
    }

    function clampCoord(value) {
        const L = latticeN(ctx);
        if (!Number.isFinite(value)) return 0;
        return Math.max(0, Math.min(L - 1, Math.round(value)));
    }

    function syncCoordinateBounds(size = latticeN(ctx)) {
        const L = Number.isFinite(Number(size)) ? Math.max(1, Math.round(Number(size))) : latticeN(ctx);
        for (const id of ['inj-x', 'inj-y', 'inj-z']) {
            const el = getEl(id);
            if (!el) continue;
            const max = String(L - 1);
            if (el.max !== max) el.max = max;
            const bounded = Math.max(0, Math.min(L - 1, Math.round(Number(el.value) || 0)));
            if (el.value !== String(bounded)) el.value = String(bounded);
        }
    }

    function getInjPos() {
        syncCoordinateBounds();
        const values = {};
        for (const [axis, id] of [['x', 'inj-x'], ['y', 'inj-y'], ['z', 'inj-z']]) {
            const el = getEl(id);
            const value = clampCoord(Number(el?.value));
            if (el && el.value !== String(value)) el.value = String(value);
            values[axis] = value;
        }
        return { ...values, state: injState };
    }

    // Scenario and lattice-resize transactions call this after the active
    // owner is known. The action handlers also clamp at dispatch time so a
    // typed, unblurred out-of-range value can never reach the engine.
    ctx.syncScale0InjectionBounds = syncCoordinateBounds;
    syncCoordinateBounds();

    // Custom stepper buttons (+/- for each coord input)
    for (const btn of document.querySelectorAll('#panel-controls-grid .coord-step[data-for^="inj-"]')) {
        btn.addEventListener('click', () => {
            const targetId = btn.dataset.for;
            const step = parseInt(btn.dataset.step, 10) || 0;
            const input = getEl(targetId);
            if (!input) return;
            const current = parseInt(input.value, 10) || 0;
            input.value = clampCoord(current + step);
            syncCoordinateBounds();
        });
    }

    // Keep user-typed values within lattice bounds
    for (const id of ['inj-x', 'inj-y', 'inj-z']) {
        getEl(id)?.addEventListener('change', (e) => {
            e.target.value = clampCoord(parseInt(e.target.value, 10));
        });
    }

    getEl('btn-center')?.addEventListener('click', () => {
        syncCoordinateBounds();
        const half = Math.floor(latticeN(ctx) / 2);
        const x = getEl('inj-x'); if (x) x.value = half;
        const y = getEl('inj-y'); if (y) y.value = half;
        const z = getEl('inj-z'); if (z) z.value = half;
    });

    getEl('btn-random')?.addEventListener('click', () => {
        syncCoordinateBounds();
        const L = latticeN(ctx);
        const margin = L >= 5 ? 2 : 0;
        const span = Math.max(1, L - 2 * margin);
        const rand = () => margin + Math.floor(Math.random() * span);
        const x = getEl('inj-x'); if (x) x.value = rand();
        const y = getEl('inj-y'); if (y) y.value = rand();
        const z = getEl('inj-z'); if (z) z.value = rand();
        const { x: px, y: py, z: pz, state } = getInjPos();
        activeHarness(ctx, (h) => h.injectWavepacket(px, py, pz, state));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject')?.addEventListener('click', () => {
        const { x, y, z, state } = getInjPos();
        activeHarness(ctx, (h) => h.injectParticle(x, y, z, state));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject-wave')?.addEventListener('click', () => {
        const { x, y, z, state } = getInjPos();
        activeHarness(ctx, (h) => h.injectWavepacket(x, y, z, state));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject-flux')?.addEventListener('click', () => {
        const { x, y, z } = getInjPos();
        const active = getActiveScale0Bridge(ctx, getScale0State()) ?? ctx.bridge;
        const kb = getPhysicsHarness(active).getParam?.('kb') || K_B;
        activeHarness(ctx, (h) => h.injectFlux(x, y, z, kb * 0.8, 0, 0));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject-pair')?.addEventListener('click', () => {
        const { x, y, z } = getInjPos();
        const active = getActiveScale0Bridge(ctx, getScale0State()) ?? ctx.bridge;
        const kb = getPhysicsHarness(active).getParam?.('kb') || K_B;
        activeHarness(ctx, (h) => h.createEntangledPair(x, y, z, kb, 0, 0));
        api.setLatticeNeedsUpload();
    });
}

function wireParameterSliders(ctx) {
    const sliders = [
        { id: 'combo-kb',    valId: 'combo-kb-val',    param: 'kb',       fmt: 3 },
        { id: 'combo-gn',    valId: 'combo-gn-val',    param: 'gn',       fmt: 3 },
        { id: 'combo-damp',  valId: 'combo-damp-val',  param: 'damping',  fmt: 4 },
    ];
    for (const s of sliders) {
        const slider = getEl(s.id);
        const display = getEl(s.valId);
        if (!slider || !display) continue;
        // These are compile-time engine constants on WASM/C++ and
        // acknowledged profile values on native. Present truthful readouts;
        // no live Scale-0 backend exposes a writable setter for this card.
        slider.step = 'any';
        slider.disabled = true;
        slider.setAttribute('aria-readonly', 'true');
        slider.setAttribute('aria-disabled', 'true');
        slider.title = `${s.param} is a read-only engine constant.`;
        slider.classList.add('ctrl-slider-disabled');
        slider.closest('.pe-ctrl-row')?.classList.add('ctrl-native-readonly');
    }
}

function wireFieldActions(ctx, api) {
    getEl('btn-clear-field')?.addEventListener('click', () => {
        activeHarness(ctx, (h) => {
            if (typeof h.clearField === 'function') {
                h.clearField();
            } else {
                h.reset();
            }
        });
        ctx.clearCharts?.();
        api.setLatticeNeedsUpload();
    });

    getEl('btn-random-flux')?.addEventListener('click', () => {
        activeHarness(ctx, (h) => {
            h.seedRandomFlux?.();
        });
        api.setLatticeNeedsUpload();
    });
}

function wireFluxVolume(ctx, api) {
    // Range inputs can outpace the display refresh rate on high-polling mice.
    // Collapse every card-wide burst to one latest-value transaction per frame,
    // and collapse point-size/threshold upload invalidation to one dirty write.
    // A scenario load increments _loadGeneration; jobs from an older generation
    // are discarded because the loader has already captured/restored the DOM.
    const scheduleInput = createLatestInputFrame(ctx, {
        afterFlush: jobs => {
            if (jobs.some(job => job.metadata?.needsUpload)) api.setLatticeNeedsUpload();
        },
    });

    const shapeSelect = getEl('flux-shape-select');
    if (shapeSelect) {
        shapeSelect.addEventListener('change', () => {
            const shape = parseInt(shapeSelect.value, 10);
            if (!Number.isInteger(shape) || shape < 0 || shape > 7) return;
            ctx.viewport.setFluxShape(shape);
            ctx.viewport.setFluxSliceShape?.(shape);
        });
    }

    const opacitySlider = getEl('flux-opacity');
    const opacityVal = getEl('flux-opacity-val');
    if (opacitySlider && opacityVal) {
        opacitySlider.addEventListener('input', () => {
            scheduleInput('opacity', opacitySlider, opacityVal, v => v.toFixed(2), (v) => {
                if (ctx._scale0ForcedVisualParameterPreferences
                    && 'fluxOpacity' in ctx._scale0ForcedVisualParameterPreferences) {
                    ctx._scale0ForcedVisualParameterPreferences.fluxOpacity = v;
                }
                ctx.viewport.setFluxOpacity(v);
                ctx.viewport.setFluxSliceOpacity?.(v);
            });
        });
    }

    const scaleSlider = getEl('flux-point-scale');
    const scaleVal = getEl('flux-point-scale-val');
    if (scaleSlider && scaleVal) {
        scaleSlider.addEventListener('input', () => {
            scheduleInput('point-scale', scaleSlider, scaleVal, v => v.toFixed(1), (v) => {
                if (ctx._scale0ForcedVisualParameterPreferences
                    && 'fluxPointScale' in ctx._scale0ForcedVisualParameterPreferences) {
                    ctx._scale0ForcedVisualParameterPreferences.fluxPointScale = v;
                }
                ctx.viewport.setFluxPointScale(v);
                ctx.viewport.setFluxSlicePointScale?.(v);
            }, { needsUpload: true });
        });
    }

    const threshSlider = getEl('flux-threshold');
    const threshVal = getEl('flux-threshold-val');
    if (threshSlider && threshVal) {
        threshSlider.addEventListener('input', () => {
            scheduleInput('threshold', threshSlider, threshVal,
                v => (v < 0.001 ? v.toFixed(4) : v.toFixed(3)), (v) => {
                    if (ctx._scale0ForcedVisualParameterPreferences
                        && 'fluxThreshold' in ctx._scale0ForcedVisualParameterPreferences) {
                        ctx._scale0ForcedVisualParameterPreferences.fluxThreshold = v;
                    }
                    ctx.viewport.setFluxThreshold(v);
                    ctx.viewport.setFluxSliceThreshold?.(v);
                }, { needsUpload: true });
        });
    }

    const scenarioScaleSlider = getEl('flux-scenario-scale');
    const scenarioScaleVal = getEl('flux-scenario-scale-val');
    if (scenarioScaleSlider && scenarioScaleVal) {
        scenarioScaleSlider.addEventListener('input', () => {
            scheduleInput('scenario-scale', scenarioScaleSlider, scenarioScaleVal,
                v => v.toFixed(1), v => ctx.viewport.setScenarioScale(v));
        });
    }

    const latticeSpacingSlider = getEl('flux-lattice-spacing');
    const latticeSpacingVal = getEl('flux-lattice-spacing-val');
    if (latticeSpacingSlider && latticeSpacingVal) {
        latticeSpacingSlider.addEventListener('input', () => {
            scheduleInput('lattice-spacing', latticeSpacingSlider, latticeSpacingVal,
                v => v.toFixed(2), v => ctx.viewport.setFluxLatticeSpacing?.(v));
        });
    }

    const wireframeBrightnessSlider = getEl('wireframe-brightness');
    const wireframeBrightnessVal = getEl('wireframe-brightness-val');
    if (wireframeBrightnessSlider && wireframeBrightnessVal) {
        wireframeBrightnessSlider.addEventListener('input', () => {
            scheduleInput('wireframe-brightness', wireframeBrightnessSlider,
                wireframeBrightnessVal, v => v.toFixed(2),
                v => ctx.viewport.setWireframeBrightness?.(v));
        });
    }
}

function wireSelection(ctx) {
    const card = document.getElementById('sel-card');
    if (!card) return;
    const isScale0Active = () => !ctx.engineMode || ctx.engineMode === 'lattice';
    const L = () => Math.max(1, Math.round(Number(latticeN(ctx)) || 1));
    const clamp = (v, size = L()) => Math.max(0, Math.min(size - 1, Math.round(v)));
    let highlightActive = false;

    function getSelPos(size = L()) {
        return {
            x: clamp(Number.parseInt(getEl('sel-x')?.value, 10) || 0, size),
            y: clamp(Number.parseInt(getEl('sel-y')?.value, 10) || 0, size),
            z: clamp(Number.parseInt(getEl('sel-z')?.value, 10) || 0, size),
        };
    }

    function setSelDisplay(x, y, z, size = L()) {
        const max = String(size - 1);
        const values = { x: clamp(Number(x) || 0, size), y: clamp(Number(y) || 0, size), z: clamp(Number(z) || 0, size) };
        for (const axis of ['x', 'y', 'z']) {
            const input = getEl(`sel-${axis}`);
            if (!input) continue;
            if (input.max !== max) input.max = max;
            const value = String(values[axis]);
            if (input.value !== value) input.value = value;
        }
        return values;
    }

    function isAreaMode() { return getEl('sel-area-toggle')?.dataset.active === 'true'; }
    function getRadius() {
        const slider = getEl('sel-radius');
        const min = Number.parseInt(slider?.min, 10) || 1;
        const max = Number.parseInt(slider?.max, 10) || 10;
        const value = Math.max(min, Math.min(max, Number.parseInt(slider?.value, 10) || 2));
        if (slider && slider.value !== String(value)) slider.value = String(value);
        return value;
    }

    function applyHighlight({ voxel = true } = {}) {
        if (!isScale0Active()) return false;
        const { x, y, z } = getSelPos();
        highlightActive = true;
        if (voxel) ctx.viewport?.setVoxelHighlight?.(x, y, z, true);
        if (isAreaMode()) {
            ctx.viewport?.setAreaHighlight?.(x, y, z, getRadius(), true);
        } else {
            ctx.viewport?.setAreaHighlight?.(0, 0, 0, 1, false);
        }
        return true;
    }

    function fireSelect() {
        const { x, y, z } = getSelPos();
        applyHighlight();
        // Use the inspector's synchronous public selection transaction. The
        // previous dynamic import could resolve after a scale/scenario switch
        // and publish a stale private `_selectedPos` into the new owner.
        ctx.inspector?.selectLatticePosition?.({ x, y, z });
    }

    function syncSelectionBounds(size = L()) {
        const safeSize = Math.max(1, Math.round(Number(size) || L()));
        const bounded = setSelDisplay(
            getEl('sel-x')?.value,
            getEl('sel-y')?.value,
            getEl('sel-z')?.value,
            safeSize,
        );
        const selected = ctx.inspector?.getSelectedLatticePosition?.();
        if (selected && isScale0Active()) {
            const selectedBounded = {
                x: clamp(Number(selected.x) || 0, safeSize),
                y: clamp(Number(selected.y) || 0, safeSize),
                z: clamp(Number(selected.z) || 0, safeSize),
            };
            if (selected.x !== selectedBounded.x
                || selected.y !== selectedBounded.y
                || selected.z !== selectedBounded.z) {
                ctx.inspector?.selectLatticePosition?.(selectedBounded);
            }
        }
        if (highlightActive && isScale0Active()) applyHighlight();
        return bounded;
    }

    // Scenario loads and successful resizes call this once the live owner and
    // lattice size are authoritative. It is intentionally synchronous so the
    // card can never expose one-frame stale bounds.
    ctx.syncScale0SelectionBounds = syncSelectionBounds;
    syncSelectionBounds();

    // Coord stepper buttons (scoped to #sel-card to avoid clash with injection steppers)
    card.querySelectorAll('.sel-coord-step').forEach(btn => {
        btn.addEventListener('click', () => {
            const input = getEl(btn.dataset.for);
            if (!input) return;
            input.value = clamp((Number.parseInt(input.value, 10) || 0) + (Number.parseInt(btn.dataset.step, 10) || 0));
            applyHighlight();
        });
    });

    // Axis navigation buttons
    card.querySelectorAll('.sel-axis-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const input = getEl(`sel-${btn.dataset.axis}`);
            if (!input) return;
            input.value = clamp((Number.parseInt(input.value, 10) || 0) + (Number.parseInt(btn.dataset.dir, 10) || 0));
            applyHighlight();
        });
    });

    // Area toggle
    const areaToggle = getEl('sel-area-toggle');
    const areaControls = getEl('sel-area-controls');
    if (areaToggle && areaControls) {
        areaToggle.addEventListener('click', () => {
            const on = areaToggle.dataset.active !== 'true';
            areaToggle.dataset.active = String(on);
            setDisplayText(areaToggle, on ? '▪ Area' : '◻ Area');
            areaControls.hidden = !on;
            if (!on) {
                ctx.viewport?.setAreaHighlight?.(0, 0, 0, 1, false);
            }
            else applyHighlight();
        });
    }

    // Radius slider
    const radiusSlider = getEl('sel-radius');
    const radiusVal = getEl('sel-radius-val');
    if (radiusSlider && radiusVal) {
        const scheduleRadius = createLatestInputFrame(ctx, {
            canCommit: isScale0Active,
        });
        radiusSlider.addEventListener('input', () => {
            scheduleRadius(
                'selection-radius',
                radiusSlider,
                radiusVal,
                value => String(Math.round(value)),
                () => {
                    const radius = getRadius();
                    setDisplayText(radiusVal, String(radius));
                    if (isAreaMode()) applyHighlight();
                },
            );
        });
    }

    // SELECT button
    getEl('btn-select')?.addEventListener('click', fireSelect);

    // sel-x/y/z manual edits
    for (const axis of ['x', 'y', 'z']) {
        getEl(`sel-${axis}`)?.addEventListener('change', (e) => {
            e.target.value = clamp(Number.parseInt(e.target.value, 10) || 0);
            applyHighlight();
        });
    }

    // Receive click-to-select events from the inspector (set in lattice.js)
    document.addEventListener('ftd:voxel-selected', (e) => {
        if (!isScale0Active() || !e.detail) return;
        const values = [e.detail.x, e.detail.y, e.detail.z].map(Number);
        if (!values.every(Number.isFinite)) return;
        setSelDisplay(...values);
        // showLatticeInspector already moved the one-voxel box. Keep the area
        // overlay in the same transaction without a redundant voxel write.
        applyHighlight({ voxel: false });
    });
    document.addEventListener('ftd:voxel-selection-cleared', () => {
        highlightActive = false;
        ctx.viewport?.setAreaHighlight?.(0, 0, 0, 1, false);
    });
}

export function syncScale0ParticleDisplay(ctx) {
    if (!ctx?.viewport || (ctx.engineMode && ctx.engineMode !== 'lattice')) return false;
    const shapeSelect = getEl('particle-shape-select');
    // Optional-card composition tests and partial embeds legitimately mount
    // other Scale-0 cards without Particle Display.
    if (!shapeSelect) return false;
    const shape = Number.parseInt(shapeSelect?.value, 10);
    const safeShape = Number.isInteger(shape) && shape >= 0 && shape <= 7 ? shape : 0;
    if (shapeSelect && shapeSelect.value !== String(safeShape)) shapeSelect.value = String(safeShape);
    ctx.viewport.setParticleShape(safeShape);

    const sliders = [
        { id: 'particle-pos-size', valId: 'particle-pos-size-val', key: 'positive', fmt: 1, fallback: 14 },
        { id: 'particle-neg-size', valId: 'particle-neg-size-val', key: 'negative', fmt: 1, fallback: 10 },
        { id: 'particle-opacity', valId: 'particle-opacity-val', fn: v => ctx.viewport.setParticleOpacity(v), fmt: 2, fallback: 0.9 },
        { id: 'particle-glow', valId: 'particle-glow-val', fn: v => ctx.viewport.setParticleGlow(v), fmt: 2, fallback: 0.15 },
    ];
    const values = {};
    for (const item of sliders) {
        const slider = getEl(item.id);
        if (!slider) continue;
        const min = Number.parseFloat(slider.min);
        const max = Number.parseFloat(slider.max);
        let value = Number.parseFloat(slider.value);
        if (!Number.isFinite(value)) value = item.fallback;
        if (Number.isFinite(min)) value = Math.max(min, value);
        if (Number.isFinite(max)) value = Math.min(max, value);
        if (slider.value !== String(value)) slider.value = String(value);
        setDisplayText(getEl(item.valId), value.toFixed(item.fmt));
        if (item.key) values[item.key] = value;
        else item.fn(value);
    }
    if (typeof ctx.viewport.setParticleSizes === 'function') {
        ctx.viewport.setParticleSizes(values.positive, values.negative);
    } else {
        ctx.viewport.setPositiveSize(values.positive);
        ctx.viewport.setNegativeSize(values.negative);
    }
    return true;
}

function formatFlowPercent(value) {
    return `${Math.round(value * 100)}%`;
}

function updateFlowLineBudget(ctx, latticeSize = latticeN(ctx)) {
    const display = getEl('flow-line-budget');
    if (!display) return;
    const settings = getFlowLineSettings();
    const activeBridge = getActiveScale0Bridge(ctx, getScale0State()) ?? ctx?.bridge;
    const params = computeStreamlineParams(latticeSize, {
        inThreadWasm: !!activeBridge?.isWasm && !activeBridge?.isWorker,
        density: settings.density,
        length: settings.length,
    });
    const bSteps = Math.ceil(params.maxSteps * 1.5);
    setDisplayText(
        display,
        `L=${latticeSize} · ${params.maxSeeds} lines · ${params.maxSteps}/${bSteps} steps`,
    );
}

/** Replay persisted values after mount, scale re-entry, or lattice resize. */
export function syncScale0FlowLineControls(ctx, latticeSize = latticeN(ctx)) {
    const card = getEl('flow-lines-card');
    if (!card) return false;
    const settings = getFlowLineSettings();
    for (const key of Object.keys(DEFAULT_FLOW_LINE_SETTINGS)) {
        const slider = getEl(`flow-line-${key}`);
        if (!slider) continue;
        const value = settings[key];
        if (slider.value !== String(value)) slider.value = String(value);
        setDisplayText(getEl(`flow-line-${key}-val`), formatFlowPercent(value));
    }
    updateFlowLineBudget(ctx, latticeSize);
    if (!ctx.engineMode || ctx.engineMode === 'lattice') {
        ctx.viewport?.setFlowLineOpacity?.(settings.opacity);
    }
    return true;
}

function wireFlowLines(ctx) {
    const card = getEl('flow-lines-card');
    if (!card) return;
    const isScale0Active = () => !ctx.engineMode || ctx.engineMode === 'lattice';
    const scheduleInput = createLatestInputFrame(ctx, {
        generationAware: false,
        canCommit: isScale0Active,
        afterFlush: () => updateFlowLineBudget(ctx),
    });

    const commit = (key, slider, display) => {
        if (!isScale0Active()) return;
        const value = Number.parseFloat(slider.value);
        if (!Number.isFinite(value)) return;
        setDisplayText(display, formatFlowPercent(value));
        setFlowLineSetting(key, value);
        if (key === 'opacity') ctx.viewport?.setFlowLineOpacity?.(value);
        updateFlowLineBudget(ctx);
    };

    for (const key of Object.keys(DEFAULT_FLOW_LINE_SETTINGS)) {
        const slider = getEl(`flow-line-${key}`);
        const display = getEl(`flow-line-${key}-val`);
        if (!slider || !display) continue;
        slider.addEventListener('input', () => {
            scheduleInput(
                `flow-line-${key}`,
                slider,
                display,
                formatFlowPercent,
                value => {
                    setFlowLineSetting(key, value);
                    if (key === 'opacity') ctx.viewport?.setFlowLineOpacity?.(value);
                },
            );
        });
        // Commit synchronously at drag end so a same-turn scenario load/reload
        // cannot discard the user's final thumb position before the queued rAF.
        slider.addEventListener('change', () => commit(key, slider, display));
    }

    getEl('flow-line-reset')?.addEventListener('click', () => {
        if (!isScale0Active()) return;
        resetFlowLineSettings();
        syncScale0FlowLineControls(ctx);
    });

    ctx.syncScale0FlowLineControls = (size) => syncScale0FlowLineControls(ctx, size);
    syncScale0FlowLineControls(ctx);
}

function wireParticleDisplay(ctx) {
    // Particle presentation is user-owned across Scale-0 scenario loads, but a
    // queued job must not overwrite another scale's renderer state after exit.
    // mount() replays the retained DOM values when Scale 0 becomes active again.
    const scheduleInput = createLatestInputFrame(ctx, {
        generationAware: false,
        canCommit: () => !ctx.engineMode || ctx.engineMode === 'lattice',
        afterFlush: jobs => {
            if (!jobs.some(job => job.metadata?.particleSize)) return;
            const positive = Number.parseFloat(getEl('particle-pos-size')?.value);
            const negative = Number.parseFloat(getEl('particle-neg-size')?.value);
            if (!Number.isFinite(positive) || !Number.isFinite(negative)) return;
            if (typeof ctx.viewport.setParticleSizes === 'function') {
                ctx.viewport.setParticleSizes(positive, negative);
            } else {
                ctx.viewport.setPositiveSize(positive);
                ctx.viewport.setNegativeSize(negative);
            }
        },
    });
    const shapeSelect = getEl('particle-shape-select');
    if (shapeSelect) {
        shapeSelect.addEventListener('change', () => {
            if (ctx.engineMode && ctx.engineMode !== 'lattice') return;
            const shape = Number.parseInt(shapeSelect.value, 10);
            if (!Number.isInteger(shape) || shape < 0 || shape > 7) return;
            ctx.viewport.setParticleShape(shape);
        });
    }

    const sliders = [
        { id: 'particle-pos-size',  valId: 'particle-pos-size-val',  fn: () => {},                                fmt: 1, particleSize: true },
        { id: 'particle-neg-size',  valId: 'particle-neg-size-val',  fn: () => {},                                fmt: 1, particleSize: true },
        { id: 'particle-opacity',   valId: 'particle-opacity-val',   fn: v => ctx.viewport.setParticleOpacity(v),  fmt: 2 },
        { id: 'particle-glow',      valId: 'particle-glow-val',      fn: v => ctx.viewport.setParticleGlow(v),     fmt: 2 },
    ];
    for (const s of sliders) {
        const slider = getEl(s.id);
        const display = getEl(s.valId);
        if (!slider || !display) continue;
        slider.addEventListener('input', () => {
            scheduleInput(s.id, slider, display, v => v.toFixed(s.fmt), s.fn,
                s.particleSize ? { particleSize: true } : null);
        });
    }
    syncScale0ParticleDisplay(ctx);
}

/**
 * Wire all Scale 0 control-panel event handlers.
 *
 * @param {object} ctx — live-accessor context (ctx.bridge / ctx.viewport / ctx.clearCharts)
 * @param {object} api
 *   @param {Function} api.setLatticeNeedsUpload — marks the lattice dirty
 */
export function wireScale0Controls(ctx, api) {
    if (_wired) return;
    _wired = true;
    wirePhysicsToggles(ctx, api);
    wireInjection(ctx, api);
    wireParameterSliders(ctx);
    wireFieldActions(ctx, api);
    wireFluxVolume(ctx, api);
    wireFlowLines(ctx);
    wireParticleDisplay(ctx);
    wireSelection(ctx);
}
