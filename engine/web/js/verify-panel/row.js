/**
 * Render a single evidence row from a manifest entry.
 *
 * Three tier-specific layouts, all framed as question + evidence — never
 * as a claim with a verdict. Reader draws their own conclusion from the
 * numbers. See docs/superpowers/specs/2026-04-18-verify-panel-redesign-design.md §4.
 */
import { renderPullStrip } from './pull-strip.js';

function escapeHtml(s) {
    return String(s ?? '').replace(/[&<>"']/g, (c) => ({
        '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;',
    }[c]));
}

function fmtNumber(v) {
    if (v == null || !Number.isFinite(v)) return '—';
    const a = Math.abs(v);
    if (a !== 0 && (a < 1e-3 || a >= 1e6)) return v.toExponential(4);
    return v.toPrecision(10).replace(/\.?0+$/, '');
}

function fmtRel(rel) {
    if (rel == null || !Number.isFinite(rel)) return '—';
    const ppb = rel * 1e9;
    const a = Math.abs(ppb);
    if (a < 1) return `${ppb.toFixed(2)} ppb`;
    if (a < 1e3) return `${ppb.toFixed(1)} ppb`;
    const ppm = rel * 1e6;
    if (Math.abs(ppm) < 1e3) return `${ppm.toFixed(1)} ppm`;
    return `${(rel * 100).toFixed(3)}%`;
}

function fmtPull(pull) {
    if (pull == null || !Number.isFinite(pull)) return '—';
    const sign = pull >= 0 ? '+' : '';
    return `${sign}${pull.toFixed(2)}σ`;
}

function fmtInputs(inputs) {
    if (!inputs || !inputs.length) return '';
    const items = inputs.map(escapeHtml).join(', ');
    return `<div class="verify-inputs">Inputs used: {${items}}</div>`;
}

function renderHardRow(row) {
    const m = row.measurement || {};
    const units = m.units ? ` ${escapeHtml(m.units)}` : '';
    return `
        <article class="verify-row verify-row--hard" data-row-id="${escapeHtml(row.id)}">
            <h3 class="verify-question">${escapeHtml(row.question)}</h3>
            <div class="verify-lines">
                <div class="verify-line verify-line--ftd">
                    <span class="verify-tag verify-tag--${escapeHtml(row.epistemic.toLowerCase())}">${escapeHtml(row.epistemic)}</span>
                    <span class="verify-formula">${escapeHtml(row.formula)}</span>
                    <span class="verify-value">${fmtNumber(row.ftd_value)}${units}</span>
                </div>
                <div class="verify-line verify-line--meas">
                    <span class="verify-source">${escapeHtml(m.source || '—')}</span>
                    <span class="verify-value">${fmtNumber(m.value)} ± ${fmtNumber(m.sigma)}${units}</span>
                </div>
                <div class="verify-line verify-line--delta">
                    <span class="verify-delta-label">Δ</span>
                    <span class="verify-delta-rel">${fmtRel(row.rel_error)}</span>
                    <span class="verify-delta-pull">${fmtPull(row.pull)}</span>
                </div>
            </div>
            <div class="verify-strip-wrap">${renderPullStrip(row.pull)}</div>
            ${fmtInputs(row.inputs_used)}
            <a class="verify-theory-ref" href="../${escapeHtml(row.theory_ref)}" target="_blank" rel="noopener">Theory: ${escapeHtml(row.theory_ref)}</a>
        </article>
    `;
}

function renderParametricRow(row) {
    const m = row.measurement || {};
    const units = m.units ? ` ${escapeHtml(m.units)}` : '';
    return `
        <article class="verify-row verify-row--parametric" data-row-id="${escapeHtml(row.id)}">
            <h3 class="verify-question">${escapeHtml(row.question)}</h3>
            <div class="verify-lines">
                <div class="verify-line verify-line--ftd">
                    <span class="verify-tag verify-tag--parametric">PARAMETRIC · SM formula with FTD inputs</span>
                    <span class="verify-formula">${escapeHtml(row.formula)}</span>
                    <span class="verify-value">${fmtNumber(row.ftd_value)}${units}</span>
                </div>
                <div class="verify-line verify-line--meas">
                    <span class="verify-source">${escapeHtml(m.source || '—')}</span>
                    <span class="verify-value">${fmtNumber(m.value)} ± ${fmtNumber(m.sigma)}${units}</span>
                </div>
                <div class="verify-line verify-line--delta">
                    <span class="verify-delta-label">Δ</span>
                    <span class="verify-delta-rel">${fmtRel(row.rel_error)}</span>
                </div>
            </div>
            ${fmtInputs(row.ftd_inputs)}
            <p class="verify-caveat">SM formula supplies the functional form; FTD supplies the numerical inputs.</p>
            <a class="verify-theory-ref" href="../${escapeHtml(row.theory_ref)}" target="_blank" rel="noopener">Theory: ${escapeHtml(row.theory_ref)}</a>
        </article>
    `;
}

function renderUnpredictedRow(row) {
    const m = row.measurement || {};
    const units = m.units ? ` ${escapeHtml(m.units)}` : '';
    return `
        <article class="verify-row verify-row--unpredicted" data-row-id="${escapeHtml(row.id)}">
            <h3 class="verify-question">${escapeHtml(row.question)}</h3>
            <div class="verify-lines">
                <div class="verify-line verify-line--ftd">
                    <span class="verify-tag verify-tag--open">OPEN</span>
                    <span class="verify-value">FTD: no prediction</span>
                </div>
                <div class="verify-line verify-line--meas">
                    <span class="verify-source">${escapeHtml(m.source || '—')}</span>
                    <span class="verify-value">${fmtNumber(m.value)} ± ${fmtNumber(m.sigma)}${units}</span>
                </div>
            </div>
        </article>
    `;
}

export function renderRow(row) {
    if (row.tier === 'hard') return renderHardRow(row);
    if (row.tier === 'parametric') return renderParametricRow(row);
    if (row.tier === 'unpredicted') return renderUnpredictedRow(row);
    return `<article class="verify-row verify-row--unknown">Unknown tier: ${escapeHtml(row.tier)}</article>`;
}
