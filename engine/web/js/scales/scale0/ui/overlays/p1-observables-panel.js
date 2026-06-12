/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js
 * @purpose Orchestrator for the Scale 0 P1 Observables panel, composing sub-components.
 */

import { getScale0State } from '../../state/store.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { CoulombComponent } from './p1-observables/coulomb.js';
import { AnisotropyComponent } from './p1-observables/anisotropy.js';
import { HydrogenComponent } from './p1-observables/hydrogen.js';
import { BellComponent } from './p1-observables/bell.js';
import { GravityComponent } from './p1-observables/gravity.js';
import { G2Component } from './p1-observables/g2.js';

const PANEL_ID = 'p1-observables-panel';
const UPDATE_INTERVAL_MS = 250;            // 4 Hz; observables are slow signals

function buildPanel(dockMode = false) {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only s0-overlay-panel p1-observables-panel';
    const baseTypography = `
        font-family: var(--font-sans, system-ui, -apple-system, "Segoe UI", sans-serif);
        font-size: 13px;
        line-height: 1.45;
        color: var(--text-primary);
    `;
    if (dockMode) {
        root.classList.add('dock-mode');
        root.style.cssText = `
            position: relative;
            width: 100%;
            padding: 14px 14px 18px;
            background: transparent;
            ${baseTypography}
        `;
    } else {
        root.style.cssText = `
            position: absolute;
            bottom: 12px;
            left: 12px;
            width: min(420px, calc(100vw - 20px));
            max-height: 70vh;
            overflow-y: auto;
            background: rgba(8, 12, 20, 0.92);
            border: 1px solid rgba(120, 200, 255, 0.25);
            border-radius: 6px;
            padding: 14px 14px 18px;
            z-index: 50;
            backdrop-filter: blur(4px);
            ${baseTypography}
        `;
    }
    const trailingBtn = dockMode
        ? `<button id="${PANEL_ID}-expand" type="button" class="p1-header-btn" title="Expand to full-screen modal">⛶</button>`
        : `<button id="${PANEL_ID}-collapse" type="button" class="p1-header-btn" title="Collapse">▴</button>`;

    const headerHTML = dockMode ? `
        <header class="p1-panel-header">
            <span class="p1-panel-title">Observables</span>
            ${trailingBtn}
        </header>
    ` : `
        <header class="p1-panel-header">
            <span class="p1-panel-title">P1 Observables</span>
            <div style="display:flex;gap:4px;">
                <button class="p1-header-btn p1-btn-reset" title="Reset all simulations">↺</button>
                <button class="p1-header-btn p1-btn-close" title="Close Panel">×</button>
            </div>
        </header>
    `;

    root.innerHTML = `
        ${headerHTML}
        <div id="${PANEL_ID}-body"></div>
    `;

    const body = root.querySelector(`#${PANEL_ID}-body`);
    if (dockMode) {
        // Expand button is wired by mountP1ObservablesPanel
    } else {
        const collapseBtn = root.querySelector(`#${PANEL_ID}-collapse`);
        let collapsed = false;
        collapseBtn?.addEventListener('click', () => {
            collapsed = !collapsed;
            body.style.display = collapsed ? 'none' : 'block';
            collapseBtn.textContent = collapsed ? '▾' : '▴';
            collapseBtn.title = collapsed ? 'Expand' : 'Collapse';
        });
    }

    return root;
}

function expandPanelToModal(panel, host, onClose) {
    const scrim = document.createElement('div');
    scrim.className = 's0-expand-scrim';
    const modal = document.createElement('div');
    modal.className = 's0-expand-modal s0-modal-narrow';

    let closed = false;
    const close = () => {
        if (closed) return;
        closed = true;
        if (marker.parentNode === host) host.replaceChild(panel, marker);
        else host.appendChild(panel);
        scrim.remove();
        modal.remove();
        if (typeof onClose === 'function') onClose();
    };
    scrim.addEventListener('click', close);

    const closeBtn = document.createElement('button');
    closeBtn.type = 'button';
    closeBtn.className = 's0-expand-close';
    closeBtn.textContent = '×';
    closeBtn.setAttribute('aria-label', 'Close expanded P1 observables');
    closeBtn.addEventListener('click', close);

    const marker = document.createComment('p1-panel-dock-slot');
    host.replaceChild(marker, panel);
    modal.appendChild(panel);
    modal.appendChild(closeBtn);
    document.body.appendChild(scrim);
    document.body.appendChild(modal);

    return { close };
}

export function mountP1ObservablesPanel(host, getBridge, { dockMode = false } = {}) {
    if (!host) return null;
    const existing = document.getElementById(PANEL_ID);
    if (existing) existing.remove();

    const panel = buildPanel(dockMode);
    host.appendChild(panel);

    let activeModal = null;
    let expandClickHandler = null;
    let expandBtnRef = null;
    if (dockMode) {
        expandBtnRef = panel.querySelector(`#${PANEL_ID}-expand`);
        expandClickHandler = () => {
            if (activeModal) {
                activeModal.close();
                activeModal = null;
            } else {
                activeModal = expandPanelToModal(panel, host, () => {
                    activeModal = null;
                    if (expandBtnRef) {
                        expandBtnRef.textContent = '⤢';
                        expandBtnRef.title = 'Expand to full-screen modal';
                        expandBtnRef.dataset.expanded = '';
                    }
                });
                if (expandBtnRef) {
                    expandBtnRef.textContent = '×';
                    expandBtnRef.title = 'Collapse back to dock';
                    expandBtnRef.dataset.expanded = '1';
                }
            }
        };
        expandBtnRef?.addEventListener('click', expandClickHandler);
    }

    const bodyEl = panel.querySelector(`#${PANEL_ID}-body`);

    // Instantiate and mount sub-components
    const coulombComp = new CoulombComponent();
    const anisotropyComp = new AnisotropyComponent();
    const hydrogenComp = new HydrogenComponent();
    const bellComp = new BellComponent();
    const gravityComp = new GravityComponent();
    const g2Comp = new G2Component();

    coulombComp.mount(bodyEl);
    anisotropyComp.mount(bodyEl);
    hydrogenComp.mount(bodyEl);
    bellComp.mount(bodyEl);
    gravityComp.mount(bodyEl);
    g2Comp.mount(bodyEl);

    function update() {
        const now = performance.now();
        const bridge = getBridge?.();
        if (!bridge) return;

        const state = getScale0State?.() || {};
        const scenarioId = state.currentScenarioId || '';

        coulombComp.update(bridge);
        anisotropyComp.update(bridge);
        hydrogenComp.update(bridge, scenarioId);
        bellComp.update(bridge, scenarioId);
        gravityComp.update(bridge, scenarioId, now);
        g2Comp.update(bridge);
    }

    const HZ = Math.round(1000 / UPDATE_INTERVAL_MS);
    const sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });

    const api = {
        update,
        element: panel,
        dispose: () => {
            sub?.unsubscribe?.();
            coulombComp.unmount();
            anisotropyComp.unmount();
            hydrogenComp.unmount();
            bellComp.unmount();
            gravityComp.unmount();
            g2Comp.unmount();
            if (activeModal) { try { activeModal.close(); } catch {} activeModal = null; }
            if (expandBtnRef && expandClickHandler) {
                expandBtnRef.removeEventListener('click', expandClickHandler);
            }
            if (typeof window !== 'undefined' && window.__ftdP1Panel === api) {
                window.__ftdP1Panel = null;
            }
            panel.remove();
        },
    };
    if (typeof window !== 'undefined') window.__ftdP1Panel = api;
    return api;
}

export function initP1ObservablesPanel() {
    if (typeof document === 'undefined') return null;
    const host = document.getElementById('panel-p1-observables');
    if (!host) return null;
    const getBridge = () => {
        const ctx = (typeof window !== 'undefined') ? window.__ftdCtx : null;
        if (!ctx) return null;
        const state = (typeof getScale0State === 'function') ? getScale0State() : null;
        if (state?.useFluxMock && state?.fluxMock) return state.fluxMock;
        return ctx.bridge;
    };
    return mountP1ObservablesPanel(host, getBridge, { dockMode: true });
}
