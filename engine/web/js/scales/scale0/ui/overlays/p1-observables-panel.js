/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables-panel.js
 * @purpose Orchestrator for the Scale 0 P1 Observables panel, composing sub-components.
 */

import { getScale0State, resolveActiveScale0BridgeFromWindow, subscribeScale0Qualification } from '../../state/store.js';
import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { CoulombComponent } from './p1-observables/coulomb.js';
import { AnisotropyComponent } from './p1-observables/anisotropy.js';
import { HydrogenComponent } from './p1-observables/hydrogen.js';
import { BellComponent } from './p1-observables/bell.js';
import { GravityComponent } from './p1-observables/gravity.js';
import { G2Component } from './p1-observables/g2.js';
import { ThomsonComponent } from './p1-observables/thomson.js?v=2';
import { FineStructureComponent } from './p1-observables/fine-structure.js?v=2';
import {
    isPanelLive,
    PANEL_VISIBILITY_CHANGE_EVENT,
} from '../../../../ui/panels/panel-visibility.js';
import { TickHistoryControl } from '../../../../ui/charts/history-window.js';
import { ScenarioApplicabilityBinding } from '../../../../ui/utils/scenario-applicability-binding.js';

const PANEL_ID = 'p1-observables-panel';
const UPDATE_INTERVAL_MS = 250;            // 4 Hz; observables are slow signals
const EMPTY_SCENARIO_ID = 'empty';
const THOMSON_SCENARIO_IDS = new Set([
    's0-field-thomson-scattering',
    's0-field-thomson-unlocked-recoil',
]);

function buildPanel(dockMode = false) {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.className = 'scale0-only s0-overlay-panel p1-observables-panel';
    root.dataset.applicability = 'applicable';
    if (dockMode) root.classList.add('dock-mode');
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
            <div class="p1-panel-header-actions">
                <button class="p1-header-btn p1-btn-reset" title="Reset all simulations">↺</button>
                <button class="p1-header-btn p1-btn-close" title="Close Panel">×</button>
            </div>
        </header>
    `;

    root.innerHTML = `
        ${headerHTML}
        <div id="${PANEL_ID}-body" class="p1-applicable-content"></div>
        <section class="mode-unavailable p1-inapplicable"
                 data-applicability="inapplicable" role="status" hidden>
            <strong>Not applicable — imposed null control</strong>
            <p>Scenario 1 · Empty prepares no source, excitation, material clock,
               particle experiment, bound state, or field probe required by these
               observables. No particle-list or field-volume sampling is performed.</p>
            <p>This is an imposed all-zero control record, not a measurement of
               physical vacuum, zero-point fluctuations, or Standard Model observables.</p>
        </section>
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
    const inapplicableMessage = panel.querySelector('.p1-inapplicable');
    const historyControl = new TickHistoryControl(panel, {
        id: 'p1-observables-panel',
        defaultTicks: 120,
        onChange: () => update(),
    });

    let components = null;
    let inapplicable = false;
    let disposed = false;
    let sub = null;
    let updateCount = 0;
    let scenarioBinding = null;

    // Invalidate retained particle IDs at the transaction boundary even while
    // the panel is hidden and its sampling callback is inactive.
    const unsubscribeQualification = subscribeScale0Qualification(() => {
        components?.g2.invalidateTracking();
    });

    function mountComponents() {
        if (components || disposed) return;
        components = {
            coulomb: new CoulombComponent(),
            anisotropy: new AnisotropyComponent(),
            hydrogen: new HydrogenComponent(),
            bell: new BellComponent(),
            gravity: new GravityComponent(),
            g2: new G2Component(historyControl),
            thomson: new ThomsonComponent(),
            fineStructure: new FineStructureComponent(),
        };
        for (const component of Object.values(components)) component.mount(bodyEl);
    }

    function unmountComponents() {
        if (!components) return;
        for (const component of Object.values(components)) component.unmount();
        components = null;
        bodyEl.replaceChildren();
    }

    function stopCoordinator() {
        sub?.unsubscribe?.();
        sub = null;
    }

    function startCoordinator() {
        if (sub || inapplicable || disposed) return;
        sub = rafCoordinator.subscribe(PANEL_ID, { hz: HZ, cb: update });
    }

    function update() {
        // Scenario intent removes the coordinator before the new generation is
        // committed. This guard also makes direct/manual calls scientifically
        // inert for Empty: no bridge, particle-list, or field-volume access.
        if (inapplicable || getScale0State().currentScenarioId === EMPTY_SCENARIO_ID) {
            if (!inapplicable) setEmptyApplicability(true);
            return;
        }
        if (!components || panel.dataset.applicability !== 'applicable') return;
        if (!isPanelLive(host)) {
            components.gravity.releaseSamplerDemand();
            return;
        }
        const now = performance.now();
        const bridge = getBridge?.();
        if (!bridge) return;

        const state = getScale0State?.() || {};
        const scenarioId = state.currentScenarioId || '';
        // Several P1 cards inspect the same particle list. Read the compact
        // native particle frame once per panel pass and fan the snapshot out,
        // rather than asking the bridge independently from each card.
        const particles = bridge.getScale0ParticleList?.() || [];

        updateCount++;
        components.coulomb.update(bridge, now, particles, scenarioId);
        components.anisotropy.update(bridge, now, particles);
        components.hydrogen.update(bridge, scenarioId);
        components.bell.update(bridge, scenarioId);
        components.gravity.update(bridge, scenarioId, now);
        components.g2.update(bridge, particles, {
            generation: state.authoritativeLoad?.loadGeneration
                ?? state.qualificationAnchor?.loadGeneration ?? null,
            mutationEpoch: state.mutationEpoch,
            ready: !state.authoritativeLoad,
        });
        // Thomson and coupling-audit cards describe the same observation.
        // Acquire it once so their rows cannot mix two bridge reads/ticks.
        const thomsonMetrics = THOMSON_SCENARIO_IDS.has(scenarioId)
            ? (bridge.getThomsonScatteringMetrics?.() || null) : null;
        components.thomson.update(bridge, scenarioId, thomsonMetrics);
        components.fineStructure.update(bridge, scenarioId, thomsonMetrics);
    }

    const handleVisibilityBoundary = () => {
        if (components && !isPanelLive(host)) components.gravity.releaseSamplerDemand();
    };
    window.addEventListener(PANEL_VISIBILITY_CHANGE_EVENT, handleVisibilityBoundary);

    const HZ = Math.round(1000 / UPDATE_INTERVAL_MS);

    function setEmptyApplicability(nextValue) {
        const next = !!nextValue;
        inapplicable = next;
        panel.dataset.applicability = next ? 'inapplicable-empty' : 'applicable';
        panel.classList.toggle('is-inapplicable', next);
        bodyEl.hidden = next;
        bodyEl.setAttribute('aria-hidden', next ? 'true' : 'false');
        inapplicableMessage.hidden = !next;

        if (next) {
            stopCoordinator();
            // Unmounting is part of the scientific boundary: it resets G-2
            // tracking and every component-local history, and removes all
            // experiment controls rather than leaving stale results hidden.
            unmountComponents();
        } else {
            mountComponents();
            startCoordinator();
            update();
        }
    }

    function handleScenarioIntent(scenarioId) {
        // Suspend immediately on selection intent so an older nonempty engine
        // generation cannot publish one last observation into the Empty panel.
        if (scenarioId === EMPTY_SCENARIO_ID) {
            setEmptyApplicability(true);
            return;
        }

        stopCoordinator();
        unmountComponents();
        panel.dataset.applicability = 'pending-scenario';
        bodyEl.hidden = true;
        bodyEl.setAttribute('aria-hidden', 'true');
        inapplicableMessage.hidden = true;

        scenarioBinding.reconcile({
            scenarioId,
            isReady: () => getScale0State().currentScenarioId === scenarioId,
            onReady: () => setEmptyApplicability(false),
        });
    }

    function rebindScenarioApplicability() {
        scenarioBinding?.bind();
    }

    scenarioBinding = new ScenarioApplicabilityBinding({
        getCurrentScenarioId: () => getScale0State().currentScenarioId,
        onIntent: handleScenarioIntent,
    });
    rebindScenarioApplicability();

    const api = {
        update,
        element: panel,
        get applicability() { return inapplicable ? 'inapplicable-empty' : 'applicable'; },
        get coordinatorActive() { return !!sub; },
        get mountedComponentCount() { return components ? Object.keys(components).length : 0; },
        get updateCount() { return updateCount; },
        rebindScenarioApplicability,
        dispose: () => {
            disposed = true;
            window.removeEventListener(PANEL_VISIBILITY_CHANGE_EVENT, handleVisibilityBoundary);
            unsubscribeQualification();
            stopCoordinator();
            unmountComponents();
            scenarioBinding?.dispose();
            scenarioBinding = null;
            if (activeModal) { try { activeModal.close(); } catch {} activeModal = null; }
            if (expandBtnRef && expandClickHandler) {
                expandBtnRef.removeEventListener('click', expandClickHandler);
            }
            historyControl.destroy();
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
    if (typeof window !== 'undefined' && window.__ftdP1Panel) return window.__ftdP1Panel;
    const host = document.getElementById('panel-p1-observables');
    if (!host) return null;
    const getBridge = () => resolveActiveScale0BridgeFromWindow();
    return mountP1ObservablesPanel(host, getBridge, { dockMode: true });
}
