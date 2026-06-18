import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { getScale0State, setFieldToggle, resolveActiveScale0BridgeFromWindow } from '../../state/store.js';

const PANEL_ID = 'knots-panel';

function ensureCss() {
    if (typeof document === 'undefined' || document.getElementById('knots-panel-css')) return;
    const s = document.createElement('style');
    s.id = 'knots-panel-css';
    s.textContent = `
    #${PANEL_ID}{font-family:var(--font-sans,sans-serif);font-size:12px;color:var(--text-primary,#eee);padding:2px}
    #${PANEL_ID} .kp-title{font-weight:600;margin:2px 0 6px}
    #${PANEL_ID} .kp-title small{color:var(--text-muted,#888);font-weight:400}
    #${PANEL_ID} .kp-rows{margin:4px 0}
    #${PANEL_ID} .kp-row{display:flex;justify-content:space-between;padding:4px 0;border-bottom:0.5px solid var(--border-light,rgba(255,255,255,0.06))}
    #${PANEL_ID} .kp-row span:last-child{font-variant-numeric:tabular-nums;color:var(--text-secondary,#ccc);font-family:var(--font-mono,monospace)}
    #${PANEL_ID} .kp-foot{margin-top:8px;padding-top:7px;border-top:0.5px solid var(--border-light,rgba(255,255,255,0.12));font-size:10px;color:var(--text-muted,#888);line-height:1.45}
    #${PANEL_ID} .kp-foot b{color:var(--text-secondary,#aaa)}
    `;
    document.head.appendChild(s);
}

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.innerHTML = `
        <div class="kp-title">Topological Knots <small>· Manifested States</small></div>
        <div class="kp-rows">
            <div class="kp-row"><span title="Total number of manifested ±1 topological defects">Total Knots (Manifested)</span><span id="kp-val-total">0</span></div>
            <div class="kp-row"><span title="Net charge differential (+1 states minus -1 states)">Charge Total (Differential)</span><span id="kp-val-charge">0</span></div>
        </div>
        <div class="kp-foot">
            <label style="display:flex;align-items:center;cursor:pointer;margin-bottom:8px;">
                <input type="checkbox" id="kp-toggle-overlay" style="margin-right:6px;">
                <b style="color:var(--text-primary,#eee);">Show Knot Overlays</b>
            </label>
            <b>Measurement Pipeline:</b><br>
            Knots (manifested states where <i>s ≠ 0</i>) are measured natively by the C++ Engine via the Lagrangian volume scan. 
            The WASM bridge extracts their positions every frame. These discrete topological defects represent the physical UV cutoff where the continuous flux wave approximation yields to the exact ternary lattice, anchoring the system's renormalization flow at the lattice scale.
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

    const valTotal = el('kp-val-total');
    const valCharge = el('kp-val-charge');
    const toggleOverlay = el('kp-toggle-overlay');

    // Reflect current overlay state; the store is the single source of truth.
    toggleOverlay.checked = !!getScale0State().fieldFlags.showKnotZones;

    toggleOverlay.addEventListener('change', (e) => {
        // setFieldToggle sets the flag, marks the frame dirty, and recomputes
        // anyFieldActive — the overlay pipeline reads getScale0State().fieldFlags.
        setFieldToggle('showKnotZones', e.target.checked);
    });

    function update() {
        if (!isPanelLive(host)) return;
        const bridge = getBridge ? getBridge() : null;
        if (!bridge) return;

        const lag = bridge.getLagrangian();
        const audit = bridge.getEnergyAudit();

        if (lag && typeof lag.manifested !== 'undefined') {
            valTotal.textContent = lag.manifested.toLocaleString();
        }
        if (audit && typeof audit.chargeTotal !== 'undefined') {
            valCharge.textContent = audit.chargeTotal.toLocaleString();
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
    mountKnotsPanel(host, resolveActiveScale0BridgeFromWindow);
}
