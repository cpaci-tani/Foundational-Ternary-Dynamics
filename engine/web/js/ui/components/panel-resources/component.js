import {
    getCosmicInfoPanelTemplate,
    getInspectorPanelTemplate,
    getMetaInfoPanelTemplate,
    getPhysicsPanelTemplate,
    getPlanetaryPanelTemplate,
    getScaleControlsBlocksTemplate,
    getZooPanelTemplate,
} from './template.js';
// Quantum Lab replaced by the Verify evidence-scoreboard panel.
// See docs/SPEC_VERIFICATION_LAB.md.
import { getVerifyPanelTemplate as getVerificationLabPanelTemplate } from '../../../verify-panel/template.js';
import { getDiagnosticsPanelTemplate } from './diagnostics-template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

function ensurePanel(panelArea, panelId, markup, beforePanelId = null) {
    if (!panelArea || panelArea.querySelector(`#panel-${panelId}`)) return;
    const panelEl = htmlToElement(markup);
    const beforeNode = beforePanelId ? panelArea.querySelector(`#panel-${beforePanelId}`) : null;
    if (beforeNode) {
        panelArea.insertBefore(panelEl, beforeNode);
        return;
    }
    panelArea.appendChild(panelEl);
}

function removePanel(panelArea, panelId) {
    panelArea?.querySelector?.(`#panel-${panelId}`)?.remove();
}

function ensureControlsBlocks(panelArea) {
    const controls = panelArea?.querySelector('#panel-controls');
    const grid = controls?.querySelector('#panel-controls-grid');
    if (!controls || !grid || controls.querySelector('[data-scale-blocks]')) return;
    const wrapper = document.createElement('div');
    wrapper.dataset.scaleBlocks = '1';
    wrapper.innerHTML = getScaleControlsBlocksTemplate();
    controls.insertBefore(wrapper, grid);
}

export function ensurePanelResources(panelArea) {
    removePanel(panelArea, 'ontic');
    ensurePanel(panelArea, 'diagnostics', getDiagnosticsPanelTemplate(), 'charts');
    ensureControlsBlocks(panelArea);
    ensurePanel(panelArea, 'zoo', getZooPanelTemplate(), 'inspector');
    ensurePanel(panelArea, 'inspector', getInspectorPanelTemplate(), 'physics');
    ensurePanel(panelArea, 'planetary', getPlanetaryPanelTemplate(), 'reference frame context');
    ensurePanel(panelArea, 'physics', getPhysicsPanelTemplate(), 'planetary');
    ensurePanel(panelArea, 'cosmic-info', getCosmicInfoPanelTemplate(), 'meta-info');
    // Old 'panel-quantum-lab' replaced by 'panel-verification-lab'.
    // Remove the old panel if it exists (e.g. cached markup), then mount
    // the new one at the end of the panel area.
    removePanel(panelArea, 'quantum-lab');
    ensurePanel(panelArea, 'meta-info', getMetaInfoPanelTemplate(), 'verification-lab');
    ensurePanel(panelArea, 'verification-lab', getVerificationLabPanelTemplate());
}
