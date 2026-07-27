/**
 * @file engine/web/js/scales/scale0/ui/overlays/wave-lab/wave-info.js
 * @purpose Live telemetry and controls for standalone RF/light/sound lattice waves.
 */

import { BaseComponent } from '../../../../../core/component.js';
import {
    getWaveScenarioSettings,
    isSingleWaveScenario,
    LIGHT_LATTICE_WAVE_SCENARIO_ID,
    RF_LATTICE_WAVE_SCENARIO_ID,
    resetWaveScenarioSettings,
    setWaveScenarioSettings,
    SOUND_LATTICE_WAVE_SCENARIO_ID,
    SOUND_COLLISION_SCENARIO_ID,
} from '../../../../../bridge/scenarios/spectrum-comparator.js';
import { markFieldDirty, setLatticeNeedsUpload } from '../../../state/store.js';
import { cardStyle, titleStyle, tagBadge, formatExp, formatFixed } from '../_card-helpers.js';
import { LatticeSynth } from '../../../../../audio/lattice-synth.js';
import { Sparkline } from '../../../../../ui/charts/sparkline.js';
import { getScale0Scenario } from '../../../scenario-registry.js';


class RingBuffer {
    constructor(size) {
        this.size = size;
        this.data = new Float64Array(size);
        this.count = 0;
    }
    push(val) {
        this.data[this.count % this.size] = val;
        this.count++;
    }
    get(i) {
        if (i < 0 || i >= this.count) return 0;
        if (this.count <= this.size) return this.data[i];
        const oldest = this.count - this.size;
        if (i < oldest) return 0;
        return this.data[i % this.size];
    }
}

const SCENARIO_IDS = new Set([
    RF_LATTICE_WAVE_SCENARIO_ID,
    LIGHT_LATTICE_WAVE_SCENARIO_ID,
    SOUND_LATTICE_WAVE_SCENARIO_ID,
]);

const SCENARIO_TITLES = {
    [RF_LATTICE_WAVE_SCENARIO_ID]: 'RF lattice wave',
    [LIGHT_LATTICE_WAVE_SCENARIO_ID]: 'Light lattice wave',
    [SOUND_LATTICE_WAVE_SCENARIO_ID]: 'Sound lattice proxy',
};

const TEMPLATE = `
    <section data-section="wave-lab" ref="root" style="${cardStyle(620)}">
        <div ref="title" style="${titleStyle()}">Wave Lab</div>
        <div ref="info"></div>
        <div ref="controls"></div>
        <div ref="body"></div>
        <div ref="trendlines" style="margin-top:10px;"></div>
    </section>
`;

function row(label, value, tag = 'M') {
    return `
        <div style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:baseline;margin:3px 0;">
            <span style="color:var(--text-muted);">${tagBadge(tag)}${label}</span>
            <span style="font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:var(--accent);">${value}</span>
        </div>
    `;
}

function fmtRatio(v, digits = 3) {
    return Number.isFinite(v) ? formatFixed(v, digits).trim() : 'nan';
}

function laneCell(value, color = 'var(--text-primary)') {
    return `<span style="font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:${color};white-space:nowrap;">${value}</span>`;
}

function setName(id) {
    if (id === 'rf') return 'RF';
    if (id === 'light') return 'Light';
    if (id === 'sound') return 'Sound';
    return id || 'other';
}

function carrierName(set) {
    if (set.id === 'sound') return 'longitudinal';
    if (set.id === 'rf' || set.id === 'light') return 'transverse';
    return set.carrier || 'proxy';
}

function laneName(lane) {
    const labels = {
        rf: 'RF',
        sound_air_proxy: 'Sound',
        light_visible: 'Light',
    };
    return labels[lane.id] || lane.label;
}

function laneRows(lanes = []) {
    const headerStyle = 'color:var(--text-muted);font-size:11px;text-transform:uppercase;';
    const rowStyle = 'display:grid;grid-template-columns:minmax(58px,1fr) 30px 48px 56px 46px;gap:6px;align-items:baseline;margin:3px 0;';
    const rows = lanes.map((lane) => {
        const energyPct = (lane.energyShare ?? 0) * 100;
        return `
            <div style="${rowStyle}">
                <span style="color:var(--text-muted);min-width:0;white-space:nowrap;" title="${lane.label}">${tagBadge(lane.tag)}${laneName(lane)}</span>
                ${laneCell(lane.modeN)}
                ${laneCell(fmtRatio(lane.lambda, 2))}
                ${laneCell(formatExp(lane.frequency), 'var(--accent)')}
                ${laneCell(`${fmtRatio(energyPct, 1)}%`, 'var(--accent)')}
            </div>
        `;
    }).join('');

    return `
        <div style="margin-top:8px;">
            <div style="${rowStyle};${headerStyle}">
                <span>lane</span><span>n</span><span>lambda</span><span>f</span><span>E%</span>
            </div>
            ${rows}
        </div>
    `;
}

function sparkRow(label, id) {
    return `
        <div style="display:flex;align-items:center;gap:12px;margin:4px 0;">
            <span style="color:var(--text-muted);font-size:12px;width:120px;flex-shrink:0;">${label}</span>
            <div data-spark="${id}" style="flex-grow:1;min-width:0;height:26px;"></div>
        </div>
    `;
}

function compactTag(text) {
    return `<span style="display:inline-flex;align-items:center;min-height:20px;padding:2px 6px;border:1px solid var(--border);border-radius:6px;background:rgba(255,255,255,0.035);color:var(--text-muted);font-size:11px;line-height:1.2;">${text}</span>`;
}

function infoCenter(m, lane) {
    if (!m?.singleScenario || !lane) {
        return `
            <div style="margin:8px 0 10px;padding:9px;border:1px solid var(--border);border-radius:7px;background:rgba(255,255,255,0.035);">
                <div style="display:flex;align-items:center;gap:6px;margin-bottom:6px;color:var(--text-primary);font-weight:600;">
                    ${tagBadge('T')} Wave information center
                </div>
                <div style="color:var(--text-muted);font-size:12px;line-height:1.35;">
                    Select an admitted transverse lattice mode or the longitudinal sound-speed negative control. This panel then becomes the live wave recipe, controls, and telemetry center.
                </div>
            </div>
        `;
    }

    const component = lane.component || 'y';
    const family = setName(lane.set);
    const frequencyNote = lane.set === 'rf'
        ? 'The legacy RF label denotes only the lowest selected transverse spatial mode; there is no SI radio calibration.'
        : lane.set === 'light'
            ? 'The legacy light label denotes only a higher transverse lattice mode; it has no optical or color calibration.'
            : 'The c/8 value changes the seed momentum only. The native engine has no medium and re-propagates this longitudinal component at its ordinary lattice pole.';
    return `
        <div style="margin:8px 0 10px;padding:9px;border:1px solid var(--border);border-radius:7px;background:rgba(255,255,255,0.035);">
            <div style="display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:6px;">
                <span style="color:var(--text-primary);font-weight:600;">${tagBadge(lane.proxy ? 'T' : 'M')} Wave information center</span>
                <span style="font-family:var(--font-mono);color:var(--accent);white-space:nowrap;">${family}</span>
            </div>
            <div style="display:flex;flex-wrap:wrap;gap:5px;margin-bottom:7px;">
                ${compactTag(`${carrierName(lane)} flux`)}
                ${compactTag(`J${component}/W${component}`)}
                ${compactTag(`n=${lane.modeN}`)}
                ${compactTag(`lambda=${fmtRatio(lane.lambda, 2)}`)}
                ${compactTag(`native vg/c=${fmtRatio(lane.speedRatioToLight ?? 0, 3)}`)}
                ${lane.proxy ? compactTag(`seed W ratio=${fmtRatio(lane.seedSpeedRatioToLight ?? 0, 3)}`) : ''}
            </div>
            <div style="color:var(--text-muted);font-size:12px;line-height:1.35;">
                ${frequencyNote} The sliders reseed the lattice immediately; measured energy, peaks, and samples below come from the buffers after that seed.
            </div>
        </div>
    `;
}

function controlValue(name, value) {
    return `<span data-wave-value="${name}" style="font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:var(--accent);">${value}</span>`;
}

function sliderRow(name, label, value, min, max, step, display, attrs = '') {
    return `
        <label style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:center;margin:7px 0;color:var(--text-muted);font-size:12px;">
            <span>${label}</span>
            ${controlValue(name, display)}
            <input data-wave-control="${name}" type="range" min="${min}" max="${max}" step="${step}" value="${value}" ${attrs}
                style="grid-column:1 / -1;width:100%;accent-color:var(--accent);">
        </label>
    `;
}

function synthSliderRow(name, label, value, min, max, step, display, attrs = '') {
    return `
        <label style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:center;margin:7px 0;color:var(--text-muted);font-size:12px;">
            <span>${label}</span>
            <span data-synth-value="${name}" style="font-family:var(--font-mono);font-variant-numeric:tabular-nums;color:#10b981;">${display}</span>
            <input data-synth-control="${name}" type="range" min="${min}" max="${max}" step="${step}" value="${value}" ${attrs}
                style="grid-column:1 / -1;width:100%;accent-color:#10b981;">
        </label>
    `;
}

function synthSelectRow(name, label, options, selected) {
    const opts = options.map(o => `<option value="${o.value}" ${o.value === selected ? 'selected' : ''} style="background:var(--bg-elevated);color:var(--text-primary);">${o.label}</option>`).join('');
    return `
        <label style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:center;margin:7px 0;color:var(--text-muted);font-size:12px;">
            <span>${label}</span>
            <select data-synth-control="${name}" style="background:var(--bg-elevated);color:var(--text-primary);border:1px solid var(--border);border-radius:4px;padding:2px 4px;font-size:12px;outline:none;">
                ${opts}
            </select>
        </label>
    `;
}

function synthToggleRow(name, label, checked) {
    return `
        <label style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:center;margin:7px 0;color:var(--text-muted);font-size:12px;cursor:pointer;">
            <span>${label}</span>
            <input data-synth-control="${name}" type="checkbox" ${checked ? 'checked' : ''} style="accent-color:#10b981;cursor:pointer;">
        </label>
    `;
}

function formatControlValue(name, value) {
    if (name === 'modeN') return `${Math.round(value)}`;
    if (name === 'amp') return Number(value).toFixed(3);
    if (name === 'sigmaFrac') return `${Math.round(Number(value) * 100)}%`;
    if (name === 'pulseFrac') return Number(value) >= 0.985 ? 'full' : `${Math.round(Number(value) * 100)}%`;
    if (name === 'phase') return Number(value).toFixed(2);
    if (name === 'speedRatio') return Number(value).toFixed(3);
    return String(value);
}

function singleWaveBody(m, lane) {
    const component = lane.component || 'y';
    const jLabel = `J${component}`;
    const wLabel = `W${component}`;
    const period = lane.frequency > 0 ? 1 / lane.frequency : 0;
    const family = setName(lane.set);
    const note = lane.set === 'sound'
        ? 'Longitudinal medium proxy: slower dashboard speed, no acoustic-material derivation.'
        : lane.set === 'rf'
            ? 'RF proxy: long-wavelength transverse flux mode, one lattice-period across the box.'
            : 'Light proxy: shorter-wavelength transverse flux mode, not SI-calibrated color.';
    return `
        ${row('tick / lattice', `${m.tick} / L=${m.latticeSize}`)}
        ${row('family', `${family} / ${carrierName(lane)}`, 'T')}
        ${row('component', `${jLabel}/${wLabel}`, 'T')}
        ${row('mode n', lane.modeN, 'T')}
        ${row('amplitude', fmtRatio(lane.amplitude ?? 0, 4), 'T')}
        ${row('lambda', fmtRatio(lane.lambda, 3), 'T')}
        ${row('frequency', formatExp(lane.frequency), 'T')}
        ${row('period', fmtRatio(period, 3), 'T')}
        ${row('beam radius', fmtRatio(lane.sigma ?? 0, 3), 'T')}
        ${row('pulse width', lane.pulseActive ? `${fmtRatio(lane.pulseFrac ?? 1, 3)} L` : 'full lattice', 'T')}
        ${row('phase velocity', fmtRatio(lane.phaseVelocity ?? 0, 6), 'T')}
        ${row('group velocity', fmtRatio(lane.groupVelocity ?? 0, 6), 'T')}
        ${row('v / c', fmtRatio(lane.speedRatioToLight ?? 0, 3), 'T')}
        ${row('lane energy', formatExp(lane.energy ?? 0))}
        ${row('field / wave E', `${formatExp(lane.fieldEnergy ?? 0)} / ${formatExp(lane.waveEnergy ?? 0)}`)}
        ${row(`peak |${jLabel}|`, formatExp(lane.peakDirectionalFlux ?? lane.peakFlux ?? 0))}
        ${row(`peak |${wLabel}|`, formatExp(lane.peakDirectionalWaveVel ?? lane.peakWaveVel ?? 0))}
        ${row(`sample ${jLabel}/${wLabel}`, `${formatExp(lane.sampleFlux ?? 0)} / ${formatExp(lane.sampleWaveVel ?? 0)}`)}
        ${row('energy centroid x', fmtRatio(lane.energyCentroidX ?? 0, 3))}
        ${laneRows(m.lanes)}
        <div style="margin-top:8px;color:var(--text-muted);font-size:12px;line-height:1.35;">
            ${tagBadge(lane.proxy ? 'T' : 'M')}${note}
        </div>
    `;
}

export class WaveInfoComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
        this.bridgeRef = null;
        this.scenarioId = '';
        this.controlKey = '';
        this.reseedHandle = 0;
        this.synth = new LatticeSynth();
        this.history = {
            energy: new RingBuffer(150),
            peakJ: new RingBuffer(150),
            peakW: new RingBuffer(150),
            sampleJ: new RingBuffer(150),
            sampleW: new RingBuffer(150),
        };
        this.sparks = {};
        this.element.addEventListener('input', (e) => this._handleControlInput(e));
        this.element.addEventListener('click', (e) => {
            if (e.target.closest('[data-wave-reset]')) this._resetControls();
            if (e.target.closest('[data-wave-audio]')) this._toggleAudio();
        });
    }

    update(bridge, scenarioId) {
        const scenario = getScale0Scenario(scenarioId);
        if (!scenario?.tags?.includes('wave-lab')) {
            this.refs.title.textContent = 'Wave Lab';
            this.refs.info.innerHTML = infoCenter(null, null);
            this.refs.controls.innerHTML = '';
            this.refs.body.innerHTML = `
                <div style="color:var(--text-muted);font-size:12px;line-height:1.35;">
                    ${tagBadge('T')}Standalone wave instruments are available as RF lattice wave, Light lattice wave, and Sound lattice proxy.
                </div>
            `;
            this._updateTrendlines(null, null);
            return;
        }
        this.bridgeRef = bridge;
        this.scenarioId = scenarioId;

        const titleText = SCENARIO_TITLES[scenarioId] || 'Wave Lab';
        const audioIcon = this.synth.active ? '🔊' : '🔈';
        const audioButton = scenarioId === SOUND_LATTICE_WAVE_SCENARIO_ID 
            ? `<button data-wave-audio style="border:none;background:none;color:var(--text-muted);cursor:pointer;font-size:16px;padding:0;margin-left:8px;" title="Toggle pure sound from lattice mode metrics">
                   ${audioIcon}
               </button>` 
            : '';
        this.refs.title.innerHTML = `
            <div style="display:flex;align-items:center;justify-content:space-between;width:100%;">
                <span>${titleText}</span>
                ${audioButton}
            </div>
        `;

        const m = bridge.getSpectrumComparatorMetrics?.(scenarioId);
        if (!m || !m.active) {
            this.synth.update(null);
            this._updateTrendlines(null, null);
            this.refs.body.innerHTML = `
                <div style="color:var(--text-muted);font-style:italic;">
                    ${tagBadge('~M')} waiting for field buffers
                </div>
            `;
            return;
        }

        const lane = m.lanes?.[0];
        this.synth.update(m);
        this.refs.info.innerHTML = infoCenter(m, lane);
        this._renderControls(m, lane);
        this.refs.body.innerHTML = m.singleScenario && lane ? singleWaveBody(m, lane) : '';
        this._updateTrendlines(m, lane);
    }

    _updateTrendlines(m, lane) {
        if (!m || !m.singleScenario || !lane) {
            this.refs.trendlines.innerHTML = '';
            for (const key in this.sparks) {
                this.sparks[key].destroy();
            }
            this.sparks = {};
            return;
        }

        // Push data into buffers
        this.history.energy.push(lane.energy || 0);
        this.history.peakJ.push(lane.peakDirectionalFlux || lane.peakFlux || 0);
        this.history.peakW.push(lane.peakDirectionalWaveVel || lane.peakWaveVel || 0);
        this.history.sampleJ.push(lane.sampleFlux || 0);
        this.history.sampleW.push(lane.sampleWaveVel || 0);

        // Build DOM once
        if (Object.keys(this.sparks).length === 0) {
            this.refs.trendlines.innerHTML = `
                <div style="margin:0 0 10px;padding:9px;border:1px solid var(--border);border-radius:7px;background:rgba(255,255,255,0.025);">
                    <div style="display:flex;align-items:center;margin-bottom:6px;color:var(--text-primary);font-weight:600;">
                        Dynamics History
                    </div>
                    ${sparkRow('Lane Energy', 'spark-energy')}
                    ${sparkRow('Peak Flux |J|', 'spark-peakJ')}
                    ${sparkRow('Peak WaveVel |W|', 'spark-peakW')}
                    ${sparkRow('Sample Jx (Probe)', 'spark-sampleJ')}
                    ${sparkRow('Sample Wx (Probe)', 'spark-sampleW')}
                </div>
            `;
            const initSpark = (id, buffer, color) => {
                const el = this.refs.trendlines.querySelector(`[data-spark="${id}"]`);
                if (el) this.sparks[id] = new Sparkline(el, { buffer, color, height: 26 });
            };
            initSpark('spark-energy', this.history.energy, '#6366f1');
            initSpark('spark-peakJ', this.history.peakJ, '#f43f5e');
            initSpark('spark-peakW', this.history.peakW, '#10b981');
            initSpark('spark-sampleJ', this.history.sampleJ, '#f59e0b');
            initSpark('spark-sampleW', this.history.sampleW, '#8b5cf6');
        }

        // Update all sparklines
        for (const key in this.sparks) {
            this.sparks[key].update();
        }
    }

    async _toggleAudio() {
        if (!isSingleWaveScenario(this.scenarioId) || this.scenarioId !== SOUND_LATTICE_WAVE_SCENARIO_ID) return;
        if (this.synth.active) {
            this.synth.stop();
        } else {
            await this.synth.init();
        }
        // Force a UI update to refresh the icon
        this.update(this.bridgeRef, this.scenarioId);
    }

    _renderControls(m, lane, force = false) {
        if (!m?.singleScenario || !lane || !isSingleWaveScenario(this.scenarioId)) {
            this.refs.controls.innerHTML = '';
            this.controlKey = '';
            return;
        }
        const settings = getWaveScenarioSettings(this.scenarioId);
        if (!settings) {
            this.refs.controls.innerHTML = '';
            this.controlKey = '';
            return;
        }
        const maxMode = Math.max(1, Math.floor((m.latticeSize || 33) / 2) - 1);
        const key = `${this.scenarioId}:${m.latticeSize}:${maxMode}`;
        if (!force && this.controlKey === key) return;
        this.controlKey = key;
        const isSoundScenario = this.scenarioId === SOUND_LATTICE_WAVE_SCENARIO_ID || this.scenarioId === SOUND_COLLISION_SCENARIO_ID;
        const speedControl = isSoundScenario
            ? sliderRow('speedRatio', 'seed W coefficient / c', settings.speedRatio.toFixed(3), 0.04, 0.50, 0.005, formatControlValue('speedRatio', settings.speedRatio))
            : `
                <div style="display:grid;grid-template-columns:minmax(0,1fr) auto;gap:8px;align-items:center;margin:7px 0;color:var(--text-muted);font-size:12px;">
                    <span>native wave coefficient / c</span>
                    ${controlValue('speedRatio', '1.000')}
                </div>
            `;
        this.refs.controls.innerHTML = `
            <div style="margin:0 0 10px;padding:9px;border:1px solid var(--border);border-radius:7px;background:rgba(255,255,255,0.025);">
                <div style="display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:4px;">
                    <span style="color:var(--text-primary);font-weight:600;">Wave controls</span>
                    <button data-wave-reset type="button" style="border:1px solid var(--border);background:rgba(255,255,255,0.04);color:var(--text-muted);border-radius:6px;padding:3px 7px;font-size:11px;cursor:pointer;">reset</button>
                </div>
                ${sliderRow('modeN', 'frequency mode n', settings.modeN, 1, maxMode, 1, formatControlValue('modeN', settings.modeN))}
                ${sliderRow('amp', 'amplitude |J|', settings.amp.toFixed(3), 0.001, 0.120, 0.001, formatControlValue('amp', settings.amp))}
                ${sliderRow('sigmaFrac', 'beam radius', settings.sigmaFrac.toFixed(3), 0.025, 0.240, 0.005, formatControlValue('sigmaFrac', settings.sigmaFrac))}
                ${sliderRow('pulseFrac', 'pulse width', settings.pulseFrac.toFixed(3), 0.120, 1.000, 0.005, formatControlValue('pulseFrac', settings.pulseFrac))}
                ${sliderRow('phase', 'phase offset', settings.phase.toFixed(3), 0, (Math.PI * 2).toFixed(3), 0.01, formatControlValue('phase', settings.phase))}
                ${speedControl}
                <div style="margin-top:6px;color:var(--text-muted);font-size:11px;line-height:1.35;">
                    ${tagBadge('T')}Frequency is the native kick-drift dispersion readout from mode n. For the sound negative control, the speed slider changes only the seeded W amplitude; it does not change the engine pole. Pulse width gates the x-envelope.
                </div>
            </div>
        `;
        if (isSoundScenario) {
            this.refs.controls.innerHTML += `
                <div style="margin:0 0 10px;padding:9px;border:1px solid var(--border);border-radius:7px;background:rgba(255,255,255,0.025);">
                    <div style="color:var(--text-primary);font-weight:600;margin-bottom:4px;">Audio Synthesizer</div>
                    ${synthSelectRow('oscType', 'waveform', [
                        { value: 'sine', label: 'Sine' },
                        { value: 'square', label: 'Square' },
                        { value: 'sawtooth', label: 'Sawtooth' },
                        { value: 'triangle', label: 'Triangle' }
                    ], this.synth._oscType)}
                    ${synthSliderRow('masterVolume', 'master volume', this.synth.masterVolume, 0, 2, 0.01, this.synth.masterVolume.toFixed(2))}
                    ${synthSliderRow('hzScale', 'frequency multiplier', this.synth.hzScale, 1000, 50000, 1000, this.synth.hzScale)}
                    <div style="border-top:1px solid rgba(255,255,255,0.05);margin:8px 0;padding-top:4px;"></div>
                    ${synthToggleRow('additiveEnabled', 'additive spectral sonification', this.synth.additiveEnabled)}
                    ${synthToggleRow('reverbEnabled', 'lattice space reverb', this.synth.reverbEnabled)}
                    ${synthToggleRow('fmEnabled', 'FM synthesis (flux index)', this.synth.fmEnabled)}
                    <div style="border-top:1px solid rgba(255,255,255,0.05);margin:8px 0;padding-top:4px;"></div>
                    ${synthToggleRow('panningEnabled', 'spatial panning (X centroid)', this.synth.panningEnabled)}
                    ${synthToggleRow('filterEnabled', 'dynamic lowpass (total energy)', this.synth.filterEnabled)}
                    ${synthToggleRow('detuneEnabled', 'detune / chorus effect', this.synth.detuneEnabled)}
                    ${synthToggleRow('tremoloEnabled', 'tremolo (wave vel W)', this.synth.tremoloEnabled)}
                    <div style="margin-top:6px;color:var(--text-muted);font-size:11px;line-height:1.35;">
                        Controls the raw Web Audio API node. Does not affect lattice physics.
                    </div>
                </div>
            `;
        }
    }

    _handleControlInput(e) {
        const synthInput = e.target.closest('[data-synth-control]');
        if (synthInput) {
            const name = synthInput.dataset.synthControl;
            if (synthInput.type === 'checkbox') {
                this.synth[name] = synthInput.checked;
            } else if (name === 'hzScale') {
                this.synth.hzScale = parseFloat(synthInput.value);
            } else if (name === 'masterVolume') {
                this.synth.masterVolume = parseFloat(synthInput.value);
            } else if (name === 'oscType') {
                this.synth.setOscType(synthInput.value);
            }
            const display = this.refs.controls.querySelector(`[data-synth-value="${name}"]`);
            if (display) {
                if (name === 'hzScale') display.textContent = `${Math.round(this.synth.hzScale)}`;
                if (name === 'masterVolume') display.textContent = `${this.synth.masterVolume.toFixed(2)}`;
            }
            return;
        }

        const input = e.target.closest('[data-wave-control]');
        if (!input || !isSingleWaveScenario(this.scenarioId)) return;
        const name = input.dataset.waveControl;
        const raw = parseFloat(input.value);
        const value = name === 'modeN' ? Math.round(raw) : raw;
        const patch = { [name]: value };
        const settings = setWaveScenarioSettings(this.scenarioId, patch);
        const display = this.refs.controls.querySelector(`[data-wave-value="${name}"]`);
        if (display && settings) display.textContent = formatControlValue(name, settings[name]);
        this._scheduleReseed();
    }

    _resetControls() {
        if (!isSingleWaveScenario(this.scenarioId)) return;
        resetWaveScenarioSettings(this.scenarioId);
        this.controlKey = '';
        this._renderControls({ singleScenario: true, latticeSize: this.bridgeRef?.latticeSize || 33 }, { id: 'reset' }, true);
        this._scheduleReseed();
    }

    _scheduleReseed() {
        if (this.reseedHandle) return;
        const run = () => {
            this.reseedHandle = 0;
            this._reseedCurrent();
        };
        if (typeof requestAnimationFrame === 'function') {
            this.reseedHandle = requestAnimationFrame(run);
        } else {
            this.reseedHandle = setTimeout(run, 0);
        }
    }

    _reseedCurrent() {
        const bridge = this.bridgeRef;
        const scenarioId = this.scenarioId;
        if (!bridge || !isSingleWaveScenario(scenarioId)) return;
        if (typeof bridge.setupScenario === 'function') bridge.setupScenario(scenarioId);
        else bridge.capabilities?.scale0?.setupScenario?.(scenarioId);
        setLatticeNeedsUpload(true);
        markFieldDirty();
    }
}
