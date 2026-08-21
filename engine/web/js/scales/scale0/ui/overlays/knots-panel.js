import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { getScale0State, setFieldToggle, setKnotTracking, markFieldDirty, resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { getFieldLineKnotTracker, forEachKnotTracker, knotHue } from '../../runtime/field-line-knots.js';
import { RingBuffer } from '../../../../telemetry-hub.js';
import { ChartHoverTooltip, formatChartValue } from '../../../../ui/charts/chart-hover-tooltip.js';

// Small fixed-range [0,1] multi-trace line chart for a knot's contribution history.
// (CanvasSparkline is single-trace + streaming + auto-ranged, so a direct draw of the
// three fraction arrays is simpler + keeps the 0–100% axis honest.)
const CONTRIB_TRACES = [
    { key: 'energyFrac', color: '#f6c453', label: 'energy' },
    { key: 'fluxFrac', color: '#5ad2e0', label: 'flux' },
    { key: 'chargeFrac', color: '#c98bf0', label: 'charge' },
];

// Reader-friendly number: 27517 → "27.5k", 2.43e6 → "2.4M", 218 → "218", 1.2 → "1.2".
// Replaces raw counts + scientific notation in the panel.
function fmtNum(v) {
    v = +v || 0;
    const a = Math.abs(v);
    if (a >= 1e9) return (v / 1e9).toFixed(a >= 1e10 ? 0 : 1) + 'B';
    if (a >= 1e6) return (v / 1e6).toFixed(a >= 1e7 ? 0 : 1) + 'M';
    if (a >= 1e3) return (v / 1e3).toFixed(a >= 1e4 ? 0 : 1) + 'k';
    if (Number.isInteger(v)) return '' + v;   // 3 → "3", 218 → "218" (no stray ".0")
    if (a >= 10) return v.toFixed(0);
    if (a >= 1) return v.toFixed(1);
    return a === 0 ? '0' : v.toFixed(2);
}
// "1 cell" / "2 cells" — singular reads cleaner for the single-voxel-knot case.
function cells(n) { return `${n} cell${n === 1 ? '' : 's'}`; }
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
    if (n < 2) {   // brand-new knot — chart works, just waiting for a 2nd sample
        ctx.fillStyle = 'rgba(255,255,255,0.4)'; ctx.font = '10px sans-serif'; ctx.textBaseline = 'middle';
        ctx.fillText('collecting history…', 4, h / 2);
        return;
    }
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
    const fld = (k) => (contrib.fields && contrib.fields[k]) || 'e';   // 'e'/'b' per bar
    const tag = (k) => fld(k).toUpperCase();
    const order = [...Array(K).keys()].sort((a, b) => contrib.energy[b] - contrib.energy[a]).slice(0, 8);
    const maxE = contrib.energy[order[0]] || 1;
    const barH = Math.max(4, (h - 2) / order.length - 2);
    canvas._tip = (_lx, ly) => {
        const row = Math.floor(ly / (barH + 2));
        if (row < 0 || row >= order.length) return null;
        const k = order[row];
        return { title: `${tag(k)}-knot #${contrib.ids[k]}`, xLabel: 'EM share', xValue: (contrib.energyFrac[k] || 0),
            rows: [
                { color: `hsl(${Math.round(knotHue(contrib.ids[k], fld(k)) * 360)},85%,55%)`, label: 'EM energy', value: formatChartValue(contrib.energy[k]) },
                { color: '#9ca3af', label: 'share', value: `${Math.round((contrib.energyFrac[k] || 0) * 100)}%` },
            ] };
    };
    let y = 1;
    ctx.font = '9px monospace'; ctx.textBaseline = 'middle';
    for (const k of order) {
        const frac = maxE > 0 ? contrib.energy[k] / maxE : 0;
        const bw = Math.max(1, frac * (w - 56));
        ctx.fillStyle = `hsl(${Math.round(knotHue(contrib.ids[k], fld(k)) * 360)},85%,55%)`;
        ctx.fillRect(0, y, bw, barH);
        ctx.fillStyle = 'rgba(255,255,255,0.75)';
        ctx.fillText(`${tag(k)}#${contrib.ids[k]} ${Math.round((contrib.energyFrac[k] || 0) * 100)}%`, bw + 3, y + barH / 2);
        y += barH + 2;
    }
}

// Merge E + B contributions into one {count, ids, energy, energyFrac, fields} for the bars.
function mergeContrib(eC, bC) {
    const ne = eC?.count || 0, nb = bC?.count || 0, n = ne + nb;
    const ids = new Int32Array(n), energy = new Float64Array(n), energyFrac = new Float64Array(n);
    const fields = new Array(n);
    let j = 0;
    for (let i = 0; i < ne; i++) { ids[j] = eC.ids[i]; energy[j] = eC.energy[i]; energyFrac[j] = eC.energyFrac[i]; fields[j] = 'e'; j++; }
    for (let i = 0; i < nb; i++) { ids[j] = bC.ids[i]; energy[j] = bC.energy[i]; energyFrac[j] = bC.energyFrac[i]; fields[j] = 'b'; j++; }
    return { count: n, ids, energy, energyFrac, fields };
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
// Plain-English meaning of each lifecycle event — shown as the per-row hover tooltip.
const EVENT_DESC = [
    'A new knot formed where the field-lines began to bunch and cross.',
    'A knot dissolved — its field-lines spread back out and it is no longer a clump.',
    'The knot carried over from the previous frame with the same identity.',
    'One knot split into two as its tangle pulled apart.',
    'Two knots merged into one as their tangles overlapped.',
    'An ambiguous reshuffle — several knots split and merged at once, so identities could not be matched one-to-one.',
];

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
    #${PANEL_ID} .kp-field-h{margin:7px 0 2px;font-size:11.5px;font-weight:600;letter-spacing:0.03em}
    #${PANEL_ID} .kp-dim{color:var(--text-muted,#888)}
    #${PANEL_ID} .kp-det b{color:var(--text-secondary,#ccc)}
    `;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
      <div class="kp-title">Field-Line Knots <small>· where the EM field-lines tangle</small></div>
      <div class="kp-head">
        <span id="kp-track-dot" title="● = tracking on, ○ = off">○</span>
        <span id="kp-alive" title="Knots currently tracked, by field family. A knot is a clump where the field-lines bunch and cross.">tracking off</span>
        <div class="kp-tally" id="kp-tally" title="Knot lifecycle this run — born / died, and split (one knot → two) / merged (two → one)">—</div>
        <div class="kp-contrib-sum" id="kp-contrib-sum" title="How much of the scenario's actual field energy each knot family accounts for — a genuine measurement, not the geometric line counts"></div>
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
        <div class="kp-em-h" title="Total electromagnetic field energy U = ½(E² + B²) and how it splits into electric vs magnetic, over time. From the engine's energy audit.">EM FIELD ENERGY <span class="kp-em-tot" id="kp-em-totals"></span></div>
        <div class="kp-em-legend" title="total = the whole EM field energy ½(E²+B²); electric = ½|E|²; magnetic = ½|B|²; wave = radiation energy. Hover the chart for live values."><span style="color:#f6c453">▬</span> total <span style="color:#5ad2e0">▬</span> electric <span style="color:#f08bb0">▬</span> magnetic <span style="color:#9be08b">▬</span> wave</div>
        <canvas class="kp-em-chart" id="kp-em-chart" data-ui-tooltip-skip>Hover for values</canvas>
        <div class="kp-em-h2" title="How the EM energy is split across the individual knots — each bar is one knot's share. Hover a bar for its value.">energy held by each knot</div>
        <canvas class="kp-em-bars" id="kp-em-bars" data-ui-tooltip-skip>Hover for per-knot values</canvas>
      </div>
      <div class="kp-list" id="kp-list" title="One row per tracked knot. Click a row to expand its details, highlight it in white in the 3-D view, and chart its energy share over time."></div>
      <div class="kp-feed-h" title="Knot lifecycle, newest first: ✦ born · • died · ⑂ split in two · ⑃ two merged into one. Each row is tick · field · event · (knots before → after). Hover a row for what it means.">RECENT EVENTS</div>
      <div class="kp-feed" id="kp-feed"></div>
      <div class="kp-note">
        <b>Electric</b> and <b>magnetic</b> knots are the two <b>orthogonal</b> field-line families (E ⊥ B) of the same
        EM field — enable both the <b>Radiative E</b> + <b>B Field</b> overlays to see them together.
        <b style="color:var(--accent-amber,#f6c453)">energy / flux / charge</b> = each knot's share of the
        scenario's actual field over its region — <b>genuine measurements</b>
        (flux exact from the dense |J| volume; energy ½(E²+B²) &amp; charge ∇·J sub-sampled).
        The <b>drawn field-line shape</b> (segments / crossings / legs / length) depends on how the lines are seeded —
        it's a Feynman-diagram <b>analogy</b>, <b>NOT</b> a physical amplitude. Ages are counted in whole ticks.
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

    // The (heavier) scientific contribution measurement runs in the field jobs only
    // while this panel is mounted. Enable for BOTH the E and B trackers; mark the
    // field dirty so a sweep measures promptly.
    forEachKnotTracker((t) => t.setContribEnabled(true));
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
    colorCb.checked = getFieldLineKnotTracker('e').getPerKnotColor();
    colorCb.addEventListener('change', (e) => {
        forEachKnotTracker((t) => t.setPerKnotColor(e.target.checked));
        markFieldDirty();   // recolor the flowlines + boxes on the next sweep
        update();
    });

    // Detection sensitivity: higher → lower density threshold → more clumps
    // qualify as knots. Re-detection happens in the next overlay sweep, so mark
    // the field dirty to force one.
    const sensSlider = el('kp-sensitivity');
    const sensVal = el('kp-sens-val');
    sensSlider.value = Math.round(getFieldLineKnotTracker('e').getSensitivity() * 100);
    sensVal.textContent = sensSlider.value + '%';
    sensSlider.addEventListener('input', (e) => {
        const pct = +e.target.value;
        sensVal.textContent = pct + '%';
        forEachKnotTracker((t) => t.setSensitivity(pct / 100));
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
        if (!e.target.checked) forEachKnotTracker((t) => t.reset());
    });

    let expandedKey = null;   // "<field>:<id>" of the expanded/selected knot row

    function renderEmptyList(list, trackingOn) {
        if (!trackingOn) {
            list.innerHTML = '<div class="kp-empty">tracking off — enable "Track field-line knots" to detect knots</div>';
        } else {
            list.innerHTML = '<div class="kp-empty">0 knots — enable the <b>Radiative E</b> + <b>B Field</b> overlays and run; '
                + 'knots form where field-lines bunch &amp; cross (no particles needed)</div>';
        }
    }

    function update() {
        if (!isPanelLive(host)) return;
        const trackingOn = !!getScale0State().knotTracking;
        el('kp-track-dot').textContent = trackingOn ? '●' : '○';

        if (!trackingOn) {
            el('kp-alive').textContent = 'tracking off';
            el('kp-tally').textContent = '';
            el('kp-contrib-sum').textContent = '';
            el('kp-em').style.display = 'none';
            renderEmptyList(el('kp-list'), false);
            el('kp-feed').innerHTML = '';
            return;
        }
        el('kp-em').style.display = '';

        const E = getFieldLineKnotTracker('e'), B = getFieldLineKnotTracker('b');
        const FIELDS = [{ key: 'e', tag: 'E', name: 'Electric', tr: E }, { key: 'b', tag: 'B', name: 'Magnetic', tr: B }];
        const eAgg = E.getAggregate(), bAgg = B.getAggregate();
        const eC = E.getContributions(), bC = B.getContributions();
        const pct = (v) => `${Math.round((v || 0) * 100)}%`;

        // Header counts (plain words) + per-field "how much energy these knots hold".
        const eTel0 = E.getTelemetry(), bTel0 = B.getTelemetry();
        el('kp-alive').innerHTML = `<b>${eTel0.count}</b> electric · <b>${bTel0.count}</b> magnetic knots`;
        el('kp-tally').textContent =
            `${(eAgg.births || 0) + (bAgg.births || 0)} born · ${(eAgg.deaths || 0) + (bAgg.deaths || 0)} died`
            + ` · ${(eAgg.fissions || 0) + (bAgg.fissions || 0)} split · ${(eAgg.fusions || 0) + (bAgg.fusions || 0)} merged`;

        const anyC = (eC.count && eC.totals.energy > 0) || (bC.count && bC.totals.energy > 0);
        el('kp-contrib-sum').innerHTML = anyC
            ? `These knots hold <b>${pct(eC.captured.energyFrac)}</b> of the field energy (electric) and <b>${pct(bC.captured.energyFrac)}</b> (magnetic)`
            : '<span style="opacity:.7">turn on the <b>Radiative E</b> + <b>B Field</b> overlays and run to measure each knot\'s share</span>';

        // ── Scenario EM energy: total + electric/magnetic breakdown over time ──
        // From the engine's energy audit; EM field energy U = ½(E²+B²).
        const audit = resolveActiveScale0BridgeFromWindow()?.capabilities?.scale0?.getScale0EnergyAudit?.();
        if (audit) {
            const eEn = audit.EFieldEnergy || 0, bEn = audit.BFieldEnergy || 0, wv = audit.waveEnergy || 0;
            const U = eEn + bEn;
            emHub.emTotal.push(U); emHub.eField.push(eEn); emHub.bField.push(bEn); emHub.wave.push(wv);
            el('kp-em-totals').innerHTML = `<b>${fmtNum(U)}</b> total · electric ${pct(U > 0 ? eEn / U : 0)} · magnetic ${pct(U > 0 ? bEn / U : 0)}`;
            drawEnergyLines(el('kp-em-chart'), [
                { rb: emHub.emTotal, color: '#f6c453', width: 1.7, label: 'total (E+B)' },
                { rb: emHub.eField, color: '#5ad2e0', label: 'electric' },
                { rb: emHub.bField, color: '#f08bb0', label: 'magnetic' },
                { rb: emHub.wave, color: '#9be08b', label: 'wave' },
            ]);
        }
        // per-knot quantization bars — both E and B families merged
        drawKnotBars(el('kp-em-bars'), mergeContrib(eC, bC));

        // Per-knot list — E knots then B knots, each row tagged + field-hued.
        // Decode stride 8: [0..2] cx,cy,cz · [3] segs · [4] crossings · [5] legs · [6] length · [7] |Φ|
        const list = el('kp-list');
        let html = '', anyKnots = false;
        for (const fld of FIELDS) {
            const tel = fld.tr.getTelemetry();
            if (!tel.count) continue;
            anyKnots = true;
            const contrib = fld.tr.getContributions();
            const cIdx = new Map();
            for (let i = 0; i < (contrib.count || 0); i++) cIdx.set(contrib.ids[i], i);
            const selectedId = fld.tr.getSelected();
            const perColor = fld.tr.getPerKnotColor();
            const f = tel.fields, S = tel.stride || 8;
            const MAX_ROWS = 40;
            const shown = Math.min(tel.count, MAX_ROWS);
            const headHue = Math.round(knotHue(0, fld.key) * 360);
            html += `<div class="kp-field-h" style="color:hsl(${headHue},70%,62%)" title="Knots detected in the ${fld.name.toLowerCase()} field's streamlines. Each is a clump where the ${fld.tag}-field lines bunch and cross.">${fld.name}-field knots · ${tel.count}</div>`;
            for (let k = 0; k < shown; k++) {
                const id = tel.ids[k]; const key = `${fld.key}:${id}`;
                const segs = f[k * S + 3] | 0, xings = f[k * S + 4] | 0, legs = f[k * S + 5] | 0;
                const dotCol = (id === selectedId) ? '#ffffff'
                    : (perColor ? `hsl(${Math.round(knotHue(id, fld.key) * 360)},85%,62%)` : (fld.key === 'b' ? '#6fcf86' : '#3fd0e0'));
                const ci = cIdx.get(id);
                const meta = `<span class="kp-dim">${cells(tel.size[k])} · age ${tel.age[k]}t</span>`;
                const body = (ci != null)
                    ? `<span class="kp-cn">energy <b>${pct(contrib.energyFrac[ci])}</b> · flux <b>${pct(contrib.fluxFrac[ci])}</b> · charge <b>${pct(contrib.chargeFrac[ci])}</b></span> · ${meta}`
                    : `${meta} <span class="kp-dim">· measuring…</span>`;
                const rowTitle = `${fld.name}-field knot #${id} — energy/flux/charge are this knot's share of the whole scenario's field (measured over its region). `
                    + `"cells" is the knot's size on the detection grid, "age" is how many ticks it has lived. Click to expand and highlight it in the 3-D view.`;
                html += `<div class="kp-row" data-field="${fld.key}" data-id="${id}" title="${rowTitle}">`
                     +  `<span style="color:${dotCol}">●</span> <b>#${id}</b> · ${body}`;
                if (key === expandedKey) {
                    const cx = f[k * S].toFixed(0), cy = f[k * S + 1].toFixed(0), cz = f[k * S + 2].toFixed(0);
                    const ex = tel.extents[k * 3].toFixed(1), ey = tel.extents[k * 3 + 1].toFixed(1), ez = tel.extents[k * 3 + 2].toFixed(1);
                    const len = f[k * S + 6], fm = f[k * S + 7].toFixed(2);
                    const dx = tel.dirs[k * 3].toFixed(2), dy = tel.dirs[k * 3 + 1].toFixed(2), dz = tel.dirs[k * 3 + 2].toFixed(2);
                    const cstr = (ci != null)
                        ? `<b>share of the scenario</b> <i class="kp-dim">[measured]</i><br>`
                          + `&nbsp;&nbsp;energy <b>${pct(contrib.energyFrac[ci])}</b> · flux <b>${pct(contrib.fluxFrac[ci])}</b> · charge <b>${pct(contrib.chargeFrac[ci])}</b><br>`
                          + `&nbsp;&nbsp;<span class="kp-dim">absolute: U ${fmtNum(contrib.energy[ci])} · |Φ| ${fmtNum(contrib.flux[ci])} · Q ${fmtNum(contrib.charge[ci])}</span><br>`
                          + `<span class="kp-legend"><span style="color:#f6c453">▬</span> energy <span style="color:#5ad2e0">▬</span> flux <span style="color:#c98bf0">▬</span> charge · share over time</span>`
                          + `<canvas class="kp-chart" data-cid="${key}"></canvas>`
                        : '<span class="kp-dim">measuring this knot\'s field share…</span><br>';
                    html += `<div class="kp-det">`
                         +  `<span class="kp-dim">born tick ${tel.birth[k]} · peak ${cells(tel.peak[k])} · center (${cx},${cy},${cz}) · size (${ex},${ey},${ez})</span><br>`
                         +  cstr
                         +  `<span class="kp-geo">drawn field-line shape <i>[seeding-dependent analogy, not physical]</i>: ${fmtNum(segs)} segments · ${fmtNum(xings)} crossings · ${legs} legs · length ${fmtNum(len)}</span><br>`
                         +  `<span class="kp-dim">net flux |Φ| ${fm}, pointing (${dx},${dy},${dz})</span>`
                         +  `</div>`;
                }
                html += `</div>`;
            }
            if (tel.count > MAX_ROWS) html += `<div class="kp-empty">… and ${tel.count - MAX_ROWS} more ${fld.name.toLowerCase()} knots (showing first ${MAX_ROWS})</div>`;
        }
        if (!anyKnots) {
            renderEmptyList(list, true);
        } else {
            list.innerHTML = html;
            list.querySelectorAll('canvas.kp-chart').forEach((cv) => {
                const [fk, idStr] = cv.dataset.cid.split(':');
                drawContribChart(cv, getFieldLineKnotTracker(fk).getKnotHistory(+idStr));
                bindCanvasTip(cv, chartTip, panel);
            });
            list.querySelectorAll('.kp-row').forEach((r) => {
                r.onclick = () => {
                    const fk = r.dataset.field, id = +r.dataset.id, key = `${fk}:${id}`;
                    expandedKey = (expandedKey === key ? null : key);
                    // Single selection across both fields: select on this field, clear others.
                    forEachKnotTracker((t, f) => t.setSelected((expandedKey && f === fk) ? id : -1));
                    markFieldDirty();
                    update();
                };
            });
        }

        // Event feed — E + B merged, newest first, tagged.
        const feed = el('kp-feed');
        const merged = [];
        for (const fld of FIELDS) {
            const ev = fld.tr.getEvents();
            for (let i = 0; i < (ev.count || 0); i++) merged.push({ tick: ev.tick[i], type: ev.type[i], np: ev.nparents[i], nc: ev.nchildren[i], tag: fld.tag });
        }
        merged.sort((a, b) => b.tick - a.tick);
        if (!merged.length) {
            feed.innerHTML = '<div class="kp-empty">no events yet</div>';
        } else {
            let h = '';
            for (let i = 0; i < Math.min(12, merged.length); i++) {
                const e = merged[i];
                const name = EVENT_NAMES[e.type] ?? 'Event';
                const fieldName = e.tag === 'B' ? 'magnetic' : 'electric';
                const before = `${e.np} knot${e.np === 1 ? '' : 's'} before → ${e.nc} after`;
                const title = `${name} · tick ${e.tick} · ${fieldName} field. ${EVENT_DESC[e.type] ?? ''} (${before})`;
                h += `<div title="${title}"><span class="kp-t">t${e.tick}</span> ${e.tag} ${EVENT_GLYPH[e.type] ?? '?'} ${name} <span class="kp-t">(${e.np}→${e.nc})</span></div>`;
            }
            feed.innerHTML = h;
        }
    }

    const { unsubscribe } = rafCoordinator.subscribe(PANEL_ID, { hz: 4, cb: update });
    const dispose = () => { unsubscribe(); forEachKnotTracker((t) => t.setContribEnabled(false)); panel.remove(); };
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
