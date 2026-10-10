import { formatClockVersus, formatEnergy } from '../../../../ui/components/live-rulers/measure.js';
import { createMooreLattice } from './moore-lattice.js';
import { Sparkline } from '../../../../ui/charts/sparkline.js';
import { TickHistoryControl } from '../../../../ui/charts/history-window.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { makeMooreTrace } from './moore-trace.js';

function engineTick() {
    const raw = resolveActiveScale0BridgeFromWindow()?.currentTick?.();
    const tick = typeof raw === 'bigint' ? Number(raw) : Number(raw);
    return Number.isFinite(tick) ? tick : null;
}

/** Write text only when it changed; the last text is kept on the node. */
function setText(node, text) {
    if (node._text === text) return;
    node._text = text;
    node.textContent = text;
}

/** Reads the viewport's Moore snapshot. It does not scan the lattice. */

export function initMoorePanel() {
    const host = document.getElementById('panel-moore');
    if (!host || host.dataset.mounted === '1') return null;
    host.dataset.mounted = '1';
    const style = document.createElement('style');
    style.textContent = `
        .moore-panel { display: flex; flex-direction: column; gap: 16px; padding: 12px 14px 20px; contain: layout style; }
        .moore-panel [hidden] { display: none !important; }
        .moore-panel-status { margin: 0; color: var(--text-muted, #9aa6b2); }
        .moore-panel-summary { display: flex; flex-direction: column; gap: 16px; margin: 0; }
        .moore-facts { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 8px; margin: 0; }
        .moore-facts div { display: flex; flex-direction: column; gap: 3px; min-width: 0; }
        .moore-facts dt, .moore-energy-card span { color: var(--text-muted, #9aa6b2); font-size: 11px; letter-spacing: 0.04em; }
        .moore-facts dd, .moore-energy-card strong { margin: 0; font-weight: 500; }
        .moore-energies { display: flex; flex-direction: column; gap: 14px; }
        .moore-energy-card { display: flex; flex-direction: column; gap: 6px; }
        .moore-energy-card header { display: flex; justify-content: space-between; align-items: baseline; gap: 12px; }
        .moore-range { margin: 0; color: var(--text-muted, #9aa6b2); font-size: 12px; }
        .moore-spark { position: relative; height: 28px; width: 100%; min-height: 28px; }
        .moore-spark .uplot, .moore-spark canvas { display: block; }
        .moore-panel-table { width: 100%; border-collapse: collapse; font-variant-numeric: tabular-nums; }
        .moore-panel-table th, .moore-panel-table td { text-align: left; padding: 6px 6px; border-bottom: 1px solid rgba(255,255,255,0.08); }
        .moore-panel-table th { color: var(--text-muted, #9aa6b2); font-size: 11px; font-weight: 500; }
        .moore-trend td { padding: 0 6px 10px; border-bottom: 1px solid rgba(255,255,255,0.08); }
        .moore-lattice { display: flex; flex-direction: column; gap: 8px; }
        .moore-lattice header { display: flex; justify-content: space-between; align-items: center; gap: 12px; color: var(--text-muted, #9aa6b2); font-size: 11px; letter-spacing: 0.04em; }
        .moore-lattice-toggle { display: inline-flex; align-items: center; gap: 6px; border: 0; background: transparent; color: inherit; font: inherit; letter-spacing: inherit; cursor: pointer; padding: 0; }
        .moore-lattice-toggle::before { content: ''; width: 0; height: 0; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 5px solid currentColor; }
        .moore-lattice.is-collapsed .moore-lattice-toggle::before { transform: rotate(-90deg); }
        .moore-lattice.is-collapsed .moore-lattice-body,
        .moore-lattice.is-collapsed .moore-lattice-hint { display: none; }
        .moore-lattice-body { display: flex; flex-direction: column; gap: 8px; min-width: 0; }
        .moore-heat { display: flex; flex-wrap: wrap; gap: 6px; }
        .moore-heat button { border: 1px solid var(--border-light, rgba(255,255,255,0.16)); background: var(--bg-input, transparent); color: var(--text-muted, #9aa6b2); border-radius: 6px; padding: 5px 9px; font-size: 11px; cursor: pointer; }
        .moore-heat button[aria-pressed="true"] { color: var(--text-primary, #e8eef5); border-color: var(--accent, #8eb4ff); background: color-mix(in oklch, var(--accent, #8eb4ff) 18%, transparent); }
        .moore-lattice-view { width: 100%; height: 460px; border-radius: 10px; background: rgba(0, 0, 0, 0.28); cursor: grab; touch-action: none; }
        .moore-lattice-view:active { cursor: grabbing; }
        .moore-lattice-readout { margin: 0; min-height: 1.2em; font-variant-numeric: tabular-nums; overflow-wrap: anywhere; }
    `;
    host.append(style);
    host.insertAdjacentHTML('beforeend', `
        <div class="moore-panel">
            <p class="moore-panel-status">Zoom into the Moore neighborhood to read it.</p>
            <div class="moore-panel-summary" hidden>
                <dl class="moore-facts">
                    <div><dt>Focus</dt><dd data-field="focus"></dd></div>
                    <div><dt>Name</dt><dd data-field="name"></dd></div>
                    <div><dt>Neighbors</dt><dd data-field="count"></dd></div>
                    <div><dt>Global clock</dt><dd data-field="global-clock"></dd></div>
                </dl>
                <div class="moore-energies">
                    <article class="moore-energy-card">
                        <header><span>Focus energy</span><strong data-field="focus-energy"></strong></header>
                        <div class="moore-spark" data-spark="focus"></div>
                    </article>
                    <article class="moore-energy-card">
                        <header><span>Neighborhood sum</span><strong data-field="sum"></strong></header>
                        <div class="moore-spark" data-spark="sum"></div>
                        <p class="moore-range">Range <span data-field="range"></span></p>
                    </article>
                </div>
            </div>
            <section class="moore-lattice" hidden>
                <header>
                    <button type="button" class="moore-lattice-toggle" aria-expanded="true" aria-controls="moore-lattice-body">Neighborhood lattice</button>
                    <span class="moore-lattice-hint">Scroll to zoom · drag to rotate</span>
                </header>
                <div class="moore-lattice-body" id="moore-lattice-body">
                <div class="moore-heat" role="group" aria-label="Lattice quantity">
                    <button type="button" data-heat="state" aria-pressed="true">State</button>
                    <button type="button" data-heat="energy" aria-pressed="false">Energy</button>
                    <button type="button" data-heat="flux" aria-pressed="false">Flux</button>
                    <button type="button" data-heat="activation" aria-pressed="false">Activation</button>
                    <button type="button" data-heat="gradient" aria-pressed="false">Gradient</button>
                    <button type="button" data-heat="contrast" aria-pressed="false">Contrast</button>
                </div>
                <div class="moore-heat moore-field" role="group" aria-label="Gradient, curl, or heat map">
                    <button type="button" data-style="heat" aria-pressed="true">Heat map</button>
                    <button type="button" data-style="gradient" aria-pressed="false">Gradient</button>
                    <button type="button" data-style="curl" aria-pressed="false">Curl</button>
                    <button type="button" data-style="links" aria-pressed="false">Links</button>
                    <button type="button" data-cubes aria-pressed="true">Cubes</button>
                </div>
                <canvas class="moore-lattice-view" aria-label="Rotatable Moore neighborhood lattice"></canvas>
                <p class="moore-lattice-readout">Drag to rotate. Scroll to zoom. Click a voxel.</p>
                </div>
            </section>
            <table class="moore-panel-table" hidden>
                <thead><tr><th>Site</th><th>Name</th><th>Energy</th><th>Size</th><th>Clock vs global</th></tr></thead>
                <tbody></tbody>
            </table>
        </div>`);
    const panel = host.querySelector('.moore-panel');
    const historyControl = new TickHistoryControl(panel, { id: 'moore-panel' });
    const status = host.querySelector('.moore-panel-status');
    const summary = host.querySelector('.moore-panel-summary');
    const latticeSection = host.querySelector('.moore-lattice');
    latticeSection.querySelector('.moore-lattice-toggle').addEventListener('click', () => {
        const collapsed = latticeSection.classList.toggle('is-collapsed');
        latticeSection.querySelector('.moore-lattice-toggle').setAttribute('aria-expanded', collapsed ? 'false' : 'true');
        if (!collapsed && last) lattice.setSnapshot(last);
    });
    const lattice = createMooreLattice(
        host.querySelector('.moore-lattice-view'),
        host.querySelector('.moore-lattice-readout'),
        host.querySelector('.moore-heat'),
        host.querySelector('.moore-field'),
    );
    const table = host.querySelector('.moore-panel-table');
    const body = table.querySelector('tbody');
    const fields = Object.fromEntries([...host.querySelectorAll('[data-field]')].map((node) => [node.dataset.field, node]));
    const focusSparkHost = host.querySelector('[data-spark="focus"]');
    const sumSparkHost = host.querySelector('[data-spark="sum"]');
    const rows = [];
    const traces = new Map();
    const sparks = new Map();
    let siteKey = '';
    let focusKey = '';
    let lastTick = null;
    let last = undefined;

    // Table rows and their sparklines are updated only while they are in the
    // panel's scrolled view (plus a margin). The table sits below the lattice
    // view, so most of its 53 rows are usually out of sight, and rewriting
    // them still cost a layout of the whole table on every flush. A row
    // catches up the moment it scrolls into view. `rowsFresh` lets a newly
    // built table paint once in full, before the observer has reported.
    const sparkRows = new Map();   // site key -> the trend row holding its sparkline
    const inView = new WeakSet();
    let rowsFresh = false;
    const rowObserver = typeof IntersectionObserver === 'function'
        ? new IntersectionObserver((entries) => {
            let entered = false;
            for (const entry of entries) {
                if (entry.isIntersecting) {
                    inView.add(entry.target);
                    entered = true;
                } else {
                    inView.delete(entry.target);
                }
            }
            if (entered) flush();
        }, { rootMargin: '160px 0px', scrollMargin: '160px 0px' })
        : null;
    const rowVisible = (row) => !rowObserver || rowsFresh || inView.has(row);

    const trace = (key) => {
        let bucket = traces.get(key);
        if (!bucket) {
            bucket = makeMooreTrace();
            traces.set(key, bucket);
        }
        return bucket;
    };

    const mountSpark = (key, node) => {
        let spark = sparks.get(key);
        if (spark) return spark;
        spark = new Sparkline(node, {
            buffer: trace(key),
            color: 'var(--accent, #8eb4ff)',
            height: 24,
            historyControl,
        });
        sparks.set(key, spark);
        return spark;
    };

    const clearSparks = () => {
        for (const spark of sparks.values()) spark.destroy();
        sparks.clear();
        traces.clear();
        siteKey = '';
        focusKey = '';
        lastTick = null;
        body.replaceChildren();
        rows.length = 0;
        sparkRows.clear();
        rowObserver?.disconnect();
    };

    const resetTraces = () => {
        for (const bucket of traces.values()) bucket.clear();
        lastTick = null;
    };

    const record = (detail) => {
        const focus = detail.focus || {};
        const nextFocus = `${focus.x},${focus.y},${focus.z}`;
        if (nextFocus !== focusKey) {
            focusKey = nextFocus;
            resetTraces();
        }
        const tick = engineTick();
        if (tick !== null && lastTick !== null && tick < lastTick) resetTraces();
        if (tick !== null && tick === lastTick) return;
        const stamp = tick ?? (trace('focus').total);
        lastTick = tick;
        trace('focus').push(focus.joules, stamp);
        trace('sum').push(detail.sum, stamp);
        for (const site of detail.sites || []) {
            trace(`${site.x},${site.y},${site.z}`).push(site.joules, stamp);
        }
    };

    const paint = (detail) => {
        if (!isPanelLive(host)) return;
        if (!detail) {
            const retained = traces.size > 0;
            status.hidden = false;
            status.textContent = retained
                ? 'Sampling pauses outside the Moore neighborhood. Chart history stays.'
                : 'Zoom into the Moore neighborhood to read it.';
            summary.hidden = !retained;
            latticeSection.hidden = !retained;
            table.hidden = !retained;
            if (retained) {
                for (const spark of sparks.values()) spark.update();
                lattice.setSnapshot(last);
            }
            return;
        }
        status.hidden = true;
        summary.hidden = false;
        latticeSection.hidden = false;
        table.hidden = false;
        const focus = detail.focus || {};
        setText(fields.focus, `${focus.x}, ${focus.y}, ${focus.z}`);
        setText(fields.name, focus.name || 'voxel');
        setText(fields.count, String(detail.count ?? 0));
        setText(fields['focus-energy'], formatEnergy(focus.joules));
        setText(fields.sum, formatEnergy(detail.sum));
        setText(fields.range, `${formatEnergy(detail.min)} – ${formatEnergy(detail.max)}`);
        const clockTick = focus.tick ?? detail.sites?.[0]?.tick ?? 0;
        setText(fields['global-clock'], `tick ${Math.max(0, Math.trunc(Number(clockTick) || 0))}`);
        mountSpark('focus', focusSparkHost);
        mountSpark('sum', sumSparkHost);

        const voxels = [];
        if (Number.isFinite(focus.x)) voxels.push({ ...focus, isFocus: true });
        for (const site of detail.sites || []) voxels.push(site);
        const nextKey = voxels.map((site) => `${site.isFocus ? 'f' : 'n'}:${site.x},${site.y},${site.z}`).join('|');
        if (nextKey !== siteKey) {
            siteKey = nextKey;
            for (const [key, spark] of sparks) {
                if (key === 'focus' || key === 'sum') continue;
                spark.destroy();
                sparks.delete(key);
            }
            body.replaceChildren();
            rows.length = 0;
            sparkRows.clear();
            rowObserver?.disconnect();
            rowsFresh = true;
            for (const site of voxels) {
                const key = `${site.x},${site.y},${site.z}`;
                const row = document.createElement('tr');
                row.innerHTML = `<td>${site.x}, ${site.y}, ${site.z}</td><td>${site.name || 'voxel'}</td><td data-energy></td><td data-size></td><td data-clock></td>`;
                body.append(row);
                row._energy = row.querySelector('[data-energy]');
                row._size = row.querySelector('[data-size]');
                row._clock = row.querySelector('[data-clock]');
                row._key = key;
                rows.push(row);
                rowObserver?.observe(row);
                if (site.isFocus) continue;
                const trend = document.createElement('tr');
                trend.className = 'moore-trend';
                const cell = document.createElement('td');
                cell.colSpan = 5;
                const sparkHost = document.createElement('div');
                sparkHost.className = 'moore-spark';
                cell.append(sparkHost);
                trend.append(cell);
                body.append(trend);
                mountSpark(key, sparkHost);
                sparkRows.set(key, trend);
                rowObserver?.observe(trend);
            }
        }
        // Rows are kept in table order when the table is built. Only rows in
        // view are written, and each cell only when its text changes.
        for (let index = 0; index < rows.length; index++) {
            const row = rows[index];
            const site = voxels[index];
            if (!site || !rowVisible(row)) continue;
            setText(row._energy, formatEnergy(site.joules));
            setText(row._size, Number.isFinite(site.size) ? Number(site.size).toFixed(1) : '—');
            setText(row._clock, formatClockVersus(site.phase, site.tick));
        }
    };

    let dirty = false;
    let flushedAt = 0;
    let paintedFocus = '';
    const flush = () => {
        if (!isPanelLive(host) || !last) return;
        // The canvas is drawn before the table is rewritten. Drawing canvas
        // text makes the browser bring styles up to date; done after the
        // table writes, that recalculated the panel on every flush.
        const drawLattice = !latticeSection.classList.contains('is-collapsed');
        const latticeShown = !latticeSection.hidden;
        if (drawLattice && latticeShown) lattice.setSnapshot(last);
        paint(last);
        // First flush after the section was hidden: it has a size only now.
        if (drawLattice && !latticeShown) lattice.setSnapshot(last);
        for (const [key, spark] of sparks) {
            const trend = sparkRows.get(key);
            // The focus and sum sparklines have no table row and always draw.
            if (!trend || rowVisible(trend)) spark.update();
        }
        rowsFresh = false;
        flushedAt = performance.now();
    };
    // A frame is requested only while a snapshot is waiting to be drawn, so
    // the panel costs nothing when no Moore telemetry arrives.
    let pump = 0;
    const visualFrame = (now) => {
        pump = 0;
        if (!dirty) return;
        const hz = window.__ftdCtx?.viewport?.mooreTelemetryHz?.() || 20;
        if (now - flushedAt < 1000 / hz) {
            pump = requestAnimationFrame(visualFrame);
            return;
        }
        dirty = false;
        flush();
    };
    const markDirty = () => {
        dirty = true;
        if (!pump) pump = requestAnimationFrame(visualFrame);
    };
    const onSnapshot = (event) => {
        if (!event.detail) {
            dirty = false;
            if (isPanelLive(host)) paint(last);
            return;
        }
        last = event.detail;
        record(last);
        const focus = last.focus || {};
        const key = `${focus.x},${focus.y},${focus.z}`;
        if (key !== paintedFocus) {
            paintedFocus = key;
            dirty = false;
            if (isPanelLive(host)) flush();
            return;
        }
        markDirty();
    };
    window.addEventListener('ftd:moore-neighborhood', onSnapshot);
    window.addEventListener('ftd:panel-visibility-change', () => {
        dirty = false;
        if (isPanelLive(host)) flush();
    });
    return {
        dispose() {
            cancelAnimationFrame(pump);
            rowObserver?.disconnect();
            window.removeEventListener('ftd:moore-neighborhood', onSnapshot);
            historyControl.destroy();
            lattice.destroy();
            clearSparks();
        },
    };
}
