import { createScale0ScenarioToolbarGroup, createScale0LatticeSizeToolbarGroup } from './toolbar/component.js?v=4';
import { registerScaleToolbarFactories } from '../../../ui/utils/scale-toolbar.js';

export function registerScale0ToolbarUI(toolbarRegistry) {
    registerScaleToolbarFactories(toolbarRegistry, 0, [{
        id: 'scale0-scenario',
        order: 10,
        factory: () => createScale0ScenarioToolbarGroup(),
    }, {
        id: 'scale0-lattice-size',
        order: 15,
        factory: () => createScale0LatticeSizeToolbarGroup(),
    }]);
}
