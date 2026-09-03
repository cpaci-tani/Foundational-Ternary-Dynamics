/**
 * Scale 2 controlled-experiment protocol runner.
 *
 * This module owns presentation-level interventions only. It never changes a
 * force law or evaluates a scenario-specific interaction. Each intervention
 * is declared in the scenario contract and forwarded through generic AtomEngine
 * controls, keeping the physics kernel independent of scenario identity.
 */

let active = null;

function setCheckbox(id, checked) {
    if (typeof document === 'undefined') return;
    const element = document.getElementById(id);
    if (element) element.checked = !!checked;
}

function setThermostatTargetUI(value) {
    if (typeof document === 'undefined') return;
    const valueNode = document.getElementById('ae-thermostat-value');
    const slider = document.getElementById('ae-thermostat-slider');
    if (slider) slider.value = String(value);
    if (valueNode) valueNode.textContent = Number(value).toFixed(2);
}

function publish(bridge, phase) {
    if (!active) return;
    bridge.aeSetExperimentState?.({
        id: active.scenarioId,
        protocol: active.protocol,
        label: active.label,
        phase: phase?.label || 'Ready',
        phaseIndex: active.phaseIndex,
        phaseStartTick: phase?.tick || 0,
        transitionCount: active.transitionCount,
        observation: active.observation,
        complete: active.phases.length > 1 && active.phaseIndex >= active.phases.length - 1,
    });
}

function applyPhase(bridge, phase) {
    switch (`${active?.protocol}:${active?.phaseIndex}`) {
        case 'bond-rupture-cycle:1':
            // Equal and opposite return velocities preserve zero total
            // momentum. IDs are deterministic because the scenario starts
            // from a freshly initialized engine.
            bridge.aeSetAtomVelocity?.(0, 0.5, 0, 0);
            bridge.aeSetAtomVelocity?.(1, -0.5, 0, 0);
            break;
        case 'bond-rupture-cycle:2':
            bridge.aeSetDamping?.(true);
            setCheckbox('ae-damping', true);
            break;
        case 'argon-thermal-cycle:0':
            bridge.aeSetThermostat?.(true);
            bridge.aeSetThermostatTemp?.(1.8);
            setCheckbox('ae-thermostat', true);
            setThermostatTargetUI(1.8);
            break;
        case 'argon-thermal-cycle:1':
            bridge.aeSetThermostat?.(true);
            bridge.aeSetThermostatTemp?.(0.08);
            setCheckbox('ae-thermostat', true);
            setThermostatTargetUI(0.08);
            break;
        case 'argon-thermal-cycle:2':
            bridge.aeSetThermostat?.(false);
            setCheckbox('ae-thermostat', false);
            break;
        default:
            break;
    }
    publish(bridge, phase);
}

export function resetAEExperiment(bridge = null) {
    active = null;
    bridge?.aeSetExperimentState?.(null);
}

export function startAEExperiment(scenario, bridge) {
    resetAEExperiment(bridge);
    const experiment = scenario?.experiment;
    if (!experiment) return null;
    active = {
        scenarioId: scenario.id,
        protocol: experiment.protocol,
        label: experiment.label,
        observation: experiment.observation,
        phases: Array.from(experiment.phases || []),
        phaseIndex: -1,
        transitionCount: 0,
        tick: 0,
    };
    advanceAEExperiment(bridge, 0);
    return bridge.aeGetRuntimeState?.()?.experiment || null;
}

export function advanceAEExperiment(bridge, tickOverride = null) {
    if (!active || active.phases.length === 0) return null;
    // The controller invokes this exactly once after each engine tick. Keep
    // protocol time locally so an experiment does not force an additional
    // O(atoms + bonds) diagnostics reduction on every simulation step.
    active.tick = Number.isFinite(tickOverride)
        ? Number(tickOverride)
        : active.tick + 1;
    const tick = active.tick;
    while (active.phaseIndex + 1 < active.phases.length &&
        tick >= active.phases[active.phaseIndex + 1].tick) {
        active.phaseIndex++;
        active.transitionCount++;
        applyPhase(bridge, active.phases[active.phaseIndex]);
    }
    return bridge.aeGetRuntimeState?.()?.experiment || null;
}

export function getAEExperimentState() {
    if (!active) return null;
    const phase = active.phases[active.phaseIndex] || null;
    return {
        id: active.scenarioId,
        protocol: active.protocol,
        label: active.label,
        phase: phase?.label || 'Ready',
        phaseIndex: active.phaseIndex,
        phaseStartTick: phase?.tick || 0,
        transitionCount: active.transitionCount,
        observation: active.observation,
        complete: active.phases.length > 1 && active.phaseIndex >= active.phases.length - 1,
    };
}
