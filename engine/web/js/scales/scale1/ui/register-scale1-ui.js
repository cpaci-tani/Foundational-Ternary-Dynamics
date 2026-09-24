import { createScale1ScenarioToolbarGroup } from './toolbar/component.js?v=10';
import { registerScaleToolbarFactories } from '../../../ui/utils/scale-toolbar.js';

export function registerScale1ToolbarUI(toolbarRegistry) {
    registerScaleToolbarFactories(toolbarRegistry, 1, [{
        id: 'scale1-scenario',
        order: 20,
        factory: () => createScale1ScenarioToolbarGroup(),
    }]);
}
