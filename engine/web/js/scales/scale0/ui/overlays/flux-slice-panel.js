/**
 * Scale 0 — Live Multi-Field Flux Slice Panel
 *
 * The flagship |J| row is visible by default; every other field mirrors its
 * 3D visualization toggle until the user explicitly enables that row. This
 * keeps the scientific panel useful on first open without silently scheduling
 * every raw sampler (and every derived dependency) over the native socket.
 * The per-row chip can force a row on/off independently of the 3D viewport.
 *
 * Supported fields (FIELD_DRIVERS, 27 rows):
 *   - 1 flagship dense volume slice: |J| via bridge.getFluxSlice(axis, mid).
 *   - 14 raw sampler-kind rows (one per SCALE0_SAMPLER_METHODS entry,
 *     bridge-contract.js): |E|, |B|, |S|, ∇·J, |J| (sparse), |ω|,
 *     helicity, kretschmann, latency, fisher, coherence, |∇×J|, s
 *     (ternary state), Gauss residual r.
 *   - 3 force-field rows: EM, gravity, strong (bridge.get{EM,Gravity,
 *     Strong}ForceField), color-matched to the 3D Forces column palette.
 *   - 9 derived Tier-1 overlay rows, each reusing the exact exported
 *     compute*Frame function from runtime/overlay-frames.js verbatim
 *     (no reimplemented formulas): |ψ|², phase φ, ℒ(x), entropy density,
 *     Φ potential, EM energy u, P_E, P_B, event horizon.
 *
 * All raw-kind + derived rows are fed by ONE shared per-frame sample
 * cache (_buildFrameSampleCache), populated only with the kinds/types
 * that currently-visible rows actually need — see that method's doc
 * comment. Derived rows read the shared cache plus a panel-owned,
 * per-driver scratch object (this._scratch[key]) that is NEVER the real
 * Scale-0 controller state.
 *
 * Visibility semantics:
 *   - Each row's visibility = (override === 'on') ? true
 *                           : (override === 'off') ? false
 *                           : !!mirroredFlags[vizFlagKey]
 *   - Every field defaults to override='on' (always visible) regardless
 *     of vizFlagKey or the 3D panel's toggle state. Clicking a row's chip
 *     cycles on -> off -> mirror -> on, so a user who wants the old
 *     "follow the 3D panel" behavior for one field can still opt into it
 *     per-row; nothing defaults to mirror/hidden anymore.
 *   - "Show all" header link forces every override back to 'on'.
 *   - Scenario change (simTick decreasing) also resets every override to
 *     'on' and resets per-field rolling autoscale, so a new scenario
 *     always opens with everything visible.
 *
 * Performance: the panel is self-driven at a bounded cadence and fetches only
 * the rows actually visible at that instant. Rows that share an underlying
 * quantity (e.g. eField feeding |E|, emEnergy, ePressure, and ℒ(x)) still
 * fetch it once per sweep. "Show all" remains available as an explicit
 * diagnostic action, rather than becoming background work every scenario
 * inherits merely by opening this tab. Scenario 1 (`empty`) is explicitly
 * inapplicable: its rows/canvases are detached and its sampler/rAF ownership
 * is released rather than rendering black tiles as putative vacuum evidence.
 */

import { ScenarioApplicabilityBinding } from '../../../../ui/utils/scenario-applicability-binding.js';
import { buildFluxSliceSampleCache } from './flux-slice-sample-source.js';

import { rampViridis } from '../../../../viewport/color-ramps.js';
import {
    getFieldStateSnapshot,
    getScale0State,
    isScale0AuthoritativeGenerationReady,
    subscribeScale0Qualification,
    resolveActiveScale0BridgeFromWindow,
} from '../../state/store.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import {
    DEFAULT_CANVAS_PX,
    DENSE_CANVAS_PX,
    FLOOR_FRAC,
    DENSE_THRESHOLD,
    FIELD_DRIVERS,
    DRIVER_BY_KEY,
    DEFAULT_FIELD_OVERRIDE,
    axisIndex,
} from './flux-slice-helpers.js';

/**
 * Keep every large-L owner interactive. Even when a worker produces sampler
 * frames, converting and repainting as many as 81 N² heatmaps still runs on
 * the browser main thread; direct WASM additionally produces dense slices
 * synchronously. Small lattices retain the configured cadence.
 */
export function effectiveFluxSliceUpdateEvery(bridge, configured = 2) {
    const base = Math.max(1, Math.trunc(Number(configured) || 1));
    const N = Math.max(0, Math.trunc(Number(bridge.latticeSize) || 0));
    // The panel's single self-drive loop runs at 24 Hz. Three to six visual
    // samples per second is ample for a heatmap instrument at large L.
    const floor = bridge?.isNativeGPU
        ? (N > 96 ? 8 : (N > 64 ? 6 : (N > 48 ? 5 : 4)))
        : (N > 96 ? 8 : (N > 64 ? 6 : (N > 48 ? 4 : base)));
    return Math.max(base, floor);
}

const FLUX_SLICE_DRIVER_HZ = 24;
const EMPTY_SCENARIO_ID = 'empty';

function setTextIfChanged(element, text) {
    if (element && element.textContent !== text) element.textContent = text;
}

function setTitleIfChanged(element, title) {
    if (element && element.title !== title) element.title = title;
}


// ── FluxSlicePanel class ────────────────────────────────────────────

export class FluxSlicePanel {
    /**
     * @param {object} opts
     * @param {() => any} opts.getBridge   - returns the live bridge
     * @param {number} [opts.canvasPx]     - per-canvas size in CSS pixels (default 220)
     * @param {number} [opts.updateEvery]  - sample/render every Nth frame (default 2)
     * @param {boolean} [opts.dockMode]    - when true, render in side-panel dock
     *                                        layout: no toggle chip, no close X,
     *                                        no display:none gate, ResizeObserver
     *                                        drives canvas size; expand button
     *                                        toggles a full-size modal overlay.
     */
    constructor({ getBridge, canvasPx = DEFAULT_CANVAS_PX, updateEvery = 2, dockMode = false } = {}) {
        this.getBridge = getBridge;
        this.canvasPx = canvasPx | 0;
        this.updateEvery = Math.max(1, updateEvery | 0);
        this._dockMode = !!dockMode;
        this._canvasPx = this.canvasPx;       // per-frame target; updated by ResizeObserver in dock mode
        this._expanded = false;
        this._dockHost = null;                // dock container element
        this._expandModal = null;             // active expand-modal element when expanded
        this._resizeObs = null;

        this.visible = false;
        this.frameCount = 0;
        this._lastN = 0;
        this._lastSimTick = 0;
        // One self-driven loop owns this panel. It deliberately does not share
        // the controller render loop: driving from both paths doubled field
        // requests and canvas paints for every visible frame.
        this._lastSelfTickMs = 0;

        this._panel = null;
        this._chip = null;
        this._rowsContainer = null;
        this._resetMirrorBtn = null;
        this._rowFragment = null;
        this._emptyMessage = null;
        this._emptyInapplicable = false;
        this._scenarioBinding = new ScenarioApplicabilityBinding({
            getCurrentScenarioId: () => getScale0State().currentScenarioId,
            onIntent: (scenarioId) => this._handleScenarioIntent(scenarioId),
        });

        // Per-axis (xy/xz/yz) visibility — applies globally across all rows.
        this._axisVisible = { xy: true, xz: true, yz: true };

        // Per-field override map. 'on' / 'off' force; null = follow the 3D
        // viz panel's toggle instead. Every field defaults to 'on' so
        // opening the panel always shows every slice, independent of the 3D
        // panel's toggle state — a user can still force an individual row
        // off (or back to null/mirror) via its chip — EXCEPT the 3 force
        // rows, which default to null/mirror (see DEFAULT_FIELD_OVERRIDE's
        // doc comment: force-field sampling is expensive enough that it
        // must be opt-in, not silently requested the moment the panel
        // opens). Built from FIELD_DRIVERS so the 27-entry registry is the
        // single source of truth — never hand-enumerated a second time here.
        this._fieldOverride = { ...DEFAULT_FIELD_OVERRIDE };

        // Snapshot of viz panel flags, refreshed once per frame.
        this._mirroredFlags = {};

        // Per-field rolling autoscale max — each field gets its own decay
        // because magnitudes differ by orders.
        this._fieldGlobalMax = Object.fromEntries(FIELD_DRIVERS.map(d => [d.key, 0]));
        this._maxDecay = 0.985;

        // Per-driver, panel-owned scratch object (one plain {} per key) for
        // the derived rows' overlay-frames.js compute functions (their own
        // internal buffers: ensureTier1Buffers, per-field decayingMax, the
        // phase row's dualLVecs/dualRVecs, …). NEVER the real Scale-0
        // controller state — a fresh object per driver key, owned solely by
        // this panel instance, so the panel's own sampling/scratch cannot
        // interfere with the 3D overlay's real state (or vice versa).
        this._scratch = Object.fromEntries(FIELD_DRIVERS.map(d => [d.key, {}]));

        // Per-field × per-axis slot bookkeeping. Filled in init().
        // Shape: { [fieldKey]: { row, label, chip, slots: { xy, xz, yz } } }
        this._fields = {};
    }

    static get AXES() { return ['xy', 'xz', 'yz']; }

    // ── Mounting ──────────────────────────────────────────────────────

    init(parentEl) {
        if (!parentEl || this._panel) return this._panel;

        const panel = document.createElement('div');
        panel.id = 'flux-slice-panel';
        panel.className = 'scale0-only flux-slice-panel';
        if (this._dockMode) {
            // Dock mode: panel fills its host (the side-panel slot) and is always
            // visible while its tab is active. No display:none toggle, no chip.
            panel.classList.add('dock-mode');
            // styles moved to .flux-slice-panel.dock-mode

            this.visible = true;
        } else {
            panel.style.display = 'none';
        }

        // Validate ramp registry — fall back to viridis with a warning if
        // anything went wrong with imports.
        for (const drv of FIELD_DRIVERS) {
            if (typeof drv.ramp !== 'function') {
                console.warn(`[flux-slice-panel] ramp missing for ${drv.key}; falling back to viridis`);
                drv.ramp = rampViridis;
            }
        }

        const trailingButton = this._dockMode
            ? `<button type="button" class="flux-slice-expand chart-card-expand"
                       title="Expand flux slices to full-size modal"
                       aria-label="Expand flux slices">⛶</button>`
            : `<button type="button" class="flux-slice-close"
                       aria-label="Hide flux slices">×</button>`;

        panel.innerHTML = `
            <div class="flux-slice-header">
                <span class="flux-slice-title">Flux Slices</span>
                <div class="flux-slice-axis-toggles" role="group" aria-label="Axis toggles">
                    <button type="button" class="flux-slice-axis-btn active"
                            data-axis="xy" aria-pressed="true"
                            title="Toggle xy plane (z=L/2)">xy</button>
                    <button type="button" class="flux-slice-axis-btn active"
                            data-axis="xz" aria-pressed="true"
                            title="Toggle xz plane (y=L/2)">xz</button>
                    <button type="button" class="flux-slice-axis-btn active"
                            data-axis="yz" aria-pressed="true"
                            title="Toggle yz plane (x=L/2)">yz</button>
                </div>
                <button type="button" class="flux-slice-reset-mirror"
                        title="Force every field back on (undoes any per-field force-off / mirror overrides)"
                        disabled>Show all</button>
                ${trailingButton}
            </div>
            <div class="flux-slice-rows">
                ${FIELD_DRIVERS.map(d => this._rowHTML(d)).join('')}
            </div>
        `;
        parentEl.appendChild(panel);
        this._panel = panel;
        this._dockHost = this._dockMode ? parentEl : null;
        this._rowsContainer = panel.querySelector('.flux-slice-rows');
        this._resetMirrorBtn = panel.querySelector('.flux-slice-reset-mirror');

        // Wire per-field rows
        for (const drv of FIELD_DRIVERS) {
            const row = panel.querySelector(`.flux-slice-row[data-field="${drv.key}"]`);
            const chip = row.querySelector('.flux-slice-mirror-chip');
            const slots = {};
            for (const axis of FluxSlicePanel.AXES) {
                slots[axis] = this._wireSlot(row, drv.key, axis);
            }
            this._fields[drv.key] = { row, label: row.querySelector('.flux-slice-field-label'), chip, slots };

            chip.addEventListener('click', () => this._cycleOverride(drv.key));
        }

        // Toggle chip — only mounted in overlay (legacy floating) mode. Dock
        // mode uses the side-panel tab as the show/hide control.
        if (!this._dockMode) {
            const chip = document.createElement('button');
            chip.type = 'button';
            chip.id = 'flux-slice-toggle';
            chip.className = 'scale0-only flux-slice-chip';
            chip.title = 'Toggle live flux slice diagnostics (xy/xz/yz across enabled fields)';
            chip.textContent = 'Flux slices';
            chip.addEventListener('click', () => this.toggle());
            parentEl.appendChild(chip);
            this._chip = chip;

            panel.querySelector('.flux-slice-close')
                ?.addEventListener('click', () => this.setVisible(false));
        } else {
            // Dock mode: wire expand button + start ResizeObserver-driven
            // canvas sizing. The panel self-drives at a bounded rate;
            // visibility is managed by the dock's tab system, not the panel.
            panel.querySelector('.flux-slice-expand')
                ?.addEventListener('click', () => this.toggleExpanded());
            this._setupResizeObserver();
        }

        // Header axis toggles — independent xy/xz/yz visibility, applies
        // to every visible row simultaneously.
        panel.querySelectorAll('.flux-slice-axis-btn').forEach((btn) => {
            btn.addEventListener('click', () => {
                const axis = btn.dataset.axis;
                if (!axis || !FluxSlicePanel.AXES.includes(axis)) return;
                this.setAxisVisible(axis, !this._axisVisible[axis]);
            });
        });

        // Reset mirror clears every per-field override at once.
        this._resetMirrorBtn.addEventListener('click', () => this.resetMirror());

        // Initial chip-label refresh + reconcile.
        for (const drv of FIELD_DRIVERS) this._refreshChip(drv.key);
        this._refreshResetButton();
        this._bindScenarioApplicability();
        let boundary = null;
        this._unsubscribeQualification = subscribeScale0Qualification(q => {
            const next = q.authoritativeLoad
                ? `${q.authoritativeLoad.status}:${q.authoritativeLoad.loadGeneration}`
                : `ready:${q.anchor?.loadGeneration}`;
            if (next === boundary) return;
            boundary = next;
            this._releaseAllWantedSamplers();
            this._scratch = Object.fromEntries(FIELD_DRIVERS.map(d => [d.key, {}]));
            for (const key of Object.keys(this._fieldGlobalMax)) this._fieldGlobalMax[key] = 0;
            this._scenarioBinding.intent(q.scenarioId);
        });
        if (this.visible && !this._emptyInapplicable) this._startSelfDrive();

        return panel;
    }

    _createEmptyMessage() {
        if (this._emptyMessage) return this._emptyMessage;
        const message = document.createElement('section');
        message.className = 'mode-unavailable flux-slice-inapplicable';
        message.dataset.applicability = 'inapplicable';
        message.setAttribute('role', 'status');
        message.innerHTML = `
            <strong>Not applicable — imposed null control</strong>
            <p>Scenario 1 · Empty does not define a field-slice domain. No field
               slice is extracted or rendered for this state.</p>
            <p>A blank heatmap would be misleading: this control is not a
               measurement of physical vacuum or zero-point fields.</p>
        `;
        this._emptyMessage = message;
        return message;
    }

    _bindScenarioApplicability() {
        this._scenarioBinding.bind();
    }

    _handleScenarioIntent(scenarioId) {
        // Suspend immediately on the user's empty request. This occurs before
        // the asynchronous worker replacement can publish any transitional
        // field sample, so a superseded nonempty generation is never painted
        // as evidence about the null control.
        if (scenarioId === EMPTY_SCENARIO_ID) {
            this._setEmptyApplicability(true);
            return;
        }

        // Resume only after canonical state agrees with the requested
        // nonempty scenario. The bounded transition watcher never runs while
        // `empty` is active and is generation-cancelled by a newer request.
        this._scenarioBinding.reconcile({
            scenarioId,
            isReady: () => getScale0State().currentScenarioId === scenarioId
                && isScale0AuthoritativeGenerationReady(getScale0State()),
            onReady: () => this._setEmptyApplicability(false),
        });
    }

    _setEmptyApplicability(inapplicable) {
        const next = !!inapplicable;
        if (next === this._emptyInapplicable) {
            if (next) {
                this._stopSelfDrive();
                this._releaseAllWantedSamplers();
            }
            return;
        }
        this._emptyInapplicable = next;
        this._panel?.classList.toggle('is-inapplicable', next);
        if (this._panel) {
            this._panel.dataset.applicability = next ? 'inapplicable-empty' : 'applicable';
        }

        this._panel?.querySelectorAll('.flux-slice-axis-btn, .flux-slice-expand')
            .forEach((control) => {
                control.disabled = next;
                control.setAttribute('aria-disabled', next ? 'true' : 'false');
            });

        if (next) {
            this._stopSelfDrive();
            this._releaseAllWantedSamplers();
            if (!this._rowFragment) this._rowFragment = document.createDocumentFragment();
            for (const drv of FIELD_DRIVERS) {
                const field = this._fields[drv.key];
                if (field?.row.parentNode === this._rowsContainer) {
                    this._rowFragment.appendChild(field.row);
                }
                for (const axis of FluxSlicePanel.AXES) {
                    const slot = field?.slots[axis];
                    if (!slot) continue;
                    slot.imgData = null;
                    slot.currentN = 0;
                    slot.rgbaBuf = null;
                    slot.tmpCanvas = null;
                    slot.tmpCtx = null;
                }
                this._fieldGlobalMax[drv.key] = 0;
            }
            this._rowsContainer?.replaceChildren(this._createEmptyMessage());
            this._lastN = 0;
            this._lastSimTick = 0;
            this.frameCount = 0;
        } else {
            this._rowsContainer?.replaceChildren();
            for (const drv of FIELD_DRIVERS) {
                const row = this._fields[drv.key]?.row;
                if (row) this._rowsContainer?.appendChild(row);
            }
            this.frameCount = 0;
            if (this.visible) this._startSelfDrive();
        }
        this._refreshResetButton();
    }

    _rowHTML(drv) {
        const tilesHTML = FluxSlicePanel.AXES.map(axis => `
            <figure class="flux-slice-tile" data-plane="${axis}" data-field="${drv.key}">
                <canvas class="flux-slice-canvas"
                        width="${this.canvasPx}" height="${this.canvasPx}"></canvas>
                <figcaption class="flux-slice-caption">
                    <span class="flux-slice-plane-label">${this._planeLabel(axis)}</span>
                    <span class="flux-slice-readout"
                          data-readout="${drv.key}-${axis}">t=— · max —</span>
                </figcaption>
            </figure>
        `).join('');
        return `
            <div class="flux-slice-row" data-field="${drv.key}">
                <div class="flux-slice-row-label">
                    <span class="flux-slice-field-label" title="${drv.label} field">${drv.label}</span>
                    <button type="button" class="flux-slice-mirror-chip"
                            data-field="${drv.key}"
                            title="Cycle: mirror → force-on → force-off → mirror">mirror</button>
                </div>
                <div class="flux-slice-row-tiles">${tilesHTML}</div>
            </div>
        `;
    }

    _planeLabel(axis) {
        if (axis === 'xy') return 'xy @ z=L/2';
        if (axis === 'xz') return 'xz @ y=L/2';
        return 'yz @ x=L/2';
    }

    _wireSlot(row, fieldKey, axis) {
        const tile = row.querySelector(
            `.flux-slice-tile[data-plane="${axis}"][data-field="${fieldKey}"]`);
        const canvas = tile.querySelector('canvas');
        const ctx = canvas.getContext('2d', { alpha: false });
        ctx.imageSmoothingEnabled = false;
        return {
            tile, canvas, ctx,
            readout: row.querySelector(`[data-readout="${fieldKey}-${axis}"]`),
            imgData: null,
            currentN: 0,
            rgbaBuf: null,
            tmpCanvas: null,
            tmpCtx: null,
        };
    }

    // ── Visibility (panel-level) ──────────────────────────────────────

    setVisible(on) {
        this.visible = !!on;
        if (this._panel) this._panel.style.display = this.visible ? '' : 'none';
        if (this._chip) this._chip.classList.toggle('active', this.visible);
        if (this.visible) {
            // Force a fresh paint + reconcile mirror state right away —
            // the panel may have been hidden across a scenario change.
            this.frameCount = 0;
            this._startSelfDrive();
        } else {
            this._stopSelfDrive();
            this._releaseAllWantedSamplers();
        }
    }

    // Release every kind@stride this panel currently has registered with
    // the worker via WasmBridgeProxy._wantSampler, regardless of which
    // driver(s) wanted them — the panel-closing / tab-switched-away
    // counterpart to the per-frame diff in _buildFrameSampleCache (which
    // only releases what a live frame determines is no longer needed).
    // Without this, closing the panel (or switching away from its dock tab)
    // left every previously-visible kind permanently registered, since
    // update() — the only place that diff runs — stops being called at all
    // once hidden.
    _releaseAllWantedSamplers() {
        const bridge = this._samplerBridge || this.getBridge?.();
        bridge?.replaceSamplerWants?.('flux-slice', []);
        this._samplerBridge = null;
        if (!this._prevWantedKeys) return;
        if (bridge && typeof bridge.unwantSampler === 'function') {
            for (const key of this._prevWantedKeys) {
                const at = key.lastIndexOf('@');
                bridge.unwantSampler(key.slice(0, at), Number(key.slice(at + 1)));
            }
        }
        this._prevWantedKeys = null;
    }

    _startSelfDrive() {
        if (this._sub || this._emptyInapplicable || this._disposed) return;
        this._sub = rafCoordinator.subscribe('flux-slice-panel', {
            hz: FLUX_SLICE_DRIVER_HZ,
            cb: () => {
                if (!this.visible) { this._stopSelfDrive(); return; }
                try { this.update(); }
                catch (e) {
                    console.warn('[flux-slice-panel] self-drive update failed:', e);
                }
            }
        });
    }

    _stopSelfDrive() {
        if (this._sub) {
            this._sub.unsubscribe();
            this._sub = null;
        }
    }

    toggle() { this.setVisible(!this.visible); }

    setAxisVisible(axis, on) {
        if (!FluxSlicePanel.AXES.includes(axis)) return;
        this._axisVisible[axis] = !!on;
        if (this._panel) {
            // Toggle the matching tile column across every row.
            this._panel
                .querySelectorAll(`.flux-slice-tile[data-plane="${axis}"]`)
                .forEach(t => t.classList.toggle('axis-hidden', !this._axisVisible[axis]));
            const btn = this._panel.querySelector(
                `.flux-slice-axis-btn[data-axis="${axis}"]`);
            if (btn) {
                btn.classList.toggle('active', this._axisVisible[axis]);
                btn.setAttribute('aria-pressed', this._axisVisible[axis] ? 'true' : 'false');
            }
        }
        if (this._axisVisible[axis]) this.frameCount = 0;
    }

    // ── Per-field override + mirror logic ─────────────────────────────

    _isFieldRowVisible(fieldKey) {
        const ov = this._fieldOverride[fieldKey];
        if (ov === 'on') return true;
        if (ov === 'off') return false;
        const drv = DRIVER_BY_KEY[fieldKey];
        return !!this._mirroredFlags?.[drv?.vizFlagKey];
    }

    setFieldOverride(fieldKey, value) {
        if (!(fieldKey in this._fieldOverride)) return;
        if (value !== null && value !== 'on' && value !== 'off') value = null;
        this._fieldOverride[fieldKey] = value;
        this._refreshChip(fieldKey);
        this._refreshResetButton();
        // Force a fresh paint so a just-revealed row gets data immediately.
        this.frameCount = 0;
    }

    _cycleOverride(fieldKey) {
        const cur = this._fieldOverride[fieldKey];
        const next = cur === null ? 'on' : (cur === 'on' ? 'off' : null);
        this.setFieldOverride(fieldKey, next);
    }

    // Forces every field back to the always-visible default ('on') — the
    // header "Show all" button's handler. Named resetMirror for historical
    // reasons (no external callers; see the button's own label/title for
    // what a user actually sees).
    resetMirror() {
        for (const k of Object.keys(this._fieldOverride)) {
            this._fieldOverride[k] = 'on';
            this._refreshChip(k);
        }
        this._refreshResetButton();
        this.frameCount = 0;
    }

    _refreshChip(fieldKey) {
        const slot = this._fields?.[fieldKey];
        if (!slot) return;
        const ov = this._fieldOverride[fieldKey];
        const drv = DRIVER_BY_KEY[fieldKey];
        const mirrorOn = !!this._mirroredFlags?.[drv?.vizFlagKey];
        slot.chip.classList.toggle('override-on', ov === 'on');
        slot.chip.classList.toggle('override-off', ov === 'off');
        if (ov === 'on') {
            setTextIfChanged(slot.chip, 'on');
            setTitleIfChanged(slot.chip, 'Forced visible. Click to force-off.');
            slot.chip.setAttribute('aria-label', 'Forced visible. Click to force off.');
        } else if (ov === 'off') {
            setTextIfChanged(slot.chip, 'off');
            setTitleIfChanged(slot.chip, 'Forced hidden. Click to return to mirror.');
            slot.chip.setAttribute('aria-label', 'Forced hidden. Click to return to mirror mode.');
        } else {
            setTextIfChanged(slot.chip, 'mirror');
            setTitleIfChanged(slot.chip,
                `Following viz toggle (currently ${mirrorOn ? 'on' : 'off'}). ` +
                'Click to force-on.');
            slot.chip.setAttribute('aria-label',
                `Mirroring the visualization toggle, currently ${mirrorOn ? 'on' : 'off'}. Click to force on.`);
        }
    }

    _refreshResetButton() {
        if (!this._resetMirrorBtn) return;
        const anyOverride = Object.values(this._fieldOverride).some(v => v !== null);
        this._resetMirrorBtn.disabled = this._emptyInapplicable || !anyOverride;
    }

    // ── Per-frame update ──────────────────────────────────────────────

    update() {
        // Scenario 1 is an imposed null control, not a field-domain
        // measurement. The event-driven transition normally stops this loop
        // before it can run; this guard also makes direct/manual update calls
        // scientifically inert while canonical state is empty.
        if (this._emptyInapplicable
            || getScale0State().currentScenarioId === EMPTY_SCENARIO_ID) {
            if (!this._emptyInapplicable) this._setEmptyApplicability(true);
            return;
        }
        // isPanelLive checks whether ITS ARGUMENT carries the `.active` class
        // (or sits inside a non-collapsed `.floating-window`) — see every other
        // Scale-0 panel (dispersion-panel.js, gravity-panel.js, thermo-panel.js,
        // etc.), which all pass their MOUNT HOST, never their own inner root.
        // In dock mode the side-panel tab system toggles `.active` on the HOST
        // container (`#panel-flux-slice`, this._dockHost) — never on this._panel
        // (the inner `#flux-slice-panel` div created by init()), so checking
        // this._panel here always read false and every dock-mode paint was
        // silently skipped forever (the panel rendered as permanently-black,
        // unpainted `{alpha:false}` canvases). In legacy overlay mode there is
        // no separate host (_dockHost is null), so this._panel itself is the
        // right thing to check (its own display:none toggle / any
        // .floating-window ancestor).
        const live = isPanelLive(this._dockHost || this._panel);
        if (!live) {
            // Dock mode never flips `this.visible` false (the tab system
            // hides it via isPanelLive instead, see doc comment above), so
            // setVisible(false)'s release path never fires for it — this is
            // the only place a dock-mode "tab switched away" transition is
            // observable. Release once, right when liveness is lost, not on
            // every subsequent dead frame.
            if (this._wasLive) this._releaseAllWantedSamplers();
            this._wasLive = false;
            return;
        }
        if (!isScale0AuthoritativeGenerationReady(getScale0State())) {
            this._releaseAllWantedSamplers();
            for (const field of Object.values(this._fields)) for (const slot of Object.values(field.slots)) {
                slot.ctx.clearRect(0, 0, slot.canvas.width, slot.canvas.height);
                setTextIfChanged(slot.readout, 'awaiting current generation');
            }
            return;
        }
        this._wasLive = true;
        if (!this.visible || !this._panel) return;
        // Defensive: if the self-drive loop isn't running but we're visible,
        // start it. Covers cases where the panel was set visible by a path
        // that bypassed setVisible (e.g. external code flipping `.visible`
        // directly, hot-reload remount of an externally-driven instance).
        if (!this._sub) this._startSelfDrive();
        this.frameCount = (this.frameCount + 1) | 0;
        const bridge = this.getBridge?.();
        if (!bridge) return;
        const cadence = effectiveFluxSliceUpdateEvery(bridge, this.updateEvery);
        if ((this.frameCount % cadence) !== 0) return;

        const N = bridge.latticeSize | 0;
        if (!Number.isFinite(N) || N < 2) return;

        // Refresh viz-panel flags + chip labels (mirror display may have
        // changed even if no override flips happened).
        try {
            this._mirroredFlags = getFieldStateSnapshot() ?? {};
        } catch (_e) {
            this._mirroredFlags = {};
        }
        for (const drv of FIELD_DRIVERS) this._refreshChip(drv.key);

        // (Re)allocate per-(field,axis) RGBA buffers if the lattice resized.
        if (N !== this._lastN) {
            const px = N * N * 4;
            for (const drv of FIELD_DRIVERS) {
                for (const axis of FluxSlicePanel.AXES) {
                    const slot = this._fields[drv.key].slots[axis];
                    slot.rgbaBuf = new Uint8ClampedArray(px);
                    slot.imgData = null;
                    slot.currentN = 0;
                    slot.tmpCanvas = null;
                    slot.tmpCtx = null;
                }
            }
            this._lastN = N;
        }

        const mid = N >> 1;
        // The panel needs only the owner clock, not a full-volume diagnostic
        // reduction. Keep unavailable/unsafe clocks distinct from tick zero.
        const tickReadback = bridge.currentTick?.();
        const simTick = Number.isSafeInteger(tickReadback) && tickReadback >= 0
            ? tickReadback : null;
        const isRunning = (typeof window !== 'undefined' && window.__ftdCtx)
            ? !!window.__ftdCtx.running
            : true;

        // Scenario change → simTick reset to 0 from a positive value.
        // Reset every per-field override back to 'on' (the always-visible
        // default) and clear rolling autoscale so the new scenario starts
        // clean and fully visible.
        if (Number.isSafeInteger(this._lastSimTick) && this._lastSimTick > 0
            && simTick !== null && simTick < this._lastSimTick) {
            // Reset to the default map (force rows → mirror), NOT blanket
            // 'on' — a scenario switch must not silently re-arm the
            // expensive force-field samplers (see DEFAULT_FIELD_OVERRIDE's
            // doc comment). "Show all" (resetMirror(), an explicit user
            // click) is the only action that forces force rows on too.
            for (const k of Object.keys(this._fieldOverride)) {
                this._fieldOverride[k] = DEFAULT_FIELD_OVERRIDE[k];
            }
            for (const k of Object.keys(this._fieldGlobalMax)) {
                this._fieldGlobalMax[k] = 0;
            }
            for (const drv of FIELD_DRIVERS) this._refreshChip(drv.key);
            this._refreshResetButton();
        }
        if (simTick !== null) this._lastSimTick = simTick;

        // Reconcile row visibility + dense layout. Also collects the
        // visible-this-frame subset so the shared sample cache below fetches
        // only what's actually needed (see _buildFrameSampleCache).
        let activeCount = 0;
        const visibleDrivers = [];
        for (const drv of FIELD_DRIVERS) {
            const visible = this._isFieldRowVisible(drv.key);
            const row = this._fields[drv.key].row;
            const wasHidden = row.classList.contains('row-hidden');
            if (visible && wasHidden) row.classList.remove('row-hidden');
            else if (!visible && !wasHidden) row.classList.add('row-hidden');
            if (visible) { activeCount++; visibleDrivers.push(drv); }
        }
        // Dense mode shrinks tile size when many rows are open.
        this._panel.classList.toggle('dense', activeCount > DENSE_THRESHOLD);

        // ONE shared per-frame sample fetch — see _buildFrameSampleCache's
        // own doc comment for exactly which kinds/types get fetched and why
        // (only what visibleDrivers actually need this frame).
        const sharedSampled = this._buildFrameSampleCache(bridge, visibleDrivers, mid);

        // Per-field sample → max → paint.
        for (const drv of visibleDrivers) {
            // Pull the per-frame source ONCE (from the shared cache, or via
            // the driver's own compute for derived rows), then filter it
            // per-axis below. Saves ~2× bridge cost vs the naive
            // sample-per-axis pattern when 3 axes are visible, and dedupes
            // across rows that share the same underlying quantity (e.g.
            // eField feeding both the |E| row and emEnergy/ePressure).
            const source = drv.source?.(bridge, sharedSampled, this._scratch[drv.key]);

            // Sample only the visible axes for this field.
            const slices = {};
            for (const axis of FluxSlicePanel.AXES) {
                slices[axis] = this._axisVisible[axis]
                    ? drv.sample(bridge, axisIndex(axis), mid, N, source)
                    : null;
            }

            // Per-axis max (for the readout) + per-field union max (for color).
            const axisMax = { xy: 0, xz: 0, yz: 0 };
            let fieldFrameMax = 0;
            for (const axis of FluxSlicePanel.AXES) {
                const s = slices[axis];
                if (!s || s.length === 0) continue;
                let m = 0;
                if (drv.signed) {
                    for (let i = 0; i < s.length; i++) {
                        const av = Math.abs(s[i]);
                        if (av > m) m = av;
                    }
                } else {
                    for (let i = 0; i < s.length; i++) {
                        const v = s[i];
                        if (v > m) m = v;
                    }
                }
                axisMax[axis] = m;
                if (m > fieldFrameMax) fieldFrameMax = m;
            }

            // Decay-then-lift rolling max, per field.
            const prevMax = this._fieldGlobalMax[drv.key] ?? 0;
            const newMax = Math.max(prevMax * this._maxDecay, fieldFrameMax);
            this._fieldGlobalMax[drv.key] = newMax;
            const norm = newMax > FLOOR_FRAC ? 1 / newMax : 0;

            // Paint each visible axis.
            for (const axis of FluxSlicePanel.AXES) {
                if (!this._axisVisible[axis]) continue;
                this._paintSlice(drv, axis, slices[axis], N, norm,
                                 axisMax[axis], simTick, isRunning);
            }
        }
    }

    // ── Shared per-frame sample cache ──────────────────────────────────
    //
    // Builds ONE sampled object for this frame, populated only with the
    // raw kinds / force-field types that the CURRENTLY VISIBLE rows in
    // `visibleDrivers` actually need — never all 14 kinds unconditionally
    // (that would defeat the "hidden rows skip sampling" perf design this
    // panel already relies on). Each kind/type is fetched via
    // bridge.getSamplerOr(kind, 1) (bridge-contract.js's samplerOr() — the
    // exact function getScale0FieldSamples itself calls under
    // capabilities.scale0, reached one property-hop shorter here since the
    // panel already holds the raw bridge) AT MOST ONCE, keyed exactly like
    // field-sample-cache.js's slots (fluxVector, eField, bField, poynting,
    // divergence, vorticity, helicity, kretschmann, latency, fisher,
    // coherence, curlJ, state, gaussResidual, plus forceEm/forceGravity/
    // forceStrong for the 3 force fields, a separate bridge-method family
    // and not a SCALE0_SAMPLER_METHODS kind).
    //
    // Raw-kind / force rows read straight out of the returned object via
    // their own `source(bridge, sampled)`. Derived rows additionally read
    // whatever their own `requiredSampledKeys` named, then call their
    // overlay-frames.js compute function against it plus their own
    // per-driver scratch object (this._scratch[drv.key]) — never fetched
    // here directly, since a derived row does not itself occupy a slot in
    // `sampled`.
    // `mid` (the slice-plane index, N>>1) is passed in from update() rather
    // than recomputed, so the stride-safety check below always matches the
    // exact plane the sample/paint pipeline is about to slice.
    //
    // Stride choice (audit fix — was hardcoded to 1 for EVERY kind here,
    // regardless of that kind's own established default): every method on
    // WasmBridgeProxy already defaults to stride 2 (getEFieldSampled(stride
    // = 2), getVorticitySampled(stride = 2), getEMForceField(stride = 2),
    // etc.) EXCEPT state/gaussResidual, which default to stride 1
    // (STRIDE_ONE_SLOTS in field-sample-cache.js — genuinely sparse/
    // threshold quantities the 3D overlay system always samples at full
    // resolution). Forcing stride 1 for every OTHER kind here scanned 8x
    // more voxels per kind than necessary, and WasmBridgeProxy's
    // _wantSampler registration is PERMANENTLY STICKY (once a kind+stride
    // is requested, the worker computes it on every postFrame() forever —
    // there is no un-want) — so opening this panel even once with all 27
    // rows visible by default permanently saddled the worker with ~15
    // needlessly full-resolution kinds for the rest of the session,
    // competing directly with the SAME worker thread's tick-advancing loop
    // and causing ticking to stall intermittently.
    //
    // coarseStride falls back to 1 whenever the coarser stride would MISS
    // the mid-plane index entirely (mid % stride !== 0) — a correctness
    // guard, not just a quality one: the underlying WASM sampler only ever
    // visits multiples of stride starting at 0, so a mismatched stride
    // makes the slice come back completely EMPTY, not just coarser.
    _buildFrameSampleCache(bridge, visibleDrivers, mid) {
        return buildFluxSliceSampleCache(this, bridge, visibleDrivers, mid);
    }

    _paintSlice(drv, axis, data, N, norm, axisFrameMax, simTick, isRunning) {
        const slot = this._fields[drv.key]?.slots[axis];
        if (!slot) return;
        const buf = slot.rgbaBuf;
        if (!buf) return;
        if (!data || data.length === 0) {
            const c = slot.ctx;
            c.fillStyle = '#0a0d14';
            c.fillRect(0, 0, slot.canvas.width, slot.canvas.height);
            setTextIfChanged(slot.readout, `${simTick === null ? 'tick unavailable' : `t=${simTick}`} · max —`);
            return;
        }

        const ramp = drv.ramp;
        const rgb = [0, 0, 0];
        if (drv.rawRamp) {
            // Feed the raw sample value straight into the ramp instead of
            // the usual autoscaled-and-clamped `data[i] * norm` — only the
            // `phase` driver sets this today. rampCyclicHSL wants a raw
            // radian angle (the 3D renderer calls it the same way), and a
            // per-frame [-1,1] autoscale ratio would be meaningless for an
            // angular quantity.
            for (let i = 0, p = 0; i < data.length; i++, p += 4) {
                ramp(data[i], rgb, 0);
                buf[p]     = (rgb[0] * 255) | 0;
                buf[p + 1] = (rgb[1] * 255) | 0;
                buf[p + 2] = (rgb[2] * 255) | 0;
                buf[p + 3] = 255;
            }
        } else if (drv.signed) {
            // Map t ∈ [-1, +1] through the diverging ramp (rampCharge etc.).
            for (let i = 0, p = 0; i < data.length; i++, p += 4) {
                let t = data[i] * norm;
                if (t > 1) t = 1; else if (t < -1) t = -1;
                ramp(t, rgb, 0);
                buf[p]     = (rgb[0] * 255) | 0;
                buf[p + 1] = (rgb[1] * 255) | 0;
                buf[p + 2] = (rgb[2] * 255) | 0;
                buf[p + 3] = 255;
            }
        } else {
            for (let i = 0, p = 0; i < data.length; i++, p += 4) {
                let t = data[i] * norm;
                if (t > 1) t = 1; else if (t < 0) t = 0;
                ramp(t, rgb, 0);
                buf[p]     = (rgb[0] * 255) | 0;
                buf[p + 1] = (rgb[1] * 255) | 0;
                buf[p + 2] = (rgb[2] * 255) | 0;
                buf[p + 3] = 255;
            }
        }

        if (!slot.imgData || slot.currentN !== N) {
            slot.imgData = new ImageData(buf, N, N);
            slot.currentN = N;
        }

        // Refresh the visible canvas size. In dock mode, _canvasPx is updated
        // by the ResizeObserver as the side panel resizes; in overlay mode, it
        // toggles between DEFAULT_CANVAS_PX and DENSE_CANVAS_PX based on the
        // active-row count.
        let desiredPx;
        if (this._dockMode && !this._expanded) {
            desiredPx = this._canvasPx | 0;
        } else {
            desiredPx = this._panel?.classList.contains('dense')
                ? DENSE_CANVAS_PX : this.canvasPx;
        }
        if (slot.canvas.width !== desiredPx || slot.canvas.height !== desiredPx) {
            slot.canvas.width = desiredPx;
            slot.canvas.height = desiredPx;
            slot.ctx.imageSmoothingEnabled = false;
        }

        const c = slot.ctx;
        const W = slot.canvas.width;
        const H = slot.canvas.height;

        if (!slot.tmpCanvas || slot.tmpCanvas.width !== N) {
            slot.tmpCanvas = document.createElement('canvas');
            slot.tmpCanvas.width = N;
            slot.tmpCanvas.height = N;
            slot.tmpCtx = slot.tmpCanvas.getContext('2d', { alpha: false });
            slot.tmpCtx.imageSmoothingEnabled = false;
        }
        slot.tmpCtx.putImageData(slot.imgData, 0, 0);
        c.imageSmoothingEnabled = false;
        c.drawImage(slot.tmpCanvas, 0, 0, N, N, 0, 0, W, H);

        const pausedTag = isRunning ? '' : ' ⏸';
        setTextIfChanged(slot.readout,
            `${simTick === null ? 'tick unavailable' : `t=${simTick}`}${pausedTag} · max ${this._fmt(axisFrameMax)}`);
    }

    _fmt(v) {
        if (!Number.isFinite(v)) return '—';
        if (v === 0) return '0';
        const abs = Math.abs(v);
        if (abs >= 1000 || abs < 0.01) return v.toExponential(2);
        return v.toFixed(3);
    }

    // ── Dock-mode helpers ─────────────────────────────────────────────

    /**
     * Computes per-tile canvas size from the available container width so
     * the configured visible axes fit horizontally without overflow.
     *
     * Budget breakdown (matched to dock-mode CSS in toolbar.css):
     *   label column        56 px
     *   label-to-tiles gap   8 px
     *   panel inner padding 24 px (12+12)
     *   inter-tile gap       6 × (visibleAxes - 1)
     *   scrollbar safety    10 px
     *
     * Lower clamp 64px keeps tiles legible on the smallest dock width
     * (320px → ~64px tiles for all 3 axes); upper clamp 220px matches
     * the overlay-mode default so dock and overlay agree at large widths.
     */
    _computeDockTileSize(containerWidth) {
        const visibleAxes = Math.max(
            1,
            Object.values(this._axisVisible).filter(Boolean).length,
        );
        const tileGaps = 6 * (visibleAxes - 1);
        const overhead = 56 + 8 + 24 + tileGaps + 10;
        const usable = Math.max(60, containerWidth - overhead);
        const px = Math.floor(usable / visibleAxes);
        return Math.max(64, Math.min(220, px));
    }

    _setupResizeObserver() {
        if (!this._dockMode || typeof ResizeObserver === 'undefined') return;
        const host = this._dockHost;
        if (!host) return;
        // Observe #panel-area (the side-panel slot) and NOT the host: the
        // host inherits its own overflow once tiles render too wide, so it
        // would feed the formula an inflated width and lock in 220px tiles.
        // panel-area's contentRect is the true sizing budget.
        const budgetEl = document.getElementById('panel-area') || host;
        let lastPx = -1;
        const measure = (rect) => {
            const cs = getComputedStyle(budgetEl);
            const padX = (parseFloat(cs.paddingLeft) || 0) + (parseFloat(cs.paddingRight) || 0);
            const w = (rect?.width ?? budgetEl.getBoundingClientRect().width) - padX;
            if (w <= 0) return;
            const target = this._computeDockTileSize(w);
            if (target === lastPx) return;
            lastPx = target;
            this._canvasPx = target;
        };
        this._resizeObs = new ResizeObserver((entries) => {
            for (const entry of entries) measure(entry.contentRect);
        });
        this._resizeObs.observe(budgetEl);
        // Seed once synchronously so the first paint uses a sensible size.
        measure();
    }

    /**
     * Toggle a full-size modal-style overlay containing the same panel
     * DOM. The panel root is reparented (not cloned) so all per-row
     * state (overrides, slots, autoscale) carries over with zero
     * coupling. Click the X (or backdrop) to return to the dock.
     */
    toggleExpanded() {
        if (!this._dockMode || !this._panel) return;
        if (this._expanded) {
            this._collapse();
        } else {
            this._expand();
        }
    }

    _expand() {
        if (this._expanded) return;
        const panel = this._panel;
        const host = this._dockHost;
        if (!panel || !host) return;

        // Backdrop scrim
        const scrim = document.createElement('div');
        scrim.className = 's0-expand-scrim';
        scrim.setAttribute('role', 'presentation');

        // Modal frame
        const modal = document.createElement('div');
        modal.className = 's0-expand-modal';

        // Move the panel root into the modal (no clone — preserve state).
        const dockSlot = document.createComment('flux-slice-panel-dock-slot');
        host.replaceChild(dockSlot, panel);
        modal.appendChild(panel);

        // Tweak panel styling for expand-mode.
        panel.dataset.modeOriginal = 'dock';
        panel.classList.add('expand-mode');
        // In expand-mode we want the original full-size canvases.
        // Using a large target signals the painter to allocate full size.
        this._canvasPx = DEFAULT_CANVAS_PX;

        const close = () => this._collapse();
        scrim.addEventListener('click', close);
        // Add an X close button to the modal
        const closeBtn = document.createElement('button');
        closeBtn.type = 'button';
        closeBtn.textContent = '×';
        closeBtn.className = 's0-expand-close';
        closeBtn.setAttribute('aria-label', 'Close expanded flux slices');
        closeBtn.addEventListener('click', close);
        modal.appendChild(closeBtn);

        document.body.appendChild(scrim);
        document.body.appendChild(modal);

        this._expandModal = modal;
        this._expandScrim = scrim;
        this._dockSlotMarker = dockSlot;
        this._expanded = true;
    }

    _collapse() {
        if (!this._expanded) return;
        const panel = this._panel;
        const host = this._dockHost;
        const marker = this._dockSlotMarker;
        if (panel && host && marker && marker.parentNode === host) {
            host.replaceChild(panel, marker);
        } else if (panel && host) {
            // Fallback: append back to host.
            host.appendChild(panel);
        }
        this._expandModal?.remove();
        this._expandScrim?.remove();
        this._expandModal = null;
        this._expandScrim = null;
        this._dockSlotMarker = null;
        panel?.classList.remove('expand-mode');
        this._expanded = false;
        // Resume dock-sized canvases.
        if (host) {
            this._canvasPx = this._computeDockTileSize(host.getBoundingClientRect().width || 380);
        }
    }

    // ── Teardown ──────────────────────────────────────────────────────

    dispose() {
        this._unsubscribeQualification?.();
        this._disposed = true;     // self-drive guard (Audit pass 2 FLUX-2)
        this._stopSelfDrive();
        this._releaseAllWantedSamplers();
        this._scenarioBinding.dispose();
        if (this._expanded) this._collapse();
        this._resizeObs?.disconnect();
        this._resizeObs = null;
        this._panel?.remove();
        this._chip?.remove();
        this._panel = null;
        this._chip = null;
        this._fields = {};
        this._rowsContainer = null;
        this._resetMirrorBtn = null;
        this._rowFragment = null;
        this._emptyMessage = null;
        this._dockHost = null;
        // Clear the window-singleton ref so the detached panel subtree
        // is GC-eligible. (Audit pass 2: cross-cutting __ftd*Panel
        // retention fix.)
        if (typeof window !== 'undefined' && window.__ftdFluxSlicePanel === this) {
            window.__ftdFluxSlicePanel = null;
        }
    }
}


/**
 * Idempotent mount used from the Scale 0 controller. Stashes the
 * singleton on `window.__ftdFluxSlicePanel` so a re-entry (scale-switch
 * round-trip) reuses the same DOM node.
 *
 * Re-mount must REFRESH the `getBridge` callback. Each re-entry of
 * Scale 0 builds a fresh `ctx`, and the active flux source can swap
 * between WASM bridge and mock as scenarios load. A stale closure
 * captured by the first mount would silently keep reading the original
 * ctx, leaving the panel pointed at a torn-down bridge.
 */
/**
 * Side-panel-tab init function. Mounts the flux-slice panel inside the
 * #panel-flux-slice slot in dock mode, with auto-shrinking tile sizing
 * driven by ResizeObserver and an expand button for full-size modal
 * inspection.
 *
 * Idempotent: calling twice reuses the singleton at window.__ftdFluxSlicePanel.
 *
 * Bridge resolution: at the time this function runs from app.js,
 * window.__ftdCtx may not yet exist. The getBridge callback is evaluated
 * per frame inside the panel's update loop, so it gracefully handles a
 * null context until the Scale 0 controller initializes. When the active
 * scenario uses MockBridge for flux dynamics (state.useFluxMock), reads
 * are routed through the mock to avoid stale-data heatmaps.
 */
export function initFluxSlicePanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-flux-slice');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    if (typeof window !== 'undefined' && window.__ftdFluxSlicePanel) {
        const existing = window.__ftdFluxSlicePanel;
        existing.getBridge = getBridge;
        existing._bindScenarioApplicability?.();
        // If a prior overlay-mode panel exists, leave it alone — the dock
        // slot is empty in that case (user has the legacy chip).
        return existing;
    }
    const panel = new FluxSlicePanel({ getBridge, dockMode: true });
    panel.init(host);
    if (typeof window !== 'undefined') window.__ftdFluxSlicePanel = panel;
    return panel;
}
