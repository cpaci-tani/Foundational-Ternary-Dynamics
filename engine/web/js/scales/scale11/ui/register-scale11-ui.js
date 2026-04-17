import { createScale11ScenarioToolbarGroup } from './toolbar/component.js';

export function registerScale11ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale11-scenario',
        slot: 'secondary',
        scales: ['11'],
        order: 80,
        factory: () => createScale11ScenarioToolbarGroup(),
    });
}
