import { G_HELIOCENTRIC, G_N } from '../../../../constants.js';
import { SCALE4_DEFAULT_SCENARIO, SCALE4_SCENARIOS } from '../../scenario-registry.js?v=1';

function scenarioOptionsMarkup() {
    const groups = new Map();
    for (const scenario of SCALE4_SCENARIOS) {
        if (!groups.has(scenario.category)) groups.set(scenario.category, []);
        groups.get(scenario.category).push(scenario);
    }
    return [...groups].map(([category, scenarios]) => `
                <optgroup label="${category}">
                    ${scenarios.map((scenario) => `<option value="${scenario.id}"${scenario.id === SCALE4_DEFAULT_SCENARIO ? ' selected' : ''} title="${scenario.summary}">${scenario.title}</option>`).join('\n                    ')}
                </optgroup>`).join('');
}

export function getScale4ScenarioToolbarTemplate() {
    return `
        <div class="tb-group tb-group-scenario scale4-only" id="planetary-controls">
            <label class="tb-label" for="planetary-scenario-select">Scenario</label>
            <select class="tb-select tb-select-scenario-medium" id="planetary-scenario-select">
                ${scenarioOptionsMarkup()}
            </select>
            <label class="tb-label" for="planetary-gravity-mode" title="Physical is the period-faithful AU/M☉/yr model. Presentation mode retains the old slow lattice-G comparison and is not an astronomical time scale.">Dynamics</label>
            <select class="tb-select" id="planetary-gravity-mode">
                <option value="physical" selected>Physical · G=${G_HELIOCENTRIC.toFixed(3)}</option>
                <option value="presentation">Slow comparison · G=${G_N}</option>
            </select>
            <label class="tb-label" for="planetary-time-step" title="The physical time advanced by one Scale 4 tick. The solver automatically subdivides longer ticks for numerical stability; the playback speed control remains a separate tick-rate multiplier.">Tick</label>
            <select class="tb-select" id="planetary-time-step" title="Choose the physical duration of one Scale 4 tick. Playback speed multiplies the number of these ticks advanced per render frame.">
                <option value="minute" selected>1 minute</option>
                <option value="hour">1 hour</option>
                <option value="day">1 day</option>
                <option value="natural" disabled hidden>0.01 natural time</option>
            </select>
        </div>
    `;
}
