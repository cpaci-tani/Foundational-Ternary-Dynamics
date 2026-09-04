/** Scale 3 molecule-engine lifecycle and scenario orchestration. */

import { BaseLifecycleController } from '../../lifecycle.js';
import { getMolecule } from '../../molecules.js';
import { telemetryHub } from '../../telemetry-hub.js';
import { resetAETogglesToDefaults, syncAEParamsFromUI } from '../scale-utils.js';
import { applyAEScenarioPhysics, applyAEVisualPreset } from '../scale2/controller.js';
import { resetAEExperiment, startAEExperiment } from '../scale2/experiment-runtime.js';
import { Scale3ControlsComponent } from './ui/controls/component.js';
import { getScale3ScenarioMeta, SCALE3_DEFAULT_SCENARIO } from './scenario-registry.js';
import { setupScale3Scenario } from './scenarios.js';
import { renderScale3ScenarioDescription } from './ui/dom.js';

export function resetScale3(ctx) {
    resetAEExperiment(ctx.bridge);
    ctx.inspector?.setCurrentMolecule(null);
}

export function loadMoleculeScenario(ctx, requestedId) {
    const { bridge, viewport, inspector } = ctx;
    if (!bridge.initAE) return false;

    const scenario = getScale3ScenarioMeta(requestedId)
        || getScale3ScenarioMeta(SCALE3_DEFAULT_SCENARIO);
    if (!scenario) throw new Error('Scale 3 registry has no default scenario');

    ctx.resetAllVisualState();
    bridge.initAE();
    telemetryHub.resetScale(2);
    resetAETogglesToDefaults(bridge);
    syncAEParamsFromUI(bridge);

    setupScale3Scenario(bridge, scenario);
    // Construction seeds generic state only. The contract is the final writer
    // for every effective force, parameter, and presentation layer.
    applyAEScenarioPhysics(bridge, scenario);
    bridge.aeSetMoleculeReference?.(scenario.id);
    startAEExperiment(scenario, bridge);
    applyAEVisualPreset(viewport, { visuals: scenario.overlays });

    inspector?.setCurrentMolecule(scenario.moleculeId || null);
    inspector?.setScenarioInfo({
        title: scenario.title,
        desc: scenario.summary,
        fields: {
            Class: scenario.scenarioClass,
            Status: scenario.epistemicStatus,
            Evidence: scenario.evidence,
        },
    });

    const molecule = scenario.moleculeId ? getMolecule(scenario.moleculeId) : null;
    if (viewport) {
        viewport.controls.target.set(0, 0, 0);
        viewport.camera.position.set(0, 0, scenario.cameraDistance || molecule?.cameraDistance || 30);
        viewport.controls.update();
    }

    renderScale3ScenarioDescription(scenario.id);
    return true;
}

// Scale 3 intentionally shares the AtomEngine integrator/render loop. Scenario
// contracts and molecule diagnostics remain Scale-3-owned surfaces.
export { animateAE } from '../scale2/controller.js';

export function bindScale3ControlsUI() {
    const panel = document.getElementById('panel-controls');
    if (panel) new Scale3ControlsComponent(panel).init();
}

class Scale3LifecycleController extends BaseLifecycleController {
    mount() {}

    destroy(ctx) {
        super.destroy(ctx);
        resetScale3(ctx);
    }
}

const lifecycleController = new Scale3LifecycleController();

export function mount(ctx) {
    lifecycleController.mount(ctx);
}

export function destroy(ctx) {
    lifecycleController.destroy(ctx);
}
