import { createScale4ScenarioToolbarGroup } from './toolbar/component.js?v=4';
import { registerScaleToolbarFactories } from '../../../ui/utils/scale-toolbar.js';

export function registerScale4ToolbarUI(toolbarRegistry) {
    registerScaleToolbarFactories(toolbarRegistry, 4, [{
        id: 'scale4-scenario',
        order: 70,
        factory: () => createScale4ScenarioToolbarGroup(),
    }]);
}
