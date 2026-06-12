/**
 * Aggregation Bridge Module — Appendix A of cpaci-tani (2026).
 *
 * Implements the 4-level aggregation hierarchy and emergence monitoring.
 * The user (external observer) provides C_0; the system evolves via f.
 * Theorem A.1: Level 3 emergence requires |R|>>1 AND t>>1.
 *
 * Three classes:
 *   AggregateDetector   — detect which aggregation levels are active
 *   ScaleBridgeVisualizer — map levels to simulation scales
 *   EmergenceMonitor     — track emergence trajectory over time
 */

// ── Aggregation Level Definitions (Appendix A) ──────────────────────

export const AGGREGATION_LEVELS = [
    { level: 0, name: 'Informational',    color: '#3498db',
      description: 'Raw coordinates exist — mathematical substrate' },
    { level: 1, name: 'Locational',       color: '#2ecc71',
      description: 'Manifested states present (s ≠ 0)' },
    { level: 2, name: 'Configurational',  color: '#f1c40f',
      description: 'Bound structures detected (locked triads / bonds)' },
    { level: 3, name: 'Emergent',         color: '#e74c3c',
      description: 'Requires |R|>>1 AND t>>1 (Theorem A.1)' },
];

// Theorem A.1 thresholds
const SPATIAL_THRESHOLD = 0.1;   // |R| / lattice_size > this for ">>1"
const TEMPORAL_THRESHOLD = 10.0; // tick / relaxation_time > this for ">>1"

// ── AggregateDetector ────────────────────────────────────────────────

export class AggregateDetector {
    /**
     * Detect which aggregation levels are active.
     *
     * @param {object} data - simulation diagnostics
     * @param {number} data.tick          - current tick count
     * @param {number} data.particleCount - manifested entities
     * @param {number} data.boundCount    - locked/bonded structures
     * @param {number} data.latticeSize   - lattice dimension N
     * @param {number} data.spatialExtent - max pairwise distance (0..1 normalized)
     * @param {number} [data.relaxTime]   - estimated relaxation time
     * @returns {{ levels: boolean[], details: object[] }}
     */
    detect(data) {
        const {
            tick = 0,
            particleCount = 0,
            boundCount = 0,
            spatialExtent = 0,
            relaxTime = 100
        } = data;

        const spatialNorm = spatialExtent;
        const temporalDepth = relaxTime > 0 ? tick / relaxTime : 0;

        const levels = [
            true,                                    // L0: always active
            particleCount > 0,                       // L1: manifested entities
            boundCount > 0,                          // L2: bound structures
            spatialNorm > SPATIAL_THRESHOLD &&        // L3: Theorem A.1
                temporalDepth > TEMPORAL_THRESHOLD
        ];

        const details = [
            { text: 'Coordinates defined', metric: '∞' },
            { text: `${particleCount} manifested`, metric: particleCount },
            { text: `${boundCount} bound`, metric: boundCount },
            {
                text: `|R|=${spatialNorm.toFixed(3)}, t/τ=${temporalDepth.toFixed(1)}`,
                metric: levels[3] ? 'ACTIVE' : 'pending',
                spatialNorm,
                temporalDepth,
                spatialThreshold: SPATIAL_THRESHOLD,
                temporalThreshold: TEMPORAL_THRESHOLD,
            }
        ];

        return { levels, details };
    }

    /**
     * Get highest active level (0-3).
     */
    highestLevel(data) {
        const { levels } = this.detect(data);
        for (let i = 3; i >= 0; i--) {
            if (levels[i]) return i;
        }
        return 0;
    }
}

// ── ScaleBridgeVisualizer ────────────────────────────────────────────

export class ScaleBridgeVisualizer {
    /**
     * Map aggregation levels to simulation scales.
     */
    getScaleMapping() {
        return [
            { level: 0, scale: -1, label: 'Pre-simulation',
              description: 'Scenario selection — observer provides C₀' },
            { level: 1, scale: 0,  label: 'Scale 0 — Lattice',
              description: 'Individual voxels with state {-1,0,+1}' },
            { level: 2, scale: 1,  label: 'Scale 1 — Particles',
              description: 'Continuous particle aggregates' },
            { level: 3, scale: 2,  label: 'Scale 2 — Atoms',
              description: 'Composite atoms, bonds, molecular structures' },
        ];
    }

    /**
     * Get OnticEntity triple {state, energy, boundary} for any entity.
     *
     * @param {number} scale - simulation scale (0, 1, 2)
     * @param {object} entity - entity data from that scale
     */
    getOnticEntity(scale, entity) {
        switch (scale) {
            case 0: return {
                state: entity.state || 0,
                energy: entity.density || 0,
                boundary: entity.isLocked ? 'locked' : 'free',
            };
            case 1: return {
                state: entity.charge > 0 ? +1 : entity.charge < 0 ? -1 : 0,
                energy: entity.mass || K_B,
                boundary: entity.rEff || 0,
            };
            case 2: return {
                state: entity.Z || 0,
                energy: entity.mass || 0,
                boundary: entity.bonds ? entity.bonds.length : 0,
            };
            default: return { state: 0, energy: 0, boundary: 'unknown' };
        }
    }

    /**
     * Get scale info for display.
     */
    getScaleInfo(scale, data) {
        const mapping = this.getScaleMapping();
        const info = mapping.find(m => m.scale === scale) || mapping[0];
        return {
            ...info,
            entityCount: data.particleCount || 0,
            totalEnergy: data.totalEnergy || 0,
            dominantForce: scale === 0 ? 'Flux coupling'
                         : scale === 1 ? 'Coulomb + Gravity'
                         : 'Ionic + vdW + Covalent',
        };
    }
}

// ── EmergenceMonitor ─────────────────────────────────────────────────

export class EmergenceMonitor {
    constructor(maxHistory = 500) {
        this._history = [];
        this._maxHistory = maxHistory;
    }

    /**
     * Record a frame of emergence data.
     */
    record(data) {
        const { tick, particleCount, boundCount, spatialExtent, totalEnergy, relaxTime } = data;
        const temporalDepth = relaxTime > 0 ? tick / relaxTime : 0;

        const detector = new AggregateDetector();
        const highest = detector.highestLevel(data);

        this._history.push({
            tick,
            spatialExtent: spatialExtent || 0,
            temporalDepth,
            level: highest,
            particleCount: particleCount || 0,
            boundCount: boundCount || 0,
            totalEnergy: totalEnergy || 0,
        });

        if (this._history.length > this._maxHistory) {
            this._history.shift();
        }
    }

    /**
     * Get the full emergence trajectory for plotting.
     */
    getTrajectory() {
        return this._history;
    }

    /**
     * Check if Theorem A.1 conditions are currently satisfied.
     */
    checkTheoremA1() {
        if (this._history.length === 0) return { satisfied: false };
        const last = this._history[this._history.length - 1];
        return {
            satisfied: last.spatialExtent > SPATIAL_THRESHOLD &&
                       last.temporalDepth > TEMPORAL_THRESHOLD,
            spatialExtent: last.spatialExtent,
            temporalDepth: last.temporalDepth,
            spatialThreshold: SPATIAL_THRESHOLD,
            temporalThreshold: TEMPORAL_THRESHOLD,
        };
    }

    /**
     * Flag any premature emergence violations.
     * Returns frames where Level 3 was claimed but A.1 not satisfied.
     */
    getPrematureViolations() {
        return this._history.filter(f =>
            f.level >= 3 &&
            (f.spatialExtent <= SPATIAL_THRESHOLD || f.temporalDepth <= TEMPORAL_THRESHOLD)
        );
    }

    /**
     * Reset the monitor.
     */
    reset() {
        this._history = [];
    }

    get length() {
        return this._history.length;
    }
}

// ── DOM Rendering ────────────────────────────────────────────────────

/**
 * Render the aggregation levels tower into a container.
 * 4-level vertical stack, each lights up when active.
 */
export function renderAggregationTower(levels, details, container) {
    let bars = '';
    for (let i = 3; i >= 0; i--) {
        const lvl = AGGREGATION_LEVELS[i];
        const active = levels[i];
        const det = details[i];
        const opacity = active ? 1.0 : 0.25;
        const pulseClass = active && i === Math.max(...levels.map((v, j) => v ? j : -1))
            ? 'is-pulsing' : '';

        bars += `
            <div class="agg-row ${pulseClass}" style="background:${lvl.color}22;opacity:${opacity}">
                <div class="agg-dot" style="background:${active ? lvl.color : 'var(--text-muted)'};box-shadow:${active ? `0 0 6px ${lvl.color}` : 'none'}"></div>
                <div class="agg-content">
                    <div style="color:${active ? lvl.color : 'var(--text-muted)'}">
                        L${lvl.level}: ${lvl.name}
                    </div>
                    <div class="agg-desc">${det.text}</div>
                </div>
                <div class="agg-metric">${det.metric}</div>
            </div>`;
    }

    container.innerHTML = `
        <div class="card-title">Aggregation Hierarchy (Appendix A)</div>
        <div class="agg-tower-container">${bars}</div>
        <div class="agg-footer">
            Level 3 requires |R| > ${SPATIAL_THRESHOLD} AND t/τ > ${TEMPORAL_THRESHOLD} (Theorem A.1)
        </div>`;
}

/**
 * Render scale bridge visualization.
 */
export function renderScaleBridge(activeScale, data, container) {
    const viz = new ScaleBridgeVisualizer();
    const scales = [
        viz.getScaleInfo(0, data),
        viz.getScaleInfo(1, data),
        viz.getScaleInfo(2, data),
    ];

    let cols = '';
    for (let i = 0; i < 3; i++) {
        const s = scales[i];
        const isActive = i === activeScale;
        const activeClass = isActive ? 'is-active' : 'is-inactive';

        cols += `
            <div class="sb-col ${activeClass}">
                <div class="sb-label">${s.label}</div>
                <div class="sb-count">${s.entityCount}</div>
                <div class="sb-force">${s.dominantForce}</div>
            </div>`;

        if (i < 2) {
            cols += `<div class="sb-arrow">⇄</div>`;
        }
    }

    container.innerHTML = `
        <div class="card-title">Scale Bridge (OnticEntity mapping)</div>
        <div class="sb-container">${cols}</div>
        <div class="agg-footer">
            Each entity = {state, energy, boundary} — universal triple across all scales
        </div>`;
}

/**
 * Render emergence scatter plot into a container.
 * X = spatial extent, Y = temporal depth, color by aggregation level.
 */
export function renderEmergenceMonitor(trajectory, container) {
    const W = 360, H = 180;
    const pad = { top: 20, right: 15, bottom: 25, left: 40 };
    const plotW = W - pad.left - pad.right;
    const plotH = H - pad.top - pad.bottom;

    let svg = `<svg viewBox="0 0 ${W} ${H}" class="em-svg">`;

    // Axes
    svg += `<line x1="${pad.left}" y1="${pad.top}" x2="${pad.left}" y2="${H - pad.bottom}" stroke="var(--text-muted)" stroke-width="0.5"/>`;
    svg += `<line x1="${pad.left}" y1="${H - pad.bottom}" x2="${W - pad.right}" y2="${H - pad.bottom}" stroke="var(--text-muted)" stroke-width="0.5"/>`;

    // Axis labels
    svg += `<text x="${W / 2}" y="${H - 3}" fill="var(--text-muted)" text-anchor="middle" font-size="8">|R| (spatial extent)</text>`;
    svg += `<text x="8" y="${H / 2}" fill="var(--text-muted)" text-anchor="middle" font-size="8" transform="rotate(-90,8,${H / 2})">t/τ (temporal)</text>`;

    // Threshold lines
    const xThresh = pad.left + SPATIAL_THRESHOLD * plotW;
    const yThresh = H - pad.bottom - (TEMPORAL_THRESHOLD / 50) * plotH;
    svg += `<line x1="${xThresh}" y1="${pad.top}" x2="${xThresh}" y2="${H - pad.bottom}" stroke="var(--warning)" stroke-width="0.5" stroke-dasharray="3,3"/>`;
    svg += `<line x1="${pad.left}" y1="${yThresh}" x2="${W - pad.right}" y2="${yThresh}" stroke="var(--warning)" stroke-width="0.5" stroke-dasharray="3,3"/>`;

    // Quadrant label
    svg += `<text x="${xThresh + 4}" y="${yThresh - 4}" fill="var(--positive)" font-size="7">Emergence</text>`;
    svg += `<text x="${pad.left + 4}" y="${H - pad.bottom - 4}" fill="var(--text-muted)" font-size="7">No emergence</text>`;

    // Plot points (last N frames)
    const maxSpatial = 1.0;
    const maxTemporal = 50;
    for (const pt of trajectory) {
        const x = pad.left + Math.min(pt.spatialExtent / maxSpatial, 1) * plotW;
        const y = H - pad.bottom - Math.min(pt.temporalDepth / maxTemporal, 1) * plotH;
        const color = AGGREGATION_LEVELS[Math.min(pt.level, 3)].color;
        svg += `<circle cx="${x}" cy="${y}" r="1.5" fill="${color}" opacity="0.6"/>`;
    }

    // Current point (larger)
    if (trajectory.length > 0) {
        const last = trajectory[trajectory.length - 1];
        const x = pad.left + Math.min(last.spatialExtent / maxSpatial, 1) * plotW;
        const y = H - pad.bottom - Math.min(last.temporalDepth / maxTemporal, 1) * plotH;
        const color = AGGREGATION_LEVELS[Math.min(last.level, 3)].color;
        svg += `<circle cx="${x}" cy="${y}" r="3" fill="${color}" stroke="white" stroke-width="0.5"/>`;
    }

    svg += '</svg>';

    const a1 = trajectory.length > 0 ? trajectory[trajectory.length - 1] : null;
    const status = a1 && a1.spatialExtent > SPATIAL_THRESHOLD && a1.temporalDepth > TEMPORAL_THRESHOLD
        ? '<span class="em-status-ok">Theorem A.1 SATISFIED</span>'
        : '<span class="em-status-pending">Theorem A.1 pending</span>';

    container.innerHTML = `
        <div class="card-title">Emergence Monitor (Theorem A.1)</div>
        ${svg}
        <div class="em-footer">${status} — ${trajectory.length} frames recorded</div>`;
}
