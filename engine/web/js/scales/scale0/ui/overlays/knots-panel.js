import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { getScale0State, setFieldToggle, setKnotTracking } from '../../state/store.js';
import { getFieldLineKnotTracker, knotHue } from '../../runtime/field-line-knots.js';

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
      <div class="kp-title">Knots <small>· Field-Line Diagrams</small></div>
      <div class="kp-head">
        <span id="kp-alive">0</span> alive · <span id="kp-segs">0</span> segs
        · <span id="kp-track-dot">○</span> tracking
        <div class="kp-tally" id="kp-tally">births 0 · deaths 0 · ⑂0 fissions · ⑃0 fusions · Σsegs —</div>
      </div>
      <label class="kp-ctl" title="Detect + track the clumps where E field-lines bunch and cross (observation-only)">
        <input type="checkbox" id="kp-toggle-tracking"> <b>Track field-line knots</b> (per rebuild)
      </label>
      <label class="kp-ctl" title="Show the cyan wireframe boxes around detected field-line knots">
        <input type="checkbox" id="kp-toggle-overlay"> <b>Show knot overlays</b>
      </label>
      <div class="kp-list" id="kp-list"></div>
      <div class="kp-feed-h">EVENT FEED</div>
      <div class="kp-feed" id="kp-feed"></div>
      <div class="kp-note">
        Knot = a clump where the rendered <b>E field-lines</b> bunch <i>and</i> cross.
        <b>segments / crossings / legs / length</b> are geometric counts of the
        <i>drawn</i> field lines (which depend on seeding) — <b>NOT</b> physical
        amplitudes. The Feynman-diagram framing is an <b>analogy</b>. Ages are integer ticks.
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

    const trackCb = el('kp-toggle-tracking');
    const overlayCb = el('kp-toggle-overlay');

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
            renderEmptyList(el('kp-list'), false);
            el('kp-feed').innerHTML = '';
            return;
        }

        const fl = getFieldLineKnotTracker();
        const agg = fl.getAggregate();
        const tel = fl.getTelemetry();
        const evs = fl.getEvents();
        const selectedId = fl.getSelected();

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
                    : `hsl(${Math.round(knotHue(id) * 360)},85%,62%)`;
                html += `<div class="kp-row" data-id="${id}">`
                     +  `<span style="color:${dotCol}">●</span> #${id} `
                     +  `N${tel.size[k]} age${tel.age[k]}t · segs${segs} ×${xings} legs${legs}`;
                if (id === expandedId) {
                    const cx = f[k * S].toFixed(0), cy = f[k * S + 1].toFixed(0), cz = f[k * S + 2].toFixed(0);
                    const ex = tel.extents[k * 3].toFixed(1), ey = tel.extents[k * 3 + 1].toFixed(1), ez = tel.extents[k * 3 + 2].toFixed(1);
                    const len = f[k * S + 6].toFixed(1), fm = f[k * S + 7].toFixed(2);
                    const dx = tel.dirs[k * 3].toFixed(2), dy = tel.dirs[k * 3 + 1].toFixed(2), dz = tel.dirs[k * 3 + 2].toFixed(2);
                    html += `<div class="kp-det">`
                         +  `born t${tel.birth[k]} · peak N${tel.peak[k]}<br>`
                         +  `pos(${cx},${cy},${cz}) · extent(${ex},${ey},${ez})<br>`
                         +  `diagram: segs ${segs} · crossings ${xings} · legs ${legs} · length ${len}<br>`
                         +  `net flux |Φ|${fm} → (${dx},${dy},${dz}) <i>[proxy]</i>`
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
                    // Drive the viewport highlight: selected knot's box turns white.
                    getFieldLineKnotTracker().setSelected(expandedId === null ? -1 : expandedId);
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
    mountKnotsPanel(host);
}
