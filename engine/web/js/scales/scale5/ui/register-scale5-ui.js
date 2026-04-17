import { createScale5ScenarioToolbarGroup, createScale5TelemetryToolbarGroup } from './toolbar/component.js';

export function registerScale5ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale5-scenario',
        slot: 'secondary',
        scales: ['5'],
        order: 90,
        factory: () => createScale5ScenarioToolbarGroup(),
    });

    toolbarRegistry.registerFactory({
        id: 'scale5-telemetry',
        slot: 'secondary',
        scales: ['5'],
        order: 100,
        factory: () => createScale5TelemetryToolbarGroup(),
    });
}
