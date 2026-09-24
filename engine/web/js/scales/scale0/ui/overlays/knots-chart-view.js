/** Canvas and text presentation for the field-line knot instrument. */
import { knotHue } from '../../runtime/field-line-knots.js';
import { formatChartValue } from '../../../../ui/charts/chart-hover-tooltip.js';

// Small fixed-range [0,1] multi-trace line chart for a knot's contribution history.
// A generic streaming sparkline is single-trace and auto-ranged, so drawing the
// three fraction arrays directly is simpler and keeps the 0–100% axis honest.
const CONTRIB_TRACES = [
    { key: 'energyFrac', color: '#f6c453', label: 'energy' },
    { key: 'fluxFrac', color: '#5ad2e0', label: 'flux' },
    { key: 'chargeFrac', color: '#c98bf0', label: 'charge' },
];

// Reader-friendly number: 27517 → "27.5k", 2.43e6 → "2.4M", 218 → "218", 1.2 → "1.2".
// Replaces raw counts + scientific notation in the panel.
export function fmtNum(v) {
    if (typeof v !== 'number' || !Number.isFinite(v)) return '—';
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
export function cells(n) { return `${n} cell${n === 1 ? '' : 's'}`; }
export function drawContribChart(canvas, hist, historyControl = null) {
    if (!canvas) return;
    const tickBuffer = {
        count: hist?.n || 0,
        getTick: (index) => hist?.ticks?.[index] ?? index,
    };
    const visibleN = historyControl?.visibleCount(tickBuffer) ?? tickBuffer.count;
    const start = Math.max(0, tickBuffer.count - visibleN);
    canvas._tip = (lx, _ly, w) => {
        const m = visibleN; if (m < 1) return null;
        const i = start + Math.max(0, Math.min(m - 1, Math.round((lx / w) * (m - 1))));
        return { title: 'knot contribution', xLabel: 'tick', xValue: hist?.ticks?.[i] ?? i, rows: [
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
    const n = visibleN;
    // gridlines at 0/50/100%
    ctx.strokeStyle = 'rgba(255,255,255,0.08)'; ctx.lineWidth = 1;
    for (const f of [0, 0.5, 1]) { const y = h - f * (h - 2) - 1; ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke(); }
    if (n < 2) {   // brand-new knot — chart works, just waiting for a 2nd sample
        ctx.fillStyle = 'rgba(255,255,255,0.4)'; ctx.font = '16px sans-serif'; ctx.textBaseline = 'middle';
        ctx.fillText('collecting history…', 4, h / 2);
        return;
    }
    for (const t of CONTRIB_TRACES) {
        const arr = hist[t.key];
        ctx.beginPath();
        for (let i = 0; i < n; i++) {
            const source = start + i;
            const x = (i / (n - 1)) * w;
            const y = h - Math.max(0, Math.min(1, arr[source])) * (h - 2) - 1;   // fixed 0..1 range
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.strokeStyle = t.color; ctx.lineWidth = 1.3; ctx.stroke();
    }
}

// Multi-trace energy line chart (auto-ranged from 0). `traces` = [{rb:RingBuffer,color,width,label}].
export function drawEnergyLines(canvas, traces, historyControl = null) {
    if (!canvas) return;
    const primary = traces[0]?.rb;
    const visibleN = historyControl?.visibleCount(primary) ?? (primary?.count || 0);
    const primaryStart = Math.max(0, (primary?.count || 0) - visibleN);
    canvas._tip = (lx, _ly, w) => {
        if (visibleN < 1) return null;
        const local = Math.max(0, Math.min(visibleN - 1, Math.round((lx / w) * (visibleN - 1))));
        const tick = primary?.getTick?.(primaryStart + local) ?? primaryStart + local;
        return { title: 'EM energy', xLabel: 'tick', xValue: tick,
            rows: traces.map(t => {
                const start = Math.max(0, t.rb.count - visibleN);
                const index = start + local;
                return { color: t.color, label: t.label || '', value: formatChartValue(t.rb.count > index ? t.rb.get(index) : null) };
            }) };
    };
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    if (canvas.width !== w * dpr || canvas.height !== h * dpr) { canvas.width = w * dpr; canvas.height = h * dpr; ctx.setTransform(dpr, 0, 0, dpr, 0, 0); }
    ctx.clearRect(0, 0, w, h);
    const n = visibleN;
    let maxV = 0;
    for (const t of traces) {
        const start = Math.max(0, t.rb.count - n);
        for (let i = start; i < t.rb.count; i++) { const v = t.rb.get(i); if (v > maxV) maxV = v; }
    }
    ctx.strokeStyle = 'rgba(255,255,255,0.07)'; ctx.lineWidth = 1; ctx.beginPath(); ctx.moveTo(0, h - 1); ctx.lineTo(w, h - 1); ctx.stroke();
    if (n < 2 || maxV <= 0) return;
    for (const t of traces) {
        const c = Math.min(t.rb.count, n);
        const start = Math.max(0, t.rb.count - c);
        if (c < 2) continue;
        ctx.beginPath();
        let drawing = false;
        for (let i = 0; i < c; i++) {
            const value = t.rb.get(start + i);
            if (!Number.isFinite(value)) { drawing = false; continue; }
            const x = (i / (c - 1)) * w;
            const y = h - (value / maxV) * (h - 2) - 1;
            if (!drawing) { ctx.moveTo(x, y); drawing = true; }
            else ctx.lineTo(x, y);
        }
        ctx.strokeStyle = t.color; ctx.lineWidth = t.width || 1.2; ctx.stroke();
    }
}

// Per-knot EM-energy bars (the "quantization" — discrete knot quanta), colored by hue.
export function drawKnotBars(canvas, contrib) {
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    if (!w || !h) return;
    if (canvas.width !== w * dpr || canvas.height !== h * dpr) { canvas.width = w * dpr; canvas.height = h * dpr; ctx.setTransform(dpr, 0, 0, dpr, 0, 0); }
    ctx.clearRect(0, 0, w, h);
    const K = contrib?.count || 0;
    if (!K) { canvas._tip = () => null; return; }
    const fld = (k) => (contrib.fields && contrib.fields[k]) || 'e';
    const tag = (k) => ({ e: 'E', b: 'B', flux: 'J' }[fld(k)] || fld(k).toUpperCase());
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
    ctx.font = '16px monospace'; ctx.textBaseline = 'middle';
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
export function mergeContrib(eC, bC, jC) {
    const ne = eC?.count || 0, nb = bC?.count || 0, nj = jC?.count || 0, n = ne + nb + nj;
    const ids = new Int32Array(n), energy = new Float64Array(n), energyFrac = new Float64Array(n);
    const fields = new Array(n);
    let j = 0;
    for (let i = 0; i < ne; i++) { ids[j] = eC.ids[i]; energy[j] = eC.energy[i]; energyFrac[j] = eC.energyFrac[i]; fields[j] = 'e'; j++; }
    for (let i = 0; i < nb; i++) { ids[j] = bC.ids[i]; energy[j] = bC.energy[i]; energyFrac[j] = bC.energyFrac[i]; fields[j] = 'b'; j++; }
    for (let i = 0; i < nj; i++) { ids[j] = jC.ids[i]; energy[j] = jC.energy[i]; energyFrac[j] = jC.energyFrac[i]; fields[j] = 'flux'; j++; }
    return { count: n, ids, energy, energyFrac, fields };
}

// Wire a canvas's stored `_tip` resolver to the shared ChartHoverTooltip (value at cursor).
export function bindCanvasTip(canvas, tooltip, panelRoot) {
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
