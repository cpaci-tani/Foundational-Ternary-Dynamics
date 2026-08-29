/**
 * @file engine/web/js/scales/scale0/ui/overlays/wave-lab-panel.js
 * @purpose Side-panel host for standalone RF/light/sound wave instruments.
 */

import { getScale0State, resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { WaveInfoComponent } from './wave-lab/wave-info.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';

const PANEL_ID = 'wave-lab-panel';
const UPDATE_INTERVAL_MS = 250;

function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only s0-overlay-panel p1-observables-panel dock-mode';
    root.style.cssText = `
        position: relative;
        width: 100%;
        padding: 14px 14px 18px;
        background: transparent;
        font-family: var(--font-sans, system-ui, -apple-system, "Segoe UI", sans-serif);
        font-size: 16px;
        line-height: 1.45;
        color: var(--text-primary);
    `;
    root.innerHTML = `
        <header class="p1-panel-header">
            <span class="p1-panel-title">Wave Lab</span>
        </header>
        <div id="${PANEL_ID}-body"></div>
    `;
    return root;
}

export function mountWaveLabPanel(host, getBridge) {
    if (!host) return null;
    const existing = document.getElementById(PANEL_ID);
    if (existing) existing.remove();

    const panel = buildPanel();
    host.appendChild(panel);

    const bodyEl = panel.querySelector(`#${PANEL_ID}-body`);
    const waveInfoComp = new WaveInfoComponent();
    waveInfoComp.mount(bodyEl);

    function update() {
        if (!isPanelLive(host)) return;
        const bridge = getBridge?.();
        if (!bridge) return;
        const state = getScale0State?.() || {};
        waveInfoComp.update(bridge, state.currentScenarioId || '');
    }

    const HZ = Math.round(1000 / UPDATE_INTERVAL_MS);
    const sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });

    const api = {
        update,
        element: panel,
        dispose: () => {
            sub?.unsubscribe?.();
            waveInfoComp.unmount();
            if (typeof window !== 'undefined' && window.__ftdWaveLabPanel === api) {
                window.__ftdWaveLabPanel = null;
            }
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdWaveLabPanel = api;
    update();
    return api;
}

export function initWaveLabPanel() {
    if (typeof document === 'undefined') return null;
    if (window.__ftdWaveLabPanel) return window.__ftdWaveLabPanel;
    const host = document.getElementById('panel-wave-lab');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountWaveLabPanel(host, getBridge);
}
