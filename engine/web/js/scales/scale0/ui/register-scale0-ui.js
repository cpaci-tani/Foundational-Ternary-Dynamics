import { createScale0ScenarioToolbarGroup, createScale0LatticeSizeToolbarGroup } from './toolbar/component.js';

export function registerScale0ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale0-scenario',
        slot: 'secondary',
        scales: ['0'],
        order: 10,
        factory: () => createScale0ScenarioToolbarGroup(),
    });

    toolbarRegistry.registerFactory({
        id: 'scale0-lattice-size',
        slot: 'secondary',
        scales: ['0'],
        order: 15,
        factory: () => createScale0LatticeSizeToolbarGroup(),
    });
}
