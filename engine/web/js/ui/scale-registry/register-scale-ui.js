import { createToolbarRegistry } from './toolbar-registry.js';
import { createOverlayRegistry } from './overlay-registry.js';
import { getPanelRegistry } from './panel-registry.js';

/**
 * Shared shell UI registry bundle.
 */
export function createScaleUiRegistry() {
    return {
        panels: getPanelRegistry(),
        toolbar: createToolbarRegistry(),
        overlays: createOverlayRegistry(),
    };
}
