import { getScale3ScenarioMeta } from '../scenario-registry.js';

export function renderScale3ScenarioDescription(scenarioId) {
    const wrap = document.getElementById('mol-scenario-desc');
    const text = document.getElementById('mol-scenario-desc-text');
    if (!wrap || !text) return;
    const scenario = getScale3ScenarioMeta(scenarioId);
    if (!scenario) {
        wrap.style.display = 'none';
        text.textContent = '';
        return;
    }
    const phases = scenario.experiment
        ? `\nProtocol: ${scenario.experiment.phases.map((phase) => `T=${phase.tick}: ${phase.label}`).join(' · ')}`
        : '';
    const observe = scenario.experiment ? `\nObserve: ${scenario.experiment.observation}` : '';
    text.textContent = `${scenario.summary}\nStatus: [${scenario.epistemicStatus.toUpperCase()}]${phases}${observe}\nEvidence: ${scenario.evidence}`;
    wrap.style.display = '';
    wrap.open = false;
}
