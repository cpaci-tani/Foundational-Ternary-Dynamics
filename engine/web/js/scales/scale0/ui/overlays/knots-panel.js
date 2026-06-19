import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { getScale0State, setFieldToggle, setKnotTracking, markFieldDirty, resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { getFieldLineKnotTracker, knotHue } from '../../runtime/field-line-knots.js';
import { RingBuffer } from '../../../../telemetry-hub.js';
import { ChartHoverTooltip, formatChartValue } from '../../../../ui/charts/chart-hover-tooltip.js';

// Small fixed-range [0,1] multi-trace line chart for a knot's contribution history.
// (CanvasSparkline is single-trace + streaming + auto-ranged, so a direct draw of the
// three fraction arrays is simpler + keeps the 0–100% axis honest.)
const CONTRIB_TRACES = [
    { key: 'energyFrac', color: '#f6c453', label: 'E' },
    { key: 'fluxFrac', color: '#5ad2e0', label: 'Φ' },
    { key: 'chargeFrac', color: '#c98bf0', label: 'ρ' },
];
function drawContribChart(canvas, hist) {
    if (!canvas) return;
    canvas._tip = (lx, _ly, w) => {
        const m = hist?.n || 0; if (m < 1) return null;
        const i = Math.max(0, Math.min(m - 1, Math.round((lx / w) * (m - 1))));
        return { title: 'knot contribution', xLabel: 'sample', xValue: i, rows: [
            { color: '#f6c453', label: 'energy', value: `${Math.round((hist.energyFrac[i] || 0) * 100)}%` },
            { color: '#5ad2e0', label: 'flux', value: `${Math.round((hist.fluxFrac[i] || 0) * 100)}%` },
            { color: '#c98bf0', label: 'charge', value: `${Math.round((hist.chargeFrac[i] || 0) * 100)}%` },
        ] };
    };
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    if (canvas.width !== w * dpr || canvas.height !== h * dpr) { canvas.width = w * dpr; canvas.height = h * dpr; ctx.setTransform(dpr, 0, 0, dpr, 0, 0); }
    ctx.clearRect(0, 0, w, h);
    const n = hist?.n || 0;
    // gridlines at 0/50/100%
    ctx.strokeStyle = 'rgba(255,255,255,0.08)'; ctx.lineWidth = 1;
    for (const f of [0, 0.5, 1]) { const y = h - f * (h - 2) - 1; ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke(); }
    if (n < 2) return;
    for (const t of CONTRIB_TRACES) {
        const arr = hist[t.key];
        ctx.beginPath();
        for (let i = 0; i < n; i++) {
            const x = (i / (n - 1)) * w;
            const y = h - Math.max(0, Math.min(1, arr[i])) * (h - 2) - 1;   // fixed 0..1 range
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.strokeStyle = t.color; ctx.lineWidth = 1.3; ctx.stroke();
    }
}

// Multi-trace energy line chart (auto-ranged from 0). `traces` = [{rb:RingBuffer,color,width,label}].
function drawEnergyLines(canvas, traces) {
    if (!canvas) return;
    canvas._tip = (lx, _ly, w) => {
        let m = 0; for (const t of traces) m = Math.max(m, t.rb.count); if (m < 1) return null;
        const i = Math.max(0, Math.min(m - 1, Math.round((lx / w) * (m - 1))));
        return { title: 'EM energy', xLabel: 'sample', xValue: i,
            rows: traces.map(t => ({ color: t.color, label: t.label || '', value: formatChartValue(t.rb.count > i ? t.rb.get(i) : null) })) };
    };
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    if (canvas.width !== w * dpr || canvas.height !== h * dpr) { canvas.width = w * dpr; canvas.height = h * dpr; ctx.setTransform(dpr, 0, 0, dpr, 0, 0); }
    ctx.clearRect(0, 0, w, h);
    let n = 0, maxV = 0;
    for (const t of traces) { const c = t.rb.count; if (c > n) n = c; for (let i = 0; i < c; i++) { const v = t.rb.get(i); if (v > maxV) maxV = v; } }
    ctx.strokeStyle = 'rgba(255,255,255,0.07)'; ctx.lineWidth = 1; ctx.beginPath(); ctx.moveTo(0, h - 1); ctx.lineTo(w, h - 1); ctx.stroke();
    if (n < 2 || maxV <= 0) return;
    for (const t of traces) {
        const c = t.rb.count;
        if (c < 2) continue;
        ctx.beginPath();
        for (let i = 0; i < c; i++) {
            const x = (i / (c - 1)) * w;
            const y = h - (t.rb.get(i) / maxV) * (h - 2) - 1;
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.strokeStyle = t.color; ctx.lineWidth = t.width || 1.2; ctx.stroke();
    }
}

// Per-knot EM-energy bars (the "quantization" — discrete knot quanta), colored by hue.
function drawKnotBars(canvas, contrib) {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    if (canvas.width !== w * dpr || canvas.height !== h * dpr) { canvas.width = w * dpr; canvas.height = h * dpr; ctx.setTransform(dpr, 0, 0, dpr, 0, 0); }
    ctx.clearRect(0, 0, w, h);
    const K = contrib?.count || 0;
    if (!K) { canvas._tip = () => null; return; }
    const order = [...Array(K).keys()].sort((a, b) => contrib.energy[b] - contrib.energy[a]).slice(0, 8);
    const maxE = contrib.energy[order[0]] || 1;
    const barH = Math.max(4, (h - 2) / order.length - 2);
    canvas._tip = (_lx, ly) => {
        const row = Math.floor(ly / (barH + 2));
        if (row < 0 || row >= order.length) return null;
        const k = order[row];
        return { title: `knot #${contrib.ids[k]}`, xLabel: 'EM share', xValue: (contrib.energyFrac[k] || 0),
            rows: [
                { color: `hsl(${Math.round(knotHue(contrib.ids[k]) * 360)},85%,55%)`, label: 'EM energy', value: formatChartValue(contrib.energy[k]) },
                { color: '#9ca3af', label: 'share', value: `${Math.round((contrib.energyFrac[k] || 0) * 100)}%` },
            ] };
    };
    let y = 1;
    ctx.font = '9px monospace'; ctx.textBaseline = 'middle';
    for (const k of order) {
        const frac = maxE > 0 ? contrib.energy[k] / maxE : 0;
        const bw = Math.max(1, frac * (w - 46));
        ctx.fillStyle = `hsl(${Math.round(knotHue(contrib.ids[k]) * 360)},85%,55%)`;
        ctx.fillRect(0, y, bw, barH);
        ctx.fillStyle = 'rgba(255,255,255,0.75)';
        ctx.fillText(`#${contrib.ids[k]} ${Math.round((contrib.energyFrac[k] || 0) * 100)}%`, bw + 3, y + barH / 2);
        y += barH + 2;
    }
}

// Wire a canvas's stored `_tip` resolver to the shared ChartHoverTooltip (value at cursor).
function bindCanvasTip(canvas, tooltip, panelRoot) {
    if (!canvas || canvas._tipBound) return;
    canvas._tipBound = true;
    canvas.addEventListener('mousemove', (e) => {
        const r = canvas.getBoundingClientRect();
        const res = canvas._tip?.(e.clientX - r.left, e.clientY - r.top, r.width, r.height);
        if (!res) { tooltip.hide(); return; }
        const pr = panelRoot.getBoundingClientRect();
        tooltip.render({ title: res.title, xLabel: res.xLabel || 'sample', xValue: res.xValue, rows: res.rows,
            anchorLeft: e.clientX - pr.left, anchorTop: e.clientY - pr.top });
    });
    canvas.addEventListener('mouseleave', () => tooltip.hide());
}

const PANEL_ID = 'knots-panel';

// Event type integer order — matches the tracker's event enum:
// 0=Birth 1=Death 2=Persist 3=Fission 4=Fusion 5=Ambiguous.
const EVENT_NAMES = ['Birth', 'Death', 'Persist', 'Fission', 'Fusion', 'Ambig'];
const EVENT_GLYPH = ['✦', '•', '·', '⑂', '⑃', '?'];

// Field-line knots are detected + tracked entirely in JS by FieldLineKnotTracker
// (a module singleton shared with the E-field overlay job, which feeds it the
// rebuilt streamlines). The panel only READS the tracker — no engine/bridge call.
function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById('knots-panel-css')) return;
    const s = document.createElement('style');
    s.id = 'knots-panel-css';
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:14px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .kp-title{font-weight:600;margin:2px 0 6px;font-size:15px}
    #${PANEL_ID} .kp-title small{color:var(--text-muted,#888);font-weight:400;font-size:12.5px}
    #${PANEL_ID} .kp-head{font-family:var(--font-mono,monospace);font-size:14.5px;line-height:1.5;color:var(--text-secondary,#ccc);margin:2px 0 4px}
    #${PANEL_ID} .kp-head #kp-track-dot{font-weight:700}
    #${PANEL_ID} .kp-tally{color:var(--text-muted,#888);font-size:12.5px;margin-top:2px}
    #${PANEL_ID} .kp-ctl{display:flex;align-items:center;cursor:pointer;margin:5px 0 1px;font-size:13px}
    #${PANEL_ID} .kp-ctl input{margin-right:6px}
    #${PANEL_ID} .kp-ctl b{color:var(--text-primary,#eee);font-weight:600}
    #${PANEL_ID} .kp-list{font-family:var(--font-mono,monospace);font-size:14px;line-height:1.55;margin:6px 0 2px;max-height:260px;overflow-y:auto;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.08))}
    #${PANEL_ID} .kp-row{padding:4px 2px;border-bottom:0.5px solid var(--border-light,rgba(255,255,255,0.05));cursor:pointer;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
    #${PANEL_ID} .kp-row:hover{background:var(--surface-hover,rgba(255,255,255,0.04))}
    #${PANEL_ID} .kp-det{margin:3px 0 4px 14px;padding:5px 8px;border-left:2px solid var(--accent-cyan,#3fd0e0);background:var(--surface-raised,rgba(63,208,224,0.06));color:var(--text-secondary,#bbb);font-size:13px;line-height:1.6;white-space:normal}
    #${PANEL_ID} .kp-empty{color:var(--text-muted,#888);font-style:italic;font-size:12.5px;padding:8px 2px;line-height:1.5}
    #${PANEL_ID} .kp-feed-h{margin-top:8px;font-size:11.5px;letter-spacing:0.06em;color:var(--text-muted,#888);font-weight:600}
    #${PANEL_ID} .kp-feed{font-family:var(--font-mono,monospace);font-size:13px;line-height:1.5;max-height:150px;overflow-y:auto;margin-top:3px;color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .kp-feed .kp-t{color:var(--text-muted,#888)}
    #${PANEL_ID} .kp-note{margin-top:8px;padding-top:6px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.1));font-size:11px;color:var(--text-muted,#777);line-height:1.5}
    #${PANEL_ID} .kp-note b{color:var(--text-secondary,#999)}
    #${PANEL_ID} .kp-contrib-sum{margin-top:3px;font-size:12.5px;color:var(--accent-amber,#f6c453)}
    #${PANEL_ID} .kp-contrib-sum b{color:var(--text-primary,#eee);font-weight:600}
    #${PANEL_ID} .kp-cn{color:var(--accent-amber,#f6c453);font-weight:600}
    #${PANEL_ID} .kp-geo{color:var(--text-muted,#888)}
    #${PANEL_ID} .kp-legend{display:block;margin-top:2px;font-size:11.5px;color:var(--text-muted,#999)}
    #${PANEL_ID} .kp-chart{display:block;width:100%;height:50px;margin:2px 0 4px}
    #${PANEL_ID} .kp-em{margin:6px 0 2px;padding:6px 7px;border:0.5px solid var(--border-light,rgba(255,255,255,0.08));border-radius:4px;background:var(--surface-raised,rgba(255,255,255,0.02))}
    #${PANEL_ID} .kp-em-h{font-size:11.5px;letter-spacing:0.06em;color:var(--text-muted,#888);font-weight:600}
    #${PANEL_ID} .kp-em-tot{font-weight:400;letter-spacing:0;color:var(--accent-amber,#f6c453);margin-left:4px}
    #${PANEL_ID} .kp-em-h2{font-size:11px;letter-spacing:0.04em;color:var(--text-muted,#888);margin-top:4px}
    #${PANEL_ID} .kp-em-legend{font-size:11px;color:var(--text-muted,#999);margin:1px 0}
    #${PANEL_ID} .kp-em-chart{display:block;width:100%;height:58px}
    #${PANEL_ID} .kp-em-bars{display:block;width:100%;height:80px;margin-top:2px}
    #${PANEL_ID} canvas{cursor:crosshair}
    `;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
      <div class="kp-title">Knots <small>· Field-Line Diagrams</small></div>
      <div class="kp-head">
        <span id="kp-alive" title="Field-line knots currently tracked (alive this tick)">0</span> alive
        · <span id="kp-segs" title="Total drawn field-line segments across all knots — a seeding-dependent geometric count, NOT a physical amplitude">0</span> segs
        · <span id="kp-track-dot" title="● = tracking on, ○ = off">○</span> tracking
        <div class="kp-tally" id="kp-tally" title="Knot lifecycle this run: births, deaths, fissions (one knot splits), fusions (knots merge), and Σ segments">births 0 · deaths 0 · ⑂0 fissions · ⑃0 fusions · Σsegs —</div>
        <div class="kp-contrib-sum" id="kp-contrib-sum" title="Share of the scenario's actual field (energy/flux/charge) captured by all tracked knots combined — genuine measurements"></div>
      </div>
      <label class="kp-ctl" title="Detect + track the clumps where E field-lines bunch and cross (observation-only)">
        <input type="checkbox" id="kp-toggle-tracking"> <b>Track field-line knots</b> (per rebuild)
      </label>
      <label class="kp-ctl" title="Show the wireframe boxes around detected field-line knots">
        <input type="checkbox" id="kp-toggle-overlay"> <b>Show knot overlays</b>
      </label>
      <label class="kp-ctl" title="Give each tracked knot its own color (boxes + rows). The selected knot is always white. Off = uniform cyan.">
        <input type="checkbox" id="kp-toggle-color"> <b>Per-knot colors</b>
      </label>
      <label class="kp-ctl" title="How readily a field-line clump counts as a knot. Higher = more (fainter) clumps detected; lower = only the densest.">
        <b style="min-width:62px;display:inline-block">Sensitivity</b>
        <input type="range" id="kp-sensitivity" min="0" max="100" value="50" style="flex:1;margin-left:6px">
        <span id="kp-sens-val" style="min-width:30px;text-align:right;color:var(--text-muted,#888)">50%</span>
      </label>
      <div class="kp-em" id="kp-em">
        <div class="kp-em-h" title="Total EM field energy U = ½(E² + B²) and its components over time, from the engine's energy audit">EM ENERGY <span class="kp-em-tot" id="kp-em-totals" title="Current totals: U (EM field energy), E/B split, Coulomb PE, particle kinetic energy"></span></div>
        <div class="kp-em-legend" title="U = total EM field energy ½(E²+B²); E = electric ½|E|²; B = magnetic ½|B|²; wave = radiation energy"><span style="color:#f6c453">▬</span>U(E+B) <span style="color:#5ad2e0">▬</span>E <span style="color:#f08bb0">▬</span>B <span style="color:#9be08b">▬</span>wave</div>
        <canvas class="kp-em-chart" id="kp-em-chart" data-ui-tooltip-skip>Hover for values</canvas>
        <div class="kp-em-h2" title="Each tracked knot's share of the EM field energy — how the energy 'quantizes' across the discrete knots">per-knot quanta — EM energy share</div>
        <canvas class="kp-em-bars" id="kp-em-bars" data-ui-tooltip-skip>Hover for per-knot values</canvas>
      </div>
      <div class="kp-list" id="kp-list" title="One row per tracked knot. Click a row to expand its details, highlight its box white, and chart its contribution over time."></div>
      <div class="kp-feed-h" title="Knot lifecycle events newest-first: ✦ birth · • death · ⑂ fission (split) · ⑃ fusion (merge)">EVENT FEED</div>
      <div class="kp-feed" id="kp-feed"></div>
      <div class="kp-note">
        <b style="color:var(--accent-amber,#f6c453)">Contribution (E / Φ / ρ)</b> = each knot's share of the
        scenario's actual field, integrated over its region — <b>genuine measurements</b>
        (flux exact from the dense |J| volume; energy ½(E²+B²) &amp; charge ∇·J over the
        sub-sampled field). · <b>segments / crossings / legs / length</b> are geometric counts
        of the <i>drawn</i> field lines (seeding-dependent) — <b>NOT</b> physical amplitudes; a
        Feynman-diagram <b>analogy</b>. Ages are integer ticks.
      </div>`;
    return root;
}

export function mountKnotsPanel(host) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${id}`);

    // The (heavier) scientific contribution measurement runs in the E-field job only
    // while this panel is mounted. Mark the field dirty so a sweep measures promptly.
    getFieldLineKnotTracker().setContribEnabled(true);
    markFieldDirty();

    // Scenario EM-energy history (sampled at the panel's 4 Hz from the engine's
    // energy audit). emTotal = ½(E²+B²); E/B/wave are the components.
    const emHub = { emTotal: new RingBuffer(240), eField: new RingBuffer(240), bField: new RingBuffer(240), wave: new RingBuffer(240) };

    // Shared hover tooltip for all charts (value-at-cursor). The static charts are
    // bound once here; the per-knot history chart (rebuilt each paint) binds in update().
    const chartTip = new ChartHoverTooltip(panel);
    bindCanvasTip(el('kp-em-chart'), chartTip, panel);
    bindCanvasTip(el('kp-em-bars'), chartTip, panel);

    const trackCb = el('kp-toggle-tracking');
    const overlayCb = el('kp-toggle-overlay');
    const colorCb = el('kp-toggle-color');

    // Per-knot colors: when on, each knot gets its own deterministic color in
    // both the viewport boxes and the panel rows; the selected knot is white
    // regardless. When off, knots are uniform cyan.
    colorCb.checked = getFieldLineKnotTracker().getPerKnotColor();
    colorCb.addEventListener('change', (e) => {
        getFieldLineKnotTracker().setPerKnotColor(e.target.checked);
        markFieldDirty();   // recolor the flowlines + boxes on the next sweep
        update();
    });

    // Detection sensitivity: higher → lower density threshold → more clumps
    // qualify as knots. Re-detection happens in the next overlay sweep, so mark
    // the field dirty to force one.
    const sensSlider = el('kp-sensitivity');
    const sensVal = el('kp-sens-val');
    sensSlider.value = Math.round(getFieldLineKnotTracker().getSensitivity() * 100);
    sensVal.textContent = sensSlider.value + '%';
    sensSlider.addEventListener('input', (e) => {
        const pct = +e.target.value;
        sensVal.textContent = pct + '%';
        getFieldLineKnotTracker().setSensitivity(pct / 100);
        markFieldDirty();
    });

    // The overlay checkbox drives the VISUAL flag (colored boxes around the
    // detected knots). The boxes are meaningless without tracking data, so
    // enabling the overlay auto-enables tracking (and syncs its checkbox).
    overlayCb.checked = !!getScale0State().fieldFlags.showKnotZones;
    overlayCb.addEventListener('change', (e) => {
        const on = e.target.checked;
        setFieldToggle('showKnotZones', on);
        if (on && !getScale0State().knotTracking) {
            setKnotTracking(true);
            trackCb.checked = true;
        }
    });

    // The tracking checkbox enables the JS FieldLineKnotTracker recorder (fed from
    // the E-field overlay job). Reset on un-check so stale knots/zones clear.
    trackCb.checked = !!getScale0State().knotTracking;
    trackCb.addEventListener('change', (e) => {
        setKnotTracking(e.target.checked);
        if (!e.target.checked) getFieldLineKnotTracker().reset();
    });

    let expandedId = null;

    function renderEmptyList(list, trackingOn) {
        if (!trackingOn) {
            list.innerHTML = '<div class="kp-empty">tracking off — enable "Track field-line knots" to detect knots</div>';
        } else {
            list.innerHTML = '<div class="kp-empty">0 knots — enable the <b>E Field</b> overlay and run; '
                + 'knots form where field-lines bunch &amp; cross (no particles needed)</div>';
        }
    }

    function update() {
        if (!isPanelLive(host)) return;
        const trackingOn = !!getScale0State().knotTracking;
        el('kp-track-dot').textContent = trackingOn ? '●' : '○';

        if (!trackingOn) {
            el('kp-alive').textContent = '0';
            el('kp-segs').textContent = '0';
            el('kp-tally').textContent = 'births 0 · deaths 0 · ⑂0 fissions · ⑃0 fusions · Σsegs —';
            el('kp-contrib-sum').textContent = '';
            el('kp-em').style.display = 'none';
            renderEmptyList(el('kp-list'), false);
            el('kp-feed').innerHTML = '';
            return;
        }
        el('kp-em').style.display = '';

        const fl = getFieldLineKnotTracker();
        const agg = fl.getAggregate();
        const tel = fl.getTelemetry();
        const evs = fl.getEvents();
        const selectedId = fl.getSelected();
        const perColor = fl.getPerKnotColor();
        const contrib = fl.getContributions();
        const cIdx = new Map();                       // knot id → contribution index
        for (let i = 0; i < (contrib.count || 0); i++) cIdx.set(contrib.ids[i], i);
        const pct = (v) => `${Math.round((v || 0) * 100)}%`;

        // Scenario-totals summary: how much of the scenario field all knots capture.
        const hasContrib = contrib.count && (contrib.totals.energy > 0 || contrib.totals.flux > 0 || contrib.totals.charge > 0);
        el('kp-contrib-sum').innerHTML = hasContrib
            ? `knots capture <b>${pct(contrib.captured.energyFrac)}</b> energy · `
              + `<b>${pct(contrib.captured.fluxFrac)}</b> flux · <b>${pct(contrib.captured.chargeFrac)}</b> charge`
            : '<span style="opacity:.7">contribution: enable the <b>E Field</b> overlay + run to measure</span>';

        // ── Scenario EM energy: overall + component breakdown (time-series) ──
        // From the engine's energy audit; EM field energy U = ½(E²+B²).
        const audit = resolveActiveScale0BridgeFromWindow()?.capabilities?.scale0?.getScale0EnergyAudit?.();
        if (audit) {
            const E = audit.EFieldEnergy || 0, B = audit.BFieldEnergy || 0, wv = audit.waveEnergy || 0;
            const U = E + B;
            emHub.emTotal.push(U); emHub.eField.push(E); emHub.bField.push(B); emHub.wave.push(wv);
            const exp = (v) => v.toExponential(2);
            el('kp-em-totals').textContent = `U=${exp(U)} · E ${pct(U > 0 ? E / U : 0)} · B ${pct(U > 0 ? B / U : 0)}`
                + ` · Coulomb ${exp(audit.coulombPE || 0)} · KE ${exp(audit.particleKE || 0)}`;
            drawEnergyLines(el('kp-em-chart'), [
                { rb: emHub.emTotal, color: '#f6c453', width: 1.7, label: 'U (E+B)' },
                { rb: emHub.eField, color: '#5ad2e0', label: 'E-field' },
                { rb: emHub.bField, color: '#f08bb0', label: 'B-field' },
                { rb: emHub.wave, color: '#9be08b', label: 'wave' },
            ]);
        }
        // per-knot quantization bars (current EM-energy share per knot)
        drawKnotBars(el('kp-em-bars'), contrib);

        el('kp-alive').textContent = agg.alive ?? 0;
        el('kp-segs').textContent = agg.sumSegs ?? 0;
        el('kp-tally').textContent =
            `births ${agg.births ?? 0} · deaths ${agg.deaths ?? 0}`
            + ` · ⑂${agg.fissions ?? 0} fissions · ⑃${agg.fusions ?? 0} fusions · Σsegs ${agg.sumSegs ?? 0}`;

        // Per-knot list (flat fields decode, stride 8):
        //   [0..2] cx,cy,cz · [3] segments · [4] crossings · [5] legs · [6] length · [7] |Φ|
        const list = el('kp-list');
        const count = tel?.count ?? 0;
        if (!count) {
            renderEmptyList(list, true);
        } else {
            const f = tel.fields, S = tel.stride || 8;
            const MAX_ROWS = 60;
            const shown = Math.min(count, MAX_ROWS);
            let html = '';
            for (let k = 0; k < shown; k++) {
                const id = tel.ids[k];
                const segs = f[k * S + 3] | 0, xings = f[k * S + 4] | 0, legs = f[k * S + 5] | 0;
                // Each knot's dot matches its viewport box color; selected → white.
                const dotCol = (id === selectedId) ? '#ffffff'
                    : (perColor ? `hsl(${Math.round(knotHue(id) * 360)},85%,62%)` : '#3fd0e0');
                const ci = cIdx.get(id);
                // Lead with the SCIENTIFIC contribution (field share); geometric counts trail.
                const cn = (ci != null)
                    ? `<span class="kp-cn">E${pct(contrib.energyFrac[ci])} Φ${pct(contrib.fluxFrac[ci])} ρ${pct(contrib.chargeFrac[ci])}</span> · `
                    : '';
                const rowTitle = `Knot #${id} — E/Φ/ρ = share of scenario energy/flux/charge (measured). `
                    + `N = knot size (cells), age in ticks. segs/×crossings/legs = drawn field-line geometry (seeding-dependent analogy). Click to expand + highlight.`;
                html += `<div class="kp-row" data-id="${id}" title="${rowTitle}">`
                     +  `<span style="color:${dotCol}">●</span> #${id} · ${cn}`
                     +  `N${tel.size[k]} age${tel.age[k]}t · segs${segs} ×${xings} legs${legs}`;
                if (id === expandedId) {
                    const cx = f[k * S].toFixed(0), cy = f[k * S + 1].toFixed(0), cz = f[k * S + 2].toFixed(0);
                    const ex = tel.extents[k * 3].toFixed(1), ey = tel.extents[k * 3 + 1].toFixed(1), ez = tel.extents[k * 3 + 2].toFixed(1);
                    const len = f[k * S + 6].toFixed(1), fm = f[k * S + 7].toFixed(2);
                    const dx = tel.dirs[k * 3].toFixed(2), dy = tel.dirs[k * 3 + 1].toFixed(2), dz = tel.dirs[k * 3 + 2].toFixed(2);
                    const cstr = (ci != null)
                        ? `<b>contribution</b> ⚡ E ${pct(contrib.energyFrac[ci])} · Φ ${pct(contrib.fluxFrac[ci])} · ρ ${pct(contrib.chargeFrac[ci])} <i>[measured]</i><br>`
                          + `abs U=${contrib.energy[ci].toExponential(2)} · |Φ|=${contrib.flux[ci].toExponential(2)} · Q=${contrib.charge[ci].toExponential(2)}<br>`
                          + `<span class="kp-legend"><span style="color:#f6c453">▬E</span> <span style="color:#5ad2e0">▬Φ</span> <span style="color:#c98bf0">▬ρ</span> · share over time</span>`
                          + `<canvas class="kp-chart" data-cid="${id}"></canvas>`
                        : '';
                    html += `<div class="kp-det">`
                         +  `born t${tel.birth[k]} · peak N${tel.peak[k]}<br>`
                         +  cstr
                         +  `pos(${cx},${cy},${cz}) · extent(${ex},${ey},${ez})<br>`
                         +  `<span class="kp-geo">geometric [analogy]: segs ${segs} · crossings ${xings} · legs ${legs} · length ${len}</span><br>`
                         +  `net flux |Φ|${fm} → (${dx},${dy},${dz}) <i>[proxy]</i>`
                         +  `</div>`;
                }
                html += `</div>`;
            }
            if (count > MAX_ROWS) {
                html += `<div class="kp-empty">… ${count - MAX_ROWS} more (showing ${MAX_ROWS})</div>`;
            }
            list.innerHTML = html;
            // Draw the selected knot's contribution-history chart (canvas rebuilt each paint).
            list.querySelectorAll('canvas.kp-chart').forEach((cv) => {
                drawContribChart(cv, fl.getKnotHistory(+cv.dataset.cid));
                bindCanvasTip(cv, chartTip, panel);
            });
            list.querySelectorAll('.kp-row').forEach((r) => {
                r.onclick = () => {
                    const id = +r.dataset.id;
                    expandedId = (expandedId === id ? null : id);
                    // Drive the viewport highlight: select the knot AND mark the
                    // field dirty so the overlay sweep re-renders the boxes this
                    // frame (even when paused). The seed cache keeps the field-lines
                    // unchanged — only the selected knot's box turns white.
                    getFieldLineKnotTracker().setSelected(expandedId === null ? -1 : expandedId);
                    markFieldDirty();
                    update();
                };
            });
        }

        // Event feed (most recent ~12, newest first).
        const feed = el('kp-feed');
        const ecount = evs?.count ?? 0;
        if (!ecount) {
            feed.innerHTML = '<div class="kp-empty">no events yet</div>';
        } else {
            let h = '';
            for (let i = ecount - 1; i >= 0 && i > ecount - 13; i--) {
                const t = evs.type[i];
                const name = EVENT_NAMES[t] ?? '?';
                const glyph = EVENT_GLYPH[t] ?? '?';
                h += `<div><span class="kp-t">t${evs.tick[i]}</span> ${glyph} ${name} `
                  +  `(${evs.nparents[i]}→${evs.nchildren[i]})</div>`;
            }
            feed.innerHTML = h;
        }
    }

    const { unsubscribe } = rafCoordinator.subscribe(PANEL_ID, { hz: 4, cb: update });
    const dispose = () => { unsubscribe(); getFieldLineKnotTracker().setContribEnabled(false); panel.remove(); };
    window.__ftdKnotsPanel = { dispose };
    return { dispose };
}

export function initKnotsPanel() {
    if (typeof window === 'undefined') return;
    const host = document.getElementById('panel-knots');
    if (!host) return;
    window.__ftdKnotsPanel?.dispose?.();
    mountKnotsPanel(host);
}
