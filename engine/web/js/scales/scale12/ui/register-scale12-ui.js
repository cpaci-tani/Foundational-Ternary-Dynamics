import { createScale12MetaToolbarGroup } from './toolbar/component.js';

export function registerScale12ToolbarUI(toolbarRegistry) {
    toolbarRegistry.registerFactory({
        id: 'scale12-meta',
        slot: 'secondary',
        scales: ['12'],
        order: 110,
        factory: () => createScale12MetaToolbarGroup(),
    });
}
