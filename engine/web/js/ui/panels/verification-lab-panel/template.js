import { CATEGORIES } from '../../../verification/registry.js';

export function getVerificationLabPanelTemplate() {
    const pills = CATEGORIES.map((c, i) => `
        <button class="verif-cat-pill ${i === 0 ? 'active' : ''}" data-verif-cat="${c.id}">
            ${c.label}
        </button>
    `).join('');

    return `
        <div class="panel" id="panel-verification-lab">
            <div class="verif-shell">
                <div class="verif-header">
                    <div>
                        <div class="verif-kicker">Verification Lab</div>
                        <div class="verif-title">Runs FTD scenarios, measures, compares to theory.</div>
                    </div>
                    <div class="verif-actions-global">
                        <button class="ctrl-btn-secondary" id="verif-export-all-json" title="Export all results as JSON">Export All</button>
                    </div>
                </div>

                <div class="verif-cat-bar" role="tablist">
                    ${pills}
                </div>

                <div class="verif-body">
                    <div class="verif-list" id="verif-experiment-list" role="list">
                        <!-- Experiment rows injected here -->
                    </div>

                    <div class="verif-detail" id="verif-experiment-detail">
                        <div class="verif-detail-empty">Select an experiment to view details.</div>
                    </div>
                </div>
            </div>
        </div>
    `;
}

export function getExperimentRowTemplate(exp, status, badge) {
    return `
        <button class="verif-row" data-verif-id="${exp.id}" data-status="${status}">
            <span class="verif-row-dot"></span>
            <span class="verif-row-name">${exp.name}</span>
            <span class="verif-row-badge ${badgeClassName(badge)}">${badgeSymbol(badge)}</span>
        </button>
    `;
}

export function getDetailTemplate(exp, state) {
    const theory = exp.theoryFn?.() || { value: '?', units: '' };
    const measurement = state.aggregate?.mean;
    const stddev = state.aggregate?.stddev;
    const badge = state.badge || 'NOT_RUN';
    const formatter = exp.formatter || ((v) => (v ?? '—').toString());
    const trials = state.results?.length || 0;
    const progressPct = state.progress != null
        ? Math.round(state.progress * 100)
        : 0;

    return `
        <div class="verif-detail-inner">
            <div class="verif-detail-head">
                <div>
                    <div class="verif-detail-name">${exp.name}</div>
                    <div class="verif-detail-meta">
                        <span class="verif-tag verif-tag-${exp.epistemicTag.toLowerCase()}">${exp.epistemicTag}</span>
                        <span class="verif-scenario">scenario: <code>${exp.scenarioId}</code></span>
                    </div>
                </div>
                <div class="verif-badge-big ${badgeClassName(badge)}">${badgeLabelFull(badge)}</div>
            </div>

            <p class="verif-description">${exp.description}</p>

            <div class="verif-controls">
                <label class="verif-ctrl-field">
                    <span>Trials</span>
                    <input type="number" id="verif-trials-input" min="1" max="500" step="1" value="${exp.defaultTrials}">
                </label>
                <label class="verif-ctrl-field">
                    <span>Ticks per trial</span>
                    <input type="number" id="verif-ticks-input" min="1" max="2000" step="1" value="${exp.defaultTicksPerTrial}">
                </label>
                <div class="verif-btn-row">
                    <button class="ctrl-btn" id="verif-run-btn" ${state.running ? 'disabled' : ''}>${state.running ? 'Running…' : 'Run ▶'}</button>
                    <button class="ctrl-btn-secondary" id="verif-abort-btn" ${state.running ? '' : 'disabled'}>Abort</button>
                </div>
            </div>

            ${state.running || state.progress ? `
                <div class="verif-progress">
                    <div class="verif-progress-track">
                        <div class="verif-progress-fill" style="width: ${progressPct}%"></div>
                    </div>
                    <div class="verif-progress-label">Trial ${state.trialIndex || 0} / ${state.totalTrials || exp.defaultTrials}</div>
                </div>
            ` : ''}

            <div class="verif-stats-grid">
                <div class="verif-stat">
                    <div class="verif-stat-label">Measured</div>
                    <div class="verif-stat-value">
                        ${measurement != null ? formatter(measurement) : '—'}
                        ${stddev != null ? `<span class="verif-stat-stddev">± ${formatter(stddev)}</span>` : ''}
                    </div>
                </div>
                <div class="verif-stat">
                    <div class="verif-stat-label">Theory</div>
                    <div class="verif-stat-value">${formatter(theory.value)} <span class="verif-stat-units">${theory.units}</span></div>
                </div>
                <div class="verif-stat">
                    <div class="verif-stat-label">Trials</div>
                    <div class="verif-stat-value">${trials}</div>
                </div>
                <div class="verif-stat">
                    <div class="verif-stat-label">Tolerance</div>
                    <div class="verif-stat-value">${formatTolerance(exp.tolerance)}</div>
                </div>
            </div>

            ${state.results?.length ? `
                <div class="verif-sparkline-wrap">
                    <canvas id="verif-sparkline" class="verif-sparkline"></canvas>
                </div>
            ` : ''}

            <div class="verif-export-row">
                <button class="ctrl-btn-secondary" id="verif-export-csv" ${state.results?.length ? '' : 'disabled'}>CSV</button>
                <button class="ctrl-btn-secondary" id="verif-export-json" ${state.results?.length ? '' : 'disabled'}>JSON</button>
                <button class="ctrl-btn-secondary" id="verif-copy-json" ${state.results?.length ? '' : 'disabled'}>Copy</button>
            </div>
        </div>
    `;
}

function badgeClassName(badge) {
    return 'verif-badge-' + (badge || 'not-run').toLowerCase().replace('_', '-');
}

function badgeSymbol(badge) {
    switch (badge) {
        case 'PASS':     return '✓';
        case 'CLOSE':    return '~';
        case 'FAIL':     return '✗';
        case 'EMERGENT': return '●';
        default:         return '—';
    }
}

function badgeLabelFull(badge) {
    switch (badge) {
        case 'PASS':     return '✓ PASS';
        case 'CLOSE':    return '~ CLOSE';
        case 'FAIL':     return '✗ FAIL';
        case 'EMERGENT': return '● MEASURED';
        default:         return '— NOT RUN';
    }
}

function formatTolerance(tol) {
    if (!tol) return 'n/a';
    if (tol.absolute != null) return `± ${tol.absolute}`;
    if (tol.relative != null) return `± ${(tol.relative * 100).toFixed(0)}%`;
    return 'n/a';
}
