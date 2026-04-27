/**
 * Shared shell panel registry.
 *
 * This is the first registry-backed UI surface: tabs, mobile panel select,
 * panel labels, and scale visibility all derive from this one definition set.
 */

export const PANEL_REGISTRY = Object.freeze([
    { id: 'controls',         label: 'Controls',        icon: '\u2699\uFE0E',  scales: null },
    { id: 'diagnostics',      label: 'Diagnostics',     icon: '\u25A4',        scales: ['0', '1', '2', '3'] },
    { id: 'charts',           label: 'Charts',          icon: '\u2248',        scales: ['0', '1', '2', '3'] },
    { id: 'lagrangian',       label: 'Lagrangian',      icon: '\u2112',        scales: ['0'] },
    { id: 'inspector',        label: 'Inspector',       icon: '\u25CE',        scales: ['0', '1', '2', '3', '4', '5'] },
    { id: 'planetary',        label: 'System Explorer', icon: '\u2641',        scales: ['4'] },
    { id: 'zoo',              label: 'Particle Zoo',    icon: '\u229B',        scales: ['1'] },
    { id: 'physics',          label: 'Physics',         icon: '\u03A8',        scales: ['1', '2', '3'] },
    { id: 'hierarchy',        label: 'Hierarchy',       icon: '\u22EE',        scales: ['0', '1', '2', '3'] },
    { id: 'scene',            label: 'Scene',           icon: '\u{1F3AC}',     scales: ['0', '1', '2', '3'] },
    { id: 'flux-slice',       label: 'Flux Slice',      icon: '▦',        scales: ['0'] },
    { id: 'p1-observables',   label: 'P1 Observables',  icon: '⦾',        scales: ['0'] },
    { id: 'spectrum',         label: 'Spectrum',        icon: '〰',       scales: ['0'] },
    { id: 'consciousness',    label: 'Consciousness',   icon: '\u25C9',        scales: ['11'] },
    { id: 'cosmic-info',      label: 'Cosmic',          icon: '\u2740',        scales: ['5'] },
    { id: 'meta-info',        label: 'Meta',            icon: '\u29BF',        scales: ['12'] },
    { id: 'verification-lab', label: 'Verify',          icon: '\u2713',        scales: ['0', '1', '2', '3'] },
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
