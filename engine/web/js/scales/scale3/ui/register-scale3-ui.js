import { createScale3ScenarioToolbarGroup } from './toolbar/component.js';
import { registerScaleToolbarFactories } from '../../../ui/utils/scale-toolbar.js';

export function registerScale3ToolbarUI(toolbarRegistry) {
    registerScaleToolbarFactories(toolbarRegistry, 3, [{
        id: 'scale3-scenario',
        order: 40,
        factory: () => createScale3ScenarioToolbarGroup(),
    }]);
}
