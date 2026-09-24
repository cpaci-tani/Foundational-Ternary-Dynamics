import { createScale2ScenarioToolbarGroup } from './toolbar/component.js';
import { registerScaleToolbarFactories } from '../../../ui/utils/scale-toolbar.js';

export function registerScale2ToolbarUI(toolbarRegistry) {
    registerScaleToolbarFactories(toolbarRegistry, 2, [{
        id: 'scale2-scenario',
        order: 30,
        factory: () => createScale2ScenarioToolbarGroup(),
    }]);
}
