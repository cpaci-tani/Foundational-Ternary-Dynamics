import { createScale1ScenarioToolbarGroup } from './toolbar/component.js';

export function registerScale1ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale1-scenario',
        slot: 'secondary',
        scales: ['1'],
        order: 20,
        factory: () => createScale1ScenarioToolbarGroup(),
    });
}
