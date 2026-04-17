import { createScale3ScenarioToolbarGroup } from './toolbar/component.js';

export function registerScale3ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale3-scenario',
        slot: 'secondary',
        scales: ['3'],
        order: 40,
        factory: () => createScale3ScenarioToolbarGroup(),
    });
}
