/**
 * Scale 0 Controls Panel Wiring
 *
 * Binds event listeners for every control card mounted by Scale0ControlsComponent:
 *   - Physics toggles card (all 18 toggles from SCALE0_TOGGLES)
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
import { getEl, markScenarioOverrideRows } from '../dom.js';
import { updateScenarioMetadata } from '../bindings.js';
import { getFluxMock } from '../../controller.js';
import { getScale0State, getActiveLatticeSize, getActiveScale0Bridge } from '../../state/store.js';

let _wired = false;

/**
 * When a WASM worker owns Scale-0 (state.fluxMock / useFluxMock — legacy names
 * for the off-thread WasmBridgeProxy), user-driven controls must mirror writes
 * to both the in-thread bridge and the worker proxy.
 */
function latticeN(ctx) {
    return getActiveLatticeSize(ctx, getScale0State());
}

/** Mirror a harness write across in-thread WASM + worker proxy. */
function dualHarness(ctx, fn) {
    fn(getPhysicsHarness(ctx.bridge));
    const worker = getFluxMock();
    if (worker) fn(getPhysicsHarness(worker));
}

function wirePhysicsToggles(ctx, api) {
    const currentScenarioId = () => getScale0State().currentScenarioId || 'flux-pulse';
    const profileIsModified = () => getScale0ScenarioToggleProfile(currentScenarioId())
        .some(([, expected, elId]) => {
            const el = getEl(elId);
            return !!el && el.checked !== expected;
        });
    const renderProfileStatus = (modified = profileIsModified()) => {
        const warning = getEl('physics-profile-warning');
        if (warning) warning.hidden = !modified;
        updateScenarioMetadata(currentScenarioId(), { profileModified: modified });
    };

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
            ctx.bridge.setToggle(toggleKey, el.checked);
            getFluxMock()?.capabilities?.scale0?.setToggle(toggleKey, el.checked);
            const row = el.closest('.toggle-row');
            if (row) row.classList.remove('scenario-override');
            renderProfileStatus();
        });
    }

    // Advanced / research toggles (SCALE0_ADVANCED_TOGGLES): same setToggle
    // wiring, but deliberately NOT profile-status-tracked and NOT scenario-reset
    // — they are absent from SCALE0_TOGGLES, so scenario-loader.js's whitelist
    // reset leaves them alone and they persist across scenario loads (their
    // "owned by the user" semantics). Requirement gating is documented in each
    // label's tooltip (e.g. Triad Binding requires Color Forces ON).
    for (const [toggleKey, , elId] of SCALE0_ADVANCED_TOGGLES) {
        const el = getEl(elId);
        if (!el) continue;
        el.addEventListener('change', () => {
            ctx.bridge.setToggle(toggleKey, el.checked);
            getFluxMock()?.capabilities?.scale0?.setToggle(toggleKey, el.checked);
        });
    }

    // Re-run the canonical scenario load so C++ configure_* isolation wins.
    // Toggle-only restore from partial tables previously re-armed disabled terms.
    const resetBtn = getEl('btn-reset-physics-toggles');
    if (resetBtn) {
        resetBtn.addEventListener('click', () => {
            const id = currentScenarioId();
            if (typeof api?.loadScenario === 'function') {
                api.loadScenario(ctx, id);
            } else {
                console.error('[Scale0] restore profile: api.loadScenario missing');
            }
            markScenarioOverrideRows(SCALE0_TOGGLES);
            renderProfileStatus(false);
            resetBtn.classList.add('ctrl-reset-flash');
            setTimeout(() => resetBtn.classList.remove('ctrl-reset-flash'), 320);
        });
    }
}

function wireInjection(ctx, api) {
    let injState = 1;
    const injPos = getEl('inj-state-pos');
    const injNeg = getEl('inj-state-neg');
    if (injPos && injNeg) {
        injPos.addEventListener('click', () => {
            injState = 1;
            injPos.classList.add('active');
            injNeg.classList.remove('active');
        });
        injNeg.addEventListener('click', () => {
            injState = -1;
            injNeg.classList.add('active');
            injPos.classList.remove('active');
        });
    }

    function getInjPos() {
        return {
            x: parseInt(getEl('inj-x')?.value, 10) || 0,
            y: parseInt(getEl('inj-y')?.value, 10) || 0,
            z: parseInt(getEl('inj-z')?.value, 10) || 0,
            state: injState,
        };
    }

    function clampCoord(value) {
        const L = latticeN(ctx);
        if (!Number.isFinite(value)) return 0;
        return Math.max(0, Math.min(L - 1, Math.round(value)));
    }

    function setCoordMax() {
        const L = latticeN(ctx);
        for (const id of ['inj-x', 'inj-y', 'inj-z']) {
            const el = getEl(id);
            if (el) el.max = String(L - 1);
        }
    }

    // Custom stepper buttons (+/- for each coord input)
    for (const btn of document.querySelectorAll('#panel-controls .coord-step')) {
        btn.addEventListener('click', () => {
            const targetId = btn.dataset.for;
            const step = parseInt(btn.dataset.step, 10) || 0;
            const input = getEl(targetId);
            if (!input) return;
            const current = parseInt(input.value, 10) || 0;
            input.value = clampCoord(current + step);
            setCoordMax();
        });
    }

    // Keep user-typed values within lattice bounds
    for (const id of ['inj-x', 'inj-y', 'inj-z']) {
        getEl(id)?.addEventListener('change', (e) => {
            e.target.value = clampCoord(parseInt(e.target.value, 10));
        });
    }

    getEl('btn-center')?.addEventListener('click', () => {
        setCoordMax();
        const half = Math.floor(latticeN(ctx) / 2);
        const x = getEl('inj-x'); if (x) x.value = half;
        const y = getEl('inj-y'); if (y) y.value = half;
        const z = getEl('inj-z'); if (z) z.value = half;
    });

    function dualInject(action) {
        dualHarness(ctx, action);
    }

    getEl('btn-random')?.addEventListener('click', () => {
        setCoordMax();
        const L = latticeN(ctx);
        const rand = () => 2 + Math.floor(Math.random() * (L - 4));
        const x = getEl('inj-x'); if (x) x.value = rand();
        const y = getEl('inj-y'); if (y) y.value = rand();
        const z = getEl('inj-z'); if (z) z.value = rand();
        const { x: px, y: py, z: pz, state } = getInjPos();
        dualInject((h) => h.injectWavepacket(px, py, pz, state));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject')?.addEventListener('click', () => {
        const { x, y, z, state } = getInjPos();
        // Wavepacket injection: bare point particles have zero flux and are
        // immediately evaporated by the neighbourhood-energy check. A Gaussian
        // flux envelope lets the self-field stabilise.
        dualInject((h) => h.injectWavepacket(x, y, z, state));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject-wave')?.addEventListener('click', () => {
        const { x, y, z, state } = getInjPos();
        dualInject((h) => h.injectWavepacket(x, y, z, state));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject-flux')?.addEventListener('click', () => {
        const { x, y, z } = getInjPos();
        const active = getActiveScale0Bridge(ctx, getScale0State()) ?? ctx.bridge;
        const kb = getPhysicsHarness(active).getParam?.('kb') || K_B;
        dualInject((h) => h.injectFlux(x, y, z, kb * 0.8, 0, 0));
        api.setLatticeNeedsUpload();
    });

    getEl('btn-inject-pair')?.addEventListener('click', () => {
        const { x, y, z } = getInjPos();
        const active = getActiveScale0Bridge(ctx, getScale0State()) ?? ctx.bridge;
        const kb = getPhysicsHarness(active).getParam?.('kb') || K_B;
        dualInject((h) => h.createEntangledPair(x, y, z, kb, 0, 0));
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
        if (ctx.bridge?.isNativeGPU) {
            slider.step = 'any';
            slider.disabled = true;
            slider.setAttribute('aria-readonly', 'true');
            slider.title = `${s.param} is a native engine constant supplied by the acknowledged scenario profile.`;
            slider.classList.add('ctrl-slider-disabled');
            const row = slider.closest('.pe-ctrl-row');
            row?.classList.add('ctrl-native-readonly');
            if (row && !row.querySelector('.ctrl-native-fixed')) {
                const badge = document.createElement('span');
                badge.className = 'ctrl-native-fixed';
                badge.textContent = 'fixed';
                badge.title = 'Read-only native engine constant';
                row.appendChild(badge);
            }
            continue;
        }
        slider.addEventListener('input', () => {
            const val = parseFloat(slider.value);
            display.textContent = val.toFixed(s.fmt);
            getFluxMock()?.setParam?.(s.param, val);
        });
        // Only mark read-only if we have NO writable target (pure-WASM
        // scenario with no fluxMock — rare, e.g. emergent-spectrum or
        // a future custom bridge). The disabled state is recomputed on
        // each scenario load via the syncComboSliders helper in the
        // scenario-loader, so the visual state stays current.
        if (ctx.bridge.isWasm && !getFluxMock()) {
            slider.disabled = true;
            slider.title = 'Read-only in WASM mode (no fluxMock active)';
            slider.classList.add('ctrl-slider-disabled');
        }
    }
}

function wireFieldActions(ctx, api) {
    getEl('btn-clear-field')?.addEventListener('click', () => {
        const L = latticeN(ctx);
        dualHarness(ctx, (h) => {
            if (typeof h.clearField === 'function') {
                h.clearField();
            } else {
                h.reset();
            }
        });
        ctx.viewport.setLatticeSize(L);
        ctx.clearCharts?.();
        api.setLatticeNeedsUpload();
    });

    getEl('btn-random-flux')?.addEventListener('click', () => {
        dualHarness(ctx, (h) => {
            h.seedRandomFlux?.();
        });
        api.setLatticeNeedsUpload();
    });
}

function wireFluxVolume(ctx, api) {
    const shapeSelect = getEl('flux-shape-select');
    if (shapeSelect) {
        shapeSelect.addEventListener('change', () => {
            const shape = parseInt(shapeSelect.value, 10);
            ctx.viewport.setFluxShape(shape);
            ctx.viewport.setFluxSliceShape?.(shape);
        });
    }

    const opacitySlider = getEl('flux-opacity');
    const opacityVal = getEl('flux-opacity-val');
    if (opacitySlider && opacityVal) {
        opacitySlider.addEventListener('input', () => {
            const v = parseFloat(opacitySlider.value);
            if (ctx._scale0ForcedVisualParameterPreferences
                && 'fluxOpacity' in ctx._scale0ForcedVisualParameterPreferences) {
                ctx._scale0ForcedVisualParameterPreferences.fluxOpacity = v;
            }
            opacityVal.textContent = v.toFixed(2);
            ctx.viewport.setFluxOpacity(v);
            ctx.viewport.setFluxSliceOpacity?.(v);
        });
    }

    const scaleSlider = getEl('flux-point-scale');
    const scaleVal = getEl('flux-point-scale-val');
    if (scaleSlider && scaleVal) {
        scaleSlider.addEventListener('input', () => {
            const v = parseFloat(scaleSlider.value);
            if (ctx._scale0ForcedVisualParameterPreferences
                && 'fluxPointScale' in ctx._scale0ForcedVisualParameterPreferences) {
                ctx._scale0ForcedVisualParameterPreferences.fluxPointScale = v;
            }
            scaleVal.textContent = v.toFixed(1);
            ctx.viewport.setFluxPointScale(v);
            ctx.viewport.setFluxSlicePointScale?.(v);
            api.setLatticeNeedsUpload();
        });
    }

    const threshSlider = getEl('flux-threshold');
    const threshVal = getEl('flux-threshold-val');
    if (threshSlider && threshVal) {
        threshSlider.addEventListener('input', () => {
            const v = parseFloat(threshSlider.value);
            if (ctx._scale0ForcedVisualParameterPreferences
                && 'fluxThreshold' in ctx._scale0ForcedVisualParameterPreferences) {
                ctx._scale0ForcedVisualParameterPreferences.fluxThreshold = v;
            }
            threshVal.textContent = v < 0.001 ? v.toFixed(4) : v.toFixed(3);
            ctx.viewport.setFluxThreshold(v);
            ctx.viewport.setFluxSliceThreshold?.(v);
            api.setLatticeNeedsUpload();
        });
    }

    const scenarioScaleSlider = getEl('flux-scenario-scale');
    const scenarioScaleVal = getEl('flux-scenario-scale-val');
    if (scenarioScaleSlider && scenarioScaleVal) {
        scenarioScaleSlider.addEventListener('input', () => {
            const v = parseFloat(scenarioScaleSlider.value);
            scenarioScaleVal.textContent = v.toFixed(1);
            ctx.viewport.setScenarioScale(v);
        });
    }

    const latticeSpacingSlider = getEl('flux-lattice-spacing');
    const latticeSpacingVal = getEl('flux-lattice-spacing-val');
    if (latticeSpacingSlider && latticeSpacingVal) {
        latticeSpacingSlider.addEventListener('input', () => {
            const v = parseFloat(latticeSpacingSlider.value);
            latticeSpacingVal.textContent = v.toFixed(2);
            ctx.viewport.setFluxLatticeSpacing?.(v);
        });
    }

    const wireframeBrightnessSlider = getEl('wireframe-brightness');
    const wireframeBrightnessVal = getEl('wireframe-brightness-val');
    if (wireframeBrightnessSlider && wireframeBrightnessVal) {
        wireframeBrightnessSlider.addEventListener('input', () => {
            const v = parseFloat(wireframeBrightnessSlider.value);
            wireframeBrightnessVal.textContent = v.toFixed(2);
            ctx.viewport.setWireframeBrightness?.(v);
        });
    }
}

function wireSelection(ctx) {
    const L = () => latticeN(ctx);
    const clamp = v => Math.max(0, Math.min(L() - 1, Math.round(v)));

    function getSelPos() {
        return {
            x: clamp(parseInt(getEl('sel-x')?.value) || 0),
            y: clamp(parseInt(getEl('sel-y')?.value) || 0),
            z: clamp(parseInt(getEl('sel-z')?.value) || 0),
        };
    }

    function setSelDisplay(x, y, z) {
        const L2 = L();
        const ex = getEl('sel-x'); if (ex) { ex.max = L2 - 1; ex.value = x; }
        const ey = getEl('sel-y'); if (ey) { ey.max = L2 - 1; ey.value = y; }
        const ez = getEl('sel-z'); if (ez) { ez.max = L2 - 1; ez.value = z; }
    }

    function isAreaMode() { return getEl('sel-area-toggle')?.dataset.active === 'true'; }
    function getRadius()   { return parseInt(getEl('sel-radius')?.value) || 2; }

    function applyHighlight() {
        const { x, y, z } = getSelPos();
        ctx.viewport?.setVoxelHighlight?.(x, y, z, true);
        if (isAreaMode()) {
            ctx.viewport?.setAreaHighlight?.(x, y, z, getRadius(), true);
        } else {
            ctx.viewport?.setAreaHighlight?.(0, 0, 0, 1, false);
        }
    }

    function fireSelect() {
        const { x, y, z } = getSelPos();
        applyHighlight();
        // Sync inspector so the data panel updates
        const insp = ctx.inspector;
        if (insp) {
            insp._selectedPos = { x, y, z };
            import('../../../../inspector/scales/lattice.js')
                .then(m => m.showLatticeInspector(insp))
                .catch(() => {});
        }
    }

    // Coord stepper buttons (scoped to #sel-card to avoid clash with injection steppers)
    const card = document.getElementById('sel-card');
    if (card) {
        card.querySelectorAll('.sel-coord-step').forEach(btn => {
            btn.addEventListener('click', () => {
                const input = getEl(btn.dataset.for);
                if (!input) return;
                input.value = clamp((parseInt(input.value) || 0) + (parseInt(btn.dataset.step) || 0));
                applyHighlight();
            });
        });
    }

    // Axis navigation buttons
    document.querySelectorAll('.sel-axis-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            const input = getEl(`sel-${btn.dataset.axis}`);
            if (!input) return;
            input.value = clamp((parseInt(input.value) || 0) + (parseInt(btn.dataset.dir) || 0));
            applyHighlight();
        });
    });

    // Area toggle
    const areaToggle = getEl('sel-area-toggle');
    const areaControls = getEl('sel-area-controls');
    if (areaToggle && areaControls) {
        areaToggle.addEventListener('click', () => {
            const on = areaToggle.dataset.active !== 'true';
            areaToggle.dataset.active = on;
            areaToggle.textContent = on ? '▪ Area' : '◻ Area';
            areaToggle.style.background = on ? 'rgba(56,189,248,0.2)' : '';
            areaToggle.style.borderColor = on ? '#38bdf8' : '';
            areaControls.style.display = on ? '' : 'none';
            if (!on) ctx.viewport?.setAreaHighlight?.(0, 0, 0, 1, false);
            else applyHighlight();
        });
    }

    // Radius slider
    const radiusSlider = getEl('sel-radius');
    const radiusVal = getEl('sel-radius-val');
    if (radiusSlider && radiusVal) {
        radiusSlider.addEventListener('input', () => {
            radiusVal.textContent = radiusSlider.value;
            if (isAreaMode()) applyHighlight();
        });
    }

    // SELECT button
    getEl('btn-select')?.addEventListener('click', fireSelect);

    // sel-x/y/z manual edits
    for (const axis of ['x', 'y', 'z']) {
        getEl(`sel-${axis}`)?.addEventListener('change', (e) => {
            e.target.value = clamp(parseInt(e.target.value) || 0);
            applyHighlight();
        });
    }

    // Receive click-to-select events from the inspector (set in lattice.js)
    document.addEventListener('ftd:voxel-selected', (e) => {
        setSelDisplay(e.detail.x, e.detail.y, e.detail.z);
    });
}

function wireParticleDisplay(ctx) {
    const shapeSelect = getEl('particle-shape-select');
    if (shapeSelect) {
        shapeSelect.addEventListener('change', () => {
            ctx.viewport.setParticleShape(parseInt(shapeSelect.value, 10));
        });
    }

    const sliders = [
        { id: 'particle-pos-size',  valId: 'particle-pos-size-val',  fn: v => ctx.viewport.setPositiveSize(v),     fmt: 1 },
        { id: 'particle-neg-size',  valId: 'particle-neg-size-val',  fn: v => ctx.viewport.setNegativeSize(v),     fmt: 1 },
        { id: 'particle-opacity',   valId: 'particle-opacity-val',   fn: v => ctx.viewport.setParticleOpacity(v),  fmt: 2 },
        { id: 'particle-glow',      valId: 'particle-glow-val',      fn: v => ctx.viewport.setParticleGlow(v),     fmt: 2 },
    ];
    for (const s of sliders) {
        const slider = getEl(s.id);
        const display = getEl(s.valId);
        if (!slider || !display) continue;
        slider.addEventListener('input', () => {
            const v = parseFloat(slider.value);
            display.textContent = v.toFixed(s.fmt);
            s.fn(v);
        });
    }
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
    wireParticleDisplay(ctx);
    wireSelection(ctx);
}
