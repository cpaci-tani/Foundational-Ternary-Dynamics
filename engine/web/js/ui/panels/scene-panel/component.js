/**
 * ScenePanelComponent — wires the Scene panel DOM to the SceneAdapter
 * and manages localStorage-backed persistence.
 *
 *   const panel = initScenePanel({ panelArea, viewport, backgroundManager });
 *
 * Contract: the component is scale-agnostic. The panel-registry gates
 * visibility to Scales 0-3 (where the shared Viewport is the renderer);
 * this component makes no scale-related decisions of its own.
 */

import { SceneAdapter } from './adapter.js';
import { getScenePanelTemplate } from './template.js';

const STORAGE_PREFIX = 'ftd.scene.';

const CONTROL_META = Object.freeze({
    // id -> { type, format(value): readoutText, setter on adapter }
    fov:               { type: 'number',  fmt: (v) => `${v.toFixed(0)}°`,    apply: 'setFov' },
    orbitRotateSpeed:  { type: 'number',  fmt: (v) => `${v.toFixed(2)}×`,    apply: 'setOrbitRotateSpeed' },
    orbitZoomSpeed:    { type: 'number',  fmt: (v) => `${v.toFixed(2)}×`,    apply: 'setOrbitZoomSpeed' },
    ambientIntensity:  { type: 'number',  fmt: (v) => v.toFixed(2),          apply: 'setAmbientIntensity' },
    ambientColor:      { type: 'color',   fmt: (v) => v,                     apply: 'setAmbientColor' },
    keyLightIntensity: { type: 'number',  fmt: (v) => v.toFixed(2),          apply: 'setKeyLightIntensity' },
    exposure:          { type: 'number',  fmt: (v) => v.toFixed(2),          apply: 'setExposure' },
    bloomEnabled:      { type: 'boolean', fmt: (v) => (v ? 'on' : 'off'),    apply: 'setBloomEnabled' },
    bloomStrength:     { type: 'number',  fmt: (v) => v.toFixed(2),          apply: 'setBloomStrength' },
    bloomThreshold:    { type: 'number',  fmt: (v) => v.toFixed(2),          apply: 'setBloomThreshold' },
    fogEnabled:        { type: 'boolean', fmt: (v) => (v ? 'on' : 'off'),    apply: null /* special: setFogEnabled with density */ },
    fogDensity:        { type: 'number',  fmt: (v) => v.toFixed(3),          apply: 'setFogDensity' },
    backgroundColor:   { type: 'color',   fmt: (v) => v,                     apply: 'setBackgroundColor' },
    hdriIntensity:     { type: 'number',  fmt: (v) => v.toFixed(2),          apply: 'setHdriIntensity' },
});

function coerce(raw, type) {
    if (type === 'boolean') return Boolean(raw);
    if (type === 'number') {
        const n = typeof raw === 'number' ? raw : parseFloat(raw);
        return Number.isFinite(n) ? n : 0;
    }
    return String(raw);
}

function readFromStorage(id, fallback, type) {
    try {
        const raw = localStorage.getItem(STORAGE_PREFIX + id);
        if (raw == null) return fallback;
        if (type === 'boolean') return raw === '1' || raw === 'true';
        if (type === 'number') return coerce(raw, 'number');
        return raw;
    } catch {
        return fallback;
    }
}

function writeToStorage(id, value, type) {
    try {
        const serialised = type === 'boolean' ? (value ? '1' : '0') : String(value);
        localStorage.setItem(STORAGE_PREFIX + id, serialised);
    } catch {
        /* ignore */
    }
}

export class ScenePanelComponent {
    constructor({ panelArea, viewport, backgroundManager = null } = {}) {
        if (!panelArea) throw new Error('ScenePanelComponent: panelArea is required');
        if (!viewport) throw new Error('ScenePanelComponent: viewport is required');
        this.panelArea = panelArea;
        this.adapter = new SceneAdapter({ viewport, backgroundManager });
        this.values = { ...SceneAdapter.DEFAULTS };
        this.panelEl = null;
    }

    init() {
        this._ensureMarkup();
        this._loadFromStorage();
        this._bindEvents();
        this._applyAllToViewport();
        this._refreshAllReadouts();
        this._refreshConditionalVisibility();
        return this;
    }

    _ensureMarkup() {
        // index.html ships an empty <div class="panel" id="panel-scene">
        // stub so panel-registry validation can find us and
        // WorkspaceTabs can render the tab. We fill that stub with the
        // scene-panel body here. If the stub is absent (unit-test /
        // standalone mount path) we create one and append.
        let existing = this.panelArea.querySelector('#panel-scene');
        if (!existing) {
            existing = document.createElement('div');
            existing.className = 'panel';
            existing.id = 'panel-scene';
            this.panelArea.appendChild(existing);
        }
        existing.classList.add('scene-panel');
        existing.innerHTML = getScenePanelTemplate().trim();
        this.panelEl = existing;
    }

    _loadFromStorage() {
        for (const [id, meta] of Object.entries(CONTROL_META)) {
            this.values[id] = readFromStorage(id, SceneAdapter.DEFAULTS[id], meta.type);
        }
    }

    _bindEvents() {
        // Every slider / color / checkbox has data-scene-control="<id>".
        const inputs = this.panelEl.querySelectorAll('[data-scene-control]');
        inputs.forEach((el) => {
            const id = el.dataset.sceneControl;
            const meta = CONTROL_META[id];
            if (!meta) return;

            // Hydrate DOM value from stored state.
            if (meta.type === 'boolean') {
                el.checked = this.values[id];
            } else {
                el.value = this.values[id];
            }

            const eventName = meta.type === 'color' ? 'input' : 'change';
            const liveEvent = meta.type === 'boolean' ? 'change' : 'input';
            const handler = () => {
                const raw = meta.type === 'boolean' ? el.checked : el.value;
                const value = coerce(raw, meta.type);
                this.values[id] = value;
                writeToStorage(id, value, meta.type);
                this._applyOne(id, value);
                this._refreshReadout(id);
                if (id === 'bloomEnabled' || id === 'fogEnabled') {
                    this._refreshConditionalVisibility();
                }
            };
            el.addEventListener(liveEvent, handler);
            if (liveEvent !== eventName) el.addEventListener(eventName, handler);
        });

        // Reset button
        const resetBtn = this.panelEl.querySelector('#scene-reset-defaults');
        if (resetBtn) resetBtn.addEventListener('click', () => this._resetDefaults());

        // Re-evaluate conditional visibility whenever #bg-select changes,
        // since background-color and hdri-intensity depend on it.
        const bgSel = document.getElementById('bg-select');
        if (bgSel) {
            bgSel.addEventListener('change', () => this._refreshConditionalVisibility());
        }
    }

    _applyAllToViewport() {
        for (const id of Object.keys(CONTROL_META)) {
            this._applyOne(id, this.values[id]);
        }
    }

    _applyOne(id, value) {
        if (id === 'fogEnabled') {
            // Fog toggle needs density + color context; delegate to setFogEnabled.
            this.adapter.setFogEnabled(value, this.values.fogDensity);
            return;
        }
        const meta = CONTROL_META[id];
        if (!meta || !meta.apply) return;
        const fn = this.adapter[meta.apply];
        if (typeof fn === 'function') fn.call(this.adapter, value);
    }

    _refreshAllReadouts() {
        for (const id of Object.keys(CONTROL_META)) this._refreshReadout(id);
    }

    _refreshReadout(id) {
        const meta = CONTROL_META[id];
        if (!meta) return;
        const el = this.panelEl.querySelector(`[data-scene-readout="${id}"]`);
        if (!el) return;
        el.textContent = meta.fmt(this.values[id]);
    }

    _refreshConditionalVisibility() {
        // Controls tagged data-scene-dependent become inert when their
        // prerequisite is not satisfied. We use a CSS class rather than
        // hidden so layout stays stable.
        const rows = this.panelEl.querySelectorAll('[data-scene-dependent]');
        const bloomOn = Boolean(this.values.bloomEnabled);
        const fogOn = Boolean(this.values.fogEnabled);
        const hdriActive = this.adapter.isHdriActive();
        const bgIsNone = this.adapter.isBackgroundNone();
        rows.forEach((row) => {
            const dep = row.dataset.sceneDependent;
            let active = true;
            if (dep === 'bloomEnabled') active = bloomOn;
            else if (dep === 'fogEnabled') active = fogOn;
            else if (dep === 'hdri') active = hdriActive;
            else if (dep === 'backgroundNone') active = bgIsNone;
            row.classList.toggle('scene-control--disabled', !active);
            const input = row.querySelector('[data-scene-control]');
            if (input) input.disabled = !active;
        });
        // Toggle the two hint paragraphs' visibility based on current background selection.
        const bgNoneHint = this.panelEl.querySelector('.scene-hint--bg-none');
        const hdriHint = this.panelEl.querySelector('.scene-hint--hdri');
        if (bgNoneHint) bgNoneHint.hidden = bgIsNone;
        if (hdriHint) hdriHint.hidden = hdriActive;
    }

    _resetDefaults() {
        try {
            for (const id of Object.keys(CONTROL_META)) {
                localStorage.removeItem(STORAGE_PREFIX + id);
            }
        } catch { /* ignore */ }
        this.values = { ...SceneAdapter.DEFAULTS };
        // Update DOM inputs from defaults.
        for (const [id, meta] of Object.entries(CONTROL_META)) {
            const el = this.panelEl.querySelector(`[data-scene-control="${id}"]`);
            if (!el) continue;
            if (meta.type === 'boolean') el.checked = this.values[id];
            else el.value = this.values[id];
        }
        this._applyAllToViewport();
        this._refreshAllReadouts();
        this._refreshConditionalVisibility();
    }
}

export function initScenePanel({ panelArea, viewport, backgroundManager = null } = {}) {
    const component = new ScenePanelComponent({ panelArea, viewport, backgroundManager });
    component.init();
    return component;
}
