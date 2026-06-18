import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { getScale0State, setFieldToggle, resolveActiveScale0BridgeFromWindow } from '../../state/store.js';

const PANEL_ID = 'knots-panel';

// Event type integer order — matches the C++ EventType enum / WASM export:
// 0=Birth 1=Death 2=Persist 3=Fission 4=Fusion 5=Ambiguous.
const EVENT_NAMES = ['Birth', 'Death', 'Persist', 'Fission', 'Fusion', 'Ambig'];
const EVENT_GLYPH = ['✦', '•', '·', '⑂', '⑃', '?'];

// Knot telemetry is computed by the C++ KnotTracker inside the WASM RenderBridge.
// We must read from the bridge that ACTUALLY TICKS:
//   - non-worker path: window.__ftdCtx.bridge (the main-thread WasmBridge);
//   - off-thread path: state.fluxMock (a WasmBridgeProxy — NOT a JS stub; it
//     hosts the real C++ physics in a Web Worker and caches knot telemetry from
//     the worker's 'frame' payload).
// resolveActiveScale0BridgeFromWindow() returns exactly that active owner
// (proxy when useFluxMock, else ctx.bridge). Both surfaces expose
// getKnotAggregate/Telemetry/Events + setToggle('knot_tracking', …), so reading
// the active bridge is correct in BOTH paths. (Reading window.__ftdCtx.bridge
// unconditionally would hit the IDLE main-thread bridge whenever the worker is
// active → empty telemetry + tracking toggled on the wrong engine.)
function resolveKnotBridge() {
    return resolveActiveScale0BridgeFromWindow();
}

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById('knots-panel-css')) return;
    const s = document.createElement('style');
    s.id = 'knots-panel-css';
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:12px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .kp-title{font-weight:600;margin:2px 0 6px}
    #${PANEL_ID} .kp-title small{color:var(--text-muted,#888);font-weight:400}
    #${PANEL_ID} .kp-head{font-family:var(--font-mono,monospace);font-size:12.5px;line-height:1.5;color:var(--text-secondary,#ccc);margin:2px 0 4px}
    #${PANEL_ID} .kp-head #kp-track-dot{font-weight:700}
    #${PANEL_ID} .kp-tally{color:var(--text-muted,#888);font-size:11px;margin-top:2px}
    #${PANEL_ID} .kp-ctl{display:flex;align-items:center;cursor:pointer;margin:5px 0 1px;font-size:11.5px}
    #${PANEL_ID} .kp-ctl input{margin-right:6px}
    #${PANEL_ID} .kp-ctl b{color:var(--text-primary,#eee);font-weight:600}
    #${PANEL_ID} .kp-list{font-family:var(--font-mono,monospace);font-size:12.5px;line-height:1.55;margin:6px 0 2px;max-height:230px;overflow-y:auto;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.08))}
    #${PANEL_ID} .kp-row{padding:3px 2px;border-bottom:0.5px solid var(--border-light,rgba(255,255,255,0.05));cursor:pointer;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
    #${PANEL_ID} .kp-row:hover{background:var(--surface-hover,rgba(255,255,255,0.04))}
    #${PANEL_ID} .kp-det{margin:3px 0 4px 14px;padding:4px 7px;border-left:2px solid var(--accent-cyan,#3fd0e0);background:var(--surface-raised,rgba(63,208,224,0.06));color:var(--text-secondary,#bbb);font-size:11.5px;line-height:1.6;white-space:normal}
    #${PANEL_ID} .kp-empty{color:var(--text-muted,#888);font-style:italic;font-size:11px;padding:8px 2px;line-height:1.5}
    #${PANEL_ID} .kp-feed-h{margin-top:8px;font-size:10px;letter-spacing:0.06em;color:var(--text-muted,#888);font-weight:600}
    #${PANEL_ID} .kp-feed{font-family:var(--font-mono,monospace);font-size:11.5px;line-height:1.5;max-height:130px;overflow-y:auto;margin-top:3px;color:var(--text-secondary,#ccc)}
    #${PANEL_ID} .kp-feed .kp-t{color:var(--text-muted,#888)}
    #${PANEL_ID} .kp-note{margin-top:8px;padding-top:6px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.1));font-size:9.5px;color:var(--text-muted,#777);line-height:1.45}
    #${PANEL_ID} .kp-note b{color:var(--text-secondary,#999)}
    `;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
      <div class="kp-title">Knots <small>· Manifested Diagrams</small></div>
      <div class="kp-head">
        <span id="kp-alive">0</span> alive · net charge <span id="kp-charge">0</span>
        · <span id="kp-track-dot">○</span> tracking
        <div class="kp-tally" id="kp-tally">births 0 · deaths 0 · ⑂0 fissions · ⑃0 fusions · Σsegs —</div>
      </div>
      <label class="kp-ctl" title="Enable the C++ per-tick KnotTracker recorder (observation-only, golden-neutral)">
        <input type="checkbox" id="kp-toggle-tracking"> <b>Track knots</b> (per-tick)
      </label>
      <label class="kp-ctl" title="Show the cyan wireframe knot overlays in the viewport">
        <input type="checkbox" id="kp-toggle-overlay"> <b>Show knot overlays</b>
      </label>
      <div class="kp-list" id="kp-list"></div>
      <div class="kp-feed-h">EVENT FEED</div>
      <div class="kp-feed" id="kp-feed"></div>
      <div class="kp-note">
        Knot = connected same-sign manifested cluster (<i>s ≠ 0</i>, ≥ 4 voxels).
        The Feynman-diagram framing is an <b>analogy</b>; <b>org</b> and any coupling are
        <b>[FTD-native proxies]</b>, not amplitudes. Ages are integer ticks.
      </div>`;
    return root;
}

export function mountKnotsPanel(host, getBridge) {
    if (!host) return null;
    ensureCss();
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const el = (id) => panel.querySelector(`#${id}`);

    const trackCb = el('kp-toggle-tracking');
    const overlayCb = el('kp-toggle-overlay');

    // The overlay checkbox drives the VISUAL flag (cyan cubes) — store is the
    // single source of truth. Unchanged from the prior panel.
    overlayCb.checked = !!getScale0State().fieldFlags.showKnotZones;
    overlayCb.addEventListener('change', (e) => setFieldToggle('showKnotZones', e.target.checked));

    // The tracking checkbox enables the C++ recorder on the WASM bridge via the
    // engine toggle path (NOT setFieldToggle — that is the visual overlay). Wire
    // to the SAME active bridge we read telemetry from (proxy when the worker is
    // active, else the main-thread WasmBridge).
    trackCb.addEventListener('change', (e) => {
        const on = e.target.checked;
        const b = resolveKnotBridge();
        if (!b) return;
        if (b.capabilities?.scale0?.setToggle) b.capabilities.scale0.setToggle('knot_tracking', on);
        else if (b.setToggle) b.setToggle('knot_tracking', on);
    });

    let expandedId = null;

    function renderEmptyList(list, trackingOn) {
        if (!trackingOn) {
            list.innerHTML = '<div class="kp-empty">tracking off — enable "Track knots" to record per-knot telemetry</div>';
        } else {
            list.innerHTML = '<div class="kp-empty">0 knots — manifested clusters ≥ 4 voxels are tracked; '
                + 'isolated single-voxel charges are not</div>';
        }
    }

    function update() {
        if (!isPanelLive(host)) return;
        const bridge = resolveKnotBridge();
        const trackingOn = !!trackCb.checked;

        // Track dot reflects the toggle intent regardless of bridge availability.
        el('kp-track-dot').textContent = trackingOn ? '●' : '○';

        // Tracking OFF: zero the display and bail. Do NOT render stale data — the
        // off-thread proxy may retain a last-frame snapshot after the toggle flips.
        if (!bridge || !trackingOn) {
            el('kp-alive').textContent = '0';
            el('kp-charge').textContent = '0';
            el('kp-tally').textContent = 'births 0 · deaths 0 · ⑂0 fissions · ⑃0 fusions · Σsegs —';
            renderEmptyList(el('kp-list'), trackingOn);
            el('kp-feed').innerHTML = '';
            return;
        }

        const cap = bridge.capabilities?.scale0;
        const agg = cap?.getScale0KnotAggregate?.() ?? bridge.getKnotAggregate?.();
        const tel = cap?.getScale0KnotTelemetry?.() ?? bridge.getKnotTelemetry?.();
        const evs = cap?.getScale0KnotEvents?.() ?? bridge.getKnotEvents?.();

        // Header aggregate.
        const alive = agg?.alive ?? 0;
        const netCharge = agg?.netCharge ?? 0;
        el('kp-alive').textContent = alive;
        el('kp-charge').textContent = (netCharge > 0 ? '+' : '') + netCharge;
        el('kp-tally').textContent =
            `births ${agg?.births ?? 0} · deaths ${agg?.deaths ?? 0}`
            + ` · ⑂${agg?.fissions ?? 0} fissions · ⑃${agg?.fusions ?? 0} fusions · Σsegs —`;

        // Knot list (flat fields decode, stride 11):
        //   [0..2] cx,cy,cz · [3..5] vx,vy,vz · [6] |J| · [7..9] flux dir · [10] org
        const list = el('kp-list');
        const count = tel?.count ?? 0;
        if (!count) {
            renderEmptyList(list, trackingOn);
        } else {
            const f = tel.fields, S = tel.stride || 11;
            const MAX_ROWS = 60;
            const shown = Math.min(count, MAX_ROWS);
            let html = '';
            for (let k = 0; k < shown; k++) {
                const id = tel.ids[k];
                const pos = tel.signs[k] > 0;
                const sgn = pos ? '+' : '−';
                const col = pos ? '#4ddd80' : '#f87070';
                const fm = f[k * S + 6].toFixed(1);
                html += `<div class="kp-row" data-id="${id}">`
                     +  `<span style="color:${col}">●</span> #${id} ${sgn} `
                     +  `N${tel.size[k]} age${tel.age[k]}t |J|${fm} segs—`;
                if (id === expandedId) {
                    const cx = f[k * S].toFixed(0), cy = f[k * S + 1].toFixed(0), cz = f[k * S + 2].toFixed(0);
                    const v = Math.hypot(f[k * S + 3], f[k * S + 4], f[k * S + 5]).toFixed(2);
                    const fx = f[k * S + 7].toFixed(2), fy = f[k * S + 8].toFixed(2), fz = f[k * S + 9].toFixed(2);
                    html += `<div class="kp-det">`
                         +  `born t${tel.birth[k]} · peak N${tel.peak[k]}<br>`
                         +  `pos(${cx},${cy},${cz}) · vel ${v}/t · org ${f[k * S + 10].toFixed(2)} <i>[proxy]</i><br>`
                         +  `flux→(${fx},${fy},${fz})<br>`
                         +  `diagram: legs— · segs— · length— <i>(enable E/B field lines)</i>`
                         +  `</div>`;
                }
                html += `</div>`;
            }
            if (count > MAX_ROWS) {
                html += `<div class="kp-empty">… ${count - MAX_ROWS} more (showing ${MAX_ROWS})</div>`;
            }
            list.innerHTML = html;
            list.querySelectorAll('.kp-row').forEach((r) => {
                r.onclick = () => {
                    const id = +r.dataset.id;
                    expandedId = (expandedId === id ? null : id);
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
    const dispose = () => { unsubscribe(); panel.remove(); };
    window.__ftdKnotsPanel = { dispose };
    return { dispose };
}

export function initKnotsPanel() {
    if (typeof window === 'undefined') return;
    const host = document.getElementById('panel-knots');
    if (!host) return;
    window.__ftdKnotsPanel?.dispose?.();
    mountKnotsPanel(host, resolveKnotBridge);
}
