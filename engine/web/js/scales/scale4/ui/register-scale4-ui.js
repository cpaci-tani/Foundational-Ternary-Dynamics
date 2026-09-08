import { createScale4ScenarioToolbarGroup } from './toolbar/component.js?v=4';

export function registerScale4ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale4-scenario',
        slot: 'secondary',
        scales: ['4'],
        order: 70,
        factory: () => createScale4ScenarioToolbarGroup(),
    });
}
