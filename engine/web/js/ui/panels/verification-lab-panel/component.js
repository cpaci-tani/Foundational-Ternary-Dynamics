/**
 * Verification Lab panel — wires the experiment catalog into the UI.
 *
 * Responsibilities:
 *   - Render category pills, experiment list, and detail pane from the registry
 *   - Manage per-experiment run state (trials, progress, results, badge)
 *   - Run experiments via the ExperimentRunner with live overlay auto-enable
 *   - Export CSV/JSON per experiment and all-results JSON globally
 */

import { CATEGORIES, EXPERIMENTS, experimentsByCategory, getExperiment } from '../../../verification/registry.js';
import { ExperimentRunner, defaultAggregate } from '../../../verification/runner.js';
import { computeBadge, BADGE } from '../../../verification/badge.js';
import { toCSV, toJSON, downloadBlob, copyToClipboard } from '../../../verification/export.js';
import { getVerificationLabPanelTemplate, getExperimentRowTemplate, getDetailTemplate } from './template.js';

// ── Per-experiment state store ──────────────────────────────────────
// Map<experimentId, { results, aggregate, badge, running, progress, trialIndex, totalTrials }>
const EXP_STATE = new Map();

function getState(expId) {
    if (!EXP_STATE.has(expId)) {
        EXP_STATE.set(expId, {
            results: null, aggregate: null, badge: BADGE.NOT_RUN,
            running: false, progress: null, trialIndex: 0, totalTrials: 0,
        });
    }
    return EXP_STATE.get(expId);
}

function setBadgeFromAggregate(exp, state) {
    const measured = state.aggregate?.mean;
    const theory = exp.theoryFn?.().value;
    state.badge = computeBadge(measured, theory, exp.tolerance, exp.epistemicTag);
}

// ── Component ───────────────────────────────────────────────────────

export class VerificationLabComponent {
    constructor({ panelArea, getCtx, onActivateOverlay }) {
        this.panelArea = panelArea;
        this.getCtx = getCtx;  // () => app ctx (bridge, viewport, Scale0Controller, etc.)
        this.onActivateOverlay = onActivateOverlay || (() => {});
        this.activeCategory = CATEGORIES[0].id;
        this.activeExperimentId = null;
        this.runner = new ExperimentRunner();
    }

    init() {
        this._ensurePanel();
        this._bindCategoryPills();
        this._renderExperimentList();
        return this;
    }

    _ensurePanel() {
        if (!this.panelArea) return;
        if (this.panelArea.querySelector('#panel-verification-lab')) return;
        const tpl = document.createElement('template');
        tpl.innerHTML = getVerificationLabPanelTemplate().trim();
        this.panelArea.appendChild(tpl.content.firstElementChild);
    }

    _bindCategoryPills() {
        const panel = this.panelArea?.querySelector('#panel-verification-lab');
        if (!panel) return;
        panel.querySelectorAll('.verif-cat-pill').forEach((pill) => {
            pill.addEventListener('click', () => {
                const cat = pill.dataset.verifCat;
                if (cat === this.activeCategory) return;
                this.activeCategory = cat;
                panel.querySelectorAll('.verif-cat-pill').forEach((p) => {
                    p.classList.toggle('active', p.dataset.verifCat === cat);
                });
                this._renderExperimentList();
            });
        });
        panel.querySelector('#verif-export-all-json')?.addEventListener('click', () => this._exportAll());
    }

    _renderExperimentList() {
        const host = this.panelArea?.querySelector('#verif-experiment-list');
        if (!host) return;
        const exps = experimentsByCategory(this.activeCategory);
        host.innerHTML = exps.map((e) => {
            const st = getState(e.id);
            return getExperimentRowTemplate(e, st.running ? 'running' : 'idle', st.badge);
        }).join('');
        host.querySelectorAll('.verif-row').forEach((row) => {
            row.addEventListener('click', () => this._selectExperiment(row.dataset.verifId));
        });
        // Auto-select first in category
        if (exps.length > 0 && (!this.activeExperimentId || !exps.some((e) => e.id === this.activeExperimentId))) {
            this._selectExperiment(exps[0].id);
        }
    }

    _selectExperiment(id) {
        this.activeExperimentId = id;
        // Highlight the row
        this.panelArea.querySelectorAll('.verif-row').forEach((r) => {
            r.classList.toggle('active', r.dataset.verifId === id);
        });
        this._renderDetail();
    }

    _renderDetail() {
        const detail = this.panelArea?.querySelector('#verif-experiment-detail');
        if (!detail) return;
        const exp = getExperiment(this.activeExperimentId);
        if (!exp) {
            detail.innerHTML = '<div class="verif-detail-empty">Select an experiment to view details.</div>';
            return;
        }
        const state = getState(exp.id);
        detail.innerHTML = getDetailTemplate(exp, state);
        this._bindDetailControls(exp);
        if (state.results?.length) this._drawSparkline(state.results);
    }

    _bindDetailControls(exp) {
        const runBtn   = this.panelArea.querySelector('#verif-run-btn');
        const abortBtn = this.panelArea.querySelector('#verif-abort-btn');
        const csvBtn   = this.panelArea.querySelector('#verif-export-csv');
        const jsonBtn  = this.panelArea.querySelector('#verif-export-json');
        const copyBtn  = this.panelArea.querySelector('#verif-copy-json');
        runBtn?.addEventListener('click', () => this._runExperiment(exp));
        abortBtn?.addEventListener('click', () => this.runner.abort());
        csvBtn?.addEventListener('click', () => {
            const state = getState(exp.id);
            downloadBlob(`${exp.id}.csv`, toCSV(exp, state.results, state.aggregate), 'text/csv');
        });
        jsonBtn?.addEventListener('click', () => {
            const state = getState(exp.id);
            downloadBlob(`${exp.id}.json`, toJSON(exp, state.results, state.aggregate), 'application/json');
        });
        copyBtn?.addEventListener('click', async () => {
            const state = getState(exp.id);
            await copyToClipboard(toJSON(exp, state.results, state.aggregate));
            copyBtn.textContent = 'Copied ✓';
            setTimeout(() => { copyBtn.textContent = 'Copy'; }, 1200);
        });
    }

    async _runExperiment(exp) {
        const state = getState(exp.id);
        const trialsInput = this.panelArea.querySelector('#verif-trials-input');
        const ticksInput  = this.panelArea.querySelector('#verif-ticks-input');
        const totalTrials = parseInt(trialsInput?.value, 10) || exp.defaultTrials;
        const ticksPerTrial = parseInt(ticksInput?.value, 10) || exp.defaultTicksPerTrial;

        const ctx = this.getCtx?.();
        if (!ctx?.bridge) { console.warn('[verif] no bridge in ctx'); return; }

        // Load the scenario + auto-enable overlays BEFORE starting trials
        try {
            if (ctx.scale0Controller?.loadScenario) {
                ctx.scale0Controller.loadScenario(ctx, exp.scenarioId);
            } else if (ctx.loadScale0Scenario) {
                ctx.loadScale0Scenario(exp.scenarioId);
            }
        } catch (e) { console.warn('[verif] loadScenario failed', e); }

        if (Array.isArray(exp.overlays)) {
            for (const overlayId of exp.overlays) this.onActivateOverlay(overlayId);
        }

        // Pause the app's RAF loop while we drive the bridge directly
        if (ctx.setRunning) ctx.setRunning(false);

        state.running = true;
        state.results = [];
        state.aggregate = null;
        state.progress = 0;
        state.trialIndex = 0;
        state.totalTrials = totalTrials;
        this._renderDetail();
        this._updateRowStatus(exp.id);

        this.runner.configure({
            scenarioId: exp.scenarioId,
            totalTrials, ticksPerTrial,
            resetFn: exp.resetFn,
            measureFn: exp.measureFn,
        });

        await this.runner.runAll(ctx.bridge, {
            onProgress: (i, total) => {
                state.progress = i / total;
                state.trialIndex = i;
                // Throttle DOM re-renders — every 2 trials
                if (i % 2 === 0 || i === total) this._renderDetail();
            },
            onComplete: (results) => {
                state.results = results;
                state.aggregate = (exp.aggregateFn || defaultAggregate)(results);
                setBadgeFromAggregate(exp, state);
                state.running = false;
                state.progress = 1;
                this._renderDetail();
                this._updateRowStatus(exp.id);
            },
            onError: (err) => {
                console.warn('[verif] run failed', err);
                state.running = false;
                state.results = null;
                state.badge = BADGE.NOT_RUN;
                this._renderDetail();
                this._updateRowStatus(exp.id);
            },
        });
    }

    _updateRowStatus(expId) {
        const row = this.panelArea.querySelector(`.verif-row[data-verif-id="${expId}"]`);
        if (!row) return;
        const state = getState(expId);
        const badgeEl = row.querySelector('.verif-row-badge');
        if (badgeEl) {
            badgeEl.className = 'verif-row-badge verif-badge-' + state.badge.toLowerCase().replace('_', '-');
            badgeEl.textContent = rowBadgeGlyph(state.badge);
        }
        row.dataset.status = state.running ? 'running' : 'idle';
    }

    _drawSparkline(results) {
        const canvas = this.panelArea.querySelector('#verif-sparkline');
        if (!canvas) return;
        const dpr = window.devicePixelRatio || 1;
        const rect = canvas.getBoundingClientRect();
        canvas.width = rect.width * dpr;
        canvas.height = rect.height * dpr;
        const ctx = canvas.getContext('2d');
        ctx.scale(dpr, dpr);
        ctx.clearRect(0, 0, rect.width, rect.height);
        const values = results.map((r) => (typeof r === 'number' ? r : r?.value ?? 0))
                              .filter((v) => Number.isFinite(v));
        if (values.length === 0) return;
        const min = Math.min(...values);
        const max = Math.max(...values);
        const span = Math.max(max - min, 1e-9);
        const accent = getComputedStyle(document.documentElement).getPropertyValue('--accent').trim() || '#00e5ff';
        ctx.strokeStyle = accent;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        for (let i = 0; i < values.length; i++) {
            const x = (i / Math.max(values.length - 1, 1)) * rect.width;
            const y = rect.height - ((values[i] - min) / span) * rect.height;
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.stroke();
    }

    _exportAll() {
        const blob = {};
        for (const exp of EXPERIMENTS) {
            const st = getState(exp.id);
            blob[exp.id] = {
                name: exp.name, category: exp.category, epistemicTag: exp.epistemicTag,
                theory: exp.theoryFn?.(), tolerance: exp.tolerance,
                badge: st.badge, trials: st.results?.length || 0, aggregate: st.aggregate,
            };
        }
        downloadBlob('verification-lab-all.json', JSON.stringify(blob, null, 2), 'application/json');
    }
}

function rowBadgeGlyph(badge) {
    switch (badge) {
        case BADGE.PASS:     return '✓';
        case BADGE.CLOSE:    return '~';
        case BADGE.FAIL:     return '✗';
        case BADGE.EMERGENT: return '●';
        default:             return '—';
    }
}

export function initVerificationLabPanel({ panelArea, getCtx, onActivateOverlay }) {
    return new VerificationLabComponent({ panelArea, getCtx, onActivateOverlay }).init();
}
