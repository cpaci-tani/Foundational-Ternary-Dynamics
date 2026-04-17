/**
 * Shared shell panel registry.
 *
 * This is the first registry-backed UI surface: tabs, mobile panel select,
 * panel labels, and scale visibility all derive from this one definition set.
 */

export const PANEL_REGISTRY = Object.freeze([
    { id: 'controls', label: 'Controls', scales: null },
    { id: 'diagnostics', label: 'Diagnostics', scales: ['0', '1', '2', '3'] },
    { id: 'charts', label: 'Charts', scales: ['0', '1', '2', '3'] },
    { id: 'lagrangian', label: 'Lagrangian', scales: ['0'] },
    { id: 'inspector', label: 'Inspector', scales: ['0', '1', '2', '3', '4', '5'] },
    { id: 'planetary', label: 'System Explorer', scales: ['4'] },
    { id: 'zoo', label: 'Particle Zoo', scales: ['1'] },
    { id: 'physics', label: 'Physics', scales: ['1', '2', '3'] },
    { id: 'hierarchy', label: 'Hierarchy', scales: ['0', '1', '2', '3'] },
    { id: 'consciousness', label: 'Consciousness', scales: ['11'] },
    { id: 'cosmic-info', label: 'Cosmic', scales: ['5'] },
    { id: 'meta-info', label: 'Meta', scales: ['12'] },
    { id: 'verification-lab', label: 'Verify', scales: ['0', '1', '2', '3'] },
]);

export function getPanelRegistry() {
    return PANEL_REGISTRY.slice();
}

export function getPanelDefinition(panelId) {
    return PANEL_REGISTRY.find((panel) => panel.id === panelId) || null;
}

export function getPanelLabel(panelId) {
    return getPanelDefinition(panelId)?.label || 'Controls';
}

export function getPanelsForScale(scaleIndex) {
    const scale = String(scaleIndex);
    return PANEL_REGISTRY.filter((panel) => !panel.scales || panel.scales.includes(scale));
}

export function annotatePanelElements(panelArea, panelDefs = PANEL_REGISTRY) {
    if (!panelArea) return;
    panelDefs.forEach((panel) => {
        const panelEl = panelArea.querySelector(`#panel-${panel.id}`);
        if (!panelEl) return;
        panelEl.dataset.panelId = panel.id;
        panelEl.dataset.panelLabel = panel.label;
        if (panel.scales) panelEl.dataset.panelScales = panel.scales.join(',');
    });
}

export function validatePanelRegistry(panelArea, panelDefs = PANEL_REGISTRY) {
    const errors = [];
    const ids = new Set();

    panelDefs.forEach((panel) => {
        if (ids.has(panel.id)) errors.push(`duplicate panel id: ${panel.id}`);
        ids.add(panel.id);
        if (!panel.label) errors.push(`missing label for panel: ${panel.id}`);
        const panelEl = panelArea?.querySelector?.(`#panel-${panel.id}`);
        if (!panelEl) errors.push(`missing DOM panel for registry entry: panel-${panel.id}`);
    });

    panelArea?.querySelectorAll?.('.panel[id^="panel-"]')?.forEach((panelEl) => {
        const panelId = panelEl.id.replace(/^panel-/, '');
        if (!ids.has(panelId)) errors.push(`panel exists in DOM but not registry: ${panelEl.id}`);
    });

    return { ok: errors.length === 0, errors };
}
