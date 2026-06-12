/**
 * Ontic Observatory — Makes the Ontic Incompleteness narrative visible.
 *
 * Surfaces 5 propositions attributed to Steinmetz (2026), an
 * "Ontic Incompleteness" framing that has NOT been audited into FTD's
 * LEDGER and has no entry in docs/papers/. The labels below are
 * intentionally demoted from "Theorem"/"Corollary" to "Proposition"
 * (audit P1-13 fix, 2026-05-27) until a canonical provenance lands.
 *
 *   Proposition I  (Ontic Incompleteness):           C_0 not derivable from f
 *   Proposition II (Ontological Separation):         Complete spec = (f, C_0)
 *   Proposition III (Boundary Information Dominance): K(C_0) >> K(f)
 *   Proposition IV (The Trilemma):                   any selector for C_0 is
 *                                                    nonlocal, translation-
 *                                                    variant, or carries
 *                                                    system-scale info
 *   Proposition V  (No Premature Emergence):         Level 3 needs |R|>>1, t>>1
 *   Remark         (First Perturbation):             P_0 = C_0 - V
 *
 * The user is positioned as the external observer of the framing.
 */

import {
    ONTIC_TOTAL_CONSTANTS, TICK_PHASES,
} from './constants.js';

// ── Kolmogorov complexity estimates ──────────────────────────────────

// K(f): complexity of the update rule
// 6 tick phases + ~32 ontic constants, each encoded at ~10 bits precision
const K_F_BITS = Math.ceil(Math.log2(TICK_PHASES.length)) + ONTIC_TOTAL_CONSTANTS * 10;

// Bits per entity at each scale
const BITS_PER_LATTICE_PARTICLE = 3 + 3 * 32; // 3 bits state + 3x32 flux
const BITS_PER_PE_PARTICLE = 3 * 64 + 3 * 64 + 64 + 64; // pos + vel + charge + mass
const BITS_PER_AE_ATOM = 7 + 8 + 3 * 64 + 3 * 64 + 16;  // Z + N + pos + vel + bonds

export class OnticObservatory {
    constructor() {
        this._scenarioName = 'Empty Lattice';
        this._scale = 0;
        this._tick = 0;
        this._particleCount = 0;
        this._boundCount = 0;
        this._totalFlux = 0;
        this._chargeBalance = 0;
        this._spatialExtent = 0;
        this._latticeSize = 32;
        this._history = []; // last 500 frames for emergence monitor
    }

    setScenario(name, scale, latticeSize) {
        this._scenarioName = name;
        this._scale = scale;
        this._latticeSize = latticeSize || 32;
    }

    // ── Information-theoretic computations ───────────────────────────

    /** K(f) — approximate Kolmogorov complexity of the update rule */
    lawComplexity() {
        return K_F_BITS;
    }

    /** K(C_0) — approximate Kolmogorov complexity of boundary conditions */
    boundaryComplexity() {
        const n = this._particleCount;
        if (n === 0) return 0;
        switch (this._scale) {
            case 0: return n * BITS_PER_LATTICE_PARTICLE;
            case 1: return n * BITS_PER_PE_PARTICLE;
            case 2: return n * BITS_PER_AE_ATOM;
            default: return n * BITS_PER_LATTICE_PARTICLE;
        }
    }

    /** log_2|Omega| — total configuration space (Proposition III upper bound) */
    configSpaceSize() {
        switch (this._scale) {
            case 0: {
                const sites = this._latticeSize ** 3;
                return sites * Math.log2(3); // 3 states per site
            }
            case 1: return this._particleCount * 6 * 64; // 6 DOF at 64-bit each
            case 2: return this._particleCount * 6 * 64;
            default: return 1;
        }
    }

    /** Boundary Information Dominance ratio — Proposition III */
    dominanceRatio() {
        const kf = this.lawComplexity();
        const kc0 = this.boundaryComplexity();
        return kf > 0 ? kc0 / kf : 0;
    }

    /** Information spectrum: { kf, kc0, logOmega } — Equation 1 */
    spectrum() {
        return {
            kf: this.lawComplexity(),
            kc0: this.boundaryComplexity(),
            logOmega: this.configSpaceSize(),
        };
    }

    // ── First Perturbation P_0 = C_0 - V (Remark / heuristic) ──────────────

    firstPerturbation() {
        return {
            nonVoidCount: this._particleCount,
            totalFlux: this._totalFlux,
            chargeBalance: this._chargeBalance,
            spatialExtent: this._spatialExtent,
        };
    }

    // ── Trilemma status (Proposition IV) ─────────────────────────────

    trilemmaStatus() {
        // The user IS the external observer. Their scenario choice carries
        // system-scale information (horn 3 of the trilemma).
        return {
            horn: 3,
            label: 'System-Scale Information',
            explanation: `You chose "${this._scenarioName}" \u2014 that choice carries `
                + `${this.boundaryComplexity()} bits, far exceeding the `
                + `${this.lawComplexity()}-bit update rule.`,
        };
    }

    // ── Aggregation Hierarchy (heuristic) ──────────────────────────

    aggregationLevels() {
        const spatialNorm = this._spatialExtent / this._latticeSize;
        const temporalDepth = this._tick / 100; // normalized to relaxation estimate

        return [
            { level: 0, name: 'Informational',   active: true,
              detail: 'Coordinates exist' },
            { level: 1, name: 'Locational',       active: this._particleCount > 0,
              detail: `${this._particleCount} manifested` },
            { level: 2, name: 'Configurational',  active: this._boundCount > 0,
              detail: `${this._boundCount} bound structures` },
            { level: 3, name: 'Emergent',          active: spatialNorm > 0.1 && temporalDepth > 10,
              detail: `|R|=${spatialNorm.toFixed(2)}, t=${temporalDepth.toFixed(1)}` },
        ];
    }

    /** Proposition V check: No Premature Emergence */
    theoremA1() {
        const spatialNorm = this._spatialExtent / this._latticeSize;
        const temporalDepth = this._tick / 100;
        return {
            satisfied: spatialNorm > 0.1 && temporalDepth > 10,
            spatialExtent: spatialNorm,
            temporalDepth: temporalDepth,
            thresholdR: 0.1,
            thresholdT: 10,
        };
    }

    // ── Update from live diagnostics ────────────────────────────────

    update(diag, scale, tick) {
        this._scale = scale;
        this._tick = tick || 0;

        if (scale === 0) {
            this._particleCount = diag.manifested || 0;
            this._totalFlux = diag.totalFlux || 0;
            this._chargeBalance = diag.chargeBalance || 0;
            this._boundCount = diag.locked || 0;
            this._spatialExtent = diag.spatialExtent || 0;
        } else if (scale === 1) {
            this._particleCount = diag.count || 0;
            this._totalFlux = diag.totalEnergy || 0;
            this._chargeBalance = 0;
            this._boundCount = 0;
            this._spatialExtent = diag.maxSep || 0;
        } else if (scale === 2) {
            this._particleCount = diag.count || 0;
            this._totalFlux = diag.totalEnergy || 0;
            this._chargeBalance = 0;
            this._boundCount = diag.bondCount || 0;
            this._spatialExtent = diag.maxSep || 0;
        }

        // Record for emergence monitor (capped at 500)
        this._history.push({
            tick: this._tick,
            spatialExtent: this._spatialExtent / this._latticeSize,
            temporalDepth: this._tick / 100,
            level: this.aggregationLevels().reduce((m, l) => l.active ? l.level : m, 0),
            particleCount: this._particleCount,
            boundCount: this._boundCount,
        });
        if (this._history.length > 500) this._history.shift();
    }

    /** Get emergence trajectory for scatter plot */
    emergenceTrajectory() {
        return this._history;
    }

    /** Reset history (e.g. on scenario change) */
    reset() {
        this._history = [];
    }
}

// ── DOM rendering helpers ───────────────────────────────────────────

/** Render the (f, C_0) ordered pair card */
export function renderFcCard(obs, container) {
    const sp = obs.spectrum();
    const maxBar = Math.max(sp.kf, sp.kc0, 1);

    container.innerHTML = `
        <div class="card-title">(f, C<sub>0</sub>) Ordered Pair <span class="oo-card-title-sub">\u2014 Proposition II</span></div>
        <div class="oo-fc-grid">
            <div>
                <div class="oo-fc-label">f = Update Rule</div>
                <div class="oo-fc-value accent">${TICK_PHASES.length}-Phase Tick</div>
                <div class="oo-fc-sub">K(f) = ${sp.kf} bits</div>
                <div class="oo-bar-track">
                    <div class="oo-bar-fill accent" style="width:${(sp.kf / maxBar * 100).toFixed(0)}%"></div>
                </div>
            </div>
            <div>
                <div class="oo-fc-label">C<sub>0</sub> = Boundary Datum</div>
                <div class="oo-fc-value warning">${obs._scenarioName}</div>
                <div class="oo-fc-sub">K(C<sub>0</sub>) = ${sp.kc0} bits</div>
                <div class="oo-bar-track">
                    <div class="oo-bar-fill warning" style="width:${(sp.kc0 / maxBar * 100).toFixed(0)}%"></div>
                </div>
            </div>
        </div>
        <div class="oo-fc-footer">
            Complete specification requires BOTH \u2014 neither determines the other
        </div>`;
}

/** Render the observer panel card */
export function renderObserverCard(obs, container) {
    const p0 = obs.firstPerturbation();
    const tri = obs.trilemmaStatus();

    container.innerHTML = `
        <div class="card-title">Observer Panel <span class="oo-card-title-sub">\u2014 Heuristic</span></div>
        <div class="oo-obs-role">
            You are the external observer providing C<sub>0</sub>
        </div>
        <div class="oo-obs-hint">
            <strong>First Perturbation P<sub>0</sub></strong> (Remark / heuristic): C<sub>0</sub> \u2212 V
        </div>
        <div class="oo-obs-grid">
            <span class="oo-obs-grid-label">Manifested</span>
            <span class="oo-obs-grid-val">${p0.nonVoidCount}</span>
            <span class="oo-obs-grid-label">Total Flux</span>
            <span class="oo-obs-grid-val">${p0.totalFlux.toFixed(2)}</span>
            <span class="oo-obs-grid-label">Charge Balance</span>
            <span class="oo-obs-grid-val">${p0.chargeBalance}</span>
        </div>
        <div class="oo-trilemma-box">
            <span class="oo-trilemma-label">Trilemma Horn ${tri.horn}:</span>
            <span class="oo-trilemma-desc">${tri.label}</span>
        </div>`;
}

/** Render the aggregation hierarchy tower */
export function renderHierarchyTower(obs, container) {
    const levels = obs.aggregationLevels();
    const colors = ['#60a5fa', '#4ade80', '#fbbf24', '#f87171'];
    const highestActive = levels.reduce((m, l) => l.active ? l.level : m, -1);

    let html = `<div class="card-title">Aggregation Hierarchy <span class="oo-card-title-sub">\u2014 Heuristic</span></div>`;
    // Render top-to-bottom (Level 3 at top)
    for (let i = levels.length - 1; i >= 0; i--) {
        const l = levels[i];
        const bg = l.active ? colors[l.level] : 'var(--bg-input)';
        const fg = l.active ? '#000' : 'var(--text-muted)';
        const pulseClass = (l.level === highestActive && l.active) ? 'is-pulsing' : '';
        html += `<div class="oo-hierarchy-row ${pulseClass}" style="background:${bg};color:${fg};">
            <span>L${l.level}: ${l.name}</span>
            <span class="oo-hierarchy-detail">${l.detail}</span>
        </div>`;
    }
    container.innerHTML = html;
}

/** Render the information dynamics card */
export function renderInfoDynamics(obs, container) {
    const sp = obs.spectrum();
    const ratio = obs.dominanceRatio();
    const maxVal = Math.max(sp.kf, sp.kc0, Math.min(sp.logOmega, 1e6), 1);
    // Clamp logOmega bar for display (can be astronomical)
    const barOmega = Math.min(sp.logOmega / maxVal * 100, 100);

    container.innerHTML = `
        <div class="card-title">Information Dynamics <span class="oo-card-title-sub">\u2014 Proposition III</span></div>
        <div class="oo-info-hint">
            K(f) \u226A K(C<sub>0</sub>) \u226A log<sub>2</sub>|\u03A9|
        </div>
        <div class="oo-info-row">
            <span class="oo-info-label accent">K(f)</span>
            <span class="oo-info-val">${sp.kf} bits</span>
            <div class="oo-bar-track thin">
                <div class="oo-bar-fill thin accent" style="width:${(sp.kf / maxVal * 100).toFixed(0)}%"></div>
            </div>
        </div>
        <div class="oo-info-row">
            <span class="oo-info-label warning">K(C<sub>0</sub>)</span>
            <span class="oo-info-val">${sp.kc0} bits</span>
            <div class="oo-bar-track thin">
                <div class="oo-bar-fill thin warning" style="width:${(sp.kc0 / maxVal * 100).toFixed(0)}%"></div>
            </div>
        </div>
        <div class="oo-info-row">
            <span class="oo-info-label muted">log<sub>2</sub>|\u03A9|</span>
            <span class="oo-info-val">${sp.logOmega > 1e6 ? sp.logOmega.toExponential(1) : sp.logOmega.toFixed(0)} bits</span>
            <div class="oo-bar-track thin">
                <div class="oo-bar-fill thin muted" style="width:${barOmega.toFixed(0)}%"></div>
            </div>
        </div>
        <div class="oo-info-footer">
            Boundary dominates law by <span class="oo-info-ratio">${ratio.toFixed(1)}x</span>
        </div>`;
}
