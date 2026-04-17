import { createScale23ForceToolbarGroup, createScale23VisualToolbarGroup } from './toolbar/component.js';

export function registerScale23ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale23-visual',
        slot: 'secondary',
        scales: ['2', '3'],
        order: 50,
        factory: () => createScale23VisualToolbarGroup(),
    });

    toolbarRegistry.registerFactory({
        id: 'scale23-force',
        slot: 'secondary',
        scales: ['2', '3'],
        order: 60,
        factory: () => createScale23ForceToolbarGroup(),
    });
}
