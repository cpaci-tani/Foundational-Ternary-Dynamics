/**
 * Shared card-rendering helpers for Scale 0 dock-mode panels.
 *
 * Extracted from p1-observables-panel.js so flux-slice, p1-observables,
 * spectrum, and conservation overlays all render visually identical
 * cards / titles / numerics without 3-way drift.
 *
 * All helpers return inline-style strings (not class names) for
 * compatibility with the existing inline-styled panels — a future
 * cleanup can migrate to CSS classes.
 */

// ── Card chrome ─────────────────────────────────────────────────────

/**
 * Returns the inline style string for a card section. min-height
 * prevents text-wrap reflow from jittering siblings.
 */
export function cardStyle(minHeightPx = 100) {
    return `
        margin-bottom: 12px;
        padding: 12px 14px;
        background: var(--bg-card, rgba(18, 26, 47, 0.55));
        border: 1px solid var(--border-light, rgba(255, 255, 255, 0.08));
        border-radius: 8px;
        min-height: ${minHeightPx}px;
    `;
}

/**
 * Returns the inline style string for a card title — uppercase,
 * letter-spaced, muted, with bottom rule.
 */
export function titleStyle() {
    return `
        font-size: 14px;
        font-weight: 600;
        color: var(--text-muted);
        text-transform: uppercase;
        letter-spacing: 0.04em;
        margin-bottom: 8px;
        padding-bottom: 4px;
        border-bottom: 1px solid var(--border-light, rgba(255, 255, 255, 0.06));
    `;
}

/**
 * Style for a hero number — largest visual element on the card.
 * Use for the "max |residual|", "residual %", "largest |Δ|" type
 * single most-important number per Auditor #4's hierarchy spec.
 */
export function heroStyle() {
    return `
        font-size: 18px;
        font-weight: 600;
        color: var(--accent);
        font-family: var(--font-mono);
        font-variant-numeric: tabular-nums;
        line-height: 1.2;
    `;
}

/**
 * Style for a numeric readout cell — fixed-width to prevent jitter.
 * minWidthCh is the min character width; 9ch fits "±1.23e±NN".
 */
export function numStyle(minWidthCh = 9, color = 'var(--accent)') {
    return `
        display: inline-block;
        min-width: ${minWidthCh}ch;
        text-align: right;
        font-family: var(--font-mono);
        font-variant-numeric: tabular-nums;
        color: ${color};
    `;
}

// ── Number formatting ───────────────────────────────────────────────

/**
 * Format a number as fixed-width scientific: ±1.23e±NN (8 chars).
 * Always emits this exact form so column widths never shift.
 *
 *   formatExp(0)         → ' 0.00e+00'
 *   formatExp(3.4e-12)   → ' 3.40e-12'
 *   formatExp(-1.234e-3) → '-1.23e-03'
 *   formatExp(NaN)       → '   nan   '
 */
export function formatExp(v) {
    if (!Number.isFinite(v)) return '   nan   ';
    if (v === 0) return ' 0.00e+00';
    const sign = v < 0 ? '-' : ' ';
    return sign + Math.abs(v).toExponential(2).padStart(8, ' ');
}

/**
 * Format a number as fixed-decimal: ±NN.NNN (8 chars including sign).
 * Use for percentages and ratios where scientific isn't appropriate.
 */
export function formatFixed(v, digits = 3) {
    if (!Number.isFinite(v)) return '  nan';
    const sign = v < 0 ? '-' : ' ';
    return sign + Math.abs(v).toFixed(digits);
}

// ── Status colors with hysteresis ───────────────────────────────────

/**
 * Maps an absolute drift value to one of four status tokens
 * (--positive, --warning, --caution, --negative).
 *
 * Thresholds match the conservation panel spec:
 *   |Δ| < 1e-10 → positive (green)
 *   |Δ| < 1e-6  → warning (yellow)
 *   |Δ| < 1e-3  → caution (orange)
 *   |Δ| ≥ 1e-3  → negative (red)
 */
export function statusToken(absDelta) {
    if (!Number.isFinite(absDelta)) return 'var(--text-muted)';
    const a = Math.abs(absDelta);
    if (a < 1e-10) return 'var(--positive)';
    if (a < 1e-6)  return 'var(--warning)';
    if (a < 1e-3)  return 'var(--caution)';
    return 'var(--negative)';
}

/**
 * Hysteresis state machine — call per sample. Holds the current
 * status until N consecutive samples cross the threshold in the same
 * direction. Prevents 4-Hz status-color flicker at threshold boundaries.
 *
 * Usage:
 *   const hyst = createHysteresis();
 *   const stable = hyst.update(rawStatusToken);   // returns the displayed token
 */
export function createHysteresis({ holdEscalate = 3, holdRelax = 5 } = {}) {
    let current = 'var(--positive)';
    let pendingTarget = current;
    let countToward = 0;
    return {
        update(target) {
            if (target === current) {
                pendingTarget = target;
                countToward = 0;
                return current;
            }
            if (target !== pendingTarget) {
                pendingTarget = target;
                countToward = 1;
                return current;
            }
            countToward += 1;
            // Severity ordering for "escalate vs relax" decision
            const order = {
                'var(--positive)': 0,
                'var(--warning)':  1,
                'var(--caution)':  2,
                'var(--negative)': 3,
            };
            const escalating = (order[target] ?? 0) > (order[current] ?? 0);
            const need = escalating ? holdEscalate : holdRelax;
            if (countToward >= need) {
                current = target;
                pendingTarget = current;
                countToward = 0;
            }
            return current;
        },
    };
}

// ── Tag badges ──────────────────────────────────────────────────────

/**
 * Render a 2–3 character monospace tag prefix per Auditor #4's
 * epistemic-discipline recommendation.
 *
 *   tagBadge('M')   → measured
 *   tagBadge('T')   → theory
 *   tagBadge('D')   → derived
 *   tagBadge('E')   → emergent
 *   tagBadge('~M')  → measured but pre-equilibrium
 */
export function tagBadge(kind, tooltip = '') {
    const labels = {
        M:  { txt: '[M]',  full: 'measured (lattice-derived)' },
        T:  { txt: '[T]',  full: 'theory / analytic reference' },
        D:  { txt: '[D]',  full: 'derived from FTD constants' },
        E:  { txt: '[E]',  full: 'emergent (lattice dynamics)' },
        '~M': { txt: '[~M]', full: 'measured but pre-equilibrium' },
    };
    const entry = labels[kind] || { txt: '[' + kind + ']', full: tooltip || kind };
    const title = tooltip || entry.full;
    return `<span title="${title}" style="font-family:var(--font-mono);font-size:10px;color:var(--text-muted);opacity:0.75;margin-right:4px;">${entry.txt}</span>`;
}
