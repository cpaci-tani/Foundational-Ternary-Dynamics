import { Inspector } from '../inspector.js?v=7';

export function createInspectorAppRuntime({ viewport, bridge, setZooMode }) {
    const inspector = new Inspector(viewport, bridge);

    function syncMode(mode) {
        inspector.setEngineMode(mode);
        viewport?.setEngineMode?.(mode);
        setZooMode?.(mode);
    }

    function setBridge(nextBridge) {
        inspector.setBridge(nextBridge);
    }

    return {
        inspector,
        setBridge,
        syncMode,
    };
}
