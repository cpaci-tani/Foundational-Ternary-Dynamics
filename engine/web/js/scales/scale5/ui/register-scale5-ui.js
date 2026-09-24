import { createScale5ScenarioToolbarGroup, createScale5TelemetryToolbarGroup } from './toolbar/component.js';
import { registerScaleToolbarFactories } from '../../../ui/utils/scale-toolbar.js';

export function registerScale5ToolbarUI(toolbarRegistry) {
    registerScaleToolbarFactories(toolbarRegistry, 5, [{
        id: 'scale5-scenario',
        order: 90,
        factory: () => createScale5ScenarioToolbarGroup(),
    }, {
        id: 'scale5-telemetry',
        order: 100,
        factory: () => createScale5TelemetryToolbarGroup(),
    }]);
}
