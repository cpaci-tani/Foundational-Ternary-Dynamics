import { createScale2ScenarioToolbarGroup } from './toolbar/component.js';

export function registerScale2ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale2-scenario',
        slot: 'secondary',
        scales: ['2'],
        order: 30,
        factory: () => createScale2ScenarioToolbarGroup(),
    });
}
