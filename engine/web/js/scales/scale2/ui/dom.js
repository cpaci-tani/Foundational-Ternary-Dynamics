/**
 * Scale 2 — DOM helpers (scenario description strip).
 */

import { getAEScenarioMeta } from '../scenario-registry.js';

/**
 * Update the toolbar scenario description strip for the given scenario id.
 * @param {string} scenarioId
 */
export function renderAEScenarioDescription(scenarioId) {
    const wrap = document.getElementById('ae-scenario-desc');
    const text = document.getElementById('ae-scenario-desc-text');
    if (!wrap || !text) return;

    const meta = getAEScenarioMeta(scenarioId);
    if (!meta?.summary) {
        text.textContent = '';
        wrap.style.display = 'none';
        wrap.open = false;
        return;
    }

    const experiment = meta.experiment;
    const protocolLine = experiment
        ? `\nProtocol: ${experiment.phases.map(phase => `T=${phase.tick}: ${phase.label}`).join(' · ')}`
        : '';
    const observationLine = experiment ? `\nObserve: ${experiment.observation}` : '';
    const tagLine = meta.tags?.length ? `\nTags: ${meta.tags.join(', ')}` : '';
    text.textContent = `${meta.summary}${protocolLine}${observationLine}${tagLine}`;
    wrap.style.display = '';
    wrap.open = false;
}
