/**
 * Lattice Spectroscopy — Scale-0 field-structure instrument.
 *
 * Characterizes the lattice field itself (NOT emergent particle masses — those
 * live in the Zoo). Four sections:
 *   ① E(k) energy spectrum (hero) — FFT-derived spatial power spectrum of the
 *      flux field J, Parseval-checked within the resampled grid; live (undersampled,
 *      downsampled) + a Deep Measure higher-grid snapshot.
 *   ② Topology — Gauss violation, defect/monopole proxy, flux-tube count, chirality.
 *   ③ Field metrics + distributions — vorticity/helicity/coherence/Fisher/
 *      Kretschmann/entropy: value + spatial histogram.
 *   ④ Energy partition — E/B/wave/field split, Poynting, drift.
 *
 * Honesty (CLAUDE.md): [M] measured, [D] derived/computed, [≈] approximate
 * (downsampled / undersampled). See SPEC_SCALE0_LATTICE_SPECTROSCOPY.md.
 */

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { cardStyle, titleStyle, tagBadge, formatExp as finiteExp, formatFixed as finiteFixed } from './_card-helpers.js';
import { SpectrumAnalysisClient, captureSpectrumObservation } from '../../analysis/spectrum-analysis-client.js';
import { isCurrentScale0TelemetryMeta } from '../../../../telemetry/scale0-read.js';
import { telemetryHub } from '../../../../telemetry-hub.js';
import { getScale0State, isScale0AuthoritativeGenerationReady, subscribeScale0Qualification, resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { isPanelLive, PANEL_VISIBILITY_CHANGE_EVENT } from '../../../../ui/panels/panel-visibility.js';

const PANEL_ID = 'spectrum-panel';
const HZ = 2;                 // exploratory data — slower cadence
const M_LIVE = 32;            // default live FFT grid (undersampled)
const M_LIVE_LARGE = 8;       // large-lattice live grid; Deep Measure remains 64³
const M_DEEP = 64;            // Deep Measure FFT grid (higher grid)
const EMPTY_SCENARIO_ID = 'empty';
const SCENARIO_SYNC_MAX_FRAMES = 120;

const METRIC_KINDS = [
    { kind: 'vorticity',   name: 'Vorticity',   sym: 'ω',  desc: 'ω = |∇×J| — local rotation / swirl of the flux field; high where the field circulates.' },
    { kind: 'helicity',    name: 'Helicity',    sym: 'H',  desc: 'H = J·(∇×J) — linking / handedness of field lines; nonzero for helical (Beltrami) flows.' },
    { kind: 'coherence',   name: 'Coherence',   sym: 'C',  desc: 'Phase coherence — how ordered (vs random / turbulent) the field is locally.' },
    { kind: 'fisher',      name: 'Fisher info', sym: 'I',  desc: 'Fisher information — local distinguishability / how sharply the field varies.' },
    { kind: 'kretschmann', name: 'Curvature proxy', sym: 'K',  desc: '(18-point Laplacian of normalized latency proxy)²; not the Riemann curvature invariant.' },
];

const SECTION_HELP = {
    spectrum: 'Spatial energy spectrum E(k): the FFT power spectrum of the flux field J — which spatial scales hold the field energy (a turbulence-style spectrum). Peak λ* is the dominant wavelength; slope p<0 = energy at large scales, p>0 = small-scale (UV) buildup; Parseval ≈ 1 confirms the transform is correct. Both modes resample without anti-alias filtering; Parseval checks only the resampled grid. Deep Measure uses a fixed 64³ FFT, not a full-lattice certification. Effective sampler stride and center origin are retained; strongest-bin representatives and nonuniform boundary gaps make peak wavelengths approximate.',
    topology: 'Field-structure diagnostics. Gauss audit: the selected div J constraint, evaluated only in vacuum cells. Defects: divergence-threshold proxies. Flux components: threshold-connected |J| regions; no confinement identification. Dual-channel imbalance compares squared flux or wave amplitudes; it does not establish physical handedness.',
    metrics: 'Field metrics with their spatial distributions. Each row shows the metric RMS plus a histogram of its per-voxel values — the histogram shape reveals structure (uniform / peaked / bimodal) a single mean would hide.',
    energy: 'Separate reference diagnostics: wave/flux terms, electric/magnetic reductions, the magnitude of a volume sum of C_SPEED²(E×B), and the hub energy change from its first nonzero audit baseline (|H| > 1e-12). These are not a complete conserved Hamiltonian or a boundary flux.',
};

function liveStride(L) {
    if (L <= 33) return 1;
    if (L <= 49) return 2;
    return Math.max(3, Math.min(8, Math.ceil(L / 16)));
}

function liveGridSize(L) { return L >= 65 ? M_LIVE_LARGE : M_LIVE; }

// ── Compute ──────────────────────────────────────────────────────────────────

function finiteValue(...values) { return values.find(Number.isFinite) ?? null; }
function formatExp(value) { return Number.isFinite(value) ? finiteExp(value) : '—'; }
function formatFixed(value, digits = 3) { return Number.isFinite(value) ? finiteFixed(value, digits) : '—'; }

function readQualifiedTelemetry(hub = telemetryHub) {
    const diagMeta = hub.getScale0TelemetryMeta?.('diagnostics') ?? null;
    const auditMeta = hub.getScale0TelemetryMeta?.('audit') ?? null;
    const diagCurrent = isCurrentScale0TelemetryMeta(diagMeta)
        && Number.isSafeInteger(diagMeta.tick) && diagMeta.tick >= 0;
    const auditCurrent = isCurrentScale0TelemetryMeta(auditMeta)
        && Number.isSafeInteger(auditMeta.tick) && auditMeta.tick >= 0;
    return {
        diag: diagCurrent ? hub.s0?.diag ?? null : null,
        audit: auditCurrent ? hub.s0?.audit ?? null : null,
        diagTick: diagCurrent ? diagMeta.tick : null,
        auditTick: auditCurrent ? auditMeta.tick : null,
    };
}

function renderSpectrum(container, r, isDeep) {
    if (!r) {
        container.innerHTML = '<div class="spec-hist-empty">Flux sample unavailable or pending.</div>';
        return;
    }
    const { spec, peak, slope, parseval } = r;
    const ks = [], es = [];
    for (let i = 0; i < spec.E.length; i++) { if (spec.E[i] > 0 && spec.k[i] > 0) { ks.push(spec.k[i]); es.push(spec.E[i]); } }
    if (ks.length < 2) {
        container.innerHTML = `<div class="spec-hist-empty">No resolved nonzero spectral range in this sampled grid.</div>`;
        return;
    }
    const W = 360, H = 188, m = { top: 16, right: 14, bottom: 34, left: 44 };
    const iW = W - m.left - m.right, iH = H - m.top - m.bottom;
    const lkMin = Math.log10(ks[0]), lkMax = Math.log10(ks[ks.length - 1]);
    const leMin = Math.log10(Math.min(...es)), leMax = Math.log10(Math.max(...es));
    const kSpan = (lkMax - lkMin) || 1, eSpan = (leMax - leMin) || 1;
    const X = (k) => m.left + ((Math.log10(k) - lkMin) / kSpan) * iW;
    const Y = (e) => m.top + (1 - (Math.log10(e) - leMin) / eSpan) * iH;

    let svg = `<svg viewBox="0 0 ${W} ${H}" class="spec-svg-plot">`;
    svg += `<rect x="${m.left}" y="${m.top}" width="${iW}" height="${iH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light)" stroke-width="1"/>`;
    // grid decade lines (y)
    for (let e = Math.ceil(leMin); e <= Math.floor(leMax); e++) {
        const yy = Y(Math.pow(10, e));
        svg += `<line x1="${m.left}" y1="${yy.toFixed(1)}" x2="${m.left + iW}" y2="${yy.toFixed(1)}" stroke="var(--border-light)" stroke-width="0.4" opacity="0.4"/>`;
        svg += `<text x="${m.left - 4}" y="${(yy + 3).toFixed(1)}" text-anchor="end" font-size="16" font-family="var(--font-mono)" fill="var(--text-muted)">1e${e}</text>`;
    }
    // peak marker
    if (peak.kPeak > 0) {
        const xp = X(peak.kPeak);
        svg += `<line x1="${xp.toFixed(1)}" y1="${m.top}" x2="${xp.toFixed(1)}" y2="${m.top + iH}" stroke="var(--warning-text)" stroke-width="0.8" stroke-dasharray="2,3" opacity="0.7"/>`;
        svg += `<text x="${xp.toFixed(1)}" y="${m.top - 3}" text-anchor="middle" font-size="16" fill="var(--warning-text)">k*</text>`;
    }
    // slope reference line over the inertial range
    if (Number.isFinite(slope.slope)) {
        const k1 = ks[2] || ks[0], k2 = ks[ks.length - 3] || ks[ks.length - 1];
        const eAtK1 = es[ks.indexOf(k1)] ?? es[0];
        const yLine = (k) => Y(eAtK1 * Math.pow(k / k1, slope.slope));
        svg += `<line x1="${X(k1).toFixed(1)}" y1="${yLine(k1).toFixed(1)}" x2="${X(k2).toFixed(1)}" y2="${yLine(k2).toFixed(1)}" stroke="var(--positive-text)" stroke-width="1" stroke-dasharray="4,2" opacity="0.65"/>`;
    }
    // E(k) polyline + points
    let path = '';
    for (let i = 0; i < ks.length; i++) path += `${i ? 'L' : 'M'}${X(ks[i]).toFixed(1)},${Y(es[i]).toFixed(1)} `;
    svg += `<path d="${path}" fill="none" stroke="var(--accent)" stroke-width="1.4"/>`;
    for (let i = 0; i < ks.length; i++) svg += `<circle cx="${X(ks[i]).toFixed(1)}" cy="${Y(es[i]).toFixed(1)}" r="1.6" fill="var(--accent)"/>`;
    // axis labels
    svg += `<text x="${m.left + iW / 2}" y="${H - 3}" text-anchor="middle" font-size="16" fill="var(--text-muted)">k (rad/voxel) — log</text>`;
    svg += `<text x="11" y="${m.top + iH / 2}" transform="rotate(-90 11 ${m.top + iH / 2})" text-anchor="middle" font-size="16" fill="var(--text-muted)">E(k) — log</text>`;
    svg += `</svg>`;

    const pOk = Math.abs(parseval - 1) < 0.05;
    const lam = Number.isFinite(peak.lambdaPeak) ? peak.lambdaPeak.toFixed(1) : '∞';
    container.innerHTML = `
        ${svg}
        <div class="spec-readouts">
            <span>${tagBadge('D')}peak λ* <b>${lam}</b> vox (k*=${peak.kPeak.toFixed(3)})</span>
            <span>${tagBadge('D')}slope p <b>${Number.isFinite(slope.slope) ? slope.slope.toFixed(2) : '—'}</b></span>
            <span title="Total Fourier power including DC / resampled-grid |J|²; this checks the transform, not sampling accuracy">${tagBadge('M')}Parseval <b style="color:${pOk ? 'var(--positive-text)' : 'var(--warning-text)'}">${parseval.toFixed(3)}</b></span>
            <span>${isDeep ? `${tagBadge('D')}DEEP M=${r.M}³ · axial Nyquist ${spec.kNyq.toFixed(2)}` : `${tagBadge('≈')}live M=${r.M}³ · undersampled axial Nyquist ${spec.kNyq.toFixed(2)}`}</span>
        </div>`;
}

// ── Render: ② topology ───────────────────────────────────────────────────────

function row(label, value, tag = 'D', color = 'var(--text-primary)', tip = '') {
    const t = tip ? ` title="${tip}"` : '';
    return `<div class="spec-row"><span class="spec-row-l"${t}>${tagBadge(tag)}${label}</span><span class="spec-row-v" style="color:${color}">${value}</span></div>`;
}

function renderTopology(container, t) {
    const gaugePresent = Number.isFinite(t.gauss);
    const gaugeOk = gaugePresent && t.gauss >= 0 && t.gauss < 1e-4;
    container.innerHTML =
        row('Vacuum constraint residual Σr²', formatExp(t.gauss), 'M', !gaugePresent ? 'var(--text-muted)' : gaugeOk ? 'var(--positive-text)' : 'var(--warning-text)',
            'Native audit r=div J−g(s−mean charge), squared and summed only at unmanifested sites; g is the selected Coulomb charge coupling. Distinct from the raw div J−s overlay.') +
        row('  max |Gauss error|', formatExp(t.gaussMax), 'M', 'var(--text-muted)', 'The worst single-voxel Gauss-law residual.') +
        row('Defects (src / sink / net)', t.defects ? `${t.defects.sources} / ${t.defects.sinks} / ${t.defects.net >= 0 ? '+' : ''}${t.defects.net}` : '—', 'D', undefined,
            'Counts above half the peak |div J| among retained sampled representatives; net is their signed imbalance. Not a quantized charge or a count of all microscopic defects.') +
        row('Flux tubes (count / largest)', t.tubes ? `${t.tubes.count} / ${t.tubes.largest}` : '—', 'D', undefined,
            'Connected components of |J| above a threshold (6-neighbour, periodic) — threshold-connected regions: count and largest size; neither coherence nor confinement follows from connectivity.') +
        row('Channel asymmetry (flux / wave)', `${formatFixed(t.chir.eAsym, 3)} / ${formatFixed(t.chir.wvAsym, 3)}`, 'D', undefined,
            '(L−R)/(L+R) of squared reference flux and wave-velocity records. Unavailable for missing channels or zero denominators; no fermion-handedness identification.');
}

// ── Render: ③ metrics + distributions ────────────────────────────────────────

function miniHist(hist, w = 90, h = 22) {
    const c = hist.counts, n = c.length, mx = Math.max(1, ...c);
    let s = `<svg viewBox="0 0 ${w} ${h}" class="spec-mini-hist">`;
    const bw = w / n;
    for (let i = 0; i < n; i++) {
        const bh = (c[i] / mx) * h;
        s += `<rect x="${(i * bw).toFixed(1)}" y="${(h - bh).toFixed(1)}" width="${(bw - 0.4).toFixed(1)}" height="${bh.toFixed(1)}" fill="var(--accent)" opacity="0.7"/>`;
    }
    return s + `</svg>`;
}

function renderMetrics(container, metrics) {
    let html = '';
    for (const m of metrics) {
        html += `<div class="spec-metric-row" title="${m.desc}">
            <span class="spec-metric-name">${m.sym} <span class="spec-metric-sub">${m.name}</span></span>
            <span class="spec-metric-val">${formatExp(m.stats?.rms)}<span class="spec-metric-unit">rms</span></span>
            <span class="spec-metric-hist">${m.hist ? miniHist(m.hist) : 'No retained sample statistics'}</span>
        </div>`;
    }
    container.innerHTML = html || `<div class="spec-hist-empty">No metric data.</div>`;
}

// ── Render: ④ energy partition ───────────────────────────────────────────────

function renderEnergy(container, audit, entropy, { auditTick = null, diagTick = null } = {}) {
    const parts = [
        { k: 'Wave',    v: finiteValue(audit?.waveEnergy), c: 'var(--chart-flux, #fb8c00)' },
        { k: 'Flux',    v: finiteValue(audit?.fieldEnergy), c: 'var(--accent)' },
    ];
    const total = parts.reduce((sum, p) => sum + p.v, 0);
    const partition = parts.every(p => Number.isFinite(p.v) && p.v >= 0)
        && Number.isFinite(total) && total > 0;
    let bar = partition
        ? '<div class="spec-energy-bar" title="Disjoint reference wave/flux terms; separate E/B diagnostics below. Not a certified conserved Hamiltonian.">'
        : '<div class="spec-hist-empty">Partition unavailable or has no positive support.</div>';
    if (partition) {
        for (const p of parts) bar += `<span style="width:${(p.v / total * 100).toFixed(1)}%;background:${p.c}" title="${p.k}: ${formatExp(p.v)}"></span>`;
        bar += '</div>';
    }
    bar += '<div class="spec-energy-legend">';
    for (const p of parts) bar += `<span><i style="background:${p.c}"></i>${p.k} ${formatExp(p.v)}</span>`;
    bar += `</div>`;
    const px = finiteValue(audit?.totalPoynting?.x, audit?.poyntingX);
    const py = finiteValue(audit?.totalPoynting?.y, audit?.poyntingY);
    const pz = finiteValue(audit?.totalPoynting?.z, audit?.poyntingZ);
    const pMag = [px, py, pz].every(Number.isFinite) ? Math.hypot(px, py, pz) : null;
    const drift = finiteValue(audit?.energyDrift);
    const tickLabel = tick => Number.isSafeInteger(tick) && tick >= 0 ? String(tick) : 'unavailable';
    container.innerHTML = bar +
        row('E diagnostic (= wave)', formatExp(finiteValue(audit?.eFieldEnergy, audit?.EFieldEnergy)), 'M', undefined, 'Same ½Σ|wave velocity|² term; do not add it a second time.') +
        row('B diagnostic', formatExp(finiteValue(audit?.bFieldEnergy, audit?.BFieldEnergy)), 'M', undefined, '(C_SPEED²/2)Σ|curl J|²; separate diagnostic, not an extra ledger partition.') +
        row('Poynting volume sum magnitude', formatExp(pMag), 'M', undefined, 'Magnitude of the volume sum of C_SPEED²(E×B). A reference diagnostic, not net boundary flux through a surface.') +
        row('Accounted energy change', Number.isFinite(drift) ? `${formatFixed(drift, 3)} %` : '—', 'D', undefined, 'Hub percentage change of dynamicEnergy (fallback totalEnergy), relative to its first nonzero audit with |H| > 1e-12 after a source/intervention baseline reset. Damping and pumping can change this value; it is not a complete conservation test.') +
        row('Flux-weight entropy', formatExp(finiteValue(entropy)), 'M', undefined, 'Native Shannon spread −Σp ln p of normalized squared flux weights, with native cutoffs. Not thermodynamic entropy.') +
        `<div class="spec-observation-scope">Audit tick ${tickLabel(auditTick)}; diagnostics tick ${tickLabel(diagTick)}. Independently sampled groups.</div>`;
}

// ── Panel shell ──────────────────────────────────────────────────────────────

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only spectrum-panel';
    root.dataset.applicability = 'applicable';
    root.innerHTML = `
        <div class="spectrum-applicable-content">
            <header class="spec-header">
                <span class="spec-title">Lattice Spectroscopy</span>
                <span class="spec-mode" id="${PANEL_ID}-mode">live</span>
            </header>
            <section style="${cardStyle(230)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.spectrum}">Energy spectrum E(k) ⓘ</div>
                <div id="${PANEL_ID}-spec" class="spec-hist-box"></div>
                <div class="spec-actions">
                    <button id="${PANEL_ID}-deep" type="button" class="spec-btn" title="Fixed 64³ FFT snapshot of sampled field data; no anti-alias or full-band guarantee">Deep Measure</button>
                    <button id="${PANEL_ID}-live" type="button" class="spec-btn spec-btn-ghost" hidden title="Resume the live undersampled view">↻ Live</button>
                </div>
            </section>
            <section style="${cardStyle(150)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.topology}">Topology ⓘ</div>
                <div id="${PANEL_ID}-topo"></div>
            </section>
            <section style="${cardStyle(170)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.metrics}">Field metrics &amp; distributions ⓘ</div>
                <div id="${PANEL_ID}-metrics"></div>
            </section>
            <section style="${cardStyle(130)}">
                <div style="${titleStyle()}" title="${SECTION_HELP.energy}">Energy diagnostics ⓘ</div>
                <div id="${PANEL_ID}-energy"></div>
            </section>
        </div>
        <section class="mode-unavailable spectrum-inapplicable"
                 data-applicability="inapplicable" role="status" hidden>
            <strong>Not applicable — imposed null control</strong>
            <p>Scenario 1 · Empty does not define a field or spatial-spectrum domain.
               No field sampling, FFT, topology, or spectral inference is performed.</p>
            <p>An empty spectrum would be misleading: this control is not a
               measurement of physical vacuum or zero-point fluctuations.</p>
        </section>
    `;
    return root;
}

export function mountSpectrumPanel(host, getBridge) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);

    const el = (id) => panel.querySelector(`#${PANEL_ID}-${id}`);
    const specBody = el('spec'), topoBody = el('topo'), metBody = el('metrics'), enBody = el('energy');
    const modeBadge = el('mode'), deepBtn = el('deep'), liveBtn = el('live');
    const applicableContent = panel.querySelector('.spectrum-applicable-content');
    const inapplicableMessage = panel.querySelector('.spectrum-inapplicable');

    let mode = 'live';   // 'live' | 'deep' (deep freezes the hero on a higher-grid snapshot)
    let lastSpec = null; // last computed spectrum (exposed for tests/diagnostics)
    let inapplicable = false;
    let disposed = false;
    let sub = null;
    let deepTimer = 0;
    let deepPending = false;
    let deepRequestToken = 0;
    let samplerWantSignature = '';
    let samplerBridge = null;
    let scenarioSelect = null;
    let scenarioSyncRaf = 0;
    let scenarioSyncToken = 0;
    let analysisOwner = null;
    let analysisGeneration = null;
    let latestAnalysisResult = null;
    const ownerIds = new WeakMap();
    let nextOwnerId = 0;
    let panelWasLive = isPanelLive(host);
    const analysis = new SpectrumAnalysisClient();

    function releaseSamplerWants(force = false) {
        if (!force && !samplerWantSignature) return;
        samplerBridge?.replaceSamplerWants?.('spectrum-panel', []);
        samplerWantSignature = '';
        samplerBridge = null;
    }

    function setSamplerWants(bridge, keys) {
        if (disposed) return;
        const signature = keys.join('|');
        if (signature === samplerWantSignature && bridge === samplerBridge) return;
        if (samplerBridge && bridge !== samplerBridge) releaseSamplerWants(true);
        bridge?.replaceSamplerWants?.('spectrum-panel', keys);
        samplerBridge = bridge;
        samplerWantSignature = signature;
    }

    function cancelDeepMeasurement(label = 'live') {
        analysis.cancel();
        latestAnalysisResult = null;
        ++deepRequestToken;
        if (deepTimer) clearTimeout(deepTimer);
        deepTimer = 0;
        deepPending = false;
        // Release the owner that received this demand, even after a bridge swap.
        releaseSamplerWants(true);
        mode = 'live';
        modeBadge.textContent = label;
        liveBtn.hidden = true;
        deepBtn.disabled = disposed || inapplicable
            || !isScale0AuthoritativeGenerationReady(getScale0State());
    }

    function stopCoordinator() {
        sub?.unsubscribe();
        sub = null;
    }

    function startCoordinator() {
        if (sub || inapplicable || disposed) return;
        sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });
    }

    function getCaps() {
        const b = getBridge?.();
        return b?.capabilities?.scale0 || null;
    }

    function currentAnalysisContext(owner) {
        if (!ownerIds.has(owner)) {
            if (nextOwnerId === Number.MAX_SAFE_INTEGER) throw new RangeError('Spectrum owner identity exhausted');
            ownerIds.set(owner, ++nextOwnerId);
        }
        return { owner, generation: getScale0State().qualificationAnchor?.loadGeneration,
            ownerId: ownerIds.get(owner), scenarioId: getScale0State().currentScenarioId, scenarioToken: scenarioSyncToken };
    }

    function canPublish(context) {
        const state = getScale0State();
        return !disposed && !inapplicable && isPanelLive(host)
            && isScale0AuthoritativeGenerationReady(state)
            && state.currentScenarioId !== EMPTY_SCENARIO_ID
            && getBridge?.() === context.owner
            && state.qualificationAnchor?.loadGeneration === context.generation
            && state.currentScenarioId === context.scenarioId
            && scenarioSyncToken === context.scenarioToken;
    }

    function wireContext(context) {
        // The owner object stays on the UI thread; this transaction-local token
        // and load/scenario identity are echoed by the observation-only worker.
        return { ownerId: context.ownerId, generation: context.generation, scenarioId: context.scenarioId,
            scenarioToken: context.scenarioToken };
    }

    function unavailableAnalysis() {
        if (mode === 'live') { lastSpec = null; renderSpectrum(specBody, null, false); }
        renderTopology(topoBody, { defects: null, tubes: null,
            chir: { eAsym: null, wvAsym: null }, gauss: null, gaussMax: null });
        renderMetrics(metBody, METRIC_KINDS.map(m => ({ ...m, stats: null, hist: null })));
    }

    function suspendAnalysis() {
        if (deepPending) cancelDeepMeasurement();
        else { analysis.cancel(); latestAnalysisResult = null; releaseSamplerWants(); }
    }

    function publishReadyAnalysis() {
        const ready = latestAnalysisResult;
        latestAnalysisResult = null;
        if (!ready || !canPublish(ready.context) || deepPending) return;
        if (ready.error) { unavailableAnalysis(); return; }
        const { result, requestedMode } = ready;
        if (requestedMode === 'live' && mode === 'live') {
            lastSpec = result.spectrum;
            renderSpectrum(specBody, lastSpec, false);
        }
        renderTopology(topoBody, result.topology);
        renderMetrics(metBody, result.metrics.map(m => ({
            ...METRIC_KINDS.find(description => description.kind === m.kind), ...m,
        })));
    }

    function update() {
        if (disposed) return;
        if (inapplicable || getScale0State().currentScenarioId === EMPTY_SCENARIO_ID) {
            if (!inapplicable) setEmptyApplicability(true);
            return;
        }
        panelWasLive = isPanelLive(host);
        if (!isScale0AuthoritativeGenerationReady(getScale0State()) || !panelWasLive) {
            suspendAnalysis();
            return;
        }
        const b = getBridge?.();
        const generation = getScale0State().qualificationAnchor?.loadGeneration;
        if ((analysisOwner && analysisOwner !== b)
            || (analysisGeneration !== null && analysisGeneration !== generation)) {
            cancelDeepMeasurement();
            lastSpec = null;
            renderSpectrum(specBody, null, false);
        }
        analysisOwner = b;
        analysisGeneration = generation;
        const caps = getCaps();
        if (!caps) { cancelDeepMeasurement(); unavailableAnalysis(); return; }
        const L = caps.latticeSize || 33;
        const stride = liveStride(L);
        setSamplerWants(b, [...new Set([
            `fluxVector@${stride}`, `divJ@${stride}`,
            ...METRIC_KINDS.map((m) => `${m.kind}@${stride}`),
            ...(deepPending ? ['fluxVector@1'] : []),
        ])]);
        const telemetry = readQualifiedTelemetry();
        // Live DOM publication remains inside this registered 2 Hz callback;
        // worker messages only replace the single bounded result mailbox.
        publishReadyAnalysis();
        renderEnergy(enBody, telemetry.audit, telemetry.diag?.entropy, telemetry);
        // Deep owns the sole active worker until it completes or its original
        // deadline expires. Its completion immediately resumes live topology.
        if (deepPending) return;
        const context = currentAnalysisContext(b);
        const requestedMode = mode === 'live' ? 'live' : 'topology';
        try {
            const observation = captureSpectrumObservation(caps, { L, stride,
                M: liveGridSize(L), mode: requestedMode, metricKinds: METRIC_KINDS, ...telemetry });
            analysis.submit(observation, { context: wireContext(context),
                onResult(result) {
                    if (!canPublish(context) || deepPending) return;
                    latestAnalysisResult = { result, requestedMode, context };
                },
                onError() { if (canPublish(context) && !deepPending) latestAnalysisResult = { context, error: true }; },
            });
        } catch { unavailableAnalysis(); }
    }

    function onVisibilityChange() {
        if (disposed) return;
        const live = isPanelLive(host), becameLive = live && !panelWasLive;
        panelWasLive = live;
        if (!live) suspendAnalysis();
        else if (becameLive) update();
    }
    window.addEventListener?.(PANEL_VISIBILITY_CHANGE_EVENT, onVisibilityChange);

    function setEmptyApplicability(nextValue) {
        if (disposed) return;
        const next = !!nextValue;
        if (next) cancelDeepMeasurement();
        if (next === inapplicable) {
            if (next) {
                // A rapid empty → nonempty → empty sequence can leave the
                // intermediate generation marked pending while the panel is
                // already logically inapplicable. Reassert the complete null-
                // control presentation as well as keeping all work stopped.
                panel.dataset.applicability = 'inapplicable-empty';
                panel.classList.add('is-inapplicable');
                applicableContent.hidden = true;
                applicableContent.setAttribute('aria-hidden', 'true');
                inapplicableMessage.hidden = false;
                deepBtn.disabled = true;
                liveBtn.disabled = true;
                stopCoordinator();
                releaseSamplerWants(true);
            } else {
                panel.dataset.applicability = 'applicable';
                deepBtn.disabled = false;
                liveBtn.disabled = false;
                startCoordinator();
                update();
            }
            return;
        }
        inapplicable = next;
        panel.dataset.applicability = next ? 'inapplicable-empty' : 'applicable';
        panel.classList.toggle('is-inapplicable', next);
        applicableContent.hidden = next;
        applicableContent.setAttribute('aria-hidden', next ? 'true' : 'false');
        inapplicableMessage.hidden = !next;
        deepBtn.disabled = next;
        liveBtn.disabled = next;

        if (next) {
            stopCoordinator();
            releaseSamplerWants(true);
            mode = 'live';
            lastSpec = null;
            modeBadge.textContent = 'live';
            liveBtn.hidden = true;
        } else {
            deepBtn.disabled = false;
            liveBtn.disabled = false;
            startCoordinator();
            update();
        }
    }

    function handleScenarioIntent(scenarioId) {
        if (disposed) return;
        const token = ++scenarioSyncToken;
        if (scenarioSyncRaf) cancelAnimationFrame(scenarioSyncRaf);
        scenarioSyncRaf = 0;

        // Suspend on intent, before an older nonempty worker generation can
        // publish a stale field into the null-control panel.
        if (scenarioId === EMPTY_SCENARIO_ID) {
            setEmptyApplicability(true);
            return;
        }

        // A nonempty→nonempty load also replaces the active bridge. Stop the
        // old generation and clear its sampler signature now; otherwise an
        // identical kind list could be mistaken for already registered on the
        // new bridge and the restored panel would read an unrequested cache.
        stopCoordinator();
        cancelDeepMeasurement();
        mode = 'live';
        lastSpec = null;
        modeBadge.textContent = 'live';
        liveBtn.hidden = true;
        deepBtn.disabled = true;
        panel.dataset.applicability = 'pending-scenario';
        for (const body of [specBody, topoBody, metBody, enBody]) {
            body.innerHTML = '<div class="spec-hist-empty">Awaiting the current scenario generation; measurement unavailable.</div>';
        }

        let remaining = SCENARIO_SYNC_MAX_FRAMES;
        const reconcile = () => {
            scenarioSyncRaf = 0;
            if (disposed || token !== scenarioSyncToken) return;
            if (getScale0State().currentScenarioId === scenarioId
                && isScale0AuthoritativeGenerationReady(getScale0State())) {
                setEmptyApplicability(false);
                return;
            }
            remaining--;
            if (remaining > 0) scenarioSyncRaf = requestAnimationFrame(reconcile);
        };
        reconcile();
    }

    function onScenarioChange(event) {
        handleScenarioIntent(String(event.currentTarget?.value || ''));
    }

    function rebindScenarioApplicability() {
        if (disposed) return;
        const nextSelect = document.getElementById('scenario-select');
        if (nextSelect !== scenarioSelect) {
            scenarioSelect?.removeEventListener('change', onScenarioChange);
            scenarioSelect = nextSelect;
            scenarioSelect?.addEventListener('change', onScenarioChange);
        }
        handleScenarioIntent(String(
            scenarioSelect?.value || getScale0State().currentScenarioId || '',
        ));
    }

    deepBtn.addEventListener('click', () => {
        if (disposed || deepPending || inapplicable || !isPanelLive(host)
            || !isScale0AuthoritativeGenerationReady(getScale0State())
            || getScale0State().currentScenarioId === EMPTY_SCENARIO_ID) return;
        const caps = getCaps();
        if (!caps) return;
        analysis.cancel();
        latestAnalysisResult = null;
        const L = caps.latticeSize || 33;
        const owner = getBridge?.();
        const context = currentAnalysisContext(owner);
        const requestToken = ++deepRequestToken;
        analysisOwner = owner;
        analysisGeneration = context.generation;
        mode = 'deep';
        deepPending = true;
        modeBadge.textContent = 'measuring\u2026';
        deepBtn.disabled = true;
        liveBtn.hidden = false;
        setSamplerWants(owner, [...new Set([
            `fluxVector@${liveStride(L)}`, `divJ@${liveStride(L)}`,
            ...METRIC_KINDS.map(m => `${m.kind}@${liveStride(L)}`), 'fluxVector@1',
        ])]);
        // One absolute budget includes availability, queueing and computation.
        const deadline = performance.now() + 5000;
        const fail = () => {
            if (requestToken !== deepRequestToken) return;
            cancelDeepMeasurement('deep sample unavailable');
            update();
        };
        const measure = () => {
            if (requestToken !== deepRequestToken) return;
            deepTimer = 0;
            if (!canPublish(context)) { cancelDeepMeasurement(); return; }
            if (performance.now() >= deadline) { fail(); return; }
            try {
                const observation = captureSpectrumObservation(caps, { L, stride: 1, M: M_DEEP, mode: 'deep' });
                if (!observation.flux) { deepTimer = setTimeout(measure, 50); return; }
                analysis.submit(observation, { context: wireContext(context), deadline,
                    onResult(result) {
                        if (requestToken !== deepRequestToken) return;
                        if (!canPublish(context)) { cancelDeepMeasurement(); return; }
                        if (!result.spectrum) { deepTimer = setTimeout(measure, 50); return; }
                        lastSpec = result.spectrum;
                        renderSpectrum(specBody, lastSpec, true);
                        modeBadge.textContent = 'deep (frozen)';
                        deepPending = false;
                        releaseSamplerWants(true);
                        deepBtn.disabled = false;
                        liveBtn.hidden = false;
                        update();
                    }, onError: fail,
                });
            } catch { fail(); }
        };
        deepTimer = setTimeout(measure, 30);
    });
    liveBtn.addEventListener('click', () => {
        if (disposed || inapplicable) return;
        cancelDeepMeasurement();
        update();
    });

    rebindScenarioApplicability();
    let boundary = null;
    const unsubscribeQualification = subscribeScale0Qualification(q => {
        const next = q.authoritativeLoad
            ? `${q.authoritativeLoad.status}:${q.authoritativeLoad.loadGeneration}`
            : `ready:${q.anchor?.loadGeneration}`;
        if (next === boundary) return;
        boundary = next;
        handleScenarioIntent(q.scenarioId);
    });
    if (!inapplicable) {
        update();
        startCoordinator();
    }

    const api = {
        update,
        element: panel,
        get lastSpec() { return lastSpec; },
        deepMeasure: () => deepBtn.click(),
        get mode() { return mode; },
        get applicability() { return inapplicable ? 'inapplicable-empty' : 'applicable'; },
        get coordinatorActive() { return !!sub; },
        get samplerWantsActive() { return !!samplerWantSignature; },
        get analysisPending() { return analysis.busy || analysis.queued; },
        rebindScenarioApplicability,
        dispose: () => {
            if (disposed) return;
            unsubscribeQualification();
            disposed = true;
            window.removeEventListener?.(PANEL_VISIBILITY_CHANGE_EVENT, onVisibilityChange);
            analysis.dispose();
            stopCoordinator();
            cancelDeepMeasurement();
            if (scenarioSyncRaf) cancelAnimationFrame(scenarioSyncRaf);
            scenarioSyncRaf = 0;
            scenarioSyncToken++;
            scenarioSelect?.removeEventListener('change', onScenarioChange);
            scenarioSelect = null;
            if (typeof window !== 'undefined' && window.__ftdSpectrumPanel === api) window.__ftdSpectrumPanel = null;
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdSpectrumPanel = api;
    return api;
}

export function initSpectrumPanel() {
    if (typeof document === 'undefined') return null;
    if (typeof window !== 'undefined' && window.__ftdSpectrumPanel) {
        window.__ftdSpectrumPanel.rebindScenarioApplicability?.();
        return window.__ftdSpectrumPanel;
    }
    const host = document.getElementById('panel-spectrum');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountSpectrumPanel(host, getBridge);
}
